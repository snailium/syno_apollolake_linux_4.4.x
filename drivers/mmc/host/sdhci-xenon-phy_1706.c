#if defined(CONFIG_SYNO_LSP_ARMADA_16_12)
/*
 * PHY support for Xenon SDHC
 *
 * Copyright (C) 2016 Marvell, All Rights Reserved.
 *
 * Author:	Hu Ziji <huziji@marvell.com>
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
 * Date:	2016-8-24
#else  // CONFIG_SYNO_LSP_ARMADA_17_02_02
 * Date:		2016-7-30
#endif // CONFIG_SYNO_LSP_ARMADA_17_02_02
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/bitmap.h>
#include <linux/bitops.h>
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#include <linux/slab.h>
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#include <linux/delay.h>
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#include <linux/irq.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/card.h>
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#include <linux/of_address.h>

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#include "../core/core.h"
#include "../core/sdio_ops.h"
#include "../core/mmc_ops.h"

#include "sdhci.h"
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#include "sdhci-pltfm.h"
#include "sdhci-xenon.h"

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
/* Register base for eMMC PHY 5.0 Version */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#define XENON_EMMC_5_0_PHY_REG_BASE		0x0160
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#define SDHCI_EMMC_5_0_PHY_REG_BASE		0x0160
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
/* Register base for eMMC PHY 5.1 Version */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#define XENON_EMMC_PHY_REG_BASE			0x0170

#define XENON_EMMC_PHY_TIMING_ADJUST		XENON_EMMC_PHY_REG_BASE
#define XENON_EMMC_5_0_PHY_TIMING_ADJUST	XENON_EMMC_5_0_PHY_REG_BASE
#define XENON_TIMING_ADJUST_SLOW_MODE		BIT(29)
#define XENON_TIMING_ADJUST_SDIO_MODE		BIT(28)
#define XENON_OUTPUT_QSN_PHASE_SELECT		BIT(17)
#define XENON_SAMPL_INV_QSP_PHASE_SELECT	BIT(18)
#define XENON_SAMPL_INV_QSP_PHASE_SELECT_SHIFT	18
#define XENON_PHY_INITIALIZAION			BIT(31)
#define XENON_WAIT_CYCLE_BEFORE_USING_MASK	0xF
#define XENON_WAIT_CYCLE_BEFORE_USING_SHIFT	12
#define XENON_FC_SYNC_EN_DURATION_MASK		0xF
#define XENON_FC_SYNC_EN_DURATION_SHIFT		8
#define XENON_FC_SYNC_RST_EN_DURATION_MASK	0xF
#define XENON_FC_SYNC_RST_EN_DURATION_SHIFT	4
#define XENON_FC_SYNC_RST_DURATION_MASK		0xF
#define XENON_FC_SYNC_RST_DURATION_SHIFT	0

#define XENON_EMMC_PHY_FUNC_CONTROL		(XENON_EMMC_PHY_REG_BASE + 0x4)
#define XENON_EMMC_5_0_PHY_FUNC_CONTROL		\
	(XENON_EMMC_5_0_PHY_REG_BASE + 0x4)
#define XENON_ASYNC_DDRMODE_MASK		BIT(23)
#define XENON_ASYNC_DDRMODE_SHIFT		23
#define XENON_CMD_DDR_MODE			BIT(16)
#define XENON_DQ_DDR_MODE_SHIFT			8
#define XENON_DQ_DDR_MODE_MASK			0xFF
#define XENON_DQ_ASYNC_MODE			BIT(4)

#define XENON_EMMC_PHY_PAD_CONTROL		(XENON_EMMC_PHY_REG_BASE + 0x8)
#define XENON_EMMC_5_0_PHY_PAD_CONTROL		\
	(XENON_EMMC_5_0_PHY_REG_BASE + 0x8)
#define XENON_REC_EN_SHIFT			24
#define XENON_REC_EN_MASK			0xF
#define XENON_FC_DQ_RECEN			BIT(24)
#define XENON_FC_CMD_RECEN			BIT(25)
#define XENON_FC_QSP_RECEN			BIT(26)
#define XENON_FC_QSN_RECEN			BIT(27)
#define XENON_OEN_QSN				BIT(28)
#define XENON_AUTO_RECEN_CTRL			BIT(30)
#define XENON_FC_ALL_CMOS_RECEIVER		0xF000

#define XENON_EMMC5_FC_QSP_PD			BIT(18)
#define XENON_EMMC5_FC_QSP_PU			BIT(22)
#define XENON_EMMC5_FC_CMD_PD			BIT(17)
#define XENON_EMMC5_FC_CMD_PU			BIT(21)
#define XENON_EMMC5_FC_DQ_PD			BIT(16)
#define XENON_EMMC5_FC_DQ_PU			BIT(20)

#define XENON_EMMC_PHY_PAD_CONTROL1		(XENON_EMMC_PHY_REG_BASE + 0xC)
#define XENON_EMMC5_1_FC_QSP_PD			BIT(9)
#define XENON_EMMC5_1_FC_QSP_PU			BIT(25)
#define XENON_EMMC5_1_FC_CMD_PD			BIT(8)
#define XENON_EMMC5_1_FC_CMD_PU			BIT(24)
#define XENON_EMMC5_1_FC_DQ_PD			0xFF
#define XENON_EMMC5_1_FC_DQ_PU			(0xFF << 16)

#define XENON_EMMC_PHY_PAD_CONTROL2		(XENON_EMMC_PHY_REG_BASE + 0x10)
#define XENON_EMMC_5_0_PHY_PAD_CONTROL2		\
	(XENON_EMMC_5_0_PHY_REG_BASE + 0xC)
#define XENON_ZNR_MASK				0x1F
#define XENON_ZNR_SHIFT				8
#define XENON_ZPR_MASK				0x1F
/* Preferred ZNR and ZPR value vary between different boards.
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#define SDHCI_EMMC_PHY_REG_BASE			0x0170

#define SDHCI_EMMC_PHY_TIMING_ADJUST		SDHCI_EMMC_PHY_REG_BASE
#define SDHCI_EMMC_5_0_PHY_TIMING_ADJUST	SDHCI_EMMC_5_0_PHY_REG_BASE
#define SDHCI_TIMING_ADJUST_SLOW_MODE		BIT(29)
#define SDHCI_TIMING_ADJUST_SDIO_MODE		BIT(28)
#define SDHCI_OUTPUT_QSN_PHASE_SELECT		BIT(17)
#define SDHCI_SAMPL_INV_QSP_PHASE_SELECT	BIT(18)
#define SDHCI_SAMPL_INV_QSP_PHASE_SELECT_SHIFT	18
#define SDHCI_PHY_INITIALIZAION			BIT(31)
#define SDHCI_WAIT_CYCLE_BEFORE_USING_MASK	0xF
#define SDHCI_WAIT_CYCLE_BEFORE_USING_SHIFT	12
#define SDHCI_FC_SYNC_EN_DURATION_MASK		0xF
#define SDHCI_FC_SYNC_EN_DURATION_SHIFT		8
#define SDHCI_FC_SYNC_RST_EN_DURATION_MASK	0xF
#define SDHCI_FC_SYNC_RST_EN_DURATION_SHIFT	4
#define SDHCI_FC_SYNC_RST_DURATION_MASK		0xF
#define SDHCI_FC_SYNC_RST_DURATION_SHIFT	0

#define SDHCI_EMMC_PHY_FUNC_CONTROL		(SDHCI_EMMC_PHY_REG_BASE + 0x4)
#define SDHCI_EMMC_5_0_PHY_FUNC_CONTROL		\
	(SDHCI_EMMC_5_0_PHY_REG_BASE + 0x4)
#define SDHCI_ASYNC_DDRMODE_MASK		BIT(23)
#define SDHCI_ASYNC_DDRMODE_SHIFT		23
#define SDHCI_CMD_DDR_MODE			BIT(16)
#define SDHCI_DQ_DDR_MODE_SHIFT			8
#define SDHCI_DQ_DDR_MODE_MASK			0xFF
#define SDHCI_DQ_ASYNC_MODE			BIT(4)

#define SDHCI_EMMC_PHY_PAD_CONTROL		(SDHCI_EMMC_PHY_REG_BASE + 0x8)
#define SDHCI_EMMC_5_0_PHY_PAD_CONTROL		\
	(SDHCI_EMMC_5_0_PHY_REG_BASE + 0x8)
#define SDHCI_REC_EN_SHIFT			24
#define SDHCI_REC_EN_MASK			0xF
#define SDHCI_FC_DQ_RECEN			BIT(24)
#define SDHCI_FC_CMD_RECEN			BIT(25)
#define SDHCI_FC_QSP_RECEN			BIT(26)
#define SDHCI_FC_QSN_RECEN			BIT(27)
#define SDHCI_OEN_QSN				BIT(28)
#define SDHCI_AUTO_RECEN_CTRL			BIT(30)
#define SDHCI_FC_ALL_CMOS_RECEIVER		0xF000

#define SDHCI_EMMC5_FC_QSP_PD			BIT(18)
#define SDHCI_EMMC5_FC_QSP_PU			BIT(22)
#define SDHCI_EMMC5_FC_CMD_PD			BIT(17)
#define SDHCI_EMMC5_FC_CMD_PU			BIT(21)
#define SDHCI_EMMC5_FC_DQ_PD			BIT(16)
#define SDHCI_EMMC5_FC_DQ_PU			BIT(20)

#define SDHCI_EMMC_PHY_PAD_CONTROL1		(SDHCI_EMMC_PHY_REG_BASE + 0xC)
#define SDHCI_EMMC5_1_FC_QSP_PD			BIT(9)
#define SDHCI_EMMC5_1_FC_QSP_PU			BIT(25)
#define SDHCI_EMMC5_1_FC_CMD_PD			BIT(8)
#define SDHCI_EMMC5_1_FC_CMD_PU			BIT(24)
#define SDHCI_EMMC5_1_FC_DQ_PD			0xFF
#define SDHCI_EMMC5_1_FC_DQ_PU			(0xFF << 16)

#define SDHCI_EMMC_PHY_PAD_CONTROL2		(SDHCI_EMMC_PHY_REG_BASE + 0x10)
#define SDHCI_EMMC_5_0_PHY_PAD_CONTROL2		\
	(SDHCI_EMMC_5_0_PHY_REG_BASE + 0xC)
#define SDHCI_ZNR_MASK				0x1F
#define SDHCI_ZNR_SHIFT				8
#define SDHCI_ZPR_MASK				0x1F
/* Perferred ZNR and ZPR value vary between different boards.
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
 * The specific ZNR and ZPR value should be defined here
 * according to board actual timing.
 */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#define XENON_ZNR_DEF_VALUE			0xF
#define XENON_ZPR_DEF_VALUE			0xF

#define XENON_EMMC_PHY_DLL_CONTROL		(XENON_EMMC_PHY_REG_BASE + 0x14)
#define XENON_EMMC_5_0_PHY_DLL_CONTROL		\
	(XENON_EMMC_5_0_PHY_REG_BASE + 0x10)
#define XENON_DLL_ENABLE			BIT(31)
#define XENON_DLL_UPDATE_STROBE_5_0		BIT(30)
#define XENON_DLL_REFCLK_SEL			BIT(30)
#define XENON_DLL_UPDATE			BIT(23)
#define XENON_DLL_PHSEL1_SHIFT			24
#define XENON_DLL_PHSEL0_SHIFT			16
#define XENON_DLL_PHASE_MASK			0x3F
#define XENON_DLL_PHASE_90_DEGREE		0x1F
#define XENON_DLL_FAST_LOCK			BIT(5)
#define XENON_DLL_GAIN2X			BIT(3)
#define XENON_DLL_BYPASS_EN			BIT(0)

#define XENON_EMMC_5_0_PHY_LOGIC_TIMING_ADJUST	\
	(XENON_EMMC_5_0_PHY_REG_BASE + 0x14)
#define XENON_EMMC_PHY_LOGIC_TIMING_ADJUST	(XENON_EMMC_PHY_REG_BASE + 0x18)
#define XENON_LOGIC_TIMING_VALUE		0x00AA8977
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#define SDHCI_ZNR_DEF_VALUE			0xF
#define SDHCI_ZPR_DEF_VALUE			0xF

#define SDHCI_EMMC_PHY_DLL_CONTROL		(SDHCI_EMMC_PHY_REG_BASE + 0x14)
#define SDHCI_EMMC_5_0_PHY_DLL_CONTROL		\
	(SDHCI_EMMC_5_0_PHY_REG_BASE + 0x10)
#define SDHCI_DLL_ENABLE			BIT(31)
#define SDHCI_DLL_UPDATE_STROBE_5_0		BIT(30)
#define SDHCI_DLL_REFCLK_SEL			BIT(30)
#define SDHCI_DLL_UPDATE			BIT(23)
#define SDHCI_DLL_PHSEL1_SHIFT			24
#define SDHCI_DLL_PHSEL0_SHIFT			16
#define SDHCI_DLL_PHASE_MASK			0x3F
#define SDHCI_DLL_PHASE_90_DEGREE		0x1F
#define SDHCI_DLL_FAST_LOCK			BIT(5)
#define SDHCI_DLL_GAIN2X			BIT(3)
#define SDHCI_DLL_BYPASS_EN			BIT(0)

#define SDHCI_EMMC_5_0_PHY_LOGIC_TIMING_ADJUST	\
	(SDHCI_EMMC_5_0_PHY_REG_BASE + 0x14)
#define SDHCI_EMMC_PHY_LOGIC_TIMING_ADJUST	(SDHCI_EMMC_PHY_REG_BASE + 0x18)
#define SDHCI_LOGIC_TIMING_VALUE		0x00AA8977

enum soc_pad_ctrl_type {
	SOC_PAD_SD,
	SOC_PAD_FIXED_1_8V,
};
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */

/*
 * List offset of PHY registers and some special register values
 * in eMMC PHY 5.0 or eMMC PHY 5.1
 */
struct xenon_emmc_phy_regs {
	/* Offset of Timing Adjust register */
	u16 timing_adj;
	/* Offset of Func Control register */
	u16 func_ctrl;
	/* Offset of Pad Control register */
	u16 pad_ctrl;
	/* Offset of Pad Control register 2 */
	u16 pad_ctrl2;
	/* Offset of DLL Control register */
	u16 dll_ctrl;
	/* Offset of Logic Timing Adjust register */
	u16 logic_timing_adj;
	/* DLL Update Enable bit */
	u32 dll_update;
};

#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static const char * const phy_types[] = {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	"sdh phy",
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	"emmc 5.0 phy",
	"emmc 5.1 phy"
};

enum phy_type_enum {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	SDH_PHY,
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	EMMC_5_0_PHY,
	EMMC_5_1_PHY,
	NR_PHY_TYPES
};

#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
enum soc_pad_ctrl_type {
	SOC_PAD_SD,
	SOC_PAD_FIXED_1_8V,
};

#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
struct soc_pad_ctrl_table {
	const char *soc;
	void (*set_soc_pad)(struct sdhci_host *host,
			    unsigned char signal_voltage);
};

struct soc_pad_ctrl {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	/* Register address of SoC PHY PAD ctrl */
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	/* Register address of SOC PHY PAD ctrl */
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	void __iomem	*reg;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	/* SoC PHY PAD ctrl type */
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	/* SOC PHY PAD ctrl type */
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	enum soc_pad_ctrl_type pad_type;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	/* SoC specific operation to set SoC PHY PAD */
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	/* SOC specific operation to set SOC PHY PAD */
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	void (*set_soc_pad)(struct sdhci_host *host,
			    unsigned char signal_voltage);
};

static struct xenon_emmc_phy_regs xenon_emmc_5_0_phy_regs = {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	.timing_adj	= XENON_EMMC_5_0_PHY_TIMING_ADJUST,
	.func_ctrl	= XENON_EMMC_5_0_PHY_FUNC_CONTROL,
	.pad_ctrl	= XENON_EMMC_5_0_PHY_PAD_CONTROL,
	.pad_ctrl2	= XENON_EMMC_5_0_PHY_PAD_CONTROL2,
	.dll_ctrl	= XENON_EMMC_5_0_PHY_DLL_CONTROL,
	.logic_timing_adj = XENON_EMMC_5_0_PHY_LOGIC_TIMING_ADJUST,
	.dll_update	= XENON_DLL_UPDATE_STROBE_5_0,
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	.timing_adj	= SDHCI_EMMC_5_0_PHY_TIMING_ADJUST,
	.func_ctrl	= SDHCI_EMMC_5_0_PHY_FUNC_CONTROL,
	.pad_ctrl	= SDHCI_EMMC_5_0_PHY_PAD_CONTROL,
	.pad_ctrl2	= SDHCI_EMMC_5_0_PHY_PAD_CONTROL2,
	.dll_ctrl	= SDHCI_EMMC_5_0_PHY_DLL_CONTROL,
	.logic_timing_adj = SDHCI_EMMC_5_0_PHY_LOGIC_TIMING_ADJUST,
	.dll_update	= SDHCI_DLL_UPDATE_STROBE_5_0,
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
};

static struct xenon_emmc_phy_regs xenon_emmc_5_1_phy_regs = {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	.timing_adj	= XENON_EMMC_PHY_TIMING_ADJUST,
	.func_ctrl	= XENON_EMMC_PHY_FUNC_CONTROL,
	.pad_ctrl	= XENON_EMMC_PHY_PAD_CONTROL,
	.pad_ctrl2	= XENON_EMMC_PHY_PAD_CONTROL2,
	.dll_ctrl	= XENON_EMMC_PHY_DLL_CONTROL,
	.logic_timing_adj = XENON_EMMC_PHY_LOGIC_TIMING_ADJUST,
	.dll_update	= XENON_DLL_UPDATE,
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	.timing_adj	= SDHCI_EMMC_PHY_TIMING_ADJUST,
	.func_ctrl	= SDHCI_EMMC_PHY_FUNC_CONTROL,
	.pad_ctrl	= SDHCI_EMMC_PHY_PAD_CONTROL,
	.pad_ctrl2	= SDHCI_EMMC_PHY_PAD_CONTROL2,
	.dll_ctrl	= SDHCI_EMMC_PHY_DLL_CONTROL,
	.logic_timing_adj = SDHCI_EMMC_PHY_LOGIC_TIMING_ADJUST,
	.dll_update	= SDHCI_DLL_UPDATE,
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
};
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static int xenon_delay_adj_test(struct mmc_card *card);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

/*
 * eMMC PHY configuration and operations
 */
struct emmc_phy_params {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	bool	slow_mode;

#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	u8 znr;
	u8 zpr;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	bool no_dll_tuning;

	/* Set SOC PHY PAD ctrl to fixed 1.8V */
	bool fixed_1_8v_pad_ctrl;

	/* MMC PAD address */
	void __iomem *pad_ctrl_addr;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	/* Nr of consecutive Sampling Points of a Valid Sampling Window */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	/* Number of consecutive Sampling Points of a Valid Sampling Window */
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	u8 nr_tun_times;
	/* Divider for calculating Tuning Step */
	u8 tun_step_divider;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	struct soc_pad_ctrl pad_ctrl;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
};

static void xenon_emmc_phy_strobe_delay_adj(struct sdhci_host *host,
					struct mmc_card *card);
static int xenon_emmc_phy_fix_sampl_delay_adj(struct sdhci_host *host,
					struct mmc_card *card);
static void xenon_emmc_phy_set(struct sdhci_host *host,
					unsigned char timing);
static void xenon_emmc_phy_config_tuning(struct sdhci_host *host);
static void xenon_emmc_soc_pad_ctrl(struct sdhci_host *host,
					unsigned char signal_voltage);

static const struct xenon_phy_ops emmc_phy_ops = {
	.strobe_delay_adj = xenon_emmc_phy_strobe_delay_adj,
	.fix_sampl_delay_adj = xenon_emmc_phy_fix_sampl_delay_adj,
	.phy_set = xenon_emmc_phy_set,
	.config_tuning = xenon_emmc_phy_config_tuning,
	.soc_pad_ctrl = xenon_emmc_soc_pad_ctrl,
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
};

static int alloc_emmc_phy(struct sdhci_xenon_priv *priv)
{
	struct emmc_phy_params *params;

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	params = kzalloc(sizeof(*params), GFP_KERNEL);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	params = kzalloc(sizeof(struct emmc_phy_params), GFP_KERNEL);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (!params)
		return -ENOMEM;

	priv->phy_params = params;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (priv->phy_type == EMMC_5_0_PHY)
		priv->emmc_phy_regs = &xenon_emmc_5_0_phy_regs;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	priv->phy_ops = emmc_phy_ops;
	return 0;
}

static int emmc_phy_parse_param_dt(struct device_node *np,
						struct emmc_phy_params *params)
{
	u32 value;

	if (of_get_property(np, "xenon,phy-no-dll-tuning", NULL))
		params->no_dll_tuning = true;
	else
		params->no_dll_tuning = false;

	if (!of_property_read_u32(np, "xenon,phy-znr", &value))
		params->znr = value & ZNR_MASK;
	else
		params->znr = ZNR_DEF_VALUE;

	if (!of_property_read_u32(np, "xenon,phy-zpr", &value))
		params->zpr = value & ZPR_MASK;
	else
		params->zpr = ZPR_DEF_VALUE;

	if (of_property_read_bool(np, "xenon,fixed-1-8v-pad-ctrl"))
		params->fixed_1_8v_pad_ctrl = true;
	else
		params->fixed_1_8v_pad_ctrl = false;

	params->pad_ctrl_addr = of_iomap(np, 1);
	if (IS_ERR(params->pad_ctrl_addr))
		params->pad_ctrl_addr = 0;

	if (!of_property_read_u32(np, "xenon,phy-nr-tun-times", &value))
		params->nr_tun_times = value & TUN_CONSECUTIVE_TIMES_MASK;
	else
		params->nr_tun_times = TUN_CONSECUTIVE_TIMES;

	if (!of_property_read_u32(np, "xenon,phy-tun-step-divider", &value))
		params->tun_step_divider = value & 0xFF;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	else
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
		priv->emmc_phy_regs = &xenon_emmc_5_1_phy_regs;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		params->tun_step_divider = TUNING_STEP_DIVIDER;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

	return 0;
}
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)

/*
 * eMMC 5.0/5.1 PHY init/re-init.
 * eMMC PHY init should be executed after:
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
 * 1. SDCLK frequency changes.
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
 * 1. SDCLK frequecny changes.
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
 * 2. SDCLK is stopped and re-enabled.
 * 3. config in emmc_phy_regs->timing_adj and emmc_phy_regs->func_ctrl
 * are changed
 */
static int emmc_phy_init(struct sdhci_host *host)
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static int xenon_emmc_phy_init(struct sdhci_host *host)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
{
	u32 reg;
	u32 wait, clock;
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_xenon_priv *priv = sdhci_pltfm_priv(pltfm_host);
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	struct xenon_emmc_phy_regs *phy_regs = priv->emmc_phy_regs;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	struct emmc_phy_params *params = priv->phy_params;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	int timing_adj_reg;

	if (priv->phy_type == EMMC_5_0_PHY)
		timing_adj_reg = EMMC_5_0_PHY_TIMING_ADJUST;
	else
		timing_adj_reg = EMMC_PHY_TIMING_ADJUST;

#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	reg = sdhci_readl(host, phy_regs->timing_adj);
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	reg |= XENON_PHY_INITIALIZAION;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	reg |= SDHCI_PHY_INITIALIZAION;
	if (params->slow_mode)
		reg |= SDHCI_TIMING_ADJUST_SLOW_MODE;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	sdhci_writel(host, reg, phy_regs->timing_adj);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	reg = sdhci_readl(host, timing_adj_reg);
	reg |= PHY_INITIALIZAION;
	sdhci_writel(host, reg, timing_adj_reg);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

	/* Add duration of FC_SYNC_RST */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	wait = ((reg >> XENON_FC_SYNC_RST_DURATION_SHIFT) &
			XENON_FC_SYNC_RST_DURATION_MASK);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	wait = ((reg >> SDHCI_FC_SYNC_RST_DURATION_SHIFT) &
			SDHCI_FC_SYNC_RST_DURATION_MASK);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	wait = ((reg >> FC_SYNC_RST_DURATION_SHIFT) &
			FC_SYNC_RST_DURATION_MASK);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	/* Add interval between FC_SYNC_EN and FC_SYNC_RST */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	wait += ((reg >> XENON_FC_SYNC_RST_EN_DURATION_SHIFT) &
			XENON_FC_SYNC_RST_EN_DURATION_MASK);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	wait += ((reg >> SDHCI_FC_SYNC_RST_EN_DURATION_SHIFT) &
			SDHCI_FC_SYNC_RST_EN_DURATION_MASK);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	wait += ((reg >> FC_SYNC_RST_EN_DURATION_SHIFT) &
			FC_SYNC_RST_EN_DURATION_MASK);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	/* Add duration of asserting FC_SYNC_EN */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	wait += ((reg >> XENON_FC_SYNC_EN_DURATION_SHIFT) &
			XENON_FC_SYNC_EN_DURATION_MASK);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	wait += ((reg >> SDHCI_FC_SYNC_EN_DURATION_SHIFT) &
			SDHCI_FC_SYNC_EN_DURATION_MASK);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	wait += ((reg >> FC_SYNC_EN_DURATION_SHIFT) &
			FC_SYNC_EN_DURATION_MASK);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	/* Add duration of waiting for PHY */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	wait += ((reg >> XENON_WAIT_CYCLE_BEFORE_USING_SHIFT) &
			XENON_WAIT_CYCLE_BEFORE_USING_MASK);
	/* 4 additional bus clock and 4 AXI bus clock are required */
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	wait += ((reg >> SDHCI_WAIT_CYCLE_BEFORE_USING_SHIFT) &
			SDHCI_WAIT_CYCLE_BEFORE_USING_MASK);
	/* 4 addtional bus clock and 4 AXI bus clock are required */
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	wait += ((reg >> WAIT_CYCLE_BEFORE_USING_SHIFT) &
			WAIT_CYCLE_BEFORE_USING_MASK);
	/*
	 * According to Moyang, 4 addtional bus clock
	 * and 4 AXI bus clock are required
	 */
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	wait += 8;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	/* left shift 20 bits */
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	wait <<= 20;

	clock = host->clock;
	if (!clock)
		/* Use the possibly slowest bus frequency value */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		clock = XENON_LOWEST_SDCLK_FREQ;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		clock = SDHCI_LOWEST_SDCLK_FREQ;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		clock = 100000;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	/* get the wait time */
	wait /= clock;
	wait++;
	/* wait for host eMMC PHY init completes */
	udelay(wait);

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	reg = sdhci_readl(host, phy_regs->timing_adj);
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	reg &= XENON_PHY_INITIALIZAION;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	reg &= SDHCI_PHY_INITIALIZAION;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	reg = sdhci_readl(host, timing_adj_reg);
	reg &= PHY_INITIALIZAION;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (reg) {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
		dev_err(mmc_dev(host->mmc), "eMMC PHY init cannot complete after %d us\n",
			wait);
		return -ETIMEDOUT;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		pr_err("%s: eMMC PHY init cannot complete after %d us\n",
			mmc_hostname(host->mmc), wait);
		return -EIO;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	}

	return 0;
}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#define ARMADA_3700_SOC_PAD_1_8V	0x1
#define ARMADA_3700_SOC_PAD_3_3V	0x0
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static inline void soc_pad_voltage_set(void __iomem *pad_ctrl,
					unsigned char signal_voltage)
{
	if (!pad_ctrl)
		return;

	if (signal_voltage == MMC_SIGNAL_VOLTAGE_180)
		writel(SOC_PAD_1_8V, pad_ctrl);
	else if (signal_voltage == MMC_SIGNAL_VOLTAGE_330)
		writel(SOC_PAD_3_3V, pad_ctrl);
}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
static void armada_3700_soc_pad_voltage_set(struct sdhci_host *host,
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static void xenon_emmc_soc_pad_ctrl(struct sdhci_host *host,
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
					unsigned char signal_voltage)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_xenon_priv *priv = sdhci_pltfm_priv(pltfm_host);
	struct emmc_phy_params *params = priv->phy_params;

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (params->pad_ctrl.pad_type == SOC_PAD_FIXED_1_8V) {
		writel(ARMADA_3700_SOC_PAD_1_8V, params->pad_ctrl.reg);
	} else if (params->pad_ctrl.pad_type == SOC_PAD_SD) {
		if (signal_voltage == MMC_SIGNAL_VOLTAGE_180)
			writel(ARMADA_3700_SOC_PAD_1_8V, params->pad_ctrl.reg);
		else if (signal_voltage == MMC_SIGNAL_VOLTAGE_330)
			writel(ARMADA_3700_SOC_PAD_3_3V, params->pad_ctrl.reg);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (params->fixed_1_8v_pad_ctrl)
		soc_pad_voltage_set(params->pad_ctrl_addr,
					MMC_SIGNAL_VOLTAGE_180);
	else
		soc_pad_voltage_set(params->pad_ctrl_addr,
					signal_voltage);
}

static int xenon_emmc_phy_set_fix_sampl_delay(struct sdhci_host *host,
			unsigned int delay, bool invert, bool delay_90_degree)
{
	u32 reg;
	unsigned long flags;
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_xenon_priv *priv = sdhci_pltfm_priv(pltfm_host);
	int timing_adj_reg;
	u32 delay_mask;
	int ret = 0;

	spin_lock_irqsave(&host->lock, flags);

	if (priv->phy_type == EMMC_5_0_PHY) {
		timing_adj_reg = EMMC_5_0_PHY_TIMING_ADJUST;
		delay_mask = EMMC_5_0_PHY_FIXED_DELAY_MASK;
	} else {
		timing_adj_reg = EMMC_PHY_TIMING_ADJUST;
		delay_mask = EMMC_PHY_FIXED_DELAY_MASK;
	}

	/* Setup Sampling fix delay */
	reg = sdhci_readl(host, SDHC_SLOT_OP_STATUS_CTRL);
	reg &= ~delay_mask;
	reg |= delay & delay_mask;
	sdhci_writel(host, reg, SDHC_SLOT_OP_STATUS_CTRL);

	if (priv->phy_type == EMMC_5_0_PHY) {
		/* set 90 degree phase if necessary */
		reg &= ~DELAY_90_DEGREE_MASK_EMMC5;
		reg |= (delay_90_degree << DELAY_90_DEGREE_SHIFT_EMMC5);
		sdhci_writel(host, reg, SDHC_SLOT_OP_STATUS_CTRL);
	}

	/* Disable SDCLK */
	reg = sdhci_readl(host, SDHCI_CLOCK_CONTROL);
	reg &= ~(SDHCI_CLOCK_CARD_EN | SDHCI_CLOCK_INT_EN);
	sdhci_writel(host, reg, SDHCI_CLOCK_CONTROL);

	udelay(200);

	if (priv->phy_type == EMMC_5_1_PHY) {
		/* set 90 degree phase if necessary */
		reg = sdhci_readl(host, EMMC_PHY_FUNC_CONTROL);
		reg &= ~ASYNC_DDRMODE_MASK;
		reg |= (delay_90_degree << ASYNC_DDRMODE_SHIFT);
		sdhci_writel(host, reg, EMMC_PHY_FUNC_CONTROL);
	}

	/* Setup Inversion of Sampling edge */
	reg = sdhci_readl(host, timing_adj_reg);
	reg &= ~SAMPL_INV_QSP_PHASE_SELECT;
	reg |= (invert << SAMPL_INV_QSP_PHASE_SELECT_SHIFT);
	sdhci_writel(host, reg, timing_adj_reg);

	/* Enable SD internal clock */
	ret = enable_xenon_internal_clk(host);
	if (ret)
		goto out;

	/* Enable SDCLK */
	reg = sdhci_readl(host, SDHCI_CLOCK_CONTROL);
	reg |= SDHCI_CLOCK_CARD_EN;
	sdhci_writel(host, reg, SDHCI_CLOCK_CONTROL);

	udelay(200);

	/*
	 * Has to re-initialize eMMC PHY here to active PHY
	 * because later get status cmd will be issued.
	 */
	ret = xenon_emmc_phy_init(host);

out:
	spin_unlock_irqrestore(&host->lock, flags);
	return ret;
}

static int xenon_emmc_phy_do_fix_sampl_delay(struct sdhci_host *host,
			struct mmc_card *card, unsigned int delay,
			bool invert, bool quarter)
{
	int ret;

	xenon_emmc_phy_set_fix_sampl_delay(host, delay, invert, quarter);

	ret = xenon_delay_adj_test(card);
	if (ret) {
		pr_debug("Xenon fail when sampling fix delay = %d, phase = %d degree\n",
				delay, invert * 180 + quarter * 90);
		return -1;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	}
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	return 0;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
/*
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
 * Set SoC PHY voltage PAD control register,
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
 * Set SOC PHY voltage PAD control register,
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
 * according to the operation voltage on PAD.
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
 * The detailed operation depends on SoC implementation.
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
 * The detailed operation depends on SOC implementaion.
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
 */
static void emmc_phy_set_soc_pad(struct sdhci_host *host,
				 unsigned char signal_voltage)
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static int xenon_emmc_phy_fix_sampl_delay_adj(struct sdhci_host *host,
					struct mmc_card *card)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
{
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	enum sampl_fix_delay_phase phase;
	int idx, nr_pair;
	int ret;
	unsigned int delay;
	unsigned int min_delay, max_delay;
	bool invert, quarter;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_xenon_priv *priv = sdhci_pltfm_priv(pltfm_host);
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	struct emmc_phy_params *params = priv->phy_params;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	u32 delay_mask, coarse_step, fine_step;
	/*
	 * Pairs to set the delay edge
	 * First column is the inversion sequence.
	 * Second column indicates delay 90 degree or not
	 */
	const enum sampl_fix_delay_phase delay_edge[] = {
		PHASE_0_DEGREE,
		PHASE_180_DEGREE,
		PHASE_90_DEGREE,
		PHASE_270_DEGREE
	};

	if (priv->phy_type == EMMC_5_0_PHY)
		delay_mask = EMMC_5_0_PHY_FIXED_DELAY_MASK;
	else
		delay_mask = EMMC_PHY_FIXED_DELAY_MASK;
	coarse_step = delay_mask >> 1;
	fine_step = coarse_step >> 2;

	nr_pair = ARRAY_SIZE(delay_edge);

	for (idx = 0; idx < nr_pair; idx++) {
		phase = delay_edge[idx];
		invert = (phase & 0x2) ? true : false;
		quarter = (phase & 0x1) ? true : false;

		/* increase dly value to get fix delay */
		for (min_delay = 0; min_delay <= delay_mask;
				min_delay += coarse_step) {
			ret = xenon_emmc_phy_do_fix_sampl_delay(host, card,
					min_delay, invert, quarter);
			if (!ret)
				break;
		}

		if (ret) {
			pr_debug("Fail to set Sampling Fixed Delay with phase = %d degree\n",
					phase * 90);
			continue;
		}

		for (max_delay = min_delay + fine_step;
			max_delay < delay_mask;
			max_delay += fine_step) {
			ret = xenon_emmc_phy_do_fix_sampl_delay(host, card,
					max_delay, invert, quarter);
			if (ret) {
				max_delay -= fine_step;
				break;
			}
		}

		if (!ret) {
			ret = xenon_emmc_phy_do_fix_sampl_delay(host, card,
					delay_mask, invert, quarter);
			if (!ret)
				max_delay = delay_mask;
		}

		/*
		 * Sampling Fixed Delay line window shoul be larger enough,
		 * thus the sampling point (the middle of the window)
		 * can work when environment varies.
		 * However, there is no clear conclusoin how large the window
		 * should be.
		 */
		if ((max_delay - min_delay) <=
				EMMC_PHY_FIXED_DELAY_WINDOW_MIN) {
			pr_info("The window size %d when phase = %d degree cannot meet timing requiremnt\n",
				max_delay - min_delay, phase * 90);
			continue;
		}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (!params->pad_ctrl.reg)
		return;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		delay = (min_delay + max_delay) / 2;
		xenon_emmc_phy_set_fix_sampl_delay(host, delay, invert,
					quarter);
		pr_debug("Xenon sampling fix delay = %d with phase = %d degree\n",
				delay, phase * 90);
		return 0;
	}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (params->pad_ctrl.set_soc_pad)
		params->pad_ctrl.set_soc_pad(host, signal_voltage);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	return -EIO;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
/*
 * Enable eMMC PHY HW DLL
 * DLL should be enabled and stable before HS200/SDR104 tuning,
 * and before HS400 data strobe setting.
 */
static int emmc_phy_enable_dll(struct sdhci_host *host)
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static int xenon_emmc_phy_enable_dll(struct sdhci_host *host)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
{
	u32 reg;
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_xenon_priv *priv = sdhci_pltfm_priv(pltfm_host);
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	struct xenon_emmc_phy_regs *phy_regs = priv->emmc_phy_regs;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	struct emmc_phy_params *params = priv->phy_params;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	u8 timeout;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	int dll_ctrl;
	u32 dll_update;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	if (params->slow_mode && (host->clock <= MMC_HIGH_52_MAX_DTR))
		return 0;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	WARN_ON(host->clock <= MMC_HIGH_52_MAX_DTR);
	if (host->clock <= MMC_HIGH_52_MAX_DTR)
		return -EINVAL;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (WARN_ON(host->clock <= MMC_HIGH_52_MAX_DTR))
		return -EINVAL;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (priv->phy_type == EMMC_5_0_PHY) {
		dll_ctrl = EMMC_5_0_PHY_DLL_CONTROL;
		dll_update = DLL_UPDATE_STROBE_5_0;
	} else {
		dll_ctrl = EMMC_PHY_DLL_CONTROL;
		dll_update = DLL_UPDATE;
	}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	reg = sdhci_readl(host, phy_regs->dll_ctrl);
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	if (reg & XENON_DLL_ENABLE)
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	if (reg & SDHCI_DLL_ENABLE)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	reg = sdhci_readl(host, dll_ctrl);
	if (reg & DLL_ENABLE)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		return 0;

	/* Enable DLL */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	reg = sdhci_readl(host, phy_regs->dll_ctrl);
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	reg |= (XENON_DLL_ENABLE | XENON_DLL_FAST_LOCK);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	reg |= (SDHCI_DLL_ENABLE | SDHCI_DLL_FAST_LOCK);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	reg = sdhci_readl(host, dll_ctrl);
	reg |= (DLL_ENABLE | DLL_FAST_LOCK);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

	/*
	 * Set Phase as 90 degree, which is most common value.
	 * Might set another value if necessary.
	 * The granularity is 1 degree.
	 */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	reg &= ~((XENON_DLL_PHASE_MASK << XENON_DLL_PHSEL0_SHIFT) |
		 (XENON_DLL_PHASE_MASK << XENON_DLL_PHSEL1_SHIFT));
	reg |= ((XENON_DLL_PHASE_90_DEGREE << XENON_DLL_PHSEL0_SHIFT) |
		(XENON_DLL_PHASE_90_DEGREE << XENON_DLL_PHSEL1_SHIFT));
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	reg &= ~((SDHCI_DLL_PHASE_MASK << SDHCI_DLL_PHSEL0_SHIFT) |
		 (SDHCI_DLL_PHASE_MASK << SDHCI_DLL_PHSEL1_SHIFT));
	reg |= ((SDHCI_DLL_PHASE_90_DEGREE << SDHCI_DLL_PHSEL0_SHIFT) |
		(SDHCI_DLL_PHASE_90_DEGREE << SDHCI_DLL_PHSEL1_SHIFT));
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	reg &= ~((DLL_PHASE_MASK << DLL_PHSEL0_SHIFT) |
			(DLL_PHASE_MASK << DLL_PHSEL1_SHIFT));
	reg |= ((DLL_PHASE_90_DEGREE << DLL_PHSEL0_SHIFT) |
			(DLL_PHASE_90_DEGREE << DLL_PHSEL1_SHIFT));
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	reg &= ~XENON_DLL_BYPASS_EN;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	reg &= ~SDHCI_DLL_BYPASS_EN;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	reg |= phy_regs->dll_update;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	reg &= ~DLL_BYPASS_EN;
	reg |= dll_update;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (priv->phy_type == EMMC_5_1_PHY)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		reg &= ~XENON_DLL_REFCLK_SEL;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		reg &= ~SDHCI_DLL_REFCLK_SEL;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	sdhci_writel(host, reg, phy_regs->dll_ctrl);

	/* Wait max 32 ms */
	timeout = 32;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	while (!(sdhci_readw(host, XENON_SLOT_EXT_PRESENT_STATE) &
		XENON_DLL_LOCK_STATE)) {
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	while (!(sdhci_readw(host, SDHCI_SLOT_EXT_PRESENT_STATE) &
		SDHCI_DLL_LOCK_STATE)) {
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		if (!timeout) {
			dev_err(mmc_dev(host->mmc), "Wait for DLL Lock time-out\n");
			return -ETIMEDOUT;
		}
		timeout--;
		mdelay(1);
	}
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		reg &= ~DLL_REFCLK_SEL;
	sdhci_writel(host, reg, dll_ctrl);

	/* Wait max 5 ms */
	mdelay(5);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	return 0;
}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
/*
 * Config to eMMC PHY to prepare for tuning.
 * Enable HW DLL and set the TUNING_STEP
 */
static int emmc_phy_config_tuning(struct sdhci_host *host)
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static void xenon_emmc_phy_config_tuning(struct sdhci_host *host)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_xenon_priv *priv = sdhci_pltfm_priv(pltfm_host);
	struct emmc_phy_params *params = priv->phy_params;
	u32 reg, tuning_step;
	int ret;
	unsigned long flags;

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	if (host->clock <= MMC_HIGH_52_MAX_DTR)
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	if (params->slow_mode && (host->clock <= MMC_HIGH_52_MAX_DTR))
		return 0;

	if (WARN_ON(host->clock <= MMC_HIGH_52_MAX_DTR))
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		return -EINVAL;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	WARN_ON(host->clock <= MMC_HIGH_52_MAX_DTR);
	if (host->clock <= MMC_HIGH_52_MAX_DTR)
		return;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

	spin_lock_irqsave(&host->lock, flags);

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	ret = emmc_phy_enable_dll(host);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	ret = xenon_emmc_phy_enable_dll(host);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (ret) {
		spin_unlock_irqrestore(&host->lock, flags);
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
		return ret;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		return;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	/* Achieve TUNING_STEP with HW DLL help */
	reg = sdhci_readl(host, XENON_SLOT_DLL_CUR_DLY_VAL);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	/* Achieve TUNGING_STEP with HW DLL help */
	reg = sdhci_readl(host, SDHCI_SLOT_DLL_CUR_DLY_VAL);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	reg = sdhci_readl(host, SDHC_SLOT_DLL_CUR_DLY_VAL);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	tuning_step = reg / params->tun_step_divider;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	if (unlikely(tuning_step > XENON_TUNING_STEP_MASK)) {
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	if (unlikely(tuning_step > SDHCI_TUNING_STEP_MASK)) {
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		dev_warn(mmc_dev(host->mmc),
			 "HS200 TUNING_STEP %d is larger than MAX value\n",
			 tuning_step);
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		tuning_step = XENON_TUNING_STEP_MASK;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		tuning_step = SDHCI_TUNING_STEP_MASK;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	}

	/* Set TUNING_STEP for later tuning */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	reg = sdhci_readl(host, XENON_SLOT_OP_STATUS_CTRL);
	reg &= ~(XENON_TUN_CONSECUTIVE_TIMES_MASK <<
		 XENON_TUN_CONSECUTIVE_TIMES_SHIFT);
	reg |= (params->nr_tun_times << XENON_TUN_CONSECUTIVE_TIMES_SHIFT);
	reg &= ~(XENON_TUNING_STEP_MASK << XENON_TUNING_STEP_SHIFT);
	reg |= (tuning_step << XENON_TUNING_STEP_SHIFT);
	sdhci_writel(host, reg, XENON_SLOT_OP_STATUS_CTRL);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	reg = sdhci_readl(host, SDHCI_SLOT_OP_STATUS_CTRL);
	reg &= ~(SDHCI_TUN_CONSECUTIVE_TIMES_MASK <<
		 SDHCI_TUN_CONSECUTIVE_TIMES_SHIFT);
	reg |= (params->nr_tun_times << SDHCI_TUN_CONSECUTIVE_TIMES_SHIFT);
	reg &= ~(SDHCI_TUNING_STEP_MASK << SDHCI_TUNING_STEP_SHIFT);
	reg |= (tuning_step << SDHCI_TUNING_STEP_SHIFT);
	sdhci_writel(host, reg, SDHCI_SLOT_OP_STATUS_CTRL);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (unlikely(tuning_step > TUNING_STEP_MASK)) {
		WARN("%s: HS200 TUNING_STEP %d is larger than MAX value\n",
					mmc_hostname(host->mmc), tuning_step);
		tuning_step = TUNING_STEP_MASK;
	}

	reg = sdhci_readl(host, SDHC_SLOT_OP_STATUS_CTRL);
	reg &= ~(TUN_CONSECUTIVE_TIMES_MASK << TUN_CONSECUTIVE_TIMES_SHIFT);
	reg |= (params->nr_tun_times << TUN_CONSECUTIVE_TIMES_SHIFT);
	reg &= ~(TUNING_STEP_MASK << TUNING_STEP_SHIFT);
	reg |= (tuning_step << TUNING_STEP_SHIFT);
	sdhci_writel(host, reg, SDHC_SLOT_OP_STATUS_CTRL);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

	spin_unlock_irqrestore(&host->lock, flags);
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	return 0;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
static void __emmc_phy_disable_data_strobe(struct sdhci_host *host)
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static void xenon_emmc_phy_strobe_delay_adj(struct sdhci_host *host,
					struct mmc_card *card)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
{
	u32 reg;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)

	/* Disable SDHC Data Strobe */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	reg = sdhci_readl(host, XENON_SLOT_EMMC_CTRL);
	reg &= ~XENON_ENABLE_DATA_STROBE;
	sdhci_writel(host, reg, XENON_SLOT_EMMC_CTRL);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	reg = sdhci_readl(host, SDHCI_SLOT_EMMC_CTRL);
	reg &= ~SDHCI_ENABLE_DATA_STROBE;
	sdhci_writel(host, reg, SDHCI_SLOT_EMMC_CTRL);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
}

/* Set HS400 Data Strobe */
static void emmc_phy_strobe_delay_adj(struct sdhci_host *host)
{
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_xenon_priv *priv = sdhci_pltfm_priv(pltfm_host);
	unsigned long flags;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	u32 reg;

	if (WARN_ON(host->timing != MMC_TIMING_MMC_HS400))
		return;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

	if (host->clock <= MMC_HIGH_52_MAX_DTR)
		return;

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	dev_dbg(mmc_dev(host->mmc), "starts HS400 strobe delay adjustment\n");
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	pr_debug("%s: starts HS400 strobe delay adjustment\n",
				mmc_hostname(host->mmc));
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

	spin_lock_irqsave(&host->lock, flags);

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	emmc_phy_enable_dll(host);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	xenon_emmc_phy_enable_dll(host);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

	/* Enable SDHC Data Strobe */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	reg = sdhci_readl(host, XENON_SLOT_EMMC_CTRL);
	reg |= XENON_ENABLE_DATA_STROBE;
	sdhci_writel(host, reg, XENON_SLOT_EMMC_CTRL);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	reg = sdhci_readl(host, SDHCI_SLOT_EMMC_CTRL);
	reg |= SDHCI_ENABLE_DATA_STROBE;
	sdhci_writel(host, reg, SDHCI_SLOT_EMMC_CTRL);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	reg = sdhci_readl(host, SDHC_SLOT_eMMC_CTRL);
	reg |= ENABLE_DATA_STROBE;
	sdhci_writel(host, reg, SDHC_SLOT_eMMC_CTRL);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

	/* Set Data Strobe Pull down */
	if (priv->phy_type == EMMC_5_0_PHY) {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		reg = sdhci_readl(host, XENON_EMMC_5_0_PHY_PAD_CONTROL);
		reg |= XENON_EMMC5_FC_QSP_PD;
		reg &= ~XENON_EMMC5_FC_QSP_PU;
		sdhci_writel(host, reg, XENON_EMMC_5_0_PHY_PAD_CONTROL);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		reg = sdhci_readl(host, SDHCI_EMMC_5_0_PHY_PAD_CONTROL);
		reg |= SDHCI_EMMC5_FC_QSP_PD;
		reg &= ~SDHCI_EMMC5_FC_QSP_PU;
		sdhci_writel(host, reg, SDHCI_EMMC_5_0_PHY_PAD_CONTROL);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		reg = sdhci_readl(host, EMMC_5_0_PHY_PAD_CONTROL);
		reg |= EMMC5_FC_QSP_PD;
		reg &= ~EMMC5_FC_QSP_PU;
		sdhci_writel(host, reg, EMMC_5_0_PHY_PAD_CONTROL);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	} else {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		reg = sdhci_readl(host, XENON_EMMC_PHY_PAD_CONTROL1);
		reg |= XENON_EMMC5_1_FC_QSP_PD;
		reg &= ~XENON_EMMC5_1_FC_QSP_PU;
		sdhci_writel(host, reg, XENON_EMMC_PHY_PAD_CONTROL1);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		reg = sdhci_readl(host, SDHCI_EMMC_PHY_PAD_CONTROL1);
		reg |= SDHCI_EMMC5_1_FC_QSP_PD;
		reg &= ~SDHCI_EMMC5_1_FC_QSP_PU;
		sdhci_writel(host, reg, SDHCI_EMMC_PHY_PAD_CONTROL1);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		reg = sdhci_readl(host, EMMC_PHY_PAD_CONTROL1);
		reg |= EMMC5_1_FC_QSP_PD;
		reg &= ~EMMC5_1_FC_QSP_PU;
		sdhci_writel(host, reg, EMMC_PHY_PAD_CONTROL1);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	}
	spin_unlock_irqrestore(&host->lock, flags);
}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
static inline bool temp_stage_hs200_to_hs400(struct sdhci_host *host,
					     struct sdhci_xenon_priv *priv)
{
	/*
	 * Tmep stages from HS200 to HS400
	 * from HS200 to HS in 200MHz
	 * from 200MHz to 52MHz
	 */
	if (((priv->timing == MMC_TIMING_MMC_HS200) &&
	     (host->timing == MMC_TIMING_MMC_HS)) ||
	    ((host->timing == MMC_TIMING_MMC_HS) &&
	     (priv->clock > host->clock)))
		return true;

	return false;
}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#define LOGIC_TIMING_VALUE	0x00aa8977
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
static inline bool temp_stage_hs400_to_h200(struct sdhci_host *host,
					    struct sdhci_xenon_priv *priv)
{
	/*
	 * Temp stages from HS400 t0 HS200:
	 * from 200MHz to 52MHz in HS400
	 * from HS400 to HS DDR in 52MHz
	 * from HS DDR to HS in 52MHz
	 * from HS to HS200 in 52MHz
	 */
	if (((priv->timing == MMC_TIMING_MMC_HS400) &&
	     ((host->clock == MMC_HIGH_52_MAX_DTR) ||
	      (host->timing == MMC_TIMING_MMC_DDR52))) ||
	    ((priv->timing == MMC_TIMING_MMC_DDR52) &&
	     (host->timing == MMC_TIMING_MMC_HS)) ||
	    ((host->timing == MMC_TIMING_MMC_HS200) &&
	     (host->clock == MMC_HIGH_52_MAX_DTR)))
		return true;

	return false;
}

#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
/*
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
 * If eMMC PHY Slow Mode is required in lower speed mode (SDCLK < 55MHz)
 * in SDR mode, enable Slow Mode to bypass eMMC PHY.
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
 * If eMMC PHY Slow Mode is required in lower speed mode in SDR mode
 * (SDLCK < 55MHz), enable Slow Mode to bypass eMMC PHY.
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
 * SDIO slower SDR mode also requires Slow Mode.
 *
 * If Slow Mode is enabled, return true.
 * Otherwise, return false.
 */
static bool emmc_phy_slow_mode(struct sdhci_host *host,
			       unsigned char timing)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_xenon_priv *priv = sdhci_pltfm_priv(pltfm_host);
	struct emmc_phy_params *params = priv->phy_params;
	struct xenon_emmc_phy_regs *phy_regs = priv->emmc_phy_regs;
	u32 reg;

#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	if (host->clock > MMC_HIGH_52_MAX_DTR)
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	/* Skip temp stages from HS200 to HS400 */
	if (temp_stage_hs200_to_hs400(host, priv))
		return false;

	/* Skip temp stages from HS400 t0 HS200 */
	if (temp_stage_hs400_to_h200(host, priv))
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		return false;

	reg = sdhci_readl(host, phy_regs->timing_adj);
	/* Enable Slow Mode for SDIO in slower SDR mode */
	if ((priv->init_card_type == MMC_TYPE_SDIO) &&
	    ((timing == MMC_TIMING_UHS_SDR25) ||
	     (timing == MMC_TIMING_UHS_SDR12) ||
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	     (timing == MMC_TIMING_SD_HS))) {
		reg |= XENON_TIMING_ADJUST_SLOW_MODE;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	     (timing == MMC_TIMING_SD_HS) ||
	     (timing == MMC_TIMING_LEGACY))) {
		reg |= SDHCI_TIMING_ADJUST_SLOW_MODE;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		sdhci_writel(host, reg, phy_regs->timing_adj);
		return true;
	}

	/* Check if Slow Mode is required in lower speed mode in SDR mode */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	if (((timing == MMC_TIMING_UHS_SDR25) ||
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	if (((timing == MMC_TIMING_UHS_SDR50) ||
	     (timing == MMC_TIMING_UHS_SDR25) ||
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	     (timing == MMC_TIMING_UHS_SDR12) ||
	     (timing == MMC_TIMING_SD_HS) ||
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	     (timing == MMC_TIMING_MMC_HS)) && params->slow_mode) {
		reg |= XENON_TIMING_ADJUST_SLOW_MODE;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	     (timing == MMC_TIMING_MMC_HS) ||
	     (timing == MMC_TIMING_LEGACY)) && params->slow_mode) {
		reg |= SDHCI_TIMING_ADJUST_SLOW_MODE;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		sdhci_writel(host, reg, phy_regs->timing_adj);
		return true;
	}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	reg &= ~XENON_TIMING_ADJUST_SLOW_MODE;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	reg &= ~SDHCI_TIMING_ADJUST_SLOW_MODE;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	sdhci_writel(host, reg, phy_regs->timing_adj);
	return false;
}

/*
 * Set-up eMMC 5.0/5.1 PHY.
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
 * Specific configuration depends on the current speed mode in use.
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
 * Specific onfiguration depends on the current speed mode in use.
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
 */
static void emmc_phy_set(struct sdhci_host *host,
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static void xenon_emmc_phy_set(struct sdhci_host *host,
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
					unsigned char timing)
{
	u32 reg;
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_xenon_priv *priv = sdhci_pltfm_priv(pltfm_host);
	struct emmc_phy_params *params = priv->phy_params;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	struct xenon_emmc_phy_regs *phy_regs = priv->emmc_phy_regs;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	struct mmc_card *card = priv->card_candidate;
	int pad_ctrl, timing_adj, pad_ctrl2, func_ctrl, logic_timing_adj;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	unsigned long flags;

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	dev_dbg(mmc_dev(host->mmc), "eMMC PHY setting starts\n");
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	pr_debug("%s: eMMC PHY setting starts\n", mmc_hostname(host->mmc));

	if (priv->phy_type == EMMC_5_0_PHY) {
		pad_ctrl = EMMC_5_0_PHY_PAD_CONTROL;
		timing_adj = EMMC_5_0_PHY_TIMING_ADJUST;
		pad_ctrl2 = EMMC_5_0_PHY_PAD_CONTROL2;
		func_ctrl = EMMC_5_0_PHY_FUNC_CONTRL;
		logic_timing_adj = EMMC_5_0_PHY_LOGIC_TIMING_ADJUST;
	} else {
		pad_ctrl = EMMC_PHY_PAD_CONTROL;
		timing_adj = EMMC_PHY_TIMING_ADJUST;
		pad_ctrl2 = EMMC_PHY_PAD_CONTROL2;
		func_ctrl = EMMC_PHY_FUNC_CONTROL;
		logic_timing_adj = EMMC_PHY_LOGIC_TIMING_ADJUST;
	}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

	spin_lock_irqsave(&host->lock, flags);

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	reg = sdhci_readl(host, SDHCI_SYS_EXT_OP_CTRL);
	reg |= SDHCI_MASK_CMD_CONFLICT_ERROR;
	sdhci_writel(host, reg, SDHCI_SYS_EXT_OP_CTRL);

#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	/* Setup pad, set bit[28] and bits[26:24] */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	reg = sdhci_readl(host, phy_regs->pad_ctrl);
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	reg |= (XENON_FC_DQ_RECEN | XENON_FC_CMD_RECEN |
		XENON_FC_QSP_RECEN | XENON_OEN_QSN);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	reg |= (SDHCI_FC_DQ_RECEN | SDHCI_FC_CMD_RECEN |
		SDHCI_FC_QSP_RECEN | SDHCI_OEN_QSN);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	/* All FC_XX_RECEIVCE should be set as CMOS Type */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	reg |= XENON_FC_ALL_CMOS_RECEIVER;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	reg |= SDHCI_FC_ALL_CMOS_RECEIVER;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	sdhci_writel(host, reg, phy_regs->pad_ctrl);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	reg = sdhci_readl(host, pad_ctrl);
	reg |= (FC_DQ_RECEN | FC_CMD_RECEN | FC_QSP_RECEN | OEN_QSN);
	/*
	 * All FC_XX_RECEIVCE should be set as CMOS Type
	 */
	reg |= FC_ALL_CMOS_RECEIVER;
	sdhci_writel(host, reg, pad_ctrl);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

	/* Set CMD and DQ Pull Up */
	if (priv->phy_type == EMMC_5_0_PHY) {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		reg = sdhci_readl(host, XENON_EMMC_5_0_PHY_PAD_CONTROL);
		reg |= (XENON_EMMC5_FC_CMD_PU | XENON_EMMC5_FC_DQ_PU);
		reg &= ~(XENON_EMMC5_FC_CMD_PD | XENON_EMMC5_FC_DQ_PD);
		sdhci_writel(host, reg, XENON_EMMC_5_0_PHY_PAD_CONTROL);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		reg = sdhci_readl(host, SDHCI_EMMC_5_0_PHY_PAD_CONTROL);
		reg |= (SDHCI_EMMC5_FC_CMD_PU | SDHCI_EMMC5_FC_DQ_PU);
		reg &= ~(SDHCI_EMMC5_FC_CMD_PD | SDHCI_EMMC5_FC_DQ_PD);
		sdhci_writel(host, reg, SDHCI_EMMC_5_0_PHY_PAD_CONTROL);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		reg = sdhci_readl(host, EMMC_5_0_PHY_PAD_CONTROL);
		reg |= (EMMC5_FC_CMD_PU | EMMC5_FC_DQ_PU);
		reg &= ~(EMMC5_FC_CMD_PD | EMMC5_FC_DQ_PD);
		sdhci_writel(host, reg, EMMC_5_0_PHY_PAD_CONTROL);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	} else {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		reg = sdhci_readl(host, XENON_EMMC_PHY_PAD_CONTROL1);
		reg |= (XENON_EMMC5_1_FC_CMD_PU | XENON_EMMC5_1_FC_DQ_PU);
		reg &= ~(XENON_EMMC5_1_FC_CMD_PD | XENON_EMMC5_1_FC_DQ_PD);
		sdhci_writel(host, reg, XENON_EMMC_PHY_PAD_CONTROL1);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		reg = sdhci_readl(host, SDHCI_EMMC_PHY_PAD_CONTROL1);
		reg |= (SDHCI_EMMC5_1_FC_CMD_PU | SDHCI_EMMC5_1_FC_DQ_PU);
		reg &= ~(SDHCI_EMMC5_1_FC_CMD_PD | SDHCI_EMMC5_1_FC_DQ_PD);
		sdhci_writel(host, reg, SDHCI_EMMC_PHY_PAD_CONTROL1);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		reg = sdhci_readl(host, EMMC_PHY_PAD_CONTROL1);
		reg |= (EMMC5_1_FC_CMD_PU | EMMC5_1_FC_DQ_PU);
		reg &= ~(EMMC5_1_FC_CMD_PD | EMMC5_1_FC_DQ_PD);
		sdhci_writel(host, reg, EMMC_PHY_PAD_CONTROL1);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	if (timing == MMC_TIMING_LEGACY) {
		/*
		 * If Slow Mode is required, enable Slow Mode by default
		 * in early init phase to avoid any potential issue.
		 */
		if (params->slow_mode) {
			reg = sdhci_readl(host, phy_regs->timing_adj);
			reg |= XENON_TIMING_ADJUST_SLOW_MODE;
			sdhci_writel(host, reg, phy_regs->timing_adj);
		}
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	if (timing == MMC_TIMING_LEGACY)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if ((timing == MMC_TIMING_LEGACY) || !card)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		goto phy_init;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */

	/*
	 * FIXME: should depends on the specific board timing.
	 */
	if ((timing == MMC_TIMING_MMC_HS400) ||
		(timing == MMC_TIMING_MMC_HS200) ||
		(timing == MMC_TIMING_UHS_SDR50) ||
		(timing == MMC_TIMING_UHS_SDR104) ||
		(timing == MMC_TIMING_UHS_DDR50) ||
		(timing == MMC_TIMING_UHS_SDR25) ||
		(timing == MMC_TIMING_MMC_DDR52)) {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
		reg = sdhci_readl(host, phy_regs->timing_adj);
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		reg &= ~XENON_OUTPUT_QSN_PHASE_SELECT;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		reg &= ~SDHCI_OUTPUT_QSN_PHASE_SELECT;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		sdhci_writel(host, reg, phy_regs->timing_adj);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		reg = sdhci_readl(host, timing_adj);
		reg &= ~OUTPUT_QSN_PHASE_SELECT;
		sdhci_writel(host, reg, timing_adj);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	}

	/*
	 * If SDIO card, set SDIO Mode
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	 * Otherwise, clear SDIO Mode
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	 * Otherwise, clear SDIO Mode and Slow Mode
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	 */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	reg = sdhci_readl(host, phy_regs->timing_adj);
	if (priv->init_card_type == MMC_TYPE_SDIO)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		reg |= XENON_TIMING_ADJUST_SDIO_MODE;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		reg |= SDHCI_TIMING_ADJUST_SDIO_MODE;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	else
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		reg &= ~XENON_TIMING_ADJUST_SDIO_MODE;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		reg &= ~SDHCI_TIMING_ADJUST_SDIO_MODE;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	sdhci_writel(host, reg, phy_regs->timing_adj);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (mmc_card_sdio(card)) {
		reg = sdhci_readl(host, timing_adj);
		reg |= TIMING_ADJUST_SDIO_MODE;

		if ((timing == MMC_TIMING_UHS_SDR25) ||
			(timing == MMC_TIMING_UHS_SDR12) ||
			(timing == MMC_TIMING_SD_HS) ||
			(timing == MMC_TIMING_LEGACY))
			reg |= TIMING_ADJUST_SLOW_MODE;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (emmc_phy_slow_mode(host, timing))
		goto phy_init;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		sdhci_writel(host, reg, timing_adj);
	} else {
		reg = sdhci_readl(host, timing_adj);
		reg &= ~(TIMING_ADJUST_SDIO_MODE | TIMING_ADJUST_SLOW_MODE);
		sdhci_writel(host, reg, timing_adj);
	}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

	/*
	 * Set preferred ZNR and ZPR value
	 * The ZNR and ZPR value vary between different boards.
	 * Define them both in sdhci-xenon-emmc-phy.h.
	 */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	reg = sdhci_readl(host, phy_regs->pad_ctrl2);
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	reg &= ~((XENON_ZNR_MASK << XENON_ZNR_SHIFT) | XENON_ZPR_MASK);
	reg |= ((params->znr << XENON_ZNR_SHIFT) | params->zpr);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	reg &= ~((SDHCI_ZNR_MASK << SDHCI_ZNR_SHIFT) | SDHCI_ZPR_MASK);
	reg |= ((params->znr << SDHCI_ZNR_SHIFT) | params->zpr);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	sdhci_writel(host, reg, phy_regs->pad_ctrl2);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	reg = sdhci_readl(host, pad_ctrl2);
	reg &= ~((ZNR_MASK << ZNR_SHIFT) | ZPR_MASK);
	reg |= ((params->znr << ZNR_SHIFT) | params->zpr);
	sdhci_writel(host, reg, pad_ctrl2);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

	/*
	 * When setting EMMC_PHY_FUNC_CONTROL register,
	 * SD clock should be disabled
	 */
	reg = sdhci_readl(host, SDHCI_CLOCK_CONTROL);
	reg &= ~SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, reg, SDHCI_CLOCK_CONTROL);

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	reg = sdhci_readl(host, phy_regs->func_ctrl);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if ((timing == MMC_TIMING_UHS_DDR50) ||
		(timing == MMC_TIMING_MMC_HS400) ||
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	    (timing == MMC_TIMING_MMC_DDR52))
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		reg |= (XENON_DQ_DDR_MODE_MASK << XENON_DQ_DDR_MODE_SHIFT) |
		       XENON_CMD_DDR_MODE;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		reg |= (SDHCI_DQ_DDR_MODE_MASK << SDHCI_DQ_DDR_MODE_SHIFT) |
		       SDHCI_CMD_DDR_MODE;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	else
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		reg &= ~((XENON_DQ_DDR_MODE_MASK << XENON_DQ_DDR_MODE_SHIFT) |
			 XENON_CMD_DDR_MODE);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		reg &= ~((SDHCI_DQ_DDR_MODE_MASK << SDHCI_DQ_DDR_MODE_SHIFT) |
			 SDHCI_CMD_DDR_MODE);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		(timing == MMC_TIMING_MMC_DDR52)) {
		reg = sdhci_readl(host, func_ctrl);
		reg |= (DQ_DDR_MODE_MASK << DQ_DDR_MODE_SHIFT) | CMD_DDR_MODE;
		sdhci_writel(host, reg, func_ctrl);
	}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (timing == MMC_TIMING_MMC_HS400)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		reg &= ~XENON_DQ_ASYNC_MODE;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		reg &= ~SDHCI_DQ_ASYNC_MODE;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	else
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		reg |= XENON_DQ_ASYNC_MODE;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		reg |= SDHCI_DQ_ASYNC_MODE;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	sdhci_writel(host, reg, phy_regs->func_ctrl);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (timing == MMC_TIMING_MMC_HS400) {
		reg = sdhci_readl(host, func_ctrl);
		reg &= ~DQ_ASYNC_MODE;
		sdhci_writel(host, reg, func_ctrl);
	}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

	/* Enable bus clock */
	reg = sdhci_readl(host, SDHCI_CLOCK_CONTROL);
	reg |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, reg, SDHCI_CLOCK_CONTROL);

	if (timing == MMC_TIMING_MMC_HS400)
		/* Hardware team recommend a value for HS400 */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		sdhci_writel(host, XENON_LOGIC_TIMING_VALUE,
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		sdhci_writel(host, SDHCI_LOGIC_TIMING_VALUE,
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
			     phy_regs->logic_timing_adj);
	else
		__emmc_phy_disable_data_strobe(host);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		sdhci_writel(host, LOGIC_TIMING_VALUE, logic_timing_adj);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

phy_init:
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	emmc_phy_init(host);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	xenon_emmc_phy_init(host);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

	spin_unlock_irqrestore(&host->lock, flags);

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	dev_dbg(mmc_dev(host->mmc), "eMMC PHY setting completes\n");
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	pr_debug("%s: eMMC PHY setting completes\n", mmc_hostname(host->mmc));
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
static int get_dt_pad_ctrl_data(struct sdhci_host *host,
				struct device_node *np,
				struct emmc_phy_params *params)
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
/*
 * SDH PHY configuration and operations
 */
static int xenon_sdh_phy_set_fix_sampl_delay(struct sdhci_host *host,
					unsigned int delay, bool invert)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
{
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	int ret = 0;
	const char *name;
	struct resource iomem;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	u32 reg;
	unsigned long flags;
	int ret;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (of_device_is_compatible(np, "marvell,armada-3700-sdhci"))
		params->pad_ctrl.set_soc_pad = armada_3700_soc_pad_voltage_set;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (invert)
		invert = 0x1;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	else
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
// do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		invert = 0x0;

	spin_lock_irqsave(&host->lock, flags);

	/* Disable SDCLK */
	reg = sdhci_readl(host, SDHCI_CLOCK_CONTROL);
	reg &= ~(SDHCI_CLOCK_CARD_EN | SDHCI_CLOCK_INT_EN);
	sdhci_writel(host, reg, SDHCI_CLOCK_CONTROL);

	udelay(200);

	/* Setup Sampling fix delay */
	reg = sdhci_readl(host, SDHC_SLOT_OP_STATUS_CTRL);
	reg &= ~(SDH_PHY_FIXED_DELAY_MASK |
			(0x1 << FORCE_SEL_INVERSE_CLK_SHIFT));
	reg |= ((delay & SDH_PHY_FIXED_DELAY_MASK) |
			(invert << FORCE_SEL_INVERSE_CLK_SHIFT));
	sdhci_writel(host, reg, SDHC_SLOT_OP_STATUS_CTRL);

	/* Enable SD internal clock */
	ret = enable_xenon_internal_clk(host);

	/* Enable SDCLK */
	reg = sdhci_readl(host, SDHCI_CLOCK_CONTROL);
	reg |= SDHCI_CLOCK_CARD_EN;
	sdhci_writel(host, reg, SDHCI_CLOCK_CONTROL);

	udelay(200);

	spin_unlock_irqrestore(&host->lock, flags);
	return ret;
}

static int xenon_sdh_phy_do_fix_sampl_delay(struct sdhci_host *host,
		struct mmc_card *card, unsigned int delay, bool invert)
{
	int ret;

	xenon_sdh_phy_set_fix_sampl_delay(host, delay, invert);

	ret = xenon_delay_adj_test(card);
	if (ret) {
		pr_debug("Xenon fail when sampling fix delay = %d, phase = %d degree\n",
				delay, invert * 180);
		return -1;
	}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	return 0;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
// do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
}

#define SDH_PHY_COARSE_FIX_DELAY		(SDH_PHY_FIXED_DELAY_MASK / 2)
#define SDH_PHY_FINE_FIX_DELAY			(SDH_PHY_COARSE_FIX_DELAY / 4)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (of_address_to_resource(np, 1, &iomem)) {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		dev_err(mmc_dev(host->mmc), "Unable to find SoC PAD ctrl register address for %s\n",
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		dev_err(mmc_dev(host->mmc), "Unable to find SOC PAD ctrl register address for %s\n",
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
			np->name);
		return -EINVAL;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static int xenon_sdh_phy_fix_sampl_delay_adj(struct sdhci_host *host,
					struct mmc_card *card)
{
	u32 reg;
	bool dll_enable = false;
	unsigned int min_delay, max_delay, delay;
	const bool sampl_edge[] = {
		false,
		true,
	};
	int i, nr;
	int ret;

	if (host->clock > HIGH_SPEED_MAX_DTR) {
		/* Enable DLL when SDCLK is higher than 50MHz */
		reg = sdhci_readl(host, SDH_PHY_SLOT_DLL_CTRL);
		if (!(reg & SDH_PHY_ENABLE_DLL)) {
			reg |= (SDH_PHY_ENABLE_DLL | SDH_PHY_FAST_LOCK_EN);
			sdhci_writel(host, reg, SDH_PHY_SLOT_DLL_CTRL);
			mdelay(1);

			reg = sdhci_readl(host, SDH_PHY_SLOT_DLL_PHASE_SEL);
			reg |= SDH_PHY_DLL_UPDATE_TUNING;
			sdhci_writel(host, reg, SDH_PHY_SLOT_DLL_PHASE_SEL);
		}
		dll_enable = true;
	}

	nr = dll_enable ? ARRAY_SIZE(sampl_edge) : 1;
	for (i = 0; i < nr; i++) {
		for (min_delay = 0; min_delay <= SDH_PHY_FIXED_DELAY_MASK;
				min_delay += SDH_PHY_COARSE_FIX_DELAY) {
			ret = xenon_sdh_phy_do_fix_sampl_delay(host, card,
						min_delay, sampl_edge[i]);
			if (!ret)
				break;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	params->pad_ctrl.reg = devm_ioremap_resource(mmc_dev(host->mmc),
						     &iomem);
	if (IS_ERR(params->pad_ctrl.reg)) {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		dev_err(mmc_dev(host->mmc), "Unable to get SoC PHY PAD ctrl register for %s\n",
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		dev_err(mmc_dev(host->mmc), "Unable to get SOC PHY PAD ctrl regiser for %s\n",
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
			np->name);
		return PTR_ERR(params->pad_ctrl.reg);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		if (ret) {
			pr_debug("Fail to set Fixed Sampling Delay with %s edge\n",
				sampl_edge[i] ? "negative" : "positive");
			continue;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	ret = of_property_read_string(np, "marvell,pad-type", &name);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		for (max_delay = min_delay + SDH_PHY_FINE_FIX_DELAY;
				max_delay < SDH_PHY_FIXED_DELAY_MASK;
				max_delay += SDH_PHY_FINE_FIX_DELAY) {
			ret = xenon_sdh_phy_do_fix_sampl_delay(host, card,
						max_delay, sampl_edge[i]);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
			if (ret) {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		dev_err(mmc_dev(host->mmc), "Unable to determine SoC PHY PAD ctrl type\n");
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		dev_err(mmc_dev(host->mmc), "Unable to determine SOC PHY PAD ctrl type\n");
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		return ret;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
				max_delay -= SDH_PHY_FINE_FIX_DELAY;
				break;
			}
		}

		if (!ret) {
			ret = xenon_sdh_phy_do_fix_sampl_delay(host, card,
				SDH_PHY_FIXED_DELAY_MASK, sampl_edge[i]);
			if (!ret)
				max_delay = SDH_PHY_FIXED_DELAY_MASK;
		}

		if ((max_delay - min_delay) <= SDH_PHY_FIXED_DELAY_WINDOW_MIN) {
			pr_info("The window size %d when %s edge cannot meet timing requiremnt\n",
				max_delay - min_delay,
				sampl_edge[i] ? "negative" : "positive");
			continue;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		}
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (!strcmp(name, "sd")) {
		params->pad_ctrl.pad_type = SOC_PAD_SD;
	} else if (!strcmp(name, "fixed-1-8v")) {
		params->pad_ctrl.pad_type = SOC_PAD_FIXED_1_8V;
	} else {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		dev_err(mmc_dev(host->mmc), "Unsupported SoC PHY PAD ctrl type %s\n",
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		dev_err(mmc_dev(host->mmc), "Unsupported SOC PHY PAD ctrl type %s\n",
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
			name);
		return -EINVAL;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

		delay = (min_delay + max_delay) / 2;
		xenon_sdh_phy_set_fix_sampl_delay(host, delay, sampl_edge[i]);
		pr_debug("Xenon sampling fix delay = %d with %s edge\n",
			delay, sampl_edge[i] ? "negative" : "positive");
		return 0;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	}
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	return -EIO;
}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	return ret;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static const struct xenon_phy_ops sdh_phy_ops = {
	.fix_sampl_delay_adj = xenon_sdh_phy_fix_sampl_delay_adj,
};

static int alloc_sdh_phy(struct sdhci_xenon_priv *priv)
{
	priv->phy_params = NULL;
	priv->phy_ops = sdh_phy_ops;
	return 0;
}

/*
 * Common functions for all PHYs
 */
void xenon_soc_pad_ctrl(struct sdhci_host *host,
			unsigned char signal_voltage)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_xenon_priv *priv = sdhci_pltfm_priv(pltfm_host);

	if (priv->phy_ops.soc_pad_ctrl)
		priv->phy_ops.soc_pad_ctrl(host, signal_voltage);
}

static int __xenon_emmc_delay_adj_test(struct mmc_card *card)
{
	int err;
	u8 *ext_csd = NULL;

	err = mmc_get_ext_csd(card, &ext_csd);
	kfree(ext_csd);

	return err;
}

static int __xenon_sdio_delay_adj_test(struct mmc_card *card)
{
	u8 reg;

	return mmc_io_rw_direct(card, 0, 0, 0, 0, &reg);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
static int emmc_phy_parse_param_dt(struct sdhci_host *host,
				   struct device_node *np,
				   struct emmc_phy_params *params)
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static int __xenon_sd_delay_adj_test(struct mmc_card *card)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
{
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	u32 value;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	return mmc_send_status(card, NULL);
}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (of_property_read_bool(np, "marvell,xenon-phy-slow-mode"))
		params->slow_mode = true;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static int xenon_delay_adj_test(struct mmc_card *card)
{
	if (mmc_card_mmc(card))
		return __xenon_emmc_delay_adj_test(card);
	else if (mmc_card_sd(card))
		return __xenon_sd_delay_adj_test(card);
	else if (mmc_card_sdio(card))
		return __xenon_sdio_delay_adj_test(card);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	else
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
		params->slow_mode = false;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		return -EINVAL;
}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (!of_property_read_u32(np, "marvell,xenon-phy-znr", &value))
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		params->znr = value & XENON_ZNR_MASK;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		params->znr = value & SDHCI_ZNR_MASK;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	else
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		params->znr = XENON_ZNR_DEF_VALUE;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		params->znr = SDHCI_ZNR_DEF_VALUE;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static void xenon_phy_set(struct sdhci_host *host,
		struct sdhci_xenon_priv *priv, unsigned char timing)
{
	if (priv->phy_ops.phy_set)
		priv->phy_ops.phy_set(host, timing);
}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (!of_property_read_u32(np, "marvell,xenon-phy-zpr", &value))
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		params->zpr = value & XENON_ZPR_MASK;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		params->zpr = value & SDHCI_ZPR_MASK;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	else
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		params->zpr = XENON_ZPR_DEF_VALUE;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		params->zpr = SDHCI_ZPR_DEF_VALUE;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static void xenon_hs400_strobe_delay_adj(struct sdhci_host *host,
					struct mmc_card *card,
					struct sdhci_xenon_priv *priv)
{
	WARN_ON(!mmc_card_hs400(card));
	if (!mmc_card_hs400(card))
		return;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (!of_property_read_u32(np, "marvell,xenon-phy-nr-success-tun",
				  &value))
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		params->nr_tun_times = value & XENON_TUN_CONSECUTIVE_TIMES_MASK;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		params->nr_tun_times = value & SDHCI_TUN_CONSECUTIVE_TIMES_MASK;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	else
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		params->nr_tun_times = XENON_TUN_CONSECUTIVE_TIMES;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		params->nr_tun_times = SDHCI_TUN_CONSECUTIVE_TIMES;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	/* Enable the DLL to automatically adjust HS400 strobe delay.
	 */
	if (priv->phy_ops.strobe_delay_adj)
		priv->phy_ops.strobe_delay_adj(host, card);
}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (!of_property_read_u32(np, "marvell,xenon-phy-tun-step-divider",
				  &value))
		params->tun_step_divider = value & 0xFF;
	else
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		params->tun_step_divider = XENON_TUNING_STEP_DIVIDER;
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		params->tun_step_divider = SDHCI_TUNING_STEP_DIVIDER;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static int xenon_fix_sampl_delay_adj(struct sdhci_host *host,
				struct mmc_card *card,
				struct sdhci_xenon_priv *priv)
{
	if (priv->phy_ops.fix_sampl_delay_adj)
		return priv->phy_ops.fix_sampl_delay_adj(host, card);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	return get_dt_pad_ctrl_data(host, np, params);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	return 0;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
/* Set SoC PHY Voltage PAD */
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
/* Set SOC PHY Voltage PAD */
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
void xenon_soc_pad_ctrl(struct sdhci_host *host,
			unsigned char signal_voltage)
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static void xenon_phy_config_tuning(struct sdhci_host *host,
				struct sdhci_xenon_priv *priv)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
{
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	emmc_phy_set_soc_pad(host, signal_voltage);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (priv->phy_ops.config_tuning)
		return priv->phy_ops.config_tuning(host);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
}

/*
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
 * Setting PHY when card is working in High Speed Mode.
 * HS400 set data strobe line.
 * HS200/SDR104 set tuning config to prepare for tuning.
#else  // CONFIG_SYNO_LSP_ARMADA_17_02_02
 * xenon_delay_adj should not be called inside IRQ context,
 * either Hard IRQ or Softirq.
#endif // CONFIG_SYNO_LSP_ARMADA_17_02_02
 */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
static int xenon_hs_delay_adj(struct sdhci_host *host)
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static int xenon_hs_delay_adj(struct mmc_host *mmc, struct mmc_card *card,
			struct sdhci_xenon_priv *priv)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
{
	int ret = 0;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	struct sdhci_host *host = mmc_priv(mmc);
	struct emmc_phy_params *params = priv->phy_params;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	if (WARN_ON(host->clock <= XENON_DEFAULT_SDCLK_FREQ))
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	if (WARN_ON(host->clock <= SDHCI_DEFAULT_SDCLK_FREQ))
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	WARN_ON(host->clock <= DEFAULT_SDCLK_FREQ);
	if (host->clock <= DEFAULT_SDCLK_FREQ)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		return -EINVAL;

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (host->timing == MMC_TIMING_MMC_HS400) {
		emmc_phy_strobe_delay_adj(host);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (mmc_card_hs400(card)) {
		xenon_hs400_strobe_delay_adj(host, card, priv);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		return 0;
	}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if ((host->timing == MMC_TIMING_MMC_HS200) ||
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	    (host->timing == MMC_TIMING_UHS_SDR104))
		return emmc_phy_config_tuning(host);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	    (host->timing == MMC_TIMING_UHS_SDR104)) {
		ret = emmc_phy_config_tuning(host);
		if (!ret)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (!params->no_dll_tuning && ((priv->phy_type == EMMC_5_1_PHY) ||
		(priv->phy_type == EMMC_5_0_PHY)) &&
		(mmc_card_hs200(card) ||
		(host->timing == MMC_TIMING_UHS_SDR104))) {
		xenon_phy_config_tuning(host, priv);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
// do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		return 0;
	}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	/*
	 * DDR Mode requires driver to scan Sampling Fixed Delay Line,
	 * to find out a perfect operation sampling point.
	 * It is hard to implement such a scan in host driver since initiating
	 * commands by host driver is not safe.
	 * Thus so far just keep PHY Sampling Fixed Delay in default value
	 * in DDR mode.
	 *
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	 * If any timing issue occurs in DDR mode on Marvell products,
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	 * If any timing issue occrus in DDR mode on Marvell products,
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	 * please contact maintainer to ask for internal support in Marvell.
	 */
	if ((host->timing == MMC_TIMING_MMC_DDR52) ||
	    (host->timing == MMC_TIMING_UHS_DDR50))
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
		dev_warn_once(mmc_dev(host->mmc), "Timing issue might occur in DDR mode\n");
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		dev_warn(mmc_dev(host->mmc), "Timing issue might occur in DDR mode\n");
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	ret = xenon_fix_sampl_delay_adj(host, card, priv);
	if (ret)
		pr_err("%s: fails sampling fixed delay adjustment\n",
			mmc_hostname(mmc));
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	return ret;
}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
/*
 * Adjust PHY setting.
 * PHY setting should be adjusted when SDCLK frequency, Bus Width
 * or Speed Mode is changed.
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
 * Additional config are required when card is working in High Speed mode,
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
 * Addtional config are required when card is working in High Speed mode,
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
 * after leaving Legacy Mode.
 */
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
int xenon_phy_adj(struct sdhci_host *host, struct mmc_ios *ios)
{
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	struct mmc_host *mmc = host->mmc;
	struct mmc_card *card;
	int ret = 0;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_xenon_priv *priv = sdhci_pltfm_priv(pltfm_host);
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	int ret = 0;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (!host->clock) {
		priv->clock = 0;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (!host->clock)
		return 0;

	if ((ios->timing != priv->timing) || (ios->clock != priv->clock))
		xenon_phy_set(host, priv, ios->timing);

	/* Legacy mode is a special case */
	if (ios->timing == MMC_TIMING_LEGACY) {
		priv->timing = ios->timing;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		return 0;
	}

	/*
	 * The timing, frequency or bus width is changed,
	 * better to set eMMC PHY based on current setting
	 * and adjust Xenon SDHC delay.
	 */
	if ((host->clock == priv->clock) &&
		(ios->bus_width == priv->bus_width) &&
		(ios->timing == priv->timing))
		return 0;

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	emmc_phy_set(host, ios->timing);

#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	/* Update the record */
	priv->bus_width = ios->bus_width;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)

#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	/* Skip temp stages from HS200 to HS400 */
	if (temp_stage_hs200_to_hs400(host, priv))
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	/* Temp stage from HS200 to HS400 */
	if (((priv->timing == MMC_TIMING_MMC_HS200) &&
		(ios->timing == MMC_TIMING_MMC_HS)) ||
		((priv->timing == MMC_TIMING_MMC_HS200) &&
		(priv->clock > host->clock))) {
		priv->timing = ios->timing;
		priv->clock = host->clock;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		return 0;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */

	/* Skip temp stages from HS400 t0 HS200 */
	if (temp_stage_hs400_to_h200(host, priv))
		return 0;

#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	priv->timing = ios->timing;
	priv->clock = host->clock;

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	/* Legacy mode is a special case */
	if (ios->timing == MMC_TIMING_LEGACY)
		return 0;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	card = priv->card_candidate;
	if (unlikely(card == NULL)) {
		WARN("%s: card is not present\n", mmc_hostname(mmc));
		return -EINVAL;
	}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	if (host->clock > XENON_DEFAULT_SDCLK_FREQ)
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
	if (host->clock > SDHCI_DEFAULT_SDCLK_FREQ)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
		ret = xenon_hs_delay_adj(host);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (host->clock > DEFAULT_SDCLK_FREQ)
		ret = xenon_hs_delay_adj(mmc, card, priv);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	return ret;
}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
static void clean_emmc_phy(struct sdhci_xenon_priv *priv)
{
	kfree(priv->phy_params);
}

#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
static int add_xenon_phy(struct device_node *np, struct sdhci_host *host,
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static int add_xenon_phy(struct device_node *np, struct sdhci_xenon_priv *priv,
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
			const char *phy_name)
{
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_xenon_priv *priv = sdhci_pltfm_priv(pltfm_host);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	int i, ret;

	for (i = 0; i < NR_PHY_TYPES; i++) {
		if (!strcmp(phy_name, phy_types[i])) {
			priv->phy_type = i;
			break;
		}
	}
	if (i == NR_PHY_TYPES) {
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
		dev_err(mmc_dev(host->mmc),
			"Unable to determine PHY name %s. Use default eMMC 5.1 PHY\n",
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		pr_err("Unable to determine PHY name %s. Use default eMMC 5.1 PHY\n",
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
			phy_name);
		priv->phy_type = EMMC_5_1_PHY;
	}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
// do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (priv->phy_type == SDH_PHY)
		return alloc_sdh_phy(priv);
	else if ((priv->phy_type == EMMC_5_0_PHY) ||
			(priv->phy_type == EMMC_5_1_PHY)) {
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		ret = alloc_emmc_phy(priv);
		if (ret)
			return ret;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
// do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
		return emmc_phy_parse_param_dt(np, priv->phy_params);
	}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	return emmc_phy_parse_param_dt(host, np, priv->phy_params);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	return -EINVAL;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
	ret = emmc_phy_parse_param_dt(host, np, priv->phy_params);
	if (ret)
		clean_emmc_phy(priv);

	return ret;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
int xenon_phy_parse_dt(struct device_node *np, struct sdhci_host *host)
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
int xenon_phy_parse_dt(struct device_node *np, struct sdhci_xenon_priv *priv)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
{
	const char *phy_type = NULL;

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	if (!of_property_read_string(np, "marvell,xenon-phy-type", &phy_type))
		return add_xenon_phy(np, host, phy_type);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	if (!of_property_read_string(np, "xenon,phy-type", &phy_type))
		return add_xenon_phy(np, priv, phy_type);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	dev_info(mmc_dev(host->mmc), "Fail to get Xenon PHY type. Use default eMMC 5.1 PHY\n");
	return add_xenon_phy(np, host, "emmc 5.1 phy");
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	pr_err("Fail to get Xenon PHY type. Use default eMMC 5.1 PHY\n");
	return add_xenon_phy(np, priv, "emmc 5.1 phy");
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
}
#endif /* CONFIG_SYNO_LSP_ARMADA_16_12 */
