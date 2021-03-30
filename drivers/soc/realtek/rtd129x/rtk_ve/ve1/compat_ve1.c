/*
 * drivers/soc/realtek/rtd129x/rtk_ve/ve1/compat_ve1.h
 *
 * Copyright (C) 2013 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/compat.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include "ve1.h"
#include "compat_ve1.h"

/* See drivers/soc/realtek/rtd129x/rtk_ve/ve1/ve1.h for the definition of these structs */
typedef struct compat_vpudrv_buffer_t {
    compat_uint_t size;
    compat_ulong_t phys_addr;
    compat_ulong_t base;							/* kernel logical address in use kernel */
    compat_ulong_t virt_addr;				/* virtual user space address */
} compat_vpudrv_buffer_t;

typedef struct compat_vpu_clock_info_t{
    compat_uint_t core_idx;
    compat_uint_t enable;
    compat_uint_t value;
} compat_vpu_clock_info_t;

typedef struct compat_vpudrv_inst_info_t {
    compat_uint_t core_idx;
    compat_uint_t inst_idx;
    compat_int_t inst_open_count;	/* for output only*/
} compat_vpudrv_inst_info_t;

typedef struct compat_vpudrv_intr_info_t {
    compat_uint_t core_idx;
    compat_uint_t timeout;
    compat_int_t intr_reason;
} compat_vpudrv_intr_info_t;

#define COMPAT_VDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY _IO(VDI_IOCTL_MAGIC, 0)
#define COMPAT_VDI_IOCTL_FREE_PHYSICALMEMORY _IO(VDI_IOCTL_MAGIC, 1)
#define COMPAT_VDI_IOCTL_WAIT_INTERRUPT _IO(VDI_IOCTL_MAGIC, 2)
#define COMPAT_VDI_IOCTL_GET_INSTANCE_POOL _IO(VDI_IOCTL_MAGIC, 5)
#define COMPAT_VDI_IOCTL_GET_COMMON_MEMORY _IO(VDI_IOCTL_MAGIC, 6)
#define COMPAT_VDI_IOCTL_GET_RESERVED_VIDEO_MEMORY_INFO _IO(VDI_IOCTL_MAGIC, 8)
#define COMPAT_VDI_IOCTL_OPEN_INSTANCE _IO(VDI_IOCTL_MAGIC, 9)
#define COMPAT_VDI_IOCTL_CLOSE_INSTANCE _IO(VDI_IOCTL_MAGIC, 10)
#define COMPAT_VDI_IOCTL_GET_INSTANCE_NUM _IO(VDI_IOCTL_MAGIC, 11)
#define COMPAT_VDI_IOCTL_GET_REGISTER_INFO _IO(VDI_IOCTL_MAGIC, 12)

/* RTK ioctl */
#define COMPAT_VDI_IOCTL_SET_RTK_CLK_GATING _IO(VDI_IOCTL_MAGIC, 16)
#define COMPAT_VDI_IOCTL_SET_RTK_CLK_PLL _IO(VDI_IOCTL_MAGIC, 17)
#define COMPAT_VDI_IOCTL_GET_RTK_CLK_PLL _IO(VDI_IOCTL_MAGIC, 18)
#define COMPAT_VDI_IOCTL_GET_RTK_SUPPORT_TYPE _IO(VDI_IOCTL_MAGIC, 19)

static int compat_get_ve1_buffer_data(
    compat_vpudrv_buffer_t __user *data32,
    vpudrv_buffer_t __user *data)
{
    compat_uint_t s;
    compat_ulong_t p;
    compat_ulong_t b;
    compat_ulong_t v;
    int err;

    err = get_user(s, &data32->size);
    err |= put_user(s, &data->size);
    err |= get_user(p, &data32->phys_addr);
    err |= put_user(p, &data->phys_addr);
    err |= get_user(b, &data32->base);
    err |= put_user(b, &data->base);
    err |= get_user(v, &data32->virt_addr);
    err |= put_user(v, &data->virt_addr);

    return err;
}

static int compat_get_ve1_inst_info_data(
    compat_vpudrv_inst_info_t __user *data32,
    vpudrv_inst_info_t __user *data)
{
    compat_uint_t c;
    compat_uint_t i;
    compat_int_t ioc;
    int err;

    err = get_user(c, &data32->core_idx);
    err |= put_user(c, &data->core_idx);
    err |= get_user(i, &data32->inst_idx);
    err |= put_user(i, &data->inst_idx);
    err |= get_user(ioc, &data32->inst_open_count);
    err |= put_user(ioc, &data->inst_open_count);

    return err;
}

static int compat_get_ve1_intr_info_data(
    compat_vpudrv_intr_info_t __user *data32,
    vpudrv_intr_info_t __user *data)
{
    compat_uint_t c;
    compat_uint_t t;
    compat_int_t i;
    int err;

    err = get_user(c, &data32->core_idx);
    err |= put_user(c, &data->core_idx);
    err |= get_user(t, &data32->timeout);
    err |= put_user(t, &data->timeout);
    err |= get_user(i, &data32->intr_reason);
    err |= put_user(i, &data->intr_reason);

    return err;
}

static int compat_get_ve1_clock_info(
    compat_vpu_clock_info_t __user *data32,
    vpu_clock_info_t __user *data)
{
    compat_uint_t c;
    compat_uint_t e;
    compat_uint_t v;
    int err;

    err = get_user(c, &data32->core_idx);
    err |= put_user(c, &data->core_idx);
    err |= get_user(e, &data32->enable);
    err |= put_user(e, &data->enable);
    err |= get_user(v, &data32->value);
    err |= put_user(v, &data->value);

    return err;
}

static int compat_put_ve1_buffer_data(
    compat_vpudrv_buffer_t __user *data32,
    vpudrv_buffer_t __user *data)
{
    compat_uint_t s;
    compat_ulong_t p;
    compat_ulong_t b;
    compat_ulong_t v;
    int err;

    err = get_user(s, &data->size);
    err |= put_user(s, &data32->size);
    err |= get_user(p, &data->phys_addr);
    err |= put_user(p, &data32->phys_addr);
    err |= get_user(b, &data->base);
    err |= put_user(b, &data32->base);
    err |= get_user(v, &data->virt_addr);
    err |= put_user(v, &data32->virt_addr);

    return err;
}

static int compat_put_ve1_inst_info_data(
    compat_vpudrv_inst_info_t __user *data32,
    vpudrv_inst_info_t __user *data)
{
    compat_uint_t c;
    compat_uint_t i;
    compat_int_t ioc;
    int err;

    err = get_user(c, &data->core_idx);
    err |= put_user(c, &data32->core_idx);
    err |= get_user(i, &data->inst_idx);
    err |= put_user(i, &data32->inst_idx);
    err |= get_user(ioc, &data->inst_open_count);
    err |= put_user(ioc, &data32->inst_open_count);

    return err;
}

static int compat_put_ve1_intr_info_data(
    compat_vpudrv_intr_info_t __user *data32,
    vpudrv_intr_info_t __user *data)
{
    compat_uint_t c;
    compat_uint_t t;
    compat_int_t i;
    int err;

    err = get_user(c, &data->core_idx);
    err |= put_user(c, &data32->core_idx);
    err |= get_user(t, &data->timeout);
    err |= put_user(t, &data32->timeout);
    err |= get_user(i, &data->intr_reason);
    err |= put_user(i, &data32->intr_reason);

    return err;
}

static int compat_put_ve1_clock_info(
    compat_vpu_clock_info_t __user *data32,
    vpu_clock_info_t __user *data)
{
    compat_uint_t c;
    compat_uint_t e;
    compat_int_t v;
    int err;

    err = get_user(c, &data->core_idx);
    err |= put_user(c, &data32->core_idx);
    err |= get_user(e, &data->enable);
    err |= put_user(e, &data32->enable);
    err |= get_user(v, &data->value);
    err |= put_user(v, &data32->value);

    return err;
}

long compat_vpu_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    long ret = 0;

    if (!filp->f_op->unlocked_ioctl)
        return -ENOTTY;

    switch (cmd) {
    case COMPAT_VDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY:
    {
        compat_vpudrv_buffer_t __user *data32;
        vpudrv_buffer_t __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_get_ve1_buffer_data(data32, data);
        if (err)
            return err;
        ret = filp->f_op->unlocked_ioctl(filp, VDI_IOCTL_ALLOCATE_PHYSICAL_MEMORY, (unsigned long)data);
        err = compat_put_ve1_buffer_data(data32, data);
        return ret ? ret : err;
    }

    case COMPAT_VDI_IOCTL_FREE_PHYSICALMEMORY:
    {
        compat_vpudrv_buffer_t __user *data32;
        vpudrv_buffer_t __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_get_ve1_buffer_data(data32, data);
        if (err)
            return err;

        return filp->f_op->unlocked_ioctl(filp, VDI_IOCTL_FREE_PHYSICALMEMORY, (unsigned long)data);
    }

    case COMPAT_VDI_IOCTL_GET_RESERVED_VIDEO_MEMORY_INFO:
    {
        compat_vpudrv_buffer_t __user *data32;
        vpudrv_buffer_t __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_get_ve1_buffer_data(data32, data);
        if (err)
            return err;
        ret = filp->f_op->unlocked_ioctl(filp, VDI_IOCTL_GET_RESERVED_VIDEO_MEMORY_INFO, (unsigned long)data);
        err = compat_put_ve1_buffer_data(data32, data);
        return ret ? ret : err;
    }

    case COMPAT_VDI_IOCTL_WAIT_INTERRUPT:
    {
        compat_vpudrv_intr_info_t __user *data32;
        vpudrv_intr_info_t __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_get_ve1_intr_info_data(data32, data);
        if (err)
            return err;
        ret = filp->f_op->unlocked_ioctl(filp, VDI_IOCTL_WAIT_INTERRUPT, (unsigned long)data);
        err = compat_put_ve1_intr_info_data(data32, data);
        return ret ? ret : err;
    }

    case VDI_IOCTL_SET_CLOCK_GATE:
    case VDI_IOCTL_RESET:
    {
        return filp->f_op->unlocked_ioctl(filp, cmd,
                                          (unsigned long)compat_ptr(arg));
    }

    case COMPAT_VDI_IOCTL_GET_INSTANCE_POOL:
    {
        compat_vpudrv_buffer_t __user *data32;
        vpudrv_buffer_t __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_get_ve1_buffer_data(data32, data);
        if (err)
            return err;
        ret = filp->f_op->unlocked_ioctl(filp, VDI_IOCTL_GET_INSTANCE_POOL, (unsigned long)data);
        err = compat_put_ve1_buffer_data(data32, data);
        return ret ? ret : err;
    }

    case COMPAT_VDI_IOCTL_GET_COMMON_MEMORY:
    {
        compat_vpudrv_buffer_t __user *data32;
        vpudrv_buffer_t __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_get_ve1_buffer_data(data32, data);
        if (err)
            return err;
        ret = filp->f_op->unlocked_ioctl(filp, VDI_IOCTL_GET_COMMON_MEMORY, (unsigned long)data);
        err = compat_put_ve1_buffer_data(data32, data);
        return ret ? ret : err;
    }

    case COMPAT_VDI_IOCTL_OPEN_INSTANCE:
    {
        compat_vpudrv_inst_info_t __user *data32;
        vpudrv_inst_info_t __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_get_ve1_inst_info_data(data32, data);
        if (err)
            return err;
        ret = filp->f_op->unlocked_ioctl(filp, VDI_IOCTL_OPEN_INSTANCE, (unsigned long)data);
        err = compat_put_ve1_inst_info_data(data32, data);
        return ret ? ret : err;
    }

    case COMPAT_VDI_IOCTL_CLOSE_INSTANCE:
    {
        compat_vpudrv_inst_info_t __user *data32;
        vpudrv_inst_info_t __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_get_ve1_inst_info_data(data32, data);
        if (err)
            return err;
        ret = filp->f_op->unlocked_ioctl(filp, VDI_IOCTL_CLOSE_INSTANCE, (unsigned long)data);
        err = compat_put_ve1_inst_info_data(data32, data);
        return ret ? ret : err;
    }

    case COMPAT_VDI_IOCTL_GET_INSTANCE_NUM:
    {
        compat_vpudrv_inst_info_t __user *data32;
        vpudrv_inst_info_t __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_get_ve1_inst_info_data(data32, data);
        if (err)
            return err;
        ret = filp->f_op->unlocked_ioctl(filp, VDI_IOCTL_GET_INSTANCE_NUM, (unsigned long)data);
        err = compat_put_ve1_inst_info_data(data32, data);
        return ret ? ret : err;
    }

    case COMPAT_VDI_IOCTL_GET_REGISTER_INFO:
    {
        compat_vpudrv_buffer_t __user *data32;
        vpudrv_buffer_t __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_get_ve1_buffer_data(data32, data);
        if (err)
            return err;
        ret = filp->f_op->unlocked_ioctl(filp, VDI_IOCTL_GET_REGISTER_INFO, (unsigned long)data);
        err = compat_put_ve1_buffer_data(data32, data);
        return ret ? ret : err;
    }

    case COMPAT_VDI_IOCTL_SET_RTK_CLK_GATING:
    {
        compat_vpu_clock_info_t __user *data32;
        vpu_clock_info_t __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_get_ve1_clock_info(data32, data);
        if (err)
            return err;

        return filp->f_op->unlocked_ioctl(filp, VDI_IOCTL_SET_RTK_CLK_GATING, (unsigned long)data);
    }

    case COMPAT_VDI_IOCTL_SET_RTK_CLK_PLL:
    {
        compat_vpu_clock_info_t __user *data32;
        vpu_clock_info_t __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_get_ve1_clock_info(data32, data);
        if (err)
            return err;

        return filp->f_op->unlocked_ioctl(filp, VDI_IOCTL_SET_RTK_CLK_PLL, (unsigned long)data);
    }

    case COMPAT_VDI_IOCTL_GET_RTK_CLK_PLL:
    {
        compat_vpu_clock_info_t __user *data32;
        vpu_clock_info_t __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_get_ve1_clock_info(data32, data);
        if (err)
            return err;
        ret = filp->f_op->unlocked_ioctl(filp, VDI_IOCTL_GET_RTK_CLK_PLL, (unsigned long)data);
        err = compat_put_ve1_clock_info(data32, data);
        return ret ? ret : err;
    }

    case COMPAT_VDI_IOCTL_GET_RTK_SUPPORT_TYPE:
    {
        compat_vpudrv_buffer_t __user *data32;
        vpudrv_buffer_t __user *data;
        int err;

        data32 = compat_ptr(arg);
        data = compat_alloc_user_space(sizeof(*data));
        if (data == NULL)
            return -EFAULT;

        err = compat_get_ve1_buffer_data(data32, data);
        if (err)
            return err;
        ret = filp->f_op->unlocked_ioctl(filp, VDI_IOCTL_GET_RTK_SUPPORT_TYPE, (unsigned long)data);
        err = compat_put_ve1_buffer_data(data32, data);
        return ret ? ret : err;
    }

    default:
    {
        printk(KERN_ERR "[COMPAT_VPUDRV] No such IOCTL, cmd is %d\n", cmd);
        return -ENOIOCTLCMD;
    }
    }
}
