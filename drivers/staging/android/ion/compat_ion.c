#ifndef MY_ABC_HERE
#define MY_ABC_HERE
#endif
/*
 * drivers/staging/android/ion/compat_ion.c
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

#include "ion.h"
#include "compat_ion.h"

/* See drivers/staging/android/uapi/ion.h for the definition of these structs */
struct compat_ion_allocation_data {
	compat_size_t len;
	compat_size_t align;
	compat_uint_t heap_id_mask;
	compat_uint_t flags;
	compat_int_t handle;
};

struct compat_ion_custom_data {
	compat_uint_t cmd;
	compat_ulong_t arg;
};

struct compat_ion_handle_data {
	compat_int_t handle;
};

#if defined(MY_DEF_HERE) || defined(CONFIG_ION_RTK) && defined(CONFIG_SYNO_LSP_RTD1619)
struct compat_ion_phys_data {
	compat_int_t handle;
	compat_ulong_t addr;
	compat_size_t len;
};
#endif /* MY_DEF_HERE || CONFIG_ION_RTK && CONFIG_SYNO_LSP_RTD1619 */

#define COMPAT_ION_IOC_ALLOC	_IOWR(ION_IOC_MAGIC, 0, \
				      struct compat_ion_allocation_data)
#define COMPAT_ION_IOC_FREE	_IOWR(ION_IOC_MAGIC, 1, \
				      struct compat_ion_handle_data)
#define COMPAT_ION_IOC_CUSTOM	_IOWR(ION_IOC_MAGIC, 6, \
				      struct compat_ion_custom_data)
#if defined(CONFIG_SYNO_LSP_RTD1619)
#if defined(CONFIG_ION_RTK)
#define COMPAT_ION_IOC_PHYS	_IOWR(ION_IOC_MAGIC, 8, \
				      struct compat_ion_phys_data)
#endif /* CONFIG_ION_RTK */
#endif /* CONFIG_SYNO_LSP_RTD1619 */

#if defined(MY_DEF_HERE)
#define COMPAT_ION_IOC_PHYS	_IOWR(ION_IOC_MAGIC, 8, \
				      struct compat_ion_phys_data)
#endif /* MY_DEF_HERE */

static int compat_get_ion_allocation_data(
			struct compat_ion_allocation_data __user *data32,
			struct ion_allocation_data __user *data)
{
	compat_size_t s;
	compat_uint_t u;
	compat_int_t i;
	int err;

	err = get_user(s, &data32->len);
	err |= put_user(s, &data->len);
	err |= get_user(s, &data32->align);
	err |= put_user(s, &data->align);
	err |= get_user(u, &data32->heap_id_mask);
	err |= put_user(u, &data->heap_id_mask);
	err |= get_user(u, &data32->flags);
	err |= put_user(u, &data->flags);
	err |= get_user(i, &data32->handle);
	err |= put_user(i, &data->handle);

	return err;
}

static int compat_get_ion_handle_data(
			struct compat_ion_handle_data __user *data32,
			struct ion_handle_data __user *data)
{
	compat_int_t i;
	int err;

	err = get_user(i, &data32->handle);
	err |= put_user(i, &data->handle);

	return err;
}

#if defined(MY_DEF_HERE) || defined(CONFIG_ION_RTK) && defined(CONFIG_SYNO_LSP_RTD1619)
static int compat_get_ion_phys_data(
			struct compat_ion_phys_data __user *data32,
			struct ion_phys_data __user *data)
{
	compat_size_t s;
	compat_int_t i;
	compat_ulong_t u;
	int err;

	err = get_user(i, &data32->handle);
	err |= put_user(i, &data->handle);
	err |= get_user(u, &data32->addr);
	err |= put_user(u, &data->addr);
	err |= get_user(s, &data32->len);
	err |= put_user(s, &data->len);

	return err;
}

static int compat_put_ion_phys_data(
			struct compat_ion_phys_data __user *data32,
			struct ion_phys_data __user *data)
{
	compat_size_t s;
	compat_int_t i;
	compat_ulong_t u;
	int err;

	err = get_user(i, &data->handle);
	err |= put_user(i, &data32->handle);
	err |= get_user(u, &data->addr);
	err |= put_user(u, &data32->addr);
	err |= get_user(s, &data->len);
	err |= put_user(s, &data32->len);

	return err;
}
#endif /* MY_DEF_HERE || CONFIG_ION_RTK && CONFIG_SYNO_LSP_RTD1619 */

static int compat_put_ion_allocation_data(
			struct compat_ion_allocation_data __user *data32,
			struct ion_allocation_data __user *data)
{
	compat_size_t s;
	compat_uint_t u;
	compat_int_t i;
	int err;

	err = get_user(s, &data->len);
	err |= put_user(s, &data32->len);
	err |= get_user(s, &data->align);
	err |= put_user(s, &data32->align);
	err |= get_user(u, &data->heap_id_mask);
	err |= put_user(u, &data32->heap_id_mask);
	err |= get_user(u, &data->flags);
	err |= put_user(u, &data32->flags);
	err |= get_user(i, &data->handle);
	err |= put_user(i, &data32->handle);

	return err;
}

static int compat_get_ion_custom_data(
			struct compat_ion_custom_data __user *data32,
			struct ion_custom_data __user *data)
{
	compat_uint_t cmd;
	compat_ulong_t arg;
	int err;

	err = get_user(cmd, &data32->cmd);
	err |= put_user(cmd, &data->cmd);
	err |= get_user(arg, &data32->arg);
	err |= put_user(arg, &data->arg);

	return err;
};

long compat_ion_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret;

	if (!filp->f_op->unlocked_ioctl)
		return -ENOTTY;

	switch (cmd) {
	case COMPAT_ION_IOC_ALLOC:
	{
		struct compat_ion_allocation_data __user *data32;
		struct ion_allocation_data __user *data;
		int err;

		data32 = compat_ptr(arg);
		data = compat_alloc_user_space(sizeof(*data));
		if (data == NULL)
			return -EFAULT;

		err = compat_get_ion_allocation_data(data32, data);
		if (err)
			return err;
		ret = filp->f_op->unlocked_ioctl(filp, ION_IOC_ALLOC,
							(unsigned long)data);
		err = compat_put_ion_allocation_data(data32, data);
		return ret ? ret : err;
	}
	case COMPAT_ION_IOC_FREE:
	{
		struct compat_ion_handle_data __user *data32;
		struct ion_handle_data __user *data;
		int err;

		data32 = compat_ptr(arg);
		data = compat_alloc_user_space(sizeof(*data));
		if (data == NULL)
			return -EFAULT;

		err = compat_get_ion_handle_data(data32, data);
		if (err)
			return err;

		return filp->f_op->unlocked_ioctl(filp, ION_IOC_FREE,
							(unsigned long)data);
	}
	case COMPAT_ION_IOC_CUSTOM: {
		struct compat_ion_custom_data __user *data32;
		struct ion_custom_data __user *data;
		int err;

		data32 = compat_ptr(arg);
		data = compat_alloc_user_space(sizeof(*data));
		if (data == NULL)
			return -EFAULT;

		err = compat_get_ion_custom_data(data32, data);
		if (err)
			return err;

		return filp->f_op->unlocked_ioctl(filp, ION_IOC_CUSTOM,
							(unsigned long)data);
	}
	case ION_IOC_SHARE:
	case ION_IOC_MAP:
	case ION_IOC_IMPORT:
	case ION_IOC_SYNC:
		return filp->f_op->unlocked_ioctl(filp, cmd,
						(unsigned long)compat_ptr(arg));
#if defined(MY_DEF_HERE) || defined(CONFIG_ION_RTK) && defined(CONFIG_SYNO_LSP_RTD1619)
	case COMPAT_ION_IOC_PHYS:
	{
		struct compat_ion_phys_data __user *data32;
		struct ion_phys_data __user *data;
		int err;

		data32 = compat_ptr(arg);
		data = compat_alloc_user_space(sizeof(*data));
		if (data == NULL)
			return -EFAULT;

		err = compat_get_ion_phys_data(data32, data);
		if (err)
			return err;

		err = filp->f_op->unlocked_ioctl(filp, ION_IOC_PHYS,
							(unsigned long)data);

		compat_put_ion_phys_data(data32, data);
		return err;

	}
#endif /* MY_DEF_HERE || CONFIG_ION_RTK && CONFIG_SYNO_LSP_RTD1619 */
	default:
		return -ENOIOCTLCMD;
	}
}
