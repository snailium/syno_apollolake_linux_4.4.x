#ifndef MY_ABC_HERE
#define MY_ABC_HERE
#endif
 
#ifndef _AHCI_H
#define _AHCI_H

#include <linux/clk.h>
#include <linux/libata.h>
#include <linux/phy/phy.h>
#include <linux/regulator/consumer.h>

#define EM_CTRL_MSG_TYPE              0x000f0000

#define EM_MSG_LED_HBA_PORT           0x0000000f
#define EM_MSG_LED_PMP_SLOT           0x0000ff00
#define EM_MSG_LED_VALUE              0xffff0000
#define EM_MSG_LED_VALUE_ACTIVITY     0x00070000
#define EM_MSG_LED_VALUE_OFF          0xfff80000
#define EM_MSG_LED_VALUE_ON           0x00010000

#ifdef MY_DEF_HERE
#define EM_MSG_LOCATE_LED_MASK        0x00380000
#define EM_MSG_FAULT_LED_MASK         0x01c00000

enum {
	ATA_FLAG_SW_LOCATE      = (1 << 24),  
	ATA_FLAG_SW_FAULT       = (1 << 25),  
};
#endif  

enum {
	AHCI_MAX_PORTS		= 32,
	AHCI_MAX_CLKS		= 5,
	AHCI_MAX_SG		= 168,  
	AHCI_DMA_BOUNDARY	= 0xffffffff,
	AHCI_MAX_CMDS		= 32,
	AHCI_CMD_SZ		= 32,
	AHCI_CMD_SLOT_SZ	= AHCI_MAX_CMDS * AHCI_CMD_SZ,
	AHCI_RX_FIS_SZ		= 256,
	AHCI_CMD_TBL_CDB	= 0x40,
	AHCI_CMD_TBL_HDR_SZ	= 0x80,
	AHCI_CMD_TBL_SZ		= AHCI_CMD_TBL_HDR_SZ + (AHCI_MAX_SG * 16),
	AHCI_CMD_TBL_AR_SZ	= AHCI_CMD_TBL_SZ * AHCI_MAX_CMDS,
	AHCI_PORT_PRIV_DMA_SZ	= AHCI_CMD_SLOT_SZ + AHCI_CMD_TBL_AR_SZ +
				  AHCI_RX_FIS_SZ,
	AHCI_PORT_PRIV_FBS_DMA_SZ	= AHCI_CMD_SLOT_SZ +
					  AHCI_CMD_TBL_AR_SZ +
					  (AHCI_RX_FIS_SZ * 16),
	AHCI_IRQ_ON_SG		= (1 << 31),
	AHCI_CMD_ATAPI		= (1 << 5),
	AHCI_CMD_WRITE		= (1 << 6),
	AHCI_CMD_PREFETCH	= (1 << 7),
	AHCI_CMD_RESET		= (1 << 8),
	AHCI_CMD_CLR_BUSY	= (1 << 10),

	RX_FIS_PIO_SETUP	= 0x20,	 
	RX_FIS_D2H_REG		= 0x40,	 
	RX_FIS_SDB		= 0x58,  
	RX_FIS_UNK		= 0x60,  

	HOST_CAP		= 0x00,  
	HOST_CTL		= 0x04,  
	HOST_IRQ_STAT		= 0x08,  
	HOST_PORTS_IMPL		= 0x0c,  
	HOST_VERSION		= 0x10,  
	HOST_EM_LOC		= 0x1c,  
	HOST_EM_CTL		= 0x20,  
	HOST_CAP2		= 0x24,  

	HOST_RESET		= (1 << 0),   
	HOST_IRQ_EN		= (1 << 1),   
	HOST_MRSM		= (1 << 2),   
	HOST_AHCI_EN		= (1 << 31),  

	HOST_CAP_SXS		= (1 << 5),   
	HOST_CAP_EMS		= (1 << 6),   
	HOST_CAP_CCC		= (1 << 7),   
	HOST_CAP_PART		= (1 << 13),  
	HOST_CAP_SSC		= (1 << 14),  
	HOST_CAP_PIO_MULTI	= (1 << 15),  
	HOST_CAP_FBS		= (1 << 16),  
	HOST_CAP_PMP		= (1 << 17),  
	HOST_CAP_ONLY		= (1 << 18),  
	HOST_CAP_CLO		= (1 << 24),  
	HOST_CAP_LED		= (1 << 25),  
	HOST_CAP_ALPM		= (1 << 26),  
	HOST_CAP_SSS		= (1 << 27),  
	HOST_CAP_MPS		= (1 << 28),  
	HOST_CAP_SNTF		= (1 << 29),  
	HOST_CAP_NCQ		= (1 << 30),  
	HOST_CAP_64		= (1 << 31),  

	HOST_CAP2_BOH		= (1 << 0),   
	HOST_CAP2_NVMHCI	= (1 << 1),   
	HOST_CAP2_APST		= (1 << 2),   
	HOST_CAP2_SDS		= (1 << 3),   
	HOST_CAP2_SADM		= (1 << 4),   
	HOST_CAP2_DESO		= (1 << 5),   

	PORT_LST_ADDR		= 0x00,  
	PORT_LST_ADDR_HI	= 0x04,  
	PORT_FIS_ADDR		= 0x08,  
	PORT_FIS_ADDR_HI	= 0x0c,  
	PORT_IRQ_STAT		= 0x10,  
	PORT_IRQ_MASK		= 0x14,  
	PORT_CMD		= 0x18,  
	PORT_TFDATA		= 0x20,	 
	PORT_SIG		= 0x24,	 
	PORT_CMD_ISSUE		= 0x38,  
	PORT_SCR_STAT		= 0x28,  
	PORT_SCR_CTL		= 0x2c,  
	PORT_SCR_ERR		= 0x30,  
	PORT_SCR_ACT		= 0x34,  
	PORT_SCR_NTF		= 0x3c,  
	PORT_FBS		= 0x40,  
	PORT_DEVSLP		= 0x44,  
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	PORT_INDIRECT_ADDR	= 0x78,  
	PORT_INDIRECT_DATA	= 0x7c,  
#endif  

	PORT_IRQ_COLD_PRES	= (1 << 31),  
	PORT_IRQ_TF_ERR		= (1 << 30),  
	PORT_IRQ_HBUS_ERR	= (1 << 29),  
	PORT_IRQ_HBUS_DATA_ERR	= (1 << 28),  
	PORT_IRQ_IF_ERR		= (1 << 27),  
	PORT_IRQ_IF_NONFATAL	= (1 << 26),  
	PORT_IRQ_OVERFLOW	= (1 << 24),  
	PORT_IRQ_BAD_PMP	= (1 << 23),  

	PORT_IRQ_PHYRDY		= (1 << 22),  
	PORT_IRQ_DEV_ILCK	= (1 << 7),  
	PORT_IRQ_CONNECT	= (1 << 6),  
	PORT_IRQ_SG_DONE	= (1 << 5),  
	PORT_IRQ_UNK_FIS	= (1 << 4),  
	PORT_IRQ_SDB_FIS	= (1 << 3),  
	PORT_IRQ_DMAS_FIS	= (1 << 2),  
	PORT_IRQ_PIOS_FIS	= (1 << 1),  
	PORT_IRQ_D2H_REG_FIS	= (1 << 0),  

	PORT_IRQ_FREEZE		= PORT_IRQ_HBUS_ERR |
				  PORT_IRQ_IF_ERR |
				  PORT_IRQ_CONNECT |
				  PORT_IRQ_PHYRDY |
				  PORT_IRQ_UNK_FIS |
				  PORT_IRQ_BAD_PMP,
	PORT_IRQ_ERROR		= PORT_IRQ_FREEZE |
				  PORT_IRQ_TF_ERR |
				  PORT_IRQ_HBUS_DATA_ERR,
	DEF_PORT_IRQ		= PORT_IRQ_ERROR | PORT_IRQ_SG_DONE |
				  PORT_IRQ_SDB_FIS | PORT_IRQ_DMAS_FIS |
				  PORT_IRQ_PIOS_FIS | PORT_IRQ_D2H_REG_FIS,

	PORT_CMD_ASP		= (1 << 27),  
	PORT_CMD_ALPE		= (1 << 26),  
	PORT_CMD_ATAPI		= (1 << 24),  
	PORT_CMD_FBSCP		= (1 << 22),  
	PORT_CMD_ESP		= (1 << 21),  
	PORT_CMD_HPCP		= (1 << 18),  
	PORT_CMD_PMP		= (1 << 17),  
	PORT_CMD_LIST_ON	= (1 << 15),  
	PORT_CMD_FIS_ON		= (1 << 14),  
	PORT_CMD_FIS_RX		= (1 << 4),  
	PORT_CMD_CLO		= (1 << 3),  
	PORT_CMD_POWER_ON	= (1 << 2),  
	PORT_CMD_SPIN_UP	= (1 << 1),  
	PORT_CMD_START		= (1 << 0),  

	PORT_CMD_ICC_MASK	= (0xf << 28),  
	PORT_CMD_ICC_ACTIVE	= (0x1 << 28),  
	PORT_CMD_ICC_PARTIAL	= (0x2 << 28),  
	PORT_CMD_ICC_SLUMBER	= (0x6 << 28),  

	PORT_FBS_DWE_OFFSET	= 16,  
	PORT_FBS_ADO_OFFSET	= 12,  
	PORT_FBS_DEV_OFFSET	= 8,   
	PORT_FBS_DEV_MASK	= (0xf << PORT_FBS_DEV_OFFSET),   
	PORT_FBS_SDE		= (1 << 2),  
	PORT_FBS_DEC		= (1 << 1),  
	PORT_FBS_EN		= (1 << 0),  

	PORT_DEVSLP_DM_OFFSET	= 25,              
	PORT_DEVSLP_DM_MASK	= (0xf << 25),     
	PORT_DEVSLP_DITO_OFFSET	= 15,              
	PORT_DEVSLP_MDAT_OFFSET	= 10,              
	PORT_DEVSLP_DETO_OFFSET	= 2,               
	PORT_DEVSLP_DSP		= (1 << 1),        
	PORT_DEVSLP_ADSE	= (1 << 0),        

#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	 
	PORT_OOB_INDIRECT_ADDR	= 0x48,		 
	PORT_OOB_COMRESET_U_MASK = 0x3f,	 
	PORT_OOB_COMWAKE_OFFSET	= 12,		 
	PORT_OOB_COMWAKE_MASK	= (0xf << 12),	 

#endif  
	 
#define AHCI_HFLAGS(flags)		.private_data	= (void *)(flags)

	AHCI_HFLAG_NO_NCQ		= (1 << 0),
	AHCI_HFLAG_IGN_IRQ_IF_ERR	= (1 << 1),  
	AHCI_HFLAG_IGN_SERR_INTERNAL	= (1 << 2),  
	AHCI_HFLAG_32BIT_ONLY		= (1 << 3),  
	AHCI_HFLAG_MV_PATA		= (1 << 4),  
	AHCI_HFLAG_NO_MSI		= (1 << 5),  
	AHCI_HFLAG_NO_PMP		= (1 << 6),  
	AHCI_HFLAG_SECT255		= (1 << 8),  
	AHCI_HFLAG_YES_NCQ		= (1 << 9),  
	AHCI_HFLAG_NO_SUSPEND		= (1 << 10),  
	AHCI_HFLAG_SRST_TOUT_IS_OFFLINE	= (1 << 11),  
	AHCI_HFLAG_NO_SNTF		= (1 << 12),  
	AHCI_HFLAG_NO_FPDMA_AA		= (1 << 13),  
	AHCI_HFLAG_YES_FBS		= (1 << 14),  
	AHCI_HFLAG_DELAY_ENGINE		= (1 << 15),  
	AHCI_HFLAG_MULTI_MSI		= (1 << 16),  
	AHCI_HFLAG_NO_DEVSLP		= (1 << 17),  
	AHCI_HFLAG_NO_FBS		= (1 << 18),  
	AHCI_HFLAG_EDGE_IRQ		= (1 << 19),  
#ifdef MY_ABC_HERE
	AHCI_HFLAG_YES_MV9235_FIX   = (1 << 31),
#endif  

	AHCI_FLAG_COMMON		= ATA_FLAG_SATA | ATA_FLAG_PIO_DMA |
					  ATA_FLAG_ACPI_SATA | ATA_FLAG_AN,

	ICH_MAP				= 0x90,  

	EM_MAX_SLOTS			= 8,
	EM_MAX_RETRY			= 5,

	EM_CTL_RST		= (1 << 9),  
	EM_CTL_TM		= (1 << 8),  
	EM_CTL_MR		= (1 << 0),  
	EM_CTL_ALHD		= (1 << 26),  
	EM_CTL_XMT		= (1 << 25),  
	EM_CTL_SMB		= (1 << 24),  
	EM_CTL_SGPIO		= (1 << 19),  
	EM_CTL_SES		= (1 << 18),  
	EM_CTL_SAFTE		= (1 << 17),  
	EM_CTL_LED		= (1 << 16),  

	EM_MSG_TYPE_LED		= (1 << 0),  
	EM_MSG_TYPE_SAFTE	= (1 << 1),  
	EM_MSG_TYPE_SES2	= (1 << 2),  
	EM_MSG_TYPE_SGPIO	= (1 << 3),  
};

struct ahci_cmd_hdr {
	__le32			opts;
	__le32			status;
	__le32			tbl_addr;
	__le32			tbl_addr_hi;
	__le32			reserved[4];
};

struct ahci_sg {
	__le32			addr;
	__le32			addr_hi;
	__le32			reserved;
	__le32			flags_size;
};

struct ahci_em_priv {
	enum sw_activity blink_policy;
	struct timer_list timer;
	unsigned long saved_activity;
	unsigned long activity;
	unsigned long led_state;
#ifdef MY_DEF_HERE
	unsigned long saved_locate;
	unsigned long locate;
	unsigned long saved_fault;
	unsigned long fault;
#endif  
};

struct ahci_port_priv {
	struct ata_link		*active_link;
	struct ahci_cmd_hdr	*cmd_slot;
	dma_addr_t		cmd_slot_dma;
	void			*cmd_tbl;
	dma_addr_t		cmd_tbl_dma;
	void			*rx_fis;
	dma_addr_t		rx_fis_dma;
	 
	unsigned int		ncq_saw_d2h:1;
	unsigned int		ncq_saw_dmas:1;
	unsigned int		ncq_saw_sdb:1;
	atomic_t		intr_status;	 
	spinlock_t		lock;		 
	u32 			intr_mask;	 
	bool			fbs_supported;	 
	bool			fbs_enabled;	 
	int			fbs_last_dev;	 
	 
	struct ahci_em_priv	em_priv[EM_MAX_SLOTS];
	char			*irq_desc;	 
};

struct ahci_host_priv {
	 
	unsigned int		flags;		 
	u32			force_port_map;	 
	u32			mask_port_map;	 

	void __iomem *		mmio;		 
	u32			cap;		 
	u32			cap2;		 
	u32			port_map;	 
	u32			saved_cap;	 
	u32			saved_cap2;	 
	u32			saved_port_map;	 
	u32 			em_loc;  
	u32			em_buf_sz;	 
	u32			em_msg_type;	 
	bool			got_runtime_pm;  
	struct clk		*clks[AHCI_MAX_CLKS];  
	struct regulator	**target_pwrs;	 
	 
	struct phy		**phys;
	unsigned		nports;		 
	void			*plat_data;	 
	unsigned int		irq;		 
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)

	u32 comreset_u;
	 
	u32 comwake;

#endif  
	 
	void			(*start_engine)(struct ata_port *ap);

#if defined(CONFIG_SYNO_LSP_ARMADA_16_12)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
 
#else  
	 
	u32			port_base;	 
	u32			port_offset;	 
	int			a8k_a0_wa;	 
#endif  
#endif  
};

extern int ahci_ignore_sss;

extern struct device_attribute *ahci_shost_attrs[];
extern struct device_attribute *ahci_sdev_attrs[];

#define AHCI_SHT(drv_name)						\
	ATA_NCQ_SHT(drv_name),						\
	.can_queue		= AHCI_MAX_CMDS - 1,			\
	.sg_tablesize		= AHCI_MAX_SG,				\
	.dma_boundary		= AHCI_DMA_BOUNDARY,			\
	.shost_attrs		= ahci_shost_attrs,			\
	.sdev_attrs		= ahci_sdev_attrs

extern struct ata_port_operations ahci_ops;
extern struct ata_port_operations ahci_platform_ops;
extern struct ata_port_operations ahci_pmp_retry_srst_ops;

#ifdef MY_ABC_HERE
extern int sata_syno_ahci_defer_cmd(struct ata_queued_cmd *qc);
#endif  

#ifdef MY_ABC_HERE
extern int ahci_syno_pmp_3x26_qc_defer(struct ata_queued_cmd *qc);
#endif  

#if defined(MY_DEF_HERE) || defined(CONFIG_SYNO_PORT_MAPPING_V2)
extern struct ata_device *ata_scsi_find_dev(struct ata_port *ap,
                                            const struct scsi_device *scsidev);
#endif  

unsigned int ahci_dev_classify(struct ata_port *ap);
void ahci_fill_cmd_slot(struct ahci_port_priv *pp, unsigned int tag,
			u32 opts);
void ahci_save_initial_config(struct device *dev,
			      struct ahci_host_priv *hpriv);
void ahci_init_controller(struct ata_host *host);
int ahci_reset_controller(struct ata_host *host);

int ahci_do_softreset(struct ata_link *link, unsigned int *class,
		      int pmp, unsigned long deadline,
		      int (*check_ready)(struct ata_link *link));

unsigned int ahci_qc_issue(struct ata_queued_cmd *qc);
int ahci_stop_engine(struct ata_port *ap);
void ahci_start_fis_rx(struct ata_port *ap);
void ahci_start_engine(struct ata_port *ap);
int ahci_check_ready(struct ata_link *link);
int ahci_kick_engine(struct ata_port *ap);
int ahci_port_resume(struct ata_port *ap);
void ahci_set_em_messages(struct ahci_host_priv *hpriv,
			  struct ata_port_info *pi);
int ahci_reset_em(struct ata_host *host);
void ahci_print_info(struct ata_host *host, const char *scc_s);
int ahci_host_activate(struct ata_host *host, struct scsi_host_template *sht);
void ahci_error_handler(struct ata_port *ap);

#if defined(MY_ABC_HERE) || defined(CONFIG_SYNO_MV_9170_GPIO_CTRL)
static inline void __iomem *ahci_host_base(struct ata_host *host)
{
	struct ahci_host_priv *hpriv = host->private_data;
	return hpriv->mmio;
}
#endif  

static inline void __iomem *__ahci_port_base(struct ata_host *host,
					     unsigned int port_no)
{
	struct ahci_host_priv *hpriv = host->private_data;
	void __iomem *mmio = hpriv->mmio;

#if defined(CONFIG_SYNO_LSP_ARMADA_16_12)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
 
#else  
	if (hpriv->a8k_a0_wa && hpriv->port_base && hpriv->port_offset)
		return mmio + hpriv->port_base + (port_no * hpriv->port_offset);

#endif  
#endif  
	return mmio + 0x100 + (port_no * 0x80);
}

static inline void __iomem *ahci_port_base(struct ata_port *ap)
{
	return __ahci_port_base(ap->host, ap->port_no);
}

static inline int ahci_nr_ports(u32 cap)
{
	return (cap & 0x1f) + 1;
}

#endif  
