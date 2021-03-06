#ifndef MY_ABC_HERE
#define MY_ABC_HERE
#endif
 
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/dnotify.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/fdtable.h>
#include <linux/fsnotify_backend.h>

int dir_notify_enable __read_mostly = 1;

static struct kmem_cache *dnotify_struct_cache __read_mostly;
static struct kmem_cache *dnotify_mark_cache __read_mostly;
static struct fsnotify_group *dnotify_group __read_mostly;

struct dnotify_mark {
	struct fsnotify_mark fsn_mark;
	struct dnotify_struct *dn;
};

static void dnotify_recalc_inode_mask(struct fsnotify_mark *fsn_mark)
{
	__u32 new_mask, old_mask;
	struct dnotify_struct *dn;
	struct dnotify_mark *dn_mark  = container_of(fsn_mark,
						     struct dnotify_mark,
						     fsn_mark);

	assert_spin_locked(&fsn_mark->lock);

	old_mask = fsn_mark->mask;
	new_mask = 0;
	for (dn = dn_mark->dn; dn != NULL; dn = dn->dn_next)
		new_mask |= (dn->dn_mask & ~FS_DN_MULTISHOT);
	fsnotify_set_mark_mask_locked(fsn_mark, new_mask);

	if (old_mask == new_mask)
		return;

	if (fsn_mark->inode)
		fsnotify_recalc_inode_mask(fsn_mark->inode);
}

static int dnotify_handle_event(struct fsnotify_group *group,
				struct inode *inode,
				struct fsnotify_mark *inode_mark,
				struct fsnotify_mark *vfsmount_mark,
				u32 mask, void *data, int data_type,
				const unsigned char *file_name, u32 cookie)
{
	struct dnotify_mark *dn_mark;
	struct dnotify_struct *dn;
	struct dnotify_struct **prev;
	struct fown_struct *fown;
	__u32 test_mask = mask & ~FS_EVENT_ON_CHILD;

#ifdef MY_ABC_HERE
	if (data_type == FSNOTIFY_EVENT_SYNO)
		return 0;
#endif  

	if (!S_ISDIR(inode->i_mode))
		return 0;

	BUG_ON(vfsmount_mark);

	dn_mark = container_of(inode_mark, struct dnotify_mark, fsn_mark);

	spin_lock(&inode_mark->lock);
	prev = &dn_mark->dn;
	while ((dn = *prev) != NULL) {
		if ((dn->dn_mask & test_mask) == 0) {
			prev = &dn->dn_next;
			continue;
		}
		fown = &dn->dn_filp->f_owner;
		send_sigio(fown, dn->dn_fd, POLL_MSG);
		if (dn->dn_mask & FS_DN_MULTISHOT)
			prev = &dn->dn_next;
		else {
			*prev = dn->dn_next;
			kmem_cache_free(dnotify_struct_cache, dn);
			dnotify_recalc_inode_mask(inode_mark);
		}
	}

	spin_unlock(&inode_mark->lock);

	return 0;
}

static void dnotify_free_mark(struct fsnotify_mark *fsn_mark)
{
	struct dnotify_mark *dn_mark = container_of(fsn_mark,
						    struct dnotify_mark,
						    fsn_mark);

	BUG_ON(dn_mark->dn);

	kmem_cache_free(dnotify_mark_cache, dn_mark);
}

static struct fsnotify_ops dnotify_fsnotify_ops = {
	.handle_event = dnotify_handle_event,
};

void dnotify_flush(struct file *filp, fl_owner_t id)
{
	struct fsnotify_mark *fsn_mark;
	struct dnotify_mark *dn_mark;
	struct dnotify_struct *dn;
	struct dnotify_struct **prev;
	struct inode *inode;
	bool free = false;

	inode = file_inode(filp);
	if (!S_ISDIR(inode->i_mode))
		return;

	fsn_mark = fsnotify_find_inode_mark(dnotify_group, inode);
	if (!fsn_mark)
		return;
	dn_mark = container_of(fsn_mark, struct dnotify_mark, fsn_mark);

	mutex_lock(&dnotify_group->mark_mutex);

	spin_lock(&fsn_mark->lock);
	prev = &dn_mark->dn;
	while ((dn = *prev) != NULL) {
		if ((dn->dn_owner == id) && (dn->dn_filp == filp)) {
			*prev = dn->dn_next;
			kmem_cache_free(dnotify_struct_cache, dn);
			dnotify_recalc_inode_mask(fsn_mark);
			break;
		}
		prev = &dn->dn_next;
	}

	spin_unlock(&fsn_mark->lock);

	if (dn_mark->dn == NULL) {
		fsnotify_detach_mark(fsn_mark);
		free = true;
	}

	mutex_unlock(&dnotify_group->mark_mutex);

	if (free)
		fsnotify_free_mark(fsn_mark);
	fsnotify_put_mark(fsn_mark);
}

static __u32 convert_arg(unsigned long arg)
{
	__u32 new_mask = FS_EVENT_ON_CHILD;

	if (arg & DN_MULTISHOT)
		new_mask |= FS_DN_MULTISHOT;
	if (arg & DN_DELETE)
		new_mask |= (FS_DELETE | FS_MOVED_FROM);
	if (arg & DN_MODIFY)
		new_mask |= FS_MODIFY;
	if (arg & DN_ACCESS)
		new_mask |= FS_ACCESS;
	if (arg & DN_ATTRIB)
		new_mask |= FS_ATTRIB;
	if (arg & DN_RENAME)
		new_mask |= FS_DN_RENAME;
	if (arg & DN_CREATE)
		new_mask |= (FS_CREATE | FS_MOVED_TO);

	return new_mask;
}

static int attach_dn(struct dnotify_struct *dn, struct dnotify_mark *dn_mark,
		     fl_owner_t id, int fd, struct file *filp, __u32 mask)
{
	struct dnotify_struct *odn;

	odn = dn_mark->dn;
	while (odn != NULL) {
		 
		if ((odn->dn_owner == id) && (odn->dn_filp == filp)) {
			odn->dn_fd = fd;
			odn->dn_mask |= mask;
			return -EEXIST;
		}
		odn = odn->dn_next;
	}

	dn->dn_mask = mask;
	dn->dn_fd = fd;
	dn->dn_filp = filp;
	dn->dn_owner = id;
	dn->dn_next = dn_mark->dn;
	dn_mark->dn = dn;

	return 0;
}

int fcntl_dirnotify(int fd, struct file *filp, unsigned long arg)
{
	struct dnotify_mark *new_dn_mark, *dn_mark;
	struct fsnotify_mark *new_fsn_mark, *fsn_mark;
	struct dnotify_struct *dn;
	struct inode *inode;
	fl_owner_t id = current->files;
	struct file *f;
	int destroy = 0, error = 0;
	__u32 mask;

	new_fsn_mark = NULL;
	dn = NULL;

	if (!dir_notify_enable) {
		error = -EINVAL;
		goto out_err;
	}

	if ((arg & ~DN_MULTISHOT) == 0) {
		dnotify_flush(filp, id);
		error = 0;
		goto out_err;
	}

	inode = file_inode(filp);
	if (!S_ISDIR(inode->i_mode)) {
		error = -ENOTDIR;
		goto out_err;
	}

	dn = kmem_cache_alloc(dnotify_struct_cache, GFP_KERNEL);
	if (!dn) {
		error = -ENOMEM;
		goto out_err;
	}

	new_dn_mark = kmem_cache_alloc(dnotify_mark_cache, GFP_KERNEL);
	if (!new_dn_mark) {
		error = -ENOMEM;
		goto out_err;
	}

	mask = convert_arg(arg);

	new_fsn_mark = &new_dn_mark->fsn_mark;
	fsnotify_init_mark(new_fsn_mark, dnotify_free_mark);
	new_fsn_mark->mask = mask;
	new_dn_mark->dn = NULL;

	mutex_lock(&dnotify_group->mark_mutex);

	fsn_mark = fsnotify_find_inode_mark(dnotify_group, inode);
	if (fsn_mark) {
		dn_mark = container_of(fsn_mark, struct dnotify_mark, fsn_mark);
		spin_lock(&fsn_mark->lock);
	} else {
		fsnotify_add_mark_locked(new_fsn_mark, dnotify_group, inode,
					 NULL, 0);
		spin_lock(&new_fsn_mark->lock);
		fsn_mark = new_fsn_mark;
		dn_mark = new_dn_mark;
		 
		new_fsn_mark = NULL;
	}

	rcu_read_lock();
	f = fcheck(fd);
	rcu_read_unlock();

	if (f != filp) {
		 
		if (dn_mark == new_dn_mark)
			destroy = 1;
		goto out;
	}

	__f_setown(filp, task_pid(current), PIDTYPE_PID, 0);

	error = attach_dn(dn, dn_mark, id, fd, filp, mask);
	 
	if (!error)
		dn = NULL;
	 
	else if (error == -EEXIST)
		error = 0;

	dnotify_recalc_inode_mask(fsn_mark);
out:
	spin_unlock(&fsn_mark->lock);

	if (destroy)
		fsnotify_detach_mark(fsn_mark);
	mutex_unlock(&dnotify_group->mark_mutex);
	if (destroy)
		fsnotify_free_mark(fsn_mark);
	fsnotify_put_mark(fsn_mark);
out_err:
	if (new_fsn_mark)
		fsnotify_put_mark(new_fsn_mark);
	if (dn)
		kmem_cache_free(dnotify_struct_cache, dn);
	return error;
}

static int __init dnotify_init(void)
{
	dnotify_struct_cache = KMEM_CACHE(dnotify_struct, SLAB_PANIC);
	dnotify_mark_cache = KMEM_CACHE(dnotify_mark, SLAB_PANIC);

	dnotify_group = fsnotify_alloc_group(&dnotify_fsnotify_ops);
	if (IS_ERR(dnotify_group))
		panic("unable to allocate fsnotify group for dnotify\n");
	return 0;
}

module_init(dnotify_init)
