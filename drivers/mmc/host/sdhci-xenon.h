#if defined(CONFIG_SYNO_LSP_ARMADA_16_12)
/*
 * Copyright (C) 2016 Marvell, All Rights Reserved.
 *
 * Author: Hu Ziji <huziji@marvell.com>
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
 * Date:	2016-8-24
#else  // CONFIG_SYNO_LSP_ARMADA_17_02_02
 * Date:   2016-7-30
#endif // CONFIG_SYNO_LSP_ARMADA_17_02_02
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 */
#ifndef SDHCI_XENON_H_
#define SDHCI_XENON_H_

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#include <linux/clk.h>
#include <linux/mmc/card.h>
#include <linux/of.h>
#include "sdhci.h"
#include "sdhci-xenon-phy.h"

/* Register Offset of SD Host Controller SOCP self-defined register */
#define SDHC_SYS_CFG_INFO			0x0104
#define SLOT_TYPE_SDIO_SHIFT			24
#define SLOT_TYPE_EMMC_MASK			0xff
#define SLOT_TYPE_EMMC_SHIFT			16
#define SLOT_TYPE_SD_SDIO_MMC_MASK		0xff
#define SLOT_TYPE_SD_SDIO_MMC_SHIFT		8

#define SDHC_SYS_OP_CTRL			0x0108
#define AUTO_CLKGATE_DISABLE_MASK		(0x1<<20)
#define SDCLK_IDLEOFF_ENABLE_SHIFT		8
#define SLOT_ENABLE_SHIFT			0

#define SDHC_SYS_EXT_OP_CTRL			0x010c

#define SDHC_SLOT_OP_STATUS_CTRL		0x0128
#define DELAY_90_DEGREE_MASK_EMMC5		(1 << 7)
#define DELAY_90_DEGREE_SHIFT_EMMC5		7
#define EMMC_5_0_PHY_FIXED_DELAY_MASK		0x7f
#define EMMC_PHY_FIXED_DELAY_MASK		0xff
#define EMMC_PHY_FIXED_DELAY_WINDOW_MIN		(EMMC_PHY_FIXED_DELAY_MASK >> 3)
#define SDH_PHY_FIXED_DELAY_MASK		0x1ff
#define SDH_PHY_FIXED_DELAY_WINDOW_MIN		(SDH_PHY_FIXED_DELAY_MASK >> 4)

#define TUN_CONSECUTIVE_TIMES_SHIFT		16
#define TUN_CONSECUTIVE_TIMES_MASK		0x7
#define TUN_CONSECUTIVE_TIMES			0x4
#define TUNING_STEP_SHIFT			12
#define TUNING_STEP_MASK			0xf

#define TUNING_STEP_DIVIDER			64

#define FORCE_SEL_INVERSE_CLK_SHIFT		11

#define SDHC_SLOT_eMMC_CTRL			0x0130
#define ENABLE_DATA_STROBE			(1 << 24)
#define SET_EMMC_RSTN				(1 << 16)
#define DISABLE_RD_DATA_CRC			(1 << 14)
#define DISABLE_CRC_STAT_TOKEN			(1 << 13)
#define eMMC_VCCQ_MASK				0x3
#define eMMC_VCCQ_1_8V				0x1
#define eMMC_VCCQ_3_3V				0x3

#define SDHC_SLOT_RETUNING_REQ_CTRL		0x0144
/* retuning compatible */
#define RETUNING_COMPATIBLE			0x1
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
/* Register Offset of Xenon SDHC self-defined register */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#define XENON_SYS_CFG_INFO			0x0104
#define XENON_SLOT_TYPE_SDIO_SHIFT		24
#define XENON_NR_SUPPORTED_SLOT_MASK		0x7
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#define SDHCI_SYS_CFG_INFO			0x0104
#define SDHCI_SLOT_TYPE_SDIO_SHIFT		24
#define SDHCI_NR_SUPPORTED_SLOT_MASK		0x7
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#define SDHC_SLOT_AUTO_RETUNING_CTRL		0x0148
#define ENABLE_AUTO_RETUNING			0x1
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#define XENON_SYS_OP_CTRL			0x0108
#define XENON_AUTO_CLKGATE_DISABLE_MASK		BIT(20)
#define XENON_SDCLK_IDLEOFF_ENABLE_SHIFT	8
#define XENON_SLOT_ENABLE_SHIFT			0
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#define SDHCI_SYS_OP_CTRL			0x0108
#define SDHCI_AUTO_CLKGATE_DISABLE_MASK		BIT(20)
#define SDHCI_SDCLK_IDLEOFF_ENABLE_SHIFT	8
#define SDHCI_SLOT_ENABLE_SHIFT			0
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#define SDHC_SLOT_DLL_CUR_DLY_VAL		0x150
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#define XENON_SYS_EXT_OP_CTRL			0x010C
#define XENON_MASK_CMD_CONFLICT_ERR		BIT(8)
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#define SDHCI_SYS_EXT_OP_CTRL			0x010C
#define SDHCI_MASK_CMD_CONFLICT_ERROR		BIT(8)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#define XENON_SLOT_OP_STATUS_CTRL		0x0128
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#define SDHCI_SLOT_OP_STATUS_CTRL		0x0128
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
/* Tuning Parameter */
#define TMR_RETUN_NO_PRESENT			0xf
#define XENON_MAX_TUN_COUNT			0xe
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#define XENON_TUN_CONSECUTIVE_TIMES_SHIFT	16
#define XENON_TUN_CONSECUTIVE_TIMES_MASK	0x7
#define XENON_TUN_CONSECUTIVE_TIMES		0x4
#define XENON_TUNING_STEP_SHIFT			12
#define XENON_TUNING_STEP_MASK			0xF
#define XENON_TUNING_STEP_DIVIDER		BIT(6)

#define XENON_SLOT_EMMC_CTRL			0x0130
#define XENON_ENABLE_DATA_STROBE		BIT(24)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_10)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_10 */
#define XENON_EMMC_VCCQ_MASK			0x3
#define XENON_EMMC_VCCQ_1_8V			0x1
#define XENON_EMMC_VCCQ_3_3V			0x3
#endif /* CONFIG_SYNO_LSP_ARMADA_17_10 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#define SDHCI_TUN_CONSECUTIVE_TIMES_SHIFT	16
#define SDHCI_TUN_CONSECUTIVE_TIMES_MASK	0x7
#define SDHCI_TUN_CONSECUTIVE_TIMES		0x4
#define SDHCI_TUNING_STEP_SHIFT			12
#define SDHCI_TUNING_STEP_MASK			0xF
#define SDHCI_TUNING_STEP_DIVIDER		BIT(6)

#define SDHCI_SLOT_EMMC_CTRL			0x0130
#define SDHCI_ENABLE_DATA_STROBE		BIT(24)
#define SDHCI_EMMC_VCCQ_MASK			0x3
#define SDHCI_EMMC_VCCQ_1_8V			0x1
#define SDHCI_EMMC_VCCQ_3_3V			0x3
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#define MMC_TIMING_FAKE				0xff
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#define XENON_SLOT_RETUNING_REQ_CTRL		0x0144
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#define SDHCI_SLOT_RETUNING_REQ_CTRL		0x0144
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
/* retuning compatible */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#define XENON_RETUNING_COMPATIBLE		0x1
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#define SDHCI_RETUNING_COMPATIBLE		0x1
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#define XENON_SLOT_EXT_PRESENT_STATE		0x014C
#define XENON_DLL_LOCK_STATE			0x1
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#define SDHCI_SLOT_EXT_PRESENT_STATE		0x014C
#define SDHCI_DLL_LOCK_STATE			0x1
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#define DEF_TUNING_COUNT			0x9
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#define XENON_SLOT_DLL_CUR_DLY_VAL		0x0150
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#define SDHCI_SLOT_DLL_CUR_DLY_VAL		0x0150
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#define DEFAULT_SDCLK_FREQ			(400000)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
/* Tuning Parameter */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#define XENON_TMR_RETUN_NO_PRESENT		0xF
#define XENON_DEF_TUNING_COUNT			0x9
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#define SDHCI_TMR_RETUN_NO_PRESENT		0xF
#define SDHCI_DEF_TUNING_COUNT			0x9
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#define XENON_DEFAULT_SDCLK_FREQ		400000
#define XENON_LOWEST_SDCLK_FREQ			100000
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#define SDHCI_DEFAULT_SDCLK_FREQ		(400000)
#define SDHCI_LOWEST_SDCLK_FREQ			(100000)
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

/* Xenon specific Mode Select value */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#define XENON_CTRL_HS200			0x5
#define XENON_CTRL_HS400			0x6
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#define SDHCI_XENON_CTRL_HS200			0x5
#define SDHCI_XENON_CTRL_HS400			0x6
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_10)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_10 */
/* Indicate Card Type is not clear yet */
#endif /* CONFIG_SYNO_LSP_ARMADA_17_10 */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#if defined(CONFIG_SYNO_LSP_ARMADA_17_10)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_10 */
#define XENON_CARD_TYPE_UNKNOWN			0xF
#endif /* CONFIG_SYNO_LSP_ARMADA_17_10 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#define SDHCI_CARD_TYPE_UNKNOWN			0xF
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
#define XENON_SDHCI_CTRL_HS200			0x5
#define	XENON_SDHCI_CTRL_HS400			0x6
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

#if defined(CONFIG_SYNO_LSP_ARMADA_17_10)
struct xenon_priv {
#else /* CONFIG_SYNO_LSP_ARMADA_17_10 */
struct sdhci_xenon_priv {
#endif /* CONFIG_SYNO_LSP_ARMADA_17_10 */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	unsigned char	tuning_count;
	/* idx of SDHC */
	u8		sdhc_id;

	/*
#if defined(CONFIG_SYNO_LSP_ARMADA_17_10)
	 * eMMC/SD/SDIO require different register settings.
	 * Xenon driver has to recognize card type
	 * before mmc_host->card is not available.
#else /* CONFIG_SYNO_LSP_ARMADA_17_10 */
	 * eMMC/SD/SDIO require different PHY settings or
	 * voltage control. It's necessary for Xenon driver to
	 * recognize card type during, or even before initialization.
	 * However, mmc_host->card is not available yet at that time.
#endif /* CONFIG_SYNO_LSP_ARMADA_17_10 */
	 * This field records the card type during init.
#if defined(CONFIG_SYNO_LSP_ARMADA_17_10)
	 * It is updated in xenon_init_card().
#else /* CONFIG_SYNO_LSP_ARMADA_17_10 */
	 * For eMMC, it is updated in dt parse. For SD/SDIO, it is
	 * updated in xenon_init_card().
#endif /* CONFIG_SYNO_LSP_ARMADA_17_10 */
	 *
	 * It is only valid during initialization after it is updated.
	 * Do not access this variable in normal transfers after
	 * initialization completes.
	 */
	unsigned int	init_card_type;

#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	/*
	 * The bus_width, timing, and clock fields in below
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	 * record the current ios setting of Xenon SDHC.
	 * Driver will adjust PHY setting if any change to
	 * ios affects PHY timing.
#else  // CONFIG_SYNO_LSP_ARMADA_17_02_02
	 * record the current setting of Xenon SDHC.
	 * Driver will call a Sampling Fixed Delay Adjustment
	 * if any setting is changed.
#endif // CONFIG_SYNO_LSP_ARMADA_17_02_02
	 */
	unsigned char	bus_width;
	unsigned char	timing;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	unsigned char	tuning_count;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	unsigned int	clock;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	struct clk      *axi_clk;

	/* Slot idx */
	u8		slot_idx;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

	int		phy_type;
	/*
	 * Contains board-specific PHY parameters
	 * passed from device tree.
	 */
	void		*phy_params;
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
	struct xenon_emmc_phy_regs *emmc_phy_regs;
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
	struct xenon_phy_ops phy_ops;

	/*
	 * When initializing card, Xenon has to determine card type and
	 * adjust Sampling Fixed delay.
	 * However, at that time, card structure is not linked to mmc_host.
	 * Thus a card pointer is added here to provide
	 * the delay adjustment function with the card structure
	 * of the card during initialization
	 */
	struct mmc_card *card_candidate;
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
};

#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
//do nothing
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
static inline int enable_xenon_internal_clk(struct sdhci_host *host)
{
	u32 reg;
	u8 timeout;

	reg = sdhci_readl(host, SDHCI_CLOCK_CONTROL);
	reg |= SDHCI_CLOCK_INT_EN;
	sdhci_writel(host, reg, SDHCI_CLOCK_CONTROL);
	/* Wait max 20 ms */
	timeout = 20;
	while (!((reg = sdhci_readw(host, SDHCI_CLOCK_CONTROL))
			& SDHCI_CLOCK_INT_STABLE)) {
		if (timeout == 0) {
			pr_err("%s: Internal clock never stabilised.\n",
					mmc_hostname(host->mmc));
			return -EIO;
		}
		timeout--;
		mdelay(1);
	}

	return 0;
}
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */

int xenon_phy_adj(struct sdhci_host *host, struct mmc_ios *ios);
int xenon_phy_parse_dt(struct device_node *np,
#if defined(CONFIG_SYNO_LSP_ARMADA_17_02_02)
		       struct sdhci_host *host);
#else /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
			struct sdhci_xenon_priv *priv);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_02_02 */
void xenon_soc_pad_ctrl(struct sdhci_host *host,
			unsigned char signal_voltage);
#endif
#endif /* CONFIG_SYNO_LSP_ARMADA_16_12 */
