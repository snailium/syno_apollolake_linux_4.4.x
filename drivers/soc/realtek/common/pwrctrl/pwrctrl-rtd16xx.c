/*
 * Power Controller for RTD-16xx SoC
 *
 * Copyright (C) 2017-2019 Realtek Semiconductor Corporation
 *
 * Author:
 *	Cheng-Yu Lee <cylee12@realtek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/pm_domain.h>
#include <linux/reset.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/suspend.h>
#include <soc/realtek/power-control.h>
#include <dt-bindings/power/rtk,power-rtd16xx.h>
#include "pwrctrl_driver.h"
#include "pwrctrl-pd.h"

static DEFINE_SPINLOCK(iso_power_lock);
static struct rtk_sram_pd gpu_pd = INIT_SRAM_PD(gpu, 0xb60, 7, &iso_power_lock);
static struct rtk_sram_pd ve1_pd = INIT_SRAM_ASYNC_POWER_CONTROL(ve1, 0xb00, 0xf, 10, &iso_power_lock);
static struct rtk_sram_pd ve2_pd = INIT_SRAM_ASYNC_POWER_CONTROL(ve2, 0xb20, 0xf, 10, &iso_power_lock);
static struct rtk_sram_pd ve3_pd = INIT_SRAM_ASYNC_POWER_CONTROL(ve3, 0x290, 0xf, 10, &iso_power_lock);
static struct rtk_sram_pd hdmirx_pd = INIT_SRAM_POWER_CONTROL(hdmirx, 0x260, 0xf, &iso_power_lock);
static struct simple_pd ve1_iso = INIT_ISO_POWER_CONTROL(ve1, 0xfd0, BIT(0), &iso_power_lock);
static struct simple_pd ve2_iso = INIT_ISO_POWER_CONTROL(ve2, 0xfd0, BIT(1), &iso_power_lock);
static struct simple_pd ve3_iso = INIT_ISO_POWER_CONTROL(ve3, 0xfd0, BIT(10), &iso_power_lock);
static struct simple_pd gpu_iso = INIT_ISO_POWER_CONTROL(gpu, 0xfd0, BIT(3), &iso_power_lock);
static struct simple_pd hdmirx_iso = INIT_ISO_POWER_CONTROL(hdmirx, 0xfd0, BIT(9), &iso_power_lock);
static struct simple_pd gpu_2_iso = INIT_ISO_POWER_CONTROL(gpu_2, 0x078, BIT(1), &iso_power_lock);

#define PD_SIMPLE_ISO_VE1               (PD_MAX)
#define PD_SIMPLE_ISO_VE2               (PD_MAX+1)
#define PD_SIMPLE_ISO_VE3               (PD_MAX+2)
#define PD_SIMPLE_ISO_GPU               (PD_MAX+3)
#define PD_SIMPLE_ISO_HDMIRX            (PD_MAX+4)
#define PD_SIMPLE_ISO_GPU_2             (PD_MAX+5)
#define INTERNAL_PD_MAX                 (PD_MAX+6)

static int ve1_manual_power_callback(struct notifier_block *nb,
				   unsigned long action,
				   void *data)
{
	struct rtk_sram_pd *spd = pc_to_rtk_sram_pd(data);

	if (action != POWER_CONTROL_ACTION_PRE_POWER_OFF)
		return NOTIFY_OK;

	pr_debug("%s: %s\n", spd->core.pc.name, __func__);
	writel(0x00008000, spd->base + spd->pwr_offset + 0xC);
	return NOTIFY_DONE;
}

static struct notifier_block ve1_nb = {
	.notifier_call = ve1_manual_power_callback,
};

static int power_chain_callback(struct notifier_block *nb, unsigned long action, void *data)
{
	struct power_chain *pcx = container_of(nb, struct power_chain, power_nb);
	struct power_chain_head *pch = pc_to_simple_pd(pcx->self)->pch;

	if (action != POWER_CONTROL_ACTION_POST_POWER_ON &&
	    action != POWER_CONTROL_ACTION_PRE_POWER_OFF)
		return NOTIFY_DONE;

	if (action == POWER_CONTROL_ACTION_POST_POWER_ON) {

		if (power_control_is_powered_on(pcx->self))
			return NOTIFY_OK;

		pr_debug("%s: %s: power_on\n", pcx->self->name, __func__);
		power_control_power_on(pcx->self);

	} else if (action == POWER_CONTROL_ACTION_PRE_POWER_OFF) {
		struct power_chain *ch;

		list_for_each_entry(ch, &pch->list, list) {
			if (ch->ref == data)
				continue;
			if (power_control_is_powered_on(ch->ref))
				return NOTIFY_OK;
		}
		pr_debug("%s: %s: power_off\n", pcx->self->name, __func__);
		power_control_power_off(pcx->self);
	}

	return NOTIFY_OK;
}

static int iso_power_controller_setup_topology(void)
{
	simple_pd_add_power_chain(&ve1_iso, &ve1_pd.core.pc, power_chain_callback);
	simple_pd_add_power_chain(&ve2_iso, &ve2_pd.core.pc, power_chain_callback);
	simple_pd_add_power_chain(&ve3_iso, &ve3_pd.core.pc, power_chain_callback);
	simple_pd_add_power_chain(&gpu_iso, &gpu_pd.core.pc, power_chain_callback);
	simple_pd_add_power_chain(&gpu_2_iso, &gpu_pd.core.pc, power_chain_callback);
	simple_pd_add_power_chain(&hdmirx_iso, &hdmirx_pd.core.pc, power_chain_callback);
	return 0;
}

static struct pwrctrl_pd *iso_ppds[INTERNAL_PD_MAX] = {
	[PD_SRAM_VE1] = &ve1_pd.core,
	[PD_SRAM_VE2] = &ve2_pd.core,
	[PD_SRAM_VE3] = &ve3_pd.core,
	[PD_SRAM_GPU] = &gpu_pd.core,
	[PD_SRAM_HDMIRX] = &hdmirx_pd.core,
	[PD_SIMPLE_ISO_VE1] = &ve1_iso.core,
	[PD_SIMPLE_ISO_VE2] = &ve2_iso.core,
	[PD_SIMPLE_ISO_VE3] = &ve3_iso.core,
	[PD_SIMPLE_ISO_GPU] = &gpu_iso.core,
	[PD_SIMPLE_ISO_GPU_2] = &gpu_2_iso.core,
	[PD_SIMPLE_ISO_HDMIRX] = &hdmirx_iso.core,
};

static int iso_power_controller_ready(struct power_controller_data *data)
{
	ve1_pd.core.pc.flags |= POWER_CONTROL_FLAG_SHARED_POWER;
	ve2_pd.core.pc.flags |= POWER_CONTROL_FLAG_SHARED_POWER;
	ve3_pd.core.pc.flags |= POWER_CONTROL_FLAG_SHARED_POWER;
	power_control_register_notifier(&ve1_pd.core.pc, &ve1_nb);
	iso_power_controller_setup_topology();
	return 0;
}

static struct power_controller_data iso_data = {
	.ppds = iso_ppds,
	.num_ppds = ARRAY_SIZE(iso_ppds),
	.ready = iso_power_controller_ready,
};

static const struct of_device_id power_controller_match[];

static int power_controller_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct power_controller_data *pcd;
	const struct of_device_id *id;

	/* get controller data */
	id = of_match_node(power_controller_match, dev->of_node);
	if (!id)
		return -ENODEV;

	pcd = (struct power_controller_data *)id->data;
	dev_set_drvdata(dev, pcd);
	power_controller_init_pds(dev, pcd);
	dev_info(dev, "initialized\n");
	return 0;
}

static const struct of_device_id power_controller_match[] = {
	{.compatible = "realtek,rtd16xx-power-controller", .data = &iso_data},
	{}
};

static struct platform_driver power_controller_driver = {
	.probe = power_controller_probe,
	.driver = {
		.name = "rtk-rtd16xx-power",
		.of_match_table = power_controller_match,
		.pm = &power_controller_pm_ops,
	},
};

static int __init power_controller_init(void)
{
	return platform_driver_register(&power_controller_driver);
}
arch_initcall(power_controller_init);
