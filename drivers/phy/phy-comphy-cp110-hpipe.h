/*
 * Marvell CP110 COMPHY HPIPE definitions.
 *
 * Copyright (C) 2017 Marvell Semiconductor
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#ifndef _COMPHY_CP110_HPIPE_H
#define _COMPHY_CP110_HPIPE_H

#define HPIPE_INTERFACE_REG			0x94
#define HPIPE_INTERFACE_GEN_MAX_OFFSET		10
#define HPIPE_INTERFACE_GEN_MAX_MASK		(0x3 << HPIPE_INTERFACE_GEN_MAX_OFFSET)
#define HPIPE_INTERFACE_DET_BYPASS_OFFSET	12
#define HPIPE_INTERFACE_DET_BYPASS_MASK		(0x1 << HPIPE_INTERFACE_DET_BYPASS_OFFSET)
#define HPIPE_INTERFACE_LINK_TRAIN_OFFSET	14
#define HPIPE_INTERFACE_LINK_TRAIN_MASK		(0x1 << HPIPE_INTERFACE_LINK_TRAIN_OFFSET)

#define HPIPE_PCIE_REG0                         0x120
#define HPIPE_PCIE_IDLE_SYNC_OFFSET		12
#define HPIPE_PCIE_IDLE_SYNC_MASK		(0x1 << HPIPE_PCIE_IDLE_SYNC_OFFSET)
#define HPIPE_PCIE_SEL_BITS_OFFSET		13
#define HPIPE_PCIE_SEL_BITS_MASK		(0x3 << HPIPE_PCIE_SEL_BITS_OFFSET)

#define HPIPE_LANE_ALIGN_REG			0x124
#define HPIPE_LANE_ALIGN_OFF_OFFSET		12
#define HPIPE_LANE_ALIGN_OFF_MASK		(0x1 << HPIPE_LANE_ALIGN_OFF_OFFSET)

#define HPIPE_MISC_REG				0x13C
#define HPIPE_MISC_CLK100M_125M_OFFSET		4
#define HPIPE_MISC_CLK100M_125M_MASK		(0x1 << HPIPE_MISC_CLK100M_125M_OFFSET)
#define HPIPE_MISC_ICP_FORCE_OFFSET		5
#define HPIPE_MISC_ICP_FORCE_MASK		(0x1 << HPIPE_MISC_ICP_FORCE_OFFSET)
#define HPIPE_MISC_TXDCLK_2X_OFFSET		6
#define HPIPE_MISC_TXDCLK_2X_MASK		(0x1 << HPIPE_MISC_TXDCLK_2X_OFFSET)
#define HPIPE_MISC_CLK500_EN_OFFSET		7
#define HPIPE_MISC_CLK500_EN_MASK		(0x1 << HPIPE_MISC_CLK500_EN_OFFSET)
#define HPIPE_MISC_REFCLK_SEL_OFFSET		10
#define HPIPE_MISC_REFCLK_SEL_MASK		(0x1 << HPIPE_MISC_REFCLK_SEL_OFFSET)

#define HPIPE_TX_TRAIN_CTRL_REG			0x26C
#define HPIPE_TX_TRAIN_CTRL_G1_OFFSET		0
#define HPIPE_TX_TRAIN_CTRL_G1_MASK		(0x1 << HPIPE_TX_TRAIN_CTRL_G1_OFFSET)
#define HPIPE_TX_TRAIN_CTRL_GN1_OFFSET		1
#define HPIPE_TX_TRAIN_CTRL_GN1_MASK		(0x1 << HPIPE_TX_TRAIN_CTRL_GN1_OFFSET)
#define HPIPE_TX_TRAIN_CTRL_G0_OFFSET		2
#define HPIPE_TX_TRAIN_CTRL_G0_MASK		(0x1 << HPIPE_TX_TRAIN_CTRL_G0_OFFSET)

#define HPIPE_TX_TRAIN_CTRL_4_REG		0x278
#define HPIPE_TRX_TRAIN_TIMER_OFFSET		0
#define HPIPE_TRX_TRAIN_TIMER_MASK		(0x3FF << HPIPE_TRX_TRAIN_TIMER_OFFSET)

#define HPIPE_TX_TRAIN_CTRL_5_REG		0x2A4
#define HPIPE_RX_TRAIN_TIMER_OFFSET		0
#define HPIPE_RX_TRAIN_TIMER_MASK		(0x3ff << HPIPE_RX_TRAIN_TIMER_OFFSET)
#define HPIPE_TX_TRAIN_START_SQ_EN_OFFSET	11
#define HPIPE_TX_TRAIN_START_SQ_EN_MASK		(0x1 << HPIPE_TX_TRAIN_START_SQ_EN_OFFSET)
#define HPIPE_TX_TRAIN_START_FRM_DET_EN_OFFSET	12
#define HPIPE_TX_TRAIN_START_FRM_DET_EN_MASK	(0x1 << HPIPE_TX_TRAIN_START_FRM_DET_EN_OFFSET)
#define HPIPE_TX_TRAIN_START_FRM_LOCK_EN_OFFSET	13
#define HPIPE_TX_TRAIN_START_FRM_LOCK_EN_MASK	(0x1 << HPIPE_TX_TRAIN_START_FRM_LOCK_EN_OFFSET)
#define HPIPE_TX_TRAIN_WAIT_TIME_EN_OFFSET	14
#define HPIPE_TX_TRAIN_WAIT_TIME_EN_MASK	(0x1 << HPIPE_TX_TRAIN_WAIT_TIME_EN_OFFSET)

#define HPIPE_TX_TRAIN_REG			0x31C
#define HPIPE_TX_TRAIN_CHK_INIT_OFFSET		4
#define HPIPE_TX_TRAIN_CHK_INIT_MASK		(0x1 << HPIPE_TX_TRAIN_CHK_INIT_OFFSET)
#define HPIPE_TX_TRAIN_COE_FM_PIN_PCIE3_OFFSET	7
#define HPIPE_TX_TRAIN_COE_FM_PIN_PCIE3_MASK	(0x1 << HPIPE_TX_TRAIN_COE_FM_PIN_PCIE3_OFFSET)
#define HPIPE_TX_TRAIN_16BIT_AUTO_EN_OFFSET	8
#define HPIPE_TX_TRAIN_16BIT_AUTO_EN_MASK	(0x1 << HPIPE_TX_TRAIN_16BIT_AUTO_EN_OFFSET)
#define HPIPE_TX_TRAIN_PAT_SEL_OFFSET		9
#define HPIPE_TX_TRAIN_PAT_SEL_MASK		(0x1 << HPIPE_TX_TRAIN_PAT_SEL_OFFSET)

#define HPIPE_CDR_CONTROL_REG			0x418
#define HPIPE_CDR_RX_MAX_DFE_ADAPT_1_OFFSET	12
#define HPIPE_CDR_RX_MAX_DFE_ADAPT_1_MASK	(0x3 << HPIPE_CDR_RX_MAX_DFE_ADAPT_1_OFFSET)
#define HPIPE_CDR_MAX_DFE_ADAPT_0_OFFSET	9
#define HPIPE_CDR_MAX_DFE_ADAPT_0_MASK		(0x7 << HPIPE_CDR_MAX_DFE_ADAPT_0_OFFSET)
#define HPIPE_CDR_MAX_DFE_ADAPT_1_OFFSET	6
#define HPIPE_CDR_MAX_DFE_ADAPT_1_MASK		(0x7 << HPIPE_CDR_MAX_DFE_ADAPT_1_OFFSET)

#define HPIPE_TX_TRAIN_CTRL_11_REG		0x438
#define HPIPE_TX_STATUS_CHECK_MODE_OFFSET	6
#define HPIPE_TX_TX_STATUS_CHECK_MODE_MASK	(0x1 << HPIPE_TX_STATUS_CHECK_MODE_OFFSET)
#define HPIPE_TX_NUM_OF_PRESET_OFFSET		10
#define HPIPE_TX_NUM_OF_PRESET_MASK		(0x7 << HPIPE_TX_NUM_OF_PRESET_OFFSET)
#define HPIPE_TX_SWEEP_PRESET_EN_OFFSET		15
#define HPIPE_TX_SWEEP_PRESET_EN_MASK		(0x1 << HPIPE_TX_SWEEP_PRESET_EN_OFFSET)

#define HPIPE_G2_SETTINGS_4_REG			0x44c
#define HPIPE_G2_DFE_RES_OFFSET			8
#define HPIPE_G2_DFE_RES_MASK			(0x3 << HPIPE_G2_DFE_RES_OFFSET)

#define HPIPE_DFE_CONTROL_REG			0x470
#define HPIPE_DFE_TX_MAX_DFE_ADAPT_OFFSET	14
#define HPIPE_DFE_TX_MAX_DFE_ADAPT_MASK		(0x3 << HPIPE_DFE_TX_MAX_DFE_ADAPT_OFFSET)

#define HPIPE_DFE_CTRL_28_REG			0x49C
#define HPIPE_DFE_CTRL_28_PIPE4_OFFSET		7
#define HPIPE_DFE_CTRL_28_PIPE4_MASK		(0x1 << HPIPE_DFE_CTRL_28_PIPE4_OFFSET)

#define HPIPE_G3_SETTING_5_REG			0x548
#define HPIPE_G3_SETTING_5_G3_ICP_OFFSET	0
#define HPIPE_G3_SETTING_5_G3_ICP_MASK		(0xf << HPIPE_G3_SETTING_5_G3_ICP_OFFSET)

#define HPIPE_LANE_CONFIG0_REG			0x600
#define HPIPE_LANE_CONFIG0_TXDEEMPH0_OFFSET	0
#define HPIPE_LANE_CONFIG0_TXDEEMPH0_MASK	(0x1 << HPIPE_LANE_CONFIG0_TXDEEMPH0_OFFSET)

#define HPIPE_LANE_STATUS1_REG			0x60C
#define HPIPE_LANE_STATUS1_PCLK_EN_OFFSET	0
#define HPIPE_LANE_STATUS1_PCLK_EN_MASK		(0x1 << HPIPE_LANE_STATUS1_PCLK_EN_OFFSET)

#define HPIPE_LANE_CFG4_REG                     0x620
#define HPIPE_LANE_CFG4_DFE_CTRL_OFFSET		0
#define HPIPE_LANE_CFG4_DFE_CTRL_MASK		(0x7 << HPIPE_LANE_CFG4_DFE_CTRL_OFFSET)
#define HPIPE_LANE_CFG4_DFE_EN_SEL_OFFSET	3
#define HPIPE_LANE_CFG4_DFE_EN_SEL_MASK		(0x1 << HPIPE_LANE_CFG4_DFE_EN_SEL_OFFSET)
#define HPIPE_LANE_CFG4_DFE_OVER_OFFSET		6
#define HPIPE_LANE_CFG4_DFE_OVER_MASK		(0x1 << HPIPE_LANE_CFG4_DFE_OVER_OFFSET)
#define HPIPE_LANE_CFG4_SSC_CTRL_OFFSET		7
#define HPIPE_LANE_CFG4_SSC_CTRL_MASK		(0x1 << HPIPE_LANE_CFG4_SSC_CTRL_OFFSET)

#define HPIPE_LANE_EQ_REMOTE_SETTING_REG	0x6f8
#define HPIPE_LANE_CFG_FOM_DIRN_OVERRIDE_OFFSET	0
#define HPIPE_LANE_CFG_FOM_DIRN_OVERRIDE_MASK	(0x1 << HPIPE_LANE_CFG_FOM_DIRN_OVERRIDE_OFFSET)
#define HPIPE_LANE_CFG_FOM_ONLY_MODE_OFFFSET	1
#define HPIPE_LANE_CFG_FOM_ONLY_MODE_MASK	(0x1 << HPIPE_LANE_CFG_FOM_ONLY_MODE_OFFFSET)
#define HPIPE_LANE_CFG_FOM_PRESET_VECTOR_OFFSET	2
#define HPIPE_LANE_CFG_FOM_PRESET_VECTOR_MASK	(0xf << HPIPE_LANE_CFG_FOM_PRESET_VECTOR_OFFSET)

#define HPIPE_LANE_EQU_CONFIG_0_REG		0x69C
#define HPIPE_CFG_PHY_RC_EP_OFFSET		12
#define HPIPE_CFG_PHY_RC_EP_MASK		(0x1 << HPIPE_CFG_PHY_RC_EP_OFFSET)

#define HPIPE_LANE_EQ_CFG1_REG			0x6a0
#define HPIPE_CFG_UPDATE_POLARITY_OFFSET	12
#define HPIPE_CFG_UPDATE_POLARITY_MASK		(0x1 << HPIPE_CFG_UPDATE_POLARITY_OFFSET)

#define HPIPE_RST_CLK_CTRL_REG			0x704
#define HPIPE_RST_CLK_CTRL_PIPE_RST_OFFSET	0
#define HPIPE_RST_CLK_CTRL_PIPE_RST_MASK	(0x1 << HPIPE_RST_CLK_CTRL_PIPE_RST_OFFSET)
#define HPIPE_RST_CLK_CTRL_FIXED_PCLK_OFFSET	2
#define HPIPE_RST_CLK_CTRL_FIXED_PCLK_MASK	(0x1 << HPIPE_RST_CLK_CTRL_FIXED_PCLK_OFFSET)
#define HPIPE_RST_CLK_CTRL_PIPE_WIDTH_OFFSET	3
#define HPIPE_RST_CLK_CTRL_PIPE_WIDTH_MASK	(0x1 << HPIPE_RST_CLK_CTRL_PIPE_WIDTH_OFFSET)
#define HPIPE_RST_CLK_CTRL_CORE_FREQ_SEL_OFFSET	9
#define HPIPE_RST_CLK_CTRL_CORE_FREQ_SEL_MASK	(0x1 << HPIPE_RST_CLK_CTRL_CORE_FREQ_SEL_OFFSET)

#define HPIPE_TST_MODE_CTRL_REG			0x708
#define HPIPE_TST_MODE_CTRL_MODE_MARGIN_OFFSET	2
#define HPIPE_TST_MODE_CTRL_MODE_MARGIN_MASK	(0x1 << HPIPE_TST_MODE_CTRL_MODE_MARGIN_OFFSET)

#define HPIPE_CLK_SRC_LO_REG			0x70c
#define HPIPE_CLK_SRC_LO_BUNDLE_PERIOD_SEL_OFFSET	1
#define HPIPE_CLK_SRC_LO_BUNDLE_PERIOD_SEL_MASK		(0x1 << HPIPE_CLK_SRC_LO_BUNDLE_PERIOD_SEL_OFFSET)
#define HPIPE_CLK_SRC_LO_BUNDLE_PERIOD_SCALE_OFFSET	2
#define HPIPE_CLK_SRC_LO_BUNDLE_PERIOD_SCALE_MASK	(0x3 << HPIPE_CLK_SRC_LO_BUNDLE_PERIOD_SCALE_OFFSET)
#define HPIPE_CLK_SRC_LO_PLL_RDY_DL_OFFSET	5
#define HPIPE_CLK_SRC_LO_PLL_RDY_DL_MASK	(0x7 << HPIPE_CLK_SRC_LO_PLL_RDY_DL_OFFSET)

#define HPIPE_CLK_SRC_HI_REG			0x710
#define HPIPE_CLK_SRC_HI_LANE_STRT_OFFSET	0
#define HPIPE_CLK_SRC_HI_LANE_STRT_MASK		(0x1 << HPIPE_CLK_SRC_HI_LANE_STRT_OFFSET)
#define HPIPE_CLK_SRC_HI_LANE_BREAK_OFFSET	1
#define HPIPE_CLK_SRC_HI_LANE_BREAK_MASK	(0x1 << HPIPE_CLK_SRC_HI_LANE_BREAK_OFFSET)
#define HPIPE_CLK_SRC_HI_LANE_MASTER_OFFSET	2
#define HPIPE_CLK_SRC_HI_LANE_MASTER_MASK	(0x1 << HPIPE_CLK_SRC_HI_LANE_MASTER_OFFSET)
#define HPIPE_CLK_SRC_HI_MODE_PIPE_OFFSET	7
#define HPIPE_CLK_SRC_HI_MODE_PIPE_MASK		(0x1 << HPIPE_CLK_SRC_HI_MODE_PIPE_OFFSET)

#define HPIPE_GLOBAL_MISC_CTRL                  0x718
#define HPIPE_GLOBAL_PM_CTRL                    0x740
#define HPIPE_GLOBAL_PM_RXDLOZ_WAIT_OFFSET	0
#define HPIPE_GLOBAL_PM_RXDLOZ_WAIT_MASK	(0xFF << HPIPE_GLOBAL_PM_RXDLOZ_WAIT_OFFSET)

#endif /* !_COMPHY_CP110_HPIPE_H */