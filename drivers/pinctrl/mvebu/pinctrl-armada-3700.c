#if defined(CONFIG_SYNO_LSP_ARMADA_16_12)
/*
 * Marvell Armada 3700 pinctrl driver based on mvebu pinctrl core
 *
 * Copyright (C) 2016 Marvell
 *
 * Terry Zhou <bjzhou@marvell.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#include <linux/syscore_ops.h>
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
#include <linux/pinctrl/pinctrl.h>
#include "pinctrl-mvebu.h"

/*
 * Armada3700 provides dual modes for most MPPs,
 * either gpio or the specific function.
 * But some MPPs can support up to 3 modes combined
 * with selecting the significant bits in the MPP's
 * register.
 */
#define ARMADA_3700_MPP_MAX_CONFIG_NUM	3

enum armada_3700_bridge {
	I_NORTHBRIDGE	= 0,
	I_SOUTHBRIDGE,
	I_MAXCONTROLLER,
};

struct  armada_3700_mpp_setting_bitmap {
	/*
	 * The mask is used for read/write MPP register bits of the function seletion.
	 * For the mpps dedicated to gpio function which is not controlled by any MPP
	 * register bits, the mask should be set to '0'.
	 */
	unsigned int mask;
	unsigned int config_num;	/* mpp configuration modes number */
	/*
	 * This array maps the MPP's modes into the relevant MPP's register bits. It
	 * is indexed by MPP's modes, which is defined in armada_3700_nb/sb_mpp_modes.
	 */
	unsigned int configs[ARMADA_3700_MPP_MAX_CONFIG_NUM];
};

static void __iomem *mpp_base[I_MAXCONTROLLER];/* north & south bridge mpp base */
#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
static unsigned int mpp_saved_regs[I_MAXCONTROLLER];/* north & south bridge mpp status, for suspend/resume usage */
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */

struct  armada_3700_mpp_conf {
	struct mvebu_pinctrl_soc_info *soc_info;
	struct mvebu_mpp_mode *modes;
	int nmodes;
	struct armada_3700_mpp_setting_bitmap *bitmap;
	int nbitmaps;
	enum armada_3700_bridge index;
};

struct armada_3700_mpp_conf *a3700_mpp_conf_ptr[I_MAXCONTROLLER];

static int armada_3700_mpp_ctrl_get(unsigned pid,
				    struct armada_3700_mpp_setting_bitmap mpp_setting_bitmap[],
				    void __iomem *mpp_base_addr,
				    unsigned long *config)
{
	unsigned int setting_num;
	unsigned int reg, mask, i;

	setting_num = mpp_setting_bitmap[pid].config_num;
	mask = mpp_setting_bitmap[pid].mask;
	if (!mask) {
		/* The empty mask means the pin has a dedicated function. */
		*config = 0;
		return 0;
	}

	reg = readl(mpp_base_addr) & mask;
	for (i = 0; i < setting_num; i++)
		if (reg == mpp_setting_bitmap[pid].configs[i])
			break;

	if (i == setting_num) {
		pr_err("Unknown config for bridge pin %u, mpp reg value(0x%08x)!\n", pid, reg);
		return -EFAULT;
	}

	*config = i;
	pr_debug("%s %d pid %d config %lu mask 0x%x reg 0x%x\n", __func__, __LINE__, pid, *config, mask, reg);
	return 0;
}

static int armada_3700_mpp_ctrl_set(unsigned pid,
				    struct armada_3700_mpp_setting_bitmap mpp_setting_bitmap[],
				    void __iomem *mpp_base_addr,
				    unsigned long config)
{
	unsigned int setting_num;
	unsigned int reg, mask, val;

	/* check whether the config value is valid */
	setting_num = mpp_setting_bitmap[pid].config_num;
	if (config >= setting_num) {
		pr_err("For bridge pin %u, invalid config %lu!\n", pid, config);
		return -EINVAL;
	}

	mask = mpp_setting_bitmap[pid].mask;
	if (!mask)
		return 0;
	val = mpp_setting_bitmap[pid].configs[config];
	reg = readl(mpp_base_addr) & ~mask;
	writel(reg | val, mpp_base_addr);

	pr_debug("%s %d pid %d config %lu reg 0x%x\n", __func__, __LINE__, pid, config, reg | val);
	return 0;
}

/* North bridge pin-ctl for Z1 */
static struct mvebu_mpp_mode armada_3700_z1_nb_mpp_modes[] = {
	MPP_MODE(0,
	   MPP_FUNCTION(0x0, "i2c1", "sck"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(1,
	   MPP_FUNCTION(0x0, "i2c1", "sda"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(2,
	   MPP_FUNCTION(0x0, "i2c2", "sck"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(3,
	   MPP_FUNCTION(0x0, "i2c2", "sda"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(4,
	   MPP_FUNCTION(0x0, "1wire", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(5,
	   MPP_FUNCTION(0x0, "gpio", NULL)),
	MPP_MODE(6,
	   MPP_FUNCTION(0x0, "pmic0", "slp-out"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(7,
	   MPP_FUNCTION(0x0, "pmic1", "slp-out"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(8,
	   MPP_FUNCTION(0x0, "sdio", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(9,
	   MPP_FUNCTION(0x0, "sdio", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "uart2", "rts")),
	MPP_MODE(10,
	   MPP_FUNCTION(0x0, "sdio", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "uart2", "cts")),
	MPP_MODE(11,
	   MPP_FUNCTION(0x0, "pwm0", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "led0-od", NULL)),
	MPP_MODE(12,
	   MPP_FUNCTION(0x0, "pwm1", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "led1-od", NULL)),
	MPP_MODE(13,
	   MPP_FUNCTION(0x0, "pwm2", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "led2-od", NULL)),
	MPP_MODE(14,
	   MPP_FUNCTION(0x0, "pwm3", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "led3-od", NULL)),
	MPP_MODE(15,
	   MPP_FUNCTION(0x0, "spi-quad", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(16,
	   MPP_FUNCTION(0x0, "spi-quad", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(17,
	   MPP_FUNCTION(0x0, "spi-cs1", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(18,
	   MPP_FUNCTION(0x0, "spi-cs2", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "uart2", "tx")),
	MPP_MODE(19,
	   MPP_FUNCTION(0x0, "spi-cs3", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "uart2", "rx")),
	MPP_MODE(20,
	   MPP_FUNCTION(0x0, "jtag", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(21,
	   MPP_FUNCTION(0x0, "jtag", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(22,
	   MPP_FUNCTION(0x0, "jtag", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(23,
	   MPP_FUNCTION(0x0, "jtag", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(24,
	   MPP_FUNCTION(0x0, "jtag", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(25,
	   MPP_FUNCTION(0x0, "uart1", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(26,
	   MPP_FUNCTION(0x0, "uart1", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(27,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(28,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(29,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(30,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(31,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(32,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(33,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(34,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(35,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	/*
	 * The mpp36 is not a GPIO pin exposed to the user.
	 * It is dedicated to spi function.
	 */
	MPP_MODE(36,
	   MPP_FUNCTION(0x0, "spi", NULL)),
};

/* North bridge pins' configs setting bitmaps for Z1, this array idx is north bridge pin id. */
static struct armada_3700_mpp_setting_bitmap armada_3700_z1_nb_mpp_bitmap[] = {
	/* mask			config_num	configs */
	{BIT(10),		2,		{0, BIT(10)}			},	/* pin 0 */
	{BIT(10),		2,		{0, BIT(10)}			},	/* pin 1 */
	{BIT(9),		2,		{0, BIT(9)}			},	/* pin 2 */
	{BIT(9),		2,		{0, BIT(9)}			},	/* pin 3 */
	{BIT(16),		2,		{0, BIT(16)}			},	/* pin 4 */
	{0,			1,		{0}				},	/* pin 5 */
	{BIT(8),		2,		{0, BIT(8)}			},	/* pin 6 */
	{BIT(7),		2,		{0, BIT(7)}			},	/* pin 7 */
	{BIT(1),		2,		{0, BIT(1)}			},	/* pin 8 */
	{BIT(1) | BIT(19),	3,		{0, BIT(1), BIT(1) | BIT(19)}	},	/* pin 9 */
	{BIT(1) | BIT(19),	3,		{0, BIT(1), BIT(1) | BIT(19)}	},	/* pin 10 */
	{BIT(3) | BIT(20),	3,		{0, BIT(3), BIT(3) | BIT(20)}	},	/* pin 11 */
	{BIT(4) | BIT(21),	3,		{0, BIT(4), BIT(4) | BIT(21)}	},	/* pin 12 */
	{BIT(5) | BIT(22),	3,		{0, BIT(5), BIT(5) | BIT(22)}	},	/* pin 13 */
	{BIT(6) | BIT(23),	3,		{0, BIT(6), BIT(6) | BIT(23)}	},	/* pin 14 */
	{BIT(18),		2,		{0, BIT(18)}			},	/* pin 15 */
	{BIT(18),		2,		{0, BIT(18)}			},	/* pin 16 */
	{BIT(12),		2,		{0, BIT(12)}			},	/* pin 17 */
	{BIT(13) | BIT(19),	3,		{0, BIT(13), BIT(19)}		},	/* pin 18 */
	{BIT(14) | BIT(19),	3,		{0, BIT(14), BIT(19)}		},	/* pin 19 */
	{BIT(0),		2,		{0, BIT(0)}			},	/* pin 20 */
	{BIT(0),		2,		{0, BIT(0)}			},	/* pin 21 */
	{BIT(0),		2,		{0, BIT(0)}			},	/* pin 22 */
	{BIT(0),		2,		{0, BIT(0)}			},	/* pin 23 */
	{BIT(0),		2,		{0, BIT(0)}			},	/* pin 24 */
	{BIT(17),		2,		{0, BIT(17)}			},	/* pin 25 */
	{BIT(17),		2,		{0, BIT(17)}			},	/* pin 26 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 27 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 28 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 29 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 30 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 31 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 32 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 33 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 34 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 35 */
	{BIT(15),		1,		{0}				},	/* pin 36 */
};

/* North bridge pin-ctl for A0 */
static struct mvebu_mpp_mode armada_3700_nb_mpp_modes[] = {
	MPP_MODE(0,
	   MPP_FUNCTION(0x0, "i2c1", "sck"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(1,
	   MPP_FUNCTION(0x0, "i2c1", "sda"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(2,
	   MPP_FUNCTION(0x0, "i2c2", "sck"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(3,
	   MPP_FUNCTION(0x0, "i2c2", "sda"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(4,
	   MPP_FUNCTION(0x0, "1wire", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(5,
	   MPP_FUNCTION(0x0, "gpio", NULL)),
	MPP_MODE(6,
	   MPP_FUNCTION(0x0, "pmic0", "slp-out"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(7,
	   MPP_FUNCTION(0x0, "pmic1", "slp-out"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(8,
	   MPP_FUNCTION(0x0, "sdio", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(9,
	   MPP_FUNCTION(0x0, "sdio", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "uart2", "rts")),
	MPP_MODE(10,
	   MPP_FUNCTION(0x0, "sdio", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "uart2", "cts")),
	MPP_MODE(11,
	   MPP_FUNCTION(0x0, "pwm0", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "led0-od", NULL)),
	MPP_MODE(12,
	   MPP_FUNCTION(0x0, "pwm1", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "led1-od", NULL)),
	MPP_MODE(13,
	   MPP_FUNCTION(0x0, "pwm2", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "led2-od", NULL)),
	MPP_MODE(14,
	   MPP_FUNCTION(0x0, "pwm3", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "led3-od", NULL)),
	MPP_MODE(15,
	   MPP_FUNCTION(0x0, "spi-quad", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(16,
	   MPP_FUNCTION(0x0, "spi-quad", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(17,
	   MPP_FUNCTION(0x0, "spi-cs1", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(18,
	   MPP_FUNCTION(0x0, "spi-cs2", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "uart2", "tx")),
	MPP_MODE(19,
	   MPP_FUNCTION(0x0, "spi-cs3", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "uart2", "rx")),
	MPP_MODE(20,
	   MPP_FUNCTION(0x0, "jtag", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(21,
	   MPP_FUNCTION(0x0, "jtag", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(22,
	   MPP_FUNCTION(0x0, "jtag", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(23,
	   MPP_FUNCTION(0x0, "jtag", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(24,
	   MPP_FUNCTION(0x0, "jtag", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(25,
	   MPP_FUNCTION(0x0, "uart1", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(26,
	   MPP_FUNCTION(0x0, "uart1", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(27,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(28,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(29,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(30,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(31,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(32,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(33,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(34,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(35,
	   MPP_FUNCTION(0x0, "mmc", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	/*
	 * The mpp36 is not a GPIO pin exposed to the user.
	 * It is dedicated to spi function.
	 */
	MPP_MODE(36,
	   MPP_FUNCTION(0x0, "spi", NULL)),
};

/* North bridge pins' configs setting bitmaps for A0, this array idx is north bridge pin id. */
static struct armada_3700_mpp_setting_bitmap armada_3700_nb_mpp_bitmap[] = {
	/* mask			config_num	configs */
	{BIT(10),		2,		{0, BIT(10)}			},	/* pin 0 */
	{BIT(10),		2,		{0, BIT(10)}			},	/* pin 1 */
	{BIT(9),		2,		{0, BIT(9)}			},	/* pin 2 */
	{BIT(9),		2,		{0, BIT(9)}			},	/* pin 3 */
	{BIT(16),		2,		{0, BIT(16)}			},	/* pin 4 */
	{0,			1,		{0}				},	/* pin 5 */
	{BIT(8),		2,		{0, BIT(8)}			},	/* pin 6 */
	{BIT(7),		2,		{0, BIT(7)}			},	/* pin 7 */
	{BIT(1),		2,		{0, BIT(1)}			},	/* pin 8 */
	{BIT(1) | BIT(19),	3,		{0, BIT(1), BIT(1) | BIT(19)}	},	/* pin 9 */
	{BIT(1) | BIT(19),	3,		{0, BIT(1), BIT(1) | BIT(19)}	},	/* pin 10 */
	{BIT(3) | BIT(20),	3,		{0, BIT(3), BIT(3) | BIT(20)}	},	/* pin 11 */
	{BIT(4) | BIT(21),	3,		{0, BIT(4), BIT(4) | BIT(21)}	},	/* pin 12 */
	{BIT(5) | BIT(22),	3,		{0, BIT(5), BIT(5) | BIT(22)}	},	/* pin 13 */
	{BIT(6) | BIT(23),	3,		{0, BIT(6), BIT(6) | BIT(23)}	},	/* pin 14 */
	{BIT(18),		2,		{0, BIT(18)}			},	/* pin 15 */
	{BIT(18),		2,		{0, BIT(18)}			},	/* pin 16 */
	{BIT(12),		2,		{0, BIT(12)}			},	/* pin 17 */
	{BIT(13) | BIT(19),	3,		{0, BIT(13), BIT(19)}		},	/* pin 18 */
	{BIT(14) | BIT(19),	3,		{0, BIT(14), BIT(19)}		},	/* pin 19 */
	{BIT(0),		2,		{0, BIT(0)}			},	/* pin 20 */
	{BIT(0),		2,		{0, BIT(0)}			},	/* pin 21 */
	{BIT(0),		2,		{0, BIT(0)}			},	/* pin 22 */
	{BIT(0),		2,		{0, BIT(0)}			},	/* pin 23 */
	{BIT(0),		2,		{0, BIT(0)}			},	/* pin 24 */
	{BIT(17),		2,		{0, BIT(17)}			},	/* pin 25 */
	{BIT(17),		2,		{0, BIT(17)}			},	/* pin 26 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 27 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 28 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 29 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 30 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 31 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 32 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 33 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 34 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 35 */
	{BIT(15),		1,		{0}				},	/* pin 36 */
};

static int armada_3700_nb_mpp_ctrl_get(unsigned pid, unsigned long *config)
{
	int rc;

	if (pid > a3700_mpp_conf_ptr[I_NORTHBRIDGE]->nmodes) {
		pr_err("North bridge pin id %u is out of range!\n", pid);
		return -EINVAL;
	}

	rc = armada_3700_mpp_ctrl_get(pid, a3700_mpp_conf_ptr[I_NORTHBRIDGE]->bitmap, mpp_base[I_NORTHBRIDGE], config);
	if (rc) {
		pr_err("Failed to get north bridge pin %d's config!\n", pid);
		return rc;
	}

	pr_debug("%s %d pid %d config %lu\n", __func__, __LINE__, pid, *config);
	return 0;
}

static int armada_3700_nb_mpp_ctrl_set(unsigned pid, unsigned long config)
{
	int rc;

	if (pid > a3700_mpp_conf_ptr[I_NORTHBRIDGE]->nmodes) {
		pr_err("North bridge Pin id %u is out of range!\n", pid);
		return -EINVAL;
	}

	rc = armada_3700_mpp_ctrl_set(pid, a3700_mpp_conf_ptr[I_NORTHBRIDGE]->bitmap, mpp_base[I_NORTHBRIDGE], config);
	if (rc) {
		pr_err("Failed to set config %lu for north bridge pin %d!\n", config, pid);
		return rc;
	}

	pr_debug("%s %d pid %d config %lu\n", __func__, __LINE__, pid, config);
	return 0;
}

static struct mvebu_mpp_ctrl armada_3700_nb_mpp_controls[] = {
	MPP_FUNC_CTRL(0, 36, NULL, armada_3700_nb_mpp_ctrl),
};

/* The mpp36 is not a GPIO pin exposed to the user, excluded from the GPIO range. */
static struct pinctrl_gpio_range armada_3700_nb_mpp_gpio_ranges[] = {
	MPP_GPIO_RANGE(0, 0, 0, 36),
};

/* south bridge pin-ctl for Z1 */
static struct mvebu_mpp_mode armada_3700_z1_sb_mpp_modes[] = {
	MPP_MODE(0,
	   MPP_FUNCTION(0x0, "usb32", "drvvbus0"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(1,
	   MPP_FUNCTION(0x0, "usb2", "drvvbus1"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(2,
	   MPP_FUNCTION(0x0, "gpio", NULL)),
	MPP_MODE(3,
	   MPP_FUNCTION(0x0, "pcie1", "resetn"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(4,
	   MPP_FUNCTION(0x0, "pcie1", "clkreq"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(5,
	   MPP_FUNCTION(0x0, "gpio", NULL)),
	MPP_MODE(6,
	   MPP_FUNCTION(0x0, "rgmii/mii/smi", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(7,
	   MPP_FUNCTION(0x0, "rgmii/mii/smi", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(8,
	   MPP_FUNCTION(0x0, "rgmii/mii/smi", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(9,
	   MPP_FUNCTION(0x0, "rgmii/mii/smi", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(10,
	   MPP_FUNCTION(0x0, "rgmii/mii/smi", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(11,
	   MPP_FUNCTION(0x0, "rgmii/mii/smi", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(12,
	   MPP_FUNCTION(0x0, "rgmii/mii/smi", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(13,
	   MPP_FUNCTION(0x0, "rgmii/mii/smi", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(14,
	   MPP_FUNCTION(0x0, "rgmii/mii/smi", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(15,
	   MPP_FUNCTION(0x0, "rgmii/mii/smi", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(16,
	   MPP_FUNCTION(0x0, "rgmii/mii/smi", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(17,
	   MPP_FUNCTION(0x0, "rgmii/mii/smi", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(18,
	   MPP_FUNCTION(0x0, "rgmii/mii/smi", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(19,
	   MPP_FUNCTION(0x0, "rgmii/mii/smi", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(20,
	   MPP_FUNCTION(0x0, "ptp/mii", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(21,
	   MPP_FUNCTION(0x0, "ptp", "clk-req"),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "mii", "tx-err")),
	MPP_MODE(22,
	   MPP_FUNCTION(0x0, "ptp", "trig-gen"),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "mii", "carrier-sense")),
	MPP_MODE(23,
	   MPP_FUNCTION(0x0, "rgmii/mii/smi", "mii-collision"),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "mii", "tx-err")),
	MPP_MODE(24,
	   MPP_FUNCTION(0x0, "sdio", "sd0-d2"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(25,
	   MPP_FUNCTION(0x0, "sdio", "sd0-d3"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(26,
	   MPP_FUNCTION(0x0, "sdio", "sd0-d1"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(27,
	   MPP_FUNCTION(0x0, "sdio", "sd0-d0"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(28,
	   MPP_FUNCTION(0x0, "sdio", "sd0-cmd"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(29,
	   MPP_FUNCTION(0x0, "sdio", "sd0-clk"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
};

/* south bridge pins' configs setting bitmaps for Z1, this array idx is south bridge pin id */
static struct armada_3700_mpp_setting_bitmap armada_3700_z1_sb_mpp_bitmap[] = {
	/*mask			config_num	configs*/
	{BIT(0),		2,		{0, BIT(0)}			},	/* pin 0 */
	{BIT(1),		2,		{0, BIT(1)}			},	/* pin 1 */
	{0,			1,		{0}				},	/* pin 2 */
	{BIT(4),		2,		{0, BIT(4)}			},	/* pin 3 */
	{BIT(4),		2,		{0, BIT(4)}			},	/* pin 4 */
	{0,			1,		{0}				},	/* pin 5 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 6 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 7 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 8 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 9 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 10 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 11 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 12 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 13 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 14 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 15 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 16 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 17 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 18 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 19 */
	{BIT(5),		2,		{0, BIT(5)}			},	/* pin 20 */
	{BIT(5) | BIT(6),	3,		{0, BIT(5), BIT(6)}		},	/* pin 21 */
	{BIT(5) | BIT(7),	3,		{0, BIT(5), BIT(7)}		},	/* pin 22 */
	{BIT(3) | BIT(8),	3,		{0, BIT(3), BIT(8)}		},	/* pin 23 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 24 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 25 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 26 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 27 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 28 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 29 */
};

/* south bridge pin-ctl for A0 */
static struct mvebu_mpp_mode armada_3700_sb_mpp_modes[] = {
	MPP_MODE(0,
	   MPP_FUNCTION(0x0, "usb32", "drvvbus0"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(1,
	   MPP_FUNCTION(0x0, "usb2", "drvvbus1"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(2,
	   MPP_FUNCTION(0x0, "gpio", NULL)),
	MPP_MODE(3,
	   MPP_FUNCTION(0x0, "pcie1", "resetn"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(4,
	   MPP_FUNCTION(0x0, "pcie1", "clkreq"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(5,
	   MPP_FUNCTION(0x0, "pcie1", "wakeup"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(6,
	   MPP_FUNCTION(0x0, "rgmii", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(7,
	   MPP_FUNCTION(0x0, "rgmii", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(8,
	   MPP_FUNCTION(0x0, "rgmii", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(9,
	   MPP_FUNCTION(0x0, "rgmii", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(10,
	   MPP_FUNCTION(0x0, "rgmii", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(11,
	   MPP_FUNCTION(0x0, "rgmii", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(12,
	   MPP_FUNCTION(0x0, "rgmii", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(13,
	   MPP_FUNCTION(0x0, "rgmii", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(14,
	   MPP_FUNCTION(0x0, "rgmii", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(15,
	   MPP_FUNCTION(0x0, "rgmii", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(16,
	   MPP_FUNCTION(0x0, "rgmii", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(17,
	   MPP_FUNCTION(0x0, "rgmii", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(18,
	   MPP_FUNCTION(0x0, "smi", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(19,
	   MPP_FUNCTION(0x0, "smi", NULL),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(20,
	   MPP_FUNCTION(0x0, "ptp", "event-req"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(21,
	   MPP_FUNCTION(0x0, "ptp", "clk-req"),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "mii", "tx-err")),
	MPP_MODE(22,
	   MPP_FUNCTION(0x0, "ptp", "trig-gen"),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "mii", "carrier-sense")),
	MPP_MODE(23,
	   MPP_FUNCTION(0x0, "mii", "collision"),
	   MPP_FUNCTION(0x1, "gpio", NULL),
	   MPP_FUNCTION(0x2, "mii", "tx-err")),
	MPP_MODE(24,
	   MPP_FUNCTION(0x0, "sdio", "sd0-d2"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(25,
	   MPP_FUNCTION(0x0, "sdio", "sd0-d3"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(26,
	   MPP_FUNCTION(0x0, "sdio", "sd0-d1"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(27,
	   MPP_FUNCTION(0x0, "sdio", "sd0-d0"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(28,
	   MPP_FUNCTION(0x0, "sdio", "sd0-cmd"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
	MPP_MODE(29,
	   MPP_FUNCTION(0x0, "sdio", "sd0-clk"),
	   MPP_FUNCTION(0x1, "gpio", NULL)),
};

/* south bridge pins' configs setting bitmaps for A0, this array idx is south bridge pin id */
static struct armada_3700_mpp_setting_bitmap armada_3700_sb_mpp_bitmap[] = {
	/*mask			config_num	configs*/
	{BIT(0),		2,		{0, BIT(0)}			},	/* pin 0 */
	{BIT(1),		2,		{0, BIT(1)}			},	/* pin 1 */
	{0,			1,		{0}				},	/* pin 2 */
	{BIT(5),		2,		{0, BIT(5)}			},	/* pin 3 */
	{BIT(9),		2,		{0, BIT(9)}			},	/* pin 4 */
	{BIT(10),		2,		{0, BIT(10)}			},	/* pin 5 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 6 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 7 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 8 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 9 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 10 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 11 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 12 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 13 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 14 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 15 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 16 */
	{BIT(3),		2,		{0, BIT(3)}			},	/* pin 17 */
	{BIT(4),		2,		{0, BIT(4)}			},	/* pin 18 */
	{BIT(4),		2,		{0, BIT(4)}			},	/* pin 19 */
	{BIT(11),		2,		{0, BIT(11)}			},	/* pin 20 */
	{BIT(12) | BIT(6),	3,		{0, BIT(12), BIT(6)}		},	/* pin 21 */
	{BIT(13) | BIT(7),	3,		{0, BIT(13), BIT(7)}		},	/* pin 22 */
	{BIT(14) | BIT(8),	3,		{0, BIT(14), BIT(8)}		},	/* pin 23 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 24 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 25 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 26 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 27 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 28 */
	{BIT(2),		2,		{0, BIT(2)}			},	/* pin 29 */
};

static int armada_3700_sb_mpp_ctrl_get(unsigned pid, unsigned long *config)
{
	int rc;

	if (pid > a3700_mpp_conf_ptr[I_SOUTHBRIDGE]->nmodes) {
		pr_err("South bridge pin id %u is out of range!\n", pid);
		return -EINVAL;
	}

	rc = armada_3700_mpp_ctrl_get(pid, a3700_mpp_conf_ptr[I_SOUTHBRIDGE]->bitmap, mpp_base[I_SOUTHBRIDGE], config);
	if (rc) {
		pr_err("Failed to get south bridge pin %d's config!\n", pid);
		return rc;
	}

	pr_debug("%s %d pid %d config %lu\n", __func__, __LINE__, pid, *config);
	return 0;
}

static int armada_3700_sb_mpp_ctrl_set(unsigned pid, unsigned long config)
{
	int rc;

	if (pid > a3700_mpp_conf_ptr[I_SOUTHBRIDGE]->nmodes) {
		pr_err("South bridge Pin id %u is out of range!\n", pid);
		return -EINVAL;
	}

	rc = armada_3700_mpp_ctrl_set(pid, a3700_mpp_conf_ptr[I_SOUTHBRIDGE]->bitmap, mpp_base[I_SOUTHBRIDGE], config);
	if (rc) {
		pr_err("Failed to set config %lu for south bridge pin %d!\n", config, pid);
		return rc;
	}

	pr_debug("%s %d pid %d config %lu\n", __func__, __LINE__, pid, config);
	return 0;
}

static unsigned int armada_3700_mpp_consistency_check(struct armada_3700_mpp_conf *mpp_conf)
{
	unsigned int i, bitmap_array_size, modes_array_size;
	struct mvebu_mpp_mode *mpp_mode;
	struct armada_3700_mpp_setting_bitmap *bit_map;
	struct mvebu_mpp_ctrl_setting *set;

	bitmap_array_size = mpp_conf->nbitmaps;
	modes_array_size = mpp_conf->nmodes;
	mpp_mode = mpp_conf->modes;
	bit_map = mpp_conf->bitmap;

	if (bitmap_array_size != modes_array_size) {
		pr_err("The sizes of bitmap and modes arrays are not same for bank %d!\n", mpp_conf->index);
		return -EFAULT;
	}

	for (i = 0; i < bitmap_array_size; i++) {
		unsigned int setting_num = 0;

		/* get mpp pin i's setting number */
		for (set = mpp_mode[i].settings; set->name != NULL; set++)
			setting_num++;

		if (bit_map[i].config_num != setting_num) {
			pr_err("bank %d pid %d's config num are not same in bitmap and modes arrays!\n",
				mpp_conf->index, i);
			return -EFAULT;
		}
	}

	return 0;
}

static struct mvebu_mpp_ctrl armada_3700_sb_mpp_controls[] = {
	MPP_FUNC_CTRL(0, 29, NULL, armada_3700_sb_mpp_ctrl),
};

/* south bridge gpio starts from global gpio id 36 */
static struct pinctrl_gpio_range armada_3700_sb_mpp_gpio_ranges[] = {
	MPP_GPIO_RANGE(0, 0, 36, 30),
};
static struct mvebu_pinctrl_soc_info a3700_z1_mpp_sb_soc_info = {
	.variant = 0,
	.controls = armada_3700_sb_mpp_controls,
	.ncontrols = ARRAY_SIZE(armada_3700_sb_mpp_controls),
	.modes = armada_3700_z1_sb_mpp_modes,
	.nmodes = ARRAY_SIZE(armada_3700_z1_sb_mpp_modes),
	.gpioranges = armada_3700_sb_mpp_gpio_ranges,
	.ngpioranges = ARRAY_SIZE(armada_3700_sb_mpp_gpio_ranges),
};

static struct armada_3700_mpp_conf a3700_z1_mpp_sb_conf = {
	.soc_info = &a3700_z1_mpp_sb_soc_info,
	.modes = armada_3700_z1_sb_mpp_modes,
	.nmodes = ARRAY_SIZE(armada_3700_z1_sb_mpp_modes),
	.bitmap = armada_3700_z1_sb_mpp_bitmap,
	.nbitmaps = ARRAY_SIZE(armada_3700_z1_sb_mpp_bitmap),
	.index = I_SOUTHBRIDGE,
};
static struct mvebu_pinctrl_soc_info a3700_mpp_sb_soc_info = {
	.variant = 0,
	.controls = armada_3700_sb_mpp_controls,
	.ncontrols = ARRAY_SIZE(armada_3700_sb_mpp_controls),
	.modes = armada_3700_sb_mpp_modes,
	.nmodes = ARRAY_SIZE(armada_3700_sb_mpp_modes),
	.gpioranges = armada_3700_sb_mpp_gpio_ranges,
	.ngpioranges = ARRAY_SIZE(armada_3700_sb_mpp_gpio_ranges),
};

static struct armada_3700_mpp_conf a3700_mpp_sb_conf = {
	.soc_info = &a3700_mpp_sb_soc_info,
	.modes = armada_3700_sb_mpp_modes,
	.nmodes = ARRAY_SIZE(armada_3700_sb_mpp_modes),
	.bitmap = armada_3700_sb_mpp_bitmap,
	.nbitmaps = ARRAY_SIZE(armada_3700_sb_mpp_bitmap),
	.index = I_SOUTHBRIDGE,
};

static struct mvebu_pinctrl_soc_info a3700_z1_mpp_nb_soc_info = {
	.variant = 0,
	.controls = armada_3700_nb_mpp_controls,
	.ncontrols = ARRAY_SIZE(armada_3700_nb_mpp_controls),
	.modes = armada_3700_z1_nb_mpp_modes,
	.nmodes = ARRAY_SIZE(armada_3700_z1_nb_mpp_modes),
	.gpioranges = armada_3700_nb_mpp_gpio_ranges,
	.ngpioranges = ARRAY_SIZE(armada_3700_nb_mpp_gpio_ranges),
};

static struct armada_3700_mpp_conf a3700_z1_mpp_nb_conf = {
	.soc_info = &a3700_z1_mpp_nb_soc_info,
	.modes = armada_3700_z1_nb_mpp_modes,
	.nmodes = ARRAY_SIZE(armada_3700_z1_nb_mpp_modes),
	.bitmap = armada_3700_z1_nb_mpp_bitmap,
	.nbitmaps = ARRAY_SIZE(armada_3700_z1_nb_mpp_bitmap),
	.index = I_NORTHBRIDGE,
};

static struct mvebu_pinctrl_soc_info a3700_mpp_nb_soc_info = {
	.variant = 0,
	.controls = armada_3700_nb_mpp_controls,
	.ncontrols = ARRAY_SIZE(armada_3700_nb_mpp_controls),
	.modes = armada_3700_nb_mpp_modes,
	.nmodes = ARRAY_SIZE(armada_3700_nb_mpp_modes),
	.gpioranges = armada_3700_nb_mpp_gpio_ranges,
	.ngpioranges = ARRAY_SIZE(armada_3700_nb_mpp_gpio_ranges),
};

static struct armada_3700_mpp_conf a3700_mpp_nb_conf = {
	.soc_info = &a3700_mpp_nb_soc_info,
	.modes = armada_3700_nb_mpp_modes,
	.nmodes = ARRAY_SIZE(armada_3700_nb_mpp_modes),
	.bitmap = armada_3700_nb_mpp_bitmap,
	.nbitmaps = ARRAY_SIZE(armada_3700_nb_mpp_bitmap),
	.index = I_NORTHBRIDGE,
};

static const struct of_device_id armada_3700_pinctrl_of_match[] = {
	{
		.compatible = "marvell,armada-3700-z1-nb-pinctrl",
		.data       = (void *) &a3700_z1_mpp_nb_conf
	},
	{
		.compatible = "marvell,armada-3700-z1-sb-pinctrl",
		.data       = (void *) &a3700_z1_mpp_sb_conf,
	},
	{
		.compatible = "marvell,armada-3700-nb-pinctrl",
		.data	    = (void *) &a3700_mpp_nb_conf
	},
	{
		.compatible = "marvell,armada-3700-sb-pinctrl",
		.data	    = (void *) &a3700_mpp_sb_conf,
	},
	{ },
};

static int armada_3700_pinctrl_probe(struct platform_device *pdev)
{
	const struct of_device_id *match =
		of_match_device(armada_3700_pinctrl_of_match, &pdev->dev);
	struct armada_3700_mpp_conf *mpp_conf;
	struct resource *res;

	if (!match)
		return -ENODEV;

	mpp_conf = (struct armada_3700_mpp_conf *) match->data;
	if (mpp_conf->index > I_MAXCONTROLLER) {
		dev_err(&pdev->dev, "controller index error, index=%d max=%d\n", mpp_conf->index, I_MAXCONTROLLER);
		return -ENODEV;
	}

	if (armada_3700_mpp_consistency_check(mpp_conf) != 0)
		return -EFAULT;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	mpp_base[mpp_conf->index] = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(mpp_base[mpp_conf->index]))
		return PTR_ERR(mpp_base[mpp_conf->index]);

	a3700_mpp_conf_ptr[mpp_conf->index] = mpp_conf;

	pdev->dev.platform_data = mpp_conf->soc_info;

	return mvebu_pinctrl_probe(pdev);
}

static int armada_3700_pinctrl_remove(struct platform_device *pdev)
{
	return mvebu_pinctrl_remove(pdev);
}

#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
#ifdef CONFIG_PM
static int armada_3700_pinctrl_suspend(void)
{
	int i;

	for (i = 0; i < I_MAXCONTROLLER; i++)
		if (mpp_base[i] && (!IS_ERR(mpp_base[i])))
			mpp_saved_regs[i] = readl(mpp_base[i]);
	return 0;
}

static void armada_3700_pinctrl_resume(void)
{
	int i;

	for (i = 0; i < I_MAXCONTROLLER; i++)
		if (mpp_base[i] && (!IS_ERR(mpp_base[i])))
			writel(mpp_saved_regs[i], mpp_base[i]);
}

static struct syscore_ops armada_3700_pinctrl_syscore_ops = {
	.suspend = armada_3700_pinctrl_suspend,
	.resume = armada_3700_pinctrl_resume,
};
#endif

#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
static struct platform_driver armada_3700_pinctrl_driver = {
	.driver = {
		.name = "armada-3700-pinctrl",
		.of_match_table = armada_3700_pinctrl_of_match,
	},
	.probe = armada_3700_pinctrl_probe,
	.remove = armada_3700_pinctrl_remove,
};

#if defined(CONFIG_SYNO_LSP_ARMADA_17_04_02)
static int __init armada_3700_pinctrl_init(void)
{
#ifdef CONFIG_PM
	/*
	 * Register syscore ops for save/restore of registers across suspend.
	 * It's important to ensure that this driver is running at an earlier
	 * initcall level than any arch-specific init calls.
	 */
	register_syscore_ops(&armada_3700_pinctrl_syscore_ops);
#endif
	return platform_driver_register(&armada_3700_pinctrl_driver);
}

postcore_initcall(armada_3700_pinctrl_init);

static void __exit armada_3700_pinctrl_exit(void)
{
	platform_driver_unregister(&armada_3700_pinctrl_driver);
}

module_exit(armada_3700_pinctrl_exit);
#else /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */
module_platform_driver(armada_3700_pinctrl_driver);
#endif /* CONFIG_SYNO_LSP_ARMADA_17_04_02 */

MODULE_AUTHOR("Terry Zhou <bjzhou@marvell.com>");
MODULE_DESCRIPTION("Marvell Armada 3700 pinctrl driver");
MODULE_LICENSE("GPL v2");
#endif /* CONFIG_SYNO_LSP_ARMADA_16_12 */
