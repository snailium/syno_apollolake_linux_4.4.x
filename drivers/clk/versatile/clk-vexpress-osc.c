/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Copyright (C) 2012 ARM Limited
 */

#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/vexpress.h>

struct vexpress_osc {
	struct regmap *reg;
	struct clk_hw hw;
	unsigned long rate_min;
	unsigned long rate_max;
};

#define to_vexpress_osc(osc) container_of(osc, struct vexpress_osc, hw)

static unsigned long vexpress_osc_recalc_rate(struct clk_hw *hw,
		unsigned long parent_rate)
{
	struct vexpress_osc *osc = to_vexpress_osc(hw);
	u32 rate;

	regmap_read(osc->reg, 0, &rate);

	return rate;
}

static long vexpress_osc_round_rate(struct clk_hw *hw, unsigned long rate,
		unsigned long *parent_rate)
{
	struct vexpress_osc *osc = to_vexpress_osc(hw);

	if (WARN_ON(osc->rate_min && rate < osc->rate_min))
		rate = osc->rate_min;

	if (WARN_ON(osc->rate_max && rate > osc->rate_max))
		rate = osc->rate_max;

	return rate;
}

static int vexpress_osc_set_rate(struct clk_hw *hw, unsigned long rate,
		unsigned long parent_rate)
{
	struct vexpress_osc *osc = to_vexpress_osc(hw);

	return regmap_write(osc->reg, 0, rate);
}

static struct clk_ops vexpress_osc_ops = {
	.recalc_rate = vexpress_osc_recalc_rate,
	.round_rate = vexpress_osc_round_rate,
	.set_rate = vexpress_osc_set_rate,
};

static int vexpress_osc_probe(struct platform_device *pdev)
{
	struct clk_init_data init;
	struct vexpress_osc *osc;
	struct clk *clk;
	u32 range[2];

	osc = devm_kzalloc(&pdev->dev, sizeof(*osc), GFP_KERNEL);
	if (!osc)
		return -ENOMEM;

	osc->reg = devm_regmap_init_vexpress_config(&pdev->dev);
	if (IS_ERR(osc->reg))
		return PTR_ERR(osc->reg);

	if (of_property_read_u32_array(pdev->dev.of_node, "freq-range", range,
			ARRAY_SIZE(range)) == 0) {
		osc->rate_min = range[0];
		osc->rate_max = range[1];
	}

	if (of_property_read_string(pdev->dev.of_node, "clock-output-names",
			&init.name) != 0)
		init.name = dev_name(&pdev->dev);

	init.ops = &vexpress_osc_ops;
	init.flags = CLK_IS_ROOT;
	init.num_parents = 0;

	osc->hw.init = &init;

	clk = clk_register(NULL, &osc->hw);
	if (IS_ERR(clk))
		return PTR_ERR(clk);

	of_clk_add_provider(pdev->dev.of_node, of_clk_src_simple_get, clk);

	dev_dbg(&pdev->dev, "Registered clock '%s'\n", init.name);

	return 0;
}

static const struct of_device_id vexpress_osc_of_match[] = {
	{ .compatible = "arm,vexpress-osc", },
	{}
};

static struct platform_driver vexpress_osc_driver = {
	.driver	= {
		.name = "vexpress-osc",
		.of_match_table = vexpress_osc_of_match,
	},
	.probe = vexpress_osc_probe,
};

static int __init vexpress_osc_init(void)
{
	return platform_driver_register(&vexpress_osc_driver);
}
core_initcall(vexpress_osc_init);
