#ifndef MY_ABC_HERE
#define MY_ABC_HERE
#endif
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/spinlock.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/suspend.h>
#include <linux/workqueue.h>
#include <linux/scatterlist.h>
#include <linux/io.h>
#include <linux/async.h>
#include <linux/log2.h>
#include <linux/slab.h>
#include <linux/glob.h>
#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_host.h>
#include <linux/libata.h>
#include <asm/byteorder.h>
#include <linux/cdrom.h>
#include <linux/ratelimit.h>
#include <linux/pm_runtime.h>
#include <linux/platform_device.h>

#ifdef MY_ABC_HERE
#include <linux/math64.h>
#endif  

#ifdef CONFIG_SYNO_PORT_MAPPING_V2
#include <linux/of.h>
#endif  

#if defined(CONFIG_SYNO_PORT_MAPPING_V2) || defined(CONFIG_SYNO_SMBUS_HDD_POWERCTL)
#include <linux/synolib.h>
#endif  

#define CREATE_TRACE_POINTS
#include <trace/events/libata.h>

#include "libata.h"
#include "libata-transport.h"

#ifdef MY_ABC_HERE
#include <linux/mutex.h>
static struct mutex mutex_spin;
static DEFINE_MUTEX(mutex_spin);
#endif  
#if defined(CONFIG_SYNO_LSP_RTD1619)
#ifdef CONFIG_AHCI_RTK
extern void rtk_sata_phy_poweron(struct ata_link *link);
#endif  
#endif  

#ifdef MY_ABC_HERE
static int syno_ata_pattern_check = 0;
extern int gSynoPatternCheckCharacter;
module_param(syno_ata_pattern_check, int, 0644);
#endif  

const unsigned long sata_deb_timing_normal[]		= {   5,  100, 2000 };
const unsigned long sata_deb_timing_hotplug[]		= {  25,  500, 2000 };
const unsigned long sata_deb_timing_long[]		= { 100, 2000, 5000 };

const struct ata_port_operations ata_base_port_ops = {
	.prereset		= ata_std_prereset,
	.postreset		= ata_std_postreset,
	.error_handler		= ata_std_error_handler,
	.sched_eh		= ata_std_sched_eh,
	.end_eh			= ata_std_end_eh,
};

const struct ata_port_operations sata_port_ops = {
	.inherits		= &ata_base_port_ops,

	.qc_defer		= ata_std_qc_defer,
	.hardreset		= sata_std_hardreset,
};

static unsigned int ata_dev_init_params(struct ata_device *dev,
					u16 heads, u16 sectors);
static unsigned int ata_dev_set_xfermode(struct ata_device *dev);
static void ata_dev_xfermask(struct ata_device *dev);
static unsigned long ata_dev_blacklisted(const struct ata_device *dev);

#ifdef CONFIG_SYNO_SMBUS_HDD_POWERCTL
extern long g_smbus_hdd_powerctl;
extern char gSynoSmbusHddType[16];
extern int gSynoSmbusHddAdapter;
extern int gSynoSmbusHddAddress;
extern SYNO_SMBUS_HDD_POWERCTL SynoSmbusHddPowerCtl;
#endif  
atomic_t ata_print_id = ATOMIC_INIT(0);

struct ata_force_param {
	const char	*name;
	unsigned int	cbl;
	int		spd_limit;
	unsigned long	xfer_mask;
	unsigned int	horkage_on;
	unsigned int	horkage_off;
	unsigned int	lflags;
};

struct ata_force_ent {
	int			port;
	int			device;
	struct ata_force_param	param;
};

static struct ata_force_ent *ata_force_tbl;
static int ata_force_tbl_size;

#ifdef CONFIG_SYNO_PORT_MAPPING_V2
struct klist syno_ata_port_head;
#endif  

#ifdef CONFIG_SYNO_PORT_MAPPING_V2
 
struct ata_port *syno_ata_port_get_by_port(const unsigned short diskPort)
{
	int index = -1;
	struct ata_port *ap = NULL;
	struct device_node *pSlotNode = NULL;
	struct klist_iter klist_iter;
	struct klist_node *ata_node = NULL;

	if (NULL == of_root) {
		goto END;
	}
	for_each_child_of_node(of_root, pSlotNode) {
		 
		if (!pSlotNode->full_name || 1 != sscanf(pSlotNode->full_name, "/"DT_INTERNAL_SLOT"@%d", &index)) {
			continue;
		}
		if (diskPort == index) {
			break;
		}
	}

	if (NULL == pSlotNode) {
		goto END;
	}

	klist_iter_init(&syno_ata_port_head, &klist_iter);
	for (ata_node = klist_next(&klist_iter); NULL != ata_node; ata_node = klist_next(&klist_iter)) {
		ap = container_of(ata_node, struct ata_port, ata_port_list);
		if (true == ap->ops->syno_compare_node_info(ap, pSlotNode)) {
			break;
		}
		ap = NULL;
	}
	klist_iter_exit(&klist_iter);

	of_node_put(pSlotNode);
END:
	return ap;
}
EXPORT_SYMBOL(syno_ata_port_get_by_port);

int syno_compare_dts_ata_port(const struct ata_port *pAtaPort, const struct device_node *pDeviceNode)
{
	int ret = -1;
	u32 ata_port_no = U32_MAX;
	if (NULL == pDeviceNode || NULL == pAtaPort) {
		goto END;
	}
	if (0 != of_property_read_u32_index(pDeviceNode, DT_ATA_PORT, 0, &ata_port_no)) {
		printk(KERN_ERR "%s: read ata_port error\n", __func__);
		goto END;
	}

	if (pAtaPort->port_no == ata_port_no) {
		ret = 0;
	}
END:
	return ret;
}
EXPORT_SYMBOL(syno_compare_dts_ata_port);
#endif  

#ifdef MY_ABC_HERE
#if defined(MY_ABC_HERE)
#include <linux/syno_gpio.h>
extern SYNO_GPIO syno_gpio;
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
#else
static SYNO_GPIO_INFO hdd_detect = {
	.name 			= "hdd detect",
	.nr_gpio		= 0,
	.gpio_port 		= {0},
	.gpio_polarity	= ACTIVE_LOW,
};
static SYNO_GPIO_INFO hdd_enable = {
	.name 			= "hdd enable",
	.nr_gpio		= 0,
	.gpio_port		= {0},
	.gpio_polarity	= ACTIVE_HIGH,
};
static SYNO_GPIO_INFO hdd_act_led = {
	.name 			= "hdd act led",
	.nr_gpio		= 0,
	.gpio_port		= {0},
	.gpio_polarity	= ACTIVE_HIGH,
};
 
static int hdd_bootargs_parser (SYNO_GPIO_INFO *gpio_info, char *args)
{
	int pin_num = 0;
	char *endp;
	if (NULL == args || NULL == gpio_info) {
		return -EINVAL;
	}
	while (1) {
		if(SYNO_GPIO_PIN_MAX_NUM < pin_num) {
			return -EMSGSIZE;
		}
		gpio_info->gpio_port[pin_num] = simple_strtol(args, &endp, 10) & 0xFF;
		pin_num++;
		 
		if (*endp == '\0') {
			break;
		}
		args = ++endp;
	}
	gpio_info->nr_gpio = pin_num;
	return 0;
}
static int __init early_hdd_detect(char *p)
{
	int i, ret;
	 
	if ((NULL == p) || (0 == strlen(p))) {
		return 1;
	}
	printk("SYNO GPIO hdd detect pin: ");
	ret = hdd_bootargs_parser(&hdd_detect, p);
	if (-EINVAL == ret) {
		printk("Error parameters, %s\n", p);
	} else if (-EMSGSIZE == ret) {
		printk("Too many arguments, %s\n", p);
	} else {
		syno_gpio.hdd_detect = &hdd_detect;
		for (i = 0; i < syno_gpio.hdd_detect->nr_gpio; i++) {
			printk("%d ", HDD_DETECT_PIN(i + 1));
		}
		printk("\n");
	}
	return 1;
}
__setup("syno_hdd_detect=", early_hdd_detect);
static int __init early_hdd_act_led(char *p)
{
	int i, ret;
	 
	if ((NULL == p) || (0 == strlen(p))) {
		return 1;
	}
	printk("SYNO GPIO hdd active led pin: ");
	ret = hdd_bootargs_parser(&hdd_act_led, p);
	if (-EINVAL == ret) {
		printk("Error parameters, %s\n", p);
	} else if (-EMSGSIZE == ret) {
		printk("Too many arguments, %s\n", p);
	} else {
		syno_gpio.hdd_act_led = &hdd_act_led;
		for (i = 0; i < syno_gpio.hdd_act_led->nr_gpio; i++) {
			printk("%d ", HDD_ACT_LED_PIN(i + 1));
		}
		printk("\n");
	}
	return 1;
}
__setup("syno_hdd_act_led=", early_hdd_act_led);
static int __init early_hdd_enable(char *p)
{
	int i, ret;
	 
	if ((NULL == p) || (0 == strlen(p))) {
		return 1;
	}
	printk("SYNO GPIO hdd enable pin: ");
	ret = hdd_bootargs_parser(&hdd_enable, p);
	if (-EINVAL == ret) {
		printk("Error parameters, %s\n", p);
	} else if (-EMSGSIZE == ret) {
		printk("Too many arguments, %s\n", p);
	} else {
		syno_gpio.hdd_enable = &hdd_enable;
		for (i = 0; i < syno_gpio.hdd_enable->nr_gpio; i++) {
			printk("%d ", HDD_ENABLE_PIN(i + 1));
		}
		printk("\n");
	}
	return 1;
}
__setup("syno_hdd_enable=", early_hdd_enable);
#endif  

int SYNO_CTRL_HDD_POWERON(int index, int value)
{
#ifdef CONFIG_SYNO_SMBUS_HDD_POWERCTL
	if (0 < g_smbus_hdd_powerctl) {
		if (!SynoSmbusHddPowerCtl.bl_init){
			syno_smbus_hdd_powerctl_init();
		}
		if (NULL != SynoSmbusHddPowerCtl.syno_smbus_hdd_enable_write) {
			SynoSmbusHddPowerCtl.syno_smbus_hdd_enable_write(gSynoSmbusHddAdapter, gSynoSmbusHddAddress, index, value);
		}
		return 0;
	}
#endif  
	if (!HAVE_HDD_ENABLE(index)) {  
		printk("No such hdd enable pin. Index: %d\n", index);
		WARN_ON(1);
		return -EINVAL;
	}
	SYNO_GPIO_WRITE(HDD_ENABLE_PIN(index), value);

	return 0;
}
EXPORT_SYMBOL(SYNO_CTRL_HDD_POWERON);
 
int SYNO_CHECK_HDD_DETECT(int index)
{
	int ret;
#ifdef CONFIG_SYNO_SMBUS_HDD_POWERCTL
	if (0 < g_smbus_hdd_powerctl) {
		if (!SynoSmbusHddPowerCtl.bl_init){
			syno_smbus_hdd_powerctl_init();
		}
		if ( NULL != SynoSmbusHddPowerCtl.syno_smbus_hdd_present_read) {
			ret = SynoSmbusHddPowerCtl.syno_smbus_hdd_present_read(gSynoSmbusHddAdapter, gSynoSmbusHddAddress, index);
			return ret;
		}
		return 0;
	}
#endif  
	if (!HAVE_HDD_DETECT(index)) {  
		 
		return 1;
	}
	ret = SYNO_GPIO_READ(HDD_DETECT_PIN(index));
	 
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
	if (ACTIVE_LOW == HDD_DETECT_POLARITY(index)) {
#else
	if (ACTIVE_LOW == HDD_DETECT_POLARITY()) {
#endif  
		return !ret;
	}
	return ret;
}
EXPORT_SYMBOL(SYNO_CHECK_HDD_DETECT);
 
int SYNO_SUPPORT_HDD_DYNAMIC_ENABLE_POWER(void)
{
#ifdef CONFIG_SYNO_SMBUS_HDD_POWERCTL
	if ( 0 < g_smbus_hdd_powerctl || HAVE_HDD_ENABLE(1)) {
#else  
	 
	if (HAVE_HDD_ENABLE(1)) {
#endif  
		return 1;
	}
	return 0;
}
EXPORT_SYMBOL(SYNO_SUPPORT_HDD_DYNAMIC_ENABLE_POWER);
#if defined(CONFIG_SYNO_PORT_MAPPING_V2)  
#else
int SYNO_CTRL_GPIO_HDD_ACT_LED(int index, int value)
{
	index++;   
	if (!HAVE_HDD_ACT_LED(index)) {
		printk("No such hdd act led pin. Index: %d\n", index);
		WARN_ON(1);
		return -EINVAL;
	}
	SYNO_GPIO_WRITE(HDD_ACT_LED_PIN(index), value);
	return 0;
}
EXPORT_SYMBOL(SYNO_CTRL_GPIO_HDD_ACT_LED);
#endif  
#ifdef MY_ABC_HERE
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
#else
static SYNO_GPIO_INFO redundant_power_detect = {
	.name 			= "redundant power detect",
	.nr_gpio		= 0,
	.gpio_port		= {0},
	.gpio_polarity	= ACTIVE_LOW,
};
static int __init early_redundant_power_detect(char *p)
{
	int i, ret;
	 
	if ((NULL == p) || (0 == strlen(p))) {
		return 1;
	}
	printk("SYNO GPIO redundant power detect pin: ");
	ret = hdd_bootargs_parser(&hdd_detect, p);
	if (-EINVAL == ret) {
		printk("Error parameters, %s\n", p);
	} else if (-EMSGSIZE == ret) {
		printk("Too many arguments, %s\n", p);
	} else {
		syno_gpio.redundant_power_detect = &redundant_power_detect;
		for (i = 0; i < syno_gpio.redundant_power_detect->nr_gpio; i++) {
			printk("%d ", RP_DETECT_PIN(i + 1));
		}
		printk("\n");
	}
	return 1;
}
__setup("syno_rp_detect=", early_redundant_power_detect);
#endif  
#endif  
#else
extern int SYNO_SUPPORT_HDD_DYNAMIC_ENABLE_POWER(void);
extern int SYNO_CTRL_HDD_POWERON(int index, int value);
extern int SYNO_CHECK_HDD_PRESENT(int index);
#ifdef MY_ABC_HERE
int g_syno_rp_detect_no;
int g_syno_rp_detect_list[SYNO_SPINUP_GROUP_PIN_MAX_NUM];
int g_syno_hdd_detect_no;
int g_syno_hdd_detect_list[SYNO_SPINUP_GROUP_PIN_MAX_NUM];
int g_syno_hdd_enable_no;
int g_syno_hdd_enable_list[SYNO_SPINUP_GROUP_PIN_MAX_NUM];
 
static int hdd_bootargs_parser (char *args, int *no, int *list)
{
        int pin_num = 0;
        char *endp;
        if (NULL == args || NULL == no || NULL == list) {
                return -EINVAL;
        }
        while (1) {
                if(SYNO_SPINUP_GROUP_PIN_MAX_NUM < pin_num) {
                        return -EMSGSIZE;
                }
                list[pin_num] = simple_strtol(args, &endp, 10) & 0xFF;
                pin_num++;
                 
                if (*endp == '\0') {
                        break;
                }
                args = ++endp;
        }
        *no = pin_num;
        return 0;
}

static int __init early_syno_rp_detect(char *p)
{
        int i, ret;

        ret = hdd_bootargs_parser(p, &g_syno_rp_detect_no, g_syno_rp_detect_list);

        printk("SYNO GPIO redundant power detect pin:");
        if (-EINVAL == ret) {
                printk("Error parameters, %s\n", p);
        } else if (-EMSGSIZE == ret) {
                printk("Too many arguments, %s\n", p);
        } else {
                for (i = 0; i < g_syno_rp_detect_no; ++i) {
                        printk(" %d", g_syno_rp_detect_list[i]);
                }
                printk("\n");
        }

        return 1;
}
__setup("syno_rp_detect=", early_syno_rp_detect);

static int __init early_syno_hdd_detect(char *p)
{
        int i, ret;

        ret = hdd_bootargs_parser(p, &g_syno_hdd_detect_no, g_syno_hdd_detect_list);

        printk("SYNO GPIO HDD detect pin:");
        if (-EINVAL == ret) {
                printk("Error parameters, %s\n", p);
        } else if (-EMSGSIZE == ret) {
                printk("Too many arguments, %s\n", p);
        } else {
                for (i = 0; i < g_syno_hdd_detect_no; ++i) {
                        printk(" %d", g_syno_hdd_detect_list[i]);
                }
                printk("\n");
        }

        return 1;
}
__setup("syno_hdd_detect=", early_syno_hdd_detect);

static int __init early_syno_hdd_enable(char *p)
{
        int i, ret;

        ret = hdd_bootargs_parser(p, &g_syno_hdd_enable_no, g_syno_hdd_enable_list);

        printk("SYNO GPIO HDD enable pin:");
        if (-EINVAL == ret) {
                printk("Error parameters, %s\n", p);
        } else if (-EMSGSIZE == ret) {
                printk("Too many arguments, %s\n", p);
        } else {
                for (i = 0; i < g_syno_hdd_enable_no; ++i) {
                        printk(" %d", g_syno_hdd_enable_list[i]);
                }
                printk("\n");
        }

        return 1;
}
__setup("syno_hdd_enable=", early_syno_hdd_enable);
#endif  
#if defined(MY_ABC_HERE)
extern u8 SYNO_GET_HDD_PRESENT_PIN(int index);
#define GPIO_UNDEF 0xFF
#endif  
#endif  
#endif  

static char ata_force_param_buf[PAGE_SIZE] __initdata;
 
module_param_string(force, ata_force_param_buf, sizeof(ata_force_param_buf), 0);
MODULE_PARM_DESC(force, "Force ATA configurations including cable type, link speed and transfer mode (see Documentation/kernel-parameters.txt for details)");

static int atapi_enabled = 1;
module_param(atapi_enabled, int, 0444);
MODULE_PARM_DESC(atapi_enabled, "Enable discovery of ATAPI devices (0=off, 1=on [default])");

static int atapi_dmadir = 0;
module_param(atapi_dmadir, int, 0444);
MODULE_PARM_DESC(atapi_dmadir, "Enable ATAPI DMADIR bridge support (0=off [default], 1=on)");

int atapi_passthru16 = 1;
module_param(atapi_passthru16, int, 0444);
MODULE_PARM_DESC(atapi_passthru16, "Enable ATA_16 passthru for ATAPI devices (0=off, 1=on [default])");

int libata_fua = 0;
module_param_named(fua, libata_fua, int, 0444);
MODULE_PARM_DESC(fua, "FUA support (0=off [default], 1=on)");

static int ata_ignore_hpa;
module_param_named(ignore_hpa, ata_ignore_hpa, int, 0644);
MODULE_PARM_DESC(ignore_hpa, "Ignore HPA limit (0=keep BIOS limits, 1=ignore limits, using full disk)");

static int libata_dma_mask = ATA_DMA_MASK_ATA|ATA_DMA_MASK_ATAPI|ATA_DMA_MASK_CFA;
module_param_named(dma, libata_dma_mask, int, 0444);
MODULE_PARM_DESC(dma, "DMA enable/disable (0x1==ATA, 0x2==ATAPI, 0x4==CF)");

static int ata_probe_timeout;
module_param(ata_probe_timeout, int, 0444);
MODULE_PARM_DESC(ata_probe_timeout, "Set ATA probing timeout (seconds)");

int libata_noacpi = 0;
module_param_named(noacpi, libata_noacpi, int, 0444);
MODULE_PARM_DESC(noacpi, "Disable the use of ACPI in probe/suspend/resume (0=off [default], 1=on)");

int libata_allow_tpm = 0;
module_param_named(allow_tpm, libata_allow_tpm, int, 0444);
MODULE_PARM_DESC(allow_tpm, "Permit the use of TPM commands (0=off [default], 1=on)");

static int atapi_an;
module_param(atapi_an, int, 0444);
MODULE_PARM_DESC(atapi_an, "Enable ATAPI AN media presence notification (0=0ff [default], 1=on)");

MODULE_AUTHOR("Jeff Garzik");
MODULE_DESCRIPTION("Library module for ATA devices");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);

static bool ata_sstatus_online(u32 sstatus)
{
	return (sstatus & 0xf) == 0x3;
}

struct ata_link *ata_link_next(struct ata_link *link, struct ata_port *ap,
			       enum ata_link_iter_mode mode)
{
	BUG_ON(mode != ATA_LITER_EDGE &&
	       mode != ATA_LITER_PMP_FIRST && mode != ATA_LITER_HOST_FIRST);

	if (!link)
		switch (mode) {
		case ATA_LITER_EDGE:
		case ATA_LITER_PMP_FIRST:
			if (sata_pmp_attached(ap))
				return ap->pmp_link;
			 
		case ATA_LITER_HOST_FIRST:
			return &ap->link;
		}

	if (link == &ap->link)
		switch (mode) {
		case ATA_LITER_HOST_FIRST:
			if (sata_pmp_attached(ap))
				return ap->pmp_link;
			 
		case ATA_LITER_PMP_FIRST:
			if (unlikely(ap->slave_link))
				return ap->slave_link;
			 
		case ATA_LITER_EDGE:
			return NULL;
		}

	if (unlikely(link == ap->slave_link))
		return NULL;

	if (++link < ap->pmp_link + ap->nr_pmp_links)
		return link;

	if (mode == ATA_LITER_PMP_FIRST)
		return &ap->link;

	return NULL;
}

struct ata_device *ata_dev_next(struct ata_device *dev, struct ata_link *link,
				enum ata_dev_iter_mode mode)
{
	BUG_ON(mode != ATA_DITER_ENABLED && mode != ATA_DITER_ENABLED_REVERSE &&
	       mode != ATA_DITER_ALL && mode != ATA_DITER_ALL_REVERSE);

	if (!dev)
		switch (mode) {
		case ATA_DITER_ENABLED:
		case ATA_DITER_ALL:
			dev = link->device;
			goto check;
		case ATA_DITER_ENABLED_REVERSE:
		case ATA_DITER_ALL_REVERSE:
			dev = link->device + ata_link_max_devices(link) - 1;
			goto check;
		}

 next:
	 
	switch (mode) {
	case ATA_DITER_ENABLED:
	case ATA_DITER_ALL:
		if (++dev < link->device + ata_link_max_devices(link))
			goto check;
		return NULL;
	case ATA_DITER_ENABLED_REVERSE:
	case ATA_DITER_ALL_REVERSE:
		if (--dev >= link->device)
			goto check;
		return NULL;
	}

 check:
	if ((mode == ATA_DITER_ENABLED || mode == ATA_DITER_ENABLED_REVERSE) &&
	    !ata_dev_enabled(dev))
		goto next;
	return dev;
}

struct ata_link *ata_dev_phys_link(struct ata_device *dev)
{
	struct ata_port *ap = dev->link->ap;

	if (!ap->slave_link)
		return dev->link;
	if (!dev->devno)
		return &ap->link;
	return ap->slave_link;
}

void ata_force_cbl(struct ata_port *ap)
{
	int i;

	for (i = ata_force_tbl_size - 1; i >= 0; i--) {
		const struct ata_force_ent *fe = &ata_force_tbl[i];

		if (fe->port != -1 && fe->port != ap->print_id)
			continue;

		if (fe->param.cbl == ATA_CBL_NONE)
			continue;

		ap->cbl = fe->param.cbl;
		ata_port_notice(ap, "FORCE: cable set to %s\n", fe->param.name);
		return;
	}
}

static void ata_force_link_limits(struct ata_link *link)
{
	bool did_spd = false;
	int linkno = link->pmp;
	int i;

	if (ata_is_host_link(link))
		linkno += 15;

	for (i = ata_force_tbl_size - 1; i >= 0; i--) {
		const struct ata_force_ent *fe = &ata_force_tbl[i];

		if (fe->port != -1 && fe->port != link->ap->print_id)
			continue;

		if (fe->device != -1 && fe->device != linkno)
			continue;

		if (!did_spd && fe->param.spd_limit) {
			link->hw_sata_spd_limit = (1 << fe->param.spd_limit) - 1;
			ata_link_notice(link, "FORCE: PHY spd limit set to %s\n",
					fe->param.name);
			did_spd = true;
		}

		if (fe->param.lflags) {
			link->flags |= fe->param.lflags;
			ata_link_notice(link,
					"FORCE: link flag 0x%x forced -> 0x%x\n",
					fe->param.lflags, link->flags);
		}
	}
}

static void ata_force_xfermask(struct ata_device *dev)
{
	int devno = dev->link->pmp + dev->devno;
	int alt_devno = devno;
	int i;

	if (ata_is_host_link(dev->link))
		alt_devno += 15;

	for (i = ata_force_tbl_size - 1; i >= 0; i--) {
		const struct ata_force_ent *fe = &ata_force_tbl[i];
		unsigned long pio_mask, mwdma_mask, udma_mask;

		if (fe->port != -1 && fe->port != dev->link->ap->print_id)
			continue;

		if (fe->device != -1 && fe->device != devno &&
		    fe->device != alt_devno)
			continue;

		if (!fe->param.xfer_mask)
			continue;

		ata_unpack_xfermask(fe->param.xfer_mask,
				    &pio_mask, &mwdma_mask, &udma_mask);
		if (udma_mask)
			dev->udma_mask = udma_mask;
		else if (mwdma_mask) {
			dev->udma_mask = 0;
			dev->mwdma_mask = mwdma_mask;
		} else {
			dev->udma_mask = 0;
			dev->mwdma_mask = 0;
			dev->pio_mask = pio_mask;
		}

		ata_dev_notice(dev, "FORCE: xfer_mask set to %s\n",
			       fe->param.name);
		return;
	}
}

static void ata_force_horkage(struct ata_device *dev)
{
	int devno = dev->link->pmp + dev->devno;
	int alt_devno = devno;
	int i;

	if (ata_is_host_link(dev->link))
		alt_devno += 15;

	for (i = 0; i < ata_force_tbl_size; i++) {
		const struct ata_force_ent *fe = &ata_force_tbl[i];

		if (fe->port != -1 && fe->port != dev->link->ap->print_id)
			continue;

		if (fe->device != -1 && fe->device != devno &&
		    fe->device != alt_devno)
			continue;

		if (!(~dev->horkage & fe->param.horkage_on) &&
		    !(dev->horkage & fe->param.horkage_off))
			continue;

		dev->horkage |= fe->param.horkage_on;
		dev->horkage &= ~fe->param.horkage_off;

		ata_dev_notice(dev, "FORCE: horkage modified (%s)\n",
			       fe->param.name);
	}
}

int atapi_cmd_type(u8 opcode)
{
	switch (opcode) {
	case GPCMD_READ_10:
	case GPCMD_READ_12:
		return ATAPI_READ;

	case GPCMD_WRITE_10:
	case GPCMD_WRITE_12:
	case GPCMD_WRITE_AND_VERIFY_10:
		return ATAPI_WRITE;

	case GPCMD_READ_CD:
	case GPCMD_READ_CD_MSF:
		return ATAPI_READ_CD;

	case ATA_16:
	case ATA_12:
		if (atapi_passthru16)
			return ATAPI_PASS_THRU;
		 
	default:
		return ATAPI_MISC;
	}
}

void ata_tf_to_fis(const struct ata_taskfile *tf, u8 pmp, int is_cmd, u8 *fis)
{
	fis[0] = 0x27;			 
	fis[1] = pmp & 0xf;		 
	if (is_cmd)
		fis[1] |= (1 << 7);	 

	fis[2] = tf->command;
	fis[3] = tf->feature;

	fis[4] = tf->lbal;
	fis[5] = tf->lbam;
	fis[6] = tf->lbah;
	fis[7] = tf->device;

	fis[8] = tf->hob_lbal;
	fis[9] = tf->hob_lbam;
	fis[10] = tf->hob_lbah;
	fis[11] = tf->hob_feature;

	fis[12] = tf->nsect;
	fis[13] = tf->hob_nsect;
	fis[14] = 0;
	fis[15] = tf->ctl;

	fis[16] = tf->auxiliary & 0xff;
	fis[17] = (tf->auxiliary >> 8) & 0xff;
	fis[18] = (tf->auxiliary >> 16) & 0xff;
	fis[19] = (tf->auxiliary >> 24) & 0xff;
}

void ata_tf_from_fis(const u8 *fis, struct ata_taskfile *tf)
{
	tf->command	= fis[2];	 
	tf->feature	= fis[3];	 

	tf->lbal	= fis[4];
	tf->lbam	= fis[5];
	tf->lbah	= fis[6];
	tf->device	= fis[7];

	tf->hob_lbal	= fis[8];
	tf->hob_lbam	= fis[9];
	tf->hob_lbah	= fis[10];

	tf->nsect	= fis[12];
	tf->hob_nsect	= fis[13];
}

static const u8 ata_rw_cmds[] = {
	 
	ATA_CMD_READ_MULTI,
	ATA_CMD_WRITE_MULTI,
	ATA_CMD_READ_MULTI_EXT,
	ATA_CMD_WRITE_MULTI_EXT,
	0,
	0,
	0,
	ATA_CMD_WRITE_MULTI_FUA_EXT,
	 
	ATA_CMD_PIO_READ,
	ATA_CMD_PIO_WRITE,
	ATA_CMD_PIO_READ_EXT,
	ATA_CMD_PIO_WRITE_EXT,
	0,
	0,
	0,
	0,
	 
	ATA_CMD_READ,
	ATA_CMD_WRITE,
	ATA_CMD_READ_EXT,
	ATA_CMD_WRITE_EXT,
	0,
	0,
	0,
	ATA_CMD_WRITE_FUA_EXT
};

static int ata_rwcmd_protocol(struct ata_taskfile *tf, struct ata_device *dev)
{
	u8 cmd;

	int index, fua, lba48, write;

	fua = (tf->flags & ATA_TFLAG_FUA) ? 4 : 0;
	lba48 = (tf->flags & ATA_TFLAG_LBA48) ? 2 : 0;
	write = (tf->flags & ATA_TFLAG_WRITE) ? 1 : 0;

	if (dev->flags & ATA_DFLAG_PIO) {
		tf->protocol = ATA_PROT_PIO;
		index = dev->multi_count ? 0 : 8;
	} else if (lba48 && (dev->link->ap->flags & ATA_FLAG_PIO_LBA48)) {
		 
		tf->protocol = ATA_PROT_PIO;
		index = dev->multi_count ? 0 : 8;
	} else {
		tf->protocol = ATA_PROT_DMA;
		index = 16;
	}

	cmd = ata_rw_cmds[index + fua + lba48 + write];
	if (cmd) {
		tf->command = cmd;
		return 0;
	}
	return -1;
}

u64 ata_tf_read_block(struct ata_taskfile *tf, struct ata_device *dev)
{
	u64 block = 0;

	if (tf->flags & ATA_TFLAG_LBA) {
		if (tf->flags & ATA_TFLAG_LBA48) {
			block |= (u64)tf->hob_lbah << 40;
			block |= (u64)tf->hob_lbam << 32;
			block |= (u64)tf->hob_lbal << 24;
		} else
			block |= (tf->device & 0xf) << 24;

		block |= tf->lbah << 16;
		block |= tf->lbam << 8;
		block |= tf->lbal;
	} else {
		u32 cyl, head, sect;

		cyl = tf->lbam | (tf->lbah << 8);
		head = tf->device & 0xf;
		sect = tf->lbal;

		if (!sect) {
			ata_dev_warn(dev,
				     "device reported invalid CHS sector 0\n");
			sect = 1;  
		}

		block = (cyl * dev->heads + head) * dev->sectors + sect - 1;
	}

	return block;
}

int ata_build_rw_tf(struct ata_taskfile *tf, struct ata_device *dev,
		    u64 block, u32 n_block, unsigned int tf_flags,
		    unsigned int tag)
{
	tf->flags |= ATA_TFLAG_ISADDR | ATA_TFLAG_DEVICE;
	tf->flags |= tf_flags;

	if (ata_ncq_enabled(dev) && likely(tag != ATA_TAG_INTERNAL)) {
		 
		if (!lba_48_ok(block, n_block))
			return -ERANGE;

		tf->protocol = ATA_PROT_NCQ;
		tf->flags |= ATA_TFLAG_LBA | ATA_TFLAG_LBA48;

		if (tf->flags & ATA_TFLAG_WRITE)
			tf->command = ATA_CMD_FPDMA_WRITE;
		else
			tf->command = ATA_CMD_FPDMA_READ;

		tf->nsect = tag << 3;
		tf->hob_feature = (n_block >> 8) & 0xff;
		tf->feature = n_block & 0xff;

		tf->hob_lbah = (block >> 40) & 0xff;
		tf->hob_lbam = (block >> 32) & 0xff;
		tf->hob_lbal = (block >> 24) & 0xff;
		tf->lbah = (block >> 16) & 0xff;
		tf->lbam = (block >> 8) & 0xff;
		tf->lbal = block & 0xff;

		tf->device = ATA_LBA;
		if (tf->flags & ATA_TFLAG_FUA)
			tf->device |= 1 << 7;
	} else if (dev->flags & ATA_DFLAG_LBA) {
		tf->flags |= ATA_TFLAG_LBA;

		if (lba_28_ok(block, n_block)) {
			 
			tf->device |= (block >> 24) & 0xf;
		} else if (lba_48_ok(block, n_block)) {
			if (!(dev->flags & ATA_DFLAG_LBA48))
				return -ERANGE;

			tf->flags |= ATA_TFLAG_LBA48;

			tf->hob_nsect = (n_block >> 8) & 0xff;

			tf->hob_lbah = (block >> 40) & 0xff;
			tf->hob_lbam = (block >> 32) & 0xff;
			tf->hob_lbal = (block >> 24) & 0xff;
		} else
			 
			return -ERANGE;

		if (unlikely(ata_rwcmd_protocol(tf, dev) < 0))
			return -EINVAL;

		tf->nsect = n_block & 0xff;

		tf->lbah = (block >> 16) & 0xff;
		tf->lbam = (block >> 8) & 0xff;
		tf->lbal = block & 0xff;

		tf->device |= ATA_LBA;
	} else {
		 
		u32 sect, head, cyl, track;

		if (!lba_28_ok(block, n_block))
			return -ERANGE;

		if (unlikely(ata_rwcmd_protocol(tf, dev) < 0))
			return -EINVAL;

		track = (u32)block / dev->sectors;
		cyl   = track / dev->heads;
		head  = track % dev->heads;
		sect  = (u32)block % dev->sectors + 1;

		DPRINTK("block %u track %u cyl %u head %u sect %u\n",
			(u32)block, track, cyl, head, sect);

		if ((cyl >> 16) || (head >> 4) || (sect >> 8) || (!sect))
			return -ERANGE;

		tf->nsect = n_block & 0xff;  
		tf->lbal = sect;
		tf->lbam = cyl;
		tf->lbah = cyl >> 8;
		tf->device |= head;
	}

	return 0;
}

unsigned long ata_pack_xfermask(unsigned long pio_mask,
				unsigned long mwdma_mask,
				unsigned long udma_mask)
{
	return ((pio_mask << ATA_SHIFT_PIO) & ATA_MASK_PIO) |
		((mwdma_mask << ATA_SHIFT_MWDMA) & ATA_MASK_MWDMA) |
		((udma_mask << ATA_SHIFT_UDMA) & ATA_MASK_UDMA);
}

void ata_unpack_xfermask(unsigned long xfer_mask, unsigned long *pio_mask,
			 unsigned long *mwdma_mask, unsigned long *udma_mask)
{
	if (pio_mask)
		*pio_mask = (xfer_mask & ATA_MASK_PIO) >> ATA_SHIFT_PIO;
	if (mwdma_mask)
		*mwdma_mask = (xfer_mask & ATA_MASK_MWDMA) >> ATA_SHIFT_MWDMA;
	if (udma_mask)
		*udma_mask = (xfer_mask & ATA_MASK_UDMA) >> ATA_SHIFT_UDMA;
}

static const struct ata_xfer_ent {
	int shift, bits;
	u8 base;
} ata_xfer_tbl[] = {
	{ ATA_SHIFT_PIO, ATA_NR_PIO_MODES, XFER_PIO_0 },
	{ ATA_SHIFT_MWDMA, ATA_NR_MWDMA_MODES, XFER_MW_DMA_0 },
	{ ATA_SHIFT_UDMA, ATA_NR_UDMA_MODES, XFER_UDMA_0 },
	{ -1, },
};

u8 ata_xfer_mask2mode(unsigned long xfer_mask)
{
	int highbit = fls(xfer_mask) - 1;
	const struct ata_xfer_ent *ent;

	for (ent = ata_xfer_tbl; ent->shift >= 0; ent++)
		if (highbit >= ent->shift && highbit < ent->shift + ent->bits)
			return ent->base + highbit - ent->shift;
	return 0xff;
}

unsigned long ata_xfer_mode2mask(u8 xfer_mode)
{
	const struct ata_xfer_ent *ent;

	for (ent = ata_xfer_tbl; ent->shift >= 0; ent++)
		if (xfer_mode >= ent->base && xfer_mode < ent->base + ent->bits)
			return ((2 << (ent->shift + xfer_mode - ent->base)) - 1)
				& ~((1 << ent->shift) - 1);
	return 0;
}

int ata_xfer_mode2shift(unsigned long xfer_mode)
{
	const struct ata_xfer_ent *ent;

	for (ent = ata_xfer_tbl; ent->shift >= 0; ent++)
		if (xfer_mode >= ent->base && xfer_mode < ent->base + ent->bits)
			return ent->shift;
	return -1;
}

const char *ata_mode_string(unsigned long xfer_mask)
{
	static const char * const xfer_mode_str[] = {
		"PIO0",
		"PIO1",
		"PIO2",
		"PIO3",
		"PIO4",
		"PIO5",
		"PIO6",
		"MWDMA0",
		"MWDMA1",
		"MWDMA2",
		"MWDMA3",
		"MWDMA4",
		"UDMA/16",
		"UDMA/25",
		"UDMA/33",
		"UDMA/44",
		"UDMA/66",
		"UDMA/100",
		"UDMA/133",
		"UDMA7",
	};
	int highbit;

	highbit = fls(xfer_mask) - 1;
	if (highbit >= 0 && highbit < ARRAY_SIZE(xfer_mode_str))
		return xfer_mode_str[highbit];
	return "<n/a>";
}

const char *sata_spd_string(unsigned int spd)
{
	static const char * const spd_str[] = {
		"1.5 Gbps",
		"3.0 Gbps",
		"6.0 Gbps",
	};

	if (spd == 0 || (spd - 1) >= ARRAY_SIZE(spd_str))
		return "<unknown>";
	return spd_str[spd - 1];
}

unsigned int ata_dev_classify(const struct ata_taskfile *tf)
{
	 
	if ((tf->lbam == 0) && (tf->lbah == 0)) {
		DPRINTK("found ATA device by sig\n");
		return ATA_DEV_ATA;
	}

	if ((tf->lbam == 0x14) && (tf->lbah == 0xeb)) {
		DPRINTK("found ATAPI device by sig\n");
		return ATA_DEV_ATAPI;
	}

	if ((tf->lbam == 0x69) && (tf->lbah == 0x96)) {
		DPRINTK("found PMP device by sig\n");
		return ATA_DEV_PMP;
	}

	if ((tf->lbam == 0x3c) && (tf->lbah == 0xc3)) {
		DPRINTK("found SEMB device by sig (could be ATA device)\n");
		return ATA_DEV_SEMB;
	}

	if ((tf->lbam == 0xcd) && (tf->lbah == 0xab)) {
		DPRINTK("found ZAC device by sig\n");
		return ATA_DEV_ZAC;
	}

	DPRINTK("unknown device\n");
	return ATA_DEV_UNKNOWN;
}

void ata_id_string(const u16 *id, unsigned char *s,
		   unsigned int ofs, unsigned int len)
{
	unsigned int c;

	BUG_ON(len & 1);

	while (len > 0) {
		c = id[ofs] >> 8;
		*s = c;
		s++;

		c = id[ofs] & 0xff;
		*s = c;
		s++;

		ofs++;
		len -= 2;
	}
}

void ata_id_c_string(const u16 *id, unsigned char *s,
		     unsigned int ofs, unsigned int len)
{
	unsigned char *p;

	ata_id_string(id, s, ofs, len - 1);

	p = s + strnlen(s, len - 1);
	while (p > s && p[-1] == ' ')
		p--;
	*p = '\0';
}

static u64 ata_id_n_sectors(const u16 *id)
{
	if (ata_id_has_lba(id)) {
		if (ata_id_has_lba48(id))
			return ata_id_u64(id, ATA_ID_LBA_CAPACITY_2);
		else
			return ata_id_u32(id, ATA_ID_LBA_CAPACITY);
	} else {
		if (ata_id_current_chs_valid(id))
			return id[ATA_ID_CUR_CYLS] * id[ATA_ID_CUR_HEADS] *
			       id[ATA_ID_CUR_SECTORS];
		else
			return id[ATA_ID_CYLS] * id[ATA_ID_HEADS] *
			       id[ATA_ID_SECTORS];
	}
}

u64 ata_tf_to_lba48(const struct ata_taskfile *tf)
{
	u64 sectors = 0;

	sectors |= ((u64)(tf->hob_lbah & 0xff)) << 40;
	sectors |= ((u64)(tf->hob_lbam & 0xff)) << 32;
	sectors |= ((u64)(tf->hob_lbal & 0xff)) << 24;
	sectors |= (tf->lbah & 0xff) << 16;
	sectors |= (tf->lbam & 0xff) << 8;
	sectors |= (tf->lbal & 0xff);

	return sectors;
}

u64 ata_tf_to_lba(const struct ata_taskfile *tf)
{
	u64 sectors = 0;

	sectors |= (tf->device & 0x0f) << 24;
	sectors |= (tf->lbah & 0xff) << 16;
	sectors |= (tf->lbam & 0xff) << 8;
	sectors |= (tf->lbal & 0xff);

	return sectors;
}

static int ata_read_native_max_address(struct ata_device *dev, u64 *max_sectors)
{
	unsigned int err_mask;
	struct ata_taskfile tf;
	int lba48 = ata_id_has_lba48(dev->id);

	ata_tf_init(dev, &tf);

	tf.flags |= ATA_TFLAG_DEVICE | ATA_TFLAG_ISADDR;

	if (lba48) {
		tf.command = ATA_CMD_READ_NATIVE_MAX_EXT;
		tf.flags |= ATA_TFLAG_LBA48;
	} else
		tf.command = ATA_CMD_READ_NATIVE_MAX;

	tf.protocol |= ATA_PROT_NODATA;
	tf.device |= ATA_LBA;

	err_mask = ata_exec_internal(dev, &tf, NULL, DMA_NONE, NULL, 0, 0);
	if (err_mask) {
		ata_dev_warn(dev,
			     "failed to read native max address (err_mask=0x%x)\n",
			     err_mask);
		if (err_mask == AC_ERR_DEV && (tf.feature & ATA_ABORTED))
			return -EACCES;
		return -EIO;
	}

	if (lba48)
		*max_sectors = ata_tf_to_lba48(&tf) + 1;
	else
		*max_sectors = ata_tf_to_lba(&tf) + 1;
	if (dev->horkage & ATA_HORKAGE_HPA_SIZE)
		(*max_sectors)--;
	return 0;
}

static int ata_set_max_sectors(struct ata_device *dev, u64 new_sectors)
{
	unsigned int err_mask;
	struct ata_taskfile tf;
	int lba48 = ata_id_has_lba48(dev->id);

	new_sectors--;

	ata_tf_init(dev, &tf);

	tf.flags |= ATA_TFLAG_DEVICE | ATA_TFLAG_ISADDR;

	if (lba48) {
		tf.command = ATA_CMD_SET_MAX_EXT;
		tf.flags |= ATA_TFLAG_LBA48;

		tf.hob_lbal = (new_sectors >> 24) & 0xff;
		tf.hob_lbam = (new_sectors >> 32) & 0xff;
		tf.hob_lbah = (new_sectors >> 40) & 0xff;
	} else {
		tf.command = ATA_CMD_SET_MAX;

		tf.device |= (new_sectors >> 24) & 0xf;
	}

	tf.protocol |= ATA_PROT_NODATA;
	tf.device |= ATA_LBA;

	tf.lbal = (new_sectors >> 0) & 0xff;
	tf.lbam = (new_sectors >> 8) & 0xff;
	tf.lbah = (new_sectors >> 16) & 0xff;

	err_mask = ata_exec_internal(dev, &tf, NULL, DMA_NONE, NULL, 0, 0);
	if (err_mask) {
		ata_dev_warn(dev,
			     "failed to set max address (err_mask=0x%x)\n",
			     err_mask);
		if (err_mask == AC_ERR_DEV &&
		    (tf.feature & (ATA_ABORTED | ATA_IDNF)))
			return -EACCES;
		return -EIO;
	}

	return 0;
}

static int ata_hpa_resize(struct ata_device *dev)
{
	struct ata_eh_context *ehc = &dev->link->eh_context;
	int print_info = ehc->i.flags & ATA_EHI_PRINTINFO;
	bool unlock_hpa = ata_ignore_hpa || dev->flags & ATA_DFLAG_UNLOCK_HPA;
	u64 sectors = ata_id_n_sectors(dev->id);
	u64 native_sectors;
	int rc;

	if ((dev->class != ATA_DEV_ATA && dev->class != ATA_DEV_ZAC) ||
	    !ata_id_has_lba(dev->id) || !ata_id_hpa_enabled(dev->id) ||
	    (dev->horkage & ATA_HORKAGE_BROKEN_HPA))
		return 0;

	rc = ata_read_native_max_address(dev, &native_sectors);
	if (rc) {
		 
		if (rc == -EACCES || !unlock_hpa) {
			ata_dev_warn(dev,
				     "HPA support seems broken, skipping HPA handling\n");
			dev->horkage |= ATA_HORKAGE_BROKEN_HPA;

			if (rc == -EACCES)
				rc = 0;
		}

		return rc;
	}
	dev->n_native_sectors = native_sectors;

	if (native_sectors <= sectors || !unlock_hpa) {
		if (!print_info || native_sectors == sectors)
			return 0;

		if (native_sectors > sectors)
			ata_dev_info(dev,
				"HPA detected: current %llu, native %llu\n",
				(unsigned long long)sectors,
				(unsigned long long)native_sectors);
		else if (native_sectors < sectors)
			ata_dev_warn(dev,
				"native sectors (%llu) is smaller than sectors (%llu)\n",
				(unsigned long long)native_sectors,
				(unsigned long long)sectors);
		return 0;
	}

	rc = ata_set_max_sectors(dev, native_sectors);
	if (rc == -EACCES) {
		 
		ata_dev_warn(dev,
			     "device aborted resize (%llu -> %llu), skipping HPA handling\n",
			     (unsigned long long)sectors,
			     (unsigned long long)native_sectors);
		dev->horkage |= ATA_HORKAGE_BROKEN_HPA;
		return 0;
	} else if (rc)
		return rc;

	rc = ata_dev_reread_id(dev, 0);
	if (rc) {
		ata_dev_err(dev,
			    "failed to re-read IDENTIFY data after HPA resizing\n");
		return rc;
	}

	if (print_info) {
		u64 new_sectors = ata_id_n_sectors(dev->id);
		ata_dev_info(dev,
			"HPA unlocked: %llu -> %llu, native %llu\n",
			(unsigned long long)sectors,
			(unsigned long long)new_sectors,
			(unsigned long long)native_sectors);
	}

	return 0;
}

static inline void ata_dump_id(const u16 *id)
{
	DPRINTK("49==0x%04x  "
		"53==0x%04x  "
		"63==0x%04x  "
		"64==0x%04x  "
		"75==0x%04x  \n",
		id[49],
		id[53],
		id[63],
		id[64],
		id[75]);
	DPRINTK("80==0x%04x  "
		"81==0x%04x  "
		"82==0x%04x  "
		"83==0x%04x  "
		"84==0x%04x  \n",
		id[80],
		id[81],
		id[82],
		id[83],
		id[84]);
	DPRINTK("88==0x%04x  "
		"93==0x%04x\n",
		id[88],
		id[93]);
}

unsigned long ata_id_xfermask(const u16 *id)
{
	unsigned long pio_mask, mwdma_mask, udma_mask;

	if (id[ATA_ID_FIELD_VALID] & (1 << 1)) {
		pio_mask = id[ATA_ID_PIO_MODES] & 0x03;
		pio_mask <<= 3;
		pio_mask |= 0x7;
	} else {
		 
		u8 mode = (id[ATA_ID_OLD_PIO_MODES] >> 8) & 0xFF;
		if (mode < 5)	 
			pio_mask = (2 << mode) - 1;
		else
			pio_mask = 1;

	}

	mwdma_mask = id[ATA_ID_MWDMA_MODES] & 0x07;

	if (ata_id_is_cfa(id)) {
		 
		int pio = (id[ATA_ID_CFA_MODES] >> 0) & 0x7;
		int dma = (id[ATA_ID_CFA_MODES] >> 3) & 0x7;

		if (pio)
			pio_mask |= (1 << 5);
		if (pio > 1)
			pio_mask |= (1 << 6);
		if (dma)
			mwdma_mask |= (1 << 3);
		if (dma > 1)
			mwdma_mask |= (1 << 4);
	}

	udma_mask = 0;
	if (id[ATA_ID_FIELD_VALID] & (1 << 2))
		udma_mask = id[ATA_ID_UDMA_MODES] & 0xff;

	return ata_pack_xfermask(pio_mask, mwdma_mask, udma_mask);
}

static void ata_qc_complete_internal(struct ata_queued_cmd *qc)
{
	struct completion *waiting = qc->private_data;

	complete(waiting);
}

unsigned ata_exec_internal_sg(struct ata_device *dev,
			      struct ata_taskfile *tf, const u8 *cdb,
			      int dma_dir, struct scatterlist *sgl,
			      unsigned int n_elem, unsigned long timeout)
{
	struct ata_link *link = dev->link;
	struct ata_port *ap = link->ap;
	u8 command = tf->command;
	int auto_timeout = 0;
	struct ata_queued_cmd *qc;
	unsigned int tag, preempted_tag;
	u32 preempted_sactive, preempted_qc_active;
	int preempted_nr_active_links;
	DECLARE_COMPLETION_ONSTACK(wait);
	unsigned long flags;
	unsigned int err_mask;
	int rc;

	spin_lock_irqsave(ap->lock, flags);

	if (ap->pflags & ATA_PFLAG_FROZEN) {
		spin_unlock_irqrestore(ap->lock, flags);
		return AC_ERR_SYSTEM;
	}

	if (ap->ops->error_handler)
		tag = ATA_TAG_INTERNAL;
	else
		tag = 0;

	qc = __ata_qc_from_tag(ap, tag);

	qc->tag = tag;
	qc->scsicmd = NULL;
	qc->ap = ap;
	qc->dev = dev;
	ata_qc_reinit(qc);

	preempted_tag = link->active_tag;
	preempted_sactive = link->sactive;
	preempted_qc_active = ap->qc_active;
	preempted_nr_active_links = ap->nr_active_links;
	link->active_tag = ATA_TAG_POISON;
	link->sactive = 0;
	ap->qc_active = 0;
	ap->nr_active_links = 0;

	qc->tf = *tf;
	if (cdb)
		memcpy(qc->cdb, cdb, ATAPI_CDB_LEN);

	if (tf->protocol == ATAPI_PROT_DMA && (dev->flags & ATA_DFLAG_DMADIR) &&
	    dma_dir == DMA_FROM_DEVICE)
		qc->tf.feature |= ATAPI_DMADIR;

	qc->flags |= ATA_QCFLAG_RESULT_TF;
	qc->dma_dir = dma_dir;
	if (dma_dir != DMA_NONE) {
		unsigned int i, buflen = 0;
		struct scatterlist *sg;

		for_each_sg(sgl, sg, n_elem, i)
			buflen += sg->length;

		ata_sg_init(qc, sgl, n_elem);
		qc->nbytes = buflen;
	}

	qc->private_data = &wait;
	qc->complete_fn = ata_qc_complete_internal;

	ata_qc_issue(qc);

	spin_unlock_irqrestore(ap->lock, flags);

	if (!timeout) {
		if (ata_probe_timeout)
			timeout = ata_probe_timeout * 1000;
		else {
			timeout = ata_internal_cmd_timeout(dev, command);
			auto_timeout = 1;
		}
	}

#ifdef MY_ABC_HERE
	 
	if (ap->ops->error_handler && !(ap->pflags & ATA_PFLAG_PMP_PMCTL))
#else
	if (ap->ops->error_handler)
#endif  
		ata_eh_release(ap);

	rc = wait_for_completion_timeout(&wait, msecs_to_jiffies(timeout));

#ifdef MY_ABC_HERE
	 
	if (ap->ops->error_handler && !(ap->pflags & ATA_PFLAG_PMP_PMCTL))
#else
	if (ap->ops->error_handler)
#endif  
		ata_eh_acquire(ap);

	ata_sff_flush_pio_task(ap);

	if (!rc) {
		spin_lock_irqsave(ap->lock, flags);

		if (qc->flags & ATA_QCFLAG_ACTIVE) {
			qc->err_mask |= AC_ERR_TIMEOUT;

			if (ap->ops->error_handler)
				ata_port_freeze(ap);
			else
				ata_qc_complete(qc);

			if (ata_msg_warn(ap))
				ata_dev_warn(dev, "qc timeout (cmd 0x%x)\n",
					     command);
		}

		spin_unlock_irqrestore(ap->lock, flags);
	}

	if (ap->ops->post_internal_cmd)
		ap->ops->post_internal_cmd(qc);

	if (qc->flags & ATA_QCFLAG_FAILED) {
		if (qc->result_tf.command & (ATA_ERR | ATA_DF))
			qc->err_mask |= AC_ERR_DEV;

		if (!qc->err_mask)
			qc->err_mask |= AC_ERR_OTHER;

		if (qc->err_mask & ~AC_ERR_OTHER)
			qc->err_mask &= ~AC_ERR_OTHER;
	}

	spin_lock_irqsave(ap->lock, flags);

	*tf = qc->result_tf;
	err_mask = qc->err_mask;

	ata_qc_free(qc);
	link->active_tag = preempted_tag;
	link->sactive = preempted_sactive;
	ap->qc_active = preempted_qc_active;
	ap->nr_active_links = preempted_nr_active_links;

	spin_unlock_irqrestore(ap->lock, flags);

	if ((err_mask & AC_ERR_TIMEOUT) && auto_timeout)
		ata_internal_cmd_timed_out(dev, command);

	return err_mask;
}

unsigned ata_exec_internal(struct ata_device *dev,
			   struct ata_taskfile *tf, const u8 *cdb,
			   int dma_dir, void *buf, unsigned int buflen,
			   unsigned long timeout)
{
	struct scatterlist *psg = NULL, sg;
	unsigned int n_elem = 0;

	if (dma_dir != DMA_NONE) {
		WARN_ON(!buf);
		sg_init_one(&sg, buf, buflen);
		psg = &sg;
		n_elem++;
	}

	return ata_exec_internal_sg(dev, tf, cdb, dma_dir, psg, n_elem,
				    timeout);
}

unsigned int ata_pio_need_iordy(const struct ata_device *adev)
{
	 
	if (adev->link->ap->pflags & ATA_PFLAG_RESETTING)
		return 0;
	 
	if (adev->link->ap->flags & ATA_FLAG_NO_IORDY)
		return 0;
	 
	if (ata_id_is_cfa(adev->id)
	    && (adev->pio_mode == XFER_PIO_5 || adev->pio_mode == XFER_PIO_6))
		return 0;
	 
	if (adev->pio_mode > XFER_PIO_2)
		return 1;
	 
	if (ata_id_has_iordy(adev->id))
		return 1;
	return 0;
}

static u32 ata_pio_mask_no_iordy(const struct ata_device *adev)
{
	 
	if (adev->id[ATA_ID_FIELD_VALID] & 2) {	 
		u16 pio = adev->id[ATA_ID_EIDE_PIO];
		 
		if (pio) {
			 
			if (pio > 240)	 
				return 3 << ATA_SHIFT_PIO;
			return 7 << ATA_SHIFT_PIO;
		}
	}
	return 3 << ATA_SHIFT_PIO;
}

unsigned int ata_do_dev_read_id(struct ata_device *dev,
					struct ata_taskfile *tf, u16 *id)
{
	return ata_exec_internal(dev, tf, NULL, DMA_FROM_DEVICE,
				     id, sizeof(id[0]) * ATA_ID_WORDS, 0);
}

int ata_dev_read_id(struct ata_device *dev, unsigned int *p_class,
		    unsigned int flags, u16 *id)
{
	struct ata_port *ap = dev->link->ap;
	unsigned int class = *p_class;
	struct ata_taskfile tf;
	unsigned int err_mask = 0;
	const char *reason;
	bool is_semb = class == ATA_DEV_SEMB;
	int may_fallback = 1, tried_spinup = 0;
	int rc;

	if (ata_msg_ctl(ap))
		ata_dev_dbg(dev, "%s: ENTER\n", __func__);

retry:
	ata_tf_init(dev, &tf);

	switch (class) {
	case ATA_DEV_SEMB:
		class = ATA_DEV_ATA;	 
	case ATA_DEV_ATA:
	case ATA_DEV_ZAC:
		tf.command = ATA_CMD_ID_ATA;
		break;
	case ATA_DEV_ATAPI:
		tf.command = ATA_CMD_ID_ATAPI;
		break;
	default:
		rc = -ENODEV;
		reason = "unsupported class";
		goto err_out;
	}

	tf.protocol = ATA_PROT_PIO;

	tf.flags |= ATA_TFLAG_ISADDR | ATA_TFLAG_DEVICE;

	tf.flags |= ATA_TFLAG_POLLING;

	if (ap->ops->read_id)
		err_mask = ap->ops->read_id(dev, &tf, id);
	else
		err_mask = ata_do_dev_read_id(dev, &tf, id);

	if (err_mask) {
		if (err_mask & AC_ERR_NODEV_HINT) {
			ata_dev_dbg(dev, "NODEV after polling detection\n");
			return -ENOENT;
		}

		if (is_semb) {
			ata_dev_info(dev,
		     "IDENTIFY failed on device w/ SEMB sig, disabled\n");
			 
			*p_class = ATA_DEV_SEMB_UNSUP;
			return 0;
		}

		if ((err_mask == AC_ERR_DEV) && (tf.feature & ATA_ABORTED)) {
			 
			if (may_fallback) {
				may_fallback = 0;

				if (class == ATA_DEV_ATA)
					class = ATA_DEV_ATAPI;
				else
					class = ATA_DEV_ATA;
				goto retry;
			}

			ata_dev_dbg(dev,
				    "both IDENTIFYs aborted, assuming NODEV\n");
			return -ENOENT;
		}

		rc = -EIO;
		reason = "I/O error";
		goto err_out;
	}

	if (dev->horkage & ATA_HORKAGE_DUMP_ID) {
		ata_dev_dbg(dev, "dumping IDENTIFY data, "
			    "class=%d may_fallback=%d tried_spinup=%d\n",
			    class, may_fallback, tried_spinup);
		print_hex_dump(KERN_DEBUG, "", DUMP_PREFIX_OFFSET,
			       16, 2, id, ATA_ID_WORDS * sizeof(*id), true);
	}

	may_fallback = 0;

	swap_buf_le16(id, ATA_ID_WORDS);

	rc = -EINVAL;
	reason = "device reports invalid type";

	if (class == ATA_DEV_ATA || class == ATA_DEV_ZAC) {
		if (!ata_id_is_ata(id) && !ata_id_is_cfa(id))
			goto err_out;
		if (ap->host->flags & ATA_HOST_IGNORE_ATA &&
							ata_id_is_ata(id)) {
			ata_dev_dbg(dev,
				"host indicates ignore ATA devices, ignored\n");
			return -ENOENT;
		}
	} else {
		if (ata_id_is_ata(id))
			goto err_out;
	}

	if (!tried_spinup && (id[2] == 0x37c8 || id[2] == 0x738c)) {
		tried_spinup = 1;
		 
		err_mask = ata_dev_set_feature(dev, SETFEATURES_SPINUP, 0);
		if (err_mask && id[2] != 0x738c) {
			rc = -EIO;
			reason = "SPINUP failed";
			goto err_out;
		}
		 
		if (id[2] == 0x37c8)
			goto retry;
	}

	if ((flags & ATA_READID_POSTRESET) &&
	    (class == ATA_DEV_ATA || class == ATA_DEV_ZAC)) {
		 
		if (ata_id_major_version(id) < 4 || !ata_id_has_lba(id)) {
			err_mask = ata_dev_init_params(dev, id[3], id[6]);
			if (err_mask) {
				rc = -EIO;
				reason = "INIT_DEV_PARAMS failed";
				goto err_out;
			}

			flags &= ~ATA_READID_POSTRESET;
			goto retry;
		}
	}

	*p_class = class;

#ifdef MY_ABC_HERE
	if (syno_ata_id_is_ssd(id)) {
		dev->is_ssd = 1;
	}
#endif  

	return 0;

 err_out:
	if (ata_msg_warn(ap))
		ata_dev_warn(dev, "failed to IDENTIFY (%s, err_mask=0x%x)\n",
			     reason, err_mask);
	return rc;
}

static int ata_do_link_spd_horkage(struct ata_device *dev)
{
	struct ata_link *plink = ata_dev_phys_link(dev);
	u32 target, target_limit;

	if (!sata_scr_valid(plink))
		return 0;

	if (dev->horkage & ATA_HORKAGE_1_5_GBPS)
		target = 1;
	else
		return 0;

	target_limit = (1 << target) - 1;

	if (plink->sata_spd_limit <= target_limit)
		return 0;

	plink->sata_spd_limit = target_limit;

	if (plink->sata_spd > target) {
		ata_dev_info(dev, "applying link speed limit horkage to %s\n",
			     sata_spd_string(target));
		return -EAGAIN;
	}
	return 0;
}

static inline u8 ata_dev_knobble(struct ata_device *dev)
{
	struct ata_port *ap = dev->link->ap;

	if (ata_dev_blacklisted(dev) & ATA_HORKAGE_BRIDGE_OK)
		return 0;

	return ((ap->cbl == ATA_CBL_SATA) && (!ata_id_is_sata(dev->id)));
}
#ifdef MY_ABC_HERE
static int syno_ata_dev_ncq_compat(struct ata_device *dev) {
	struct ata_port *ap = NULL;
	struct pci_dev *pdev = NULL;
	u16 vendor = 0;
	u16 device = 0;
	int ret = 1;

	if (dev && dev->link) {
		ap = dev->link->ap;
	}

	if (ap && ap->host) {
		pdev = to_pci_dev(ap->host->dev);
		vendor = sata_pmp_gscr_vendor(ap->link.device[0].gscr);
		device = sata_pmp_gscr_devid(ap->link.device[0].gscr);
	}

	if (pdev && ((pdev->vendor == 0x1b4b && pdev->device == 0x9170)
		|| (pdev->vendor == 0x1b4b && pdev->device == 0x9235)
		|| (pdev->vendor == 0x1b4b && pdev->device == 0x9215))
		&& syno_pm_is_3xxx(vendor, device)) {
		ret = 0;
	}

	return ret;
}
#endif  
static int ata_dev_config_ncq(struct ata_device *dev,
			       char *desc, size_t desc_sz)
{
	struct ata_port *ap = dev->link->ap;
	int hdepth = 0, ddepth = ata_id_queue_depth(dev->id);
	unsigned int err_mask;
	char *aa_desc = "";

	if (!ata_id_has_ncq(dev->id)) {
		desc[0] = '\0';
		return 0;
	}
	if (dev->horkage & ATA_HORKAGE_NONCQ) {
		snprintf(desc, desc_sz, "NCQ (not used)");
		return 0;
	}
	if (ap->flags & ATA_FLAG_NCQ) {
#ifdef MY_ABC_HERE
		if(!syno_ata_dev_ncq_compat(dev)) {
			snprintf(desc, desc_sz, "NCQ (not used)");
			return 0;
		}
#endif  
		hdepth = min(ap->scsi_host->can_queue, ATA_MAX_QUEUE - 1);
		dev->flags |= ATA_DFLAG_NCQ;
	}

	if (!(dev->horkage & ATA_HORKAGE_BROKEN_FPDMA_AA) &&
		(ap->flags & ATA_FLAG_FPDMA_AA) &&
		ata_id_has_fpdma_aa(dev->id)) {
		err_mask = ata_dev_set_feature(dev, SETFEATURES_SATA_ENABLE,
			SATA_FPDMA_AA);
		if (err_mask) {
			ata_dev_err(dev,
				    "failed to enable AA (error_mask=0x%x)\n",
				    err_mask);
			if (err_mask != AC_ERR_DEV) {
				dev->horkage |= ATA_HORKAGE_BROKEN_FPDMA_AA;
				return -EIO;
			}
		} else
			aa_desc = ", AA";
	}

	if (hdepth >= ddepth)
		snprintf(desc, desc_sz, "NCQ (depth %d)%s", ddepth, aa_desc);
	else
		snprintf(desc, desc_sz, "NCQ (depth %d/%d)%s", hdepth,
			ddepth, aa_desc);

	if ((ap->flags & ATA_FLAG_FPDMA_AUX) &&
	    ata_id_has_ncq_send_and_recv(dev->id)) {
		err_mask = ata_read_log_page(dev, ATA_LOG_NCQ_SEND_RECV,
					     0, ap->sector_buf, 1);
		if (err_mask) {
			ata_dev_dbg(dev,
				    "failed to get NCQ Send/Recv Log Emask 0x%x\n",
				    err_mask);
		} else {
			u8 *cmds = dev->ncq_send_recv_cmds;

			dev->flags |= ATA_DFLAG_NCQ_SEND_RECV;
			memcpy(cmds, ap->sector_buf, ATA_LOG_NCQ_SEND_RECV_SIZE);

#ifdef MY_ABC_HERE
			if (1) {
				 
#else
			if (dev->horkage & ATA_HORKAGE_NO_NCQ_TRIM) {
#endif  

				ata_dev_dbg(dev, "disabling queued TRIM support\n");
				cmds[ATA_LOG_NCQ_SEND_RECV_DSM_OFFSET] &=
					~ATA_LOG_NCQ_SEND_RECV_DSM_TRIM;
			}
		}
	}

	return 0;
}

#ifdef MY_ABC_HERE
static void syno_pmp_horkage(struct ata_port *ap, struct ata_device **dev) {
	unsigned char model_num[ATA_ID_PROD_LEN + 1];

	if (!ap || !dev || !*dev) {
		goto END;
	}

	ata_id_c_string((*dev)->id, model_num, ATA_ID_PROD, sizeof(model_num));
	if (syno_pm_is_synology_9705(ap) && glob_match("WDC WD?0EFRX-*", model_num)) {
		(*dev)->horkage |= ATA_HORKAGE_1_5_GBPS;
	}
END:
	return;
}
#endif  

int ata_dev_configure(struct ata_device *dev)
{
	struct ata_port *ap = dev->link->ap;
	struct ata_eh_context *ehc = &dev->link->eh_context;
	int print_info = ehc->i.flags & ATA_EHI_PRINTINFO;
	const u16 *id = dev->id;
	unsigned long xfer_mask;
	unsigned int err_mask;
	char revbuf[7];		 
	char fwrevbuf[ATA_ID_FW_REV_LEN+1];
	char modelbuf[ATA_ID_PROD_LEN+1];
#ifdef MY_ABC_HERE
	char snbuf[ATA_ID_SERNO_LEN+1];
#endif  
	int rc;
#ifdef MY_ABC_HERE
	struct pci_dev *pdev = NULL;
#endif  

	if (!ata_dev_enabled(dev) && ata_msg_info(ap)) {
		ata_dev_info(dev, "%s: ENTER/EXIT -- nodev\n", __func__);
		return 0;
	}

	if (ata_msg_probe(ap))
		ata_dev_dbg(dev, "%s: ENTER\n", __func__);

#ifdef MY_ABC_HERE
	 
	memset(&(dev->link->ata_latency), 0, sizeof(struct syno_ata_latency));
	memset(&(dev->link->latency_stat), 0, sizeof(struct syno_latency_stat));
	generate_random_uuid(dev->link->latency_stat.uuid);
#endif  

	dev->horkage |= ata_dev_blacklisted(dev);
#ifdef MY_ABC_HERE
	if (ap->nr_pmp_links) {
		 
		if (IS_SYNOLOGY_DX510(ap->PMSynoUnique)) {
			ata_dev_printk(dev, KERN_ERR,
				"Enhance DX510 compatibility, limit the speed to 1.5 GBPS\n");
			dev->horkage |= ATA_HORKAGE_1_5_GBPS;
			 
			sata_down_spd_limit(&ap->link, 0x1);
			sata_set_spd(&ap->link);
		}
#ifdef MY_ABC_HERE
		syno_pmp_horkage(ap, &dev);
#endif  
	}
#endif  

	ata_force_horkage(dev);

	if (dev->horkage & ATA_HORKAGE_DISABLE) {
		ata_dev_info(dev, "unsupported device, disabling\n");
		ata_dev_disable(dev);
		return 0;
	}

	if ((!atapi_enabled || (ap->flags & ATA_FLAG_NO_ATAPI)) &&
	    dev->class == ATA_DEV_ATAPI) {
		ata_dev_warn(dev, "WARNING: ATAPI is %s, device ignored\n",
			     atapi_enabled ? "not supported with this driver"
			     : "disabled");
		ata_dev_disable(dev);
		return 0;
	}

	rc = ata_do_link_spd_horkage(dev);
	if (rc)
		return rc;

	if ((dev->horkage & ATA_HORKAGE_WD_BROKEN_LPM) &&
	    (id[ATA_ID_SATA_CAPABILITY] & 0xe) == 0x2)
		dev->horkage |= ATA_HORKAGE_NOLPM;

	if (dev->horkage & ATA_HORKAGE_NOLPM) {
		ata_dev_warn(dev, "LPM support broken, forcing max_power\n");
		dev->link->ap->target_lpm_policy = ATA_LPM_MAX_POWER;
	}

	rc = ata_acpi_on_devcfg(dev);
	if (rc)
		return rc;

	rc = ata_hpa_resize(dev);
	if (rc)
		return rc;

	if (ata_msg_probe(ap))
		ata_dev_dbg(dev,
			    "%s: cfg 49:%04x 82:%04x 83:%04x 84:%04x "
			    "85:%04x 86:%04x 87:%04x 88:%04x\n",
			    __func__,
			    id[49], id[82], id[83], id[84],
			    id[85], id[86], id[87], id[88]);

	dev->flags &= ~ATA_DFLAG_CFG_MASK;
	dev->max_sectors = 0;
	dev->cdb_len = 0;
	dev->n_sectors = 0;
	dev->cylinders = 0;
	dev->heads = 0;
	dev->sectors = 0;
	dev->multi_count = 0;

	xfer_mask = ata_id_xfermask(id);

	if (ata_msg_probe(ap))
		ata_dump_id(id);

	ata_id_c_string(dev->id, fwrevbuf, ATA_ID_FW_REV,
			sizeof(fwrevbuf));

	ata_id_c_string(dev->id, modelbuf, ATA_ID_PROD,
			sizeof(modelbuf));
#ifdef MY_ABC_HERE
	ata_id_c_string(dev->id, snbuf, ATA_ID_SERNO,
			sizeof(snbuf));
#endif  

#ifdef MY_ABC_HERE
	 
	pdev = to_pci_dev(ap->host->dev);
	if (pdev != NULL && (pdev->device == 0x9215 || 0x9235 == pdev->device)) {
		if (glob_match("WDC WD7500BPKX-*", modelbuf) ||
			glob_match("WDC WD5000BPKX-*", modelbuf)) {
			dev->horkage |= ATA_HORKAGE_NONCQ;
		}
	}
#endif

	if (dev->class == ATA_DEV_ATA || dev->class == ATA_DEV_ZAC) {
		if (ata_id_is_cfa(id)) {
			 
			if (id[ATA_ID_CFA_KEY_MGMT] & 1)
				ata_dev_warn(dev,
	"supports DRM functions and may not be fully accessible\n");
			snprintf(revbuf, 7, "CFA");
		} else {
			snprintf(revbuf, 7, "ATA-%d", ata_id_major_version(id));
			 
			if (ata_id_has_tpm(id))
				ata_dev_warn(dev,
	"supports DRM functions and may not be fully accessible\n");
		}

		dev->n_sectors = ata_id_n_sectors(id);

		if ((dev->id[47] >> 8) == 0x80 && (dev->id[59] & 0x100)) {
			unsigned int max = dev->id[47] & 0xff;
			unsigned int cnt = dev->id[59] & 0xff;
			 
			if (is_power_of_2(max) && is_power_of_2(cnt))
				if (cnt <= max)
					dev->multi_count = cnt;
		}

		if (ata_id_has_lba(id)) {
			const char *lba_desc;
			char ncq_desc[24];

			lba_desc = "LBA";
			dev->flags |= ATA_DFLAG_LBA;
			if (ata_id_has_lba48(id)) {
				dev->flags |= ATA_DFLAG_LBA48;
				lba_desc = "LBA48";

				if (dev->n_sectors >= (1UL << 28) &&
				    ata_id_has_flush_ext(id))
					dev->flags |= ATA_DFLAG_FLUSH_EXT;
			}

			rc = ata_dev_config_ncq(dev, ncq_desc, sizeof(ncq_desc));
			if (rc)
				return rc;

			if (ata_msg_drv(ap) && print_info) {
				ata_dev_info(dev, "%s: %s, %s, max %s\n",
					     revbuf, modelbuf, fwrevbuf,
					     ata_mode_string(xfer_mask));
				ata_dev_info(dev,
					     "%llu sectors, multi %u: %s %s\n",
					(unsigned long long)dev->n_sectors,
					dev->multi_count, lba_desc, ncq_desc);
#ifdef MY_ABC_HERE
				ata_dev_info(dev,
					     "SN:%s\n",
					snbuf);
#endif  
			}
		} else {
			 
			dev->cylinders	= id[1];
			dev->heads	= id[3];
			dev->sectors	= id[6];

			if (ata_id_current_chs_valid(id)) {
				 
				dev->cylinders = id[54];
				dev->heads     = id[55];
				dev->sectors   = id[56];
			}

			if (ata_msg_drv(ap) && print_info) {
				ata_dev_info(dev, "%s: %s, %s, max %s\n",
					     revbuf,	modelbuf, fwrevbuf,
					     ata_mode_string(xfer_mask));
				ata_dev_info(dev,
					     "%llu sectors, multi %u, CHS %u/%u/%u\n",
					     (unsigned long long)dev->n_sectors,
					     dev->multi_count, dev->cylinders,
					     dev->heads, dev->sectors);
#ifdef MY_ABC_HERE
				ata_dev_info(dev,
					     "SN:%s\n",
					     snbuf);
#endif  
			}
		}

#ifdef MY_DEF_HERE
		if (!glob_match(modelbuf, "SATADOM-SH TYPE D 3SE")) {
			dev->id[ATA_ID_FEATURE_SUPP] &= cpu_to_le16(~(1 << 8));
		}
#endif  

		if (ata_id_has_devslp(dev->id)) {
			u8 *sata_setting = ap->sector_buf;
			int i, j;

			dev->flags |= ATA_DFLAG_DEVSLP;
			err_mask = ata_read_log_page(dev,
						     ATA_LOG_SATA_ID_DEV_DATA,
						     ATA_LOG_SATA_SETTINGS,
						     sata_setting,
						     1);
			if (err_mask)
				ata_dev_dbg(dev,
					    "failed to get Identify Device Data, Emask 0x%x\n",
					    err_mask);
			else
				for (i = 0; i < ATA_LOG_DEVSLP_SIZE; i++) {
					j = ATA_LOG_DEVSLP_OFFSET + i;
					dev->devslp_timing[i] = sata_setting[j];
				}
		}

		dev->cdb_len = 16;
#ifdef MY_ABC_HERE
		if ((dev->horkage & ATA_HORKAGE_NOWCACHE) &&
			!(dev->flags & ATA_DFLAG_NO_WCACHE)) {
			unsigned int err_mask;

			ata_dev_printk(dev, KERN_ERR, "Disable disk write cache");
			dev->flags |= ATA_DFLAG_NO_WCACHE;

			err_mask = ata_dev_set_feature(dev, SETFEATURES_WC_OFF, 0);
			if (err_mask)
				ata_dev_printk(dev, KERN_ERR,
					"failed to dsiable write cache "
					"(err_mask=0x%x)\n", err_mask);
		}
#endif  
	}

	else if (dev->class == ATA_DEV_ATAPI) {
		const char *cdb_intr_string = "";
		const char *atapi_an_string = "";
		const char *dma_dir_string = "";
		u32 sntf;

		rc = atapi_cdb_len(id);
		if ((rc < 12) || (rc > ATAPI_CDB_LEN)) {
			if (ata_msg_warn(ap))
				ata_dev_warn(dev, "unsupported CDB len\n");
			rc = -EINVAL;
			goto err_out_nosup;
		}
		dev->cdb_len = (unsigned int) rc;

		if (atapi_an &&
		    (ap->flags & ATA_FLAG_AN) && ata_id_has_atapi_AN(id) &&
		    (!sata_pmp_attached(ap) ||
		     sata_scr_read(&ap->link, SCR_NOTIFICATION, &sntf) == 0)) {
			 
			err_mask = ata_dev_set_feature(dev,
					SETFEATURES_SATA_ENABLE, SATA_AN);
			if (err_mask)
				ata_dev_err(dev,
					    "failed to enable ATAPI AN (err_mask=0x%x)\n",
					    err_mask);
			else {
				dev->flags |= ATA_DFLAG_AN;
				atapi_an_string = ", ATAPI AN";
			}
		}

		if (ata_id_cdb_intr(dev->id)) {
			dev->flags |= ATA_DFLAG_CDB_INTR;
			cdb_intr_string = ", CDB intr";
		}

		if (atapi_dmadir || (dev->horkage & ATA_HORKAGE_ATAPI_DMADIR) || atapi_id_dmadir(dev->id)) {
			dev->flags |= ATA_DFLAG_DMADIR;
			dma_dir_string = ", DMADIR";
		}

		if (ata_id_has_da(dev->id)) {
			dev->flags |= ATA_DFLAG_DA;
			zpodd_init(dev);
		}

		if (ata_msg_drv(ap) && print_info)
			ata_dev_info(dev,
				     "ATAPI: %s, %s, max %s%s%s%s\n",
				     modelbuf, fwrevbuf,
				     ata_mode_string(xfer_mask),
				     cdb_intr_string, atapi_an_string,
				     dma_dir_string);
	}

	dev->max_sectors = ATA_MAX_SECTORS;
	if (dev->flags & ATA_DFLAG_LBA48)
		dev->max_sectors = ATA_MAX_SECTORS_LBA48;

	if (ata_dev_knobble(dev)) {
		if (ata_msg_drv(ap) && print_info)
			ata_dev_info(dev, "applying bridge limits\n");
		dev->udma_mask &= ATA_UDMA5;
		dev->max_sectors = ATA_MAX_SECTORS;
	}

	if ((dev->class == ATA_DEV_ATAPI) &&
	    (atapi_command_packet_set(id) == TYPE_TAPE)) {
		dev->max_sectors = ATA_MAX_SECTORS_TAPE;
		dev->horkage |= ATA_HORKAGE_STUCK_ERR;
	}

	if (dev->horkage & ATA_HORKAGE_MAX_SEC_128)
		dev->max_sectors = min_t(unsigned int, ATA_MAX_SECTORS_128,
					 dev->max_sectors);

	if (dev->horkage & ATA_HORKAGE_MAX_SEC_1024)
		dev->max_sectors = min_t(unsigned int, ATA_MAX_SECTORS_1024,
					 dev->max_sectors);

	if (dev->horkage & ATA_HORKAGE_MAX_SEC_LBA48)
		dev->max_sectors = ATA_MAX_SECTORS_LBA48;

	if (ap->ops->dev_config)
		ap->ops->dev_config(dev);

	if (dev->horkage & ATA_HORKAGE_DIAGNOSTIC) {
		 
		if (print_info) {
			ata_dev_warn(dev,
"Drive reports diagnostics failure. This may indicate a drive\n");
			ata_dev_warn(dev,
"fault or invalid emulation. Contact drive vendor for information.\n");
		}
	}

	if ((dev->horkage & ATA_HORKAGE_FIRMWARE_WARN) && print_info) {
		ata_dev_warn(dev, "WARNING: device requires firmware update to be fully functional\n");
		ata_dev_warn(dev, "         contact the vendor or visit http://ata.wiki.kernel.org\n");
	}

	return 0;

err_out_nosup:
	if (ata_msg_probe(ap))
		ata_dev_dbg(dev, "%s: EXIT, err\n", __func__);
	return rc;
}

int ata_cable_40wire(struct ata_port *ap)
{
	return ATA_CBL_PATA40;
}

int ata_cable_80wire(struct ata_port *ap)
{
	return ATA_CBL_PATA80;
}

int ata_cable_unknown(struct ata_port *ap)
{
	return ATA_CBL_PATA_UNK;
}

int ata_cable_ignore(struct ata_port *ap)
{
	return ATA_CBL_PATA_IGN;
}

int ata_cable_sata(struct ata_port *ap)
{
	return ATA_CBL_SATA;
}

int ata_bus_probe(struct ata_port *ap)
{
	unsigned int classes[ATA_MAX_DEVICES];
	int tries[ATA_MAX_DEVICES];
	int rc;
	struct ata_device *dev;

	ata_for_each_dev(dev, &ap->link, ALL)
		tries[dev->devno] = ATA_PROBE_MAX_TRIES;

 retry:
	ata_for_each_dev(dev, &ap->link, ALL) {
		 
		dev->pio_mode = XFER_PIO_0;
		dev->dma_mode = 0xff;

		if (ap->ops->set_piomode)
			ap->ops->set_piomode(ap, dev);
	}

	ap->ops->phy_reset(ap);

	ata_for_each_dev(dev, &ap->link, ALL) {
		if (dev->class != ATA_DEV_UNKNOWN)
			classes[dev->devno] = dev->class;
		else
			classes[dev->devno] = ATA_DEV_NONE;

		dev->class = ATA_DEV_UNKNOWN;
	}

	ata_for_each_dev(dev, &ap->link, ALL_REVERSE) {
		if (tries[dev->devno])
			dev->class = classes[dev->devno];

		if (!ata_dev_enabled(dev))
			continue;

		rc = ata_dev_read_id(dev, &dev->class, ATA_READID_POSTRESET,
				     dev->id);
		if (rc)
			goto fail;
	}

	if (ap->ops->cable_detect)
		ap->cbl = ap->ops->cable_detect(ap);

	ata_for_each_dev(dev, &ap->link, ENABLED)
		if (ata_id_is_sata(dev->id))
			ap->cbl = ATA_CBL_SATA;

	ata_for_each_dev(dev, &ap->link, ENABLED) {
		ap->link.eh_context.i.flags |= ATA_EHI_PRINTINFO;
		rc = ata_dev_configure(dev);
		ap->link.eh_context.i.flags &= ~ATA_EHI_PRINTINFO;
		if (rc)
			goto fail;
	}

	rc = ata_set_mode(&ap->link, &dev);
	if (rc)
		goto fail;

	ata_for_each_dev(dev, &ap->link, ENABLED)
		return 0;

	return -ENODEV;

 fail:
	tries[dev->devno]--;

	switch (rc) {
	case -EINVAL:
		 
		tries[dev->devno] = 0;
		break;

	case -ENODEV:
		 
		tries[dev->devno] = min(tries[dev->devno], 1);
	case -EIO:
		if (tries[dev->devno] == 1) {
			 
			sata_down_spd_limit(&ap->link, 0);
			ata_down_xfermask_limit(dev, ATA_DNXFER_PIO);
		}
	}

	if (!tries[dev->devno])
		ata_dev_disable(dev);

	goto retry;
}

#if defined(MY_ABC_HERE)
void syno_ata_present_print(struct ata_port *ap, const char *eventlog)
{
	bool blPresent = false;
	int iHavePres = 0;
	if (ap->nr_pmp_links == 0) {
#ifdef MY_ABC_HERE
#ifdef CONFIG_SYNO_SMBUS_HDD_POWERCTL
		if (0 < g_smbus_hdd_powerctl && (0 <= ap->syno_disk_index)) {
#else  
		if (HAVE_HDD_DETECT(ap->syno_disk_index + 1)) {
#endif  
			if (SYNO_CHECK_HDD_DETECT(ap->syno_disk_index + 1)) {
				ata_port_printk(ap, KERN_ERR, "Disk is present for %s event\n", eventlog);
				blPresent = true;
			} else {
				ata_port_printk(ap, KERN_ERR, "Disk is not present for %s event\n", eventlog);
			}
			iHavePres = 1;
		}
#else  
#ifdef CONFIG_SYNO_SMBUS_HDD_POWERCTL
		if (0 < g_smbus_hdd_powerctl && (0 <= ap->syno_disk_index)) {
#else  
		if (GPIO_UNDEF != SYNO_GET_HDD_PRESENT_PIN(ap->syno_disk_index + 1)) {
#endif  
			if (SYNO_CHECK_HDD_PRESENT(ap->syno_disk_index + 1)) {
				ata_port_printk(ap, KERN_ERR, "Disk is present for %s event\n", eventlog);
				blPresent = true;
			} else {
				ata_port_printk(ap, KERN_ERR, "Disk is not present for %s event\n", eventlog);
			}
			iHavePres = 1;
		}
#endif  
	}

	if (!iHavePres) {
		ata_port_printk(ap, KERN_ERR, "No present pin info for %s event\n", eventlog);
	}
}
EXPORT_SYMBOL(syno_ata_present_print);
#endif  
 
#ifdef MY_ABC_HERE
void sata_print_link_status(struct ata_link *link)
#else  
static void sata_print_link_status(struct ata_link *link)
#endif  
{
	u32 sstatus, scontrol, tmp;

	if (sata_scr_read(link, SCR_STATUS, &sstatus))
		return;
	sata_scr_read(link, SCR_CONTROL, &scontrol);

	if (ata_phys_link_online(link)) {
		tmp = (sstatus >> 4) & 0xf;
		ata_link_info(link, "SATA link up %s (SStatus %X SControl %X)\n",
			      sata_spd_string(tmp), sstatus, scontrol);
	} else {
		ata_link_info(link, "SATA link down (SStatus %X SControl %X)\n",
			      sstatus, scontrol);
#if defined(MY_ABC_HERE)
		if (link->ap) {
			syno_ata_present_print(link->ap, "SATA link down");
		}
#endif  
	}
}

struct ata_device *ata_dev_pair(struct ata_device *adev)
{
	struct ata_link *link = adev->link;
	struct ata_device *pair = &link->device[1 - adev->devno];
	if (!ata_dev_enabled(pair))
		return NULL;
	return pair;
}

int sata_down_spd_limit(struct ata_link *link, u32 spd_limit)
{
	u32 sstatus, spd, mask;
	int rc, bit;

	if (!sata_scr_valid(link))
		return -EOPNOTSUPP;

	rc = sata_scr_read(link, SCR_STATUS, &sstatus);
	if (rc == 0 && ata_sstatus_online(sstatus))
		spd = (sstatus >> 4) & 0xf;
	else
		spd = link->sata_spd;

	mask = link->sata_spd_limit;
	if (mask <= 1)
		return -EINVAL;

	bit = fls(mask) - 1;
	mask &= ~(1 << bit);

	if (spd > 1)
		mask &= (1 << (spd - 1)) - 1;
	else
		mask &= 1;

	if (!mask)
		return -EINVAL;

	if (spd_limit) {
		if (mask & ((1 << spd_limit) - 1))
			mask &= (1 << spd_limit) - 1;
		else {
			bit = ffs(mask) - 1;
			mask = 1 << bit;
		}
	}

	link->sata_spd_limit = mask;

	ata_link_warn(link, "limiting SATA link speed to %s\n",
		      sata_spd_string(fls(mask)));

	return 0;
}

static int __sata_set_spd_needed(struct ata_link *link, u32 *scontrol)
{
	struct ata_link *host_link = &link->ap->link;
	u32 limit, target, spd;

	limit = link->sata_spd_limit;

	if (!ata_is_host_link(link) && host_link->sata_spd)
		limit &= (1 << host_link->sata_spd) - 1;

	if (limit == UINT_MAX)
		target = 0;
	else
		target = fls(limit);

	spd = (*scontrol >> 4) & 0xf;
	*scontrol = (*scontrol & ~0xf0) | ((target & 0xf) << 4);

	return spd != target;
}

static int sata_set_spd_needed(struct ata_link *link)
{
	u32 scontrol;

	if (sata_scr_read(link, SCR_CONTROL, &scontrol))
		return 1;

	return __sata_set_spd_needed(link, &scontrol);
}

int sata_set_spd(struct ata_link *link)
{
	u32 scontrol;
	int rc;

	if ((rc = sata_scr_read(link, SCR_CONTROL, &scontrol)))
		return rc;

	if (!__sata_set_spd_needed(link, &scontrol))
		return 0;

	if ((rc = sata_scr_write(link, SCR_CONTROL, scontrol)))
		return rc;

	return 1;
}

static const struct ata_timing ata_timing[] = {
 
	{ XFER_PIO_0,     70, 290, 240, 600, 165, 150, 0,  600,   0 },
	{ XFER_PIO_1,     50, 290,  93, 383, 125, 100, 0,  383,   0 },
	{ XFER_PIO_2,     30, 290,  40, 330, 100,  90, 0,  240,   0 },
	{ XFER_PIO_3,     30,  80,  70, 180,  80,  70, 0,  180,   0 },
	{ XFER_PIO_4,     25,  70,  25, 120,  70,  25, 0,  120,   0 },
	{ XFER_PIO_5,     15,  65,  25, 100,  65,  25, 0,  100,   0 },
	{ XFER_PIO_6,     10,  55,  20,  80,  55,  20, 0,   80,   0 },

	{ XFER_SW_DMA_0, 120,   0,   0,   0, 480, 480, 50, 960,   0 },
	{ XFER_SW_DMA_1,  90,   0,   0,   0, 240, 240, 30, 480,   0 },
	{ XFER_SW_DMA_2,  60,   0,   0,   0, 120, 120, 20, 240,   0 },

	{ XFER_MW_DMA_0,  60,   0,   0,   0, 215, 215, 20, 480,   0 },
	{ XFER_MW_DMA_1,  45,   0,   0,   0,  80,  50, 5,  150,   0 },
	{ XFER_MW_DMA_2,  25,   0,   0,   0,  70,  25, 5,  120,   0 },
	{ XFER_MW_DMA_3,  25,   0,   0,   0,  65,  25, 5,  100,   0 },
	{ XFER_MW_DMA_4,  25,   0,   0,   0,  55,  20, 5,   80,   0 },

	{ XFER_UDMA_0,     0,   0,   0,   0,   0,   0, 0,    0, 120 },
	{ XFER_UDMA_1,     0,   0,   0,   0,   0,   0, 0,    0,  80 },
	{ XFER_UDMA_2,     0,   0,   0,   0,   0,   0, 0,    0,  60 },
	{ XFER_UDMA_3,     0,   0,   0,   0,   0,   0, 0,    0,  45 },
	{ XFER_UDMA_4,     0,   0,   0,   0,   0,   0, 0,    0,  30 },
	{ XFER_UDMA_5,     0,   0,   0,   0,   0,   0, 0,    0,  20 },
	{ XFER_UDMA_6,     0,   0,   0,   0,   0,   0, 0,    0,  15 },

	{ 0xFF }
};

#define ENOUGH(v, unit)		(((v)-1)/(unit)+1)
#define EZ(v, unit)		((v)?ENOUGH(v, unit):0)

static void ata_timing_quantize(const struct ata_timing *t, struct ata_timing *q, int T, int UT)
{
	q->setup	= EZ(t->setup      * 1000,  T);
	q->act8b	= EZ(t->act8b      * 1000,  T);
	q->rec8b	= EZ(t->rec8b      * 1000,  T);
	q->cyc8b	= EZ(t->cyc8b      * 1000,  T);
	q->active	= EZ(t->active     * 1000,  T);
	q->recover	= EZ(t->recover    * 1000,  T);
	q->dmack_hold	= EZ(t->dmack_hold * 1000,  T);
	q->cycle	= EZ(t->cycle      * 1000,  T);
	q->udma		= EZ(t->udma       * 1000, UT);
}

void ata_timing_merge(const struct ata_timing *a, const struct ata_timing *b,
		      struct ata_timing *m, unsigned int what)
{
	if (what & ATA_TIMING_SETUP  ) m->setup   = max(a->setup,   b->setup);
	if (what & ATA_TIMING_ACT8B  ) m->act8b   = max(a->act8b,   b->act8b);
	if (what & ATA_TIMING_REC8B  ) m->rec8b   = max(a->rec8b,   b->rec8b);
	if (what & ATA_TIMING_CYC8B  ) m->cyc8b   = max(a->cyc8b,   b->cyc8b);
	if (what & ATA_TIMING_ACTIVE ) m->active  = max(a->active,  b->active);
	if (what & ATA_TIMING_RECOVER) m->recover = max(a->recover, b->recover);
	if (what & ATA_TIMING_DMACK_HOLD) m->dmack_hold = max(a->dmack_hold, b->dmack_hold);
	if (what & ATA_TIMING_CYCLE  ) m->cycle   = max(a->cycle,   b->cycle);
	if (what & ATA_TIMING_UDMA   ) m->udma    = max(a->udma,    b->udma);
}

const struct ata_timing *ata_timing_find_mode(u8 xfer_mode)
{
	const struct ata_timing *t = ata_timing;

	while (xfer_mode > t->mode)
		t++;

	if (xfer_mode == t->mode)
		return t;

	WARN_ONCE(true, "%s: unable to find timing for xfer_mode 0x%x\n",
			__func__, xfer_mode);

	return NULL;
}

int ata_timing_compute(struct ata_device *adev, unsigned short speed,
		       struct ata_timing *t, int T, int UT)
{
	const u16 *id = adev->id;
	const struct ata_timing *s;
	struct ata_timing p;

	if (!(s = ata_timing_find_mode(speed)))
		return -EINVAL;

	memcpy(t, s, sizeof(*s));

	if (id[ATA_ID_FIELD_VALID] & 2) {	 
		memset(&p, 0, sizeof(p));

		if (speed >= XFER_PIO_0 && speed < XFER_SW_DMA_0) {
			if (speed <= XFER_PIO_2)
				p.cycle = p.cyc8b = id[ATA_ID_EIDE_PIO];
			else if ((speed <= XFER_PIO_4) ||
				 (speed == XFER_PIO_5 && !ata_id_is_cfa(id)))
				p.cycle = p.cyc8b = id[ATA_ID_EIDE_PIO_IORDY];
		} else if (speed >= XFER_MW_DMA_0 && speed <= XFER_MW_DMA_2)
			p.cycle = id[ATA_ID_EIDE_DMA_MIN];

		ata_timing_merge(&p, t, t, ATA_TIMING_CYCLE | ATA_TIMING_CYC8B);
	}

	ata_timing_quantize(t, t, T, UT);

	if (speed > XFER_PIO_6) {
		ata_timing_compute(adev, adev->pio_mode, &p, T, UT);
		ata_timing_merge(&p, t, t, ATA_TIMING_ALL);
	}

	if (t->act8b + t->rec8b < t->cyc8b) {
		t->act8b += (t->cyc8b - (t->act8b + t->rec8b)) / 2;
		t->rec8b = t->cyc8b - t->act8b;
	}

	if (t->active + t->recover < t->cycle) {
		t->active += (t->cycle - (t->active + t->recover)) / 2;
		t->recover = t->cycle - t->active;
	}

	if (t->active + t->recover > t->cycle)
		t->cycle = t->active + t->recover;

	return 0;
}

u8 ata_timing_cycle2mode(unsigned int xfer_shift, int cycle)
{
	u8 base_mode = 0xff, last_mode = 0xff;
	const struct ata_xfer_ent *ent;
	const struct ata_timing *t;

	for (ent = ata_xfer_tbl; ent->shift >= 0; ent++)
		if (ent->shift == xfer_shift)
			base_mode = ent->base;

	for (t = ata_timing_find_mode(base_mode);
	     t && ata_xfer_mode2shift(t->mode) == xfer_shift; t++) {
		unsigned short this_cycle;

		switch (xfer_shift) {
		case ATA_SHIFT_PIO:
		case ATA_SHIFT_MWDMA:
			this_cycle = t->cycle;
			break;
		case ATA_SHIFT_UDMA:
			this_cycle = t->udma;
			break;
		default:
			return 0xff;
		}

		if (cycle > this_cycle)
			break;

		last_mode = t->mode;
	}

	return last_mode;
}

int ata_down_xfermask_limit(struct ata_device *dev, unsigned int sel)
{
	char buf[32];
	unsigned long orig_mask, xfer_mask;
	unsigned long pio_mask, mwdma_mask, udma_mask;
	int quiet, highbit;

	quiet = !!(sel & ATA_DNXFER_QUIET);
	sel &= ~ATA_DNXFER_QUIET;

	xfer_mask = orig_mask = ata_pack_xfermask(dev->pio_mask,
						  dev->mwdma_mask,
						  dev->udma_mask);
	ata_unpack_xfermask(xfer_mask, &pio_mask, &mwdma_mask, &udma_mask);

	switch (sel) {
	case ATA_DNXFER_PIO:
		highbit = fls(pio_mask) - 1;
		pio_mask &= ~(1 << highbit);
		break;

	case ATA_DNXFER_DMA:
		if (udma_mask) {
			highbit = fls(udma_mask) - 1;
			udma_mask &= ~(1 << highbit);
			if (!udma_mask)
				return -ENOENT;
		} else if (mwdma_mask) {
			highbit = fls(mwdma_mask) - 1;
			mwdma_mask &= ~(1 << highbit);
			if (!mwdma_mask)
				return -ENOENT;
		}
		break;

	case ATA_DNXFER_40C:
		udma_mask &= ATA_UDMA_MASK_40C;
		break;

	case ATA_DNXFER_FORCE_PIO0:
		pio_mask &= 1;
	case ATA_DNXFER_FORCE_PIO:
		mwdma_mask = 0;
		udma_mask = 0;
		break;

	default:
		BUG();
	}

	xfer_mask &= ata_pack_xfermask(pio_mask, mwdma_mask, udma_mask);

	if (!(xfer_mask & ATA_MASK_PIO) || xfer_mask == orig_mask)
		return -ENOENT;

	if (!quiet) {
		if (xfer_mask & (ATA_MASK_MWDMA | ATA_MASK_UDMA))
			snprintf(buf, sizeof(buf), "%s:%s",
				 ata_mode_string(xfer_mask),
				 ata_mode_string(xfer_mask & ATA_MASK_PIO));
		else
			snprintf(buf, sizeof(buf), "%s",
				 ata_mode_string(xfer_mask));

		ata_dev_warn(dev, "limiting speed to %s\n", buf);
	}

	ata_unpack_xfermask(xfer_mask, &dev->pio_mask, &dev->mwdma_mask,
			    &dev->udma_mask);

	return 0;
}

static int ata_dev_set_mode(struct ata_device *dev)
{
	struct ata_port *ap = dev->link->ap;
	struct ata_eh_context *ehc = &dev->link->eh_context;
	const bool nosetxfer = dev->horkage & ATA_HORKAGE_NOSETXFER;
	const char *dev_err_whine = "";
	int ign_dev_err = 0;
	unsigned int err_mask = 0;
	int rc;

	dev->flags &= ~ATA_DFLAG_PIO;
	if (dev->xfer_shift == ATA_SHIFT_PIO)
		dev->flags |= ATA_DFLAG_PIO;

	if (nosetxfer && ap->flags & ATA_FLAG_SATA && ata_id_is_sata(dev->id))
		dev_err_whine = " (SET_XFERMODE skipped)";
	else {
		if (nosetxfer)
			ata_dev_warn(dev,
				     "NOSETXFER but PATA detected - can't "
				     "skip SETXFER, might malfunction\n");
		err_mask = ata_dev_set_xfermode(dev);
	}

	if (err_mask & ~AC_ERR_DEV)
		goto fail;

	ehc->i.flags |= ATA_EHI_POST_SETMODE;
	rc = ata_dev_revalidate(dev, ATA_DEV_UNKNOWN, 0);
	ehc->i.flags &= ~ATA_EHI_POST_SETMODE;
	if (rc)
		return rc;

	if (dev->xfer_shift == ATA_SHIFT_PIO) {
		 
		if (ata_id_is_cfa(dev->id))
			ign_dev_err = 1;
		 
		if (ata_id_major_version(dev->id) == 0 &&
					dev->pio_mode <= XFER_PIO_2)
			ign_dev_err = 1;
		 
		if (!ata_id_has_iordy(dev->id) && dev->pio_mode <= XFER_PIO_2)
			ign_dev_err = 1;
	}
	 
	if (dev->xfer_shift == ATA_SHIFT_MWDMA &&
	    dev->dma_mode == XFER_MW_DMA_0 &&
	    (dev->id[63] >> 8) & 1)
		ign_dev_err = 1;

	if (dev->xfer_mode == ata_xfer_mask2mode(ata_id_xfermask(dev->id)))
		ign_dev_err = 1;

	if (err_mask & AC_ERR_DEV) {
		if (!ign_dev_err)
			goto fail;
		else
			dev_err_whine = " (device error ignored)";
	}

	DPRINTK("xfer_shift=%u, xfer_mode=0x%x\n",
		dev->xfer_shift, (int)dev->xfer_mode);

	ata_dev_info(dev, "configured for %s%s\n",
		     ata_mode_string(ata_xfer_mode2mask(dev->xfer_mode)),
		     dev_err_whine);

	return 0;

 fail:
	ata_dev_err(dev, "failed to set xfermode (err_mask=0x%x)\n", err_mask);
	return -EIO;
}

int ata_do_set_mode(struct ata_link *link, struct ata_device **r_failed_dev)
{
	struct ata_port *ap = link->ap;
	struct ata_device *dev;
	int rc = 0, used_dma = 0, found = 0;

	ata_for_each_dev(dev, link, ENABLED) {
		unsigned long pio_mask, dma_mask;
		unsigned int mode_mask;

		mode_mask = ATA_DMA_MASK_ATA;
		if (dev->class == ATA_DEV_ATAPI)
			mode_mask = ATA_DMA_MASK_ATAPI;
		else if (ata_id_is_cfa(dev->id))
			mode_mask = ATA_DMA_MASK_CFA;

		ata_dev_xfermask(dev);
		ata_force_xfermask(dev);

		pio_mask = ata_pack_xfermask(dev->pio_mask, 0, 0);

		if (libata_dma_mask & mode_mask)
			dma_mask = ata_pack_xfermask(0, dev->mwdma_mask,
						     dev->udma_mask);
		else
			dma_mask = 0;

		dev->pio_mode = ata_xfer_mask2mode(pio_mask);
		dev->dma_mode = ata_xfer_mask2mode(dma_mask);

		found = 1;
		if (ata_dma_enabled(dev))
			used_dma = 1;
	}
	if (!found)
		goto out;

	ata_for_each_dev(dev, link, ENABLED) {
		if (dev->pio_mode == 0xff) {
			ata_dev_warn(dev, "no PIO support\n");
			rc = -EINVAL;
			goto out;
		}

		dev->xfer_mode = dev->pio_mode;
		dev->xfer_shift = ATA_SHIFT_PIO;
		if (ap->ops->set_piomode)
			ap->ops->set_piomode(ap, dev);
	}

	ata_for_each_dev(dev, link, ENABLED) {
		if (!ata_dma_enabled(dev))
			continue;

		dev->xfer_mode = dev->dma_mode;
		dev->xfer_shift = ata_xfer_mode2shift(dev->dma_mode);
		if (ap->ops->set_dmamode)
			ap->ops->set_dmamode(ap, dev);
	}

	ata_for_each_dev(dev, link, ENABLED) {
		rc = ata_dev_set_mode(dev);
		if (rc)
			goto out;
	}

	if (used_dma && (ap->host->flags & ATA_HOST_SIMPLEX))
		ap->host->simplex_claimed = ap;

 out:
	if (rc)
		*r_failed_dev = dev;
	return rc;
}

int ata_wait_ready(struct ata_link *link, unsigned long deadline,
		   int (*check_ready)(struct ata_link *link))
{
	unsigned long start = jiffies;
	unsigned long nodev_deadline;
	int warned = 0;

	if (link->ap->host->flags & ATA_HOST_PARALLEL_SCAN)
		nodev_deadline = ata_deadline(start, ATA_TMOUT_FF_WAIT_LONG);
	else
		nodev_deadline = ata_deadline(start, ATA_TMOUT_FF_WAIT);

	WARN_ON(link == link->ap->slave_link);

	if (time_after(nodev_deadline, deadline))
		nodev_deadline = deadline;

	while (1) {
		unsigned long now = jiffies;
		int ready, tmp;

		ready = tmp = check_ready(link);
		if (ready > 0)
			return 0;

		if (ready == -ENODEV) {
			if (ata_link_online(link))
				ready = 0;
			else if ((link->ap->flags & ATA_FLAG_SATA) &&
				 !ata_link_offline(link) &&
				 time_before(now, nodev_deadline))
				ready = 0;
		}

		if (ready)
			return ready;
		if (time_after(now, deadline))
			return -EBUSY;

		if (!warned && time_after(now, start + 5 * HZ) &&
		    (deadline - now > 3 * HZ)) {
			ata_link_warn(link,
				"link is slow to respond, please be patient "
				"(ready=%d)\n", tmp);
			warned = 1;
		}

		ata_msleep(link->ap, 50);
	}
}

int ata_wait_after_reset(struct ata_link *link, unsigned long deadline,
				int (*check_ready)(struct ata_link *link))
{
	ata_msleep(link->ap, ATA_WAIT_AFTER_RESET);

	return ata_wait_ready(link, deadline, check_ready);
}

int sata_link_debounce(struct ata_link *link, const unsigned long *params,
		       unsigned long deadline)
{
	unsigned long interval = params[0];
	unsigned long duration = params[1];
	unsigned long last_jiffies, t;
	u32 last, cur;
	int rc;

	t = ata_deadline(jiffies, params[2]);
	if (time_before(t, deadline))
		deadline = t;

	if ((rc = sata_scr_read(link, SCR_STATUS, &cur)))
		return rc;
	cur &= 0xf;

	last = cur;
	last_jiffies = jiffies;

	while (1) {
		ata_msleep(link->ap, interval);
		if ((rc = sata_scr_read(link, SCR_STATUS, &cur)))
			return rc;
		cur &= 0xf;

		if (cur == last) {
			if (cur == 1 && time_before(jiffies, deadline))
				continue;
			if (time_after(jiffies,
				       ata_deadline(last_jiffies, duration)))
				return 0;
			continue;
		}

		last = cur;
		last_jiffies = jiffies;

		if (time_after(jiffies, deadline))
			return -EPIPE;
	}
}

int sata_link_resume(struct ata_link *link, const unsigned long *params,
		     unsigned long deadline)
{
	int tries = ATA_LINK_RESUME_TRIES;
	u32 scontrol, serror;
	int rc;

	if ((rc = sata_scr_read(link, SCR_CONTROL, &scontrol)))
		return rc;

	do {
		scontrol = (scontrol & 0x0f0) | 0x300;
		if ((rc = sata_scr_write(link, SCR_CONTROL, scontrol)))
			return rc;
		 
		ata_msleep(link->ap, 200);

		if ((rc = sata_scr_read(link, SCR_CONTROL, &scontrol)))
			return rc;
	} while ((scontrol & 0xf0f) != 0x300 && --tries);

#if defined(CONFIG_SYNO_LSP_RTD1619)
#if defined(CONFIG_AHCI_RTK)
	rtk_sata_phy_poweron(link);
#endif  
#endif  
	if ((scontrol & 0xf0f) != 0x300) {
		ata_link_warn(link, "failed to resume link (SControl %X)\n",
			     scontrol);
		return 0;
	}

	if (tries < ATA_LINK_RESUME_TRIES)
		ata_link_warn(link, "link resume succeeded after %d retries\n",
			      ATA_LINK_RESUME_TRIES - tries);

	if ((rc = sata_link_debounce(link, params, deadline)))
		return rc;

	if (!(rc = sata_scr_read(link, SCR_ERROR, &serror)))
		rc = sata_scr_write(link, SCR_ERROR, serror);

	return rc != -EINVAL ? rc : 0;
}

int sata_link_scr_lpm(struct ata_link *link, enum ata_lpm_policy policy,
		      bool spm_wakeup)
{
	struct ata_eh_context *ehc = &link->eh_context;
	bool woken_up = false;
	u32 scontrol;
	int rc;

	rc = sata_scr_read(link, SCR_CONTROL, &scontrol);
	if (rc)
		return rc;

	switch (policy) {
	case ATA_LPM_MAX_POWER:
		 
		scontrol |= (0x7 << 8);
		 
		if (spm_wakeup) {
			scontrol |= (0x4 << 12);
			woken_up = true;
		}
		break;
	case ATA_LPM_MED_POWER:
		 
		scontrol &= ~(0x1 << 8);
		scontrol |= (0x6 << 8);
		break;
	case ATA_LPM_MIN_POWER:
		if (ata_link_nr_enabled(link) > 0)
			 
			scontrol &= ~(0x7 << 8);
		else {
			 
			scontrol &= ~0xf;
			scontrol |= (0x1 << 2);
		}
		break;
	default:
		WARN_ON(1);
	}

	rc = sata_scr_write(link, SCR_CONTROL, scontrol);
	if (rc)
		return rc;

	if (woken_up)
		msleep(10);

	ehc->i.serror &= ~SERR_PHYRDY_CHG;
	return sata_scr_write(link, SCR_ERROR, SERR_PHYRDY_CHG);
}

int ata_std_prereset(struct ata_link *link, unsigned long deadline)
{
	struct ata_port *ap = link->ap;
	struct ata_eh_context *ehc = &link->eh_context;
	const unsigned long *timing = sata_ehc_deb_timing(ehc);
	int rc;

	if (ehc->i.action & ATA_EH_HARDRESET)
		return 0;

	if (ap->flags & ATA_FLAG_SATA) {
		rc = sata_link_resume(link, timing, deadline);
		 
		if (rc && rc != -EOPNOTSUPP)
			ata_link_warn(link,
				      "failed to resume link for reset (errno=%d)\n",
				      rc);
	}

	if (ata_phys_link_offline(link))
		ehc->i.action &= ~ATA_EH_SOFTRESET;

	return 0;
}

int sata_link_hardreset(struct ata_link *link, const unsigned long *timing,
			unsigned long deadline,
			bool *online, int (*check_ready)(struct ata_link *))
{
	u32 scontrol;
	int rc;

	DPRINTK("ENTER\n");

	if (online)
		*online = false;

	if (sata_set_spd_needed(link)) {
		 
		if ((rc = sata_scr_read(link, SCR_CONTROL, &scontrol)))
			goto out;

		scontrol = (scontrol & 0x0f0) | 0x304;

		if ((rc = sata_scr_write(link, SCR_CONTROL, scontrol)))
			goto out;

		sata_set_spd(link);
	}

	if ((rc = sata_scr_read(link, SCR_CONTROL, &scontrol)))
		goto out;

	scontrol = (scontrol & 0x0f0) | 0x301;

	if ((rc = sata_scr_write_flush(link, SCR_CONTROL, scontrol)))
		goto out;

	ata_msleep(link->ap, 1);

	rc = sata_link_resume(link, timing, deadline);
	if (rc)
		goto out;
	 
	if (ata_phys_link_offline(link))
		goto out;

	if (online)
		*online = true;

	if (sata_pmp_supported(link->ap) && ata_is_host_link(link)) {
		 
		if (check_ready) {
			unsigned long pmp_deadline;

			pmp_deadline = ata_deadline(jiffies,
						    ATA_TMOUT_PMP_SRST_WAIT);
#ifdef MY_ABC_HERE
			 
			if (time_before(pmp_deadline, deadline))
#else
			if (time_after(pmp_deadline, deadline))
#endif  
				pmp_deadline = deadline;
			ata_wait_ready(link, pmp_deadline, check_ready);
		}
		rc = -EAGAIN;
		goto out;
	}

	rc = 0;
	if (check_ready)
		rc = ata_wait_ready(link, deadline, check_ready);
 out:
	if (rc && rc != -EAGAIN) {
		 
		if (online)
			*online = false;
		ata_link_err(link, "COMRESET failed (errno=%d)\n", rc);
	}
	DPRINTK("EXIT, rc=%d\n", rc);
	return rc;
}

int sata_std_hardreset(struct ata_link *link, unsigned int *class,
		       unsigned long deadline)
{
	const unsigned long *timing = sata_ehc_deb_timing(&link->eh_context);
	bool online;
	int rc;

	rc = sata_link_hardreset(link, timing, deadline, &online, NULL);
	return online ? -EAGAIN : rc;
}

void ata_std_postreset(struct ata_link *link, unsigned int *classes)
{
	u32 serror;

	DPRINTK("ENTER\n");

	if (!sata_scr_read(link, SCR_ERROR, &serror))
		sata_scr_write(link, SCR_ERROR, serror);

	sata_print_link_status(link);

	DPRINTK("EXIT\n");
}

static int ata_dev_same_device(struct ata_device *dev, unsigned int new_class,
			       const u16 *new_id)
{
	const u16 *old_id = dev->id;
	unsigned char model[2][ATA_ID_PROD_LEN + 1];
	unsigned char serial[2][ATA_ID_SERNO_LEN + 1];

	if (dev->class != new_class) {
		ata_dev_info(dev, "class mismatch %d != %d\n",
			     dev->class, new_class);
		return 0;
	}

	ata_id_c_string(old_id, model[0], ATA_ID_PROD, sizeof(model[0]));
	ata_id_c_string(new_id, model[1], ATA_ID_PROD, sizeof(model[1]));
	ata_id_c_string(old_id, serial[0], ATA_ID_SERNO, sizeof(serial[0]));
	ata_id_c_string(new_id, serial[1], ATA_ID_SERNO, sizeof(serial[1]));

	if (strcmp(model[0], model[1])) {
		ata_dev_info(dev, "model number mismatch '%s' != '%s'\n",
			     model[0], model[1]);
		return 0;
	}

	if (strcmp(serial[0], serial[1])) {
		ata_dev_info(dev, "serial number mismatch '%s' != '%s'\n",
			     serial[0], serial[1]);
		return 0;
	}

	return 1;
}

int ata_dev_reread_id(struct ata_device *dev, unsigned int readid_flags)
{
	unsigned int class = dev->class;
	u16 *id = (void *)dev->link->ap->sector_buf;
	int rc;

	rc = ata_dev_read_id(dev, &class, readid_flags, id);
	if (rc)
		return rc;

	if (!ata_dev_same_device(dev, class, id))
		return -ENODEV;

	memcpy(dev->id, id, sizeof(id[0]) * ATA_ID_WORDS);
	return 0;
}

int ata_dev_revalidate(struct ata_device *dev, unsigned int new_class,
		       unsigned int readid_flags)
{
	u64 n_sectors = dev->n_sectors;
	u64 n_native_sectors = dev->n_native_sectors;
	int rc;

	if (!ata_dev_enabled(dev))
		return -ENODEV;

	if (ata_class_enabled(new_class) &&
	    new_class != ATA_DEV_ATA &&
	    new_class != ATA_DEV_ATAPI &&
	    new_class != ATA_DEV_ZAC &&
	    new_class != ATA_DEV_SEMB) {
		ata_dev_info(dev, "class mismatch %u != %u\n",
			     dev->class, new_class);
		rc = -ENODEV;
		goto fail;
	}

	rc = ata_dev_reread_id(dev, readid_flags);
	if (rc)
		goto fail;

	rc = ata_dev_configure(dev);
	if (rc)
		goto fail;

	if (dev->class != ATA_DEV_ATA || !n_sectors ||
	    dev->n_sectors == n_sectors)
		return 0;

	ata_dev_warn(dev, "n_sectors mismatch %llu != %llu\n",
		     (unsigned long long)n_sectors,
		     (unsigned long long)dev->n_sectors);

	if (dev->n_native_sectors == n_native_sectors &&
	    dev->n_sectors > n_sectors && dev->n_sectors == n_native_sectors) {
		ata_dev_warn(dev,
			     "new n_sectors matches native, probably "
			     "late HPA unlock, n_sectors updated\n");
		 
		return 0;
	}

	if (dev->n_native_sectors == n_native_sectors &&
	    dev->n_sectors < n_sectors && n_sectors == n_native_sectors &&
	    !(dev->horkage & ATA_HORKAGE_BROKEN_HPA)) {
		ata_dev_warn(dev,
			     "old n_sectors matches native, probably "
			     "late HPA lock, will try to unlock HPA\n");
		 
		dev->flags |= ATA_DFLAG_UNLOCK_HPA;
		rc = -EIO;
	} else
		rc = -ENODEV;

	dev->n_native_sectors = n_native_sectors;
	dev->n_sectors = n_sectors;
 fail:
	ata_dev_err(dev, "revalidation failed (errno=%d)\n", rc);
	return rc;
}
#ifdef MY_ABC_HERE
struct ata_blacklist_entry ata_device_blacklist [] = {
#else  
struct ata_blacklist_entry {
	const char *model_num;
	const char *model_rev;
	unsigned long horkage;
};

static const struct ata_blacklist_entry ata_device_blacklist [] = {
#endif  
	 
	{ "WDC AC11000H",	NULL,		ATA_HORKAGE_NODMA },
	{ "WDC AC22100H",	NULL,		ATA_HORKAGE_NODMA },
	{ "WDC AC32500H",	NULL,		ATA_HORKAGE_NODMA },
	{ "WDC AC33100H",	NULL,		ATA_HORKAGE_NODMA },
	{ "WDC AC31600H",	NULL,		ATA_HORKAGE_NODMA },
	{ "WDC AC32100H",	"24.09P07",	ATA_HORKAGE_NODMA },
	{ "WDC AC23200L",	"21.10N21",	ATA_HORKAGE_NODMA },
	{ "Compaq CRD-8241B", 	NULL,		ATA_HORKAGE_NODMA },
	{ "CRD-8400B",		NULL, 		ATA_HORKAGE_NODMA },
	{ "CRD-848[02]B",	NULL,		ATA_HORKAGE_NODMA },
	{ "CRD-84",		NULL,		ATA_HORKAGE_NODMA },
	{ "SanDisk SDP3B",	NULL,		ATA_HORKAGE_NODMA },
	{ "SanDisk SDP3B-64",	NULL,		ATA_HORKAGE_NODMA },
	{ "SANYO CD-ROM CRD",	NULL,		ATA_HORKAGE_NODMA },
	{ "HITACHI CDR-8",	NULL,		ATA_HORKAGE_NODMA },
	{ "HITACHI CDR-8[34]35",NULL,		ATA_HORKAGE_NODMA },
	{ "Toshiba CD-ROM XM-6202B", NULL,	ATA_HORKAGE_NODMA },
	{ "TOSHIBA CD-ROM XM-1702BC", NULL,	ATA_HORKAGE_NODMA },
	{ "CD-532E-A", 		NULL,		ATA_HORKAGE_NODMA },
	{ "E-IDE CD-ROM CR-840",NULL,		ATA_HORKAGE_NODMA },
	{ "CD-ROM Drive/F5A",	NULL,		ATA_HORKAGE_NODMA },
	{ "WPI CDD-820", 	NULL,		ATA_HORKAGE_NODMA },
	{ "SAMSUNG CD-ROM SC-148C", NULL,	ATA_HORKAGE_NODMA },
	{ "SAMSUNG CD-ROM SC",	NULL,		ATA_HORKAGE_NODMA },
	{ "ATAPI CD-ROM DRIVE 40X MAXIMUM",NULL,ATA_HORKAGE_NODMA },
	{ "_NEC DV5800A", 	NULL,		ATA_HORKAGE_NODMA },
	{ "SAMSUNG CD-ROM SN-124", "N001",	ATA_HORKAGE_NODMA },
	{ "Seagate STT20000A", NULL,		ATA_HORKAGE_NODMA },
	{ " 2GB ATA Flash Disk", "ADMA428M",	ATA_HORKAGE_NODMA },
	 
	{ "Config  Disk",	NULL,		ATA_HORKAGE_DISABLE },

	{ "TORiSAN DVD-ROM DRD-N216", NULL,	ATA_HORKAGE_MAX_SEC_128 },
	{ "QUANTUM DAT    DAT72-000", NULL,	ATA_HORKAGE_ATAPI_MOD16_DMA },
	{ "Slimtype DVD A  DS8A8SH", NULL,	ATA_HORKAGE_MAX_SEC_LBA48 },
	{ "Slimtype DVD A  DS8A9SH", NULL,	ATA_HORKAGE_MAX_SEC_LBA48 },

	{ "ST380013AS",		"3.20",		ATA_HORKAGE_MAX_SEC_1024 },

	{ "LITEON CX1-JB*-HP",	NULL,		ATA_HORKAGE_MAX_SEC_1024 },

	{ "WDC WD740ADFD-00",	NULL,		ATA_HORKAGE_NONCQ },
	{ "WDC WD740ADFD-00NLR1", NULL,		ATA_HORKAGE_NONCQ, },
	 
	{ "FUJITSU MHT2060BH",	NULL,		ATA_HORKAGE_NONCQ },
	 
	{ "Maxtor *",		"BANC*",	ATA_HORKAGE_NONCQ },
	{ "Maxtor 7V300F0",	"VA111630",	ATA_HORKAGE_NONCQ },
	{ "ST380817AS",		"3.42",		ATA_HORKAGE_NONCQ },
	{ "ST3160023AS",	"3.42",		ATA_HORKAGE_NONCQ },
	{ "OCZ CORE_SSD",	"02.10104",	ATA_HORKAGE_NONCQ },

	{ "ST31500341AS",	"SD1[5-9]",	ATA_HORKAGE_NONCQ |
						ATA_HORKAGE_FIRMWARE_WARN },

	{ "ST31000333AS",	"SD1[5-9]",	ATA_HORKAGE_NONCQ |
						ATA_HORKAGE_FIRMWARE_WARN },

	{ "ST3640[36]23AS",	"SD1[5-9]",	ATA_HORKAGE_NONCQ |
						ATA_HORKAGE_FIRMWARE_WARN },

	{ "ST3320[68]13AS",	"SD1[5-9]",	ATA_HORKAGE_NONCQ |
						ATA_HORKAGE_FIRMWARE_WARN },

	{ "ST1000LM024 HN-M101MBB", "2AR10001",	ATA_HORKAGE_BROKEN_FPDMA_AA },
	{ "ST1000LM024 HN-M101MBB", "2BA30001",	ATA_HORKAGE_BROKEN_FPDMA_AA },
	{ "VB0250EAVER",	"HPG7",		ATA_HORKAGE_BROKEN_FPDMA_AA },

	{ "HTS541060G9SA00",    "MB3OC60D",     ATA_HORKAGE_NONCQ, },
	{ "HTS541080G9SA00",    "MB4OC60D",     ATA_HORKAGE_NONCQ, },
	{ "HTS541010G9SA00",    "MBZOC60D",     ATA_HORKAGE_NONCQ, },

	{ "C300-CTFDDAC128MAG",	"0001",		ATA_HORKAGE_NONCQ, },

	{ "HDS724040KLSA80",	"KFAOA20N",	ATA_HORKAGE_BROKEN_HPA, },
	{ "WDC WD3200JD-00KLB0", "WD-WCAMR1130137", ATA_HORKAGE_BROKEN_HPA },
	{ "WDC WD2500JD-00HBB0", "WD-WMAL71490727", ATA_HORKAGE_BROKEN_HPA },
	{ "MAXTOR 6L080L4",	"A93.0500",	ATA_HORKAGE_BROKEN_HPA },

	{ "OCZ-VERTEX",		    "1.30",	ATA_HORKAGE_BROKEN_HPA },

	{ "ST340823A",		NULL,		ATA_HORKAGE_HPA_SIZE, },
	{ "ST320413A",		NULL,		ATA_HORKAGE_HPA_SIZE, },
	{ "ST310211A",		NULL,		ATA_HORKAGE_HPA_SIZE, },

	{ "QUANTUM FIREBALLlct10 05", "A03.0900", ATA_HORKAGE_IVB, },
	 
	{ "TSSTcorp CDDVDW SH-S202[HJN]", "SB0[01]",  ATA_HORKAGE_IVB, },

	{ "MTRON MSP-SATA*",		NULL,	ATA_HORKAGE_BRIDGE_OK, },
	{ "BUFFALO HD-QSU2/R5",		NULL,	ATA_HORKAGE_BRIDGE_OK, },

	{ "WD My Book",			NULL,	ATA_HORKAGE_1_5_GBPS, },
	{ "Seagate FreeAgent GoFlex",	NULL,	ATA_HORKAGE_1_5_GBPS, },

#ifdef MY_ABC_HERE
	 
	{ "Hitachi HUA723030ALA640",	NULL,	ATA_HORKAGE_1_5_GBPS, },
	{ "Hitachi HUA723030ALA641",	NULL,	ATA_HORKAGE_1_5_GBPS, },
	{ "Hitachi HUA723020ALA640",	NULL,	ATA_HORKAGE_1_5_GBPS, },
	{ "Hitachi HUA723020ALA641",	NULL,	ATA_HORKAGE_1_5_GBPS, },

	{ "Hitachi HDS723030ALA640",	NULL,	ATA_HORKAGE_1_5_GBPS, },
	{ "Hitachi HDS723020BLA642",	NULL,	ATA_HORKAGE_1_5_GBPS, },
	{ "Hitachi HDS723015BLA642",	NULL,	ATA_HORKAGE_1_5_GBPS, },

	{ "Hitachi HDS5C3030ALA630",	NULL,	ATA_HORKAGE_1_5_GBPS, },
	{ "Hitachi HDS5C3020ALA632",	NULL,	ATA_HORKAGE_1_5_GBPS, },
	{ "Hitachi HDS5C3015ALA632",	NULL,	ATA_HORKAGE_1_5_GBPS, },
#endif  
	{ "PIONEER DVD-RW  DVRTD08",	NULL,	ATA_HORKAGE_NOSETXFER },
	{ "PIONEER DVD-RW  DVRTD08A",	NULL,	ATA_HORKAGE_NOSETXFER },
	{ "PIONEER DVD-RW  DVR-215",	NULL,	ATA_HORKAGE_NOSETXFER },
	{ "PIONEER DVD-RW  DVR-212D",	NULL,	ATA_HORKAGE_NOSETXFER },
	{ "PIONEER DVD-RW  DVR-216D",	NULL,	ATA_HORKAGE_NOSETXFER },

	{ "Micron_M500_*",		NULL,	ATA_HORKAGE_NO_NCQ_TRIM |
						ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "Crucial_CT*M500*",		NULL,	ATA_HORKAGE_NO_NCQ_TRIM |
						ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "Micron_M5[15]0_*",		"MU01",	ATA_HORKAGE_NO_NCQ_TRIM |
						ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "Crucial_CT*M550*",		"MU01",	ATA_HORKAGE_NO_NCQ_TRIM |
						ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "Crucial_CT*MX100*",		"MU01",	ATA_HORKAGE_NO_NCQ_TRIM |
						ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "Samsung SSD 8*",		NULL,	ATA_HORKAGE_NO_NCQ_TRIM |
						ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "FCCT*M500*",			NULL,	ATA_HORKAGE_NO_NCQ_TRIM |
						ATA_HORKAGE_ZERO_AFTER_TRIM, },

	{ "SuperSSpeed S238*",		NULL,	ATA_HORKAGE_NOTRIM, },

	{ "INTEL*SSDSC2MH*",		NULL,	0, },

	{ "Micron*",			NULL,	ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "Crucial*",			NULL,	ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "INTEL*SSD*", 		NULL,	ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "SSD*INTEL*",			NULL,	ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "Samsung*SSD*",		NULL,	ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "SAMSUNG*SSD*",		NULL,	ATA_HORKAGE_ZERO_AFTER_TRIM, },
	{ "ST[1248][0248]0[FH]*",	NULL,	ATA_HORKAGE_ZERO_AFTER_TRIM, },

	{ "WDC WD800JD-*",		NULL,	ATA_HORKAGE_WD_BROKEN_LPM },
	{ "WDC WD1200JD-*",		NULL,	ATA_HORKAGE_WD_BROKEN_LPM },
	{ "WDC WD1600JD-*",		NULL,	ATA_HORKAGE_WD_BROKEN_LPM },
	{ "WDC WD2000JD-*",		NULL,	ATA_HORKAGE_WD_BROKEN_LPM },
	{ "WDC WD2500JD-*",		NULL,	ATA_HORKAGE_WD_BROKEN_LPM },
	{ "WDC WD3000JD-*",		NULL,	ATA_HORKAGE_WD_BROKEN_LPM },
	{ "WDC WD3200JD-*",		NULL,	ATA_HORKAGE_WD_BROKEN_LPM },

	{ }
};

static unsigned long ata_dev_blacklisted(const struct ata_device *dev)
{
	unsigned char model_num[ATA_ID_PROD_LEN + 1];
	unsigned char model_rev[ATA_ID_FW_REV_LEN + 1];
	const struct ata_blacklist_entry *ad = ata_device_blacklist;

	ata_id_c_string(dev->id, model_num, ATA_ID_PROD, sizeof(model_num));
	ata_id_c_string(dev->id, model_rev, ATA_ID_FW_REV, sizeof(model_rev));

	while (ad->model_num) {
		if (glob_match(ad->model_num, model_num)) {
			if (ad->model_rev == NULL)
				return ad->horkage;
			if (glob_match(ad->model_rev, model_rev))
				return ad->horkage;
		}
		ad++;
	}
	return 0;
}

static int ata_dma_blacklisted(const struct ata_device *dev)
{
	 
	if ((dev->link->ap->flags & ATA_FLAG_PIO_POLLING) &&
	    (dev->flags & ATA_DFLAG_CDB_INTR))
		return 1;
	return (dev->horkage & ATA_HORKAGE_NODMA) ? 1 : 0;
}

static int ata_is_40wire(struct ata_device *dev)
{
	if (dev->horkage & ATA_HORKAGE_IVB)
		return ata_drive_40wire_relaxed(dev->id);
	return ata_drive_40wire(dev->id);
}

static int cable_is_40wire(struct ata_port *ap)
{
	struct ata_link *link;
	struct ata_device *dev;

	if (ap->cbl == ATA_CBL_PATA40)
		return 1;

	if (ap->cbl == ATA_CBL_PATA80 || ap->cbl == ATA_CBL_SATA)
		return 0;

	if (ap->cbl == ATA_CBL_PATA40_SHORT)
		return 0;

	ata_for_each_link(link, ap, EDGE) {
		ata_for_each_dev(dev, link, ENABLED) {
			if (!ata_is_40wire(dev))
				return 0;
		}
	}
	return 1;
}

static void ata_dev_xfermask(struct ata_device *dev)
{
	struct ata_link *link = dev->link;
	struct ata_port *ap = link->ap;
	struct ata_host *host = ap->host;
	unsigned long xfer_mask;

	xfer_mask = ata_pack_xfermask(ap->pio_mask,
				      ap->mwdma_mask, ap->udma_mask);

	xfer_mask &= ata_pack_xfermask(dev->pio_mask,
				       dev->mwdma_mask, dev->udma_mask);
	xfer_mask &= ata_id_xfermask(dev->id);

	if (ata_dev_pair(dev)) {
		 
		xfer_mask &= ~(0x03 << (ATA_SHIFT_PIO + 5));
		 
		xfer_mask &= ~(0x03 << (ATA_SHIFT_MWDMA + 3));
	}

	if (ata_dma_blacklisted(dev)) {
		xfer_mask &= ~(ATA_MASK_MWDMA | ATA_MASK_UDMA);
		ata_dev_warn(dev,
			     "device is on DMA blacklist, disabling DMA\n");
	}

	if ((host->flags & ATA_HOST_SIMPLEX) &&
	    host->simplex_claimed && host->simplex_claimed != ap) {
		xfer_mask &= ~(ATA_MASK_MWDMA | ATA_MASK_UDMA);
		ata_dev_warn(dev,
			     "simplex DMA is claimed by other device, disabling DMA\n");
	}

	if (ap->flags & ATA_FLAG_NO_IORDY)
		xfer_mask &= ata_pio_mask_no_iordy(dev);

	if (ap->ops->mode_filter)
		xfer_mask = ap->ops->mode_filter(dev, xfer_mask);

	if (xfer_mask & (0xF8 << ATA_SHIFT_UDMA))
		 
		if (cable_is_40wire(ap)) {
			ata_dev_warn(dev,
				     "limited to UDMA/33 due to 40-wire cable\n");
			xfer_mask &= ~(0xF8 << ATA_SHIFT_UDMA);
		}

	ata_unpack_xfermask(xfer_mask, &dev->pio_mask,
			    &dev->mwdma_mask, &dev->udma_mask);
}

static unsigned int ata_dev_set_xfermode(struct ata_device *dev)
{
	struct ata_taskfile tf;
	unsigned int err_mask;

	DPRINTK("set features - xfer mode\n");

	ata_tf_init(dev, &tf);
	tf.command = ATA_CMD_SET_FEATURES;
	tf.feature = SETFEATURES_XFER;
	tf.flags |= ATA_TFLAG_ISADDR | ATA_TFLAG_DEVICE | ATA_TFLAG_POLLING;
	tf.protocol = ATA_PROT_NODATA;
	 
	if (ata_pio_need_iordy(dev))
		tf.nsect = dev->xfer_mode;
	 
 	else if (ata_id_has_iordy(dev->id))
		tf.nsect = 0x01;
	else  
		return 0;

	err_mask = ata_exec_internal(dev, &tf, NULL, DMA_NONE, NULL, 0, 15000);

	DPRINTK("EXIT, err_mask=%x\n", err_mask);
	return err_mask;
}

unsigned int ata_dev_set_feature(struct ata_device *dev, u8 enable, u8 feature)
{
	struct ata_taskfile tf;
	unsigned int err_mask;

	DPRINTK("set features - SATA features\n");

	ata_tf_init(dev, &tf);
	tf.command = ATA_CMD_SET_FEATURES;
	tf.feature = enable;
	tf.flags |= ATA_TFLAG_ISADDR | ATA_TFLAG_DEVICE;
	tf.protocol = ATA_PROT_NODATA;
	tf.nsect = feature;

	err_mask = ata_exec_internal(dev, &tf, NULL, DMA_NONE, NULL, 0, 0);

	DPRINTK("EXIT, err_mask=%x\n", err_mask);
	return err_mask;
}
EXPORT_SYMBOL_GPL(ata_dev_set_feature);

static unsigned int ata_dev_init_params(struct ata_device *dev,
					u16 heads, u16 sectors)
{
	struct ata_taskfile tf;
	unsigned int err_mask;

	if (sectors < 1 || sectors > 255 || heads < 1 || heads > 16)
		return AC_ERR_INVALID;

	DPRINTK("init dev params \n");

	ata_tf_init(dev, &tf);
	tf.command = ATA_CMD_INIT_DEV_PARAMS;
	tf.flags |= ATA_TFLAG_ISADDR | ATA_TFLAG_DEVICE;
	tf.protocol = ATA_PROT_NODATA;
	tf.nsect = sectors;
	tf.device |= (heads - 1) & 0x0f;  

	err_mask = ata_exec_internal(dev, &tf, NULL, DMA_NONE, NULL, 0, 0);
	 
	if (err_mask == AC_ERR_DEV && (tf.feature & ATA_ABORTED))
		err_mask = 0;

	DPRINTK("EXIT, err_mask=%x\n", err_mask);
	return err_mask;
}

void ata_sg_clean(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct scatterlist *sg = qc->sg;
	int dir = qc->dma_dir;

	WARN_ON_ONCE(sg == NULL);

	VPRINTK("unmapping %u sg elements\n", qc->n_elem);

	if (qc->n_elem)
		dma_unmap_sg(ap->dev, sg, qc->orig_n_elem, dir);

	qc->flags &= ~ATA_QCFLAG_DMAMAP;
	qc->sg = NULL;
}

int atapi_check_dma(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;

	if (!(qc->dev->horkage & ATA_HORKAGE_ATAPI_MOD16_DMA) &&
	    unlikely(qc->nbytes & 15))
		return 1;

	if (ap->ops->check_atapi_dma)
		return ap->ops->check_atapi_dma(qc);

	return 0;
}

int ata_std_qc_defer(struct ata_queued_cmd *qc)
{
	struct ata_link *link = qc->dev->link;

	if (qc->tf.protocol == ATA_PROT_NCQ) {
		if (!ata_tag_valid(link->active_tag))
			return 0;
	} else {
		if (!ata_tag_valid(link->active_tag) && !link->sactive)
			return 0;
	}

	return ATA_DEFER_LINK;
}

void ata_noop_qc_prep(struct ata_queued_cmd *qc) { }

void ata_sg_init(struct ata_queued_cmd *qc, struct scatterlist *sg,
		 unsigned int n_elem)
{
	qc->sg = sg;
	qc->n_elem = n_elem;
	qc->cursg = qc->sg;
}

static int ata_sg_setup(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	unsigned int n_elem;

	VPRINTK("ENTER, ata%u\n", ap->print_id);

	n_elem = dma_map_sg(ap->dev, qc->sg, qc->n_elem, qc->dma_dir);
	if (n_elem < 1)
		return -1;

	DPRINTK("%d sg elements mapped\n", n_elem);
	qc->orig_n_elem = qc->n_elem;
	qc->n_elem = n_elem;
	qc->flags |= ATA_QCFLAG_DMAMAP;

	return 0;
}

void swap_buf_le16(u16 *buf, unsigned int buf_words)
{
#ifdef __BIG_ENDIAN
	unsigned int i;

	for (i = 0; i < buf_words; i++)
		buf[i] = le16_to_cpu(buf[i]);
#endif  
}

struct ata_queued_cmd *ata_qc_new_init(struct ata_device *dev, int tag)
{
	struct ata_port *ap = dev->link->ap;
	struct ata_queued_cmd *qc;

	if (unlikely(ap->pflags & ATA_PFLAG_FROZEN))
		return NULL;

	if (ap->flags & ATA_FLAG_SAS_HOST) {
		tag = ata_sas_allocate_tag(ap);
		if (tag < 0)
			return NULL;
	}

	qc = __ata_qc_from_tag(ap, tag);
	qc->tag = tag;
	qc->scsicmd = NULL;
	qc->ap = ap;
	qc->dev = dev;

	ata_qc_reinit(qc);

	return qc;
}

void ata_qc_free(struct ata_queued_cmd *qc)
{
	struct ata_port *ap;
	unsigned int tag;

	WARN_ON_ONCE(qc == NULL);  
	ap = qc->ap;

	qc->flags = 0;
	tag = qc->tag;
	if (likely(ata_tag_valid(tag))) {
		qc->tag = ATA_TAG_POISON;
		if (ap->flags & ATA_FLAG_SAS_HOST)
			ata_sas_free_tag(tag, ap);
	}
}

#ifdef MY_ABC_HERE
static inline unsigned int syno_ata_latency_bucket_offset_get(const u64 u64Latency)
{
	unsigned int uOffset = 0;

	if ( 0x1F < u64Latency) {
		uOffset += 1;
	}

	if (unlikely( 0x3FF < u64Latency)) {
		uOffset += 1;
	}

	if (unlikely( 0x7FFF < u64Latency)) {
		uOffset += 1;
	}

	return uOffset;
}

static void syno_ata_cmd_latency_preprocess(struct ata_queued_cmd *qc,
											struct ata_link *link,
											u64 u64AtaIntrTime)
{
	u8 u8QcType = 0;
	unsigned int uBucketOffset = 0;
	u64 u64StepOffset = 0;
	u64 u64CmdRespTime = 0;

	u8QcType = qc->qc_stat.u8QcType;
	if (unlikely(2 < u8QcType)) {
		u8QcType = 0;
	}

	u64CmdRespTime = (u64AtaIntrTime - qc->qc_stat.u64IssueTime);;

	link->ata_latency.u16TotalCplCmdCnt += 1;
	link->ata_latency.u16CplCmdCnt[u8QcType] += 1;
	link->latency_stat.u64TotalCount[u8QcType] += 1;
	link->latency_stat.u64TotalBytes[u8QcType] += qc->nbytes;
	link->latency_stat.u64TotalRespTime[u8QcType] += u64CmdRespTime;
	if ((0 == link->ata_latency.u64FirstCmdStartTime)
			|| (qc->qc_stat.u64IssueTime < link->ata_latency.u64FirstCmdStartTime)) {
		link->ata_latency.u64FirstCmdStartTime = qc->qc_stat.u64IssueTime;
	}

	u64StepOffset = (u64CmdRespTime >> 15);
	 
	uBucketOffset = syno_ata_latency_bucket_offset_get(u64StepOffset);
	u64StepOffset >>= (5 * uBucketOffset);
	if (unlikely(31 < u64StepOffset)) {
		uBucketOffset = (SYNO_LATENCY_BUCKETS_END - 1);
		u64StepOffset = 31;
	}
	link->ata_latency.u64RespTimeBuckets[u8QcType][uBucketOffset][u64StepOffset] += 1;
	return;
}
#endif  

void __ata_qc_complete(struct ata_queued_cmd *qc)
{
	struct ata_port *ap;
	struct ata_link *link;
#ifdef MY_ABC_HERE
	struct Scsi_Host *host;
	unsigned long flags;
	struct req_iterator iter;
	struct bio_vec bv;
	unsigned char *start;
#endif  

	WARN_ON_ONCE(qc == NULL);  
	WARN_ON_ONCE(!(qc->flags & ATA_QCFLAG_ACTIVE));
	ap = qc->ap;
	link = qc->dev->link;

	if (likely(qc->flags & ATA_QCFLAG_DMAMAP)) {
		ata_sg_clean(qc);
#ifdef MY_ABC_HERE
		if ( 1 == syno_ata_pattern_check && !(qc->flags & ATA_QCFLAG_FAILED) && qc->scsicmd && qc->scsicmd->request) {
			host = ap->scsi_host;
			__rq_for_each_bio(iter.bio, qc->scsicmd->request) {
				if (!(iter.bio->bi_rw & REQ_SYNO_PATTERN_CHECK)) {
					continue;
				}
				bio_for_each_segment(bv, iter.bio, iter.iter) {
					char *data = bvec_kmap_irq(&bv, &flags);
					start = memchr_inv(data, gSynoPatternCheckCharacter , bv.bv_len);
					if (!start) {
						continue;
					} else {
						printk("Syno Pattern Check: %s:%d, host: %d, bio_sec: %lu\n",
						__func__, __LINE__,host->host_no, iter.bio->bi_iter.bi_sector);
						break;
					}
					bvec_kunmap_irq(data, &flags);
				}
			}
		}
#endif  
	}
	 
	if (qc->tf.protocol == ATA_PROT_NCQ) {
		link->sactive &= ~(1 << qc->tag);
		if (!link->sactive)
			ap->nr_active_links--;
	} else {
		link->active_tag = ATA_TAG_POISON;
		ap->nr_active_links--;
	}

	if (unlikely(qc->flags & ATA_QCFLAG_CLEAR_EXCL &&
		     ap->excl_link == link))
		ap->excl_link = NULL;

#ifdef MY_ABC_HERE
	syno_ata_cmd_latency_preprocess(qc, link, ap->u64AtaIntrTime);
#endif  

	qc->flags &= ~ATA_QCFLAG_ACTIVE;
	ap->qc_active &= ~(1 << qc->tag);

	qc->complete_fn(qc);
}

static void fill_result_tf(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;

	qc->result_tf.flags = qc->tf.flags;
	ap->ops->qc_fill_rtf(qc);
}

static void ata_verify_xfer(struct ata_queued_cmd *qc)
{
	struct ata_device *dev = qc->dev;

	if (ata_is_nodata(qc->tf.protocol))
		return;

	if ((dev->mwdma_mask || dev->udma_mask) && ata_is_pio(qc->tf.protocol))
		return;

	dev->flags &= ~ATA_DFLAG_DUBIOUS_XFER;
}

void ata_qc_complete(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
#ifdef MY_ABC_HERE
	struct ata_taskfile *tf = &qc->tf;
#endif  

	if (ap->ops->error_handler) {
		struct ata_device *dev = qc->dev;
		struct ata_eh_info *ehi = &dev->link->eh_info;

		if (unlikely(qc->err_mask))
			qc->flags |= ATA_QCFLAG_FAILED;

		if (unlikely(ata_tag_internal(qc->tag))) {
			fill_result_tf(qc);
			trace_ata_qc_complete_internal(qc);
			__ata_qc_complete(qc);
			return;
		}

		if (unlikely(qc->flags & ATA_QCFLAG_FAILED)) {
			fill_result_tf(qc);

#ifdef MY_ABC_HERE
			if ((IS_SYNO_PMP_CMD(tf) && NULL == qc->scsicmd)) {
				__ata_qc_complete(qc);
			} else {
#endif  
#ifdef MY_ABC_HERE
				if (IS_SYNO_SPINUP_CMD(qc))
				{
					__ata_qc_complete(qc);
				} else {
#endif  
					trace_ata_qc_complete_failed(qc);
					ata_qc_schedule_eh(qc);
#ifdef MY_ABC_HERE
				}
#endif  
#ifdef MY_ABC_HERE
			}
#endif  
			return;
		}

		WARN_ON_ONCE(ap->pflags & ATA_PFLAG_FROZEN);

		if (qc->flags & ATA_QCFLAG_RESULT_TF)
			fill_result_tf(qc);

		trace_ata_qc_complete_done(qc);
		 
		switch (qc->tf.command) {
		case ATA_CMD_SET_FEATURES:
			if (qc->tf.feature != SETFEATURES_WC_ON &&
			    qc->tf.feature != SETFEATURES_WC_OFF)
				break;
			 
		case ATA_CMD_INIT_DEV_PARAMS:  
		case ATA_CMD_SET_MULTI:  
			 
			ehi->dev_action[dev->devno] |= ATA_EH_REVALIDATE;
			ata_port_schedule_eh(ap);
			break;

		case ATA_CMD_SLEEP:
			dev->flags |= ATA_DFLAG_SLEEPING;
			break;
		}

		if (unlikely(dev->flags & ATA_DFLAG_DUBIOUS_XFER))
			ata_verify_xfer(qc);

		__ata_qc_complete(qc);
	} else {
		if (qc->flags & ATA_QCFLAG_EH_SCHEDULED)
			return;

		if (qc->err_mask || qc->flags & ATA_QCFLAG_RESULT_TF)
			fill_result_tf(qc);

		__ata_qc_complete(qc);
	}
}
#ifdef MY_ABC_HERE
 
static void syno_ata_latency_calculate(struct ata_link *link, u64 u64AtaIntrTime)
{
	int iType					= 0;
	unsigned int uBucketOffset	= 0;
	u64 u64CmdStartTime			= 0;
	u64 u64CmdLatencyTime		= 0;
	u64 u64StepOffset			= 0;

	if (0 == link->ata_latency.u16TotalCplCmdCnt) {
		goto END;
	}

	u64CmdStartTime = (link->ata_latency.u64FirstCmdStartTime
						> link->ata_latency.u64LastIntrTime)
									? link->ata_latency.u64FirstCmdStartTime
									: link->ata_latency.u64LastIntrTime;
	u64CmdLatencyTime = div_u64((u64AtaIntrTime - u64CmdStartTime),
										link->ata_latency.u16TotalCplCmdCnt);
	 
	u64StepOffset = (u64CmdLatencyTime >> 15);
	 
	uBucketOffset = syno_ata_latency_bucket_offset_get(u64StepOffset);
	u64StepOffset >>= (5 * uBucketOffset);
	if (unlikely(31 < u64StepOffset)) {
		uBucketOffset = (SYNO_LATENCY_BUCKETS_END - 1);
		u64StepOffset = 31;
	}

	for (iType = 0 ; iType < SYNO_LATENCY_TYPE_COUNT; iType++) {
		link->latency_stat.u64TotalTime[iType]
				+= (u64CmdLatencyTime * link->ata_latency.u16CplCmdCnt[iType]);

		link->ata_latency.u64TimeBuckets[iType][uBucketOffset][u64StepOffset]
									+= link->ata_latency.u16CplCmdCnt[iType];
	}

	if (!link->sactive) {
		 
		link->ata_latency.u64BatchComplete = u64AtaIntrTime;
		link->latency_stat.u64TotalBatchCount += 1;
		link->latency_stat.u64TotalBatchTime +=
					(u64AtaIntrTime - link->ata_latency.u64BatchIssue);
	}

	link->ata_latency.u64LastIntrTime = u64AtaIntrTime;
END:
	 
	link->ata_latency.u64FirstCmdStartTime = 0;
	link->ata_latency.u16TotalCplCmdCnt = 0;
	for (iType = 0 ; iType < SYNO_LATENCY_TYPE_COUNT; iType++) {
		link->ata_latency.u16CplCmdCnt[iType] = 0;
	}
	return;
}
#endif  

int ata_qc_complete_multiple(struct ata_port *ap, u32 qc_active)
{
	int nr_done = 0;
	u32 done_mask;

#ifdef MY_ABC_HERE
	struct ata_link *link = NULL;

	ap->u64AtaIntrTime = cpu_clock(0);
#endif  
	done_mask = ap->qc_active ^ qc_active;

	if (unlikely(done_mask & qc_active)) {
		ata_port_err(ap, "illegal qc_active transition (%08x->%08x)\n",
			     ap->qc_active, qc_active);
		return -EINVAL;
	}

	while (done_mask) {
		struct ata_queued_cmd *qc;
		unsigned int tag = __ffs(done_mask);

		qc = ata_qc_from_tag(ap, tag);
		if (qc) {
			ata_qc_complete(qc);
			nr_done++;
		}
		done_mask &= ~(1 << tag);
	}
#ifdef MY_ABC_HERE
	ata_for_each_link(link, ap, HOST_FIRST) {
		syno_ata_latency_calculate(link, ap->u64AtaIntrTime);
	}
#endif  

	return nr_done;
}

#ifdef MY_DEF_HERE
static int syno_ata_read_qc_delay = 50;
int ata_qc_complete_multiple_delay(struct ata_port *ap, u32 qc_active)
{
	int nr_done = 0;
	u32 done_mask;
	int is_first_read_qc = 1;

#ifdef MY_ABC_HERE
	struct ata_link *link = NULL;

	ap->u64AtaIntrTime = cpu_clock(0);
#endif  
	done_mask = ap->qc_active ^ qc_active;

	if (unlikely(done_mask & qc_active)) {
		ata_port_err(ap, "illegal qc_active transition (%08x->%08x)\n",
			     ap->qc_active, qc_active);
		return -EINVAL;
	}

	while (done_mask) {
		struct ata_queued_cmd *qc;
		unsigned int tag = __ffs(done_mask);

		qc = ata_qc_from_tag(ap, tag);
		if (qc) {
			if (is_first_read_qc && qc->dma_dir == DMA_FROM_DEVICE) {
				udelay(syno_ata_read_qc_delay);
				is_first_read_qc = 0;
			}
			ata_qc_complete(qc);
			nr_done++;
		}
		done_mask &= ~(1 << tag);
	}
#ifdef MY_ABC_HERE
	ata_for_each_link(link, ap, HOST_FIRST) {
		syno_ata_latency_calculate(link, ap->u64AtaIntrTime);
	}
#endif  

	return nr_done;
}
EXPORT_SYMBOL(ata_qc_complete_multiple_delay);
module_param(syno_ata_read_qc_delay, int, 0644);
#endif  

#ifdef MY_ABC_HERE
static inline void syno_ata_latency_start(struct ata_queued_cmd *qc, u8 blIsBatchHead)
{
	struct ata_link *link = qc->dev->link;

	qc->qc_stat.u64IssueTime = cpu_clock(0);

	if (1 == blIsBatchHead) {
		 
		link->ata_latency.u64BatchIssue = qc->qc_stat.u64IssueTime;
	}

	return;
}
#endif  

void ata_qc_issue(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct ata_link *link = qc->dev->link;
	u8 prot = qc->tf.protocol;
#ifdef MY_ABC_HERE
	u8 blIsBatchHead = 0;
#endif  

	WARN_ON_ONCE(ap->ops->error_handler && ata_tag_valid(link->active_tag));

	if (ata_is_ncq(prot)) {
		WARN_ON_ONCE(link->sactive & (1 << qc->tag));

		if (!link->sactive)
#ifdef MY_ABC_HERE
		{
			 
#endif  
			ap->nr_active_links++;
#ifdef MY_ABC_HERE
			blIsBatchHead = 1;
		}
#endif  
		link->sactive |= 1 << qc->tag;
	} else {
#ifdef MY_ABC_HERE
		blIsBatchHead = 1;
#endif  
		WARN_ON_ONCE(link->sactive);

		ap->nr_active_links++;
		link->active_tag = qc->tag;
	}

	qc->flags |= ATA_QCFLAG_ACTIVE;
	ap->qc_active |= 1 << qc->tag;

	if (WARN_ON_ONCE(ata_is_data(prot) &&
			 (!qc->sg || !qc->n_elem || !qc->nbytes)))
		goto sys_err;

	if (ata_is_dma(prot) || (ata_is_pio(prot) &&
				 (ap->flags & ATA_FLAG_PIO_DMA)))
		if (ata_sg_setup(qc))
			goto sys_err;

	if (unlikely(qc->dev->flags & ATA_DFLAG_SLEEPING)) {
		link->eh_info.action |= ATA_EH_RESET;
		ata_ehi_push_desc(&link->eh_info, "waking up from sleep");
		ata_link_abort(link);
		return;
	}

	ap->ops->qc_prep(qc);
	trace_ata_qc_issue(qc);
#ifdef MY_ABC_HERE
	syno_ata_latency_start(qc, blIsBatchHead);
#endif  

	qc->err_mask |= ap->ops->qc_issue(qc);
	if (unlikely(qc->err_mask))
		goto err;
	return;

sys_err:
	qc->err_mask |= AC_ERR_SYSTEM;
err:
	ata_qc_complete(qc);
}

int sata_scr_valid(struct ata_link *link)
{
	struct ata_port *ap = link->ap;

	return (ap->flags & ATA_FLAG_SATA) && ap->ops->scr_read;
}

int sata_scr_read(struct ata_link *link, int reg, u32 *val)
{
	if (ata_is_host_link(link)) {
		if (sata_scr_valid(link))
			return link->ap->ops->scr_read(link, reg, val);
		return -EOPNOTSUPP;
	}

	return sata_pmp_scr_read(link, reg, val);
}

int sata_scr_write(struct ata_link *link, int reg, u32 val)
{
	if (ata_is_host_link(link)) {
		if (sata_scr_valid(link))
			return link->ap->ops->scr_write(link, reg, val);
		return -EOPNOTSUPP;
	}

	return sata_pmp_scr_write(link, reg, val);
}

int sata_scr_write_flush(struct ata_link *link, int reg, u32 val)
{
	if (ata_is_host_link(link)) {
		int rc;

		if (sata_scr_valid(link)) {
			rc = link->ap->ops->scr_write(link, reg, val);
			if (rc == 0)
				rc = link->ap->ops->scr_read(link, reg, &val);
			return rc;
		}
		return -EOPNOTSUPP;
	}

	return sata_pmp_scr_write(link, reg, val);
}

bool ata_phys_link_online(struct ata_link *link)
{
	u32 sstatus;

	if (sata_scr_read(link, SCR_STATUS, &sstatus) == 0 &&
	    ata_sstatus_online(sstatus))
		return true;
	return false;
}

bool ata_phys_link_offline(struct ata_link *link)
{
	u32 sstatus;

	if (sata_scr_read(link, SCR_STATUS, &sstatus) == 0 &&
	    !ata_sstatus_online(sstatus))
		return true;
	return false;
}

bool ata_link_online(struct ata_link *link)
{
	struct ata_link *slave = link->ap->slave_link;

	WARN_ON(link == slave);	 

	return ata_phys_link_online(link) ||
		(slave && ata_phys_link_online(slave));
}

bool ata_link_offline(struct ata_link *link)
{
	struct ata_link *slave = link->ap->slave_link;

	WARN_ON(link == slave);	 

	return ata_phys_link_offline(link) &&
		(!slave || ata_phys_link_offline(slave));
}

#ifdef CONFIG_PM
static void ata_port_request_pm(struct ata_port *ap, pm_message_t mesg,
				unsigned int action, unsigned int ehi_flags,
				bool async)
{
	struct ata_link *link;
	unsigned long flags;

	if (ap->pflags & ATA_PFLAG_PM_PENDING) {
		ata_port_wait_eh(ap);
		WARN_ON(ap->pflags & ATA_PFLAG_PM_PENDING);
	}

	spin_lock_irqsave(ap->lock, flags);

	ap->pm_mesg = mesg;
	ap->pflags |= ATA_PFLAG_PM_PENDING;
	ata_for_each_link(link, ap, HOST_FIRST) {
		link->eh_info.action |= action;
		link->eh_info.flags |= ehi_flags;
	}

	ata_port_schedule_eh(ap);

	spin_unlock_irqrestore(ap->lock, flags);

	if (!async) {
		ata_port_wait_eh(ap);
		WARN_ON(ap->pflags & ATA_PFLAG_PM_PENDING);
	}
}

static const unsigned int ata_port_suspend_ehi = ATA_EHI_QUIET
						 | ATA_EHI_NO_AUTOPSY
						 | ATA_EHI_NO_RECOVERY;

static void ata_port_suspend(struct ata_port *ap, pm_message_t mesg)
{
	ata_port_request_pm(ap, mesg, 0, ata_port_suspend_ehi, false);
}

static void ata_port_suspend_async(struct ata_port *ap, pm_message_t mesg)
{
	ata_port_request_pm(ap, mesg, 0, ata_port_suspend_ehi, true);
}

static int ata_port_pm_suspend(struct device *dev)
{
	struct ata_port *ap = to_ata_port(dev);

	if (pm_runtime_suspended(dev))
		return 0;

	ata_port_suspend(ap, PMSG_SUSPEND);
	return 0;
}

static int ata_port_pm_freeze(struct device *dev)
{
	struct ata_port *ap = to_ata_port(dev);

	if (pm_runtime_suspended(dev))
		return 0;

	ata_port_suspend(ap, PMSG_FREEZE);
	return 0;
}

static int ata_port_pm_poweroff(struct device *dev)
{
	ata_port_suspend(to_ata_port(dev), PMSG_HIBERNATE);
	return 0;
}

static const unsigned int ata_port_resume_ehi = ATA_EHI_NO_AUTOPSY
						| ATA_EHI_QUIET;

static void ata_port_resume(struct ata_port *ap, pm_message_t mesg)
{
	ata_port_request_pm(ap, mesg, ATA_EH_RESET, ata_port_resume_ehi, false);
}

static void ata_port_resume_async(struct ata_port *ap, pm_message_t mesg)
{
	ata_port_request_pm(ap, mesg, ATA_EH_RESET, ata_port_resume_ehi, true);
}

static int ata_port_pm_resume(struct device *dev)
{
	ata_port_resume_async(to_ata_port(dev), PMSG_RESUME);
	pm_runtime_disable(dev);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);
	return 0;
}

static int ata_port_runtime_idle(struct device *dev)
{
	struct ata_port *ap = to_ata_port(dev);
	struct ata_link *link;
	struct ata_device *adev;

	ata_for_each_link(link, ap, HOST_FIRST) {
		ata_for_each_dev(adev, link, ENABLED)
			if (adev->class == ATA_DEV_ATAPI &&
			    !zpodd_dev_enabled(adev))
				return -EBUSY;
	}

	return 0;
}

static int ata_port_runtime_suspend(struct device *dev)
{
	ata_port_suspend(to_ata_port(dev), PMSG_AUTO_SUSPEND);
	return 0;
}

static int ata_port_runtime_resume(struct device *dev)
{
	ata_port_resume(to_ata_port(dev), PMSG_AUTO_RESUME);
	return 0;
}

static const struct dev_pm_ops ata_port_pm_ops = {
	.suspend = ata_port_pm_suspend,
	.resume = ata_port_pm_resume,
	.freeze = ata_port_pm_freeze,
	.thaw = ata_port_pm_resume,
	.poweroff = ata_port_pm_poweroff,
	.restore = ata_port_pm_resume,

	.runtime_suspend = ata_port_runtime_suspend,
	.runtime_resume = ata_port_runtime_resume,
	.runtime_idle = ata_port_runtime_idle,
};

void ata_sas_port_suspend(struct ata_port *ap)
{
	ata_port_suspend_async(ap, PMSG_SUSPEND);
}
EXPORT_SYMBOL_GPL(ata_sas_port_suspend);

void ata_sas_port_resume(struct ata_port *ap)
{
	ata_port_resume_async(ap, PMSG_RESUME);
}
EXPORT_SYMBOL_GPL(ata_sas_port_resume);

int ata_host_suspend(struct ata_host *host, pm_message_t mesg)
{
	host->dev->power.power_state = mesg;
	return 0;
}

void ata_host_resume(struct ata_host *host)
{
	host->dev->power.power_state = PMSG_ON;
}
#endif

struct device_type ata_port_type = {
	.name = "ata_port",
#ifdef CONFIG_PM
	.pm = &ata_port_pm_ops,
#endif
};

void ata_dev_init(struct ata_device *dev)
{
	struct ata_link *link = ata_dev_phys_link(dev);
	struct ata_port *ap = link->ap;
	unsigned long flags;

	link->sata_spd_limit = link->hw_sata_spd_limit;
	link->sata_spd = 0;

#if defined(CONFIG_SYNO_LSP_RTD1619)
#if defined(CONFIG_AHCI_RTK)
	sata_set_spd(link);
#endif  
#endif  
	 
	spin_lock_irqsave(ap->lock, flags);
	dev->flags &= ~ATA_DFLAG_INIT_MASK;
	dev->horkage = 0;
	spin_unlock_irqrestore(ap->lock, flags);

	memset((void *)dev + ATA_DEVICE_CLEAR_BEGIN, 0,
	       ATA_DEVICE_CLEAR_END - ATA_DEVICE_CLEAR_BEGIN);
	dev->pio_mask = UINT_MAX;
	dev->mwdma_mask = UINT_MAX;
	dev->udma_mask = UINT_MAX;
}

void ata_link_init(struct ata_port *ap, struct ata_link *link, int pmp)
{
	int i;

	memset((void *)link + ATA_LINK_CLEAR_BEGIN, 0,
	       ATA_LINK_CLEAR_END - ATA_LINK_CLEAR_BEGIN);

	link->ap = ap;
	link->pmp = pmp;
	link->active_tag = ATA_TAG_POISON;
	link->hw_sata_spd_limit = UINT_MAX;

	for (i = 0; i < ATA_MAX_DEVICES; i++) {
		struct ata_device *dev = &link->device[i];

		dev->link = link;
		dev->devno = dev - link->device;
#ifdef CONFIG_ATA_ACPI
		dev->gtf_filter = ata_acpi_gtf_filter;
#endif
#ifdef MY_ABC_HERE
		dev->ulSpinupState = 0;
		dev->ulLastCmd = 0;
		dev->iCheckPwr = 0;
#endif  
		ata_dev_init(dev);
	}
}

int sata_link_init_spd(struct ata_link *link)
{
	u8 spd;
	int rc;

	rc = sata_scr_read(link, SCR_CONTROL, &link->saved_scontrol);
	if (rc)
		return rc;

	spd = (link->saved_scontrol >> 4) & 0xf;
	if (spd)
		link->hw_sata_spd_limit &= (1 << spd) - 1;

	ata_force_link_limits(link);

	link->sata_spd_limit = link->hw_sata_spd_limit;

	return 0;
}

struct ata_port *ata_port_alloc(struct ata_host *host)
{
	struct ata_port *ap;

	DPRINTK("ENTER\n");

	ap = kzalloc(sizeof(*ap), GFP_KERNEL);
	if (!ap)
		return NULL;

	ap->pflags |= ATA_PFLAG_INITIALIZING | ATA_PFLAG_FROZEN;
	ap->lock = &host->lock;
	ap->print_id = -1;
	ap->local_port_no = -1;
	ap->host = host;
	ap->dev = host->dev;

#if defined(ATA_VERBOSE_DEBUG)
	 
	ap->msg_enable = 0x00FF;
#elif defined(ATA_DEBUG)
	ap->msg_enable = ATA_MSG_DRV | ATA_MSG_INFO | ATA_MSG_CTL | ATA_MSG_WARN | ATA_MSG_ERR;
#else
	ap->msg_enable = ATA_MSG_DRV | ATA_MSG_ERR | ATA_MSG_WARN;
#endif

	mutex_init(&ap->scsi_scan_mutex);
	INIT_DELAYED_WORK(&ap->hotplug_task, ata_scsi_hotplug);
#ifdef MY_ABC_HERE
	INIT_DELAYED_WORK(&ap->syno_pmp_task, ata_syno_pmp_hotplug);
#endif  
	INIT_WORK(&ap->scsi_rescan_task, ata_scsi_dev_rescan);
	INIT_LIST_HEAD(&ap->eh_done_q);
	init_waitqueue_head(&ap->eh_wait_q);
	init_completion(&ap->park_req_pending);
	init_timer_deferrable(&ap->fastdrain_timer);
	ap->fastdrain_timer.function = ata_eh_fastdrain_timerfn;
	ap->fastdrain_timer.data = (unsigned long)ap;

	ap->cbl = ATA_CBL_NONE;

	ata_link_init(ap, &ap->link, 0);

#ifdef ATA_IRQ_TRAP
	ap->stats.unhandled_irq = 1;
	ap->stats.idle_irq = 1;
#endif

#ifdef MY_ABC_HERE
	ap->error_handling = 0;
#endif  

	ata_sff_port_init(ap);
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
	ap->syno_disk_index = -1;
	 
	syno_libata_index_get(ap->scsi_host, 0, 0, 0);
	klist_add_tail(&ap->ata_port_list, &syno_ata_port_head);
#endif  
	return ap;
}

static void ata_host_release(struct device *gendev, void *res)
{
	struct ata_host *host = dev_get_drvdata(gendev);
	int i;

	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap = host->ports[i];

		if (!ap)
			continue;

		if (ap->scsi_host)
			scsi_host_put(ap->scsi_host);

		kfree(ap->pmp_link);
		kfree(ap->slave_link);
		kfree(ap);
		host->ports[i] = NULL;
	}

	dev_set_drvdata(gendev, NULL);
}

struct ata_host *ata_host_alloc(struct device *dev, int max_ports)
{
	struct ata_host *host;
	size_t sz;
	int i;
	DPRINTK("ENTER\n");

	if (!devres_open_group(dev, NULL, GFP_KERNEL))
		return NULL;

	sz = sizeof(struct ata_host) + (max_ports + 1) * sizeof(void *);
	 
	host = devres_alloc(ata_host_release, sz, GFP_KERNEL);
	if (!host)
		goto err_out;

	devres_add(dev, host);
	dev_set_drvdata(dev, host);

	spin_lock_init(&host->lock);
	mutex_init(&host->eh_mutex);
	host->dev = dev;
	host->n_ports = max_ports;

	for (i = 0; i < max_ports; i++) {
		struct ata_port *ap;

		ap = ata_port_alloc(host);
		if (!ap)
			goto err_out;

		ap->port_no = i;
		host->ports[i] = ap;
	}

	devres_remove_group(dev, NULL);

#ifdef MY_ABC_HERE
	host->host_no = gSynoSataHostCnt;
	gSynoSataHostCnt += 1;
#endif  

	return host;

 err_out:
	devres_release_group(dev, NULL);
	return NULL;
}

struct ata_host *ata_host_alloc_pinfo(struct device *dev,
				      const struct ata_port_info * const * ppi,
				      int n_ports)
{
	const struct ata_port_info *pi;
	struct ata_host *host;
	int i, j;

	host = ata_host_alloc(dev, n_ports);
	if (!host)
		return NULL;

	for (i = 0, j = 0, pi = NULL; i < host->n_ports; i++) {
		struct ata_port *ap = host->ports[i];

		if (ppi[j])
			pi = ppi[j++];

		ap->pio_mask = pi->pio_mask;
		ap->mwdma_mask = pi->mwdma_mask;
		ap->udma_mask = pi->udma_mask;
		ap->flags |= pi->flags;
		ap->link.flags |= pi->link_flags;
		ap->ops = pi->port_ops;

		if (!host->ops && (pi->port_ops != &ata_dummy_port_ops))
			host->ops = pi->port_ops;
	}

	return host;
}

int ata_slave_link_init(struct ata_port *ap)
{
	struct ata_link *link;

	WARN_ON(ap->slave_link);
	WARN_ON(ap->flags & ATA_FLAG_PMP);

	link = kzalloc(sizeof(*link), GFP_KERNEL);
	if (!link)
		return -ENOMEM;

	ata_link_init(ap, link, 1);
	ap->slave_link = link;
	return 0;
}

static void ata_host_stop(struct device *gendev, void *res)
{
	struct ata_host *host = dev_get_drvdata(gendev);
	int i;

	WARN_ON(!(host->flags & ATA_HOST_STARTED));

	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap = host->ports[i];

		if (ap->ops->port_stop)
			ap->ops->port_stop(ap);
	}

	if (host->ops->host_stop)
		host->ops->host_stop(host);
}

static void ata_finalize_port_ops(struct ata_port_operations *ops)
{
	static DEFINE_SPINLOCK(lock);
	const struct ata_port_operations *cur;
	void **begin = (void **)ops;
	void **end = (void **)&ops->inherits;
	void **pp;

	if (!ops || !ops->inherits)
		return;

	spin_lock(&lock);

	for (cur = ops->inherits; cur; cur = cur->inherits) {
		void **inherit = (void **)cur;

		for (pp = begin; pp < end; pp++, inherit++)
			if (!*pp)
				*pp = *inherit;
	}

	for (pp = begin; pp < end; pp++)
		if (IS_ERR(*pp))
			*pp = NULL;

	ops->inherits = NULL;

	spin_unlock(&lock);
}

int ata_host_start(struct ata_host *host)
{
	int have_stop = 0;
	void *start_dr = NULL;
	int i, rc;

	if (host->flags & ATA_HOST_STARTED)
		return 0;

	ata_finalize_port_ops(host->ops);

	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap = host->ports[i];

		ata_finalize_port_ops(ap->ops);

		if (!host->ops && !ata_port_is_dummy(ap))
			host->ops = ap->ops;

		if (ap->ops->port_stop)
			have_stop = 1;
	}

	if (host->ops->host_stop)
		have_stop = 1;

	if (have_stop) {
		start_dr = devres_alloc(ata_host_stop, 0, GFP_KERNEL);
		if (!start_dr)
			return -ENOMEM;
	}

	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap = host->ports[i];

		if (ap->ops->port_start) {
			rc = ap->ops->port_start(ap);
			if (rc) {
				if (rc != -ENODEV)
					dev_err(host->dev,
						"failed to start port %d (errno=%d)\n",
						i, rc);
				goto err_out;
			}
		}
		ata_eh_freeze_port(ap);
	}

	if (start_dr)
		devres_add(host->dev, start_dr);
	host->flags |= ATA_HOST_STARTED;
	return 0;

 err_out:
	while (--i >= 0) {
		struct ata_port *ap = host->ports[i];

		if (ap->ops->port_stop)
			ap->ops->port_stop(ap);
	}
	devres_free(start_dr);
	return rc;
}

void ata_host_init(struct ata_host *host, struct device *dev,
		   struct ata_port_operations *ops)
{
	spin_lock_init(&host->lock);
	mutex_init(&host->eh_mutex);
	host->n_tags = ATA_MAX_QUEUE - 1;
	host->dev = dev;
	host->ops = ops;
}

#ifdef MY_ABC_HERE
static void HddPowerOn(struct ata_port *pAp) {
	int iSynoDiskIdx = -1;
	if (!pAp) {
		goto END;
	}

	iSynoDiskIdx = syno_libata_index_get(pAp->scsi_host, 0, 0, 0) + 1;
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
	 
	if (0 == iSynoDiskIdx) {
		 
		goto END;
	}
#endif  
	 
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
	if (gSynoHddPowerupSeq && gSynoInternalHddNumber < iSynoDiskIdx) {
#else  
	if (0 < g_syno_hdd_powerup_seq && g_syno_hdd_powerup_seq < iSynoDiskIdx) {
#endif  
		goto END;
	}

#ifdef CONFIG_SYNO_SMBUS_HDD_POWERCTL
#ifdef MY_ABC_HERE
	DBG_SpinupGroup("Power on disk: %d\n", iSynoDiskIdx);
#endif  
	SYNO_CTRL_HDD_POWERON(iSynoDiskIdx, 1);
	SleepForLatency();
#else  
	 
#ifdef MY_ABC_HERE
	if (1 == SYNO_CHECK_HDD_DETECT(iSynoDiskIdx)) {
#else
	if (1 == SYNO_CHECK_HDD_PRESENT(iSynoDiskIdx)) {
#endif  

#ifdef MY_ABC_HERE
		DBG_SpinupGroup("Power on disk: %d\n", iSynoDiskIdx);
#endif  
		SYNO_CTRL_HDD_POWERON(iSynoDiskIdx, 1);
		SleepForLatency();
	}
#endif  
END:
	return;
}
#endif  

#ifdef MY_ABC_HERE
 
static void DelayForHWCtl(struct ata_port *pAp)
{
	int iSynoDiskIdx = -1;
#ifdef MY_ABC_HERE
	int iIsDoLatency = 0;
#endif

	if (!pAp) {
		goto END;
	}

	iSynoDiskIdx = syno_libata_index_get(pAp->scsi_host, 0, 0, 0) + 1;
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
	 
	if (0 == iSynoDiskIdx) {
		 
		goto END;
	}
#endif  
	 
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
	if (gSynoHddPowerupSeq && gSynoInternalHddNumber < iSynoDiskIdx) {
#else  
	if (0 < g_syno_hdd_powerup_seq && g_syno_hdd_powerup_seq < iSynoDiskIdx) {
#endif  
		goto END;
	}

#ifdef MY_ABC_HERE
	if(SYNO_SUPPORT_HDD_DYNAMIC_ENABLE_POWER()) {
		 
#ifdef MY_ABC_HERE
		if (0 == SYNO_CHECK_HDD_DETECT(iSynoDiskIdx)) {
#else
		if (0 == SYNO_CHECK_HDD_PRESENT(iSynoDiskIdx)) {
#endif  
			goto END;
		}
		iIsDoLatency = 1;
	}
#endif  

	if (!(pAp->host->flags & ATA_HOST_LLD_SPINUP_DELAY)) {
#ifdef MY_ABC_HERE
		SleepForHW(iSynoDiskIdx, iIsDoLatency);
#else
		SleepForHW(iSynoDiskIdx, 0);
#endif  
	}

END:
	return;
}
#endif  

void __ata_port_probe(struct ata_port *ap)
{
	struct ata_eh_info *ehi = &ap->link.eh_info;
	unsigned long flags;

	spin_lock_irqsave(ap->lock, flags);

	ehi->probe_mask |= ATA_ALL_DEVICES;
	ehi->action |= ATA_EH_RESET;
	ehi->flags |= ATA_EHI_NO_AUTOPSY | ATA_EHI_QUIET;

	ap->pflags &= ~ATA_PFLAG_INITIALIZING;
	ap->pflags |= ATA_PFLAG_LOADING;
	ata_port_schedule_eh(ap);

	spin_unlock_irqrestore(ap->lock, flags);
}

int ata_port_probe(struct ata_port *ap)
{
	int rc = 0;
#ifdef MY_ABC_HERE
	unsigned long flags;
#endif  

#ifdef MY_ABC_HERE
#ifdef MY_ABC_HERE
	 
	if (0 < giSynoSpinupGroupNum) {
		mutex_lock(&mutex_spin);
	}
#endif  
	 
	DelayForHWCtl(ap);
#ifdef MY_ABC_HERE
	if (0 < giSynoSpinupGroupNum) {
		mutex_unlock(&mutex_spin);
	}
#endif  
#endif  

#ifdef MY_ABC_HERE
	 
	if (SYNO_SUPPORT_HDD_DYNAMIC_ENABLE_POWER()) {
		HddPowerOn(ap);
	}
#endif  

	if (ap->ops->error_handler) {
		__ata_port_probe(ap);
		ata_port_wait_eh(ap);

#ifdef MY_ABC_HERE
		spin_lock_irqsave(ap->lock, flags);
		if (ap->pflags & ATA_PFLAG_PMP_DISCONNECT ||
				 ap->pflags & ATA_PFLAG_PMP_CONNECT) {
			 
			ap->pflags &= ~ATA_PFLAG_PMP_DISCONNECT;
			ap->pflags &= ~ATA_PFLAG_PMP_CONNECT;
		}
		spin_unlock_irqrestore(ap->lock, flags);
#endif  
	} else {
		DPRINTK("ata%u: bus probe begin\n", ap->print_id);
		rc = ata_bus_probe(ap);
		DPRINTK("ata%u: bus probe end\n", ap->print_id);
	}
	return rc;
}

static void async_port_probe(void *data, async_cookie_t cookie)
{
	struct ata_port *ap = data;

#ifdef MY_ABC_HERE
	 
#else
	 
	if (!(ap->host->flags & ATA_HOST_PARALLEL_SCAN) && ap->port_no != 0)
		async_synchronize_cookie(cookie);
#endif  

	(void)ata_port_probe(ap);

	async_synchronize_cookie(cookie);

	ata_scsi_scan_host(ap, 1);
}

int ata_host_register(struct ata_host *host, struct scsi_host_template *sht)
{
	int i, rc;

	host->n_tags = clamp(sht->can_queue, 1, ATA_MAX_QUEUE - 1);

	if (!(host->flags & ATA_HOST_STARTED)) {
		dev_err(host->dev, "BUG: trying to register unstarted host\n");
		WARN_ON(1);
		return -EINVAL;
	}

	for (i = host->n_ports; host->ports[i]; i++)
		kfree(host->ports[i]);

	for (i = 0; i < host->n_ports; i++) {
		host->ports[i]->print_id = atomic_inc_return(&ata_print_id);
		host->ports[i]->local_port_no = i + 1;
	}

	for (i = 0; i < host->n_ports; i++) {
		rc = ata_tport_add(host->dev,host->ports[i]);
		if (rc) {
			goto err_tadd;
		}
	}

	rc = ata_scsi_add_hosts(host, sht);
	if (rc)
		goto err_tadd;

	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap = host->ports[i];
		unsigned long xfer_mask;

		if (ap->cbl == ATA_CBL_NONE && (ap->flags & ATA_FLAG_SATA))
			ap->cbl = ATA_CBL_SATA;

		sata_link_init_spd(&ap->link);
		if (ap->slave_link)
			sata_link_init_spd(ap->slave_link);

		xfer_mask = ata_pack_xfermask(ap->pio_mask, ap->mwdma_mask,
					      ap->udma_mask);

		if (!ata_port_is_dummy(ap)) {
			ata_port_info(ap, "%cATA max %s %s\n",
				      (ap->flags & ATA_FLAG_SATA) ? 'S' : 'P',
				      ata_mode_string(xfer_mask),
				      ap->link.eh_info.desc);
			ata_ehi_clear_desc(&ap->link.eh_info);
		} else
			ata_port_info(ap, "DUMMY\n");
	}
#ifdef MY_ABC_HERE
	 
	if (host->flags & ATA_HOST_LLD_SPINUP_DELAY) {
		for (i = 0; i < host->n_ports; i++) {
			struct ata_port *ap = host->ports[i];
			SleepForHD(ap->print_id);
		}
	}
#endif  

	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap = host->ports[i];
#ifdef MY_ABC_HERE
		 
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
		if (gSynoHddPowerupSeq) {
#else  
		if (0 < giSynoSpinupGroupNum) {
#endif  
			async_schedule(async_port_probe, ap);
			continue;
		}
#endif  
#ifdef MY_ABC_HERE
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
		if (0 == gSynoHddPowerupSeq) {
#else  
		if (0 == g_syno_hdd_powerup_seq) {
#endif  
			async_schedule(async_port_probe, ap);
		} else {
			ata_port_probe(ap);
			ata_scsi_scan_host(ap, 1);
		}
#else  
		async_schedule(async_port_probe, ap);
#endif  
	}
#ifdef MY_ABC_HERE
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
	if (gSynoHddPowerupSeq) {
#else  
	if (0 != g_syno_hdd_powerup_seq) {
#endif  
		for (i = 0; i < host->n_ports; i++) {
			struct ata_port *ap = host->ports[i];
			ata_port_wait_eh(ap);
			ata_scsi_scan_host(ap, 1);
		}
	}
	if (host->flags & ATA_HOST_LLD_SPINUP_DELAY) {
		host->flags &= ~ATA_HOST_LLD_SPINUP_DELAY;
	}
#endif  

	return 0;

 err_tadd:
	while (--i >= 0) {
		ata_tport_delete(host->ports[i]);
	}
	return rc;

}

int ata_host_activate(struct ata_host *host, int irq,
		      irq_handler_t irq_handler, unsigned long irq_flags,
		      struct scsi_host_template *sht)
{
	int i, rc;

	rc = ata_host_start(host);
	if (rc)
		return rc;

	if (!irq) {
		WARN_ON(irq_handler);
		return ata_host_register(host, sht);
	}

	rc = devm_request_irq(host->dev, irq, irq_handler, irq_flags,
			      dev_name(host->dev), host);
	if (rc)
		return rc;

	for (i = 0; i < host->n_ports; i++)
		ata_port_desc(host->ports[i], "irq %d", irq);

	rc = ata_host_register(host, sht);
	 
	if (rc)
		devm_free_irq(host->dev, irq, host);

	return rc;
}

static void ata_port_detach(struct ata_port *ap)
{
	unsigned long flags;
	struct ata_link *link;
	struct ata_device *dev;

	if (!ap->ops->error_handler)
		goto skip_eh;

	spin_lock_irqsave(ap->lock, flags);
	ap->pflags |= ATA_PFLAG_UNLOADING;
	ata_port_schedule_eh(ap);
	spin_unlock_irqrestore(ap->lock, flags);

	ata_port_wait_eh(ap);

	WARN_ON(!(ap->pflags & ATA_PFLAG_UNLOADED));

	cancel_delayed_work_sync(&ap->hotplug_task);

 skip_eh:
	 
	ata_for_each_link(link, ap, HOST_FIRST) {
		ata_for_each_dev(dev, link, ALL) {
			if (zpodd_dev_enabled(dev))
				zpodd_exit(dev);
		}
	}
	if (ap->pmp_link) {
		int i;
		for (i = 0; i < SATA_PMP_MAX_PORTS; i++)
			ata_tlink_delete(&ap->pmp_link[i]);
	}
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
	klist_del(&ap->ata_port_list);
#endif  
	 
	scsi_remove_host(ap->scsi_host);
	ata_tport_delete(ap);
}

void ata_host_detach(struct ata_host *host)
{
	int i;

	for (i = 0; i < host->n_ports; i++)
		ata_port_detach(host->ports[i]);

	ata_acpi_dissociate(host);
}

#ifdef CONFIG_PCI

void ata_pci_remove_one(struct pci_dev *pdev)
{
	struct ata_host *host = pci_get_drvdata(pdev);

	ata_host_detach(host);
}

int pci_test_config_bits(struct pci_dev *pdev, const struct pci_bits *bits)
{
	unsigned long tmp = 0;

	switch (bits->width) {
	case 1: {
		u8 tmp8 = 0;
		pci_read_config_byte(pdev, bits->reg, &tmp8);
		tmp = tmp8;
		break;
	}
	case 2: {
		u16 tmp16 = 0;
		pci_read_config_word(pdev, bits->reg, &tmp16);
		tmp = tmp16;
		break;
	}
	case 4: {
		u32 tmp32 = 0;
		pci_read_config_dword(pdev, bits->reg, &tmp32);
		tmp = tmp32;
		break;
	}

	default:
		return -EINVAL;
	}

	tmp &= bits->mask;

	return (tmp == bits->val) ? 1 : 0;
}

#ifdef CONFIG_PM
void ata_pci_device_do_suspend(struct pci_dev *pdev, pm_message_t mesg)
{
	pci_save_state(pdev);
	pci_disable_device(pdev);

	if (mesg.event & PM_EVENT_SLEEP)
		pci_set_power_state(pdev, PCI_D3hot);
}

int ata_pci_device_do_resume(struct pci_dev *pdev)
{
	int rc;

	pci_set_power_state(pdev, PCI_D0);
	pci_restore_state(pdev);

	rc = pcim_enable_device(pdev);
	if (rc) {
		dev_err(&pdev->dev,
			"failed to enable device after resume (%d)\n", rc);
		return rc;
	}

	pci_set_master(pdev);
	return 0;
}

int ata_pci_device_suspend(struct pci_dev *pdev, pm_message_t mesg)
{
	struct ata_host *host = pci_get_drvdata(pdev);
	int rc = 0;

	rc = ata_host_suspend(host, mesg);
	if (rc)
		return rc;

	ata_pci_device_do_suspend(pdev, mesg);

	return 0;
}

int ata_pci_device_resume(struct pci_dev *pdev)
{
	struct ata_host *host = pci_get_drvdata(pdev);
	int rc;

	rc = ata_pci_device_do_resume(pdev);
	if (rc == 0)
		ata_host_resume(host);
	return rc;
}
#endif  

#endif  

int ata_platform_remove_one(struct platform_device *pdev)
{
	struct ata_host *host = platform_get_drvdata(pdev);

	ata_host_detach(host);

	return 0;
}

static int __init ata_parse_force_one(char **cur,
				      struct ata_force_ent *force_ent,
				      const char **reason)
{
	static const struct ata_force_param force_tbl[] __initconst = {
		{ "40c",	.cbl		= ATA_CBL_PATA40 },
		{ "80c",	.cbl		= ATA_CBL_PATA80 },
		{ "short40c",	.cbl		= ATA_CBL_PATA40_SHORT },
		{ "unk",	.cbl		= ATA_CBL_PATA_UNK },
		{ "ign",	.cbl		= ATA_CBL_PATA_IGN },
		{ "sata",	.cbl		= ATA_CBL_SATA },
		{ "1.5Gbps",	.spd_limit	= 1 },
		{ "3.0Gbps",	.spd_limit	= 2 },
		{ "noncq",	.horkage_on	= ATA_HORKAGE_NONCQ },
		{ "ncq",	.horkage_off	= ATA_HORKAGE_NONCQ },
		{ "noncqtrim",	.horkage_on	= ATA_HORKAGE_NO_NCQ_TRIM },
		{ "ncqtrim",	.horkage_off	= ATA_HORKAGE_NO_NCQ_TRIM },
		{ "dump_id",	.horkage_on	= ATA_HORKAGE_DUMP_ID },
		{ "pio0",	.xfer_mask	= 1 << (ATA_SHIFT_PIO + 0) },
		{ "pio1",	.xfer_mask	= 1 << (ATA_SHIFT_PIO + 1) },
		{ "pio2",	.xfer_mask	= 1 << (ATA_SHIFT_PIO + 2) },
		{ "pio3",	.xfer_mask	= 1 << (ATA_SHIFT_PIO + 3) },
		{ "pio4",	.xfer_mask	= 1 << (ATA_SHIFT_PIO + 4) },
		{ "pio5",	.xfer_mask	= 1 << (ATA_SHIFT_PIO + 5) },
		{ "pio6",	.xfer_mask	= 1 << (ATA_SHIFT_PIO + 6) },
		{ "mwdma0",	.xfer_mask	= 1 << (ATA_SHIFT_MWDMA + 0) },
		{ "mwdma1",	.xfer_mask	= 1 << (ATA_SHIFT_MWDMA + 1) },
		{ "mwdma2",	.xfer_mask	= 1 << (ATA_SHIFT_MWDMA + 2) },
		{ "mwdma3",	.xfer_mask	= 1 << (ATA_SHIFT_MWDMA + 3) },
		{ "mwdma4",	.xfer_mask	= 1 << (ATA_SHIFT_MWDMA + 4) },
		{ "udma0",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 0) },
		{ "udma16",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 0) },
		{ "udma/16",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 0) },
		{ "udma1",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 1) },
		{ "udma25",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 1) },
		{ "udma/25",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 1) },
		{ "udma2",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 2) },
		{ "udma33",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 2) },
		{ "udma/33",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 2) },
		{ "udma3",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 3) },
		{ "udma44",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 3) },
		{ "udma/44",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 3) },
		{ "udma4",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 4) },
		{ "udma66",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 4) },
		{ "udma/66",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 4) },
		{ "udma5",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 5) },
		{ "udma100",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 5) },
		{ "udma/100",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 5) },
		{ "udma6",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 6) },
		{ "udma133",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 6) },
		{ "udma/133",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 6) },
		{ "udma7",	.xfer_mask	= 1 << (ATA_SHIFT_UDMA + 7) },
		{ "nohrst",	.lflags		= ATA_LFLAG_NO_HRST },
		{ "nosrst",	.lflags		= ATA_LFLAG_NO_SRST },
		{ "norst",	.lflags		= ATA_LFLAG_NO_HRST | ATA_LFLAG_NO_SRST },
		{ "rstonce",	.lflags		= ATA_LFLAG_RST_ONCE },
		{ "atapi_dmadir", .horkage_on	= ATA_HORKAGE_ATAPI_DMADIR },
		{ "disable",	.horkage_on	= ATA_HORKAGE_DISABLE },
	};
	char *start = *cur, *p = *cur;
	char *id, *val, *endp;
	const struct ata_force_param *match_fp = NULL;
	int nr_matches = 0, i;

	while (*p != '\0' && *p != ',')
		p++;

	if (*p == '\0')
		*cur = p;
	else
		*cur = p + 1;

	*p = '\0';

	p = strchr(start, ':');
	if (!p) {
		val = strstrip(start);
		goto parse_val;
	}
	*p = '\0';

	id = strstrip(start);
	val = strstrip(p + 1);

	p = strchr(id, '.');
	if (p) {
		*p++ = '\0';
		force_ent->device = simple_strtoul(p, &endp, 10);
		if (p == endp || *endp != '\0') {
			*reason = "invalid device";
			return -EINVAL;
		}
	}

	force_ent->port = simple_strtoul(id, &endp, 10);
	if (p == endp || *endp != '\0') {
		*reason = "invalid port/link";
		return -EINVAL;
	}

 parse_val:
	 
	for (i = 0; i < ARRAY_SIZE(force_tbl); i++) {
		const struct ata_force_param *fp = &force_tbl[i];

		if (strncasecmp(val, fp->name, strlen(val)))
			continue;

		nr_matches++;
		match_fp = fp;

		if (strcasecmp(val, fp->name) == 0) {
			nr_matches = 1;
			break;
		}
	}

	if (!nr_matches) {
		*reason = "unknown value";
		return -EINVAL;
	}
	if (nr_matches > 1) {
		*reason = "ambigious value";
		return -EINVAL;
	}

	force_ent->param = *match_fp;

	return 0;
}

static void __init ata_parse_force_param(void)
{
	int idx = 0, size = 1;
	int last_port = -1, last_device = -1;
	char *p, *cur, *next;

	for (p = ata_force_param_buf; *p; p++)
		if (*p == ',')
			size++;

	ata_force_tbl = kzalloc(sizeof(ata_force_tbl[0]) * size, GFP_KERNEL);
	if (!ata_force_tbl) {
		printk(KERN_WARNING "ata: failed to extend force table, "
		       "libata.force ignored\n");
		return;
	}

	for (cur = ata_force_param_buf; *cur != '\0'; cur = next) {
		const char *reason = "";
		struct ata_force_ent te = { .port = -1, .device = -1 };

		next = cur;
		if (ata_parse_force_one(&next, &te, &reason)) {
			printk(KERN_WARNING "ata: failed to parse force "
			       "parameter \"%s\" (%s)\n",
			       cur, reason);
			continue;
		}

		if (te.port == -1) {
			te.port = last_port;
			te.device = last_device;
		}

		ata_force_tbl[idx++] = te;

		last_port = te.port;
		last_device = te.device;
	}

	ata_force_tbl_size = idx;
}

static int __init ata_init(void)
{
	int rc;

	ata_parse_force_param();

	rc = ata_sff_init();
	if (rc) {
		kfree(ata_force_tbl);
		return rc;
	}

	libata_transport_init();
	ata_scsi_transport_template = ata_attach_transport();
	if (!ata_scsi_transport_template) {
		ata_sff_exit();
		rc = -ENOMEM;
		goto err_out;
	}
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
	klist_init(&syno_ata_port_head, NULL, NULL);
#endif  

	printk(KERN_DEBUG "libata version " DRV_VERSION " loaded.\n");
	return 0;

err_out:
	return rc;
}

static void __exit ata_exit(void)
{
	ata_release_transport(ata_scsi_transport_template);
	libata_transport_exit();
	ata_sff_exit();
	kfree(ata_force_tbl);
}

subsys_initcall(ata_init);
module_exit(ata_exit);

static DEFINE_RATELIMIT_STATE(ratelimit, HZ / 5, 1);

int ata_ratelimit(void)
{
	return __ratelimit(&ratelimit);
}

void ata_msleep(struct ata_port *ap, unsigned int msecs)
{
	bool owns_eh = ap && ap->host->eh_owner == current;

	if (owns_eh)
		ata_eh_release(ap);

	msleep(msecs);

	if (owns_eh)
		ata_eh_acquire(ap);
}

u32 ata_wait_register(struct ata_port *ap, void __iomem *reg, u32 mask, u32 val,
		      unsigned long interval, unsigned long timeout)
{
	unsigned long deadline;
	u32 tmp;

	tmp = ioread32(reg);

	deadline = ata_deadline(jiffies, timeout);

	while ((tmp & mask) == val && time_before(jiffies, deadline)) {
		ata_msleep(ap, interval);
		tmp = ioread32(reg);
	}

	return tmp;
}

bool sata_lpm_ignore_phy_events(struct ata_link *link)
{
	unsigned long lpm_timeout = link->last_lpm_change +
				    msecs_to_jiffies(ATA_TMOUT_SPURIOUS_PHY);

	if (link->lpm_policy > ATA_LPM_MAX_POWER)
		return true;

	if ((link->flags & ATA_LFLAG_CHANGED) &&
	    time_before(jiffies, lpm_timeout))
		return true;

	return false;
}
EXPORT_SYMBOL_GPL(sata_lpm_ignore_phy_events);

static unsigned int ata_dummy_qc_issue(struct ata_queued_cmd *qc)
{
	return AC_ERR_SYSTEM;
}

static void ata_dummy_error_handler(struct ata_port *ap)
{
	 
}

struct ata_port_operations ata_dummy_port_ops = {
	.qc_prep		= ata_noop_qc_prep,
	.qc_issue		= ata_dummy_qc_issue,
	.error_handler		= ata_dummy_error_handler,
	.sched_eh		= ata_std_sched_eh,
	.end_eh			= ata_std_end_eh,
};

const struct ata_port_info ata_dummy_port_info = {
	.port_ops		= &ata_dummy_port_ops,
};

#ifdef MY_ABC_HERE
void syno_ata_info_print(struct ata_port *ap)
{
	u32 sstatus;

	if(NULL == ap->ops || NULL == ap->ops->scr_read) {
		return;
	}

	ap->ops->scr_read(&ap->link, SCR_STATUS, &sstatus);
	 
	if(0x0 == (0xf & sstatus)) {
		printk("ata%u: device unplugged sstatus 0x%x\n", ap->print_id, sstatus);
	}else if (0x1 == (0xf & sstatus) || 0x3 == (0xf & sstatus)) {
		printk("ata%u: device plugged sstatus 0x%x\n", ap->print_id, sstatus);
	} else {
		printk("ata%u: PHY in offline mode 0x%x\n", ap->print_id, sstatus);
	}
}

EXPORT_SYMBOL(syno_ata_info_print);
#endif  

void ata_port_printk(const struct ata_port *ap, const char *level,
		     const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, fmt);

	vaf.fmt = fmt;
	vaf.va = &args;
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
	if (0 <= ap->syno_disk_index) {
		printk("%sata%u (slot %d): %pV", level, ap->print_id, ap->syno_disk_index + 1 , &vaf);
	} else {
		printk("%sata%u : %pV", level, ap->print_id, &vaf);
	}
#else  
	printk("%sata%u: %pV", level, ap->print_id, &vaf);
#endif  

	va_end(args);
}
EXPORT_SYMBOL(ata_port_printk);

void ata_link_printk(const struct ata_link *link, const char *level,
		     const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, fmt);

	vaf.fmt = fmt;
	vaf.va = &args;

#ifdef CONFIG_SYNO_PORT_MAPPING_V2
	if (sata_pmp_attached(link->ap) || link->ap->slave_link) {
		if (0 <= link->ap->syno_disk_index) {
			printk("%sata%u.%02u (slot %d): %pV",
			       level, link->ap->print_id, link->pmp, link->ap->syno_disk_index + 1, &vaf);
		} else {
			printk("%sata%u.%02u : %pV",
			       level, link->ap->print_id, link->pmp, &vaf);
		}
	} else {
		if (0 <= link->ap->syno_disk_index) {
			printk("%sata%u (slot %d): %pV",
			       level, link->ap->print_id, link->ap->syno_disk_index + 1, &vaf);
		} else {
			printk("%sata%u : %pV",
			       level, link->ap->print_id, &vaf);
		}
	}
#else  
	if (sata_pmp_attached(link->ap) || link->ap->slave_link)
		printk("%sata%u.%02u: %pV",
		       level, link->ap->print_id, link->pmp, &vaf);
	else
		printk("%sata%u: %pV",
		       level, link->ap->print_id, &vaf);
#endif  

	va_end(args);
}
EXPORT_SYMBOL(ata_link_printk);

void ata_dev_printk(const struct ata_device *dev, const char *level,
		    const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;

	va_start(args, fmt);

	vaf.fmt = fmt;
	vaf.va = &args;
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
	if (0 <= dev->link->ap->syno_disk_index) {
		printk("%sata%u.%02u (slot %d): %pV",
		       level, dev->link->ap->print_id, dev->link->pmp + dev->devno, dev->link->ap->syno_disk_index + 1,
		       &vaf);
	} else {
		printk("%sata%u.%02u : %pV",
		       level, dev->link->ap->print_id, dev->link->pmp + dev->devno,
		       &vaf);
	}
#else  
	printk("%sata%u.%02u: %pV",
	       level, dev->link->ap->print_id, dev->link->pmp + dev->devno,
	       &vaf);
#endif  

	va_end(args);
}
EXPORT_SYMBOL(ata_dev_printk);

void ata_print_version(const struct device *dev, const char *version)
{
	dev_printk(KERN_DEBUG, dev, "version %s\n", version);
}
EXPORT_SYMBOL(ata_print_version);

EXPORT_SYMBOL_GPL(sata_deb_timing_normal);
EXPORT_SYMBOL_GPL(sata_deb_timing_hotplug);
EXPORT_SYMBOL_GPL(sata_deb_timing_long);
EXPORT_SYMBOL_GPL(ata_base_port_ops);
EXPORT_SYMBOL_GPL(sata_port_ops);
EXPORT_SYMBOL_GPL(ata_dummy_port_ops);
EXPORT_SYMBOL_GPL(ata_dummy_port_info);
EXPORT_SYMBOL_GPL(ata_link_next);
EXPORT_SYMBOL_GPL(ata_dev_next);
EXPORT_SYMBOL_GPL(ata_std_bios_param);
EXPORT_SYMBOL_GPL(ata_scsi_unlock_native_capacity);
EXPORT_SYMBOL_GPL(ata_host_init);
EXPORT_SYMBOL_GPL(ata_host_alloc);
EXPORT_SYMBOL_GPL(ata_host_alloc_pinfo);
EXPORT_SYMBOL_GPL(ata_slave_link_init);
EXPORT_SYMBOL_GPL(ata_host_start);
EXPORT_SYMBOL_GPL(ata_host_register);
EXPORT_SYMBOL_GPL(ata_host_activate);
EXPORT_SYMBOL_GPL(ata_host_detach);
EXPORT_SYMBOL_GPL(ata_sg_init);
EXPORT_SYMBOL_GPL(ata_qc_complete);
EXPORT_SYMBOL_GPL(ata_qc_complete_multiple);
EXPORT_SYMBOL_GPL(atapi_cmd_type);
EXPORT_SYMBOL_GPL(ata_tf_to_fis);
EXPORT_SYMBOL_GPL(ata_tf_from_fis);
EXPORT_SYMBOL_GPL(ata_pack_xfermask);
EXPORT_SYMBOL_GPL(ata_unpack_xfermask);
EXPORT_SYMBOL_GPL(ata_xfer_mask2mode);
EXPORT_SYMBOL_GPL(ata_xfer_mode2mask);
EXPORT_SYMBOL_GPL(ata_xfer_mode2shift);
EXPORT_SYMBOL_GPL(ata_mode_string);
EXPORT_SYMBOL_GPL(ata_id_xfermask);
EXPORT_SYMBOL_GPL(ata_do_set_mode);
EXPORT_SYMBOL_GPL(ata_std_qc_defer);
EXPORT_SYMBOL_GPL(ata_noop_qc_prep);
EXPORT_SYMBOL_GPL(ata_dev_disable);
EXPORT_SYMBOL_GPL(sata_set_spd);
EXPORT_SYMBOL_GPL(ata_wait_after_reset);
EXPORT_SYMBOL_GPL(sata_link_debounce);
EXPORT_SYMBOL_GPL(sata_link_resume);
EXPORT_SYMBOL_GPL(sata_link_scr_lpm);
EXPORT_SYMBOL_GPL(ata_std_prereset);
EXPORT_SYMBOL_GPL(sata_link_hardreset);
EXPORT_SYMBOL_GPL(sata_std_hardreset);
EXPORT_SYMBOL_GPL(ata_std_postreset);
EXPORT_SYMBOL_GPL(ata_dev_classify);
EXPORT_SYMBOL_GPL(ata_dev_pair);
EXPORT_SYMBOL_GPL(ata_ratelimit);
EXPORT_SYMBOL_GPL(ata_msleep);
EXPORT_SYMBOL_GPL(ata_wait_register);
EXPORT_SYMBOL_GPL(ata_scsi_queuecmd);
EXPORT_SYMBOL_GPL(ata_scsi_slave_config);
EXPORT_SYMBOL_GPL(ata_scsi_slave_destroy);
EXPORT_SYMBOL_GPL(ata_scsi_change_queue_depth);
EXPORT_SYMBOL_GPL(__ata_change_queue_depth);
EXPORT_SYMBOL_GPL(sata_scr_valid);
EXPORT_SYMBOL_GPL(sata_scr_read);
EXPORT_SYMBOL_GPL(sata_scr_write);
EXPORT_SYMBOL_GPL(sata_scr_write_flush);
EXPORT_SYMBOL_GPL(ata_link_online);
EXPORT_SYMBOL_GPL(ata_link_offline);
#ifdef CONFIG_PM
EXPORT_SYMBOL_GPL(ata_host_suspend);
EXPORT_SYMBOL_GPL(ata_host_resume);
#endif  
EXPORT_SYMBOL_GPL(ata_id_string);
EXPORT_SYMBOL_GPL(ata_id_c_string);
EXPORT_SYMBOL_GPL(ata_do_dev_read_id);
EXPORT_SYMBOL_GPL(ata_scsi_simulate);

EXPORT_SYMBOL_GPL(ata_pio_need_iordy);
EXPORT_SYMBOL_GPL(ata_timing_find_mode);
EXPORT_SYMBOL_GPL(ata_timing_compute);
EXPORT_SYMBOL_GPL(ata_timing_merge);
EXPORT_SYMBOL_GPL(ata_timing_cycle2mode);

#ifdef CONFIG_PCI
EXPORT_SYMBOL_GPL(pci_test_config_bits);
EXPORT_SYMBOL_GPL(ata_pci_remove_one);
#ifdef CONFIG_PM
EXPORT_SYMBOL_GPL(ata_pci_device_do_suspend);
EXPORT_SYMBOL_GPL(ata_pci_device_do_resume);
EXPORT_SYMBOL_GPL(ata_pci_device_suspend);
EXPORT_SYMBOL_GPL(ata_pci_device_resume);
#endif  
#endif  

EXPORT_SYMBOL_GPL(ata_platform_remove_one);

EXPORT_SYMBOL_GPL(__ata_ehi_push_desc);
EXPORT_SYMBOL_GPL(ata_ehi_push_desc);
EXPORT_SYMBOL_GPL(ata_ehi_clear_desc);
EXPORT_SYMBOL_GPL(ata_port_desc);
#ifdef CONFIG_PCI
EXPORT_SYMBOL_GPL(ata_port_pbar_desc);
#endif  
EXPORT_SYMBOL_GPL(ata_port_schedule_eh);
EXPORT_SYMBOL_GPL(ata_link_abort);
EXPORT_SYMBOL_GPL(ata_port_abort);
EXPORT_SYMBOL_GPL(ata_port_freeze);
EXPORT_SYMBOL_GPL(sata_async_notification);
EXPORT_SYMBOL_GPL(ata_eh_freeze_port);
EXPORT_SYMBOL_GPL(ata_eh_thaw_port);
EXPORT_SYMBOL_GPL(ata_eh_qc_complete);
EXPORT_SYMBOL_GPL(ata_eh_qc_retry);
EXPORT_SYMBOL_GPL(ata_eh_analyze_ncq_error);
EXPORT_SYMBOL_GPL(ata_do_eh);
EXPORT_SYMBOL_GPL(ata_std_error_handler);

EXPORT_SYMBOL_GPL(ata_cable_40wire);
EXPORT_SYMBOL_GPL(ata_cable_80wire);
EXPORT_SYMBOL_GPL(ata_cable_unknown);
EXPORT_SYMBOL_GPL(ata_cable_ignore);
EXPORT_SYMBOL_GPL(ata_cable_sata);

int (*funcSYNOSendDiskResetPwrEvent)(unsigned int, unsigned int) = NULL;
EXPORT_SYMBOL(funcSYNOSendDiskResetPwrEvent);
int (*funcSYNOSendDiskPortDisEvent)(unsigned int, unsigned int) = NULL;
EXPORT_SYMBOL(funcSYNOSendDiskPortDisEvent);
int (*funcSYNOSataErrorReport)(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int) = NULL;
EXPORT_SYMBOL(funcSYNOSataErrorReport);
int (*funcSYNODiskRetryReport)(unsigned int, unsigned int) = NULL;
EXPORT_SYMBOL(funcSYNODiskRetryReport);
int (*funcSYNODeepSleepEvent)(unsigned int, unsigned int) = NULL;
EXPORT_SYMBOL(funcSYNODeepSleepEvent);
int (*funcSYNODiskPowerShortBreakReport)(unsigned int, unsigned int) = NULL;
EXPORT_SYMBOL(funcSYNODiskPowerShortBreakReport);
