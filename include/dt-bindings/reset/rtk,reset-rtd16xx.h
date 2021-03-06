#ifndef __DT_BINDINGS_RTK_RESET_RTD16XX_H
#define __DT_BINDINGS_RTK_RESET_RTD16XX_H

/* 0x98000000 */
#define RSTN_MISC               0
#define RSTN_DIP                2
#define RSTN_GSPI               4
#define RSTN_SDS                6
#define RSTN_SDS_REG            8
#define RSTN_SDS_PHY           10
#define RSTN_VE1               12
#define RSTN_VE2               14
#define RSTN_R2RDSC            16
#define RSTN_RSA               18
#define RSTN_GPU               20
#define RSTN_DC_PHY            22
#define RSTN_DCPHY_CRT         24
#define RSTN_LSADC             26
#define RSTN_SE                28
#define RSTN_HSE               30

/* 0x98000004 */
#define RSTN_JPEG               0
#define RSTN_SD                 2
#define RSTN_EMMC               4
#define RSTN_SDIO               6
#define RSTN_PCR_CNT            8
#define RSTN_PCIE0_STITCH      10
#define RSTN_PCIE0_PHY         12
#define RSTN_PCIE0             14
#define RSTN_PCIE0_CORE        16
#define RSTN_PCIE0_POWER       18
#define RSTN_PCIE0_NONSTITCH   20
#define RSTN_PCIE0_PHY_MDIO    22
#define RSTN_PCIE0_SGMII_MDIO  24
#define RSTN_UR2               26
#define RSTN_UR1               28
#define RSTN_MISC_SC0          30

/* 0x98000008 */
#define RSTN_AE                 0
#define RSTN_CABLERX            2
#define RSTN_MD                 4
#define RSTN_TP                 6
#define RSTN_NF                 8
#define RSTN_MISC_SC1          10
#define RSTN_I2C_3             12
#define RSTN_FAN               14
#define RSTN_TVE               16
#define RSTN_AIO               18
#define RSTN_VO                20
#define RSTN_MIPI              22
#define RSTN_HDMIRX            24
#define RSTN_HDMIRX_WRAP       26
#define RSTN_HDMI              28
#define RSTN_DISP              30

/* 0x9800000C */
#define RSTN_SATA_PHY_POW1      0
#define RSTN_SATA_PHY_POW0      2
#define RSTN_SATA_MDIO1         4
#define RSTN_SATA_MDIO0         6
#define RSTN_SATA_WRAP          8
#define RSTN_SATA_MAC_P1       10
#define RSTN_SATA_MAC_P0       12
#define RSTN_SATA_MAC_COM      14
#define RSTN_PCIE1_STITCH      16
#define RSTN_PCIE1_PHY         18
#define RSTN_PCIE1             20
#define RSTN_PCIE1_CORE        22
#define RSTN_PCIE1_POWER       24
#define RSTN_PCIE1_NONSTITCH   26
#define RSTN_PCIE1_PHY_MDIO    28
#define RSTN_HDMITOP           30

/* 0x98000014 */
#define RSTN_HSE_2              6
#define RSTN_R2RDSC_2           8
#define RSTN_EMMC_2            10
#define RSTN_NF_2              12
#define RSTN_MD_2              14
#define RSTN_TPB_2             24
#define RSTN_TP_2              26
#define RSTN_MIPI_2            28

/* MUX reset */
#define RSTN_MUX_HSE            0
#define RSTN_MUX_R2RDSC         1
#define RSTN_MUX_EMMC           2
#define RSTN_MUX_NF             3
#define RSTN_MUX_MD             4
#define RSTN_MUX_TPB            5
#define RSTN_MUX_TP             6
#define RSTN_MUX_MIPI           7

/* 0x98000068 */
#define RSTN_TBP                0
#define RSTN_I2C_4              2
#define RSTN_I2C_5              4
#define RSTN_TSIO               6
#define RSTN_VE3                8
#define RSTN_EDP               10

/* 0x98007088 */
#define RSTN_VFD                0
#define RSTN_IR                 1
#define RSTN_CEC0               2
#define RSTN_CEC1               3
#define RSTN_DP                 4
#define RSTN_CBUSTX             5
#define RSTN_CBUSRX             6
#define RSTN_EFUSE              7
#define RSTN_UR0                8
#define RSTN_GMAC               9
#define RSTN_GPHY              10
#define RSTN_I2C_0             11
#define RSTN_I2C_1             12
#define RSTN_CBUS              13
#define RSTN_USB_DRD           14
#define RSTN_USB_HOST          15
#define RSTN_USB_PHY_0         16
#define RSTN_USB_PHY_1         17
#define RSTN_USB_PHY_2         18
#define RSTN_USB               19
#define RSTN_TYPE_C            20
#define RSTN_USB_U3_HOST       21
#define RSTN_USB3_PHY0_POW     22
#define RSTN_USB3_P0_MDIO      23
#define RSTN_USB3_PHY1_POW     24
#define RSTN_USB3_P1_MDIO      25

#endif
