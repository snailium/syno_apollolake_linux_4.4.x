#ifndef MY_ABC_HERE
#define MY_ABC_HERE
#endif
 
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/bio.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/errno.h>
#include <linux/idr.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/string_helpers.h>
#include <linux/async.h>
#include <linux/slab.h>
#if defined(MY_DEF_HERE) || defined(CONFIG_SYNO_PORT_MAPPING_V2)
#include <linux/pci.h>
#endif  
#include <linux/pm_runtime.h>
#include <linux/pr.h>
#include <asm/uaccess.h>
#include <asm/unaligned.h>

#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_dbg.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_driver.h>
#include <scsi/scsi_eh.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_ioctl.h>
#include <scsi/scsicam.h>

#include "sd.h"
#include "scsi_priv.h"
#include "scsi_logging.h"

#if defined(MY_ABC_HERE) || defined(CONFIG_SYNO_PORT_MAPPING_V2)
#include <linux/libata.h>
#include <linux/usb.h>
#include "../usb/storage/usb.h"
#endif  
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
#include "../ata/ahci.h"
#include <linux/synobios.h>
#endif  
#if defined(MY_DEF_HERE) || defined(CONFIG_SYNO_PORT_MAPPING_V2)
#include <linux/synolib.h>
#endif  
#ifdef MY_DEF_HERE
static DEFINE_IDA(cache_index_ida);
#endif  

MODULE_AUTHOR("Eric Youngdale");
MODULE_DESCRIPTION("SCSI disk (sd) driver");
MODULE_LICENSE("GPL");

MODULE_ALIAS_BLOCKDEV_MAJOR(SCSI_DISK0_MAJOR);
MODULE_ALIAS_BLOCKDEV_MAJOR(SCSI_DISK1_MAJOR);
MODULE_ALIAS_BLOCKDEV_MAJOR(SCSI_DISK2_MAJOR);
MODULE_ALIAS_BLOCKDEV_MAJOR(SCSI_DISK3_MAJOR);
MODULE_ALIAS_BLOCKDEV_MAJOR(SCSI_DISK4_MAJOR);
MODULE_ALIAS_BLOCKDEV_MAJOR(SCSI_DISK5_MAJOR);
MODULE_ALIAS_BLOCKDEV_MAJOR(SCSI_DISK6_MAJOR);
MODULE_ALIAS_BLOCKDEV_MAJOR(SCSI_DISK7_MAJOR);
MODULE_ALIAS_BLOCKDEV_MAJOR(SCSI_DISK8_MAJOR);
MODULE_ALIAS_BLOCKDEV_MAJOR(SCSI_DISK9_MAJOR);
MODULE_ALIAS_BLOCKDEV_MAJOR(SCSI_DISK10_MAJOR);
MODULE_ALIAS_BLOCKDEV_MAJOR(SCSI_DISK11_MAJOR);
MODULE_ALIAS_BLOCKDEV_MAJOR(SCSI_DISK12_MAJOR);
MODULE_ALIAS_BLOCKDEV_MAJOR(SCSI_DISK13_MAJOR);
MODULE_ALIAS_BLOCKDEV_MAJOR(SCSI_DISK14_MAJOR);
MODULE_ALIAS_BLOCKDEV_MAJOR(SCSI_DISK15_MAJOR);
MODULE_ALIAS_SCSI_DEVICE(TYPE_DISK);
MODULE_ALIAS_SCSI_DEVICE(TYPE_MOD);
MODULE_ALIAS_SCSI_DEVICE(TYPE_RBC);

#if !defined(CONFIG_DEBUG_BLOCK_EXT_DEVT)
#define SD_MINORS	16
#else
#define SD_MINORS	0
#endif

#ifdef MY_ABC_HERE
extern int gSynoHasDynModule;
#endif  

#ifdef MY_DEF_HERE
extern int gSynoBootSATADOM;
#endif  

#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY

struct SpinupQueue {
	spinlock_t q_lock;
	unsigned int q_id;
	atomic_t q_spinup_quota;
	struct list_head q_disk_list;
	struct list_head q_head;
};

LIST_HEAD(SpinupListHead);
DEFINE_SPINLOCK(SpinupListLock);

#define MAX_ALLOWED_SPINUP_NUM  12
atomic_t gSpinupCmdNum = ATOMIC_INIT(0);

#endif  

static void sd_config_discard(struct scsi_disk *, unsigned int);
static void sd_config_write_same(struct scsi_disk *);
static int  sd_revalidate_disk(struct gendisk *);
static void sd_unlock_native_capacity(struct gendisk *disk);
static int  sd_probe(struct device *);
static int  sd_remove(struct device *);
static void sd_shutdown(struct device *);
static int sd_suspend_system(struct device *);
static int sd_suspend_runtime(struct device *);
static int sd_resume(struct device *);
static void sd_rescan(struct device *);
static int sd_init_command(struct scsi_cmnd *SCpnt);
static void sd_uninit_command(struct scsi_cmnd *SCpnt);
static int sd_done(struct scsi_cmnd *);
static int sd_eh_action(struct scsi_cmnd *, int);
static void sd_read_capacity(struct scsi_disk *sdkp, unsigned char *buffer);
static void scsi_disk_release(struct device *cdev);
static void sd_print_sense_hdr(struct scsi_disk *, struct scsi_sense_hdr *);
static void sd_print_result(const struct scsi_disk *, const char *, int);

#if defined(MY_ABC_HERE)
extern u8 syno_is_synology_pm(const struct ata_port *ap);
#endif  
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
static DEFINE_IDA(usb_index_ida);
static DEFINE_IDA(iscsi_index_ida);
#elif defined(CONFIG_SYNO_SAS_DISK_NAME)  
static DEFINE_IDA(usb_index_ida);
#if defined(CONFIG_SYNO_SAS_DISK_NAME_ISCSI_DEVICE)
static DEFINE_IDA(iscsi_index_ida);
#endif  
#endif  
#if defined(CONFIG_SYNO_SAS_DISK_NAME) || (defined(CONFIG_SYNO_PORT_MAPPING_V2) && defined(CONFIG_SYNO_SAS))
static DEFINE_IDA(sas_index_ida);
extern int g_is_sas_model;
 
#define SCSI_HOST_SEARCH_DEPTH 5
#endif  
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
static DEFINE_IDA(sata_index_ida);
#endif  

#if defined(MY_DEF_HERE) || defined(CONFIG_SYNO_NEXTKVMX64)
#define _isCDSM_() (syno_is_hw_version(HW_C2DSM))
#define _isSynobootScsiDev_(sdp) (0 == (sdp->host->host_no | sdp->channel | sdp->id | sdp->lun))
#endif  

static DEFINE_SPINLOCK(sd_index_lock);
static DEFINE_IDA(sd_index_ida);
 
static DEFINE_MUTEX(sd_ref_mutex);

static struct kmem_cache *sd_cdb_cache;
static mempool_t *sd_cdb_pool;

static const char *sd_cache_types[] = {
	"write through", "none", "write back",
	"write back, no read (daft)"
};

static void sd_set_flush_flag(struct scsi_disk *sdkp)
{
	unsigned flush = 0;

	if (sdkp->WCE) {
		flush |= REQ_FLUSH;
		if (sdkp->DPOFUA)
			flush |= REQ_FUA;
	}

	blk_queue_flush(sdkp->disk->queue, flush);
}

static ssize_t
cache_type_store(struct device *dev, struct device_attribute *attr,
		 const char *buf, size_t count)
{
	int i, ct = -1, rcd, wce, sp;
	struct scsi_disk *sdkp = to_scsi_disk(dev);
	struct scsi_device *sdp = sdkp->device;
	char buffer[64];
	char *buffer_data;
	struct scsi_mode_data data;
	struct scsi_sense_hdr sshdr;
	static const char temp[] = "temporary ";
	int len;

	if (sdp->type != TYPE_DISK)
		 
		return -EINVAL;

	if (strncmp(buf, temp, sizeof(temp) - 1) == 0) {
		buf += sizeof(temp) - 1;
		sdkp->cache_override = 1;
	} else {
		sdkp->cache_override = 0;
	}

	for (i = 0; i < ARRAY_SIZE(sd_cache_types); i++) {
		len = strlen(sd_cache_types[i]);
		if (strncmp(sd_cache_types[i], buf, len) == 0 &&
		    buf[len] == '\n') {
			ct = i;
			break;
		}
	}
	if (ct < 0)
		return -EINVAL;
	rcd = ct & 0x01 ? 1 : 0;
	wce = (ct & 0x02) && !sdkp->write_prot ? 1 : 0;

	if (sdkp->cache_override) {
		sdkp->WCE = wce;
		sdkp->RCD = rcd;
		sd_set_flush_flag(sdkp);
		return count;
	}

	if (scsi_mode_sense(sdp, 0x08, 8, buffer, sizeof(buffer), SD_TIMEOUT,
			    SD_MAX_RETRIES, &data, NULL))
		return -EINVAL;
	len = min_t(size_t, sizeof(buffer), data.length - data.header_length -
		  data.block_descriptor_length);
	buffer_data = buffer + data.header_length +
		data.block_descriptor_length;
	buffer_data[2] &= ~0x05;
	buffer_data[2] |= wce << 2 | rcd;
	sp = buffer_data[0] & 0x80 ? 1 : 0;
	buffer_data[0] &= ~0x80;

	if (scsi_mode_select(sdp, 1, sp, 8, buffer_data, len, SD_TIMEOUT,
			     SD_MAX_RETRIES, &data, &sshdr)) {
		if (scsi_sense_valid(&sshdr))
			sd_print_sense_hdr(sdkp, &sshdr);
		return -EINVAL;
	}
	revalidate_disk(sdkp->disk);
	return count;
}

static ssize_t
manage_start_stop_show(struct device *dev, struct device_attribute *attr,
		       char *buf)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);
	struct scsi_device *sdp = sdkp->device;

	return snprintf(buf, 20, "%u\n", sdp->manage_start_stop);
}

static ssize_t
manage_start_stop_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);
	struct scsi_device *sdp = sdkp->device;

	if (!capable(CAP_SYS_ADMIN))
		return -EACCES;

	sdp->manage_start_stop = simple_strtoul(buf, NULL, 10);

	return count;
}
static DEVICE_ATTR_RW(manage_start_stop);

#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY

#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY_DEBUG
static void
SpinupQueueDump(struct SpinupQueue *q)
{
	struct scsi_device *d;

	printk(" QUEUE %d:\n", q->q_id);
	list_for_each_entry(d, &(q->q_disk_list), spinup_list) {
		printk("  disk [%d]\n", d->id);
	}
}

static void
SpinupQueueDumpAll(void)
{
	struct SpinupQueue *q;

	printk(" -------- queue dump\n");
	list_for_each_entry(q, &SpinupListHead, q_head) {
		SpinupQueueDump(q);
	}
	printk(" ======== queue dump\n");
}
#endif  

static struct SpinupQueue *
SpinupQueueFindById(unsigned int id)
{
	struct SpinupQueue *q;

	list_for_each_entry(q, &SpinupListHead, q_head) {
		if (q->q_id == id) {
			return q;
		}
	}
	return NULL;
}

static struct SpinupQueue *
SpinupQueueAlloc(unsigned int id)
{
	struct SpinupQueue *qNew;

	qNew = kmalloc(sizeof(struct SpinupQueue), GFP_KERNEL);
	INIT_LIST_HEAD(&(qNew->q_disk_list));
	INIT_LIST_HEAD(&(qNew->q_head));
	qNew->q_id = id;
	spin_lock_init(&(qNew->q_lock));
	if (syno_is_hw_version(HW_HD3400) || syno_is_hw_version(HW_HD6400)) {
		atomic_set(&(qNew->q_spinup_quota), 15);  
	} else {
		atomic_set(&(qNew->q_spinup_quota), 4);  
	}

#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY_DEBUG
	printk(" == add queue %p for id %d\n", qNew, id);
#endif  
	return qNew;
}

static int
SpinupQueueDiskAdd(struct SpinupQueue *sq, struct scsi_device *sd)
{
	unsigned long flags;

	spin_lock_irqsave(&(sq->q_lock), flags);

	list_add_tail(&(sd->spinup_list), &(sq->q_disk_list));
	sd->spinup_queue = sq;

	spin_unlock_irqrestore(&(sq->q_lock), flags);
	return 0;
}

static int
SpinupQueueDiskRemove(struct SpinupQueue *pSQ, struct scsi_device *pSD)
{
	unsigned long flags;

	if (NULL == pSD->spinup_queue) {
		return 0;
	}
	spin_lock_irqsave(&(pSQ->q_lock), flags);

	BUG_ON(pSQ != pSD->spinup_queue);

	list_del(&(pSD->spinup_list));
	pSD->spinup_queue = NULL;

	spin_unlock_irqrestore(&(pSQ->q_lock), flags);
	return 0;
}

static void
SpinupQueueSet(struct scsi_device *sdp, unsigned int new_id)
{
	unsigned int old_id;
	struct SpinupQueue *qOld;
	struct SpinupQueue *qNew;

	unsigned long flags;

	spin_lock_irqsave(&SpinupListLock, flags);

	old_id = sdp->spinup_queue_id;

	if (old_id == new_id) {
		 
		spin_unlock_irqrestore(&SpinupListLock, flags);
		return;
	}
	 
	if (NULL != sdp->spinup_queue) {
		unsigned long flags_sd;

#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY_DEBUG
		sdev_printk(KERN_ERR, sdp, " = remove disk from queue %d\n",
				sdp->spinup_queue->q_id);
#endif  
		 
		qOld = sdp->spinup_queue;
		BUG_ON(NULL == qOld);
		SpinupQueueDiskRemove(qOld, sdp);

		spin_lock_irqsave(&(qOld->q_lock), flags_sd);
		if (list_empty(&(qOld->q_disk_list))) {
			list_del(&(qOld->q_head));
		}
		spin_unlock_irqrestore(&(qOld->q_lock), flags_sd);
	}
	if (new_id) {  
#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY_DEBUG
		sdev_printk(KERN_ERR, sdp, " = add disk to queue %d\n",
				new_id);
#endif  

		qNew = SpinupQueueFindById(new_id);
		 
		if (NULL == qNew) {
			 
			qNew = SpinupQueueAlloc(new_id);
			 
			list_add_tail( &(qNew->q_head), &SpinupListHead);
		}
		 
		SpinupQueueDiskAdd(qNew, sdp);

	}
	sdp->spinup_queue_id = new_id;

#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY_DEBUG
	SpinupQueueDumpAll();
#endif  
	 
	spin_unlock_irqrestore(&SpinupListLock, flags);

}

static ssize_t
spinup_queue_id_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);
	struct scsi_device *sdp = sdkp->device;

	return snprintf(buf, 20, "%u\n", sdp->spinup_queue_id);
}

static ssize_t
spinup_queue_id_store(struct device *dev, struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);
	struct scsi_device *sdp = sdkp->device;
	unsigned int new_id;

	if (!capable(CAP_SYS_ADMIN))
		return -EACCES;

	new_id = simple_strtoul(buf, NULL, 10);

	SpinupQueueSet(sdp, new_id);

	return count;
}
static DEVICE_ATTR_RW(spinup_queue_id);

int
SynoSpinupBegin(struct scsi_device *device)
{
	int ret = 0;
	struct SpinupQueue *q = NULL;
	unsigned long flags;

	if (device && device->spinup_queue) {
		q = device->spinup_queue;
	}
	if (NULL == q) {
		goto Return;
	}
	spin_lock_irqsave(&(q->q_lock), flags);
	 
	if (device->spinup_in_process) {
		 
		if (jiffies - device->spinup_timer > SYNO_SPINUP_RESEND_TIMER) {
#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY_DEBUG
			sdev_printk(KERN_ERR, device, "Retry spinup disk...\n");
#endif  
			device->spinup_timer = jiffies;
			ret = 1;
		}
		goto Return;
	}
	 
	if (atomic_read(&(q->q_spinup_quota))) {
		atomic_dec(&(q->q_spinup_quota));
	} else {
#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY_DEBUG
		sdev_printk(KERN_ERR, device, "No quota to spinup disk...\n");
#endif  
		 
		goto Return;
	}

#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY_DEBUG
	sdev_printk(KERN_ERR, device, "Spinup disk...\n");
#endif  
	device->spinup_in_process = 1;
	device->spinup_timer = jiffies;

	ret = 1;
Return:
	if (NULL != q) {
		spin_unlock_irqrestore(&(q->q_lock), flags);
	}
	return ret;
}

void SynoSpinupEnd(struct scsi_device *sdev)
{
	struct SpinupQueue *q = NULL;
	unsigned long flags;

	if (sdev && sdev->spinup_queue) {
		q = sdev->spinup_queue;
	}

	if(NULL == q) {
		goto Return;
	}

	spin_lock_irqsave(&(q->q_lock), flags);
	if (sdev->spinup_in_process == 0) {
#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY_DEBUG
		sdev_printk(KERN_ERR, sdev, "Spinup should be done already. Q %d remaining %d \n",
			sdev->spinup_queue_id,
			atomic_read(&(q->q_spinup_quota)));
#endif  
		goto Return;
	}
	atomic_inc(&(q->q_spinup_quota));
	sdev->spinup_in_process = 0;
#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY_DEBUG
	sdev_printk(KERN_ERR, sdev, "Spinup done. Q %d remaining %d \n",
			sdev->spinup_queue_id,
			atomic_read(&(q->q_spinup_quota)));
#endif  

Return:
	if (NULL != q) {
		spin_unlock_irqrestore(&(q->q_lock), flags);
	}
}

int SynoSpinupRemove(struct scsi_device *sdev)
{
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&SpinupListLock, flags);
	ret = SpinupQueueDiskRemove(sdev->spinup_queue, sdev);
	spin_unlock_irqrestore(&SpinupListLock, flags);

	return ret;
}
#endif  

static ssize_t
allow_restart_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);

	return snprintf(buf, 40, "%d\n", sdkp->device->allow_restart);
}

static ssize_t
allow_restart_store(struct device *dev, struct device_attribute *attr,
		    const char *buf, size_t count)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);
	struct scsi_device *sdp = sdkp->device;

	if (!capable(CAP_SYS_ADMIN))
		return -EACCES;

	if (sdp->type != TYPE_DISK)
		return -EINVAL;

	sdp->allow_restart = simple_strtoul(buf, NULL, 10);

	return count;
}
static DEVICE_ATTR_RW(allow_restart);

static ssize_t
cache_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);
	int ct = sdkp->RCD + 2*sdkp->WCE;

	return snprintf(buf, 40, "%s\n", sd_cache_types[ct]);
}
static DEVICE_ATTR_RW(cache_type);

static ssize_t
FUA_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);

	return snprintf(buf, 20, "%u\n", sdkp->DPOFUA);
}
static DEVICE_ATTR_RO(FUA);

static ssize_t
protection_type_show(struct device *dev, struct device_attribute *attr,
		     char *buf)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);

	return snprintf(buf, 20, "%u\n", sdkp->protection_type);
}

static ssize_t
protection_type_store(struct device *dev, struct device_attribute *attr,
		      const char *buf, size_t count)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);
	unsigned int val;
	int err;

	if (!capable(CAP_SYS_ADMIN))
		return -EACCES;

	err = kstrtouint(buf, 10, &val);

	if (err)
		return err;

	if (val >= 0 && val <= SD_DIF_TYPE3_PROTECTION)
		sdkp->protection_type = val;

	return count;
}
static DEVICE_ATTR_RW(protection_type);

static ssize_t
protection_mode_show(struct device *dev, struct device_attribute *attr,
		     char *buf)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);
	struct scsi_device *sdp = sdkp->device;
	unsigned int dif, dix;

	dif = scsi_host_dif_capable(sdp->host, sdkp->protection_type);
	dix = scsi_host_dix_capable(sdp->host, sdkp->protection_type);

	if (!dix && scsi_host_dix_capable(sdp->host, SD_DIF_TYPE0_PROTECTION)) {
		dif = 0;
		dix = 1;
	}

	if (!dif && !dix)
		return snprintf(buf, 20, "none\n");

	return snprintf(buf, 20, "%s%u\n", dix ? "dix" : "dif", dif);
}
static DEVICE_ATTR_RO(protection_mode);

static ssize_t
app_tag_own_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);

	return snprintf(buf, 20, "%u\n", sdkp->ATO);
}
static DEVICE_ATTR_RO(app_tag_own);

static ssize_t
thin_provisioning_show(struct device *dev, struct device_attribute *attr,
		       char *buf)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);

	return snprintf(buf, 20, "%u\n", sdkp->lbpme);
}
static DEVICE_ATTR_RO(thin_provisioning);

static const char *lbp_mode[] = {
	[SD_LBP_FULL]		= "full",
	[SD_LBP_UNMAP]		= "unmap",
	[SD_LBP_WS16]		= "writesame_16",
	[SD_LBP_WS10]		= "writesame_10",
	[SD_LBP_ZERO]		= "writesame_zero",
	[SD_LBP_DISABLE]	= "disabled",
};

static ssize_t
provisioning_mode_show(struct device *dev, struct device_attribute *attr,
		       char *buf)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);

	return snprintf(buf, 20, "%s\n", lbp_mode[sdkp->provisioning_mode]);
}

static ssize_t
provisioning_mode_store(struct device *dev, struct device_attribute *attr,
			const char *buf, size_t count)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);
	struct scsi_device *sdp = sdkp->device;

	if (!capable(CAP_SYS_ADMIN))
		return -EACCES;

	if (sdp->type != TYPE_DISK)
		return -EINVAL;

	if (!strncmp(buf, lbp_mode[SD_LBP_UNMAP], 20))
		sd_config_discard(sdkp, SD_LBP_UNMAP);
	else if (!strncmp(buf, lbp_mode[SD_LBP_WS16], 20))
		sd_config_discard(sdkp, SD_LBP_WS16);
	else if (!strncmp(buf, lbp_mode[SD_LBP_WS10], 20))
		sd_config_discard(sdkp, SD_LBP_WS10);
	else if (!strncmp(buf, lbp_mode[SD_LBP_ZERO], 20))
		sd_config_discard(sdkp, SD_LBP_ZERO);
	else if (!strncmp(buf, lbp_mode[SD_LBP_DISABLE], 20))
		sd_config_discard(sdkp, SD_LBP_DISABLE);
	else
		return -EINVAL;

	return count;
}
static DEVICE_ATTR_RW(provisioning_mode);

static ssize_t
max_medium_access_timeouts_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);

	return snprintf(buf, 20, "%u\n", sdkp->max_medium_access_timeouts);
}

static ssize_t
max_medium_access_timeouts_store(struct device *dev,
				 struct device_attribute *attr, const char *buf,
				 size_t count)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);
	int err;

	if (!capable(CAP_SYS_ADMIN))
		return -EACCES;

	err = kstrtouint(buf, 10, &sdkp->max_medium_access_timeouts);

	return err ? err : count;
}
static DEVICE_ATTR_RW(max_medium_access_timeouts);

static ssize_t
max_write_same_blocks_show(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);

	return snprintf(buf, 20, "%u\n", sdkp->max_ws_blocks);
}

static ssize_t
max_write_same_blocks_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);
	struct scsi_device *sdp = sdkp->device;
	unsigned long max;
	int err;

	if (!capable(CAP_SYS_ADMIN))
		return -EACCES;

	if (sdp->type != TYPE_DISK)
		return -EINVAL;

	err = kstrtoul(buf, 10, &max);

	if (err)
		return err;

	if (max == 0)
		sdp->no_write_same = 1;
	else if (max <= SD_MAX_WS16_BLOCKS) {
		sdp->no_write_same = 0;
		sdkp->max_ws_blocks = max;
	}

	sd_config_write_same(sdkp);

	return count;
}
static DEVICE_ATTR_RW(max_write_same_blocks);

static struct attribute *sd_disk_attrs[] = {
	&dev_attr_cache_type.attr,
	&dev_attr_FUA.attr,
	&dev_attr_allow_restart.attr,
	&dev_attr_manage_start_stop.attr,
#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY
	&dev_attr_spinup_queue_id.attr,
#endif  
	&dev_attr_protection_type.attr,
	&dev_attr_protection_mode.attr,
	&dev_attr_app_tag_own.attr,
	&dev_attr_thin_provisioning.attr,
	&dev_attr_provisioning_mode.attr,
	&dev_attr_max_write_same_blocks.attr,
	&dev_attr_max_medium_access_timeouts.attr,
	NULL,
};
ATTRIBUTE_GROUPS(sd_disk);

static struct class sd_disk_class = {
	.name		= "scsi_disk",
	.owner		= THIS_MODULE,
	.dev_release	= scsi_disk_release,
	.dev_groups	= sd_disk_groups,
};

static const struct dev_pm_ops sd_pm_ops = {
	.suspend		= sd_suspend_system,
	.resume			= sd_resume,
	.poweroff		= sd_suspend_system,
	.restore		= sd_resume,
	.runtime_suspend	= sd_suspend_runtime,
	.runtime_resume		= sd_resume,
};

static struct scsi_driver sd_template = {
	.gendrv = {
		.name		= "sd",
		.owner		= THIS_MODULE,
		.probe		= sd_probe,
		.remove		= sd_remove,
		.shutdown	= sd_shutdown,
		.pm		= &sd_pm_ops,
	},
	.rescan			= sd_rescan,
	.init_command		= sd_init_command,
	.uninit_command		= sd_uninit_command,
	.done			= sd_done,
	.eh_action		= sd_eh_action,
};

static struct kobject *sd_default_probe(dev_t devt, int *partno, void *data)
{
	return NULL;
}

static int sd_major(int major_idx)
{
	switch (major_idx) {
	case 0:
		return SCSI_DISK0_MAJOR;
	case 1 ... 7:
		return SCSI_DISK1_MAJOR + major_idx - 1;
	case 8 ... 15:
		return SCSI_DISK8_MAJOR + major_idx - 8;
	default:
		BUG();
		return 0;	 
	}
}

static struct scsi_disk *scsi_disk_get(struct gendisk *disk)
{
	struct scsi_disk *sdkp = NULL;

	mutex_lock(&sd_ref_mutex);

	if (disk->private_data) {
		sdkp = scsi_disk(disk);
		if (scsi_device_get(sdkp->device) == 0)
			get_device(&sdkp->dev);
		else
			sdkp = NULL;
	}
	mutex_unlock(&sd_ref_mutex);
	return sdkp;
}

static void scsi_disk_put(struct scsi_disk *sdkp)
{
	struct scsi_device *sdev = sdkp->device;

	mutex_lock(&sd_ref_mutex);
	put_device(&sdkp->dev);
	scsi_device_put(sdev);
	mutex_unlock(&sd_ref_mutex);
}

static unsigned char sd_setup_protect_cmnd(struct scsi_cmnd *scmd,
					   unsigned int dix, unsigned int dif)
{
	struct bio *bio = scmd->request->bio;
	unsigned int prot_op = sd_prot_op(rq_data_dir(scmd->request), dix, dif);
	unsigned int protect = 0;

	if (dix) {				 
		if (bio_integrity_flagged(bio, BIP_IP_CHECKSUM))
			scmd->prot_flags |= SCSI_PROT_IP_CHECKSUM;

		if (bio_integrity_flagged(bio, BIP_CTRL_NOCHECK) == false)
			scmd->prot_flags |= SCSI_PROT_GUARD_CHECK;
	}

	if (dif != SD_DIF_TYPE3_PROTECTION) {	 
		scmd->prot_flags |= SCSI_PROT_REF_INCREMENT;

		if (bio_integrity_flagged(bio, BIP_CTRL_NOCHECK) == false)
			scmd->prot_flags |= SCSI_PROT_REF_CHECK;
	}

	if (dif) {				 
		scmd->prot_flags |= SCSI_PROT_TRANSFER_PI;

		if (bio_integrity_flagged(bio, BIP_DISK_NOCHECK))
			protect = 3 << 5;	 
		else
			protect = 1 << 5;	 
	}

	scsi_set_prot_op(scmd, prot_op);
	scsi_set_prot_type(scmd, dif);
	scmd->prot_flags &= sd_prot_flag_mask(prot_op);

	return protect;
}

static void sd_config_discard(struct scsi_disk *sdkp, unsigned int mode)
{
	struct request_queue *q = sdkp->disk->queue;
	unsigned int logical_block_size = sdkp->device->sector_size;
	unsigned int max_blocks = 0;

	q->limits.discard_zeroes_data = 0;

	if (sdkp->lbprz) {
		q->limits.discard_alignment = 0;
		q->limits.discard_granularity = logical_block_size;
	} else {
		q->limits.discard_alignment = sdkp->unmap_alignment *
			logical_block_size;
		q->limits.discard_granularity =
			max(sdkp->physical_block_size,
			    sdkp->unmap_granularity * logical_block_size);
	}

	sdkp->provisioning_mode = mode;

	switch (mode) {

	case SD_LBP_DISABLE:
		blk_queue_max_discard_sectors(q, 0);
		queue_flag_clear_unlocked(QUEUE_FLAG_DISCARD, q);
		return;

	case SD_LBP_UNMAP:
		max_blocks = min_not_zero(sdkp->max_unmap_blocks,
					  (u32)SD_MAX_WS16_BLOCKS);
		break;

	case SD_LBP_WS16:
		max_blocks = min_not_zero(sdkp->max_ws_blocks,
					  (u32)SD_MAX_WS16_BLOCKS);
		q->limits.discard_zeroes_data = sdkp->lbprz;
		break;

	case SD_LBP_WS10:
		max_blocks = min_not_zero(sdkp->max_ws_blocks,
					  (u32)SD_MAX_WS10_BLOCKS);
		q->limits.discard_zeroes_data = sdkp->lbprz;
		break;

	case SD_LBP_ZERO:
		max_blocks = min_not_zero(sdkp->max_ws_blocks,
					  (u32)SD_MAX_WS10_BLOCKS);
		q->limits.discard_zeroes_data = 1;
		break;
	}

	blk_queue_max_discard_sectors(q, max_blocks * (logical_block_size >> 9));
	queue_flag_set_unlocked(QUEUE_FLAG_DISCARD, q);
}

static int sd_setup_discard_cmnd(struct scsi_cmnd *cmd)
{
	struct request *rq = cmd->request;
	struct scsi_device *sdp = cmd->device;
	struct scsi_disk *sdkp = scsi_disk(rq->rq_disk);
	sector_t sector = blk_rq_pos(rq);
	unsigned int nr_sectors = blk_rq_sectors(rq);
	unsigned int nr_bytes = blk_rq_bytes(rq);
	unsigned int len;
	int ret;
	char *buf;
	struct page *page;

	sector >>= ilog2(sdp->sector_size) - 9;
	nr_sectors >>= ilog2(sdp->sector_size) - 9;

	page = alloc_page(GFP_ATOMIC | __GFP_ZERO);
	if (!page)
		return BLKPREP_DEFER;

	switch (sdkp->provisioning_mode) {
	case SD_LBP_UNMAP:
		buf = page_address(page);

		cmd->cmd_len = 10;
		cmd->cmnd[0] = UNMAP;
		cmd->cmnd[8] = 24;

		put_unaligned_be16(6 + 16, &buf[0]);
		put_unaligned_be16(16, &buf[2]);
		put_unaligned_be64(sector, &buf[8]);
		put_unaligned_be32(nr_sectors, &buf[16]);

		len = 24;
		break;

	case SD_LBP_WS16:
		cmd->cmd_len = 16;
		cmd->cmnd[0] = WRITE_SAME_16;
		cmd->cmnd[1] = 0x8;  
		put_unaligned_be64(sector, &cmd->cmnd[2]);
		put_unaligned_be32(nr_sectors, &cmd->cmnd[10]);

		len = sdkp->device->sector_size;
		break;

	case SD_LBP_WS10:
	case SD_LBP_ZERO:
		cmd->cmd_len = 10;
		cmd->cmnd[0] = WRITE_SAME;
		if (sdkp->provisioning_mode == SD_LBP_WS10)
			cmd->cmnd[1] = 0x8;  
		put_unaligned_be32(sector, &cmd->cmnd[2]);
		put_unaligned_be16(nr_sectors, &cmd->cmnd[7]);

		len = sdkp->device->sector_size;
		break;

	default:
		ret = BLKPREP_KILL;
		goto out;
	}

	rq->completion_data = page;
	rq->timeout = SD_TIMEOUT;

	cmd->transfersize = len;
	cmd->allowed = SD_MAX_RETRIES;

	blk_add_request_payload(rq, page, len);
	ret = scsi_init_io(cmd);
	rq->__data_len = nr_bytes;

out:
	if (ret != BLKPREP_OK)
		__free_page(page);
	return ret;
}

static void sd_config_write_same(struct scsi_disk *sdkp)
{
	struct request_queue *q = sdkp->disk->queue;
	unsigned int logical_block_size = sdkp->device->sector_size;

	if (sdkp->device->no_write_same) {
		sdkp->max_ws_blocks = 0;
		goto out;
	}

	if (sdkp->max_ws_blocks > SD_MAX_WS10_BLOCKS)
		sdkp->max_ws_blocks = min_not_zero(sdkp->max_ws_blocks,
						   (u32)SD_MAX_WS16_BLOCKS);
	else if (sdkp->ws16 || sdkp->ws10 || sdkp->device->no_report_opcodes)
		sdkp->max_ws_blocks = min_not_zero(sdkp->max_ws_blocks,
						   (u32)SD_MAX_WS10_BLOCKS);
	else {
		sdkp->device->no_write_same = 1;
		sdkp->max_ws_blocks = 0;
	}

out:
	blk_queue_max_write_same_sectors(q, sdkp->max_ws_blocks *
					 (logical_block_size >> 9));
}

static int sd_setup_write_same_cmnd(struct scsi_cmnd *cmd)
{
	struct request *rq = cmd->request;
	struct scsi_device *sdp = cmd->device;
	struct scsi_disk *sdkp = scsi_disk(rq->rq_disk);
	struct bio *bio = rq->bio;
	sector_t sector = blk_rq_pos(rq);
	unsigned int nr_sectors = blk_rq_sectors(rq);
	unsigned int nr_bytes = blk_rq_bytes(rq);
	int ret;

	if (sdkp->device->no_write_same)
		return BLKPREP_KILL;

	BUG_ON(bio_offset(bio) || bio_iovec(bio).bv_len != sdp->sector_size);

	sector >>= ilog2(sdp->sector_size) - 9;
	nr_sectors >>= ilog2(sdp->sector_size) - 9;

	rq->timeout = SD_WRITE_SAME_TIMEOUT;

	if (sdkp->ws16 || sector > 0xffffffff || nr_sectors > 0xffff) {
		cmd->cmd_len = 16;
		cmd->cmnd[0] = WRITE_SAME_16;
		put_unaligned_be64(sector, &cmd->cmnd[2]);
		put_unaligned_be32(nr_sectors, &cmd->cmnd[10]);
	} else {
		cmd->cmd_len = 10;
		cmd->cmnd[0] = WRITE_SAME;
		put_unaligned_be32(sector, &cmd->cmnd[2]);
		put_unaligned_be16(nr_sectors, &cmd->cmnd[7]);
	}

	cmd->transfersize = sdp->sector_size;
	cmd->allowed = SD_MAX_RETRIES;

	rq->__data_len = sdp->sector_size;
	ret = scsi_init_io(cmd);
	rq->__data_len = nr_bytes;
	return ret;
}

static int sd_setup_flush_cmnd(struct scsi_cmnd *cmd)
{
	struct request *rq = cmd->request;

	memset(&cmd->sdb, 0, sizeof(cmd->sdb));

	cmd->cmnd[0] = SYNCHRONIZE_CACHE;
	cmd->cmd_len = 10;
	cmd->transfersize = 0;
	cmd->allowed = SD_MAX_RETRIES;

	rq->timeout = rq->q->rq_timeout * SD_FLUSH_TIMEOUT_MULTIPLIER;
	return BLKPREP_OK;
}

static int sd_setup_read_write_cmnd(struct scsi_cmnd *SCpnt)
{
	struct request *rq = SCpnt->request;
	struct scsi_device *sdp = SCpnt->device;
	struct gendisk *disk = rq->rq_disk;
	struct scsi_disk *sdkp;
	sector_t block = blk_rq_pos(rq);
	sector_t threshold;
	unsigned int this_count = blk_rq_sectors(rq);
	unsigned int dif, dix;
	int ret;
	unsigned char protect;

	ret = scsi_init_io(SCpnt);
	if (ret != BLKPREP_OK)
		goto out;
	SCpnt = rq->special;
	sdkp = scsi_disk(disk);

	ret = BLKPREP_KILL;

	SCSI_LOG_HLQUEUE(1,
		scmd_printk(KERN_INFO, SCpnt,
			"%s: block=%llu, count=%d\n",
			__func__, (unsigned long long)block, this_count));

	if (!sdp || !scsi_device_online(sdp) ||
	    block + blk_rq_sectors(rq) > get_capacity(disk)) {
		SCSI_LOG_HLQUEUE(2, scmd_printk(KERN_INFO, SCpnt,
						"Finishing %u sectors\n",
						blk_rq_sectors(rq)));
		SCSI_LOG_HLQUEUE(2, scmd_printk(KERN_INFO, SCpnt,
						"Retry with 0x%p\n", SCpnt));
		goto out;
	}

	if (sdp->changed) {
		 
		goto out;
	}

	threshold = get_capacity(disk) - SD_LAST_BUGGY_SECTORS *
		(sdp->sector_size / 512);

	if (unlikely(sdp->last_sector_bug && block + this_count > threshold)) {
		if (block < threshold) {
			 
			this_count = threshold - block;
		} else {
			 
			this_count = sdp->sector_size / 512;
		}
	}

	SCSI_LOG_HLQUEUE(2, scmd_printk(KERN_INFO, SCpnt, "block=%llu\n",
					(unsigned long long)block));

	if (sdp->sector_size == 1024) {
		if ((block & 1) || (blk_rq_sectors(rq) & 1)) {
			scmd_printk(KERN_ERR, SCpnt,
				    "Bad block number requested\n");
			goto out;
		} else {
			block = block >> 1;
			this_count = this_count >> 1;
		}
	}
	if (sdp->sector_size == 2048) {
		if ((block & 3) || (blk_rq_sectors(rq) & 3)) {
			scmd_printk(KERN_ERR, SCpnt,
				    "Bad block number requested\n");
			goto out;
		} else {
			block = block >> 2;
			this_count = this_count >> 2;
		}
	}
	if (sdp->sector_size == 4096) {
		if ((block & 7) || (blk_rq_sectors(rq) & 7)) {
			scmd_printk(KERN_ERR, SCpnt,
				    "Bad block number requested\n");
			goto out;
		} else {
			block = block >> 3;
			this_count = this_count >> 3;
		}
	}
	if (rq_data_dir(rq) == WRITE) {
		SCpnt->cmnd[0] = WRITE_6;

		if (blk_integrity_rq(rq))
			sd_dif_prepare(SCpnt);

	} else if (rq_data_dir(rq) == READ) {
		SCpnt->cmnd[0] = READ_6;
	} else {
		scmd_printk(KERN_ERR, SCpnt, "Unknown command %llx\n", (unsigned long long) rq->cmd_flags);
		goto out;
	}

	SCSI_LOG_HLQUEUE(2, scmd_printk(KERN_INFO, SCpnt,
					"%s %d/%u 512 byte blocks.\n",
					(rq_data_dir(rq) == WRITE) ?
					"writing" : "reading", this_count,
					blk_rq_sectors(rq)));

	dix = scsi_prot_sg_count(SCpnt);
	dif = scsi_host_dif_capable(SCpnt->device->host, sdkp->protection_type);

	if (dif || dix)
		protect = sd_setup_protect_cmnd(SCpnt, dix, dif);
	else
		protect = 0;

	if (protect && sdkp->protection_type == SD_DIF_TYPE2_PROTECTION) {
		SCpnt->cmnd = mempool_alloc(sd_cdb_pool, GFP_ATOMIC);

		if (unlikely(SCpnt->cmnd == NULL)) {
			ret = BLKPREP_DEFER;
			goto out;
		}

		SCpnt->cmd_len = SD_EXT_CDB_SIZE;
		memset(SCpnt->cmnd, 0, SCpnt->cmd_len);
		SCpnt->cmnd[0] = VARIABLE_LENGTH_CMD;
		SCpnt->cmnd[7] = 0x18;
		SCpnt->cmnd[9] = (rq_data_dir(rq) == READ) ? READ_32 : WRITE_32;
		SCpnt->cmnd[10] = protect | ((rq->cmd_flags & REQ_FUA) ? 0x8 : 0);

		SCpnt->cmnd[12] = sizeof(block) > 4 ? (unsigned char) (block >> 56) & 0xff : 0;
		SCpnt->cmnd[13] = sizeof(block) > 4 ? (unsigned char) (block >> 48) & 0xff : 0;
		SCpnt->cmnd[14] = sizeof(block) > 4 ? (unsigned char) (block >> 40) & 0xff : 0;
		SCpnt->cmnd[15] = sizeof(block) > 4 ? (unsigned char) (block >> 32) & 0xff : 0;
		SCpnt->cmnd[16] = (unsigned char) (block >> 24) & 0xff;
		SCpnt->cmnd[17] = (unsigned char) (block >> 16) & 0xff;
		SCpnt->cmnd[18] = (unsigned char) (block >> 8) & 0xff;
		SCpnt->cmnd[19] = (unsigned char) block & 0xff;

		SCpnt->cmnd[20] = (unsigned char) (block >> 24) & 0xff;
		SCpnt->cmnd[21] = (unsigned char) (block >> 16) & 0xff;
		SCpnt->cmnd[22] = (unsigned char) (block >> 8) & 0xff;
		SCpnt->cmnd[23] = (unsigned char) block & 0xff;

		SCpnt->cmnd[28] = (unsigned char) (this_count >> 24) & 0xff;
		SCpnt->cmnd[29] = (unsigned char) (this_count >> 16) & 0xff;
		SCpnt->cmnd[30] = (unsigned char) (this_count >> 8) & 0xff;
		SCpnt->cmnd[31] = (unsigned char) this_count & 0xff;
	} else if (sdp->use_16_for_rw || (this_count > 0xffff)) {
		SCpnt->cmnd[0] += READ_16 - READ_6;
		SCpnt->cmnd[1] = protect | ((rq->cmd_flags & REQ_FUA) ? 0x8 : 0);
		SCpnt->cmnd[2] = sizeof(block) > 4 ? (unsigned char) (block >> 56) & 0xff : 0;
		SCpnt->cmnd[3] = sizeof(block) > 4 ? (unsigned char) (block >> 48) & 0xff : 0;
		SCpnt->cmnd[4] = sizeof(block) > 4 ? (unsigned char) (block >> 40) & 0xff : 0;
		SCpnt->cmnd[5] = sizeof(block) > 4 ? (unsigned char) (block >> 32) & 0xff : 0;
		SCpnt->cmnd[6] = (unsigned char) (block >> 24) & 0xff;
		SCpnt->cmnd[7] = (unsigned char) (block >> 16) & 0xff;
		SCpnt->cmnd[8] = (unsigned char) (block >> 8) & 0xff;
		SCpnt->cmnd[9] = (unsigned char) block & 0xff;
		SCpnt->cmnd[10] = (unsigned char) (this_count >> 24) & 0xff;
		SCpnt->cmnd[11] = (unsigned char) (this_count >> 16) & 0xff;
		SCpnt->cmnd[12] = (unsigned char) (this_count >> 8) & 0xff;
		SCpnt->cmnd[13] = (unsigned char) this_count & 0xff;
		SCpnt->cmnd[14] = SCpnt->cmnd[15] = 0;
	} else if ((this_count > 0xff) || (block > 0x1fffff) ||
		   scsi_device_protection(SCpnt->device) ||
		   SCpnt->device->use_10_for_rw) {
		SCpnt->cmnd[0] += READ_10 - READ_6;
		SCpnt->cmnd[1] = protect | ((rq->cmd_flags & REQ_FUA) ? 0x8 : 0);
		SCpnt->cmnd[2] = (unsigned char) (block >> 24) & 0xff;
		SCpnt->cmnd[3] = (unsigned char) (block >> 16) & 0xff;
		SCpnt->cmnd[4] = (unsigned char) (block >> 8) & 0xff;
		SCpnt->cmnd[5] = (unsigned char) block & 0xff;
		SCpnt->cmnd[6] = SCpnt->cmnd[9] = 0;
		SCpnt->cmnd[7] = (unsigned char) (this_count >> 8) & 0xff;
		SCpnt->cmnd[8] = (unsigned char) this_count & 0xff;
	} else {
		if (unlikely(rq->cmd_flags & REQ_FUA)) {
			 
			scmd_printk(KERN_ERR, SCpnt,
				    "FUA write on READ/WRITE(6) drive\n");
			goto out;
		}

		SCpnt->cmnd[1] |= (unsigned char) ((block >> 16) & 0x1f);
		SCpnt->cmnd[2] = (unsigned char) ((block >> 8) & 0xff);
		SCpnt->cmnd[3] = (unsigned char) block & 0xff;
		SCpnt->cmnd[4] = (unsigned char) this_count;
		SCpnt->cmnd[5] = 0;
	}
	SCpnt->sdb.length = this_count * sdp->sector_size;

	SCpnt->transfersize = sdp->sector_size;
	SCpnt->underflow = this_count << 9;
	SCpnt->allowed = SD_MAX_RETRIES;

	ret = BLKPREP_OK;
 out:
	return ret;
}

static int sd_init_command(struct scsi_cmnd *cmd)
{
	struct request *rq = cmd->request;

	if (rq->cmd_flags & REQ_DISCARD)
		return sd_setup_discard_cmnd(cmd);
	else if (rq->cmd_flags & REQ_WRITE_SAME)
		return sd_setup_write_same_cmnd(cmd);
	else if (rq->cmd_flags & REQ_FLUSH)
		return sd_setup_flush_cmnd(cmd);
	else
		return sd_setup_read_write_cmnd(cmd);
}

static void sd_uninit_command(struct scsi_cmnd *SCpnt)
{
	struct request *rq = SCpnt->request;

	if (rq->cmd_flags & REQ_DISCARD)
		__free_page(rq->completion_data);

	if (SCpnt->cmnd != rq->cmd) {
		mempool_free(SCpnt->cmnd, sd_cdb_pool);
		SCpnt->cmnd = NULL;
		SCpnt->cmd_len = 0;
	}
}

static int sd_open(struct block_device *bdev, fmode_t mode)
{
	struct scsi_disk *sdkp = scsi_disk_get(bdev->bd_disk);
	struct scsi_device *sdev;
	int retval;

	if (!sdkp)
		return -ENXIO;

	SCSI_LOG_HLQUEUE(3, sd_printk(KERN_INFO, sdkp, "sd_open\n"));

	sdev = sdkp->device;

	retval = -ENXIO;
	if (!scsi_block_when_processing_errors(sdev))
		goto error_out;

	if (sdev->removable || sdkp->write_prot)
		check_disk_change(bdev);

	retval = -ENOMEDIUM;
	if (sdev->removable && !sdkp->media_present && !(mode & FMODE_NDELAY))
		goto error_out;

	retval = -EROFS;
	if (sdkp->write_prot && (mode & FMODE_WRITE))
		goto error_out;

	retval = -ENXIO;
	if (!scsi_device_online(sdev))
		goto error_out;

	if ((atomic_inc_return(&sdkp->openers) == 1) && sdev->removable) {
		if (scsi_block_when_processing_errors(sdev))
			scsi_set_medium_removal(sdev, SCSI_REMOVAL_PREVENT);
	}

	return 0;

error_out:
	scsi_disk_put(sdkp);
	return retval;	
}

static void sd_release(struct gendisk *disk, fmode_t mode)
{
	struct scsi_disk *sdkp = scsi_disk(disk);
	struct scsi_device *sdev = sdkp->device;

	SCSI_LOG_HLQUEUE(3, sd_printk(KERN_INFO, sdkp, "sd_release\n"));

	if (atomic_dec_return(&sdkp->openers) == 0 && sdev->removable) {
		if (scsi_block_when_processing_errors(sdev))
			scsi_set_medium_removal(sdev, SCSI_REMOVAL_ALLOW);
	}

	scsi_disk_put(sdkp);
}

static int sd_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	struct scsi_disk *sdkp = scsi_disk(bdev->bd_disk);
	struct scsi_device *sdp = sdkp->device;
	struct Scsi_Host *host = sdp->host;
	sector_t capacity = logical_to_sectors(sdp, sdkp->capacity);
	int diskinfo[4];

	diskinfo[0] = 0x40;	 
	diskinfo[1] = 0x20;	 
	diskinfo[2] = capacity >> 11;

	if (host->hostt->bios_param)
		host->hostt->bios_param(sdp, bdev, capacity, diskinfo);
	else
		scsicam_bios_param(bdev, capacity, diskinfo);

	geo->heads = diskinfo[0];
	geo->sectors = diskinfo[1];
	geo->cylinders = diskinfo[2];
	return 0;
}

#ifdef MY_ABC_HERE
static int ScsiSetBadSector(struct gendisk *pDisk, SDBADSECTORS *pSectors)
{
	int iDrive = SynoGetInternalDiskSeq(pDisk->disk_name);
	int max_support_disk = sizeof(grgSdBadSectors)/sizeof(SDBADSECTORS);

	if (NULL == pSectors) {
		return -EINVAL;
	}

	gBadSectorTest = 1;
	if (iDrive >= 0 &&
		iDrive < max_support_disk) {
		if (copy_from_user(&grgSdBadSectors[iDrive], pSectors, sizeof(SDBADSECTORS))) {
			return -EINVAL;
		}

		if (grgSdBadSectors[iDrive].uiEnable) {
			int i;
			for (i = 0; i < 100; i++) {
				printk("%s[%d]:%s set bad sector: %u, max support disk: %d\n",
					   __FILE__, __LINE__, pDisk->disk_name,
					   grgSdBadSectors[iDrive].rgSectors[i], max_support_disk);
				if (0xFFFFFFFF == grgSdBadSectors[iDrive].rgSectors[i]) {
					break;
				}
			}
		}
		return 0;
	} else {
		return -EINVAL;
	}
}
#endif  

static int sd_ioctl(struct block_device *bdev, fmode_t mode,
		    unsigned int cmd, unsigned long arg)
{
	struct gendisk *disk = bdev->bd_disk;
	struct scsi_disk *sdkp = scsi_disk(disk);
	struct scsi_device *sdp = sdkp->device;
	void __user *p = (void __user *)arg;
	int error;
    
	SCSI_LOG_IOCTL(1, sd_printk(KERN_INFO, sdkp, "sd_ioctl: disk=%s, "
				    "cmd=0x%x\n", disk->disk_name, cmd));

	error = scsi_verify_blk_ioctl(bdev, cmd);
	if (error < 0)
		return error;

	error = scsi_ioctl_block_when_processing_errors(sdp, cmd,
			(mode & FMODE_NDELAY) != 0);
	if (error)
		goto out;

	switch (cmd) {
		case SCSI_IOCTL_GET_IDLUN:
		case SCSI_IOCTL_GET_BUS_NUMBER:
			error = scsi_ioctl(sdp, cmd, p);
			break;
#ifdef MY_ABC_HERE
		case SCSI_IOCTL_SET_BADSECTORS:
			return ScsiSetBadSector(disk, p);
#endif  
#ifdef MY_ABC_HERE
		case SD_IOCTL_IDLE:
			return (jiffies - sdp->idle) / HZ + 1;
		case SD_IOCTL_SUPPORT_SLEEP:
		{
			const int iCanSleep = sdp->nospindown ? 0 : 1;
			if (copy_to_user(p, &iCanSleep, sizeof(iCanSleep))) {
				return -EINVAL;
			}
			return 0;
		}
#endif  
#ifdef CONFIG_SYNO_SAS_HOST_DISK_LED_CTRL
		case SD_IOCTL_SASHOST_DISK_LED:
			if (NULL == sdp->host->hostt->syno_set_sashost_disk_led){
				break;
			}
			return sdp->host->hostt->syno_set_sashost_disk_led(sdp, arg);
#endif  
		default:
			error = scsi_cmd_blk_ioctl(bdev, mode, cmd, p);
			if (error != -ENOTTY)
				break;
			error = scsi_ioctl(sdp, cmd, p);
			break;
	}
out:
	return error;
}

static void set_media_not_present(struct scsi_disk *sdkp)
{
	if (sdkp->media_present)
		sdkp->device->changed = 1;

	if (sdkp->device->removable) {
		sdkp->media_present = 0;
		sdkp->capacity = 0;
	}
}

static int media_not_present(struct scsi_disk *sdkp,
			     struct scsi_sense_hdr *sshdr)
{
	if (!scsi_sense_valid(sshdr))
		return 0;

	switch (sshdr->sense_key) {
	case UNIT_ATTENTION:
	case NOT_READY:
		 
		if (sshdr->asc == 0x3A) {
			set_media_not_present(sdkp);
			return 1;
		}
	}
	return 0;
}

static unsigned int sd_check_events(struct gendisk *disk, unsigned int clearing)
{
	struct scsi_disk *sdkp = scsi_disk_get(disk);
	struct scsi_device *sdp;
	struct scsi_sense_hdr *sshdr = NULL;
	int retval;

	if (!sdkp)
		return 0;

	sdp = sdkp->device;
	SCSI_LOG_HLQUEUE(3, sd_printk(KERN_INFO, sdkp, "sd_check_events\n"));

	if (!scsi_device_online(sdp)) {
		set_media_not_present(sdkp);
		goto out;
	}

	retval = -ENODEV;

	if (scsi_block_when_processing_errors(sdp)) {
		sshdr  = kzalloc(sizeof(*sshdr), GFP_KERNEL);
		retval = scsi_test_unit_ready(sdp, SD_TIMEOUT, SD_MAX_RETRIES,
					      sshdr);
	}

	if (host_byte(retval)) {
		set_media_not_present(sdkp);
		goto out;
	}

	if (media_not_present(sdkp, sshdr))
		goto out;

	if (!sdkp->media_present)
		sdp->changed = 1;
	sdkp->media_present = 1;
out:
	 
	kfree(sshdr);
	retval = sdp->changed ? DISK_EVENT_MEDIA_CHANGE : 0;
	sdp->changed = 0;
	scsi_disk_put(sdkp);
	return retval;
}

static int sd_sync_cache(struct scsi_disk *sdkp)
{
	int retries, res;
	struct scsi_device *sdp = sdkp->device;
	const int timeout = sdp->request_queue->rq_timeout
		* SD_FLUSH_TIMEOUT_MULTIPLIER;
	struct scsi_sense_hdr sshdr;

	if (!scsi_device_online(sdp))
		return -ENODEV;

	for (retries = 3; retries > 0; --retries) {
		unsigned char cmd[10] = { 0 };

		cmd[0] = SYNCHRONIZE_CACHE;
		 
		res = scsi_execute_req_flags(sdp, cmd, DMA_NONE, NULL, 0,
					     &sshdr, timeout, SD_MAX_RETRIES,
					     NULL, REQ_PM);
		if (res == 0)
			break;
	}

	if (res) {
		sd_print_result(sdkp, "Synchronize Cache(10) failed", res);

		if (driver_byte(res) & DRIVER_SENSE)
			sd_print_sense_hdr(sdkp, &sshdr);
		 
		if (scsi_sense_valid(&sshdr) &&
			(sshdr.asc == 0x3a ||	 
			 sshdr.asc == 0x20))	 
				 
				return 0;

		switch (host_byte(res)) {
		 
		case DID_BAD_TARGET:
		case DID_NO_CONNECT:
			return 0;
		 
		case DID_BUS_BUSY:
		case DID_IMM_RETRY:
		case DID_REQUEUE:
		case DID_SOFT_ERROR:
			return -EBUSY;
		default:
			return -EIO;
		}
	}
	return 0;
}

static void sd_rescan(struct device *dev)
{
	struct scsi_disk *sdkp = dev_get_drvdata(dev);

	revalidate_disk(sdkp->disk);
}

#ifdef CONFIG_COMPAT
 
static int sd_compat_ioctl(struct block_device *bdev, fmode_t mode,
			   unsigned int cmd, unsigned long arg)
{
	struct scsi_device *sdev = scsi_disk(bdev->bd_disk)->device;
	int error;

	error = scsi_ioctl_block_when_processing_errors(sdev, cmd,
			(mode & FMODE_NDELAY) != 0);
	if (error)
		return error;
	       
	if (!sdev->host->hostt->compat_ioctl)
		return -ENOIOCTLCMD; 
	return sdev->host->hostt->compat_ioctl(sdev, cmd, (void __user *)arg);
}
#endif

static char sd_pr_type(enum pr_type type)
{
	switch (type) {
	case PR_WRITE_EXCLUSIVE:
		return 0x01;
	case PR_EXCLUSIVE_ACCESS:
		return 0x03;
	case PR_WRITE_EXCLUSIVE_REG_ONLY:
		return 0x05;
	case PR_EXCLUSIVE_ACCESS_REG_ONLY:
		return 0x06;
	case PR_WRITE_EXCLUSIVE_ALL_REGS:
		return 0x07;
	case PR_EXCLUSIVE_ACCESS_ALL_REGS:
		return 0x08;
	default:
		return 0;
	}
};

static int sd_pr_command(struct block_device *bdev, u8 sa,
		u64 key, u64 sa_key, u8 type, u8 flags)
{
	struct scsi_device *sdev = scsi_disk(bdev->bd_disk)->device;
	struct scsi_sense_hdr sshdr;
	int result;
	u8 cmd[16] = { 0, };
	u8 data[24] = { 0, };

	cmd[0] = PERSISTENT_RESERVE_OUT;
	cmd[1] = sa;
	cmd[2] = type;
	put_unaligned_be32(sizeof(data), &cmd[5]);

	put_unaligned_be64(key, &data[0]);
	put_unaligned_be64(sa_key, &data[8]);
	data[20] = flags;

	result = scsi_execute_req(sdev, cmd, DMA_TO_DEVICE, &data, sizeof(data),
			&sshdr, SD_TIMEOUT, SD_MAX_RETRIES, NULL);

	if ((driver_byte(result) & DRIVER_SENSE) &&
	    (scsi_sense_valid(&sshdr))) {
		sdev_printk(KERN_INFO, sdev, "PR command failed: %d\n", result);
		scsi_print_sense_hdr(sdev, NULL, &sshdr);
	}

	return result;
}

static int sd_pr_register(struct block_device *bdev, u64 old_key, u64 new_key,
		u32 flags)
{
	if (flags & ~PR_FL_IGNORE_KEY)
		return -EOPNOTSUPP;
	return sd_pr_command(bdev, (flags & PR_FL_IGNORE_KEY) ? 0x06 : 0x00,
			old_key, new_key, 0,
			(1 << 0)   |
			(1 << 2)  );
}

static int sd_pr_reserve(struct block_device *bdev, u64 key, enum pr_type type,
		u32 flags)
{
	if (flags)
		return -EOPNOTSUPP;
	return sd_pr_command(bdev, 0x01, key, 0, sd_pr_type(type), 0);
}

static int sd_pr_release(struct block_device *bdev, u64 key, enum pr_type type)
{
	return sd_pr_command(bdev, 0x02, key, 0, sd_pr_type(type), 0);
}

static int sd_pr_preempt(struct block_device *bdev, u64 old_key, u64 new_key,
		enum pr_type type, bool abort)
{
	return sd_pr_command(bdev, abort ? 0x05 : 0x04, old_key, new_key,
			     sd_pr_type(type), 0);
}

static int sd_pr_clear(struct block_device *bdev, u64 key)
{
	return sd_pr_command(bdev, 0x03, key, 0, 0, 0);
}

static const struct pr_ops sd_pr_ops = {
	.pr_register	= sd_pr_register,
	.pr_reserve	= sd_pr_reserve,
	.pr_release	= sd_pr_release,
	.pr_preempt	= sd_pr_preempt,
	.pr_clear	= sd_pr_clear,
};

static const struct block_device_operations sd_fops = {
	.owner			= THIS_MODULE,
	.open			= sd_open,
	.release		= sd_release,
	.ioctl			= sd_ioctl,
	.getgeo			= sd_getgeo,
#ifdef CONFIG_COMPAT
	.compat_ioctl		= sd_compat_ioctl,
#endif
	.check_events		= sd_check_events,
	.revalidate_disk	= sd_revalidate_disk,
	.unlock_native_capacity	= sd_unlock_native_capacity,
	.pr_ops			= &sd_pr_ops,
};

static int sd_eh_action(struct scsi_cmnd *scmd, int eh_disp)
{
	struct scsi_disk *sdkp = scsi_disk(scmd->request->rq_disk);

	if (!scsi_device_online(scmd->device) ||
	    !scsi_medium_access_command(scmd) ||
	    host_byte(scmd->result) != DID_TIME_OUT ||
	    eh_disp != SUCCESS)
		return eh_disp;

	sdkp->medium_access_timed_out++;

	if (sdkp->medium_access_timed_out >= sdkp->max_medium_access_timeouts) {
		scmd_printk(KERN_ERR, scmd,
			    "Medium access timeout failure. Offlining disk!\n");
		scsi_device_set_state(scmd->device, SDEV_OFFLINE);

		return FAILED;
	}

	return eh_disp;
}

static unsigned int sd_completed_bytes(struct scsi_cmnd *scmd)
{
	u64 start_lba = blk_rq_pos(scmd->request);
	u64 end_lba = blk_rq_pos(scmd->request) + (scsi_bufflen(scmd) / 512);
	u64 factor = scmd->device->sector_size / 512;
	u64 bad_lba;
	int info_valid;
	 
	unsigned int transferred = scsi_bufflen(scmd) - scsi_get_resid(scmd);
	unsigned int good_bytes;

	if (scmd->request->cmd_type != REQ_TYPE_FS)
		return 0;

	info_valid = scsi_get_sense_info_fld(scmd->sense_buffer,
					     SCSI_SENSE_BUFFERSIZE,
					     &bad_lba);
	if (!info_valid)
		return 0;

	if (scsi_bufflen(scmd) <= scmd->device->sector_size)
		return 0;

	do_div(start_lba, factor);
	do_div(end_lba, factor);

	if (bad_lba < start_lba  || bad_lba >= end_lba)
		return 0;

	good_bytes = (bad_lba - start_lba) * scmd->device->sector_size;
	return min(good_bytes, transferred);
}

static int sd_done(struct scsi_cmnd *SCpnt)
{
	int result = SCpnt->result;
	unsigned int good_bytes = result ? 0 : scsi_bufflen(SCpnt);
	struct scsi_sense_hdr sshdr;
	struct scsi_disk *sdkp = scsi_disk(SCpnt->request->rq_disk);
	struct request *req = SCpnt->request;
	int sense_valid = 0;
	int sense_deferred = 0;
	unsigned char op = SCpnt->cmnd[0];
	unsigned char unmap = SCpnt->cmnd[1] & 8;

	if (req->cmd_flags & REQ_DISCARD || req->cmd_flags & REQ_WRITE_SAME) {
		if (!result) {
			good_bytes = blk_rq_bytes(req);
			scsi_set_resid(SCpnt, 0);
		} else {
			good_bytes = 0;
			scsi_set_resid(SCpnt, blk_rq_bytes(req));
		}
	}

	if (result) {
		sense_valid = scsi_command_normalize_sense(SCpnt, &sshdr);
		if (sense_valid)
			sense_deferred = scsi_sense_is_deferred(&sshdr);
	}
	sdkp->medium_access_timed_out = 0;

	if (driver_byte(result) != DRIVER_SENSE &&
	    (!sense_valid || sense_deferred))
		goto out;

	switch (sshdr.sense_key) {
	case HARDWARE_ERROR:
	case MEDIUM_ERROR:
		good_bytes = sd_completed_bytes(SCpnt);
		break;
	case RECOVERED_ERROR:
		good_bytes = scsi_bufflen(SCpnt);
		break;
	case NO_SENSE:
		 
		SCpnt->result = 0;
		memset(SCpnt->sense_buffer, 0, SCSI_SENSE_BUFFERSIZE);
		break;
	case ABORTED_COMMAND:
		if (sshdr.asc == 0x10)   
			good_bytes = sd_completed_bytes(SCpnt);
		break;
	case ILLEGAL_REQUEST:
		if (sshdr.asc == 0x10)   
			good_bytes = sd_completed_bytes(SCpnt);
		 
		if (sshdr.asc == 0x20 || sshdr.asc == 0x24) {
			switch (op) {
			case UNMAP:
				sd_config_discard(sdkp, SD_LBP_DISABLE);
				break;
			case WRITE_SAME_16:
			case WRITE_SAME:
				if (unmap)
					sd_config_discard(sdkp, SD_LBP_DISABLE);
				else {
					sdkp->device->no_write_same = 1;
					sd_config_write_same(sdkp);

					good_bytes = 0;
					req->__data_len = blk_rq_bytes(req);
					req->cmd_flags |= REQ_QUIET;
				}
			}
		}
		break;
	default:
		break;
	}
 out:
	SCSI_LOG_HLCOMPLETE(1, scmd_printk(KERN_INFO, SCpnt,
					   "sd_done: completed %d of %d bytes\n",
					   good_bytes, scsi_bufflen(SCpnt)));

	if (rq_data_dir(SCpnt->request) == READ && scsi_prot_sg_count(SCpnt))
		sd_dif_complete(SCpnt, good_bytes);

	return good_bytes;
}

static void
sd_spinup_disk(struct scsi_disk *sdkp)
{
	unsigned char cmd[10];
	unsigned long spintime_expire = 0;
	int retries, spintime;
	unsigned int the_result;
	struct scsi_sense_hdr sshdr;
	int sense_valid = 0;

	spintime = 0;

	do {
		retries = 0;

		do {
			cmd[0] = TEST_UNIT_READY;
			memset((void *) &cmd[1], 0, 9);

			the_result = scsi_execute_req(sdkp->device, cmd,
						      DMA_NONE, NULL, 0,
						      &sshdr, SD_TIMEOUT,
						      SD_MAX_RETRIES, NULL);

			if (media_not_present(sdkp, &sshdr))
				return;

			if (the_result)
				sense_valid = scsi_sense_valid(&sshdr);
			retries++;
		} while (retries < 3 && 
			 (!scsi_status_is_good(the_result) ||
			  ((driver_byte(the_result) & DRIVER_SENSE) &&
			  sense_valid && sshdr.sense_key == UNIT_ATTENTION)));

		if ((driver_byte(the_result) & DRIVER_SENSE) == 0) {
			 
			if(!spintime && !scsi_status_is_good(the_result)) {
				sd_print_result(sdkp, "Test Unit Ready failed",
						the_result);
			}
			break;
		}

		if (sdkp->device->no_start_on_add)
			break;

		if (sense_valid && sshdr.sense_key == NOT_READY) {
			if (sshdr.asc == 4 && sshdr.ascq == 3)
				break;	 
			if (sshdr.asc == 4 && sshdr.ascq == 0xb)
				break;	 
			if (sshdr.asc == 4 && sshdr.ascq == 0xc)
				break;	 
			 
			if (!spintime) {
				sd_printk(KERN_NOTICE, sdkp, "Spinning up disk...");
				cmd[0] = START_STOP;
				cmd[1] = 1;	 
				memset((void *) &cmd[2], 0, 8);
				cmd[4] = 1;	 
				if (sdkp->device->start_stop_pwr_cond)
					cmd[4] |= 1 << 4;
				scsi_execute_req(sdkp->device, cmd, DMA_NONE,
						 NULL, 0, &sshdr,
						 SD_TIMEOUT, SD_MAX_RETRIES,
						 NULL);
				spintime_expire = jiffies + 100 * HZ;
				spintime = 1;
			}
			 
			msleep(1000);
			printk(".");

		} else if (sense_valid &&
				sshdr.sense_key == UNIT_ATTENTION &&
				sshdr.asc == 0x28) {
			if (!spintime) {
				spintime_expire = jiffies + 5 * HZ;
				spintime = 1;
			}
			 
			msleep(1000);
		} else {
			 
			if(!spintime) {
				sd_printk(KERN_NOTICE, sdkp, "Unit Not Ready\n");
				sd_print_sense_hdr(sdkp, &sshdr);
			}
			break;
		}
				
	} while (spintime && time_before_eq(jiffies, spintime_expire));

	if (spintime) {
		if (scsi_status_is_good(the_result))
			printk("ready\n");
		else
			printk("not responding...\n");
	}
}

static int sd_read_protection_type(struct scsi_disk *sdkp, unsigned char *buffer)
{
	struct scsi_device *sdp = sdkp->device;
	u8 type;
	int ret = 0;

	if (scsi_device_protection(sdp) == 0 || (buffer[12] & 1) == 0)
		return ret;

	type = ((buffer[12] >> 1) & 7) + 1;  

	if (type > SD_DIF_TYPE3_PROTECTION)
		ret = -ENODEV;
	else if (scsi_host_dif_capable(sdp->host, type))
		ret = 1;

	if (sdkp->first_scan || type != sdkp->protection_type)
		switch (ret) {
		case -ENODEV:
			sd_printk(KERN_ERR, sdkp, "formatted with unsupported" \
				  " protection type %u. Disabling disk!\n",
				  type);
			break;
		case 1:
			sd_printk(KERN_NOTICE, sdkp,
				  "Enabling DIF Type %u protection\n", type);
			break;
		case 0:
			sd_printk(KERN_NOTICE, sdkp,
				  "Disabling DIF Type %u protection\n", type);
			break;
		}

	sdkp->protection_type = type;

	return ret;
}

static void read_capacity_error(struct scsi_disk *sdkp, struct scsi_device *sdp,
			struct scsi_sense_hdr *sshdr, int sense_valid,
			int the_result)
{
	if (driver_byte(the_result) & DRIVER_SENSE)
		sd_print_sense_hdr(sdkp, sshdr);
	else
		sd_printk(KERN_NOTICE, sdkp, "Sense not available.\n");

	if (sdp->removable &&
	    sense_valid && sshdr->sense_key == NOT_READY)
		set_media_not_present(sdkp);

	sdkp->capacity = 0;  
}

#define RC16_LEN 32
#if RC16_LEN > SD_BUF_SIZE
#error RC16_LEN must not be more than SD_BUF_SIZE
#endif

#define READ_CAPACITY_RETRIES_ON_RESET	10

static int read_capacity_16(struct scsi_disk *sdkp, struct scsi_device *sdp,
						unsigned char *buffer)
{
	unsigned char cmd[16];
	struct scsi_sense_hdr sshdr;
	int sense_valid = 0;
	int the_result;
	int retries = 3, reset_retries = READ_CAPACITY_RETRIES_ON_RESET;
	unsigned int alignment;
	unsigned long long lba;
	unsigned sector_size;

	if (sdp->no_read_capacity_16)
		return -EINVAL;

	do {
		memset(cmd, 0, 16);
		cmd[0] = SERVICE_ACTION_IN_16;
		cmd[1] = SAI_READ_CAPACITY_16;
		cmd[13] = RC16_LEN;
		memset(buffer, 0, RC16_LEN);

		the_result = scsi_execute_req(sdp, cmd, DMA_FROM_DEVICE,
					buffer, RC16_LEN, &sshdr,
					SD_TIMEOUT, SD_MAX_RETRIES, NULL);

		if (media_not_present(sdkp, &sshdr))
			return -ENODEV;

		if (the_result) {
			sense_valid = scsi_sense_valid(&sshdr);
			if (sense_valid &&
			    sshdr.sense_key == ILLEGAL_REQUEST &&
			    (sshdr.asc == 0x20 || sshdr.asc == 0x24) &&
			    sshdr.ascq == 0x00)
				 
				return -EINVAL;
			if (sense_valid &&
			    sshdr.sense_key == UNIT_ATTENTION &&
			    sshdr.asc == 0x29 && sshdr.ascq == 0x00)
				 
				if (--reset_retries > 0)
					continue;
		}
		retries--;

	} while (the_result && retries);

	if (the_result) {
		sd_print_result(sdkp, "Read Capacity(16) failed", the_result);
		read_capacity_error(sdkp, sdp, &sshdr, sense_valid, the_result);
		return -EINVAL;
	}

	sector_size = get_unaligned_be32(&buffer[8]);
	lba = get_unaligned_be64(&buffer[0]);

	if (sd_read_protection_type(sdkp, buffer) < 0) {
		sdkp->capacity = 0;
		return -ENODEV;
	}

	if ((sizeof(sdkp->capacity) == 4) && (lba >= 0xffffffffULL)) {
		sd_printk(KERN_ERR, sdkp, "Too big for this kernel. Use a "
			"kernel compiled with support for large block "
			"devices.\n");
		sdkp->capacity = 0;
		return -EOVERFLOW;
	}

	sdkp->physical_block_size = (1 << (buffer[13] & 0xf)) * sector_size;

	alignment = ((buffer[14] & 0x3f) << 8 | buffer[15]) * sector_size;
	blk_queue_alignment_offset(sdp->request_queue, alignment);
	if (alignment && sdkp->first_scan)
		sd_printk(KERN_NOTICE, sdkp,
			  "physical block alignment offset: %u\n", alignment);

	if (buffer[14] & 0x80) {  
		sdkp->lbpme = 1;

		if (buffer[14] & 0x40)  
			sdkp->lbprz = 1;

		sd_config_discard(sdkp, SD_LBP_WS16);
	}

	sdkp->capacity = lba + 1;
	return sector_size;
}

static int read_capacity_10(struct scsi_disk *sdkp, struct scsi_device *sdp,
						unsigned char *buffer)
{
	unsigned char cmd[16];
	struct scsi_sense_hdr sshdr;
	int sense_valid = 0;
	int the_result;
	int retries = 3, reset_retries = READ_CAPACITY_RETRIES_ON_RESET;
	sector_t lba;
	unsigned sector_size;

	do {
		cmd[0] = READ_CAPACITY;
		memset(&cmd[1], 0, 9);
		memset(buffer, 0, 8);

		the_result = scsi_execute_req(sdp, cmd, DMA_FROM_DEVICE,
					buffer, 8, &sshdr,
					SD_TIMEOUT, SD_MAX_RETRIES, NULL);

		if (media_not_present(sdkp, &sshdr))
			return -ENODEV;

		if (the_result) {
			sense_valid = scsi_sense_valid(&sshdr);
			if (sense_valid &&
			    sshdr.sense_key == UNIT_ATTENTION &&
			    sshdr.asc == 0x29 && sshdr.ascq == 0x00)
				 
				if (--reset_retries > 0)
					continue;
		}
		retries--;

	} while (the_result && retries);

	if (the_result) {
		sd_print_result(sdkp, "Read Capacity(10) failed", the_result);
		read_capacity_error(sdkp, sdp, &sshdr, sense_valid, the_result);
		return -EINVAL;
	}

	sector_size = get_unaligned_be32(&buffer[4]);
	lba = get_unaligned_be32(&buffer[0]);

	if (sdp->no_read_capacity_16 && (lba == 0xffffffff)) {
		 
		sdkp->capacity = 0;
		sdkp->physical_block_size = sector_size;
		return sector_size;
	}

	if ((sizeof(sdkp->capacity) == 4) && (lba == 0xffffffff)) {
		sd_printk(KERN_ERR, sdkp, "Too big for this kernel. Use a "
			"kernel compiled with support for large block "
			"devices.\n");
		sdkp->capacity = 0;
		return -EOVERFLOW;
	}

	sdkp->capacity = lba + 1;
	sdkp->physical_block_size = sector_size;
	return sector_size;
}

static int sd_try_rc16_first(struct scsi_device *sdp)
{
	if (sdp->host->max_cmd_len < 16)
		return 0;
	if (sdp->try_rc_10_first)
		return 0;
	if (sdp->scsi_level > SCSI_SPC_2)
		return 1;
	if (scsi_device_protection(sdp))
		return 1;
	return 0;
}

static void
sd_read_capacity(struct scsi_disk *sdkp, unsigned char *buffer)
{
	int sector_size;
	struct scsi_device *sdp = sdkp->device;
	sector_t old_capacity = sdkp->capacity;

	if (sd_try_rc16_first(sdp)) {
		sector_size = read_capacity_16(sdkp, sdp, buffer);
		if (sector_size == -EOVERFLOW)
			goto got_data;
		if (sector_size == -ENODEV)
			return;
		if (sector_size < 0)
			sector_size = read_capacity_10(sdkp, sdp, buffer);
		if (sector_size < 0)
			return;
	} else {
		sector_size = read_capacity_10(sdkp, sdp, buffer);
		if (sector_size == -EOVERFLOW)
			goto got_data;
		if (sector_size < 0)
			return;
		if ((sizeof(sdkp->capacity) > 4) &&
		    (sdkp->capacity > 0xffffffffULL)) {
			int old_sector_size = sector_size;
			sd_printk(KERN_NOTICE, sdkp, "Very big device. "
					"Trying to use READ CAPACITY(16).\n");
			sector_size = read_capacity_16(sdkp, sdp, buffer);
			if (sector_size < 0) {
				sd_printk(KERN_NOTICE, sdkp,
					"Using 0xffffffff as device size\n");
				sdkp->capacity = 1 + (sector_t) 0xffffffff;
				sector_size = old_sector_size;
				goto got_data;
			}
		}
	}

	if (sdp->fix_capacity ||
	    (sdp->guess_capacity && (sdkp->capacity & 0x01))) {
		sd_printk(KERN_INFO, sdkp, "Adjusting the sector count "
				"from its reported value: %llu\n",
				(unsigned long long) sdkp->capacity);
		--sdkp->capacity;
	}

got_data:
	if (sector_size == 0) {
		sector_size = 512;
		sd_printk(KERN_NOTICE, sdkp, "Sector size 0 reported, "
			  "assuming 512.\n");
	}

	if (sector_size != 512 &&
	    sector_size != 1024 &&
	    sector_size != 2048 &&
	    sector_size != 4096) {
		sd_printk(KERN_NOTICE, sdkp, "Unsupported sector size %d.\n",
			  sector_size);
		 
		sdkp->capacity = 0;
		 
		sector_size = 512;
	}
	blk_queue_logical_block_size(sdp->request_queue, sector_size);

	{
		char cap_str_2[10], cap_str_10[10];

		string_get_size(sdkp->capacity, sector_size,
				STRING_UNITS_2, cap_str_2, sizeof(cap_str_2));
		string_get_size(sdkp->capacity, sector_size,
				STRING_UNITS_10, cap_str_10,
				sizeof(cap_str_10));

		if (sdkp->first_scan || old_capacity != sdkp->capacity) {
			sd_printk(KERN_NOTICE, sdkp,
				  "%llu %d-byte logical blocks: (%s/%s)\n",
				  (unsigned long long)sdkp->capacity,
				  sector_size, cap_str_10, cap_str_2);

			if (sdkp->physical_block_size != sector_size)
				sd_printk(KERN_NOTICE, sdkp,
					  "%u-byte physical blocks\n",
					  sdkp->physical_block_size);
		}
	}

	if (sdkp->capacity > 0xffffffff)
		sdp->use_16_for_rw = 1;

	blk_queue_physical_block_size(sdp->request_queue,
				      sdkp->physical_block_size);
	sdkp->device->sector_size = sector_size;
}

static inline int
sd_do_mode_sense(struct scsi_device *sdp, int dbd, int modepage,
		 unsigned char *buffer, int len, struct scsi_mode_data *data,
		 struct scsi_sense_hdr *sshdr)
{
	return scsi_mode_sense(sdp, dbd, modepage, buffer, len,
			       SD_TIMEOUT, SD_MAX_RETRIES, data,
			       sshdr);
}

static void
sd_read_write_protect_flag(struct scsi_disk *sdkp, unsigned char *buffer)
{
	int res;
	struct scsi_device *sdp = sdkp->device;
	struct scsi_mode_data data;
	int old_wp = sdkp->write_prot;

	set_disk_ro(sdkp->disk, 0);
	if (sdp->skip_ms_page_3f) {
		sd_first_printk(KERN_NOTICE, sdkp, "Assuming Write Enabled\n");
		return;
	}

	if (sdp->use_192_bytes_for_3f) {
		res = sd_do_mode_sense(sdp, 0, 0x3F, buffer, 192, &data, NULL);
	} else {
		 
		res = sd_do_mode_sense(sdp, 0, 0x3F, buffer, 4, &data, NULL);

		if (!scsi_status_is_good(res))
			res = sd_do_mode_sense(sdp, 0, 0, buffer, 4, &data, NULL);

		if (!scsi_status_is_good(res))
			res = sd_do_mode_sense(sdp, 0, 0x3F, buffer, 255,
					       &data, NULL);
	}

	if (!scsi_status_is_good(res)) {
		sd_first_printk(KERN_WARNING, sdkp,
			  "Test WP failed, assume Write Enabled\n");
	} else {
		sdkp->write_prot = ((data.device_specific & 0x80) != 0);
		set_disk_ro(sdkp->disk, sdkp->write_prot);
		if (sdkp->first_scan || old_wp != sdkp->write_prot) {
			sd_printk(KERN_NOTICE, sdkp, "Write Protect is %s\n",
				  sdkp->write_prot ? "on" : "off");
			sd_printk(KERN_DEBUG, sdkp,
				  "Mode Sense: %02x %02x %02x %02x\n",
				  buffer[0], buffer[1], buffer[2], buffer[3]);
		}
	}
}

static void
sd_read_cache_type(struct scsi_disk *sdkp, unsigned char *buffer)
{
	int len = 0, res;
	struct scsi_device *sdp = sdkp->device;

	int dbd;
	int modepage;
	int first_len;
	struct scsi_mode_data data;
	struct scsi_sense_hdr sshdr;
	int old_wce = sdkp->WCE;
	int old_rcd = sdkp->RCD;
	int old_dpofua = sdkp->DPOFUA;

	if (sdkp->cache_override)
		return;

	first_len = 4;
	if (sdp->skip_ms_page_8) {
		if (sdp->type == TYPE_RBC)
			goto defaults;
		else {
			if (sdp->skip_ms_page_3f)
				goto defaults;
			modepage = 0x3F;
			if (sdp->use_192_bytes_for_3f)
				first_len = 192;
			dbd = 0;
		}
	} else if (sdp->type == TYPE_RBC) {
		modepage = 6;
		dbd = 8;
	} else {
		modepage = 8;
		dbd = 0;
	}

	res = sd_do_mode_sense(sdp, dbd, modepage, buffer, first_len,
			&data, &sshdr);

	if (!scsi_status_is_good(res))
		goto bad_sense;

	if (!data.header_length) {
		modepage = 6;
		first_len = 0;
		sd_first_printk(KERN_ERR, sdkp,
				"Missing header in MODE_SENSE response\n");
	}

	len = data.length;

	if (len < 3)
		goto bad_sense;
	else if (len > SD_BUF_SIZE) {
		sd_first_printk(KERN_NOTICE, sdkp, "Truncating mode parameter "
			  "data from %d to %d bytes\n", len, SD_BUF_SIZE);
		len = SD_BUF_SIZE;
	}
	if (modepage == 0x3F && sdp->use_192_bytes_for_3f)
		len = 192;

	if (len > first_len)
		res = sd_do_mode_sense(sdp, dbd, modepage, buffer, len,
				&data, &sshdr);

	if (scsi_status_is_good(res)) {
		int offset = data.header_length + data.block_descriptor_length;

		while (offset < len) {
			u8 page_code = buffer[offset] & 0x3F;
			u8 spf       = buffer[offset] & 0x40;

			if (page_code == 8 || page_code == 6) {
				 
				if (len - offset <= 2) {
					sd_first_printk(KERN_ERR, sdkp,
						"Incomplete mode parameter "
							"data\n");
					goto defaults;
				} else {
					modepage = page_code;
					goto Page_found;
				}
			} else {
				 
				if (spf && len - offset > 3)
					offset += 4 + (buffer[offset+2] << 8) +
						buffer[offset+3];
				else if (!spf && len - offset > 1)
					offset += 2 + buffer[offset+1];
				else {
					sd_first_printk(KERN_ERR, sdkp,
							"Incomplete mode "
							"parameter data\n");
					goto defaults;
				}
			}
		}

		sd_first_printk(KERN_ERR, sdkp, "No Caching mode page found\n");
		goto defaults;

	Page_found:
		if (modepage == 8) {
			sdkp->WCE = ((buffer[offset + 2] & 0x04) != 0);
			sdkp->RCD = ((buffer[offset + 2] & 0x01) != 0);
		} else {
			sdkp->WCE = ((buffer[offset + 2] & 0x01) == 0);
			sdkp->RCD = 0;
		}

		sdkp->DPOFUA = (data.device_specific & 0x10) != 0;
		if (sdp->broken_fua) {
			sd_first_printk(KERN_NOTICE, sdkp, "Disabling FUA\n");
			sdkp->DPOFUA = 0;
		} else if (sdkp->DPOFUA && !sdkp->device->use_10_for_rw) {
			sd_first_printk(KERN_NOTICE, sdkp,
				  "Uses READ/WRITE(6), disabling FUA\n");
			sdkp->DPOFUA = 0;
		}

		if (sdkp->WCE && sdkp->write_prot)
			sdkp->WCE = 0;

		if (sdkp->first_scan || old_wce != sdkp->WCE ||
		    old_rcd != sdkp->RCD || old_dpofua != sdkp->DPOFUA)
			sd_printk(KERN_NOTICE, sdkp,
				  "Write cache: %s, read cache: %s, %s\n",
				  sdkp->WCE ? "enabled" : "disabled",
				  sdkp->RCD ? "disabled" : "enabled",
				  sdkp->DPOFUA ? "supports DPO and FUA"
				  : "doesn't support DPO or FUA");

		return;
	}

bad_sense:
	if (scsi_sense_valid(&sshdr) &&
	    sshdr.sense_key == ILLEGAL_REQUEST &&
	    sshdr.asc == 0x24 && sshdr.ascq == 0x0)
		 
		sd_first_printk(KERN_NOTICE, sdkp, "Cache data unavailable\n");
	else
		sd_first_printk(KERN_ERR, sdkp,
				"Asking for cache data failed\n");

defaults:
	if (sdp->wce_default_on) {
		sd_first_printk(KERN_NOTICE, sdkp,
				"Assuming drive cache: write back\n");
		sdkp->WCE = 1;
	} else {
		sd_first_printk(KERN_ERR, sdkp,
				"Assuming drive cache: write through\n");
		sdkp->WCE = 0;
	}
	sdkp->RCD = 0;
	sdkp->DPOFUA = 0;
}

static void sd_read_app_tag_own(struct scsi_disk *sdkp, unsigned char *buffer)
{
	int res, offset;
	struct scsi_device *sdp = sdkp->device;
	struct scsi_mode_data data;
	struct scsi_sense_hdr sshdr;

	if (sdp->type != TYPE_DISK)
		return;

	if (sdkp->protection_type == 0)
		return;

	res = scsi_mode_sense(sdp, 1, 0x0a, buffer, 36, SD_TIMEOUT,
			      SD_MAX_RETRIES, &data, &sshdr);

	if (!scsi_status_is_good(res) || !data.header_length ||
	    data.length < 6) {
		sd_first_printk(KERN_WARNING, sdkp,
			  "getting Control mode page failed, assume no ATO\n");

		if (scsi_sense_valid(&sshdr))
			sd_print_sense_hdr(sdkp, &sshdr);

		return;
	}

	offset = data.header_length + data.block_descriptor_length;

	if ((buffer[offset] & 0x3f) != 0x0a) {
		sd_first_printk(KERN_ERR, sdkp, "ATO Got wrong page\n");
		return;
	}

	if ((buffer[offset + 5] & 0x80) == 0)
		return;

	sdkp->ATO = 1;

	return;
}

#ifdef MY_ABC_HERE
 
int
syno_get_ata_identity(struct scsi_device *sdev, u16 *id)
{
	unsigned char scsi_cmd[MAX_COMMAND_SIZE] = {0};

	scsi_cmd[0] = ATA_16;
	scsi_cmd[1] = 0x08;  
	scsi_cmd[2] = 0x0e;  
	scsi_cmd[14] = ATA_CMD_ID_ATA;

	if (scsi_execute_req(sdev, scsi_cmd, DMA_FROM_DEVICE,
		id, 512, NULL, 10 * HZ, 5, NULL)) {
		return 0;
	}

	return 1;
}
EXPORT_SYMBOL(syno_get_ata_identity);
#endif  

static void sd_read_block_limits(struct scsi_disk *sdkp)
{
	unsigned int sector_sz = sdkp->device->sector_size;
	const int vpd_len = 64;
	unsigned char *buffer = kmalloc(vpd_len, GFP_KERNEL);

	if (!buffer ||
	     
	    scsi_get_vpd_page(sdkp->device, 0xb0, buffer, vpd_len))
		goto out;

	blk_queue_io_min(sdkp->disk->queue,
			 get_unaligned_be16(&buffer[6]) * sector_sz);

	sdkp->max_xfer_blocks = get_unaligned_be32(&buffer[8]);
	sdkp->opt_xfer_blocks = get_unaligned_be32(&buffer[12]);

	if (buffer[3] == 0x3c) {
		unsigned int lba_count, desc_count;

		sdkp->max_ws_blocks = (u32)get_unaligned_be64(&buffer[36]);

		if (!sdkp->lbpme)
			goto out;

		lba_count = get_unaligned_be32(&buffer[20]);
		desc_count = get_unaligned_be32(&buffer[24]);

		if (lba_count && desc_count)
			sdkp->max_unmap_blocks = lba_count;

		sdkp->unmap_granularity = get_unaligned_be32(&buffer[28]);

#ifdef CONFIG_SYNO_SAS_FIX_TRIM_GRANULARITY
#define SYNO_MD_CHUNK_SIZE 65536
	 
	if (1 == g_is_sas_model && (SYNO_MD_CHUNK_SIZE >> 9) < sdkp->unmap_granularity) {
		u16 id[ATA_ID_WORDS] = {0};
		if (syno_get_ata_identity(sdkp->device, id) && ata_id_has_trim(id)) {
			sdkp->unmap_granularity = 1;
		}
	}
#endif  

		if (buffer[32] & 0x80)
			sdkp->unmap_alignment =
				get_unaligned_be32(&buffer[32]) & ~(1 << 31);

		if (!sdkp->lbpvpd) {  

			if (sdkp->max_unmap_blocks)
				sd_config_discard(sdkp, SD_LBP_UNMAP);
			else
				sd_config_discard(sdkp, SD_LBP_WS16);

		} else {	 
			if (sdkp->lbpu && sdkp->max_unmap_blocks && !sdkp->lbprz)
				sd_config_discard(sdkp, SD_LBP_UNMAP);
			else if (sdkp->lbpws)
				sd_config_discard(sdkp, SD_LBP_WS16);
			else if (sdkp->lbpws10)
				sd_config_discard(sdkp, SD_LBP_WS10);
			else if (sdkp->lbpu && sdkp->max_unmap_blocks)
				sd_config_discard(sdkp, SD_LBP_UNMAP);
			else
				sd_config_discard(sdkp, SD_LBP_DISABLE);
		}
	}

 out:
	kfree(buffer);
}

static void sd_read_block_characteristics(struct scsi_disk *sdkp)
{
	unsigned char *buffer;
	u16 rot;
	const int vpd_len = 64;

	buffer = kmalloc(vpd_len, GFP_KERNEL);

	if (!buffer ||
	     
	    scsi_get_vpd_page(sdkp->device, 0xb1, buffer, vpd_len))
		goto out;

	rot = get_unaligned_be16(&buffer[4]);

	if (rot == 1) {
		queue_flag_set_unlocked(QUEUE_FLAG_NONROT, sdkp->disk->queue);
		queue_flag_clear_unlocked(QUEUE_FLAG_ADD_RANDOM, sdkp->disk->queue);
	}

 out:
	kfree(buffer);
}

static void sd_read_block_provisioning(struct scsi_disk *sdkp)
{
	unsigned char *buffer;
	const int vpd_len = 8;

	if (sdkp->lbpme == 0)
		return;

	buffer = kmalloc(vpd_len, GFP_KERNEL);

	if (!buffer || scsi_get_vpd_page(sdkp->device, 0xb2, buffer, vpd_len))
		goto out;

	sdkp->lbpvpd	= 1;
	sdkp->lbpu	= (buffer[5] >> 7) & 1;	 
	sdkp->lbpws	= (buffer[5] >> 6) & 1;	 
	sdkp->lbpws10	= (buffer[5] >> 5) & 1;	 

 out:
	kfree(buffer);
}

static void sd_read_write_same(struct scsi_disk *sdkp, unsigned char *buffer)
{
	struct scsi_device *sdev = sdkp->device;

	if (sdev->host->no_write_same) {
		sdev->no_write_same = 1;

		return;
	}

	if (scsi_report_opcode(sdev, buffer, SD_BUF_SIZE, INQUIRY) < 0) {
		 
		int vpd_buf_len = 64;

		sdev->no_report_opcodes = 1;

		if (!scsi_get_vpd_page(sdev, 0x89, buffer, vpd_buf_len))
			sdev->no_write_same = 1;
	}

	if (scsi_report_opcode(sdev, buffer, SD_BUF_SIZE, WRITE_SAME_16) == 1)
		sdkp->ws16 = 1;

	if (scsi_report_opcode(sdev, buffer, SD_BUF_SIZE, WRITE_SAME) == 1)
		sdkp->ws10 = 1;
}

static int sd_try_extended_inquiry(struct scsi_device *sdp)
{
	 
	if (sdp->try_vpd_pages)
		return 1;
	 
	if (sdp->scsi_level > SCSI_SPC_2 && !sdp->skip_vpd_pages)
		return 1;
	return 0;
}

static int sd_revalidate_disk(struct gendisk *disk)
{
	struct scsi_disk *sdkp = scsi_disk(disk);
	struct scsi_device *sdp = sdkp->device;
	struct request_queue *q = sdkp->disk->queue;
	unsigned char *buffer;
	unsigned int dev_max, rw_max;

	SCSI_LOG_HLQUEUE(3, sd_printk(KERN_INFO, sdkp,
				      "sd_revalidate_disk\n"));

	if (!scsi_device_online(sdp))
		goto out;

	buffer = kmalloc(SD_BUF_SIZE, GFP_KERNEL);
	if (!buffer) {
		sd_printk(KERN_WARNING, sdkp, "sd_revalidate_disk: Memory "
			  "allocation failure.\n");
		goto out;
	}

#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY
	if (1 == g_is_sas_model) {
		 
		while (MAX_ALLOWED_SPINUP_NUM < atomic_read(&(gSpinupCmdNum))) {
			msleep(1000);
		}

		atomic_inc(&(gSpinupCmdNum));
		sd_spinup_disk(sdkp);
		atomic_dec(&(gSpinupCmdNum));
	} else {
#endif  
	sd_spinup_disk(sdkp);
#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY
	}
#endif  

	if (sdkp->media_present) {
		sd_read_capacity(sdkp, buffer);

		if (sd_try_extended_inquiry(sdp)) {
			sd_read_block_provisioning(sdkp);
			sd_read_block_limits(sdkp);
			sd_read_block_characteristics(sdkp);
		}

		sd_read_write_protect_flag(sdkp, buffer);
		sd_read_cache_type(sdkp, buffer);
		sd_read_app_tag_own(sdkp, buffer);
		sd_read_write_same(sdkp, buffer);
	}

	sdkp->first_scan = 0;

	sd_set_flush_flag(sdkp);

	dev_max = sdp->use_16_for_rw ? SD_MAX_XFER_BLOCKS : SD_DEF_XFER_BLOCKS;

	dev_max = min_not_zero(dev_max, sdkp->max_xfer_blocks);
	q->limits.max_dev_sectors = logical_to_sectors(sdp, dev_max);

	if (sdkp->opt_xfer_blocks &&
	    sdkp->opt_xfer_blocks <= dev_max &&
	    sdkp->opt_xfer_blocks <= SD_DEF_XFER_BLOCKS &&
	    logical_to_bytes(sdp, sdkp->opt_xfer_blocks) >= PAGE_CACHE_SIZE) {
		q->limits.io_opt = logical_to_bytes(sdp, sdkp->opt_xfer_blocks);
		rw_max = logical_to_sectors(sdp, sdkp->opt_xfer_blocks);
	} else
		rw_max = BLK_DEF_MAX_SECTORS;

	q->limits.max_sectors = min(rw_max, queue_max_hw_sectors(q));

	set_capacity(disk, logical_to_sectors(sdp, sdkp->capacity));
	sd_config_write_same(sdkp);
	kfree(buffer);

 out:
	return 0;
}

#if defined(MY_ABC_HERE ) || defined(CONFIG_SYNO_PORT_MAPPING_V2)
extern int syno_ida_get_new(struct ida *idp, int starting_id, int *id);
#endif  

#if defined(CONFIG_SYNO_SAS_DISK_NAME) || defined(MY_DEF_HERE) || defined(CONFIG_SYNO_PORT_MAPPING_V2)
 
static int syno_sd_format_numeric_disk_name(char *prefix, int synoindex, char *buf, int buflen)
{
	 
	if (buflen <= (strlen(prefix) + (synoindex + 1)/10 + 1)) {
		return -EINVAL;
	}

	if (snprintf(buf, buflen, "%s%d", prefix, synoindex + 1) <= 0) {
		return -EINVAL;
	}

	return 0;
}
#endif  

static void sd_unlock_native_capacity(struct gendisk *disk)
{
	struct scsi_device *sdev = scsi_disk(disk)->device;

	if (sdev->host->hostt->unlock_native_capacity)
		sdev->host->hostt->unlock_native_capacity(sdev);
}
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
#else
 
static int sd_format_disk_name(char *prefix, int index, char *buf, int buflen)
{
	const int base = 'z' - 'a' + 1;
	char *begin = buf + strlen(prefix);
	char *end = buf + buflen;
	char *p;
	int unit;

	p = end - 1;
	*p = '\0';
	unit = base;
	do {
		if (p == begin)
			return -EINVAL;
		*--p = 'a' + (index % unit);
		index = (index / unit) - 1;
	} while (index >= 0);

	memmove(begin, p, end - p);
	memcpy(buf, prefix, strlen(prefix));

	return 0;
}
#endif  

static void sd_probe_async(void *data, async_cookie_t cookie)
{
	struct scsi_disk *sdkp = data;
	struct scsi_device *sdp;
	struct gendisk *gd;
	u32 index;
	struct device *dev;

	sdp = sdkp->device;
	gd = sdkp->disk;
	index = sdkp->index;
	dev = &sdp->sdev_gendev;

	gd->major = sd_major((index & 0xf0) >> 4);
	gd->first_minor = ((index & 0xf) << 4) | (index & 0xfff00);
	gd->minors = SD_MINORS;

	gd->fops = &sd_fops;
	gd->private_data = &sdkp->driver;
	gd->queue = sdkp->device->request_queue;

	sdp->sector_size = 512;
	sdkp->capacity = 0;
	sdkp->media_present = 1;
	sdkp->write_prot = 0;
	sdkp->cache_override = 0;
	sdkp->WCE = 0;
	sdkp->RCD = 0;
	sdkp->ATO = 0;
	sdkp->first_scan = 1;
	sdkp->max_medium_access_timeouts = SD_MAX_MEDIUM_TIMEOUTS;

	sd_revalidate_disk(gd);

	gd->driverfs_dev = &sdp->sdev_gendev;
	gd->flags = GENHD_FL_EXT_DEVT;
	if (sdp->removable) {
		gd->flags |= GENHD_FL_REMOVABLE;
		gd->events |= DISK_EVENT_MEDIA_CHANGE;
	}

	blk_pm_runtime_init(sdp->request_queue, dev);
	add_disk(gd);
	if (sdkp->capacity)
		sd_dif_config_host(sdkp);

	sd_revalidate_disk(gd);

	sd_printk(KERN_NOTICE, sdkp, "Attached SCSI %sdisk\n",
		  sdp->removable ? "removable " : "");
	scsi_autopm_put_device(sdp);
	put_device(&sdkp->dev);
}

#ifdef MY_ABC_HERE
static bool syno_find_synoboot(void)
{
	bool find = false;
	struct scsi_disk *sdisk = NULL;
	struct class_dev_iter iter;
	struct device *dev;

	class_dev_iter_init(&iter, &sd_disk_class, NULL, NULL);
	dev = class_dev_iter_next(&iter);
	while (dev) {
		if (!dev->parent) {
			dev = class_dev_iter_next(&iter);
			continue;
		}
		sdisk = dev_get_drvdata(dev->parent);
		if (sdisk && sdisk->disk) {
			if (0 == strcmp(CONFIG_SYNO_USB_FLASH_DEVICE_NAME, sdisk->disk->disk_name)) {
				find = true;
				goto OUT;
			}
		}
		dev = class_dev_iter_next(&iter);
	}
OUT:
	class_dev_iter_exit(&iter);
	return find;
}
#endif  

#if defined(MY_ABC_HERE) || defined(CONFIG_SYNO_PORT_MAPPING_V2)
static SYNO_DISK_TYPE syno_disk_type_get(struct device *dev)
{
	struct scsi_device *sdp = to_scsi_device(dev);
#ifdef MY_DEF_HERE
	struct device *virtdev = sdp->host->shost_gendev.parent;
	struct pci_dev *pcidev = NULL;
#endif  
#ifdef MY_DEF_HERE
	bool blIsSynoboot = false;
#endif  
#ifdef CONFIG_SYNO_REMAP_SATA_TO_SYS_ONLY_DEV
	int synoDiskIdx = 0;
#endif  

#if defined(MY_DEF_HERE) || defined(CONFIG_SYNO_NEXTKVMX64)
	if (_isCDSM_() && _isSynobootScsiDev_(sdp)) {
		return SYNO_DISK_SYNOBOOT;
	}
#endif  
#ifdef MY_ABC_HERE
	 
	if (0 == strcmp(sdp->host->hostt->name, "iSCSI Initiator over TCP/IP")) {
		return SYNO_DISK_ISCSI;
	}
#endif  

#ifdef MY_ABC_HERE
	if(strcmp(sdp->host->hostt->name, "TCM_Loopback") == 0){
		return SYNO_DISK_ISCSI;
	}
#endif  

#if defined(MY_DEF_HERE) || defined(CONFIG_SYNO_NEXTKVMX64)
	if (strcmp(sdp->host->hostt->name, "Virtio SCSI HBA") == 0){
#ifdef MY_DEF_HERE
		while (virtdev) {
			if (virtdev->driver && virtdev->driver->name && !strcmp(virtdev->driver->name, "virtio-pci")) {
				pcidev = to_pci_dev(virtdev);
				break;
			}
			virtdev = virtdev->parent;
		}
		if (pcidev && PCI_SLOT(pcidev->devfn) == CONFIG_SYNO_KVMX64_PCI_SLOT_BOOT) {
			return SYNO_DISK_SYNOBOOT;
		}
#endif  
		return SYNO_DISK_SATA;
	}
#endif  

	if (SYNO_PORT_TYPE_USB == sdp->host->hostt->syno_port_type) {
#ifdef MY_ABC_HERE
		struct us_data *us = host_to_us(sdp->host);
		struct usb_device *usbdev = us->pusb_dev;
		 
		if (NULL == us) {
			return SYNO_DISK_USB;
		}
		if (NULL == usbdev) {
			return SYNO_DISK_USB;
		}
		if (IS_SYNO_USBBOOT_ID_VENDOR(le16_to_cpu(usbdev->descriptor.idVendor)) &&
			IS_SYNO_USBBOOT_ID_PRODUCT(le16_to_cpu(usbdev->descriptor.idProduct))) {
#ifdef MY_ABC_HERE
			if (0 == gSynoHasDynModule) {
				return SYNO_DISK_USB;
			}
#endif  
			if (!syno_find_synoboot()) {
				return SYNO_DISK_SYNOBOOT;
			}
		}
#endif  
		return SYNO_DISK_USB;
	}

	if (SYNO_PORT_TYPE_SATA == sdp->host->hostt->syno_port_type) {
#ifdef CONFIG_SYNO_REMAP_SATA_TO_SYS_ONLY_DEV
		if (sdp->host->hostt->syno_index_get) {
			synoDiskIdx = sdp->host->hostt->syno_index_get(sdp->host,
														   sdp->channel,
														   sdp->id, sdp->lun);
			if ((SYSTEM_DEVICE_START_IDX <= synoDiskIdx) &&
				(synoDiskIdx <= SYSTEM_DEVICE_START_IDX + SYSTEM_DEVICE_NUM_MAX)) {
				return SYNO_DISK_SYSTEM;
			}
		}
#endif  

#ifdef MY_DEF_HERE
		 
		if (1 == gSynoBootSATADOM) {
			if (!strncmp(CONFIG_SYNO_SATA_DOM_VENDOR, sdp->vendor, strlen(CONFIG_SYNO_SATA_DOM_VENDOR))
				&& !strncmp(CONFIG_SYNO_SATA_DOM_MODEL, sdp->model, strlen(CONFIG_SYNO_SATA_DOM_MODEL))) {
				blIsSynoboot = true;
			} else if (!strncmp(CONFIG_SYNO_SATA_DOM_VENDOR_SECOND_SRC, sdp->vendor, strlen(CONFIG_SYNO_SATA_DOM_VENDOR_SECOND_SRC))
				&& !strncmp(CONFIG_SYNO_SATA_DOM_MODEL_SECOND_SRC, sdp->model, strlen(CONFIG_SYNO_SATA_DOM_MODEL_SECOND_SRC))) {
				blIsSynoboot = true;
			}
#ifdef MY_ABC_HERE
			if (blIsSynoboot && !syno_find_synoboot()) {
				return SYNO_DISK_SYNOBOOT;
			}
#endif  
		}
#endif  
		 
#ifdef MY_DEF_HERE
		if (sdp->host->is_nvc_ssd) {
			return SYNO_DISK_CACHE;
		}
#endif  
		return SYNO_DISK_SATA;
	}
	 
	if (SYNO_PORT_TYPE_SAS == sdp->host->hostt->syno_port_type) {
		return SYNO_DISK_SAS;
	}
	return SYNO_DISK_UNKNOWN;
}
#endif  

#ifdef CONFIG_SYNO_PORT_MAPPING_V2
extern int syno_pciepath_dts_pattern_get(struct pci_dev *pdev, char *szPciePath, const int size);
static void syno_pciepath_enum(struct device *dev, char *buf) {
	struct pci_dev *pdev = NULL;
	char sztemp[SYNO_DTS_PROPERTY_CONTENT_LENGTH] = {'\0'};

	if (NULL == buf || NULL == dev) {
		return;
	}
	pdev = to_pci_dev(dev);

	if (-1 == syno_pciepath_dts_pattern_get(pdev, sztemp, sizeof(sztemp))) {
		return;
	}

	if (NULL != sztemp) {
		snprintf(buf, BLOCK_INFO_SIZE, "%spciepath=%s\n", buf, sztemp);
	}
}

static int get_ata_port_property_string(struct ata_port *ap, const char *prop_name, const char **prop_value)
{
	struct device_node *slot_node = NULL;
	int ret = -EINVAL;

	if (!prop_name || !prop_value || !ap ) {
		printk(KERN_INFO "Bad parameter.\n");
		goto END;
	}

	if (!ap->ops || !ap->ops->syno_compare_node_info) {
		printk(KERN_INFO "Operation not available: syno_compare_node_info.\n");
		goto END;
	}

	for_each_child_of_node(of_root, slot_node) {
		if (ap->ops->syno_compare_node_info(ap, slot_node)) {
			break;
		}
	}
	if (!slot_node) {
		printk(KERN_ERR "Cannot find slot node of this ata_port.\n");
		goto END;
	}

	ret = of_property_read_string(slot_node, prop_name, prop_value);
	of_node_put(slot_node);
END:
	return ret;
}

static void syno_ata_info_enum(struct ata_port *ap, struct scsi_device *sdev) {
	struct ata_device *dev = NULL;
	const char* form_factor = NULL;

	if (NULL == ap || NULL == sdev || NULL == ap->host) {
		return;
	}
	dev = ata_scsi_find_dev(ap, sdev);

	snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%sata_port_no=%u\n", sdev->syno_block_info, ap->port_no);
#ifdef MY_ABC_HERE
	if (syno_is_synology_pm(ap) && NULL != dev && NULL != dev->link) {
		snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%sis_syno_pmp=1\n", sdev->syno_block_info);
		snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%spmp_link=%u\n", sdev->syno_block_info, dev->link->pmp);

		if (IS_SYNOLOGY_RX4(ap->PMSynoUnique)) {
			snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%sunique=%s\n", sdev->syno_block_info, EBOX_INFO_UNIQUE_RX4);
		} else if (IS_SYNOLOGY_DX5(ap->PMSynoUnique)) {
			snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%sunique=%s\n", sdev->syno_block_info, EBOX_INFO_UNIQUE_DX5);
		} else if (IS_SYNOLOGY_DX513(ap->PMSynoUnique)) {
			snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%sunique=%s\n", sdev->syno_block_info, EBOX_INFO_UNIQUE_DX513);
		} else if (IS_SYNOLOGY_DX213(ap->PMSynoUnique)) {
			snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%sunique=%s\n", sdev->syno_block_info, EBOX_INFO_UNIQUE_DX213);
		} else if (IS_SYNOLOGY_DX517(ap->PMSynoUnique)) {
			snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%sunique=%s\n", sdev->syno_block_info, EBOX_INFO_UNIQUE_DX517);
		} else if (IS_SYNOLOGY_RX413(ap->PMSynoUnique)) {
			snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%sunique=%s\n", sdev->syno_block_info, EBOX_INFO_UNIQUE_RX413);
		} else if (IS_SYNOLOGY_RX415(ap->PMSynoUnique)) {
			snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%sunique=%s\n", sdev->syno_block_info, EBOX_INFO_UNIQUE_RX415);
		} else if (IS_SYNOLOGY_RX418(ap->PMSynoUnique)) {
			snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%sunique=%s\n", sdev->syno_block_info, EBOX_INFO_UNIQUE_RX418);
		} else if (IS_SYNOLOGY_DXC(ap->PMSynoUnique)) {
			snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%sunique=%s\n", sdev->syno_block_info, EBOX_INFO_UNIQUE_DXC);
		} else if (IS_SYNOLOGY_RXC(ap->PMSynoUnique)) {
			snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%sunique=%s\n", sdev->syno_block_info, EBOX_INFO_UNIQUE_RXC);
		} else if (IS_SYNOLOGY_RX1214(ap->PMSynoUnique)) {
			snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%sunique=%s\n", sdev->syno_block_info, EBOX_INFO_UNIQUE_RX1214);
		} else if (IS_SYNOLOGY_RX1217(ap->PMSynoUnique)) {
			snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%sunique=%s\n", sdev->syno_block_info, EBOX_INFO_UNIQUE_RX1217);
		} else if (IS_SYNOLOGY_DX1215(ap->PMSynoUnique)) {
			snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%sunique=%s\n", sdev->syno_block_info, EBOX_INFO_UNIQUE_DX1215);
		}
	}
#endif  

	if (0 == get_ata_port_property_string(ap, DT_FORM_FACTOR, &form_factor)) {
		snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%sform_factor=%s\n", sdev->syno_block_info, form_factor);
	}
}
static void syno_usb_info_enum(struct scsi_device *sdev) {
	struct us_data *us = NULL;

	if (NULL == sdev) {
		return;
	}
	us = host_to_us(sdev->host);

	if (NULL == us || NULL == us->pusb_intf) {
		return;
	}

	snprintf(sdev->syno_block_info, BLOCK_INFO_SIZE, "%susb_path=%s\n", sdev->syno_block_info, dev_name(&us->pusb_dev->dev));
}
#endif  
 
static int sd_probe(struct device *dev)
{
	struct scsi_device *sdp = to_scsi_device(dev);
	struct scsi_disk *sdkp;
	struct gendisk *gd;
	int index;
	int error;
#if defined(MY_ABC_HERE) || defined(CONFIG_SYNO_PORT_MAPPING_V2)
#if defined(MY_ABC_HERE) || defined(CONFIG_SYNO_PORT_MAPPING_V2)
	struct ata_port *ap;
#endif  
#if defined(MY_ABC_HERE) && defined(MY_ABC_HERE)
	int start_index;
#endif  
	u32 want_idx = 0;
#ifdef MY_DEF_HERE
	u32 cache_idx = 0;
#endif  
#endif  
#if defined(CONFIG_SYNO_SAS_DISK_NAME) || defined(CONFIG_SYNO_PORT_MAPPING_V2)
	u32 synoidx;
#endif  
#if defined(CONFIG_SYNO_SAS_DISK_NAME) || (defined(CONFIG_SYNO_PORT_MAPPING_V2) && defined(CONFIG_SYNO_SAS))
	struct device *searchDev = dev;
	int i = 0;
#endif  
#ifdef MY_ABC_HERE
	int iRetry = 0;
#endif  

	scsi_autopm_get_device(sdp);
	error = -ENODEV;
	if (sdp->type != TYPE_DISK && sdp->type != TYPE_MOD && sdp->type != TYPE_RBC)
		goto out;

	SCSI_LOG_HLQUEUE(3, sdev_printk(KERN_INFO, sdp,
					"sd_probe\n"));

	error = -ENOMEM;
	sdkp = kzalloc(sizeof(*sdkp), GFP_KERNEL);
	if (!sdkp)
		goto out;

	gd = alloc_disk(SD_MINORS);
	if (!gd)
		goto out_free;
#if defined(MY_ABC_HERE) || defined(CONFIG_SYNO_PORT_MAPPING_V2)
	sdkp->synodisktype = syno_disk_type_get(dev);
#endif  
	do {
		if (!ida_pre_get(&sd_index_ida, GFP_KERNEL))
			goto out_put;
#ifdef MY_DEF_HERE
		if (SYNO_DISK_CACHE == sdkp->synodisktype) {
			if (!ida_pre_get(&cache_index_ida, GFP_KERNEL))
			goto out_put;
		}
#endif  

#ifdef CONFIG_SYNO_PORT_MAPPING_V2
		switch (sdkp->synodisktype) {
			case SYNO_DISK_ISCSI:
				if (!ida_pre_get(&iscsi_index_ida, GFP_KERNEL))
					goto out_put;
				break;
			case SYNO_DISK_USB:
				if (!ida_pre_get(&usb_index_ida, GFP_KERNEL))
					goto out_put;
				break;
			case SYNO_DISK_SATA:
				if (!ida_pre_get(&sata_index_ida, GFP_KERNEL))
					goto out_put;
				break;
#ifdef CONFIG_SYNO_SAS
			case SYNO_DISK_SAS:
				if (1 == g_is_sas_model) {
					if (!ida_pre_get(&sas_index_ida, GFP_KERNEL))
						goto out_put;
				}
				break;
#endif  
			default:
				break;
		}
#else  
#ifdef CONFIG_SYNO_SAS_DISK_NAME
		if (1 == g_is_sas_model) {
			 
			switch (sdkp->synodisktype) {
#ifdef CONFIG_SYNO_SAS_DISK_NAME_ISCSI_DEVICE
				case SYNO_DISK_ISCSI:
					if (!ida_pre_get(&iscsi_index_ida, GFP_KERNEL))
						goto out_put;
					break;
#endif  
				case SYNO_DISK_USB:
					if (!ida_pre_get(&usb_index_ida, GFP_KERNEL))
						goto out_put;
					break;
				case SYNO_DISK_SAS:
					if (!ida_pre_get(&sas_index_ida, GFP_KERNEL))
						goto out_put;
					break;
				default:
					break;
			}
		}
#endif  
#endif  

#ifdef MY_ABC_HERE
		sdp->idle = jiffies;
		sdp->nospindown = 0;
		sdp->spindown = 0;
#endif  

		spin_lock(&sd_index_lock);

#ifdef CONFIG_SYNO_PORT_MAPPING_V2
		switch (sdkp->synodisktype) {
#ifdef MY_ABC_HERE
			case SYNO_DISK_ISCSI:
				error = syno_ida_get_new(&iscsi_index_ida, 0, &synoidx);
				want_idx = 0;
				break;
#endif  
#ifdef MY_ABC_HERE
			case SYNO_DISK_SYNOBOOT:
				want_idx = CONFIG_SYNO_USB_FLASH_DEVICE_INDEX;
				break;
#endif  
			case SYNO_DISK_USB:
				error = syno_ida_get_new(&usb_index_ida, 0, &synoidx);
				want_idx = 0;
				break;
			case SYNO_DISK_SATA:
				error = syno_ida_get_new(&sata_index_ida, 0, &synoidx);
				want_idx = 0;
				break;
#ifdef CONFIG_SYNO_SAS
			case SYNO_DISK_SAS:
				if (1 == g_is_sas_model) {
					error = syno_ida_get_new(&sas_index_ida, 0, &synoidx);
					want_idx = 0;
				}
				break;
#endif  
			default:
				break;	

		}

		error = syno_ida_get_new(&sd_index_ida, want_idx, &index);
        sdkp->synoindex = synoidx;

#elif defined(MY_ABC_HERE)  
		switch (sdkp->synodisktype) {
#ifdef MY_ABC_HERE
			case SYNO_DISK_ISCSI:
#ifdef CONFIG_SYNO_SAS_DISK_NAME
				if (1 == g_is_sas_model) {
					error = syno_ida_get_new(&iscsi_index_ida, 0, &synoidx);
					want_idx = 0;
					break;
				}
#endif  
				want_idx = SYNO_ISCSI_DEVICE_INDEX;
				break;
#endif  
#ifdef MY_ABC_HERE
			case SYNO_DISK_SYNOBOOT:
				want_idx = CONFIG_SYNO_USB_FLASH_DEVICE_INDEX;
				break;
#endif  
			case SYNO_DISK_USB:
#ifdef CONFIG_SYNO_SAS_DISK_NAME
				if (1 == g_is_sas_model) {
					error = syno_ida_get_new(&usb_index_ida, 0, &synoidx);
					want_idx = 0;
					break;
				}
#endif  
				want_idx = CONFIG_SYNO_MAX_INTERNAL_DISK + 1;
				break;
#ifdef MY_DEF_HERE
			case SYNO_DISK_CACHE:
				if (sdp->host->hostt->syno_index_get) {
					want_idx = sdp->host->hostt->syno_index_get(
							sdp->host,
							sdp->channel,
							sdp->id,
							sdp->lun);
				} else {
					want_idx = sdp->host->host_no;
				}
				break;
#endif  
#ifdef CONFIG_SYNO_REMAP_SATA_TO_SYS_ONLY_DEV
			case SYNO_DISK_SYSTEM:
				if (sdp->host->hostt->syno_index_get) {
					want_idx = sdp->host->hostt->syno_index_get(sdp->host,
																sdp->channel,
																sdp->id,
																sdp->lun);
				}
				break;
#endif  
			case SYNO_DISK_SAS:
			case SYNO_DISK_SATA:
			default:
#ifdef CONFIG_SYNO_SAS_DISK_NAME
				if (1 == g_is_sas_model) {
					error = syno_ida_get_new(&sas_index_ida, 0, &synoidx);
					want_idx = 0;
					break;
				}
#endif  
				if (sdp->host->hostt->syno_index_get) {
#if defined(MY_DEF_HERE) || defined(CONFIG_SYNO_NEXTKVMX64)
					if (_isCDSM_()) {
						want_idx = (0 < sdp->lun ? sdp->lun - 1 : 0);
					} else {
#endif  
						want_idx = sdp->host->hostt->syno_index_get(sdp->host, sdp->channel, sdp->id, sdp->lun);
#if defined(MY_DEF_HERE) || defined(CONFIG_SYNO_NEXTKVMX64)
					}
#endif  
				} else {
					want_idx = sdp->host->host_no;
				}
				break;
		}

		error = syno_ida_get_new(&sd_index_ida, want_idx, &index);
#ifdef CONFIG_SYNO_SAS_DISK_NAME
		if ((1 == g_is_sas_model)
#ifdef MY_DEF_HERE
			&& (SYNO_DISK_CACHE != sdkp->synodisktype)
#endif  
		) {
			sdkp->synoindex = synoidx;
			goto SYNO_SKIP_WANT_RETRY;
		}
#endif  

#ifdef MY_DEF_HERE
		if (SYNO_DISK_CACHE == sdkp->synodisktype) {
			error = syno_ida_get_new(&cache_index_ida, (want_idx - M2SATA_START_IDX), &cache_idx);
			sdkp->synoindex = cache_idx;
		}
#endif  

		while (want_idx != index &&
			(SYNO_DISK_SATA == sdkp->synodisktype) && iRetry < 15) {
			 
			printk("want_idx %d index %d. delay and reget\n", want_idx, index);

			ida_remove(&sd_index_ida, index);
			spin_unlock(&sd_index_lock);

			schedule_timeout_uninterruptible(HZ);

			spin_lock(&sd_index_lock);
			error = syno_ida_get_new(&sd_index_ida, want_idx, &index);

			printk("want_idx %d index %d\n", want_idx, index);
			iRetry++;
		}
#ifdef CONFIG_SYNO_SAS_DISK_NAME
SYNO_SKIP_WANT_RETRY:
#endif  

#else  
		error = ida_get_new(&sd_index_ida, &index);
#endif  

		spin_unlock(&sd_index_lock);
	} while (error == -EAGAIN);

	if (error) {
		sdev_printk(KERN_WARNING, sdp, "sd_probe: memory exhausted.\n");
		goto out_put;
	}

#ifdef CONFIG_SYNO_PORT_MAPPING_V2
	gd->systemDisk = 0;
	switch (sdkp->synodisktype) {
#ifdef MY_ABC_HERE
		case SYNO_DISK_ISCSI:
			error = syno_sd_format_numeric_disk_name(CONFIG_SYNO_ISCSI_DEVICE_NEW_PREFIX, synoidx, gd->disk_name, DISK_NAME_LEN);
			printk("got iSCSI disk[%d]\n", synoidx);
			break;
#endif  
#ifdef MY_ABC_HERE
		case SYNO_DISK_SYNOBOOT:
			 
			sprintf(gd->disk_name, CONFIG_SYNO_USB_FLASH_DEVICE_NAME);
			error = 0;
			break;
#endif  
#ifdef CONFIG_SYNO_SAS
		case SYNO_DISK_SAS:
			error = syno_sd_format_numeric_disk_name(CONFIG_SYNO_SAS_DEVICE_PREFIX, synoidx, gd->disk_name, DISK_NAME_LEN);
			for (i = 0; i < SCSI_HOST_SEARCH_DEPTH && NULL != searchDev; i++) {
				if (scsi_is_host_device(searchDev)) {
					gd->systemDisk = 1;
					break;
				}
				searchDev = searchDev->parent;
			}
			break;
#endif  
		case SYNO_DISK_SATA:
			ap = ata_shost_to_port(sdp->host);
			 
			if (NULL != ap
#ifdef MY_ABC_HERE
					&& !syno_is_synology_pm(ap)
#endif  
					) {
				gd->systemDisk = 1;
				gd->syno_slot_index = ap->syno_disk_index + 1;
			}
			error = syno_sd_format_numeric_disk_name(CONFIG_SYNO_SATA_DEVICE_NEW_PREFIX, synoidx, gd->disk_name, DISK_NAME_LEN);
			if (NULL != ap) {
				 
				if (ap->dev->bus && !strcmp("pci", ap->dev->bus->name)) {
					syno_pciepath_enum(ap->dev, sdp->syno_block_info);
				}
				syno_ata_info_enum(ap, sdp);
			}
			printk("got SATA disk[%d]\n", synoidx);
			break;
		case SYNO_DISK_USB:
		default:
			syno_usb_info_enum(sdp);
			error = syno_sd_format_numeric_disk_name(CONFIG_SYNO_USB_DEVICE_NEW_PREFIX, synoidx, gd->disk_name, DISK_NAME_LEN);
			break;
	}
#elif defined(MY_ABC_HERE)  
	gd->systemDisk = 0;
	switch (sdkp->synodisktype) {
#ifdef MY_ABC_HERE
		case SYNO_DISK_ISCSI:
#ifdef CONFIG_SYNO_SAS_DISK_NAME
			if (1 == g_is_sas_model) {
				error = syno_sd_format_numeric_disk_name(CONFIG_SYNO_SAS_ISCSI_DEVICE_PREFIX, synoidx, gd->disk_name, DISK_NAME_LEN);
				printk("got iSCSI disk[%d]\n", synoidx);
				break;
			}
#endif  
			start_index = index - SYNO_ISCSI_DEVICE_INDEX;
			error = sd_format_disk_name(CONFIG_SYNO_ISCSI_DEVICE_PREFIX, start_index, gd->disk_name, DISK_NAME_LEN);
			printk("got iSCSI disk[%d]\n", start_index);
			break;
#endif  
#ifdef MY_ABC_HERE
		case SYNO_DISK_SYNOBOOT:
			 
			sprintf(gd->disk_name, CONFIG_SYNO_USB_FLASH_DEVICE_NAME);
			error = 0;
			break;
#endif  

		case SYNO_DISK_SAS:
#ifdef CONFIG_SYNO_SAS_DISK_NAME
			error = syno_sd_format_numeric_disk_name(CONFIG_SYNO_SAS_DEVICE_PREFIX, synoidx, gd->disk_name, DISK_NAME_LEN);
			for (i = 0; i < SCSI_HOST_SEARCH_DEPTH && NULL != searchDev; i++) {
				if (scsi_is_host_device(searchDev)) {
					gd->systemDisk = 1;
					break;
				}
				searchDev = searchDev->parent;
			}
#endif  
			break;
#ifdef CONFIG_SYNO_REMAP_SATA_TO_SYS_ONLY_DEV
		case SYNO_DISK_SYSTEM:
			gd->systemDisk = 1;
			synoidx = index - SYSTEM_DEVICE_START_IDX;
			error = syno_sd_format_numeric_disk_name(
						CONFIG_SYNO_SATA_SYSTEM_DEVICE_PREFIX, synoidx,
						gd->disk_name, DISK_NAME_LEN);
			break;
#endif  
		case SYNO_DISK_SATA:
#ifdef MY_ABC_HERE
			ap = ata_shost_to_port(sdp->host);
			 
			if (NULL != ap && !syno_is_synology_pm(ap)) {
#endif  
			gd->systemDisk = 1;
#ifdef MY_ABC_HERE
			}
#endif  
			error = sd_format_disk_name(CONFIG_SYNO_SATA_DEVICE_PREFIX, index, gd->disk_name, DISK_NAME_LEN);
			break;
#ifdef MY_DEF_HERE
		case SYNO_DISK_CACHE:
			error = syno_sd_format_numeric_disk_name(CONFIG_SYNO_CACHE_DEVICE_PREFIX, cache_idx, gd->disk_name, DISK_NAME_LEN);
			break;
#endif  
		case SYNO_DISK_USB:
		default:
#ifdef CONFIG_SYNO_SAS_DISK_NAME
			if (1 == g_is_sas_model) {
				error = syno_sd_format_numeric_disk_name(CONFIG_SYNO_SAS_USB_DEVICE_PREFIX, synoidx, gd->disk_name, DISK_NAME_LEN);
				break;
			}
#endif  
			error = sd_format_disk_name(CONFIG_SYNO_SATA_DEVICE_PREFIX, index, gd->disk_name, DISK_NAME_LEN);
			break;
	}
#else
	error = sd_format_disk_name("sd", index, gd->disk_name, DISK_NAME_LEN);
#endif  

	if (error) {
		sdev_printk(KERN_WARNING, sdp, "SCSI disk (sd) name length exceeded.\n");
		goto out_free_index;
	}

	sdkp->device = sdp;
	sdkp->driver = &sd_template;
	sdkp->disk = gd;
	sdkp->index = index;
	atomic_set(&sdkp->openers, 0);
	atomic_set(&sdkp->device->ioerr_cnt, 0);

	if (!sdp->request_queue->rq_timeout) {
		if (sdp->type != TYPE_MOD)
			blk_queue_rq_timeout(sdp->request_queue, SD_TIMEOUT);
		else
			blk_queue_rq_timeout(sdp->request_queue,
					     SD_MOD_TIMEOUT);
	}

	device_initialize(&sdkp->dev);
	sdkp->dev.parent = dev;
	sdkp->dev.class = &sd_disk_class;
	dev_set_name(&sdkp->dev, "%s", dev_name(dev));

	error = device_add(&sdkp->dev);
	if (error)
		goto out_free_index;

	get_device(dev);
	dev_set_drvdata(dev, sdkp);

	get_device(&sdkp->dev);	 
	async_schedule_domain(sd_probe_async, sdkp, &scsi_sd_probe_domain);
#if defined(MY_ABC_HERE) || defined(CONFIG_SYNO_PORT_MAPPING_V2)
	strlcpy(sdp->syno_disk_name, gd->disk_name, BDEVNAME_SIZE);
#endif  

	return 0;

 out_free_index:
	spin_lock(&sd_index_lock);
	ida_remove(&sd_index_ida, index);
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
	switch (sdkp->synodisktype) {
		case SYNO_DISK_ISCSI:
			ida_remove(&iscsi_index_ida, synoidx);
			break;
		case SYNO_DISK_SATA:
			ida_remove(&sata_index_ida, synoidx);
			break;
		case SYNO_DISK_USB:
			ida_remove(&usb_index_ida, synoidx);
			break;
#ifdef CONFIG_SYNO_SAS
		case SYNO_DISK_SAS:
			if (1 == g_is_sas_model) {
				ida_remove(&sas_index_ida, synoidx);
			}
			break;
#endif  
		default:
			break;
	}
#else  
#ifdef CONFIG_SYNO_SAS_DISK_NAME
	if (1 == g_is_sas_model) {
		switch (sdkp->synodisktype) {
#ifdef CONFIG_SYNO_SAS_DISK_NAME_ISCSI_DEVICE
			case SYNO_DISK_ISCSI:
				ida_remove(&iscsi_index_ida, synoidx);
				break;
#endif  
			case SYNO_DISK_USB:
				ida_remove(&usb_index_ida, synoidx);
				break;
			case SYNO_DISK_SAS:
				ida_remove(&sas_index_ida, synoidx);
				break;
			default:
				break;
		}
	}
#endif  
#endif  
#ifdef MY_DEF_HERE
	if (SYNO_DISK_CACHE == sdkp->synodisktype) {
		ida_remove(&cache_index_ida, cache_idx);
	}
#endif  
	spin_unlock(&sd_index_lock);
 out_put:
	put_disk(gd);
 out_free:
	kfree(sdkp);
 out:
	scsi_autopm_put_device(sdp);
	return error;
}

static int sd_remove(struct device *dev)
{
	struct scsi_disk *sdkp;
	dev_t devt;

	sdkp = dev_get_drvdata(dev);
	devt = disk_devt(sdkp->disk);
	scsi_autopm_get_device(sdkp->device);

	async_synchronize_full_domain(&scsi_sd_pm_domain);
	async_synchronize_full_domain(&scsi_sd_probe_domain);
	device_del(&sdkp->dev);
	del_gendisk(sdkp->disk);
	sd_shutdown(dev);

	blk_register_region(devt, SD_MINORS, NULL,
			    sd_default_probe, NULL, NULL);

	mutex_lock(&sd_ref_mutex);
	dev_set_drvdata(dev, NULL);
	put_device(&sdkp->dev);
	mutex_unlock(&sd_ref_mutex);

	return 0;
}

static void scsi_disk_release(struct device *dev)
{
	struct scsi_disk *sdkp = to_scsi_disk(dev);
	struct gendisk *disk = sdkp->disk;
	
	spin_lock(&sd_index_lock);
	ida_remove(&sd_index_ida, sdkp->index);
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
	switch (sdkp->synodisktype) {
		case SYNO_DISK_ISCSI:
			ida_remove(&iscsi_index_ida, sdkp->synoindex);
			break;
		case SYNO_DISK_USB:
			ida_remove(&usb_index_ida, sdkp->synoindex);
			break;
		case SYNO_DISK_SATA:
			ida_remove(&sata_index_ida, sdkp->synoindex);
			break;
#ifdef CONFIG_SYNO_SAS
		case SYNO_DISK_SAS:
			if (1 == g_is_sas_model) {
				ida_remove(&sas_index_ida, sdkp->synoindex);
			}
			break;
#endif  
		default:
			break;
	}
#else  
#ifdef CONFIG_SYNO_SAS_DISK_NAME
	if (1 == g_is_sas_model) {
		switch (sdkp->synodisktype) {
#ifdef CONFIG_SYNO_SAS_DISK_NAME_ISCSI_DEVICE
			case SYNO_DISK_ISCSI:
				ida_remove(&iscsi_index_ida, sdkp->synoindex);
				break;
#endif  
			case SYNO_DISK_USB:
				ida_remove(&usb_index_ida, sdkp->synoindex);
				break;
			case SYNO_DISK_SAS:
				ida_remove(&sas_index_ida, sdkp->synoindex);
				break;
			default:
				break;
		}
	}
#endif  
#endif  
#ifdef MY_DEF_HERE
	if (SYNO_DISK_CACHE == sdkp->synodisktype) {
		ida_remove(&cache_index_ida, sdkp->synoindex);
	}
#endif  
	spin_unlock(&sd_index_lock);

	disk->private_data = NULL;
	put_disk(disk);
	put_device(&sdkp->device->sdev_gendev);

	kfree(sdkp);
}

static int sd_start_stop_device(struct scsi_disk *sdkp, int start)
{
	unsigned char cmd[6] = { START_STOP };	 
	struct scsi_sense_hdr sshdr;
	struct scsi_device *sdp = sdkp->device;
	int res;

	if (start)
		cmd[4] |= 1;	 

	if (sdp->start_stop_pwr_cond)
		cmd[4] |= start ? 1 << 4 : 3 << 4;	 

	if (!scsi_device_online(sdp))
		return -ENODEV;

	res = scsi_execute_req_flags(sdp, cmd, DMA_NONE, NULL, 0, &sshdr,
			       SD_TIMEOUT, SD_MAX_RETRIES, NULL, REQ_PM);
	if (res) {
		sd_print_result(sdkp, "Start/Stop Unit failed", res);
		if (driver_byte(res) & DRIVER_SENSE)
			sd_print_sense_hdr(sdkp, &sshdr);
		if (scsi_sense_valid(&sshdr) &&
			 
			sshdr.asc == 0x3a)
			res = 0;
	}

	if (res)
		return -EIO;

	return 0;
}

static void sd_shutdown(struct device *dev)
{
	struct scsi_disk *sdkp = dev_get_drvdata(dev);

	if (!sdkp)
		return;          

	if (pm_runtime_suspended(dev))
		return;

	if (sdkp->WCE && sdkp->media_present) {
		sd_printk(KERN_NOTICE, sdkp, "Synchronizing SCSI cache\n");
		sd_sync_cache(sdkp);
	}

	if (system_state != SYSTEM_RESTART && sdkp->device->manage_start_stop) {
		sd_printk(KERN_NOTICE, sdkp, "Stopping disk\n");
		sd_start_stop_device(sdkp, 0);
	}
}

static int sd_suspend_common(struct device *dev, bool ignore_stop_errors)
{
	struct scsi_disk *sdkp = dev_get_drvdata(dev);
	int ret = 0;

	if (!sdkp)	 
		return 0;

	if (sdkp->WCE && sdkp->media_present) {
		sd_printk(KERN_NOTICE, sdkp, "Synchronizing SCSI cache\n");
		ret = sd_sync_cache(sdkp);
		if (ret) {
			 
			if (ret == -ENODEV)
				ret = 0;
			goto done;
		}
	}

	if (sdkp->device->manage_start_stop) {
		sd_printk(KERN_NOTICE, sdkp, "Stopping disk\n");
		 
		ret = sd_start_stop_device(sdkp, 0);
		if (ignore_stop_errors)
			ret = 0;
	}

done:
	return ret;
}

static int sd_suspend_system(struct device *dev)
{
	return sd_suspend_common(dev, true);
}

static int sd_suspend_runtime(struct device *dev)
{
	return sd_suspend_common(dev, false);
}

static int sd_resume(struct device *dev)
{
	struct scsi_disk *sdkp = dev_get_drvdata(dev);

	if (!sdkp)	 
		return 0;

	if (!sdkp->device->manage_start_stop)
		return 0;

	sd_printk(KERN_NOTICE, sdkp, "Starting disk\n");
	return sd_start_stop_device(sdkp, 1);
}

#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY
struct workqueue_struct *spinup_workqueue = NULL;
#endif  
 
static int __init init_sd(void)
{
	int majors = 0, i, err;

	SCSI_LOG_HLQUEUE(3, printk("init_sd: sd driver entry point\n"));

	for (i = 0; i < SD_MAJORS; i++) {
		if (register_blkdev(sd_major(i), "sd") != 0)
			continue;
		majors++;
		blk_register_region(sd_major(i), SD_MINORS, NULL,
				    sd_default_probe, NULL, NULL);
	}

#ifdef MY_ABC_HERE
	gBadSectorTest = 0;
#endif  

	if (!majors)
		return -ENODEV;

	err = class_register(&sd_disk_class);
	if (err)
		goto err_out;

	sd_cdb_cache = kmem_cache_create("sd_ext_cdb", SD_EXT_CDB_SIZE,
					 0, 0, NULL);
	if (!sd_cdb_cache) {
		printk(KERN_ERR "sd: can't init extended cdb cache\n");
		err = -ENOMEM;
		goto err_out_class;
	}

	sd_cdb_pool = mempool_create_slab_pool(SD_MEMPOOL_SIZE, sd_cdb_cache);
	if (!sd_cdb_pool) {
		printk(KERN_ERR "sd: can't init extended cdb pool\n");
		err = -ENOMEM;
		goto err_out_cache;
	}

	err = scsi_register_driver(&sd_template.gendrv);
	if (err)
		goto err_out_driver;

#ifdef CONFIG_SYNO_SAS_SPINUP_DELAY
	spinup_workqueue = create_workqueue("spinup_wq");
	if (NULL == spinup_workqueue) {
		printk(KERN_ERR "sd: can't init spinup_wq, fall back to global queue\n");
	}
#endif  

	return 0;

err_out_driver:
	mempool_destroy(sd_cdb_pool);

err_out_cache:
	kmem_cache_destroy(sd_cdb_cache);

err_out_class:
	class_unregister(&sd_disk_class);
err_out:
	for (i = 0; i < SD_MAJORS; i++)
		unregister_blkdev(sd_major(i), "sd");
	return err;
}

static void __exit exit_sd(void)
{
	int i;

	SCSI_LOG_HLQUEUE(3, printk("exit_sd: exiting sd driver\n"));

	scsi_unregister_driver(&sd_template.gendrv);
	mempool_destroy(sd_cdb_pool);
	kmem_cache_destroy(sd_cdb_cache);

	class_unregister(&sd_disk_class);

	for (i = 0; i < SD_MAJORS; i++) {
		blk_unregister_region(sd_major(i), SD_MINORS);
		unregister_blkdev(sd_major(i), "sd");
	}
}

module_init(init_sd);
module_exit(exit_sd);

static void sd_print_sense_hdr(struct scsi_disk *sdkp,
			       struct scsi_sense_hdr *sshdr)
{
	scsi_print_sense_hdr(sdkp->device,
			     sdkp->disk ? sdkp->disk->disk_name : NULL, sshdr);
}

static void sd_print_result(const struct scsi_disk *sdkp, const char *msg,
			    int result)
{
	const char *hb_string = scsi_hostbyte_string(result);
	const char *db_string = scsi_driverbyte_string(result);

	if (hb_string || db_string)
		sd_printk(KERN_INFO, sdkp,
			  "%s: Result: hostbyte=%s driverbyte=%s\n", msg,
			  hb_string ? hb_string : "invalid",
			  db_string ? db_string : "invalid");
	else
		sd_printk(KERN_INFO, sdkp,
			  "%s: Result: hostbyte=0x%02x driverbyte=0x%02x\n",
			  msg, host_byte(result), driver_byte(result));
}

#ifdef MY_ABC_HERE
int SynoSCSIGetDeviceIndex(struct block_device *bdev)
{
	struct gendisk *disk = NULL;

	BUG_ON(bdev == NULL);
	disk = bdev->bd_disk;

#if defined(CONFIG_SYNO_SAS_DISK_NAME_ISCSI_DEVICE)
	if (g_is_sas_model) {
		return container_of(disk->private_data, struct scsi_disk, driver)->synoindex;
	}
#endif  
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
	return container_of(disk->private_data, struct scsi_disk, driver)->synoindex;
#else  
	return container_of(disk->private_data, struct scsi_disk, driver)->index;
#endif  
}
EXPORT_SYMBOL(SynoSCSIGetDeviceIndex);
#endif  

#if defined(MY_ABC_HERE) || defined(MY_ABC_HERE)
 
static unsigned char
blIsScsiDevice(int major)
{
	unsigned char ret = 0;

	switch (major) {
	case SCSI_DISK0_MAJOR:
	case SCSI_DISK1_MAJOR ... SCSI_DISK7_MAJOR:
	case SCSI_DISK8_MAJOR ... SCSI_DISK15_MAJOR:
		ret = 1;
		break;
	default:
		break;
	}

	return ret;
}
#endif  

#ifdef MY_ABC_HERE
int
IsDeviceDisappear(struct block_device *bdev)
{
	struct gendisk *disk = NULL;
	struct scsi_disk *sdkp;
	int ret = 0;

	if (!bdev) {
		WARN_ON(bdev == NULL);
		goto END;
	}

	disk = bdev->bd_disk;
	if (!disk) {
		WARN_ON(disk == NULL);
		goto END;
	}

	if (!blIsScsiDevice(disk->major)) {
		 
		printk("This is not a kind of scsi disk %d\n", disk->major);
		goto END;
	}

	sdkp = container_of(disk->private_data, struct scsi_disk, driver);
	if (!sdkp) {
		WARN_ON(!sdkp);
		goto END;
	}

	switch (sdkp->device->sdev_state) {
	case SDEV_OFFLINE:
	case SDEV_DEL:
	case SDEV_CANCEL:
		ret = 1;
		break;
	default:
		break;
	}

END:
	return ret;
}

EXPORT_SYMBOL(IsDeviceDisappear);
#endif  

#ifdef MY_ABC_HERE
 
void
PartitionRemapModeSet(struct gendisk *gd,
					  struct hd_struct *phd,
					  unsigned char blAutoRemap)
{
	struct scsi_disk *sdkp;
	struct scsi_device *sdev;

	if (!gd || !phd) {
		goto END;
	}

	phd->auto_remap = blAutoRemap;
	if (!blAutoRemap) {
		if (!blIsScsiDevice(gd->major)) {
			 
			printk("This is not a kind of scsi disk %d\n", gd->major);
			goto END;
		}

		sdkp = container_of(gd->private_data, struct scsi_disk, driver);
		if (!sdkp) {
			printk(" sdkp is NULL\n");
			goto END;
		}

		sdev = sdkp->device;
		if(!sdev) {
			printk(" sdev is NULL\n");
			goto END;
		}
		sdev->auto_remap = 0;
	}
END:
	return;
}

void
ScsiRemapModeSet(struct scsi_device *sdev,
				 unsigned char blAutoRemap)
{
	struct scsi_disk *sdkp;
	struct gendisk *gd;
	struct hd_struct *phd;
	int i = 0;

	if (!sdev) {
		goto END;
	}

	if (TYPE_DISK != sdev->type) {
		printk("Only support scsi disk\n");
		goto END;
	}

	sdev->auto_remap = blAutoRemap;
	sdkp = dev_get_drvdata(&sdev->sdev_gendev);
	if (!sdkp) {
		goto END;
	}

	gd = sdkp->disk;
	if (!gd) {
		goto END;
	}

	for (i = 0; i < gd->minors; i++) {
		phd = disk_get_part(gd, i+1);
		if (!phd || !phd->nr_sects)
			continue;

		phd->auto_remap = blAutoRemap;
	}
END:
	return;
}

void
RaidRemapModeSet(struct block_device *bdev, unsigned char blAutoRemap)
{
	struct gendisk *disk = NULL;
	struct scsi_disk *sdkp;

	if (!bdev) {
		WARN_ON(bdev == NULL);
		return;
	}

	disk = bdev->bd_disk;
	if (!disk) {
		WARN_ON(disk == NULL);
		return;
	}

	if (!blIsScsiDevice(disk->major)) {
		 
		printk("This is not a kind of scsi disk %d\n", disk->major);
		return;
	}

	if (bdev->bd_part) {
		 
		bdev->bd_part->auto_remap = blAutoRemap;
	} else {
		 
		sdkp = container_of(disk->private_data, struct scsi_disk, driver);
		if (!sdkp) {
			WARN_ON(!sdkp);
			return;
		}
		ScsiRemapModeSet(sdkp->device, blAutoRemap);
	}
}

unsigned char
blSectorNeedAutoRemap(struct scsi_cmnd *scsi_cmd,
					  sector_t lba)
{
	struct scsi_device *sdev;
	struct scsi_disk *sdkp;
	struct gendisk *gd;
	struct hd_struct *phd;
	char szName[BDEVNAME_SIZE];
	sector_t start, end;
	u8 ret = 0;
	int i = 0;

	if (!scsi_cmd) {
		WARN_ON(1);
		goto END;
	}

	sdev = scsi_cmd->device;
	if (!sdev) {
		WARN_ON(1);
		goto END;
	}

	if (TYPE_DISK != sdev->type) {
		printk("Only support scsi disk\n");
		goto END;
	}

	if (sdev->auto_remap) {
		ret = 1;
		printk("%s auto remap is on\n", dev_name(&sdev->sdev_gendev));
		goto END;
	}

	sdkp = dev_get_drvdata(&sdev->sdev_gendev);
	if (!sdkp) {
		goto END;
	}

	gd = sdkp->disk;
	if (!gd) {
		goto END;
	}

	for (i = 0; i < gd->minors; i++) {
		phd = disk_get_part(gd, i+1);
		if (!phd || !phd->nr_sects)
			continue;

		start = phd->start_sect;
		end = phd->nr_sects + start - 1;

		if (lba >= start && lba <= end) {
			printk("lba %llu start %llu end %llu\n", (unsigned long long)lba, (unsigned long long)start, (unsigned long long)end);
			ret = phd->auto_remap;
			printk("%s auto_remap %u\n", disk_name(gd, i+1, szName), phd->auto_remap);
		}
	}
END:
	return ret;
}

EXPORT_SYMBOL(blSectorNeedAutoRemap);
EXPORT_SYMBOL(RaidRemapModeSet);
EXPORT_SYMBOL(ScsiRemapModeSet);
EXPORT_SYMBOL(PartitionRemapModeSet);
#endif  
