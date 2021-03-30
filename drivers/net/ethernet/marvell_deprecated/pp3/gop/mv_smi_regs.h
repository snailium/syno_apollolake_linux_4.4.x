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
#ifndef __mv_smi_regs_h__
#define __mv_smi_regs_h__

/* includes */

/* SMI_MANAGEMENT Register */
#define MV_SMI_MANAGEMENT_REG					(0x5000000)

#define MV_SMI_MANAGEMENT_BUSY_OFFS						28
#define MV_SMI_MANAGEMENT_BUSY_MASK						\
	(0x1 << MV_SMI_MANAGEMENT_BUSY_OFFS)
#define MV_SMI_MANAGEMENT_READ_VALID_OFFS					27
#define MV_SMI_MANAGEMENT_READ_VALID_MASK					\
	(0x1 << MV_SMI_MANAGEMENT_READ_VALID_OFFS)
#define MV_SMI_MANAGEMENT_OPCODE_OFFS					26
#define MV_SMI_MANAGEMENT_OPCODE_MASK					\
	(0x1 << MV_SMI_MANAGEMENT_OPCODE_OFFS)
#define MV_SMI_MANAGEMENT_REGAD_OFFS					21
#define MV_SMI_MANAGEMENT_REGAD_MASK					\
	(0x1F << MV_SMI_MANAGEMENT_REGAD_OFFS)
#define MV_SMI_MANAGEMENT_PHYAD_OFFS					16
#define MV_SMI_MANAGEMENT_PHYAD_MASK					\
	(0x1F << MV_SMI_MANAGEMENT_PHYAD_OFFS)
#define MV_SMI_MANAGEMENT_DATA_OFFS						0
#define MV_SMI_MANAGEMENT_DATA_MASK						\
	(0xFFFF << MV_SMI_MANAGEMENT_DATA_OFFS)

/* SMI_MISC_CFG Register */
#define MV_SMI_MISC_CFG_REG							(0x5000004)

#define MV_SMI_MISC_CFG_SMI_ACCELERATE_OFFS					0
#define MV_SMI_MISC_CFG_SMI_ACCELERATE_MASK					\
	(0x1 << MV_SMI_MISC_CFG_SMI_ACCELERATE_OFFS)
#define MV_SMI_MISC_CFG_SMI_FASTMDC_OFFS					1
#define MV_SMI_MISC_CFG_SMI_FASTMDC_MASK					\
	(0x1 << MV_SMI_MISC_CFG_SMI_FASTMDC_OFFS)
#define MV_SMI_MISC_CFG_FAST_MDC_DIVISION_SELECTOR_OFFS			2
#define MV_SMI_MISC_CFG_FAST_MDC_DIVISION_SELECTOR_MASK			\
	(0x3 << MV_SMI_MISC_CFG_FAST_MDC_DIVISION_SELECTOR_OFFS)
#define MV_SMI_MISC_CFG_ENABLE_MDIO_OUT_LATENCY_OFFS			4
#define MV_SMI_MISC_CFG_ENABLE_MDIO_OUT_LATENCY_MASK			\
	(0x1 << MV_SMI_MISC_CFG_ENABLE_MDIO_OUT_LATENCY_OFFS)
#define MV_SMI_MISC_CFG_AUTOPOLLNUMOFPORTS_OFFS				5
#define MV_SMI_MISC_CFG_AUTOPOLLNUMOFPORTS_MASK				\
	(0x1F << MV_SMI_MISC_CFG_AUTOPOLLNUMOFPORTS_OFFS)
#define MV_SMI_MISC_CFG_ENABLE_POLLING_OFFS					10
#define MV_SMI_MISC_CFG_ENABLE_POLLING_MASK					\
	(0x1 << MV_SMI_MISC_CFG_ENABLE_POLLING_OFFS)
#define MV_SMI_MISC_CFG_INVERT_MDC_OFFS					11
#define MV_SMI_MISC_CFG_INVERT_MDC_MASK					\
	(0x1 << MV_SMI_MISC_CFG_INVERT_MDC_OFFS)

/* PHY_AN_CFG Register */
#define MV_SMI_PHY_AN_CFG_REG							(0x5000008)

#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT0_OFFS					0
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT0_MASK					\
	(0x1 << MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT0_OFFS)
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT1_OFFS					1
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT1_MASK					\
	(0x1 << MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT1_OFFS)
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT2_OFFS					2
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT2_MASK					\
	(0x1 << MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT2_OFFS)
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT3_OFFS					3
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT3_MASK					\
	(0x1 << MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT3_OFFS)
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT4_OFFS					4
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT4_MASK					\
	(0x1 << MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT4_OFFS)
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT5_OFFS					5
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT5_MASK					\
	(0x1 << MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT5_OFFS)
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT6_OFFS					6
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT6_MASK					\
	(0x1 << MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT6_OFFS)
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT7_OFFS					7
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT7_MASK					\
	(0x1 << MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT7_OFFS)
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT8_OFFS					8
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT8_MASK					\
	(0x1 << MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT8_OFFS)
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT9_OFFS					9
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT9_MASK					\
	(0x1 << MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT9_OFFS)
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT10_OFFS				10
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT10_MASK				\
	(0x1 << MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT10_OFFS)
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT11_OFFS				11
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT11_MASK				\
	(0x1 << MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT11_OFFS)
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT12_OFFS				12
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT12_MASK				\
	(0x1 << MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT12_OFFS)
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT13_OFFS				13
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT13_MASK				\
	(0x1 << MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT13_OFFS)
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT14_OFFS				14
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT14_MASK				\
	(0x1 << MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT14_OFFS)
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT15_OFFS				15
#define MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT15_MASK				\
	(0x1 << MV_SMI_PHY_AN_CFG_AUTOMEDIA_SELECTEN_PORT15_OFFS)
#define MV_SMI_PHY_AN_CFG_SKIPSWRESET_SMI_OFFS					16
#define MV_SMI_PHY_AN_CFG_SKIPSWRESET_SMI_MASK					\
	(0x1 << MV_SMI_PHY_AN_CFG_SKIPSWRESET_SMI_OFFS)
#define MV_SMI_PHY_AN_CFG_STOP_AUTONEGSMI_OFFS					17
#define MV_SMI_PHY_AN_CFG_STOP_AUTONEGSMI_MASK					\
	(0x1 << MV_SMI_PHY_AN_CFG_STOP_AUTONEGSMI_OFFS)
#define MV_SMI_PHY_AN_CFG_MASTERSMI_OFFS					18
#define MV_SMI_PHY_AN_CFG_MASTERSMI_MASK					\
	(0x1 << MV_SMI_PHY_AN_CFG_MASTERSMI_OFFS)
#define MV_SMI_PHY_AN_CFG_SGMIIINBANDFCEN_OFFS					19
#define MV_SMI_PHY_AN_CFG_SGMIIINBANDFCEN_MASK					\
	(0x1 << MV_SMI_PHY_AN_CFG_SGMIIINBANDFCEN_OFFS)
#define MV_SMI_PHY_AN_CFG_FCADVSETFIBER_OFFS					20
#define MV_SMI_PHY_AN_CFG_FCADVSETFIBER_MASK					\
	(0x1 << MV_SMI_PHY_AN_CFG_FCADVSETFIBER_OFFS)

/* PHY_ADDRESS_REGISTER0 Register */
#define MV_SMI_PHY_ADDRESS_REG(n)						(0x500000C + 0x4*n)
#define MV_SMI_PHY_ADDRESS_PHYAD_OFFS						0
#define MV_SMI_PHY_ADDRESS_PHYAD_MASK						\
	(0x1F << MV_SMI_PHY_ADDRESS_PHYAD_OFFS)

#endif /* __mv_smi_regs_h__ */
#endif /* CONFIG_SYNO_LSP_ARMADA_16_12 */
