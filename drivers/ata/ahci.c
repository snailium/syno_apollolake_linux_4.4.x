#ifndef MY_ABC_HERE
#define MY_ABC_HERE
#endif
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/blkdev.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/device.h>
#include <linux/dmi.h>
#include <linux/gfp.h>
#include <linux/msi.h>
#include <scsi/scsi_host.h>
#include <scsi/scsi_cmnd.h>
#ifdef MY_DEF_HERE
#include <scsi/scsi.h>
#include <scsi/scsi_eh.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_tcq.h>
#include <scsi/scsi_transport.h>
#endif  
#include <linux/libata.h>
#include "ahci.h"

#define DRV_NAME	"ahci"
#define DRV_VERSION	"3.0"

#ifdef MY_ABC_HERE
extern char g_ahci_switch;
#endif  

enum {
	AHCI_PCI_BAR_STA2X11	= 0,
	AHCI_PCI_BAR_CAVIUM	= 0,
	AHCI_PCI_BAR_ENMOTUS	= 2,
	AHCI_PCI_BAR_STANDARD	= 5,
};

enum board_ids {
	 
	board_ahci,
	board_ahci_ign_iferr,
	board_ahci_nomsi,
	board_ahci_noncq,
	board_ahci_nosntf,
	board_ahci_yes_fbs,
#ifdef MY_DEF_HERE
	board_ahci_yes_fbs_no_ncq,
#endif  

	board_ahci_avn,
#ifdef CONFIG_SYNO_JMICRON_585_FIX
	board_ahci_jmb585,
#endif  
	board_ahci_mcp65,
	board_ahci_mcp77,
	board_ahci_mcp89,
	board_ahci_mv,
	board_ahci_sb600,
	board_ahci_sb700,	 
	board_ahci_vt8251,

	board_ahci_mcp_linux	= board_ahci_mcp65,
	board_ahci_mcp67	= board_ahci_mcp65,
	board_ahci_mcp73	= board_ahci_mcp65,
	board_ahci_mcp79	= board_ahci_mcp77,
};

#ifdef CONFIG_SYNO_JMICRON_585_FIX
static int syno_ahci_hardreset_jmb(struct ata_link *link, unsigned int *class, unsigned long deadline);
static int syno_ahci_softreset_jmb(struct ata_link *link, unsigned int *class, unsigned long deadline);
#endif  

static int ahci_init_one(struct pci_dev *pdev, const struct pci_device_id *ent);
static int ahci_vt8251_hardreset(struct ata_link *link, unsigned int *class,
				 unsigned long deadline);
static int ahci_avn_hardreset(struct ata_link *link, unsigned int *class,
			      unsigned long deadline);
static void ahci_mcp89_apple_enable(struct pci_dev *pdev);
static bool is_mcp89_apple(struct pci_dev *pdev);
static int ahci_p5wdh_hardreset(struct ata_link *link, unsigned int *class,
				unsigned long deadline);
#ifdef CONFIG_PM
static int ahci_pci_device_suspend(struct pci_dev *pdev, pm_message_t mesg);
static int ahci_pci_device_resume(struct pci_dev *pdev);
#endif

static struct scsi_host_template ahci_sht = {
	AHCI_SHT("ahci"),
};

#ifdef MY_ABC_HERE
static struct ata_port_operations ahci_pmp_ops = {
	.inherits		= &ahci_ops,
	.qc_defer		= sata_syno_ahci_defer_cmd,
};
#endif  

static struct ata_port_operations ahci_vt8251_ops = {
	.inherits		= &ahci_ops,
	.hardreset		= ahci_vt8251_hardreset,
};

static struct ata_port_operations ahci_p5wdh_ops = {
	.inherits		= &ahci_ops,
	.hardreset		= ahci_p5wdh_hardreset,
};

static struct ata_port_operations ahci_avn_ops = {
	.inherits		= &ahci_ops,
	.hardreset		= ahci_avn_hardreset,
};

#ifdef CONFIG_SYNO_JMICRON_585_FIX
static struct ata_port_operations ahci_jmb585_ops = {
	.inherits		= &ahci_ops,
	.hardreset		= syno_ahci_hardreset_jmb,
	.softreset		= syno_ahci_softreset_jmb,
	.pmp_softreset		= syno_ahci_softreset_jmb,
};
#endif  

static const struct ata_port_info ahci_port_info[] = {
	 
	[board_ahci] = {
		.flags		= AHCI_FLAG_COMMON,
		.pio_mask	= ATA_PIO4,
		.udma_mask	= ATA_UDMA6,
		.port_ops	= &ahci_ops,
	},
	[board_ahci_ign_iferr] = {
		AHCI_HFLAGS	(AHCI_HFLAG_IGN_IRQ_IF_ERR),
		.flags		= AHCI_FLAG_COMMON,
		.pio_mask	= ATA_PIO4,
		.udma_mask	= ATA_UDMA6,
		.port_ops	= &ahci_ops,
	},
	[board_ahci_nomsi] = {
		AHCI_HFLAGS	(AHCI_HFLAG_NO_MSI),
		.flags		= AHCI_FLAG_COMMON,
		.pio_mask	= ATA_PIO4,
		.udma_mask	= ATA_UDMA6,
		.port_ops	= &ahci_ops,
	},
	[board_ahci_noncq] = {
		AHCI_HFLAGS	(AHCI_HFLAG_NO_NCQ),
		.flags		= AHCI_FLAG_COMMON,
		.pio_mask	= ATA_PIO4,
		.udma_mask	= ATA_UDMA6,
		.port_ops	= &ahci_ops,
	},
	[board_ahci_nosntf] = {
		AHCI_HFLAGS	(AHCI_HFLAG_NO_SNTF),
		.flags		= AHCI_FLAG_COMMON,
		.pio_mask	= ATA_PIO4,
		.udma_mask	= ATA_UDMA6,
#ifdef MY_ABC_HERE
		.port_ops	= &ahci_pmp_ops,
#else
		.port_ops	= &ahci_ops,
#endif  
	},
	[board_ahci_yes_fbs] = {
		AHCI_HFLAGS	(AHCI_HFLAG_YES_FBS),
		.flags		= AHCI_FLAG_COMMON,
		.pio_mask	= ATA_PIO4,
		.udma_mask	= ATA_UDMA6,
		.port_ops	= &ahci_ops,
	},
#ifdef MY_DEF_HERE
	[board_ahci_yes_fbs_no_ncq] = {
		AHCI_HFLAGS	(AHCI_HFLAG_YES_FBS | AHCI_HFLAG_NO_NCQ),
		.flags		= AHCI_FLAG_COMMON,
		.pio_mask	= ATA_PIO4,
		.udma_mask	= ATA_UDMA6,
		.port_ops	= &ahci_ops,
	},
#endif  
	 
	[board_ahci_avn] = {
		.flags		= AHCI_FLAG_COMMON,
		.pio_mask	= ATA_PIO4,
		.udma_mask	= ATA_UDMA6,
		.port_ops	= &ahci_avn_ops,
	},
	[board_ahci_mcp65] = {
		AHCI_HFLAGS	(AHCI_HFLAG_NO_FPDMA_AA | AHCI_HFLAG_NO_PMP |
				 AHCI_HFLAG_YES_NCQ),
		.flags		= AHCI_FLAG_COMMON | ATA_FLAG_NO_DIPM,
		.pio_mask	= ATA_PIO4,
		.udma_mask	= ATA_UDMA6,
		.port_ops	= &ahci_ops,
	},
	[board_ahci_mcp77] = {
		AHCI_HFLAGS	(AHCI_HFLAG_NO_FPDMA_AA | AHCI_HFLAG_NO_PMP),
		.flags		= AHCI_FLAG_COMMON,
		.pio_mask	= ATA_PIO4,
		.udma_mask	= ATA_UDMA6,
		.port_ops	= &ahci_ops,
	},
	[board_ahci_mcp89] = {
		AHCI_HFLAGS	(AHCI_HFLAG_NO_FPDMA_AA),
		.flags		= AHCI_FLAG_COMMON,
		.pio_mask	= ATA_PIO4,
		.udma_mask	= ATA_UDMA6,
		.port_ops	= &ahci_ops,
	},
	[board_ahci_mv] = {
		AHCI_HFLAGS	(AHCI_HFLAG_NO_NCQ | AHCI_HFLAG_NO_MSI |
				 AHCI_HFLAG_MV_PATA | AHCI_HFLAG_NO_PMP),
		.flags		= ATA_FLAG_SATA | ATA_FLAG_PIO_DMA,
		.pio_mask	= ATA_PIO4,
		.udma_mask	= ATA_UDMA6,
		.port_ops	= &ahci_ops,
	},
	[board_ahci_sb600] = {
		AHCI_HFLAGS	(AHCI_HFLAG_IGN_SERR_INTERNAL |
				 AHCI_HFLAG_NO_MSI | AHCI_HFLAG_SECT255 |
				 AHCI_HFLAG_32BIT_ONLY),
		.flags		= AHCI_FLAG_COMMON,
		.pio_mask	= ATA_PIO4,
		.udma_mask	= ATA_UDMA6,
		.port_ops	= &ahci_pmp_retry_srst_ops,
	},
	[board_ahci_sb700] = {	 
		AHCI_HFLAGS	(AHCI_HFLAG_IGN_SERR_INTERNAL),
		.flags		= AHCI_FLAG_COMMON,
		.pio_mask	= ATA_PIO4,
		.udma_mask	= ATA_UDMA6,
		.port_ops	= &ahci_pmp_retry_srst_ops,
	},
	[board_ahci_vt8251] = {
		AHCI_HFLAGS	(AHCI_HFLAG_NO_NCQ | AHCI_HFLAG_NO_PMP),
		.flags		= AHCI_FLAG_COMMON,
		.pio_mask	= ATA_PIO4,
		.udma_mask	= ATA_UDMA6,
		.port_ops	= &ahci_vt8251_ops,
	},
#ifdef CONFIG_SYNO_JMICRON_585_FIX
	[board_ahci_jmb585] = {
		.flags		= AHCI_FLAG_COMMON,
		.pio_mask	= ATA_PIO4,
		.udma_mask	= ATA_UDMA6,
		.port_ops	= &ahci_jmb585_ops,
	},
#endif  
};

static const struct pci_device_id ahci_pci_tbl[] = {
	 
	{ PCI_VDEVICE(INTEL, 0x2652), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2653), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x27c1), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x27c5), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x27c3), board_ahci },  
	{ PCI_VDEVICE(AL, 0x5288), board_ahci_ign_iferr },  
	{ PCI_VDEVICE(INTEL, 0x2681), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2682), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2683), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x27c6), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2821), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2822), board_ahci_nosntf },  
	{ PCI_VDEVICE(INTEL, 0x2824), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2829), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x282a), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2922), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2923), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2924), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2925), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2927), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2929), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x292a), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x292b), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x292c), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x292f), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x294d), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x294e), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x502a), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x502b), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x3a05), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x3a22), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x3a25), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x3b22), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x3b23), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x3b24), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x3b25), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x3b29), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x3b2b), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x3b2c), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x3b2f), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19b0), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19b1), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19b2), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19b3), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19b4), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19b5), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19b6), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19b7), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19bE), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19bF), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19c0), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19c1), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19c2), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19c3), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19c4), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19c5), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19c6), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19c7), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19cE), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19cF), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1c02), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1c03), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1c04), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1c05), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1c06), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1c07), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1d02), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1d04), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1d06), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2826), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2323), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1e02), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1e03), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1e04), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1e05), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1e06), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1e07), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1e0e), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8c02), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8c03), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8c04), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8c05), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8c06), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8c07), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8c0e), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8c0f), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x9c02), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x9c03), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x9c04), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x9c05), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x9c06), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x9c07), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x9c0e), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x9c0f), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1f22), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1f23), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1f24), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1f25), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1f26), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1f27), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1f2e), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1f2f), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x1f32), board_ahci_avn },  
	{ PCI_VDEVICE(INTEL, 0x1f33), board_ahci_avn },  
	{ PCI_VDEVICE(INTEL, 0x1f34), board_ahci_avn },  
	{ PCI_VDEVICE(INTEL, 0x1f35), board_ahci_avn },  
	{ PCI_VDEVICE(INTEL, 0x1f36), board_ahci_avn },  
	{ PCI_VDEVICE(INTEL, 0x1f37), board_ahci_avn },  
	{ PCI_VDEVICE(INTEL, 0x1f3e), board_ahci_avn },  
	{ PCI_VDEVICE(INTEL, 0x1f3f), board_ahci_avn },  
	{ PCI_VDEVICE(INTEL, 0x2823), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2827), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8d02), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8d04), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8d06), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8d0e), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8d62), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8d64), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8d66), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8d6e), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x23a3), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x9c83), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x9c85), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x9c87), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x9c8f), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19b0), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19b1), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19b2), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19b3), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19b4), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19b5), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19b6), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19b7), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19be), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19bf), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19c0), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19c1), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19c2), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19c3), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19c4), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19c5), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19c6), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19c7), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19ce), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x19cf), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8c82), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8c83), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8c84), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8c85), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8c86), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8c87), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8c8e), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x8c8f), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x9d03), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x9d05), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x9d07), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa102), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa103), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa105), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa106), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa107), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa10f), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2822), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2823), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2826), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0x2827), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa182), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa184), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa186), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa18e), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa1d2), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa1d6), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa202), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa204), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa206), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa20e), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa252), board_ahci },  
	{ PCI_VDEVICE(INTEL, 0xa256), board_ahci },  

#ifdef CONFIG_SYNO_JMICRON_585_DUBIOUS_IFS_FIX
	 
	{ PCI_VDEVICE(JMICRON, 0x0585), board_ahci_jmb585 },
#endif  
	 
	{ PCI_VENDOR_ID_JMICRON, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	  PCI_CLASS_STORAGE_SATA_AHCI, 0xffffff, board_ahci_ign_iferr },
	 
	{ PCI_VDEVICE(JMICRON, 0x2362), board_ahci_ign_iferr },
	{ PCI_VDEVICE(JMICRON, 0x236f), board_ahci_ign_iferr },
	 
	{ PCI_VDEVICE(ATI, 0x4380), board_ahci_sb600 },  
	{ PCI_VDEVICE(ATI, 0x4390), board_ahci_sb700 },  
	{ PCI_VDEVICE(ATI, 0x4391), board_ahci_sb700 },  
	{ PCI_VDEVICE(ATI, 0x4392), board_ahci_sb700 },  
	{ PCI_VDEVICE(ATI, 0x4393), board_ahci_sb700 },  
	{ PCI_VDEVICE(ATI, 0x4394), board_ahci_sb700 },  
	{ PCI_VDEVICE(ATI, 0x4395), board_ahci_sb700 },  

	{ PCI_VDEVICE(AMD, 0x7800), board_ahci },  
	{ PCI_VDEVICE(AMD, 0x7900), board_ahci },  
	 
	{ PCI_VENDOR_ID_AMD, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	  PCI_CLASS_STORAGE_RAID << 8, 0xffffff, board_ahci },

	{ PCI_VDEVICE(VIA, 0x3349), board_ahci_vt8251 },  
	{ PCI_VDEVICE(VIA, 0x6287), board_ahci_vt8251 },  

	{ PCI_VDEVICE(NVIDIA, 0x044c), board_ahci_mcp65 },	 
	{ PCI_VDEVICE(NVIDIA, 0x044d), board_ahci_mcp65 },	 
	{ PCI_VDEVICE(NVIDIA, 0x044e), board_ahci_mcp65 },	 
	{ PCI_VDEVICE(NVIDIA, 0x044f), board_ahci_mcp65 },	 
	{ PCI_VDEVICE(NVIDIA, 0x045c), board_ahci_mcp65 },	 
	{ PCI_VDEVICE(NVIDIA, 0x045d), board_ahci_mcp65 },	 
	{ PCI_VDEVICE(NVIDIA, 0x045e), board_ahci_mcp65 },	 
	{ PCI_VDEVICE(NVIDIA, 0x045f), board_ahci_mcp65 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0550), board_ahci_mcp67 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0551), board_ahci_mcp67 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0552), board_ahci_mcp67 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0553), board_ahci_mcp67 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0554), board_ahci_mcp67 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0555), board_ahci_mcp67 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0556), board_ahci_mcp67 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0557), board_ahci_mcp67 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0558), board_ahci_mcp67 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0559), board_ahci_mcp67 },	 
	{ PCI_VDEVICE(NVIDIA, 0x055a), board_ahci_mcp67 },	 
	{ PCI_VDEVICE(NVIDIA, 0x055b), board_ahci_mcp67 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0580), board_ahci_mcp_linux },	 
	{ PCI_VDEVICE(NVIDIA, 0x0581), board_ahci_mcp_linux },	 
	{ PCI_VDEVICE(NVIDIA, 0x0582), board_ahci_mcp_linux },	 
	{ PCI_VDEVICE(NVIDIA, 0x0583), board_ahci_mcp_linux },	 
	{ PCI_VDEVICE(NVIDIA, 0x0584), board_ahci_mcp_linux },	 
	{ PCI_VDEVICE(NVIDIA, 0x0585), board_ahci_mcp_linux },	 
	{ PCI_VDEVICE(NVIDIA, 0x0586), board_ahci_mcp_linux },	 
	{ PCI_VDEVICE(NVIDIA, 0x0587), board_ahci_mcp_linux },	 
	{ PCI_VDEVICE(NVIDIA, 0x0588), board_ahci_mcp_linux },	 
	{ PCI_VDEVICE(NVIDIA, 0x0589), board_ahci_mcp_linux },	 
	{ PCI_VDEVICE(NVIDIA, 0x058a), board_ahci_mcp_linux },	 
	{ PCI_VDEVICE(NVIDIA, 0x058b), board_ahci_mcp_linux },	 
	{ PCI_VDEVICE(NVIDIA, 0x058c), board_ahci_mcp_linux },	 
	{ PCI_VDEVICE(NVIDIA, 0x058d), board_ahci_mcp_linux },	 
	{ PCI_VDEVICE(NVIDIA, 0x058e), board_ahci_mcp_linux },	 
	{ PCI_VDEVICE(NVIDIA, 0x058f), board_ahci_mcp_linux },	 
	{ PCI_VDEVICE(NVIDIA, 0x07f0), board_ahci_mcp73 },	 
	{ PCI_VDEVICE(NVIDIA, 0x07f1), board_ahci_mcp73 },	 
	{ PCI_VDEVICE(NVIDIA, 0x07f2), board_ahci_mcp73 },	 
	{ PCI_VDEVICE(NVIDIA, 0x07f3), board_ahci_mcp73 },	 
	{ PCI_VDEVICE(NVIDIA, 0x07f4), board_ahci_mcp73 },	 
	{ PCI_VDEVICE(NVIDIA, 0x07f5), board_ahci_mcp73 },	 
	{ PCI_VDEVICE(NVIDIA, 0x07f6), board_ahci_mcp73 },	 
	{ PCI_VDEVICE(NVIDIA, 0x07f7), board_ahci_mcp73 },	 
	{ PCI_VDEVICE(NVIDIA, 0x07f8), board_ahci_mcp73 },	 
	{ PCI_VDEVICE(NVIDIA, 0x07f9), board_ahci_mcp73 },	 
	{ PCI_VDEVICE(NVIDIA, 0x07fa), board_ahci_mcp73 },	 
	{ PCI_VDEVICE(NVIDIA, 0x07fb), board_ahci_mcp73 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ad0), board_ahci_mcp77 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ad1), board_ahci_mcp77 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ad2), board_ahci_mcp77 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ad3), board_ahci_mcp77 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ad4), board_ahci_mcp77 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ad5), board_ahci_mcp77 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ad6), board_ahci_mcp77 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ad7), board_ahci_mcp77 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ad8), board_ahci_mcp77 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ad9), board_ahci_mcp77 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ada), board_ahci_mcp77 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0adb), board_ahci_mcp77 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ab4), board_ahci_mcp79 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ab5), board_ahci_mcp79 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ab6), board_ahci_mcp79 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ab7), board_ahci_mcp79 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ab8), board_ahci_mcp79 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0ab9), board_ahci_mcp79 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0aba), board_ahci_mcp79 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0abb), board_ahci_mcp79 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0abc), board_ahci_mcp79 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0abd), board_ahci_mcp79 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0abe), board_ahci_mcp79 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0abf), board_ahci_mcp79 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0d84), board_ahci_mcp89 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0d85), board_ahci_mcp89 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0d86), board_ahci_mcp89 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0d87), board_ahci_mcp89 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0d88), board_ahci_mcp89 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0d89), board_ahci_mcp89 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0d8a), board_ahci_mcp89 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0d8b), board_ahci_mcp89 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0d8c), board_ahci_mcp89 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0d8d), board_ahci_mcp89 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0d8e), board_ahci_mcp89 },	 
	{ PCI_VDEVICE(NVIDIA, 0x0d8f), board_ahci_mcp89 },	 

	{ PCI_VDEVICE(SI, 0x1184), board_ahci },		 
	{ PCI_VDEVICE(SI, 0x1185), board_ahci },		 
	{ PCI_VDEVICE(SI, 0x0186), board_ahci },		 

	{ PCI_VDEVICE(STMICRO, 0xCC06), board_ahci },		 

	{ PCI_VDEVICE(MARVELL, 0x6145), board_ahci_mv },	 
	{ PCI_VDEVICE(MARVELL, 0x6121), board_ahci_mv },	 
	{ PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9123),
	  .class = PCI_CLASS_STORAGE_SATA_AHCI,
	  .class_mask = 0xffffff,
	  .driver_data = board_ahci_yes_fbs },			 
	{ PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9125),
	  .driver_data = board_ahci_yes_fbs },			 
	{ PCI_DEVICE_SUB(PCI_VENDOR_ID_MARVELL_EXT, 0x9178,
			 PCI_VENDOR_ID_MARVELL_EXT, 0x9170),
	  .driver_data = board_ahci_yes_fbs },			 
	{ PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x917a),
	  .driver_data = board_ahci_yes_fbs },			 
	{ PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9172),
	  .driver_data = board_ahci_yes_fbs },			 
	{ PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9182),
	  .driver_data = board_ahci_yes_fbs },			 
	{ PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9192),
	  .driver_data = board_ahci_yes_fbs },			 
	{ PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x91a0),
	  .driver_data = board_ahci_yes_fbs },
	{ PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x91a2), 	 
	  .driver_data = board_ahci_yes_fbs },
	{ PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x91a3),
	  .driver_data = board_ahci_yes_fbs },
	{ PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9230),
	  .driver_data = board_ahci_yes_fbs },
	{ PCI_DEVICE(PCI_VENDOR_ID_TTI, 0x0642),
	  .driver_data = board_ahci_yes_fbs },
#ifdef MY_ABC_HERE
	{ PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9235),
	  .driver_data = board_ahci_yes_fbs },			 
	{ PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9215),
	  .driver_data = board_ahci_yes_fbs },			 
#endif  
#ifdef MY_DEF_HERE
	{ PCI_DEVICE(PCI_VENDOR_ID_MARVELL_EXT, 0x9170),
	  .driver_data = board_ahci_yes_fbs_no_ncq },    
#endif  

	{ PCI_VDEVICE(PROMISE, 0x3f20), board_ahci },	 
	{ PCI_VDEVICE(PROMISE, 0x3781), board_ahci },    

	{ PCI_VDEVICE(ASMEDIA, 0x0601), board_ahci },	 
	{ PCI_VDEVICE(ASMEDIA, 0x0602), board_ahci },	 
	{ PCI_VDEVICE(ASMEDIA, 0x0611), board_ahci },	 
	{ PCI_VDEVICE(ASMEDIA, 0x0612), board_ahci },	 

	{ PCI_VDEVICE(SAMSUNG, 0x1600), board_ahci_nomsi },
	{ PCI_VDEVICE(SAMSUNG, 0xa800), board_ahci_nomsi },

	{ PCI_DEVICE(0x1c44, 0x8000), board_ahci },

	{ PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID,
	  PCI_CLASS_STORAGE_SATA_AHCI, 0xffffff, board_ahci },

	{ }	 
};

#ifdef CONFIG_SYNO_MV_9170_GPIO_CTRL
 
enum {
	MV_9170_GPIO_DATA_OUT			= 0x220,
	MV_9170_GPIO_DATA_OUT_ENABLE		= 0x224,
	MV_9170_GPIO_ACTIVE			= 0x258,
	MV_9170_VENDOR_SPEC1_ADDR_OFFSET	= 0xA8,			 
	MV_9170_VENDOR_SPEC1_DATA_OFFSET	= 0xAC,
};

u32 syno_mv_9170_gpio_reg_read(struct ata_host *host, const unsigned int gpioaddr)
{
	void __iomem *host_mmio = NULL;
	u32 value = 0;

	host_mmio = ahci_host_base(host);
	if (NULL == host_mmio) {
		goto END;
	}

	writel(gpioaddr, host_mmio + MV_9170_VENDOR_SPEC1_ADDR_OFFSET);
	 
	value = readl(host_mmio + MV_9170_VENDOR_SPEC1_DATA_OFFSET);
END:
	return value;
}

void syno_mv_9170_gpio_reg_set(struct ata_host *host, const unsigned int gpioaddr, u32 value)
{
	void __iomem *host_mmio = NULL;
	u32 reg_val;

	host_mmio = ahci_host_base(host);
	if (NULL == host_mmio) {
		goto END;
	}

	writel(gpioaddr, host_mmio + MV_9170_VENDOR_SPEC1_ADDR_OFFSET);
	 
	reg_val = readl(host_mmio + MV_9170_VENDOR_SPEC1_DATA_OFFSET);
	 
	writel(value, host_mmio + MV_9170_VENDOR_SPEC1_DATA_OFFSET);
END:
	return;
}

void syno_mv_9170_gpio_active_init(struct ata_host *host)
{
	 
	syno_mv_9170_gpio_reg_set(host, MV_9170_GPIO_DATA_OUT_ENABLE, 0x0);
#if defined(CONFIG_SYNO_MV_9170_GPIO_ACTIVE_LOW)
	 
	syno_mv_9170_gpio_reg_set(host, MV_9170_GPIO_DATA_OUT, 0x00000021);
#else
	 
	syno_mv_9170_gpio_reg_set(host, MV_9170_GPIO_DATA_OUT, 0x0);
#endif  

	syno_mv_9170_gpio_reg_set(host, MV_9170_GPIO_ACTIVE, 0x00743217);
}

static int syno_mv_9170_gpio_disk_map(const unsigned short hostnum)
{
	 
	if (0 == hostnum) {
		return 0;
	} else if (1 == hostnum) {
		return 5;
	} else {
		printk("ERROR: There is no definition for disk %d in MV9170\n",hostnum);
		return -1;
	}
}

int syno_mv_9170_disk_led_get(const unsigned short hostnum)
{
	struct Scsi_Host *shost = scsi_host_lookup(hostnum);
	struct ata_port *ap = NULL;
	int ret = -1;
	u32 value;
	int led_idx;

	if (NULL == shost) {
		goto END;
	}

	ap = ata_shost_to_port(shost);
	if (NULL == ap) {
		goto END;
	}

	if (-1 == (led_idx = syno_mv_9170_gpio_disk_map(hostnum))) {
		goto END;
	}

	value = syno_mv_9170_gpio_reg_read(ap->host, MV_9170_GPIO_DATA_OUT);

	if (value & (1 << led_idx)) {
		ret = 1;
	} else {
		ret = 0;
	}
#if defined(CONFIG_SYNO_MV_9170_GPIO_ACTIVE_LOW)
	ret = !ret;
#endif  
END:
	if (NULL != shost) {
		scsi_host_put(shost);
	}
	return ret;
}
EXPORT_SYMBOL(syno_mv_9170_disk_led_get);

static u32 syno_mv_9170_prepare_value_by_polarity(u32 orig_reg_value, int iValue, const int led_idx)
{
	u32 target_reg_value = orig_reg_value;
	int new_bit_value;
#if defined(CONFIG_SYNO_MV_9170_GPIO_ACTIVE_LOW)
        new_bit_value = !iValue;
#else  
        new_bit_value = iValue;
#endif  

	target_reg_value &= ~(1 << led_idx);

	target_reg_value |= (new_bit_value << led_idx);

	return target_reg_value;
}

int __syno_mv_9170_disk_led_set(const struct ata_port *ap, int iValue)
{
	int ret = -EINVAL;
	u32 value;
	int led_idx;

	if (NULL == ap ||-1 == (led_idx = syno_mv_9170_gpio_disk_map(ap->port_no))) {
		goto END;
	}

	value = syno_mv_9170_gpio_reg_read(ap->host, MV_9170_GPIO_DATA_OUT);
	value = syno_mv_9170_prepare_value_by_polarity(value, iValue, led_idx);
	syno_mv_9170_gpio_reg_set(ap->host, MV_9170_GPIO_DATA_OUT, value);
	ret = 0;

END:
	return ret;
}
#ifdef CONFIG_SYNO_PORT_MAPPING_V2
 
int syno_mv_9170_disk_led_set_by_port(const int iDiskPort, int iValue)
{
	struct ata_port *ap;
	ap = syno_ata_port_get_by_port(iDiskPort);
	return __syno_mv_9170_disk_led_set(ap, iValue);
}
EXPORT_SYMBOL(syno_mv_9170_disk_led_set_by_port);
#endif  

int syno_mv_9170_disk_led_set(const unsigned short hostnum, int iValue)
{
	struct Scsi_Host *shost = scsi_host_lookup(hostnum);
	struct ata_port *ap = NULL;
	int ret = -EINVAL;

	if (NULL == shost) {
		goto END;
	}

	ap = ata_shost_to_port(shost);
	if (NULL == ap) {
		goto END;
	}

	ret = __syno_mv_9170_disk_led_set(ap, iValue);

END:
	if (NULL != shost) {
		scsi_host_put(shost);
	}
	return ret;
}
EXPORT_SYMBOL(syno_mv_9170_disk_led_set);
#endif  

#if defined(CONFIG_SYNO_MV_9170_GPIO_CTRL) || defined(MY_ABC_HERE)
#define MV_GEN 3
#define MV_PORT 4
 
const unsigned int mv_sata_gen[MV_GEN] = {0x8D, 0x8F, 0x91};
const unsigned mv_port_addr[MV_PORT] = {0x178, 0x1f8, 0x278, 0x2f8};
const unsigned mv_port_data[MV_PORT] = {0x17c, 0x1fc, 0x27c, 0x2fc};
#endif  

#ifdef MY_ABC_HERE
 
enum {
	 
	MV_9235_GPIO_DATA_OUT			= 0x07071020,
	MV_9235_GPIO_DATA_OUT_ENABLE		= 0x07071024,
	MV_9235_GPIO_ACTIVE			= 0x07071058,
	MV_9235_VENDOR_SPEC1_ADDR_OFFSET	= 0xA8,			 
	MV_9235_VENDOR_SPEC1_DATA_OFFSET	= 0xAC,
};

u32 syno_mv_9235_gpio_reg_read(struct ata_host *host, const unsigned int gpioaddr)
{
	void __iomem *host_mmio = NULL;
	u32 value = 0;

	if(NULL == (host_mmio = ahci_host_base(host))) {
		goto END;
	}

	writel(gpioaddr, host_mmio + MV_9235_VENDOR_SPEC1_ADDR_OFFSET);
	 
	value = readl(host_mmio + MV_9235_VENDOR_SPEC1_DATA_OFFSET);
END:
	return value;
}

void syno_mv_9235_gpio_reg_set(struct ata_host *host, const unsigned int gpioaddr, u32 value)
{
	void __iomem *host_mmio = NULL;
	u32 reg_val;

	if(NULL == (host_mmio = ahci_host_base(host))) {
		goto END;
	}

	writel(gpioaddr, host_mmio + MV_9235_VENDOR_SPEC1_ADDR_OFFSET);
	 
	reg_val = readl(host_mmio + MV_9235_VENDOR_SPEC1_DATA_OFFSET);
	 
	writel(value, host_mmio + MV_9235_VENDOR_SPEC1_DATA_OFFSET);
END:
	return;
}
#endif  

#if defined(CONFIG_SYNO_MV_9170_GPIO_CTRL) || defined(MY_ABC_HERE)
 
static u32 syno_mv_9xxx_reg_get(struct ata_host *host, const unsigned int reg_addr, unsigned int addr_offset, unsigned int data_offset)
{
	void __iomem *host_mmio = NULL;
	u32 value = 0;

	if(NULL == (host_mmio = ahci_host_base(host))) {
		goto END;
	}

	writel(reg_addr, host_mmio + addr_offset);
	 
	value = readl(host_mmio + data_offset);
END:
	return value;
}

static void syno_mv_9xxx_reg_set(struct ata_host *host, const unsigned int reg_addr, u32 value, const unsigned int addr_offset, const unsigned int data_offset)
{
	void __iomem *host_mmio = NULL;

	if(NULL == (host_mmio = ahci_host_base(host))) {
		goto END;
	}

	writel(reg_addr, host_mmio + addr_offset);
	 
	writel(value, host_mmio + data_offset);
END:
	return;
}
#endif  

#ifdef MY_ABC_HERE
 
void syno_mv_9235_gpio_active_init(struct ata_host *host)
{
	 
	syno_mv_9235_gpio_reg_set(host, MV_9235_GPIO_DATA_OUT_ENABLE, 0x0);
	syno_mv_9235_gpio_reg_set(host, MV_9235_GPIO_DATA_OUT, 0x0);
	 
	syno_mv_9235_gpio_reg_set(host, MV_9235_GPIO_ACTIVE, 0x00B6D8D1);
}
#endif  

#if defined(CONFIG_SYNO_MV_9170_GPIO_CTRL) || defined(MY_ABC_HERE)
void syno_mv_9xxx_amp_adjust_by_port(struct ata_host *host, u32 val, unsigned int addr_offset, const unsigned int data_offset, const unsigned int reg_addr)
{
	u32 reg_val = 0;

	reg_val = syno_mv_9xxx_reg_get(host, 0x0E, addr_offset, data_offset);
	syno_mv_9xxx_reg_set(host, 0xE, reg_val & ~0x100, addr_offset, data_offset);
	reg_val = syno_mv_9xxx_reg_get(host, reg_addr, addr_offset, data_offset);
	 
	val &= 0xFBE;
	reg_val &= ~0xFBE;
	reg_val |= val;
	syno_mv_9xxx_reg_set(host, reg_addr, reg_val, addr_offset, data_offset);
}

void syno_mv_9xxx_amp_adjust(struct ata_host *host, struct pci_dev *pdev)
{
	int port = 0;

	if (syno_is_hw_version(HW_RS2416p) || syno_is_hw_version(HW_RS2416rpp)) {
		for (port = 0; port < 4; port++) {
			 
			syno_mv_9xxx_amp_adjust_by_port(host, 0xFBE, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		}
	} else if (syno_is_hw_version(HW_DS916p) || syno_is_hw_version(HW_DS416play)) {
		 
		port = 0;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xFBE, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		port = 1;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xFBE, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);

		port = 2;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xFBE, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
	} else if (syno_is_hw_version(HW_DS716p) || syno_is_hw_version(HW_DS716pII) || syno_is_hw_version(HW_DS718p)) {
		 
		port = 0;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xFBE, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xFBE, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xFBE, mv_port_addr[port], mv_port_data[port], mv_sata_gen[0]);
	} else if (syno_is_hw_version(HW_DS1616p)) {
		 
		port = 0;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xE3E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		port = 1;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xE3E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
	} else if (syno_is_hw_version(HW_RS2418p) || syno_is_hw_version(HW_RS2418rpp)) {
		if (0x09 == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE7F, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xEFF, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 3;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xEFF, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		} else if (0x0A == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xDF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xDF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 3;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xDF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		} else if (0x0B == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 3;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		} else if (0x0C == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xEF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xEF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 3;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
                } else if (0x0E == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			for (port = 0; port < 4; port++) {
				syno_mv_9xxx_amp_adjust_by_port(host, 0xA75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
				syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			}
		}
	} else if (syno_is_hw_version(HW_DS218p)) {
		 
		port = 0;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xE3E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xA30, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
	} else if (syno_is_hw_version(HW_DS1517p)) {
		if (0x02 == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 0;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xD75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAE7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAE7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xD75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAE7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		} else if (0x03 == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 0;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xD75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAE7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAE7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		}
	} else if (syno_is_hw_version(HW_DS1817p)) {
		if (0x02 == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 0;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xD75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAE7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAE7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAE7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 3;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAE7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		} else if (0x03 == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 0;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xCF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAE7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAE7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAE7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 3;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAE7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		}
	} else if (syno_is_hw_version(HW_DS3018xs)) {
		if (0x03 == PCI_SLOT(pdev->bus->self->devfn) && (0x00 == PCI_FUNC(pdev->bus->self->devfn) || 0x01 == PCI_FUNC(pdev->bus->self->devfn))) {
			 
			port = 0;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xF7B, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xF7B, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xFF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 3;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xF77, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		} else if (0x03 == PCI_SLOT(pdev->bus->self->devfn) && 0x02 == PCI_FUNC(pdev->bus->self->devfn)) {
			 
			port = 0;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xD7F, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xD7F, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		}
	} else if (syno_is_hw_version(HW_DS918p)) {
		 
		port = 0;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xC962, mv_port_addr[port], mv_port_data[port], mv_sata_gen[0]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xAA7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xE7F, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		port = 1;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xC962, mv_port_addr[port], mv_port_data[port], mv_sata_gen[0]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xAA7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xE7F, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		port = 2;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xC962, mv_port_addr[port], mv_port_data[port], mv_sata_gen[0]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xAA7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xE7F, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
	} else if (syno_is_hw_version(HW_RS819)) {
		port = 1;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xD75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
	} else if (syno_is_hw_version(HW_FS1018)) {
		if (0x03 == PCI_SLOT(pdev->bus->self->devfn) && (0x00 == PCI_FUNC(pdev->bus->self->devfn) || 0x01 == PCI_FUNC(pdev->bus->self->devfn))) {
			 
			port = 0;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xF7B, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xF7B, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xFF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 3;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xF77, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		}
	} else if (syno_is_hw_version(HW_DS219j)) {
		port = 0;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xD75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		port = 1;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xD6D, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
	} else if (syno_is_hw_version(HW_DS219se)) {
		port = 0;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xD75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		port = 1;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xD6D, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
	} else if (syno_is_hw_version(HW_RS818p) || syno_is_hw_version(HW_RS818rpp)) {
		if (0x02 == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xFFF, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAE7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			port = 3;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		} else if (0x03 == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xB75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		}
	} else if (syno_is_hw_version(HW_RS2818rpp)) {
		if (0x9215 == pdev->device) {
			 
			for (port = 0; port < 4; port++) {
				syno_mv_9xxx_amp_adjust_by_port(host, 0xE7F, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
				syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			}
		} else if (0x9235 == pdev->device && 0x0E == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			for (port = 0; port < 4; port++) {
				syno_mv_9xxx_amp_adjust_by_port(host, 0xA75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
				syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			}
		}
	} else if (syno_is_hw_version(HW_DS3619xs)) {
		if (0x01 == PCI_SLOT(pdev->bus->self->devfn)) {
			if (0x01 == PCI_FUNC(pdev->bus->self->devfn)) {
				 
				port = 0;
				syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
				port = 1;
				syno_mv_9xxx_amp_adjust_by_port(host, 0xE7F, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
				port = 2;
				syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			} else if (0x02 == PCI_FUNC(pdev->bus->self->devfn)) {
				 
				port = 0;
				syno_mv_9xxx_amp_adjust_by_port(host, 0xEFB, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
				port = 1;
				syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
				port = 2;
				syno_mv_9xxx_amp_adjust_by_port(host, 0xFFD, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			}
		} else if (0x1c == PCI_SLOT(pdev->bus->self->devfn)) {
			if (0x00 == PCI_FUNC(pdev->bus->self->devfn)) {
				 
				port = 0;
				syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
				port = 1;
				syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
				port = 2;
				syno_mv_9xxx_amp_adjust_by_port(host, 0xE7F, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
				port = 3;
				syno_mv_9xxx_amp_adjust_by_port(host, 0xE7F, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			} else if (0x06 == PCI_FUNC(pdev->bus->self->devfn)) {
				 
				port = 0;
				syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
				port = 1;
				syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
				port = 2;
				syno_mv_9xxx_amp_adjust_by_port(host, 0xE71, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
				port = 3;
				syno_mv_9xxx_amp_adjust_by_port(host, 0xE71, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			}
		}
	} else if (syno_is_hw_version(HW_DS2419p)) {
		if (0x09 == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xF75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 3;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xFF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		} else if (0x0A == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xF75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 3;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		} else if (0x0B == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xEF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xD75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 3;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xEF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		} else if (0x0C == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xD75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xEF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 3;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xA75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		} else if (0x0E == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 0;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xDF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xDF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xD75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 3;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xD75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		}
	} else if (syno_is_hw_version(HW_DS1019p)) {
		 
		port = 0;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xC962, mv_port_addr[port], mv_port_data[port], mv_sata_gen[0]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		port = 1;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xC962, mv_port_addr[port], mv_port_data[port], mv_sata_gen[0]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		port = 2;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xC962, mv_port_addr[port], mv_port_data[port], mv_sata_gen[0]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		port = 3;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xC962, mv_port_addr[port], mv_port_data[port], mv_sata_gen[0]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xAA7E, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xE7F, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
	} else if (syno_is_hw_version(HW_DS620slim)) {
		 
		port = 0;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xC962, mv_port_addr[port], mv_port_data[port], mv_sata_gen[0]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xBF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		port = 1;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xC962, mv_port_addr[port], mv_port_data[port], mv_sata_gen[0]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xBF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		port = 2;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xC962, mv_port_addr[port], mv_port_data[port], mv_sata_gen[0]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xAF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		port = 3;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xC962, mv_port_addr[port], mv_port_data[port], mv_sata_gen[0]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
		syno_mv_9xxx_amp_adjust_by_port(host, 0x8F5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
	} else if (syno_is_hw_version(HW_DS1819p)) {
		if (0x0e == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 0;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xD75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 3;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xA75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		} else if (0x10 == PCI_SLOT(pdev->bus->self->devfn)) {
			 
			port = 0;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xD75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xD75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 2;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 3;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		}	
	} else if (syno_is_hw_version(HW_DVA3219) || syno_is_hw_version(HW_DVA3221)) {
		if (0x10 == PCI_SLOT(pdev->bus->self->devfn)) {
                         
                        port = 0;
                        syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
                        syno_mv_9xxx_amp_adjust_by_port(host, 0xE75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
                        port = 1;
                        syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
                        syno_mv_9xxx_amp_adjust_by_port(host, 0xFF9, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
                }
	} else if (syno_is_hw_version(HW_DS720p)) {
		 
		port = 0;
		syno_mv_9xxx_amp_adjust_by_port(host, 0xA75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
	} else if (syno_is_hw_version(HW_RS820p) || syno_is_hw_version(HW_RS820rpp)) {
		if (0x11 == PCI_SLOT(pdev->bus->self->devfn)) {
			port = 0;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xAA62, mv_port_addr[port], mv_port_data[port], mv_sata_gen[1]);
			syno_mv_9xxx_amp_adjust_by_port(host, 0xCF5, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		}
	} else if (syno_is_hw_version(HW_DS1520p)) {
		if (0x9170 == pdev->device) {
			port = 0;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xD75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
			port = 1;
			syno_mv_9xxx_amp_adjust_by_port(host, 0xD75, mv_port_addr[port], mv_port_data[port], mv_sata_gen[2]);
		}
	} 
}
#endif  

#ifdef MY_ABC_HERE
int __syno_mv_9235_disk_led_get(const struct ata_port *ap)
{
	int ret = -1;
	u32 value;
	int led_idx;

	if (NULL == ap) {
		goto END;
	}

	led_idx = ap->print_id - ap->host->ports[0]->print_id + 4;

	value = syno_mv_9235_gpio_reg_read(ap->host, MV_9235_GPIO_DATA_OUT);

	if (value & (1 << led_idx)) {
		ret = 1;
	} else {
		ret = 0;
	}
END:
	return ret;
}

int syno_mv_9235_disk_led_get(const unsigned short hostnum)
{
	struct Scsi_Host *shost = scsi_host_lookup(hostnum);
	struct ata_port *ap = NULL;
	int ret = -1;

	if (NULL == shost) {
		goto END;
	}

	if (NULL == (ap = ata_shost_to_port(shost))) {
		goto END;
	}

	ret = __syno_mv_9235_disk_led_get(ap);
END:
	if (NULL != shost) {
		scsi_host_put(shost);
	}
	return ret;
}
EXPORT_SYMBOL(syno_mv_9235_disk_led_get);

#ifdef CONFIG_SYNO_PORT_MAPPING_V2
 
int syno_mv_9235_disk_led_get_by_port(const int iDiskPort)
{
	struct ata_port *ap;
	ap = syno_ata_port_get_by_port(iDiskPort);
	return __syno_mv_9235_disk_led_get(ap);
}
EXPORT_SYMBOL(syno_mv_9235_disk_led_get_by_port);
#endif  

int __syno_mv_9235_disk_led_set(const struct ata_port *ap, int iValue)
{
	int led_idx;
	u32 value;
	int ret = -EINVAL;
	if (NULL == ap) {
		goto END;
	}
	led_idx = ap->print_id - ap->host->ports[0]->print_id + 4;
	value = syno_mv_9235_gpio_reg_read(ap->host, MV_9235_GPIO_DATA_OUT);
	if (1 == iValue) {
		value |= (1 << led_idx);
	} else {
		value &= ~(1 << led_idx);
	}
	syno_mv_9235_gpio_reg_set(ap->host, MV_9235_GPIO_DATA_OUT, value);
	ret = 0;
END:
	return ret;
}

#ifdef CONFIG_SYNO_PORT_MAPPING_V2
 
int syno_mv_9235_disk_led_set_by_port(const int iDiskPort, int iValue)
{
	struct ata_port *ap;
	ap = syno_ata_port_get_by_port(iDiskPort);
	return __syno_mv_9235_disk_led_set(ap, iValue);
}
EXPORT_SYMBOL(syno_mv_9235_disk_led_set_by_port);
#endif  

int syno_mv_9235_disk_led_set(const unsigned short hostnum, int iValue)
{
	struct Scsi_Host *shost = scsi_host_lookup(hostnum);
	struct ata_port *ap = NULL;
	int ret = -EINVAL;

	if(NULL == shost) {
		goto END;
	}

	if(NULL == (ap = ata_shost_to_port(shost))) {
		goto END;
	}

	ret = __syno_mv_9235_disk_led_set(ap, iValue);

END:
	if (NULL != shost) {
		scsi_host_put(shost);
	}
	return ret;
}

EXPORT_SYMBOL(syno_mv_9235_disk_led_set);
#endif  

static struct pci_driver ahci_pci_driver = {
	.name			= DRV_NAME,
	.id_table		= ahci_pci_tbl,
	.probe			= ahci_init_one,
	.remove			= ata_pci_remove_one,
#ifdef CONFIG_PM
	.suspend		= ahci_pci_device_suspend,
	.resume			= ahci_pci_device_resume,
#endif
};

#if defined(CONFIG_PATA_MARVELL) || defined(CONFIG_PATA_MARVELL_MODULE)
static int marvell_enable;
#else
static int marvell_enable = 1;
#endif
module_param(marvell_enable, int, 0644);
MODULE_PARM_DESC(marvell_enable, "Marvell SATA via AHCI (1 = enabled)");

static void ahci_pci_save_initial_config(struct pci_dev *pdev,
					 struct ahci_host_priv *hpriv)
{
	if (pdev->vendor == PCI_VENDOR_ID_JMICRON && pdev->device == 0x2361) {
		dev_info(&pdev->dev, "JMB361 has only one port\n");
		hpriv->force_port_map = 1;
	}

	if (hpriv->flags & AHCI_HFLAG_MV_PATA) {
		if (pdev->device == 0x6121)
			hpriv->mask_port_map = 0x3;
		else
			hpriv->mask_port_map = 0xf;
		dev_info(&pdev->dev,
			  "Disabling your PATA port. Use the boot option 'ahci.marvell_enable=0' to avoid this.\n");
	}

	ahci_save_initial_config(&pdev->dev, hpriv);
}

static int ahci_pci_reset_controller(struct ata_host *host)
{
	struct pci_dev *pdev = to_pci_dev(host->dev);

	ahci_reset_controller(host);

	if (pdev->vendor == PCI_VENDOR_ID_INTEL) {
		struct ahci_host_priv *hpriv = host->private_data;
		u16 tmp16;

		pci_read_config_word(pdev, 0x92, &tmp16);
		if ((tmp16 & hpriv->port_map) != hpriv->port_map) {
			tmp16 |= hpriv->port_map;
			pci_write_config_word(pdev, 0x92, tmp16);
		}
	}

	return 0;
}

static void ahci_pci_init_controller(struct ata_host *host)
{
	struct ahci_host_priv *hpriv = host->private_data;
	struct pci_dev *pdev = to_pci_dev(host->dev);
	void __iomem *port_mmio;
	u32 tmp;
	int mv;

	if (hpriv->flags & AHCI_HFLAG_MV_PATA) {
		if (pdev->device == 0x6121)
			mv = 2;
		else
			mv = 4;
		port_mmio = __ahci_port_base(host, mv);

		writel(0, port_mmio + PORT_IRQ_MASK);

		tmp = readl(port_mmio + PORT_IRQ_STAT);
		VPRINTK("PORT_IRQ_STAT 0x%x\n", tmp);
		if (tmp)
			writel(tmp, port_mmio + PORT_IRQ_STAT);
	}

	ahci_init_controller(host);
}

static int ahci_vt8251_hardreset(struct ata_link *link, unsigned int *class,
				 unsigned long deadline)
{
	struct ata_port *ap = link->ap;
	struct ahci_host_priv *hpriv = ap->host->private_data;
	bool online;
	int rc;

	DPRINTK("ENTER\n");

	ahci_stop_engine(ap);

	rc = sata_link_hardreset(link, sata_ehc_deb_timing(&link->eh_context),
				 deadline, &online, NULL);

	hpriv->start_engine(ap);

	DPRINTK("EXIT, rc=%d, class=%u\n", rc, *class);

	return online ? -EAGAIN : rc;
}

static int ahci_p5wdh_hardreset(struct ata_link *link, unsigned int *class,
				unsigned long deadline)
{
	struct ata_port *ap = link->ap;
	struct ahci_port_priv *pp = ap->private_data;
	struct ahci_host_priv *hpriv = ap->host->private_data;
	u8 *d2h_fis = pp->rx_fis + RX_FIS_D2H_REG;
	struct ata_taskfile tf;
	bool online;
	int rc;

	ahci_stop_engine(ap);

	ata_tf_init(link->device, &tf);
	tf.command = ATA_BUSY;
	ata_tf_to_fis(&tf, 0, 0, d2h_fis);

	rc = sata_link_hardreset(link, sata_ehc_deb_timing(&link->eh_context),
				 deadline, &online, NULL);

	hpriv->start_engine(ap);

	if (online) {
		rc = ata_wait_after_reset(link, jiffies + 2 * HZ,
					  ahci_check_ready);
		if (rc)
			ahci_kick_engine(ap);
	}
	return rc;
}

#ifdef CONFIG_SYNO_JMICRON_585_FIX
static int syno_ahci_hardreset_jmb(struct ata_link *link, unsigned int *class,
			  unsigned long deadline)
{
	const unsigned long *timing = sata_ehc_deb_timing(&link->eh_context);
	struct ata_port *ap = link->ap;
	struct ahci_port_priv *pp = ap->private_data;
	struct ahci_host_priv *hpriv = ap->host->private_data;
	u8 *d2h_fis = pp->rx_fis + RX_FIS_D2H_REG;
	struct ata_taskfile tf;
	bool online;
	int rc;
	void __iomem *port_mmio = ahci_port_base(link->ap);
	u32 uIRQStatus = 0;

	DPRINTK("ENTER\n");
	ahci_stop_engine(ap);

	ata_tf_init(link->device, &tf);
	tf.command = ATA_BUSY;
	ata_tf_to_fis(&tf, 0, 0, d2h_fis);

	uIRQStatus = readl(port_mmio + PORT_IRQ_MASK);
	writel(uIRQStatus & ~PORT_IRQ_BAD_PMP, port_mmio + PORT_IRQ_MASK);
	rc = sata_link_hardreset(link, timing, deadline, &online,
				 ahci_check_ready);
	writel(uIRQStatus, port_mmio + PORT_IRQ_MASK);

	hpriv->start_engine(ap);

	if (online)
		*class = ahci_dev_classify(ap);

	DPRINTK("EXIT, rc=%d, class=%u\n", rc, *class);
	return rc;
}

static int syno_ahci_softreset_jmb(struct ata_link *link, unsigned int *class,
			  unsigned long deadline)
{
	int pmp = sata_srst_pmp(link);
	int iRet = 0;
	void __iomem *port_mmio = ahci_port_base(link->ap);
	u32 uIRQStatus = 0;
	DPRINTK("ENTER\n");

	uIRQStatus = readl(port_mmio + PORT_IRQ_MASK);
	writel(uIRQStatus & ~PORT_IRQ_BAD_PMP, port_mmio + PORT_IRQ_MASK);
	iRet = ahci_do_softreset(link, class, pmp, deadline, ahci_check_ready);
	writel(uIRQStatus, port_mmio + PORT_IRQ_MASK);
	return iRet;
}
#endif  

static int ahci_avn_hardreset(struct ata_link *link, unsigned int *class,
			      unsigned long deadline)
{
	const unsigned long *timing = sata_ehc_deb_timing(&link->eh_context);
	struct ata_port *ap = link->ap;
	struct ahci_port_priv *pp = ap->private_data;
	struct ahci_host_priv *hpriv = ap->host->private_data;
	u8 *d2h_fis = pp->rx_fis + RX_FIS_D2H_REG;
	unsigned long tmo = deadline - jiffies;
	struct ata_taskfile tf;
	bool online;
	int rc, i;

	DPRINTK("ENTER\n");

	ahci_stop_engine(ap);

	for (i = 0; i < 2; i++) {
		u16 val;
		u32 sstatus;
		int port = ap->port_no;
		struct ata_host *host = ap->host;
		struct pci_dev *pdev = to_pci_dev(host->dev);

		ata_tf_init(link->device, &tf);
		tf.command = ATA_BUSY;
		ata_tf_to_fis(&tf, 0, 0, d2h_fis);

		rc = sata_link_hardreset(link, timing, deadline, &online,
				ahci_check_ready);

		if (sata_scr_read(link, SCR_STATUS, &sstatus) != 0 ||
				(sstatus & 0xf) != 1)
			break;

		ata_link_printk(link, KERN_INFO, "avn bounce port%d\n",
				port);

		pci_read_config_word(pdev, 0x92, &val);
		val &= ~(1 << port);
		pci_write_config_word(pdev, 0x92, val);
		ata_msleep(ap, 1000);
		val |= 1 << port;
		pci_write_config_word(pdev, 0x92, val);
		deadline += tmo;
	}

	hpriv->start_engine(ap);

	if (online)
		*class = ahci_dev_classify(ap);

	DPRINTK("EXIT, rc=%d, class=%u\n", rc, *class);
	return rc;
}

#ifdef CONFIG_PM
static int ahci_pci_device_suspend(struct pci_dev *pdev, pm_message_t mesg)
{
	struct ata_host *host = pci_get_drvdata(pdev);
	struct ahci_host_priv *hpriv = host->private_data;
	void __iomem *mmio = hpriv->mmio;
	u32 ctl;

	if (mesg.event & PM_EVENT_SUSPEND &&
	    hpriv->flags & AHCI_HFLAG_NO_SUSPEND) {
		dev_err(&pdev->dev,
			"BIOS update required for suspend/resume\n");
		return -EIO;
	}

	if (mesg.event & PM_EVENT_SLEEP) {
		 
		ctl = readl(mmio + HOST_CTL);
		ctl &= ~HOST_IRQ_EN;
		writel(ctl, mmio + HOST_CTL);
		readl(mmio + HOST_CTL);  
	}

	return ata_pci_device_suspend(pdev, mesg);
}

static int ahci_pci_device_resume(struct pci_dev *pdev)
{
	struct ata_host *host = pci_get_drvdata(pdev);
	int rc;

	rc = ata_pci_device_do_resume(pdev);
	if (rc)
		return rc;

	if (is_mcp89_apple(pdev))
		ahci_mcp89_apple_enable(pdev);

	if (pdev->dev.power.power_state.event == PM_EVENT_SUSPEND) {
		rc = ahci_pci_reset_controller(host);
		if (rc)
			return rc;

		ahci_pci_init_controller(host);
	}

	ata_host_resume(host);

	return 0;
}
#endif

static int ahci_configure_dma_masks(struct pci_dev *pdev, int using_dac)
{
	int rc;

	if (pdev->dma_mask && pdev->dma_mask < DMA_BIT_MASK(32))
		return 0;

	if (using_dac &&
	    !dma_set_mask(&pdev->dev, DMA_BIT_MASK(64))) {
		rc = dma_set_coherent_mask(&pdev->dev, DMA_BIT_MASK(64));
		if (rc) {
			rc = dma_set_coherent_mask(&pdev->dev, DMA_BIT_MASK(32));
			if (rc) {
				dev_err(&pdev->dev,
					"64-bit DMA enable failed\n");
				return rc;
			}
		}
	} else {
		rc = dma_set_mask(&pdev->dev, DMA_BIT_MASK(32));
		if (rc) {
			dev_err(&pdev->dev, "32-bit DMA enable failed\n");
			return rc;
		}
		rc = dma_set_coherent_mask(&pdev->dev, DMA_BIT_MASK(32));
		if (rc) {
			dev_err(&pdev->dev,
				"32-bit consistent DMA enable failed\n");
			return rc;
		}
	}
	return 0;
}

static void ahci_pci_print_info(struct ata_host *host)
{
	struct pci_dev *pdev = to_pci_dev(host->dev);
	u16 cc;
	const char *scc_s;

	pci_read_config_word(pdev, 0x0a, &cc);
	if (cc == PCI_CLASS_STORAGE_IDE)
		scc_s = "IDE";
	else if (cc == PCI_CLASS_STORAGE_SATA)
		scc_s = "SATA";
	else if (cc == PCI_CLASS_STORAGE_RAID)
		scc_s = "RAID";
	else
		scc_s = "unknown";

	ahci_print_info(host, scc_s);
}

static void ahci_p5wdh_workaround(struct ata_host *host)
{
	static const struct dmi_system_id sysids[] = {
		{
			.ident = "P5W DH Deluxe",
			.matches = {
				DMI_MATCH(DMI_SYS_VENDOR,
					  "ASUSTEK COMPUTER INC"),
				DMI_MATCH(DMI_PRODUCT_NAME, "P5W DH Deluxe"),
			},
		},
		{ }
	};
	struct pci_dev *pdev = to_pci_dev(host->dev);

	if (pdev->bus->number == 0 && pdev->devfn == PCI_DEVFN(0x1f, 2) &&
	    dmi_check_system(sysids)) {
		struct ata_port *ap = host->ports[1];

		dev_info(&pdev->dev,
			 "enabling ASUS P5W DH Deluxe on-board SIMG4726 workaround\n");

		ap->ops = &ahci_p5wdh_ops;
		ap->link.flags |= ATA_LFLAG_NO_SRST | ATA_LFLAG_ASSUME_ATA;
	}
}

static void ahci_mcp89_apple_enable(struct pci_dev *pdev)
{
	u32 val;

	printk(KERN_INFO "ahci: enabling MCP89 AHCI mode\n");

	pci_read_config_dword(pdev, 0xf8, &val);
	val |= 1 << 0x1b;
	 
	pci_write_config_dword(pdev, 0xf8, val);

	pci_read_config_dword(pdev, 0x54c, &val);
	val |= 1 << 0xc;
	pci_write_config_dword(pdev, 0x54c, val);

	pci_read_config_dword(pdev, 0x4a4, &val);
	val &= 0xff;
	val |= 0x01060100;
	pci_write_config_dword(pdev, 0x4a4, val);

	pci_read_config_dword(pdev, 0x54c, &val);
	val &= ~(1 << 0xc);
	pci_write_config_dword(pdev, 0x54c, val);

	pci_read_config_dword(pdev, 0xf8, &val);
	val &= ~(1 << 0x1b);
	pci_write_config_dword(pdev, 0xf8, val);
}

static bool is_mcp89_apple(struct pci_dev *pdev)
{
	return pdev->vendor == PCI_VENDOR_ID_NVIDIA &&
		pdev->device == PCI_DEVICE_ID_NVIDIA_NFORCE_MCP89_SATA &&
		pdev->subsystem_vendor == PCI_VENDOR_ID_APPLE &&
		pdev->subsystem_device == 0xcb89;
}

static bool ahci_sb600_enable_64bit(struct pci_dev *pdev)
{
	static const struct dmi_system_id sysids[] = {
		 
		{
			.ident = "ASUS M2A-VM",
			.matches = {
				DMI_MATCH(DMI_BOARD_VENDOR,
					  "ASUSTeK Computer INC."),
				DMI_MATCH(DMI_BOARD_NAME, "M2A-VM"),
			},
			.driver_data = "20071026",	 
		},
		 
		{
			.ident = "MSI K9A2 Platinum",
			.matches = {
				DMI_MATCH(DMI_BOARD_VENDOR,
					  "MICRO-STAR INTER"),
				DMI_MATCH(DMI_BOARD_NAME, "MS-7376"),
			},
		},
		 
		{
			.ident = "MSI K9AGM2",
			.matches = {
				DMI_MATCH(DMI_BOARD_VENDOR,
					  "MICRO-STAR INTER"),
				DMI_MATCH(DMI_BOARD_NAME, "MS-7327"),
			},
		},
		 
		{
			.ident = "ASUS M3A",
			.matches = {
				DMI_MATCH(DMI_BOARD_VENDOR,
					  "ASUSTeK Computer INC."),
				DMI_MATCH(DMI_BOARD_NAME, "M3A"),
			},
		},
		{ }
	};
	const struct dmi_system_id *match;
	int year, month, date;
	char buf[9];

	match = dmi_first_match(sysids);
	if (pdev->bus->number != 0 || pdev->devfn != PCI_DEVFN(0x12, 0) ||
	    !match)
		return false;

	if (!match->driver_data)
		goto enable_64bit;

	dmi_get_date(DMI_BIOS_DATE, &year, &month, &date);
	snprintf(buf, sizeof(buf), "%04d%02d%02d", year, month, date);

	if (strcmp(buf, match->driver_data) >= 0)
		goto enable_64bit;
	else {
		dev_warn(&pdev->dev,
			 "%s: BIOS too old, forcing 32bit DMA, update BIOS\n",
			 match->ident);
		return false;
	}

enable_64bit:
	dev_warn(&pdev->dev, "%s: enabling 64bit DMA\n", match->ident);
	return true;
}

static bool ahci_broken_system_poweroff(struct pci_dev *pdev)
{
	static const struct dmi_system_id broken_systems[] = {
		{
			.ident = "HP Compaq nx6310",
			.matches = {
				DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
				DMI_MATCH(DMI_PRODUCT_NAME, "HP Compaq nx6310"),
			},
			 
			.driver_data = (void *)0x1FUL,
		},
		{
			.ident = "HP Compaq 6720s",
			.matches = {
				DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
				DMI_MATCH(DMI_PRODUCT_NAME, "HP Compaq 6720s"),
			},
			 
			.driver_data = (void *)0x1FUL,
		},

		{ }	 
	};
	const struct dmi_system_id *dmi = dmi_first_match(broken_systems);

	if (dmi) {
		unsigned long slot = (unsigned long)dmi->driver_data;
		 
		return slot == PCI_SLOT(pdev->devfn);
	}

	return false;
}

static bool ahci_broken_suspend(struct pci_dev *pdev)
{
	static const struct dmi_system_id sysids[] = {
		 
		{
			.ident = "dv4",
			.matches = {
				DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
				DMI_MATCH(DMI_PRODUCT_NAME,
					  "HP Pavilion dv4 Notebook PC"),
			},
			.driver_data = "20090105",	 
		},
		{
			.ident = "dv5",
			.matches = {
				DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
				DMI_MATCH(DMI_PRODUCT_NAME,
					  "HP Pavilion dv5 Notebook PC"),
			},
			.driver_data = "20090506",	 
		},
		{
			.ident = "dv6",
			.matches = {
				DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
				DMI_MATCH(DMI_PRODUCT_NAME,
					  "HP Pavilion dv6 Notebook PC"),
			},
			.driver_data = "20090423",	 
		},
		{
			.ident = "HDX18",
			.matches = {
				DMI_MATCH(DMI_SYS_VENDOR, "Hewlett-Packard"),
				DMI_MATCH(DMI_PRODUCT_NAME,
					  "HP HDX18 Notebook PC"),
			},
			.driver_data = "20090430",	 
		},
		 
		{
			.ident = "G725",
			.matches = {
				DMI_MATCH(DMI_SYS_VENDOR, "eMachines"),
				DMI_MATCH(DMI_PRODUCT_NAME, "eMachines G725"),
			},
			.driver_data = "20091216",	 
		},
		{ }	 
	};
	const struct dmi_system_id *dmi = dmi_first_match(sysids);
	int year, month, date;
	char buf[9];

	if (!dmi || pdev->bus->number || pdev->devfn != PCI_DEVFN(0x1f, 2))
		return false;

	dmi_get_date(DMI_BIOS_DATE, &year, &month, &date);
	snprintf(buf, sizeof(buf), "%04d%02d%02d", year, month, date);

	return strcmp(buf, dmi->driver_data) < 0;
}

static bool ahci_broken_online(struct pci_dev *pdev)
{
#define ENCODE_BUSDEVFN(bus, slot, func)			\
	(void *)(unsigned long)(((bus) << 8) | PCI_DEVFN((slot), (func)))
	static const struct dmi_system_id sysids[] = {
		 
		{
			.ident = "EP45-DQ6",
			.matches = {
				DMI_MATCH(DMI_BOARD_VENDOR,
					  "Gigabyte Technology Co., Ltd."),
				DMI_MATCH(DMI_BOARD_NAME, "EP45-DQ6"),
			},
			.driver_data = ENCODE_BUSDEVFN(0x0a, 0x00, 0),
		},
		{
			.ident = "EP45-DS5",
			.matches = {
				DMI_MATCH(DMI_BOARD_VENDOR,
					  "Gigabyte Technology Co., Ltd."),
				DMI_MATCH(DMI_BOARD_NAME, "EP45-DS5"),
			},
			.driver_data = ENCODE_BUSDEVFN(0x03, 0x00, 0),
		},
		{ }	 
	};
#undef ENCODE_BUSDEVFN
	const struct dmi_system_id *dmi = dmi_first_match(sysids);
	unsigned int val;

	if (!dmi)
		return false;

	val = (unsigned long)dmi->driver_data;

	return pdev->bus->number == (val >> 8) && pdev->devfn == (val & 0xff);
}

static bool ahci_broken_devslp(struct pci_dev *pdev)
{
	 
	static const struct pci_device_id ids[] = {
		{ PCI_VDEVICE(INTEL, 0x0f23)},  
		{}
	};

	return pci_match_id(ids, pdev);
}

#ifdef CONFIG_ATA_ACPI
static void ahci_gtf_filter_workaround(struct ata_host *host)
{
	static const struct dmi_system_id sysids[] = {
		 
		{
			.ident = "Aspire 3810T",
			.matches = {
				DMI_MATCH(DMI_SYS_VENDOR, "Acer"),
				DMI_MATCH(DMI_PRODUCT_NAME, "Aspire 3810T"),
			},
			.driver_data = (void *)ATA_ACPI_FILTER_FPDMA_OFFSET,
		},
		{ }
	};
	const struct dmi_system_id *dmi = dmi_first_match(sysids);
	unsigned int filter;
	int i;

	if (!dmi)
		return;

	filter = (unsigned long)dmi->driver_data;
	dev_info(host->dev, "applying extra ACPI _GTF filter 0x%x for %s\n",
		 filter, dmi->ident);

	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap = host->ports[i];
		struct ata_link *link;
		struct ata_device *dev;

		ata_for_each_link(link, ap, EDGE)
			ata_for_each_dev(dev, link, ALL)
				dev->gtf_filter |= filter;
	}
}
#else
static inline void ahci_gtf_filter_workaround(struct ata_host *host)
{}
#endif

static int ahci_init_msix(struct pci_dev *pdev, unsigned int n_ports,
			  struct ahci_host_priv *hpriv)
{
	int rc, nvec;
	struct msix_entry entry = {};

	if (hpriv->flags & AHCI_HFLAG_NO_MSI)
		return -ENODEV;

	nvec = pci_msix_vec_count(pdev);
	if (nvec < 0)
		return nvec;

	if (!nvec) {
		rc = -ENODEV;
		goto fail;
	}

	rc = pci_enable_msix_exact(pdev, &entry, 1);
	if (rc < 0)
		goto fail;

	hpriv->irq = entry.vector;

	return 1;
fail:
	dev_err(&pdev->dev,
		"failed to enable MSI-X with error %d, # of vectors: %d\n",
		rc, nvec);

	return rc;
}

static int ahci_init_msi(struct pci_dev *pdev, unsigned int n_ports,
			struct ahci_host_priv *hpriv)
{
	int rc, nvec;

	if (hpriv->flags & AHCI_HFLAG_NO_MSI)
		return -ENODEV;

	nvec = pci_msi_vec_count(pdev);
	if (nvec < 0)
		return nvec;

	if (nvec < n_ports)
		goto single_msi;

	rc = pci_enable_msi_exact(pdev, nvec);
	if (rc == -ENOSPC)
		goto single_msi;
	if (rc < 0)
		return rc;

	if (readl(hpriv->mmio + HOST_CTL) & HOST_MRSM) {
		pci_disable_msi(pdev);
		printk(KERN_INFO "ahci: MRSM is on, fallback to single MSI\n");
		goto single_msi;
	}

	if (nvec > 1)
		hpriv->flags |= AHCI_HFLAG_MULTI_MSI;

	goto out;

single_msi:
	nvec = 1;

	rc = pci_enable_msi(pdev);
	if (rc < 0)
		return rc;
out:
	hpriv->irq = pdev->irq;

	return nvec;
}

static int ahci_init_interrupts(struct pci_dev *pdev, unsigned int n_ports,
				struct ahci_host_priv *hpriv)
{
	int nvec;

	nvec = ahci_init_msi(pdev, n_ports, hpriv);
	if (nvec >= 0)
		return nvec;

	nvec = ahci_init_msix(pdev, n_ports, hpriv);
	if (nvec >= 0)
		return nvec;

	pci_intx(pdev, 1);
	hpriv->irq = pdev->irq;

	return 0;
}

#ifdef MY_ABC_HERE
 
static int syno_asmedia_1061_check(struct pci_dev* pdev)
{
	return (pdev->vendor == 0x1b21 && pdev->device == 0x0612) ? 0 : -1;
}

static u8 syno_asmedia_1061_reg_get(struct pci_dev *pdev, unsigned int devfn, unsigned int reg_addr)
{
	int check = -1;
	u8 reg_data = 0;

	if (0 != syno_asmedia_1061_check(pdev)) {
		dev_warn(&pdev->dev, "Invalid PCI device !!\n");
		goto END;
	}

	check = pci_bus_read_config_byte(pdev->bus, devfn, reg_addr, &reg_data);
	if (0 != check) {
		dev_warn(&pdev->dev, "pci_bus_read_config_byte error, return = %d\n", check);
		goto END;
	}
END:
	return reg_data;
}

static void syno_asmedia_1061_reg_set(struct pci_dev *pdev, unsigned int devfn, unsigned int reg_addr, u8 reg_data)
{
	int check = -1;

	if (0 != syno_asmedia_1061_check(pdev)) {
		dev_warn(&pdev->dev, "Invalid PCI device !!\n");
		goto END;
	}

	check = pci_bus_write_config_byte(pdev->bus, devfn, reg_addr, reg_data);
	if (0 != check) {
		dev_warn(&pdev->dev, "pci_bus_write_config_byte error, return = %d\n", check);
		goto END;
	}
END:
	return;
}

#define SYNO_ASM_PORT_NUM 2
#define SYNO_ASM_GEN 3

void syno_asmedia_1061_amp_adjust(struct pci_dev *pdev, unsigned int devfn, unsigned int *addr, unsigned int *data, int len, unsigned int gen_offset)
{
	int port = 0;

	for (port = 0; port < len; ++port) {
		syno_asmedia_1061_reg_set(pdev, devfn, addr[port] | gen_offset, data[port]);
		dev_info(&pdev->dev, "Asmedia 1061 port=%d, reg_addr=0x%x, reg_data=0x%x\n", port, addr[port] | gen_offset, syno_asmedia_1061_reg_get(pdev, devfn, addr[port] | gen_offset));
	}
}

void syno_asmedia_1061_gpio2_led_mode(struct pci_dev *pdev, unsigned int devfn)
{
	unsigned int led_mode_addr = 0xA02;
	u8 reg_data = 0;

	reg_data = syno_asmedia_1061_reg_get(pdev, devfn, led_mode_addr);
	reg_data |= (1 << 3);
	syno_asmedia_1061_reg_set(pdev, devfn, led_mode_addr, reg_data);
}

void syno_asmedia_1061_init(struct ata_host *host)
{
	struct pci_dev *pdev = to_pci_dev(host->dev);
	unsigned int devfn = 0;
	unsigned int asmedia_addr[SYNO_ASM_PORT_NUM] = {0xCA0, 0xDA0};
	unsigned int asmedia_gen[SYNO_ASM_GEN] = {0x4, 0x5, 0x6};
	unsigned int asmedia_DS418_data[SYNO_ASM_PORT_NUM] = {0x6f, 0x4f};
	unsigned int asmedia_DS418j_data[SYNO_ASM_PORT_NUM] = {0xaf, 0xaf};
	unsigned int asmedia_DS418play_data_gen3[SYNO_ASM_PORT_NUM] = {0x65, 0x45};
	unsigned int asmedia_DS419p_data_gen1[SYNO_ASM_PORT_NUM] = {0x4e, 0x4e};
	unsigned int asmedia_DS419p_data_gen2[SYNO_ASM_PORT_NUM] = {0x4e, 0x4e};
	unsigned int asmedia_DS419p_data_gen3[SYNO_ASM_PORT_NUM] = {0x4e, 0x4e};
	unsigned int asmedia_DS420p_data_gen1[SYNO_ASM_PORT_NUM] = {0xab, 0xab};
	unsigned int asmedia_DS420p_data_gen2[SYNO_ASM_PORT_NUM] = {0x26, 0x26};
	unsigned int asmedia_DS420p_data_gen3[SYNO_ASM_PORT_NUM] = {0x38, 0x28};
	unsigned int asmedia_DS420j_data[SYNO_ASM_PORT_NUM] = {0xaf, 0xaf};

	if (syno_is_hw_version(HW_DS418)) {
		devfn = PCI_DEVFN(0x00, 0x0);
		syno_asmedia_1061_amp_adjust(pdev, devfn, asmedia_addr, asmedia_DS418_data, SYNO_ASM_PORT_NUM, asmedia_gen[2]);
		syno_asmedia_1061_gpio2_led_mode(pdev, devfn);
	} else if (syno_is_hw_version(HW_DS418j)) {
		devfn = PCI_DEVFN(0x00, 0x0);
		syno_asmedia_1061_amp_adjust(pdev, devfn, asmedia_addr, asmedia_DS418j_data, SYNO_ASM_PORT_NUM, asmedia_gen[2]);
		syno_asmedia_1061_gpio2_led_mode(pdev, devfn);
	} else if (syno_is_hw_version(HW_DS420j)) {
		devfn = PCI_DEVFN(0x00, 0x0);
		syno_asmedia_1061_amp_adjust(pdev, devfn, asmedia_addr, asmedia_DS420j_data, SYNO_ASM_PORT_NUM, asmedia_gen[2]);
		syno_asmedia_1061_gpio2_led_mode(pdev, devfn);
	} else if (syno_is_hw_version(HW_DS418play)) {
		devfn = PCI_DEVFN(0x00, 0x0);
		syno_asmedia_1061_amp_adjust(pdev, devfn, asmedia_addr, asmedia_DS418play_data_gen3, SYNO_ASM_PORT_NUM, asmedia_gen[2]);
		mdelay(100);
		syno_asmedia_1061_reg_set(pdev, devfn, 0xCAE, 0x92);
		mdelay(100);
		syno_asmedia_1061_reg_set(pdev, devfn, 0xDAE, 0x92);
	} else if (syno_is_hw_version(HW_DS419p)) {
		devfn = PCI_DEVFN(0x00, 0x0);
		syno_asmedia_1061_amp_adjust(pdev, devfn, asmedia_addr, asmedia_DS419p_data_gen1, SYNO_ASM_PORT_NUM, asmedia_gen[0]);
		syno_asmedia_1061_amp_adjust(pdev, devfn, asmedia_addr, asmedia_DS419p_data_gen2, SYNO_ASM_PORT_NUM, asmedia_gen[1]);
		syno_asmedia_1061_amp_adjust(pdev, devfn, asmedia_addr, asmedia_DS419p_data_gen3, SYNO_ASM_PORT_NUM, asmedia_gen[2]);
		mdelay(100);
		syno_asmedia_1061_reg_set(pdev, devfn, 0xCAE, 0x92);
		mdelay(100);
		syno_asmedia_1061_reg_set(pdev, devfn, 0xDAE, 0x92);
	} else if (syno_is_hw_version(HW_DS420p)) {
		devfn = PCI_DEVFN(0x00, 0x0);
		syno_asmedia_1061_amp_adjust(pdev, devfn, asmedia_addr, asmedia_DS420p_data_gen1, SYNO_ASM_PORT_NUM, asmedia_gen[0]);
		syno_asmedia_1061_amp_adjust(pdev, devfn, asmedia_addr, asmedia_DS420p_data_gen2, SYNO_ASM_PORT_NUM, asmedia_gen[1]);
		syno_asmedia_1061_amp_adjust(pdev, devfn, asmedia_addr, asmedia_DS420p_data_gen3, SYNO_ASM_PORT_NUM, asmedia_gen[2]);
		mdelay(100);
		syno_asmedia_1061_reg_set(pdev, devfn, 0xCAE, 0x92);
		mdelay(100);
		syno_asmedia_1061_reg_set(pdev, devfn, 0xDAE, 0x92);
	}
}
#endif  

#ifdef CONFIG_SYNO_PORT_MAPPING_V2
extern int syno_compare_dts_ata_port(const struct ata_port *pAtaPort, const struct device_node *pDeviceNode);
extern int syno_compare_dts_pciepath(const struct pci_dev *pdev, const struct device_node *pDeviceNode);
 
bool syno_ahci_compare_ata_devicetree_info(const struct ata_port *ap, const struct device_node *pNode)
{
	int ret = false;
	struct device_node *pAhciNode = NULL;
	if (NULL == ap || NULL == pNode) {
		goto END;
	}

	pAhciNode = of_get_child_by_name(pNode, DT_AHCI);
	if (0 != syno_compare_dts_ata_port(ap, pAhciNode)) {
		goto END;
	}

	if (0 != syno_compare_dts_pciepath(to_pci_dev(ap->dev), pAhciNode)) {
		goto END;
	}
	ret = true;
END:
	if (pAhciNode) {
		of_node_put(pAhciNode);
	}
	return ret;
}
#endif  

static int ahci_init_one(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	unsigned int board_id = ent->driver_data;
	struct ata_port_info pi = ahci_port_info[board_id];
	const struct ata_port_info *ppi[] = { &pi, NULL };
	struct device *dev = &pdev->dev;
	struct ahci_host_priv *hpriv;
	struct ata_host *host;
	int n_ports, i, rc;
	int ahci_pci_bar = AHCI_PCI_BAR_STANDARD;

	VPRINTK("ENTER\n");

#ifdef MY_ABC_HERE
	if ('0' == g_ahci_switch) {
		printk("AHCI is disabled.\n");
		return 0;
	}
#endif  

	WARN_ON((int)ATA_MAX_QUEUE > AHCI_MAX_CMDS);

	ata_print_version_once(&pdev->dev, DRV_VERSION);

	if (pdev->vendor == PCI_VENDOR_ID_MARVELL && !marvell_enable)
		return -ENODEV;

	if (is_mcp89_apple(pdev))
		ahci_mcp89_apple_enable(pdev);

	if (pdev->vendor == PCI_VENDOR_ID_PROMISE)
		dev_info(&pdev->dev,
			 "PDC42819 can only drive SATA devices with this driver\n");

	if (pdev->vendor == PCI_VENDOR_ID_STMICRO && pdev->device == 0xCC06)
		ahci_pci_bar = AHCI_PCI_BAR_STA2X11;
	else if (pdev->vendor == 0x1c44 && pdev->device == 0x8000)
		ahci_pci_bar = AHCI_PCI_BAR_ENMOTUS;
	else if (pdev->vendor == 0x177d && pdev->device == 0xa01c)
		ahci_pci_bar = AHCI_PCI_BAR_CAVIUM;

	rc = pcim_enable_device(pdev);
	if (rc)
		return rc;

	if (pdev->vendor == PCI_VENDOR_ID_INTEL &&
	    (pdev->device == 0x2652 || pdev->device == 0x2653)) {
		u8 map;

		pci_read_config_byte(pdev, ICH_MAP, &map);
		if (map & 0x3) {
			dev_info(&pdev->dev,
				 "controller is in combined mode, can't enable AHCI mode\n");
			return -ENODEV;
		}
	}

	rc = pcim_iomap_regions_request_all(pdev, 1 << ahci_pci_bar, DRV_NAME);
	if (rc == -EBUSY)
		pcim_pin_device(pdev);
	if (rc)
		return rc;

	hpriv = devm_kzalloc(dev, sizeof(*hpriv), GFP_KERNEL);
	if (!hpriv)
		return -ENOMEM;
	hpriv->flags |= (unsigned long)pi.private_data;

	if (board_id == board_ahci_mcp65 &&
	    (pdev->revision == 0xa1 || pdev->revision == 0xa2))
		hpriv->flags |= AHCI_HFLAG_NO_MSI;

	if (board_id == board_ahci_sb700 && pdev->revision >= 0x40)
		hpriv->flags &= ~AHCI_HFLAG_IGN_SERR_INTERNAL;

	if (ahci_sb600_enable_64bit(pdev))
		hpriv->flags &= ~AHCI_HFLAG_32BIT_ONLY;

	hpriv->mmio = pcim_iomap_table(pdev)[ahci_pci_bar];

	if (ahci_broken_devslp(pdev))
		hpriv->flags |= AHCI_HFLAG_NO_DEVSLP;

	ahci_pci_save_initial_config(pdev, hpriv);

	if (hpriv->cap & HOST_CAP_NCQ) {
		pi.flags |= ATA_FLAG_NCQ;
		 
		if (!(hpriv->flags & AHCI_HFLAG_NO_FPDMA_AA))
			pi.flags |= ATA_FLAG_FPDMA_AA;

		pi.flags |= ATA_FLAG_FPDMA_AUX;
	}

	if (hpriv->cap & HOST_CAP_PMP)
		pi.flags |= ATA_FLAG_PMP;

	ahci_set_em_messages(hpriv, &pi);

	if (ahci_broken_system_poweroff(pdev)) {
		pi.flags |= ATA_FLAG_NO_POWEROFF_SPINDOWN;
		dev_info(&pdev->dev,
			"quirky BIOS, skipping spindown on poweroff\n");
	}

	if (ahci_broken_suspend(pdev)) {
		hpriv->flags |= AHCI_HFLAG_NO_SUSPEND;
		dev_warn(&pdev->dev,
			 "BIOS update required for suspend/resume\n");
	}

	if (ahci_broken_online(pdev)) {
		hpriv->flags |= AHCI_HFLAG_SRST_TOUT_IS_OFFLINE;
		dev_info(&pdev->dev,
			 "online status unreliable, applying workaround\n");
	}

#ifdef MY_ABC_HERE
	if (gSynoSataHostCnt < sizeof(gszSataPortMap) && 0 != gszSataPortMap[gSynoSataHostCnt]) {
		n_ports = gszSataPortMap[gSynoSataHostCnt] - '0';
	} else {
#endif  
	n_ports = max(ahci_nr_ports(hpriv->cap), fls(hpriv->port_map));
#ifdef MY_ABC_HERE
	}
#endif  

	host = ata_host_alloc_pinfo(&pdev->dev, ppi, n_ports);
	if (!host)
		return -ENOMEM;
	host->private_data = hpriv;

#ifdef MY_ABC_HERE
	if (pdev->vendor == 0x1b4b && pdev->device == 0x9235) {
		hpriv->flags |= AHCI_HFLAG_YES_MV9235_FIX;

		for (i = 0; i < host->n_ports; i++) {
			struct ata_port *ap = host->ports[i];
			ap->link.uiStsFlags |= SYNO_STATUS_IS_MV9235;
		}
	}
#endif  

	ahci_init_interrupts(pdev, n_ports, hpriv);

	if (!(hpriv->cap & HOST_CAP_SSS) || ahci_ignore_sss)
		host->flags |= ATA_HOST_PARALLEL_SCAN;
	else
		dev_info(&pdev->dev, "SSS flag set, parallel bus scan disabled\n");

	if (pi.flags & ATA_FLAG_EM)
		ahci_reset_em(host);

	for (i = 0; i < host->n_ports; i++) {
		struct ata_port *ap = host->ports[i];

		ata_port_pbar_desc(ap, ahci_pci_bar, -1, "abar");
		ata_port_pbar_desc(ap, ahci_pci_bar,
				   0x100 + ap->port_no * 0x80, "port");

		if (ap->flags & ATA_FLAG_EM)
			ap->em_message_type = hpriv->em_msg_type;

		if (!(hpriv->port_map & (1 << i)))
			ap->ops = &ata_dummy_port_ops;

#ifdef CONFIG_SYNO_PORT_MAPPING_V2
		ap->ops->syno_compare_node_info = syno_ahci_compare_ata_devicetree_info;
#endif  
	}

#ifdef MY_ABC_HERE
	if (pdev->vendor == 0x1b4b) {
		for (i = 0; i < host->n_ports; i++) {
			struct ata_port *ap = host->ports[i];
			printk("Change defer qc mode on external port for compatibility\n");
			ap->ops->qc_defer = &ahci_syno_pmp_3x26_qc_defer;
		}
	}
#endif  

#ifdef MY_DEF_HERE
	if (syno_is_hw_version(HW_DS916p) ||
			syno_is_hw_version(HW_DS918p) ||
			syno_is_hw_version(HW_DS416play)) {
		if (pdev->vendor == 0x1b4b && pdev->device == 0x9215) {
			host->ports[2]->flags &= ~ATA_FLAG_NCQ;
			host->ports[2]->flags &= ~ATA_FLAG_FPDMA_AA;
		}
	}
#endif  
	 
	ahci_p5wdh_workaround(host);

	ahci_gtf_filter_workaround(host);

	rc = ahci_configure_dma_masks(pdev, hpriv->cap & HOST_CAP_64);
	if (rc)
		return rc;

	rc = ahci_pci_reset_controller(host);
	if (rc)
		return rc;

	ahci_pci_init_controller(host);
	ahci_pci_print_info(host);

	pci_set_master(pdev);
#ifdef MY_ABC_HERE
	if (pdev->vendor == 0x1b4b && (pdev->device == 0x9235 || pdev->device == 0x9215)) {
		syno_mv_9235_gpio_active_init(host);
	}
#endif  
#if defined(CONFIG_SYNO_MV_9170_GPIO_CTRL) || defined(MY_ABC_HERE)
	if (pdev->vendor == 0x1b4b && (pdev->device == 0x9235 || pdev->device == 0x9215 || pdev->device == 0x9170)) {
		syno_mv_9xxx_amp_adjust(host, pdev);
	}
#endif  

#ifdef MY_ABC_HERE
	if (0 == syno_asmedia_1061_check(pdev)) {
		syno_asmedia_1061_init(host);
	}
#endif  

#ifdef CONFIG_SYNO_MV_9170_GPIO_CTRL
	if (pdev->vendor == 0x1b4b && pdev->device == 0x9170) {
		syno_mv_9170_gpio_active_init(host);
	}
#endif  

	return ahci_host_activate(host, &ahci_sht);
}

module_pci_driver(ahci_pci_driver);

MODULE_AUTHOR("Jeff Garzik");
MODULE_DESCRIPTION("AHCI SATA low-level driver");
MODULE_LICENSE("GPL");
MODULE_DEVICE_TABLE(pci, ahci_pci_tbl);
MODULE_VERSION(DRV_VERSION);
