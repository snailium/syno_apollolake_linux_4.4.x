/*
 * Copyright (C) 2019 Realtek Semiconductor Corporation
 *
 * Author:
 *      Cheng-Yu Lee <cylee12@realtek.com>
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
 *
 */

#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of_address.h>
#include "sensor.h"

/*
 * thermal sensor offset
 */
#define TM_SENSOR_CTRL0    0x00
#define TM_SENSOR_CTRL1    0x04
#define TM_SENSOR_CTRL2    0x08
#define TM_SENSOR_STATUS0  0x40
#define TM_SENSOR_STATUS1  0x44

static void rtd13xx_sensor_reset(struct thermal_sensor_device *tdev, int index)
{
	struct thermal_sensor *sen = &tdev->sensors[index];
	u32 rstb = BIT(17);
	u32 val;

	val = readl(sen->reg + TM_SENSOR_CTRL2);
	writel(val & ~rstb, sen->reg + TM_SENSOR_CTRL2);
	writel(val | rstb, sen->reg + TM_SENSOR_CTRL2);
	usleep_range(10000, 12000);
}

static int rtd13xx_sensor_init(struct thermal_sensor_device *tdev, int index)
{
	struct thermal_sensor *sen = &tdev->sensors[index];

	sen->reg = of_iomap(tdev->dev->of_node, index);
	if (!sen->reg)
		return -ENOMEM;
	sen->available = true;

	writel(0x07ce7ae1, sen->reg + TM_SENSOR_CTRL0);
	writel(0x00378228, sen->reg + TM_SENSOR_CTRL1);
	writel(0x00011114, sen->reg + TM_SENSOR_CTRL2);
	rtd13xx_sensor_reset(tdev, index);
	return 0;
}

static void rtd13xx_sensor_exit(struct thermal_sensor_device *tdev,
	int index)
{
	struct thermal_sensor *sen = &tdev->sensors[index];

	if (!sen->available)
		return;

	sen->available = false;
	iounmap(sen->reg);
}

static inline int __hw_get_temp(void *reg)
{
	unsigned int val = readl(reg + TM_SENSOR_STATUS0);

	return __signext(18, val) * 1000 / 1024;
}

static inline int rtd13xx_sensor_get_temp(struct thermal_sensor_device *tdev,
	int index, int *temp)
{
	struct thermal_sensor *sen = &tdev->sensors[index];
	int t = 0;

	t = __hw_get_temp(sen->reg);

	if (t < -3000 || t > 150000) {
		dev_info(tdev->dev, "reset: temp=%d, status={%08x, %08x}\n",
			t, readl(sen->reg + TM_SENSOR_STATUS0),
			readl(sen->reg + TM_SENSOR_STATUS1));

		rtd13xx_sensor_reset(tdev, index);
		t = __hw_get_temp(sen->reg);
	}
	*temp = t;

	return 0;
}

static const struct thermal_sensor_hw_ops rtd13xx_sensor_hw_ops = {
	.get_temp = rtd13xx_sensor_get_temp,
	.reset    = rtd13xx_sensor_reset,
	.init     = rtd13xx_sensor_init,
	.exit     = rtd13xx_sensor_exit,
};

const struct thermal_sensor_desc rtd13xx_sensor_desc = {
	.hw_ops = &rtd13xx_sensor_hw_ops,
	.num_sensors = 1,
};