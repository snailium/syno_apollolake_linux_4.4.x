#ifndef MY_ABC_HERE
#define MY_ABC_HERE
#endif
 
#ifdef CONFIG_USB_STORAGE_DEBUG
#define DEBUG
#endif

#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/freezer.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/utsname.h>
#if defined(CONFIG_USB_ETRON_HUB)
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#endif  

#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>

#include "usb.h"
#include "scsiglue.h"
#include "transport.h"
#include "protocol.h"
#include "debug.h"
#include "initializers.h"

#include "sierra_ms.h"
#include "option_ms.h"

#if IS_ENABLED(CONFIG_USB_UAS)
#include "uas-detect.h"
#endif

#define DRV_NAME "usb-storage"

MODULE_AUTHOR("Matthew Dharm <mdharm-usb@one-eyed-alien.net>");
MODULE_DESCRIPTION("USB Mass Storage driver for Linux");
MODULE_LICENSE("GPL");

static unsigned int delay_use = 1;
module_param(delay_use, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(delay_use, "seconds to delay before using a new device");

static char quirks[128];
module_param_string(quirks, quirks, sizeof(quirks), S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(quirks, "supplemental list of device IDs and their quirks");

#define UNUSUAL_DEV(idVendor, idProduct, bcdDeviceMin, bcdDeviceMax, \
		    vendor_name, product_name, use_protocol, use_transport, \
		    init_function, Flags) \
{ \
	.vendorName = vendor_name,	\
	.productName = product_name,	\
	.useProtocol = use_protocol,	\
	.useTransport = use_transport,	\
	.initFunction = init_function,	\
}

#define COMPLIANT_DEV	UNUSUAL_DEV

#define USUAL_DEV(use_protocol, use_transport) \
{ \
	.useProtocol = use_protocol,	\
	.useTransport = use_transport,	\
}

#define UNUSUAL_VENDOR_INTF(idVendor, cl, sc, pr, \
		vendor_name, product_name, use_protocol, use_transport, \
		init_function, Flags) \
{ \
	.vendorName = vendor_name,	\
	.productName = product_name,	\
	.useProtocol = use_protocol,	\
	.useTransport = use_transport,	\
	.initFunction = init_function,	\
}

static struct us_unusual_dev us_unusual_dev_list[] = {
#	include "unusual_devs.h"
	{ }		 
};

static struct us_unusual_dev for_dynamic_ids =
		USUAL_DEV(USB_SC_SCSI, USB_PR_BULK);

#undef UNUSUAL_DEV
#undef COMPLIANT_DEV
#undef USUAL_DEV
#undef UNUSUAL_VENDOR_INTF

#ifdef CONFIG_LOCKDEP

static struct lock_class_key us_interface_key[USB_MAXINTERFACES];

static void us_set_lock_class(struct mutex *mutex,
		struct usb_interface *intf)
{
	struct usb_device *udev = interface_to_usbdev(intf);
	struct usb_host_config *config = udev->actconfig;
	int i;

	for (i = 0; i < config->desc.bNumInterfaces; i++) {
		if (config->interface[i] == intf)
			break;
	}

	BUG_ON(i == config->desc.bNumInterfaces);

	lockdep_set_class(mutex, &us_interface_key[i]);
}

#else

static void us_set_lock_class(struct mutex *mutex,
		struct usb_interface *intf)
{
}

#endif

#ifdef CONFIG_PM	 

int usb_stor_suspend(struct usb_interface *iface, pm_message_t message)
{
	struct us_data *us = usb_get_intfdata(iface);

	mutex_lock(&us->dev_mutex);

	if (us->suspend_resume_hook)
		(us->suspend_resume_hook)(us, US_SUSPEND);

	mutex_unlock(&us->dev_mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(usb_stor_suspend);

int usb_stor_resume(struct usb_interface *iface)
{
	struct us_data *us = usb_get_intfdata(iface);

	mutex_lock(&us->dev_mutex);

	if (us->suspend_resume_hook)
		(us->suspend_resume_hook)(us, US_RESUME);

	mutex_unlock(&us->dev_mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(usb_stor_resume);

int usb_stor_reset_resume(struct usb_interface *iface)
{
	struct us_data *us = usb_get_intfdata(iface);

	usb_stor_report_bus_reset(us);

	return 0;
}
EXPORT_SYMBOL_GPL(usb_stor_reset_resume);

#endif  

int usb_stor_pre_reset(struct usb_interface *iface)
{
	struct us_data *us = usb_get_intfdata(iface);

	mutex_lock(&us->dev_mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(usb_stor_pre_reset);

int usb_stor_post_reset(struct usb_interface *iface)
{
	struct us_data *us = usb_get_intfdata(iface);

	usb_stor_report_bus_reset(us);

	mutex_unlock(&us->dev_mutex);
	return 0;
}
EXPORT_SYMBOL_GPL(usb_stor_post_reset);

void fill_inquiry_response(struct us_data *us, unsigned char *data,
		unsigned int data_len)
{
	if (data_len < 36)  
		return;

	memset(data+8, ' ', 28);
	if (data[0]&0x20) {  
	} else {
		u16 bcdDevice = le16_to_cpu(us->pusb_dev->descriptor.bcdDevice);
		int n;

		n = strlen(us->unusual_dev->vendorName);
		memcpy(data+8, us->unusual_dev->vendorName, min(8, n));
		n = strlen(us->unusual_dev->productName);
		memcpy(data+16, us->unusual_dev->productName, min(16, n));

		data[32] = 0x30 + ((bcdDevice>>12) & 0x0F);
		data[33] = 0x30 + ((bcdDevice>>8) & 0x0F);
		data[34] = 0x30 + ((bcdDevice>>4) & 0x0F);
		data[35] = 0x30 + ((bcdDevice) & 0x0F);
	}

	usb_stor_set_xfer_buf(data, data_len, us->srb);
}
EXPORT_SYMBOL_GPL(fill_inquiry_response);

#if defined(CONFIG_USB_ETRON_HUB)
static int usb_stor_no_test_unit_ready(struct us_data *us)
{
	struct usb_device *udev = us->pusb_dev;

	if (us->srb->cmnd[0] != TEST_UNIT_READY)
		return -EINVAL;

	if (udev->descriptor.idVendor != cpu_to_le16(0x1759) ||
		udev->descriptor.idProduct != cpu_to_le16(0x5002))
		return -EINVAL;

	return 0;
}
#endif  

static int usb_stor_control_thread(void * __us)
{
	struct us_data *us = (struct us_data *)__us;
	struct Scsi_Host *host = us_to_host(us);

	for (;;) {
		usb_stor_dbg(us, "*** thread sleeping\n");
		if (wait_for_completion_interruptible(&us->cmnd_ready))
			break;

		usb_stor_dbg(us, "*** thread awakened\n");

		mutex_lock(&(us->dev_mutex));

		scsi_lock(host);

		if (us->srb == NULL) {
			scsi_unlock(host);
			mutex_unlock(&us->dev_mutex);
			usb_stor_dbg(us, "-- exiting\n");
			break;
		}

		if (test_bit(US_FLIDX_TIMED_OUT, &us->dflags)) {
			us->srb->result = DID_ABORT << 16;
			goto SkipForAbort;
		}

		scsi_unlock(host);

		if (us->srb->sc_data_direction == DMA_BIDIRECTIONAL) {
			usb_stor_dbg(us, "UNKNOWN data direction\n");
			us->srb->result = DID_ERROR << 16;
		}

		else if (us->srb->device->id &&
				!(us->fflags & US_FL_SCM_MULT_TARG)) {
			usb_stor_dbg(us, "Bad target number (%d:%llu)\n",
				     us->srb->device->id,
				     us->srb->device->lun);
			us->srb->result = DID_BAD_TARGET << 16;
		}

		else if (us->srb->device->lun > us->max_lun) {
			usb_stor_dbg(us, "Bad LUN (%d:%llu)\n",
				     us->srb->device->id,
				     us->srb->device->lun);
			us->srb->result = DID_BAD_TARGET << 16;
		}

		else if ((us->srb->cmnd[0] == INQUIRY) &&
			    (us->fflags & US_FL_FIX_INQUIRY)) {
			unsigned char data_ptr[36] = {
			    0x00, 0x80, 0x02, 0x02,
			    0x1F, 0x00, 0x00, 0x00};

			usb_stor_dbg(us, "Faking INQUIRY command\n");
			fill_inquiry_response(us, data_ptr, 36);
			us->srb->result = SAM_STAT_GOOD;
		}
#if defined(CONFIG_USB_ETRON_HUB)
		else if (usb_is_etron_hcd(us->pusb_dev) && !usb_stor_no_test_unit_ready(us)) {
				usb_stor_dbg(us, "Ignoring TEST_UNIT_READY command\n");
				us->srb->result = SAM_STAT_GOOD;
		}
#endif  
		 
		else {
			US_DEBUG(usb_stor_show_command(us, us->srb));
			us->proto_handler(us->srb, us);
			usb_mark_last_busy(us->pusb_dev);
		}

		scsi_lock(host);

		if (us->srb->result != DID_ABORT << 16) {
			usb_stor_dbg(us, "scsi cmd done, result=0x%x\n",
				     us->srb->result);
			us->srb->scsi_done(us->srb);
		} else {
SkipForAbort:
			usb_stor_dbg(us, "scsi command aborted\n");
		}

		if (test_bit(US_FLIDX_TIMED_OUT, &us->dflags)) {
			complete(&(us->notify));

			clear_bit(US_FLIDX_ABORTING, &us->dflags);
			clear_bit(US_FLIDX_TIMED_OUT, &us->dflags);
		}

		us->srb = NULL;
		scsi_unlock(host);

		mutex_unlock(&us->dev_mutex);
	}  

	for (;;) {
		set_current_state(TASK_INTERRUPTIBLE);
		if (kthread_should_stop())
			break;
		schedule();
	}
	__set_current_state(TASK_RUNNING);
	return 0;
}

static int associate_dev(struct us_data *us, struct usb_interface *intf)
{
	 
	us->pusb_dev = interface_to_usbdev(intf);
	us->pusb_intf = intf;
	us->ifnum = intf->cur_altsetting->desc.bInterfaceNumber;
	usb_stor_dbg(us, "Vendor: 0x%04x, Product: 0x%04x, Revision: 0x%04x\n",
		     le16_to_cpu(us->pusb_dev->descriptor.idVendor),
		     le16_to_cpu(us->pusb_dev->descriptor.idProduct),
		     le16_to_cpu(us->pusb_dev->descriptor.bcdDevice));
	usb_stor_dbg(us, "Interface Subclass: 0x%02x, Protocol: 0x%02x\n",
		     intf->cur_altsetting->desc.bInterfaceSubClass,
		     intf->cur_altsetting->desc.bInterfaceProtocol);

	usb_set_intfdata(intf, us);

	us->cr = kmalloc(sizeof(*us->cr), GFP_KERNEL);
	if (!us->cr)
		return -ENOMEM;

	us->iobuf = usb_alloc_coherent(us->pusb_dev, US_IOBUF_SIZE,
			GFP_KERNEL, &us->iobuf_dma);
	if (!us->iobuf) {
		usb_stor_dbg(us, "I/O buffer allocation failed\n");
		return -ENOMEM;
	}
	return 0;
}

#define TOLOWER(x) ((x) | 0x20)

void usb_stor_adjust_quirks(struct usb_device *udev, unsigned long *fflags)
{
	char *p;
	u16 vid = le16_to_cpu(udev->descriptor.idVendor);
	u16 pid = le16_to_cpu(udev->descriptor.idProduct);
	unsigned f = 0;
	unsigned int mask = (US_FL_SANE_SENSE | US_FL_BAD_SENSE |
			US_FL_FIX_CAPACITY | US_FL_IGNORE_UAS |
			US_FL_CAPACITY_HEURISTICS | US_FL_IGNORE_DEVICE |
			US_FL_NOT_LOCKABLE | US_FL_MAX_SECTORS_64 |
			US_FL_CAPACITY_OK | US_FL_IGNORE_RESIDUE |
			US_FL_SINGLE_LUN | US_FL_NO_WP_DETECT |
			US_FL_NO_READ_DISC_INFO | US_FL_NO_READ_CAPACITY_16 |
			US_FL_INITIAL_READ10 | US_FL_WRITE_CACHE |
			US_FL_NO_ATA_1X | US_FL_NO_REPORT_OPCODES |
			US_FL_MAX_SECTORS_240 | US_FL_NO_REPORT_LUNS);

	p = quirks;
	while (*p) {
		 
		if (vid == simple_strtoul(p, &p, 16) &&
				*p == ':' &&
				pid == simple_strtoul(p+1, &p, 16) &&
				*p == ':')
			break;

		while (*p) {
			if (*p++ == ',')
				break;
		}
	}
	if (!*p)	 
		return;

	while (*++p && *p != ',') {
		switch (TOLOWER(*p)) {
		case 'a':
			f |= US_FL_SANE_SENSE;
			break;
		case 'b':
			f |= US_FL_BAD_SENSE;
			break;
		case 'c':
			f |= US_FL_FIX_CAPACITY;
			break;
		case 'd':
			f |= US_FL_NO_READ_DISC_INFO;
			break;
		case 'e':
			f |= US_FL_NO_READ_CAPACITY_16;
			break;
		case 'f':
			f |= US_FL_NO_REPORT_OPCODES;
			break;
		case 'g':
			f |= US_FL_MAX_SECTORS_240;
			break;
		case 'h':
			f |= US_FL_CAPACITY_HEURISTICS;
			break;
		case 'i':
			f |= US_FL_IGNORE_DEVICE;
			break;
		case 'j':
			f |= US_FL_NO_REPORT_LUNS;
			break;
		case 'l':
			f |= US_FL_NOT_LOCKABLE;
			break;
		case 'm':
			f |= US_FL_MAX_SECTORS_64;
			break;
		case 'n':
			f |= US_FL_INITIAL_READ10;
			break;
		case 'o':
			f |= US_FL_CAPACITY_OK;
			break;
		case 'p':
			f |= US_FL_WRITE_CACHE;
			break;
		case 'r':
			f |= US_FL_IGNORE_RESIDUE;
			break;
		case 's':
			f |= US_FL_SINGLE_LUN;
			break;
		case 't':
			f |= US_FL_NO_ATA_1X;
			break;
		case 'u':
			f |= US_FL_IGNORE_UAS;
			break;
		case 'w':
			f |= US_FL_NO_WP_DETECT;
			break;
		 
		}
	}
	*fflags = (*fflags & ~mask) | f;
}
EXPORT_SYMBOL_GPL(usb_stor_adjust_quirks);

static int get_device_info(struct us_data *us, const struct usb_device_id *id,
		struct us_unusual_dev *unusual_dev)
{
	struct usb_device *dev = us->pusb_dev;
	struct usb_interface_descriptor *idesc =
		&us->pusb_intf->cur_altsetting->desc;
	struct device *pdev = &us->pusb_intf->dev;

	us->unusual_dev = unusual_dev;
	us->subclass = (unusual_dev->useProtocol == USB_SC_DEVICE) ?
			idesc->bInterfaceSubClass :
			unusual_dev->useProtocol;
	us->protocol = (unusual_dev->useTransport == USB_PR_DEVICE) ?
			idesc->bInterfaceProtocol :
			unusual_dev->useTransport;
	us->fflags = id->driver_info;
	usb_stor_adjust_quirks(us->pusb_dev, &us->fflags);

	if (us->fflags & US_FL_IGNORE_DEVICE) {
		dev_info(pdev, "device ignored\n");
		return -ENODEV;
	}

	if (dev->speed != USB_SPEED_HIGH)
		us->fflags &= ~US_FL_GO_SLOW;

	if (us->fflags)
		dev_info(pdev, "Quirks match for vid %04x pid %04x: %lx\n",
				le16_to_cpu(dev->descriptor.idVendor),
				le16_to_cpu(dev->descriptor.idProduct),
				us->fflags);

	if (id->idVendor || id->idProduct) {
		static const char *msgs[3] = {
			"an unneeded SubClass entry",
			"an unneeded Protocol entry",
			"unneeded SubClass and Protocol entries"};
		struct usb_device_descriptor *ddesc = &dev->descriptor;
		int msg = -1;

		if (unusual_dev->useProtocol != USB_SC_DEVICE &&
			us->subclass == idesc->bInterfaceSubClass)
			msg += 1;
		if (unusual_dev->useTransport != USB_PR_DEVICE &&
			us->protocol == idesc->bInterfaceProtocol)
			msg += 2;
		if (msg >= 0 && !(us->fflags & US_FL_NEED_OVERRIDE))
			dev_notice(pdev, "This device "
					"(%04x,%04x,%04x S %02x P %02x)"
					" has %s in unusual_devs.h (kernel"
					" %s)\n"
					"   Please send a copy of this message to "
					"<linux-usb@vger.kernel.org> and "
					"<usb-storage@lists.one-eyed-alien.net>\n",
					le16_to_cpu(ddesc->idVendor),
					le16_to_cpu(ddesc->idProduct),
					le16_to_cpu(ddesc->bcdDevice),
					idesc->bInterfaceSubClass,
					idesc->bInterfaceProtocol,
					msgs[msg],
					utsname()->release);
	}

	return 0;
}

static void get_transport(struct us_data *us)
{
	switch (us->protocol) {
	case USB_PR_CB:
		us->transport_name = "Control/Bulk";
		us->transport = usb_stor_CB_transport;
		us->transport_reset = usb_stor_CB_reset;
		us->max_lun = 7;
		break;

	case USB_PR_CBI:
		us->transport_name = "Control/Bulk/Interrupt";
		us->transport = usb_stor_CB_transport;
		us->transport_reset = usb_stor_CB_reset;
		us->max_lun = 7;
		break;

	case USB_PR_BULK:
		us->transport_name = "Bulk";
		us->transport = usb_stor_Bulk_transport;
		us->transport_reset = usb_stor_Bulk_reset;
		break;
	}
}

static void get_protocol(struct us_data *us)
{
	switch (us->subclass) {
	case USB_SC_RBC:
		us->protocol_name = "Reduced Block Commands (RBC)";
		us->proto_handler = usb_stor_transparent_scsi_command;
		break;

	case USB_SC_8020:
		us->protocol_name = "8020i";
		us->proto_handler = usb_stor_pad12_command;
		us->max_lun = 0;
		break;

	case USB_SC_QIC:
		us->protocol_name = "QIC-157";
		us->proto_handler = usb_stor_pad12_command;
		us->max_lun = 0;
		break;

	case USB_SC_8070:
		us->protocol_name = "8070i";
		us->proto_handler = usb_stor_pad12_command;
		us->max_lun = 0;
		break;

	case USB_SC_SCSI:
		us->protocol_name = "Transparent SCSI";
		us->proto_handler = usb_stor_transparent_scsi_command;
		break;

	case USB_SC_UFI:
		us->protocol_name = "Uniform Floppy Interface (UFI)";
		us->proto_handler = usb_stor_ufi_command;
		break;
	}
}

static int get_pipes(struct us_data *us)
{
	struct usb_host_interface *altsetting =
		us->pusb_intf->cur_altsetting;
	int i;
	struct usb_endpoint_descriptor *ep;
	struct usb_endpoint_descriptor *ep_in = NULL;
	struct usb_endpoint_descriptor *ep_out = NULL;
	struct usb_endpoint_descriptor *ep_int = NULL;

	for (i = 0; i < altsetting->desc.bNumEndpoints; i++) {
		ep = &altsetting->endpoint[i].desc;

		if (usb_endpoint_xfer_bulk(ep)) {
			if (usb_endpoint_dir_in(ep)) {
				if (!ep_in)
					ep_in = ep;
			} else {
				if (!ep_out)
					ep_out = ep;
			}
		}

		else if (usb_endpoint_is_int_in(ep)) {
			if (!ep_int)
				ep_int = ep;
		}
	}

	if (!ep_in || !ep_out || (us->protocol == USB_PR_CBI && !ep_int)) {
		usb_stor_dbg(us, "Endpoint sanity check failed! Rejecting dev.\n");
		return -EIO;
	}

	us->send_ctrl_pipe = usb_sndctrlpipe(us->pusb_dev, 0);
	us->recv_ctrl_pipe = usb_rcvctrlpipe(us->pusb_dev, 0);
	us->send_bulk_pipe = usb_sndbulkpipe(us->pusb_dev,
		usb_endpoint_num(ep_out));
	us->recv_bulk_pipe = usb_rcvbulkpipe(us->pusb_dev,
		usb_endpoint_num(ep_in));
	if (ep_int) {
		us->recv_intr_pipe = usb_rcvintpipe(us->pusb_dev,
			usb_endpoint_num(ep_int));
		us->ep_bInterval = ep_int->bInterval;
	}
	return 0;
}

static int usb_stor_acquire_resources(struct us_data *us)
{
	int p;
	struct task_struct *th;

	us->current_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!us->current_urb) {
		usb_stor_dbg(us, "URB allocation failed\n");
		return -ENOMEM;
	}

	if (us->unusual_dev->initFunction) {
		p = us->unusual_dev->initFunction(us);
		if (p)
			return p;
	}

	th = kthread_run(usb_stor_control_thread, us, "usb-storage");
	if (IS_ERR(th)) {
		dev_warn(&us->pusb_intf->dev,
				"Unable to start control thread\n");
		return PTR_ERR(th);
	}
	us->ctl_thread = th;

	return 0;
}

static void usb_stor_release_resources(struct us_data *us)
{
	 
	usb_stor_dbg(us, "-- sending exit command to thread\n");
	complete(&us->cmnd_ready);
	if (us->ctl_thread)
		kthread_stop(us->ctl_thread);

	if (us->extra_destructor) {
		usb_stor_dbg(us, "-- calling extra_destructor()\n");
		us->extra_destructor(us->extra);
	}

	kfree(us->extra);
	usb_free_urb(us->current_urb);
}

static void dissociate_dev(struct us_data *us)
{
	 
	kfree(us->cr);
	usb_free_coherent(us->pusb_dev, US_IOBUF_SIZE, us->iobuf, us->iobuf_dma);

	usb_set_intfdata(us->pusb_intf, NULL);
}

static void quiesce_and_remove_host(struct us_data *us)
{
	struct Scsi_Host *host = us_to_host(us);

	if (us->pusb_dev->state == USB_STATE_NOTATTACHED) {
		set_bit(US_FLIDX_DISCONNECTING, &us->dflags);
		wake_up(&us->delay_wait);
	}

	cancel_delayed_work_sync(&us->scan_dwork);

	if (test_bit(US_FLIDX_SCAN_PENDING, &us->dflags))
		usb_autopm_put_interface_no_suspend(us->pusb_intf);

#ifdef MY_ABC_HERE
	scsi_lock(host);
	usb_stor_stop_transport(us);
	scsi_unlock(host);
#endif  

	scsi_remove_host(host);

	scsi_lock(host);
	set_bit(US_FLIDX_DISCONNECTING, &us->dflags);
	scsi_unlock(host);
	wake_up(&us->delay_wait);
}

static void release_everything(struct us_data *us)
{
	usb_stor_release_resources(us);
	dissociate_dev(us);

	scsi_host_put(us_to_host(us));
}

static void usb_stor_scan_dwork(struct work_struct *work)
{
	struct us_data *us = container_of(work, struct us_data,
			scan_dwork.work);
	struct device *dev = &us->pusb_intf->dev;

	dev_dbg(dev, "starting scan\n");

	if (us->protocol == USB_PR_BULK &&
	    !(us->fflags & US_FL_SINGLE_LUN) &&
	    !(us->fflags & US_FL_SCM_MULT_TARG)) {
		mutex_lock(&us->dev_mutex);
		us->max_lun = usb_stor_Bulk_max_lun(us);
		 
		if (us->max_lun >= 8)
			us_to_host(us)->max_lun = us->max_lun+1;
		mutex_unlock(&us->dev_mutex);
	}
	scsi_scan_host(us_to_host(us));
	dev_dbg(dev, "scan complete\n");

	usb_autopm_put_interface(us->pusb_intf);
	clear_bit(US_FLIDX_SCAN_PENDING, &us->dflags);
}

static unsigned int usb_stor_sg_tablesize(struct usb_interface *intf)
{
	struct usb_device *usb_dev = interface_to_usbdev(intf);

	if (usb_dev->bus->sg_tablesize) {
		return usb_dev->bus->sg_tablesize;
	}
	return SG_ALL;
}

int usb_stor_probe1(struct us_data **pus,
		struct usb_interface *intf,
		const struct usb_device_id *id,
		struct us_unusual_dev *unusual_dev,
		struct scsi_host_template *sht)
{
	struct Scsi_Host *host;
	struct us_data *us;
	int result;

	dev_info(&intf->dev, "USB Mass Storage device detected\n");

	host = scsi_host_alloc(sht, sizeof(*us));
	if (!host) {
		dev_warn(&intf->dev, "Unable to allocate the scsi host\n");
		return -ENOMEM;
	}

	host->max_cmd_len = 16;
	host->sg_tablesize = usb_stor_sg_tablesize(intf);
	*pus = us = host_to_us(host);
	mutex_init(&(us->dev_mutex));
	us_set_lock_class(&us->dev_mutex, intf);
	init_completion(&us->cmnd_ready);
	init_completion(&(us->notify));
	init_waitqueue_head(&us->delay_wait);
	INIT_DELAYED_WORK(&us->scan_dwork, usb_stor_scan_dwork);

	result = associate_dev(us, intf);
	if (result)
		goto BadDevice;

	result = get_device_info(us, id, unusual_dev);
	if (result)
		goto BadDevice;

	get_transport(us);
	get_protocol(us);

	return 0;

BadDevice:
	usb_stor_dbg(us, "storage_probe() failed\n");
	release_everything(us);
	return result;
}
EXPORT_SYMBOL_GPL(usb_stor_probe1);

int usb_stor_probe2(struct us_data *us)
{
	int result;
	struct device *dev = &us->pusb_intf->dev;

	if (!us->transport || !us->proto_handler) {
		result = -ENXIO;
		goto BadDevice;
	}
	usb_stor_dbg(us, "Transport: %s\n", us->transport_name);
	usb_stor_dbg(us, "Protocol: %s\n", us->protocol_name);

	if (us->fflags & US_FL_SCM_MULT_TARG) {
		 
		us->max_lun = 7;
		 
		us_to_host(us)->this_id = 7;
		 
	} else {
		 
		us_to_host(us)->max_id = 1;
		 
		if (us->transport == usb_stor_Bulk_transport)
			us_to_host(us)->no_scsi2_lun_in_cdb = 1;
	}

	if (us->fflags & US_FL_SINGLE_LUN)
		us->max_lun = 0;

	result = get_pipes(us);
	if (result)
		goto BadDevice;

	if (us->fflags & US_FL_INITIAL_READ10)
		set_bit(US_FLIDX_REDO_READ10, &us->dflags);

	result = usb_stor_acquire_resources(us);
	if (result)
		goto BadDevice;
	snprintf(us->scsi_name, sizeof(us->scsi_name), "usb-storage %s",
					dev_name(&us->pusb_intf->dev));
	result = scsi_add_host(us_to_host(us), dev);
	if (result) {
		dev_warn(dev,
				"Unable to add the scsi host\n");
		goto BadDevice;
	}

	usb_autopm_get_interface_no_resume(us->pusb_intf);
	set_bit(US_FLIDX_SCAN_PENDING, &us->dflags);

	if (delay_use > 0)
		dev_dbg(dev, "waiting for device to settle before scanning\n");
	queue_delayed_work(system_freezable_wq, &us->scan_dwork,
			delay_use * HZ);
	return 0;

BadDevice:
	usb_stor_dbg(us, "storage_probe() failed\n");
	release_everything(us);
	return result;
}
EXPORT_SYMBOL_GPL(usb_stor_probe2);

void usb_stor_disconnect(struct usb_interface *intf)
{
	struct us_data *us = usb_get_intfdata(intf);

	quiesce_and_remove_host(us);
	release_everything(us);
}
EXPORT_SYMBOL_GPL(usb_stor_disconnect);

static struct scsi_host_template usb_stor_host_template;

#if IS_ENABLED(CONFIG_USB_ETRON_UAS)
static int is_uas_device(struct usb_interface *intf)
{
	struct usb_device *udev = interface_to_usbdev(intf);

#define USB_QUIRK_UAS_MODE		0x80000000

	return !!(udev->quirks & USB_QUIRK_UAS_MODE);
}
#endif  

static int storage_probe(struct usb_interface *intf,
			 const struct usb_device_id *id)
{
	struct us_unusual_dev *unusual_dev;
	struct us_data *us;
	int result;
	int size;

#if IS_ENABLED(CONFIG_USB_ETRON_UAS)
	if (is_uas_device(intf))
		return -ENODEV;
#endif  

#if IS_ENABLED(CONFIG_USB_UAS)
	if (uas_use_uas_driver(intf, id, NULL))
		return -ENXIO;
#endif

	if (usb_usual_ignore_device(intf))
		return -ENXIO;

	size = ARRAY_SIZE(us_unusual_dev_list);
	if (id >= usb_storage_usb_ids && id < usb_storage_usb_ids + size) {
		unusual_dev = (id - usb_storage_usb_ids) + us_unusual_dev_list;
	} else {
		unusual_dev = &for_dynamic_ids;

		dev_dbg(&intf->dev, "Use Bulk-Only transport with the Transparent SCSI protocol for dynamic id: 0x%04x 0x%04x\n",
			id->idVendor, id->idProduct);
	}

	result = usb_stor_probe1(&us, intf, id, unusual_dev,
				 &usb_stor_host_template);
	if (result)
		return result;

	result = usb_stor_probe2(us);
	return result;
}

static struct usb_driver usb_storage_driver = {
	.name =		DRV_NAME,
	.probe =	storage_probe,
	.disconnect =	usb_stor_disconnect,
	.suspend =	usb_stor_suspend,
	.resume =	usb_stor_resume,
	.reset_resume =	usb_stor_reset_resume,
	.pre_reset =	usb_stor_pre_reset,
	.post_reset =	usb_stor_post_reset,
	.id_table =	usb_storage_usb_ids,
	.supports_autosuspend = 1,
	.soft_unbind =	1,
};

module_usb_stor_driver(usb_storage_driver, usb_stor_host_template, DRV_NAME);
