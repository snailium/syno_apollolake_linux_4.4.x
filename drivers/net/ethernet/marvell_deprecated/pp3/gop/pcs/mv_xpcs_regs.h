#if defined(CONFIG_SYNO_LSP_ARMADA_16_12)
/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
*******************************************************************************/
#ifndef __mv_xpcs_regs_h__
#define __mv_xpcs_regs_h__

/* includes */

/* unit offset */
#define MV_PP3_XPCS_UNIT_OFFSET		0x03180000

/* Global Configuration 0 */
#define MV_XPCS_GLOBAL_CFG_0_REG				(MV_PP3_XPCS_UNIT_OFFSET + 0x0400)
#define MV_XPCS_GLOBAL_CFG_0_PCSRESET_OFFS		0
#define MV_XPCS_GLOBAL_CFG_0_PCSRESET_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_0_PCSRESET_OFFS)

#define MV_XPCS_GLOBAL_CFG_0_DESKEWRESET_OFFS		1
#define MV_XPCS_GLOBAL_CFG_0_DESKEWRESET_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_0_DESKEWRESET_OFFS)

#define MV_XPCS_GLOBAL_CFG_0_TXRESET_OFFS		2
#define MV_XPCS_GLOBAL_CFG_0_TXRESET_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_0_TXRESET_OFFS)

#define MV_XPCS_GLOBAL_CFG_0_PCSMODE_OFFS		3
#define MV_XPCS_GLOBAL_CFG_0_PCSMODE_MASK    \
		(0x00000003 << MV_XPCS_GLOBAL_CFG_0_PCSMODE_OFFS)

#define MV_XPCS_GLOBAL_CFG_0_LANEACTIVE_OFFS		5
#define MV_XPCS_GLOBAL_CFG_0_LANEACTIVE_MASK    \
		(0x00000003 << MV_XPCS_GLOBAL_CFG_0_LANEACTIVE_OFFS)

#define MV_XPCS_GLOBAL_CFG_0_INDIVIDUALMODE_OFFS		7
#define MV_XPCS_GLOBAL_CFG_0_INDIVIDUALMODE_MASK    \
		(0x0000003f << MV_XPCS_GLOBAL_CFG_0_INDIVIDUALMODE_OFFS)

#define MV_XPCS_GLOBAL_CFG_0_TXSMMODE_OFFS		13
#define MV_XPCS_GLOBAL_CFG_0_TXSMMODE_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_0_TXSMMODE_OFFS)

#define MV_XPCS_GLOBAL_CFG_0_TXSMIDLECNTDISABLE_OFFS		14
#define MV_XPCS_GLOBAL_CFG_0_TXSMIDLECNTDISABLE_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_0_TXSMIDLECNTDISABLE_OFFS)

#define MV_XPCS_GLOBAL_CFG_0_COMMADETCT2NDSYNCSMEN_OFFS		15
#define MV_XPCS_GLOBAL_CFG_0_COMMADETCT2NDSYNCSMEN_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_0_COMMADETCT2NDSYNCSMEN_OFFS)

/* Global Configuration 1 */
#define MV_XPCS_GLOBAL_CFG_1_REG				(MV_PP3_XPCS_UNIT_OFFSET + 0x0404)
#define MV_XPCS_GLOBAL_CFG_1_MACLOOPBACKEN_OFFS		0
#define MV_XPCS_GLOBAL_CFG_1_MACLOOPBACKEN_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_1_MACLOOPBACKEN_OFFS)

#define MV_XPCS_GLOBAL_CFG_1_PCSLOOPBACKEN_OFFS		1
#define MV_XPCS_GLOBAL_CFG_1_PCSLOOPBACKEN_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_1_PCSLOOPBACKEN_OFFS)

#define MV_XPCS_GLOBAL_CFG_1_REPEATERMODEEN_OFFS		2
#define MV_XPCS_GLOBAL_CFG_1_REPEATERMODEEN_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_1_REPEATERMODEEN_OFFS)

#define MV_XPCS_GLOBAL_CFG_1_LOOPBACKCLKSEL_OFFS		3
#define MV_XPCS_GLOBAL_CFG_1_LOOPBACKCLKSEL_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_1_LOOPBACKCLKSEL_OFFS)

#define MV_XPCS_GLOBAL_CFG_1_DESKEWCLKSEL_OFFS		4
#define MV_XPCS_GLOBAL_CFG_1_DESKEWCLKSEL_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_1_DESKEWCLKSEL_OFFS)

#define MV_XPCS_GLOBAL_CFG_1_TXSMREPEATERMODE_OFFS		5
#define MV_XPCS_GLOBAL_CFG_1_TXSMREPEATERMODE_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_1_TXSMREPEATERMODE_OFFS)

#define MV_XPCS_GLOBAL_CFG_1_RXLOCKBYPASSEN_OFFS		6
#define MV_XPCS_GLOBAL_CFG_1_RXLOCKBYPASSEN_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_1_RXLOCKBYPASSEN_OFFS)

#define MV_XPCS_GLOBAL_CFG_1_TXLOCKBYPASSEN_OFFS		7
#define MV_XPCS_GLOBAL_CFG_1_TXLOCKBYPASSEN_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_1_TXLOCKBYPASSEN_OFFS)

#define MV_XPCS_GLOBAL_CFG_1_REMOTEFAULTDIS_OFFS		8
#define MV_XPCS_GLOBAL_CFG_1_REMOTEFAULTDIS_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_1_REMOTEFAULTDIS_OFFS)

#define MV_XPCS_GLOBAL_CFG_1_SIGNALDETDOWNLOCALFAULTGENDIS_OFFS		9
#define MV_XPCS_GLOBAL_CFG_1_SIGNALDETDOWNLOCALFAULTGENDIS_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_1_SIGNALDETDOWNLOCALFAULTGENDIS_OFFS)

#define MV_XPCS_GLOBAL_CFG_1_CJPATGENEN_OFFS		10
#define MV_XPCS_GLOBAL_CFG_1_CJPATGENEN_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_1_CJPATGENEN_OFFS)

#define MV_XPCS_GLOBAL_CFG_1_CRPATGENEN_OFFS		11
#define MV_XPCS_GLOBAL_CFG_1_CRPATGENEN_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_1_CRPATGENEN_OFFS)

#define MV_XPCS_GLOBAL_CFG_1_CJRFORCEDISPEN_OFFS		12
#define MV_XPCS_GLOBAL_CFG_1_CJRFORCEDISPEN_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_CFG_1_CJRFORCEDISPEN_OFFS)

/* Global Fifo Threshold Configuration */
#define MV_XPCS_GLOBAL_FIFO_THR_CFG_REG				(MV_PP3_XPCS_UNIT_OFFSET + 0x0408)
#define MV_XPCS_GLOBAL_FIFO_THR_CFG_DESKEWTIMEOUTLIMIT_OFFS		0
#define MV_XPCS_GLOBAL_FIFO_THR_CFG_DESKEWTIMEOUTLIMIT_MASK    \
		(0x0000000f << MV_XPCS_GLOBAL_FIFO_THR_CFG_DESKEWTIMEOUTLIMIT_OFFS)

#define MV_XPCS_GLOBAL_FIFO_THR_CFG_DESKEWFIFOWRADDRFIX_OFFS		4
#define MV_XPCS_GLOBAL_FIFO_THR_CFG_DESKEWFIFOWRADDRFIX_MASK    \
		(0x0000001f << MV_XPCS_GLOBAL_FIFO_THR_CFG_DESKEWFIFOWRADDRFIX_OFFS)

#define MV_XPCS_GLOBAL_FIFO_THR_CFG_DESKEWFIFORDTH_OFFS		9
#define MV_XPCS_GLOBAL_FIFO_THR_CFG_DESKEWFIFORDTH_MASK    \
		(0x0000000f << MV_XPCS_GLOBAL_FIFO_THR_CFG_DESKEWFIFORDTH_OFFS)

#define MV_XPCS_GLOBAL_FIFO_THR_CFG_PPMFIFORDTH_OFFS		13
#define MV_XPCS_GLOBAL_FIFO_THR_CFG_PPMFIFORDTH_MASK    \
		(0x00000007 << MV_XPCS_GLOBAL_FIFO_THR_CFG_PPMFIFORDTH_OFFS)

#define MV_XPCS_GLOBAL_FIFO_THR_CFG_PPMFIFOEXTRAIDLECHKDIS_OFFS		16
#define MV_XPCS_GLOBAL_FIFO_THR_CFG_PPMFIFOEXTRAIDLECHKDIS_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_FIFO_THR_CFG_PPMFIFOEXTRAIDLECHKDIS_OFFS)

/* Global Max Idle Counter */
#define MV_XPCS_GLOBAL_MAX_IDLE_CNTR_REG			(MV_PP3_XPCS_UNIT_OFFSET + 0x040c)
#define MV_XPCS_GLOBAL_MAX_IDLE_CNTR_MAXIDLECNT_OFFS		0
#define MV_XPCS_GLOBAL_MAX_IDLE_CNTR_MAXIDLECNT_MASK    \
		(0x0000ffff << MV_XPCS_GLOBAL_MAX_IDLE_CNTR_MAXIDLECNT_OFFS)

/* Global Status */
#define MV_XPCS_GLOBAL_STATUS_REG				(MV_PP3_XPCS_UNIT_OFFSET + 0x0410)
#define MV_XPCS_GLOBAL_STATUS_LINKUP_OFFS		0
#define MV_XPCS_GLOBAL_STATUS_LINKUP_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_STATUS_LINKUP_OFFS)

#define MV_XPCS_GLOBAL_STATUS_DESKEWACQUIRED_OFFS		1
#define MV_XPCS_GLOBAL_STATUS_DESKEWACQUIRED_MASK    \
		(0x00000001 << MV_XPCS_GLOBAL_STATUS_DESKEWACQUIRED_OFFS)

/* Global Deskew Error Counter */
#define MV_XPCS_GLOBAL_DESKEW_ERR_CNTR_REG			(MV_PP3_XPCS_UNIT_OFFSET + 0x0420)
#define MV_XPCS_GLOBAL_DESKEW_ERR_CNTR_DESKEWERRCNT_OFFS		0
#define MV_XPCS_GLOBAL_DESKEW_ERR_CNTR_DESKEWERRCNT_MASK    \
		(0x0000ffff << MV_XPCS_GLOBAL_DESKEW_ERR_CNTR_DESKEWERRCNT_OFFS)

/* Tx Packets Counter LSB */
#define MV_XPCS_TX_PCKTS_CNTR_LSB_REG				(MV_PP3_XPCS_UNIT_OFFSET + 0x0430)
#define MV_XPCS_TX_PCKTS_CNTR_LSB_TXPCKTCNTRLSB_OFFS		0
#define MV_XPCS_TX_PCKTS_CNTR_LSB_TXPCKTCNTRLSB_MASK    \
		(0x0000ffff << MV_XPCS_TX_PCKTS_CNTR_LSB_TXPCKTCNTRLSB_OFFS)

/* Tx Packets Counter MSB */
#define MV_XPCS_TX_PCKTS_CNTR_MSB_REG				(MV_PP3_XPCS_UNIT_OFFSET + 0x0434)
#define MV_XPCS_TX_PCKTS_CNTR_MSB_TXPCKTCNTRMSB_OFFS		0
#define MV_XPCS_TX_PCKTS_CNTR_MSB_TXPCKTCNTRMSB_MASK    \
		(0x0000ffff << MV_XPCS_TX_PCKTS_CNTR_MSB_TXPCKTCNTRMSB_OFFS)

/* XPCS per Lane registers */

/* Lane Configuration 0 */
#define MV_XPCS_LANE_CFG_0_REG(lane)			(MV_PP3_XPCS_UNIT_OFFSET + (0x0450 + lane * 0x44))
#define MV_XPCS_LANE_CFG_0_TXRESETIND_OFFS		0
#define MV_XPCS_LANE_CFG_0_TXRESETIND_MASK    \
		(0x00000001 << MV_XPCS_LANE_CFG_0_TXRESETIND_OFFS)

#define MV_XPCS_LANE_CFG_0_RXRESETIND_OFFS		1
#define MV_XPCS_LANE_CFG_0_RXRESETIND_MASK    \
		(0x00000001 << MV_XPCS_LANE_CFG_0_RXRESETIND_OFFS)

#define MV_XPCS_LANE_CFG_0_INDIVIDUALLOOPBACK_OFFS		2
#define MV_XPCS_LANE_CFG_0_INDIVIDUALLOOPBACK_MASK    \
		(0x00000001 << MV_XPCS_LANE_CFG_0_INDIVIDUALLOOPBACK_OFFS)

#define MV_XPCS_LANE_CFG_0_INDIVIDUALLINELOOPBACK_OFFS		3
#define MV_XPCS_LANE_CFG_0_INDIVIDUALLINELOOPBACK_MASK    \
		(0x00000001 << MV_XPCS_LANE_CFG_0_INDIVIDUALLINELOOPBACK_OFFS)

#define MV_XPCS_LANE_CFG_0_TXSMBYPASSEN_OFFS		4
#define MV_XPCS_LANE_CFG_0_TXSMBYPASSEN_MASK    \
		(0x00000001 << MV_XPCS_LANE_CFG_0_TXSMBYPASSEN_OFFS)

#define MV_XPCS_LANE_CFG_0_RXIDLEGENBYPASSEN_OFFS		5
#define MV_XPCS_LANE_CFG_0_RXIDLEGENBYPASSEN_MASK    \
		(0x00000001 << MV_XPCS_LANE_CFG_0_RXIDLEGENBYPASSEN_OFFS)

#define MV_XPCS_LANE_CFG_0_SIGNALDETECTBYPASSEN_OFFS		6
#define MV_XPCS_LANE_CFG_0_SIGNALDETECTBYPASSEN_MASK    \
		(0x00000001 << MV_XPCS_LANE_CFG_0_SIGNALDETECTBYPASSEN_OFFS)

#define MV_XPCS_LANE_CFG_0_CJPATCHKEN_OFFS		7
#define MV_XPCS_LANE_CFG_0_CJPATCHKEN_MASK    \
		(0x00000001 << MV_XPCS_LANE_CFG_0_CJPATCHKEN_OFFS)

#define MV_XPCS_LANE_CFG_0_CRPATCHKEN_OFFS		8
#define MV_XPCS_LANE_CFG_0_CRPATCHKEN_MASK    \
		(0x00000001 << MV_XPCS_LANE_CFG_0_CRPATCHKEN_OFFS)

#define MV_XPCS_LANE_CFG_0_PRBSCHECKEN_OFFS		11
#define MV_XPCS_LANE_CFG_0_PRBSCHECKEN_MASK    \
		(0x00000001 << MV_XPCS_LANE_CFG_0_PRBSCHECKEN_OFFS)

#define MV_XPCS_LANE_CFG_0_TESTGENEN_OFFS		12
#define MV_XPCS_LANE_CFG_0_TESTGENEN_MASK    \
		(0x00000001 << MV_XPCS_LANE_CFG_0_TESTGENEN_OFFS)

#define MV_XPCS_LANE_CFG_0_TESTMODE_OFFS		13
#define MV_XPCS_LANE_CFG_0_TESTMODE_MASK    \
		(0x00000003 << MV_XPCS_LANE_CFG_0_TESTMODE_OFFS)

#define MV_XPCS_LANE_CFG_0_TESTMODEEN_OFFS		15
#define MV_XPCS_LANE_CFG_0_TESTMODEEN_MASK    \
		(0x00000001 << MV_XPCS_LANE_CFG_0_TESTMODEEN_OFFS)

/* Lane Configuration 1 */
#define MV_XPCS_LANE_CFG_1_REG(lane)			(MV_PP3_XPCS_UNIT_OFFSET + (0x0454 + lane * 0x44))
#define MV_XPCS_LANE_CFG_1_LED0CTRL_OFFS		0
#define MV_XPCS_LANE_CFG_1_LED0CTRL_MASK    \
		(0x0000000f << MV_XPCS_LANE_CFG_1_LED0CTRL_OFFS)

#define MV_XPCS_LANE_CFG_1_LED1CTRL_OFFS		4
#define MV_XPCS_LANE_CFG_1_LED1CTRL_MASK    \
		(0x0000000f << MV_XPCS_LANE_CFG_1_LED1CTRL_OFFS)

#define MV_XPCS_LANE_CFG_1_TXSWPSEL_OFFS		8
#define MV_XPCS_LANE_CFG_1_TXSWPSEL_MASK    \
		(0x00000007 << MV_XPCS_LANE_CFG_1_TXSWPSEL_OFFS)

#define MV_XPCS_LANE_CFG_1_RXSWPSEL_OFFS		11
#define MV_XPCS_LANE_CFG_1_RXSWPSEL_MASK    \
		(0x00000007 << MV_XPCS_LANE_CFG_1_RXSWPSEL_OFFS)

/* Lane Status */
#define MV_XPCS_LANE_STATUS_REG(lane)			(MV_PP3_XPCS_UNIT_OFFSET + (0x045c + lane * 0x44))
#define MV_XPCS_LANE_STATUS_PRBSCHECKLOCKED_OFFS		0
#define MV_XPCS_LANE_STATUS_PRBSCHECKLOCKED_MASK    \
		(0x00000001 << MV_XPCS_LANE_STATUS_PRBSCHECKLOCKED_OFFS)

#define MV_XPCS_LANE_STATUS_PLLLOCKED_OFFS		1
#define MV_XPCS_LANE_STATUS_PLLLOCKED_MASK    \
		(0x00000001 << MV_XPCS_LANE_STATUS_PLLLOCKED_OFFS)

#define MV_XPCS_LANE_STATUS_SIGNALDETECTED_OFFS		2
#define MV_XPCS_LANE_STATUS_SIGNALDETECTED_MASK    \
		(0x00000001 << MV_XPCS_LANE_STATUS_SIGNALDETECTED_OFFS)

#define MV_XPCS_LANE_STATUS_COMMADETECTED_OFFS		3
#define MV_XPCS_LANE_STATUS_COMMADETECTED_MASK    \
		(0x00000001 << MV_XPCS_LANE_STATUS_COMMADETECTED_OFFS)

#define MV_XPCS_LANE_STATUS_SYNCOK_OFFS		4
#define MV_XPCS_LANE_STATUS_SYNCOK_MASK    \
		(0x00000001 << MV_XPCS_LANE_STATUS_SYNCOK_OFFS)

/* Symbol Error Counter */
#define MV_XPCS_SYMBOL_ERR_CNTR_REG(lane)		(MV_PP3_XPCS_UNIT_OFFSET + (0x0468 + lane * 0x44))
#define MV_XPCS_SYMBOL_ERR_CNTR_SYMBOLERRCNT_OFFS	0
#define MV_XPCS_SYMBOL_ERR_CNTR_SYMBOLERRCNT_MASK    \
		(0x0000ffff << MV_XPCS_SYMBOL_ERR_CNTR_SYMBOLERRCNT_OFFS)

/* Disparity Error Counter */
#define MV_XPCS_DISPARITY_ERR_CNTR_REG(lane)		(MV_PP3_XPCS_UNIT_OFFSET + (0x046c + lane * 0x44))
#define MV_XPCS_DISPARITY_ERR_CNTR_DISPARITYERRCNT_OFFS		0
#define MV_XPCS_DISPARITY_ERR_CNTR_DISPARITYERRCNT_MASK    \
		(0x0000ffff << MV_XPCS_DISPARITY_ERR_CNTR_DISPARITYERRCNT_OFFS)

/* Prbs Error Counter */
#define MV_XPCS_PRBS_ERR_CNTR_REG(lane)			(MV_PP3_XPCS_UNIT_OFFSET + (0x0470 + lane * 0x44))
#define MV_XPCS_PRBS_ERR_CNTR_PRBSERRCNT_OFFS		0
#define MV_XPCS_PRBS_ERR_CNTR_PRBSERRCNT_MASK    \
		(0x0000ffff << MV_XPCS_PRBS_ERR_CNTR_PRBSERRCNT_OFFS)

/* Rx Packets Counter LSB */
#define MV_XPCS_RX_PCKTS_CNTR_LSB_REG(lane)		(MV_PP3_XPCS_UNIT_OFFSET + (0x0474 + lane * 0x44))
#define MV_XPCS_RX_PCKTS_CNTR_LSB_RXPCKTCNTRLSB_OFFS		0
#define MV_XPCS_RX_PCKTS_CNTR_LSB_RXPCKTCNTRLSB_MASK    \
		(0x0000ffff << MV_XPCS_RX_PCKTS_CNTR_LSB_RXPCKTCNTRLSB_OFFS)

/* Rx Packets Counter MSB */
#define MV_XPCS_RX_PCKTS_CNTR_MSB_REG(lane)		(MV_PP3_XPCS_UNIT_OFFSET + (0x0478 + lane * 0x44))
#define MV_XPCS_RX_PCKTS_CNTR_MSB_RXPCKTCNTRMSB_OFFS	0
#define MV_XPCS_RX_PCKTS_CNTR_MSB_RXPCKTCNTRMSB_MASK    \
		(0x0000ffff << MV_XPCS_RX_PCKTS_CNTR_MSB_RXPCKTCNTRMSB_OFFS)

/* Rx Bad Packets Counter LSB */
#define MV_XPCS_RX_BAD_PCKTS_CNTR_LSB_REG(lane)			(MV_PP3_XPCS_UNIT_OFFSET + (0x047c + lane * 0x44))
#define MV_XPCS_RX_BAD_PCKTS_CNTR_LSB_RXBADPCKTCNTRLSB_OFFS		0
#define MV_XPCS_RX_BAD_PCKTS_CNTR_LSB_RXBADPCKTCNTRLSB_MASK    \
		(0x0000ffff << MV_XPCS_RX_BAD_PCKTS_CNTR_LSB_RXBADPCKTCNTRLSB_OFFS)

/* Rx Bad Packets Counter MSB */
#define MV_XPCS_RX_BAD_PCKTS_CNTR_MSB_REG(lane)			(MV_PP3_XPCS_UNIT_OFFSET + (0x0480 + lane * 0x44))
#define MV_XPCS_RX_BAD_PCKTS_CNTR_MSB_RXBADPCKTCNTRMSB_OFFS		0
#define MV_XPCS_RX_BAD_PCKTS_CNTR_MSB_RXBADPCKTCNTRMSB_MASK    \
		(0x0000ffff << MV_XPCS_RX_BAD_PCKTS_CNTR_MSB_RXBADPCKTCNTRMSB_OFFS)

/* Cyclic Data 0 */
#define MV_XPCS_CYCLIC_DATA_0_REG(lane)				(MV_PP3_XPCS_UNIT_OFFSET + (0x0484 + lane * 0x44))
#define MV_XPCS_CYCLIC_DATA_0_CYCLICDATA0_OFFS		0
#define MV_XPCS_CYCLIC_DATA_0_CYCLICDATA0_MASK    \
		(0x000003ff << MV_XPCS_CYCLIC_DATA_0_CYCLICDATA0_OFFS)

/* Cyclic Data 1 */
#define MV_XPCS_CYCLIC_DATA_1_REG(lane)				(MV_PP3_XPCS_UNIT_OFFSET + (0x0488 + lane * 0x44))
#define MV_XPCS_CYCLIC_DATA_1_CYCLICDATA1_OFFS		0
#define MV_XPCS_CYCLIC_DATA_1_CYCLICDATA1_MASK    \
		(0x000003ff << MV_XPCS_CYCLIC_DATA_1_CYCLICDATA1_OFFS)

/* Cyclic Data 2 */
#define MV_XPCS_CYCLIC_DATA_2_REG(lane)				(MV_PP3_XPCS_UNIT_OFFSET + (0x048c + lane * 0x44))
#define MV_XPCS_CYCLIC_DATA_2_CYCLICDATA2_OFFS		0
#define MV_XPCS_CYCLIC_DATA_2_CYCLICDATA2_MASK    \
		(0x000003ff << MV_XPCS_CYCLIC_DATA_2_CYCLICDATA2_OFFS)

/* Cyclic Data 3 */
#define MV_XPCS_CYCLIC_DATA_3_REG(lane)				(MV_PP3_XPCS_UNIT_OFFSET + (0x0490 + lane * 0x44))
#define MV_XPCS_CYCLIC_DATA_3_CYCLICDATA3_OFFS		0
#define MV_XPCS_CYCLIC_DATA_3_CYCLICDATA3_MASK    \
		(0x000003ff << MV_XPCS_CYCLIC_DATA_3_CYCLICDATA3_OFFS)

#endif /* __mv_xpcs_regs_h__ */
#endif /* CONFIG_SYNO_LSP_ARMADA_16_12 */
