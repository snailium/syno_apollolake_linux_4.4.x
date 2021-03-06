#ifndef MY_ABC_HERE
#define MY_ABC_HERE
#endif
 
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/gfp.h>
#include <linux/poll.h>
#include <linux/usb.h>
#include <linux/usbdevice_fs.h>
#include <linux/usb/hcd.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#if defined(MY_DEF_HERE) || defined(CONFIG_SYNO_RTD1619)
#include <linux/synobios.h>
#endif  

#include "usb.h"

#define ALLOW_SERIAL_NUMBER

static const char format_topo[] =
 
"\nT:  Bus=%2.2d Lev=%2.2d Prnt=%2.2d Port=%2.2d Cnt=%2.2d Dev#=%3d Spd=%-4s MxCh=%2d\n";

static const char format_string_manufacturer[] =
 
  "S:  Manufacturer=%.100s\n";

static const char format_string_product[] =
 
  "S:  Product=%.100s\n";

#ifdef ALLOW_SERIAL_NUMBER
static const char format_string_serialnumber[] =
 
  "S:  SerialNumber=%.100s\n";
#endif

static const char format_bandwidth[] =
 
  "B:  Alloc=%3d/%3d us (%2d%%), #Int=%3d, #Iso=%3d\n";

static const char format_device1[] =
 
  "D:  Ver=%2x.%02x Cls=%02x(%-5s) Sub=%02x Prot=%02x MxPS=%2d #Cfgs=%3d\n";

static const char format_device2[] =
 
  "P:  Vendor=%04x ProdID=%04x Rev=%2x.%02x\n";

static const char format_config[] =
 
  "C:%c #Ifs=%2d Cfg#=%2d Atr=%02x MxPwr=%3dmA\n";

static const char format_iad[] =
 
  "A:  FirstIf#=%2d IfCount=%2d Cls=%02x(%-5s) Sub=%02x Prot=%02x\n";

static const char format_iface[] =
 
  "I:%c If#=%2d Alt=%2d #EPs=%2d Cls=%02x(%-5s) Sub=%02x Prot=%02x Driver=%s\n";

static const char format_endpt[] =
 
  "E:  Ad=%02x(%c) Atr=%02x(%-4s) MxPS=%4d Ivl=%d%cs\n";

static struct device_connect_event {
	atomic_t count;
	wait_queue_head_t wait;
} device_event = {
	.count = ATOMIC_INIT(1),
	.wait = __WAIT_QUEUE_HEAD_INITIALIZER(device_event.wait)
};

struct class_info {
	int class;
	char *class_name;
};

static const struct class_info clas_info[] = {
	 
	{USB_CLASS_PER_INTERFACE,	">ifc"},
	{USB_CLASS_AUDIO,		"audio"},
	{USB_CLASS_COMM,		"comm."},
	{USB_CLASS_HID,			"HID"},
	{USB_CLASS_PHYSICAL,		"PID"},
	{USB_CLASS_STILL_IMAGE,		"still"},
	{USB_CLASS_PRINTER,		"print"},
	{USB_CLASS_MASS_STORAGE,	"stor."},
	{USB_CLASS_HUB,			"hub"},
	{USB_CLASS_CDC_DATA,		"data"},
	{USB_CLASS_CSCID,		"scard"},
	{USB_CLASS_CONTENT_SEC,		"c-sec"},
	{USB_CLASS_VIDEO,		"video"},
	{USB_CLASS_WIRELESS_CONTROLLER,	"wlcon"},
	{USB_CLASS_MISC,		"misc"},
	{USB_CLASS_APP_SPEC,		"app."},
	{USB_CLASS_VENDOR_SPEC,		"vend."},
	{-1,				"unk."}		 
};

#ifdef MY_ABC_HERE
#define SDCOPY_PORT_LOCATION 98
#endif  

#ifdef MY_ABC_HERE
#define USBCOPY_PORT_LOCATION 99
#endif  

void usbfs_conn_disc_event(void)
{
	atomic_add(2, &device_event.count);
	wake_up(&device_event.wait);
}

static const char *class_decode(const int class)
{
	int ix;

	for (ix = 0; clas_info[ix].class != -1; ix++)
		if (clas_info[ix].class == class)
			break;
	return clas_info[ix].class_name;
}

static char *usb_dump_endpoint_descriptor(int speed, char *start, char *end,
				const struct usb_endpoint_descriptor *desc)
{
	char dir, unit, *type;
	unsigned interval, bandwidth = 1;

	if (start > end)
		return start;

	dir = usb_endpoint_dir_in(desc) ? 'I' : 'O';

	if (speed == USB_SPEED_HIGH) {
		switch (usb_endpoint_maxp(desc) & (0x03 << 11)) {
		case 1 << 11:
			bandwidth = 2; break;
		case 2 << 11:
			bandwidth = 3; break;
		}
	}

	switch (usb_endpoint_type(desc)) {
	case USB_ENDPOINT_XFER_CONTROL:
		type = "Ctrl";
		if (speed == USB_SPEED_HIGH)	 
			interval = desc->bInterval;
		else
			interval = 0;
		dir = 'B';			 
		break;
	case USB_ENDPOINT_XFER_ISOC:
		type = "Isoc";
		interval = 1 << (desc->bInterval - 1);
		break;
	case USB_ENDPOINT_XFER_BULK:
		type = "Bulk";
		if (speed == USB_SPEED_HIGH && dir == 'O')  
			interval = desc->bInterval;
		else
			interval = 0;
		break;
	case USB_ENDPOINT_XFER_INT:
		type = "Int.";
		if (speed == USB_SPEED_HIGH || speed >= USB_SPEED_SUPER)
			interval = 1 << (desc->bInterval - 1);
		else
			interval = desc->bInterval;
		break;
	default:	 
		return start;
	}
	interval *= (speed == USB_SPEED_HIGH ||
		     speed >= USB_SPEED_SUPER) ? 125 : 1000;
	if (interval % 1000)
		unit = 'u';
	else {
		unit = 'm';
		interval /= 1000;
	}

	start += sprintf(start, format_endpt, desc->bEndpointAddress, dir,
			 desc->bmAttributes, type,
			 (usb_endpoint_maxp(desc) & 0x07ff) *
			 bandwidth,
			 interval, unit);
	return start;
}

static char *usb_dump_interface_descriptor(char *start, char *end,
					const struct usb_interface_cache *intfc,
					const struct usb_interface *iface,
					int setno)
{
	const struct usb_interface_descriptor *desc;
	const char *driver_name = "";
	int active = 0;

	if (start > end)
		return start;
	desc = &intfc->altsetting[setno].desc;
	if (iface) {
		driver_name = (iface->dev.driver
				? iface->dev.driver->name
				: "(none)");
		active = (desc == &iface->cur_altsetting->desc);
	}
	start += sprintf(start, format_iface,
			 active ? '*' : ' ',	 
			 desc->bInterfaceNumber,
			 desc->bAlternateSetting,
			 desc->bNumEndpoints,
			 desc->bInterfaceClass,
			 class_decode(desc->bInterfaceClass),
			 desc->bInterfaceSubClass,
			 desc->bInterfaceProtocol,
			 driver_name);
	return start;
}

static char *usb_dump_interface(int speed, char *start, char *end,
				const struct usb_interface_cache *intfc,
				const struct usb_interface *iface, int setno)
{
	const struct usb_host_interface *desc = &intfc->altsetting[setno];
	int i;

	start = usb_dump_interface_descriptor(start, end, intfc, iface, setno);
	for (i = 0; i < desc->desc.bNumEndpoints; i++) {
		if (start > end)
			return start;
		start = usb_dump_endpoint_descriptor(speed,
				start, end, &desc->endpoint[i].desc);
	}
	return start;
}

static char *usb_dump_iad_descriptor(char *start, char *end,
			const struct usb_interface_assoc_descriptor *iad)
{
	if (start > end)
		return start;
	start += sprintf(start, format_iad,
			 iad->bFirstInterface,
			 iad->bInterfaceCount,
			 iad->bFunctionClass,
			 class_decode(iad->bFunctionClass),
			 iad->bFunctionSubClass,
			 iad->bFunctionProtocol);
	return start;
}

static char *usb_dump_config_descriptor(char *start, char *end,
				const struct usb_config_descriptor *desc,
				int active, int speed)
{
	int mul;

	if (start > end)
		return start;
	if (speed >= USB_SPEED_SUPER)
		mul = 8;
	else
		mul = 2;
	start += sprintf(start, format_config,
			  
			 active ? '*' : ' ',
			 desc->bNumInterfaces,
			 desc->bConfigurationValue,
			 desc->bmAttributes,
			 desc->bMaxPower * mul);
	return start;
}

static char *usb_dump_config(int speed, char *start, char *end,
			     const struct usb_host_config *config, int active)
{
	int i, j;
	struct usb_interface_cache *intfc;
	struct usb_interface *interface;

	if (start > end)
		return start;
	if (!config)
		 
		return start + sprintf(start, "(null Cfg. desc.)\n");
	start = usb_dump_config_descriptor(start, end, &config->desc, active,
			speed);
	for (i = 0; i < USB_MAXIADS; i++) {
		if (config->intf_assoc[i] == NULL)
			break;
		start = usb_dump_iad_descriptor(start, end,
					config->intf_assoc[i]);
	}
	for (i = 0; i < config->desc.bNumInterfaces; i++) {
		intfc = config->intf_cache[i];
		interface = config->interface[i];
		for (j = 0; j < intfc->num_altsetting; j++) {
			if (start > end)
				return start;
			start = usb_dump_interface(speed,
				start, end, intfc, interface, j);
		}
	}
	return start;
}

static char *usb_dump_device_descriptor(char *start, char *end,
				const struct usb_device_descriptor *desc)
{
	u16 bcdUSB = le16_to_cpu(desc->bcdUSB);
	u16 bcdDevice = le16_to_cpu(desc->bcdDevice);

	if (start > end)
		return start;
	start += sprintf(start, format_device1,
			  bcdUSB >> 8, bcdUSB & 0xff,
			  desc->bDeviceClass,
			  class_decode(desc->bDeviceClass),
			  desc->bDeviceSubClass,
			  desc->bDeviceProtocol,
			  desc->bMaxPacketSize0,
			  desc->bNumConfigurations);
	if (start > end)
		return start;
	start += sprintf(start, format_device2,
			 le16_to_cpu(desc->idVendor),
			 le16_to_cpu(desc->idProduct),
			 bcdDevice >> 8, bcdDevice & 0xff);
	return start;
}

static char *usb_dump_device_strings(char *start, char *end,
				     struct usb_device *dev)
{
	if (start > end)
		return start;
	if (dev->manufacturer)
		start += sprintf(start, format_string_manufacturer,
				 dev->manufacturer);
	if (start > end)
		goto out;
	if (dev->product)
		start += sprintf(start, format_string_product, dev->product);
	if (start > end)
		goto out;
#ifdef ALLOW_SERIAL_NUMBER
	if (dev->serial)
		start += sprintf(start, format_string_serialnumber,
				 dev->serial);
#endif
 out:
	return start;
}

static char *usb_dump_desc(char *start, char *end, struct usb_device *dev)
{
	int i;

	if (start > end)
		return start;

	start = usb_dump_device_descriptor(start, end, &dev->descriptor);

	if (start > end)
		return start;

	start = usb_dump_device_strings(start, end, dev);

	for (i = 0; i < dev->descriptor.bNumConfigurations; i++) {
		if (start > end)
			return start;
		start = usb_dump_config(dev->speed,
				start, end, dev->config + i,
				 
				(dev->config + i) == dev->actconfig);
	}
	return start;
}

#ifdef PROC_EXTRA  

static char *usb_dump_hub_descriptor(char *start, char *end,
				     const struct usb_hub_descriptor *desc)
{
	int leng = USB_DT_HUB_NONVAR_SIZE;
	unsigned char *ptr = (unsigned char *)desc;

	if (start > end)
		return start;
	start += sprintf(start, "Interface:");
	while (leng && start <= end) {
		start += sprintf(start, " %02x", *ptr);
		ptr++; leng--;
	}
	*start++ = '\n';
	return start;
}

static char *usb_dump_string(char *start, char *end,
			     const struct usb_device *dev, char *id, int index)
{
	if (start > end)
		return start;
	start += sprintf(start, "Interface:");
	if (index <= dev->maxstring && dev->stringindex &&
	    dev->stringindex[index])
		start += sprintf(start, "%s: %.100s ", id,
				 dev->stringindex[index]);
	return start;
}

#endif  

#ifdef MY_ABC_HERE
int blIsUSBDeviceAtFrontPort(struct usb_device *usbdev)
{
	char buf[256];

	if(usbdev && usbdev->bus) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%s-%s", usbdev->bus->bus_name, usbdev->devpath);
#if defined(CONFIG_SYNO_BRASWELL)
		if(!strcmp(buf,"0000:00:14.0-2")) {
			return 1;
		}
#endif  
#if defined(MY_ABC_HERE)
		if(!strcmp(buf,"0000:00:15.0-3")) {
			return 1;
		}
#endif  
#if defined(CONFIG_SYNO_GEMINILAKE)
		if(!strcmp(buf,"0000:00:15.0-1")) {
			return 1;
		}
#endif  
#if defined(CONFIG_SYNO_X86)
#if defined(CONFIG_ARCH_GEN3)
		if(!strcmp(buf,"0000:01:0d.0-1")) {
			return 1;
		}
#endif  
#endif  
#if defined(MY_ABC_HERE)
#if defined(CONFIG_SYNO_CEDARVIEW)
		if(!strcmp(buf,"0000:00:1d.7-2")) {
			return 1;
		}
#else
		if(!strcmp(buf,"0000:00:1d.7-3") || !strcmp(buf,"0000:00:1d.1-1")) {
			return 1;
		}
#endif  
#endif  
#if defined(CONFIG_SYNO_ARMADA) && defined(CONFIG_ARMADA_XP)
		if(!strcmp(buf, "ehci_marvell.1-1") ||
		   !strcmp(buf, "ehci_marvell.0-1")) {
			return 1;
		}
#endif  
#if defined(MY_DEF_HERE)
		if(syno_is_hw_version(HW_DS218)) {
			if(!strcmp(buf, "xhci-hcd.5.auto-1")) {
				return 1;
			}
		}
#endif  
#if defined(CONFIG_SYNO_RTD1619)
		if(syno_is_hw_version(HW_DS220)) {
			if(!strcmp(buf, "xhci-hcd.7.auto-1")) {
				return 1;
			}
		}
#endif  
	}
	return 0;
}
#endif  

#ifdef MY_ABC_HERE
int blIsCardReader(struct usb_device *usbdev)
{
	char buf[256];

	if(usbdev && usbdev->bus) {
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%s-%s", usbdev->bus->bus_name, usbdev->devpath);

#if defined(CONFIG_SYNO_X86)
#if defined(CONFIG_ARCH_GEN3)
		if (syno_is_hw_version(HW_DS214play)) {
			if(!strcmp(buf,"0000:01:0d.1-1")) {
			return 1;
			}
		}
#endif  
#endif  

#if defined(CONFIG_SYNO_ARMADA)
		if (syno_is_hw_version(HW_US3v10)) {
			if (!strcmp(buf, "0000:00:00.0-1")) {
				return 1;
			}
		}
#endif  

	}
	return 0;
}
EXPORT_SYMBOL(blIsCardReader);
#endif  

static ssize_t usb_device_dump(char __user **buffer, size_t *nbytes,
			       loff_t *skip_bytes, loff_t *file_offset,
			       struct usb_device *usbdev, struct usb_bus *bus,
			       int level, int index, int count)
{
	int chix;
	int ret, cnt = 0;
	int parent_devnum = 0;
	char *pages_start, *data_end, *speed;
	unsigned int length;
	ssize_t total_written = 0;
	struct usb_device *childdev = NULL;
#if defined(MY_ABC_HERE) || defined(MY_ABC_HERE)
	int port = 0;
#endif  

	if (*nbytes <= 0)
		return 0;

	if (level > MAX_TOPO_LEVEL)
		return 0;
	 
	pages_start = (char *)__get_free_pages(GFP_NOIO, 1);
	if (!pages_start)
		return -ENOMEM;

	if (usbdev->parent && usbdev->parent->devnum != -1)
		parent_devnum = usbdev->parent->devnum;
	 
	switch (usbdev->speed) {
	case USB_SPEED_LOW:
		speed = "1.5"; break;
	case USB_SPEED_UNKNOWN:		 
	case USB_SPEED_FULL:
		speed = "12"; break;
	case USB_SPEED_WIRELESS:	 
	case USB_SPEED_HIGH:
		speed = "480"; break;
	case USB_SPEED_SUPER:
		speed = "5000"; break;
	case USB_SPEED_SUPER_PLUS:
		speed = "10000"; break;
	default:
		speed = "??";
	}
#if defined(MY_ABC_HERE) || defined(MY_ABC_HERE)
#if defined(MY_ABC_HERE)
	if(blIsUSBDeviceAtFrontPort(usbdev)) {
		port = USBCOPY_PORT_LOCATION;
	}
#endif  
#if defined(MY_ABC_HERE)
	if(blIsCardReader(usbdev)) {
		port = SDCOPY_PORT_LOCATION;
	}
#endif  

	if (port) {
		data_end = pages_start + sprintf(pages_start, format_topo,
				bus->busnum, level, parent_devnum,
				port, count, usbdev->devnum,
				speed, usbdev->maxchild);
	} else {
#endif  
	data_end = pages_start + sprintf(pages_start, format_topo,
			bus->busnum, level, parent_devnum,
			index, count, usbdev->devnum,
			speed, usbdev->maxchild);
#if defined(MY_ABC_HERE) || defined(MY_ABC_HERE)
	}
#endif  

	if (level == 0) {
		int	max;

		if (usbdev->speed == USB_SPEED_HIGH ||
		    usbdev->speed >= USB_SPEED_SUPER)
			max = 800;
		else
			max = FRAME_TIME_MAX_USECS_ALLOC;

		data_end += sprintf(data_end, format_bandwidth,
				bus->bandwidth_allocated, max,
				(100 * bus->bandwidth_allocated + max / 2)
					/ max,
				bus->bandwidth_int_reqs,
				bus->bandwidth_isoc_reqs);

	}
	data_end = usb_dump_desc(data_end, pages_start + (2 * PAGE_SIZE) - 256,
				 usbdev);

	if (data_end > (pages_start + (2 * PAGE_SIZE) - 256))
		data_end += sprintf(data_end, "(truncated)\n");

	length = data_end - pages_start;
	 
	if (length > *skip_bytes) {
		length -= *skip_bytes;
		if (length > *nbytes)
			length = *nbytes;
		if (copy_to_user(*buffer, pages_start + *skip_bytes, length)) {
			free_pages((unsigned long)pages_start, 1);
			return -EFAULT;
		}
		*nbytes -= length;
		*file_offset += length;
		total_written += length;
		*buffer += length;
		*skip_bytes = 0;
	} else
		*skip_bytes -= length;

	free_pages((unsigned long)pages_start, 1);

	usb_hub_for_each_child(usbdev, chix, childdev) {
		usb_lock_device(childdev);
#if defined(MY_ABC_HERE) || defined(MY_ABC_HERE)
		if (port) {
			ret = usb_device_dump(buffer, nbytes, skip_bytes, file_offset,
					childdev, bus, level + 1, port, ++cnt);
		} else {
#endif  
		ret = usb_device_dump(buffer, nbytes, skip_bytes,
				      file_offset, childdev, bus,
				      level + 1, chix - 1, ++cnt);
#if defined(MY_ABC_HERE) || defined(MY_ABC_HERE)
		}
#endif  

		usb_unlock_device(childdev);
		if (ret == -EFAULT)
			return total_written;
		total_written += ret;
	}
	return total_written;
}

static ssize_t usb_device_read(struct file *file, char __user *buf,
			       size_t nbytes, loff_t *ppos)
{
	struct usb_bus *bus;
	ssize_t ret, total_written = 0;
	loff_t skip_bytes = *ppos;

	if (*ppos < 0)
		return -EINVAL;
	if (nbytes <= 0)
		return 0;
	if (!access_ok(VERIFY_WRITE, buf, nbytes))
		return -EFAULT;

	mutex_lock(&usb_bus_list_lock);
	 
	list_for_each_entry(bus, &usb_bus_list, bus_list) {
		 
		if (!bus_to_hcd(bus)->rh_registered)
			continue;
		usb_lock_device(bus->root_hub);
		ret = usb_device_dump(&buf, &nbytes, &skip_bytes, ppos,
				      bus->root_hub, bus, 0, 0, 0);
		usb_unlock_device(bus->root_hub);
		if (ret < 0) {
			mutex_unlock(&usb_bus_list_lock);
			return ret;
		}
		total_written += ret;
	}
	mutex_unlock(&usb_bus_list_lock);
	return total_written;
}

static unsigned int usb_device_poll(struct file *file,
				    struct poll_table_struct *wait)
{
	unsigned int event_count;

	poll_wait(file, &device_event.wait, wait);

	event_count = atomic_read(&device_event.count);
	if (file->f_version != event_count) {
		file->f_version = event_count;
		return POLLIN | POLLRDNORM;
	}

	return 0;
}

static loff_t usb_device_lseek(struct file *file, loff_t offset, int orig)
{
	loff_t ret;

	mutex_lock(&file_inode(file)->i_mutex);

	switch (orig) {
	case 0:
		file->f_pos = offset;
		ret = file->f_pos;
		break;
	case 1:
		file->f_pos += offset;
		ret = file->f_pos;
		break;
	case 2:
	default:
		ret = -EINVAL;
	}

	mutex_unlock(&file_inode(file)->i_mutex);
	return ret;
}

const struct file_operations usbfs_devices_fops = {
	.llseek =	usb_device_lseek,
	.read =		usb_device_read,
	.poll =		usb_device_poll,
};
