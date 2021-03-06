#ifndef MY_ABC_HERE
#define MY_ABC_HERE
#endif
 
#ifndef __LINUX_FSNOTIFY_BACKEND_H
#define __LINUX_FSNOTIFY_BACKEND_H

#ifdef __KERNEL__

#include <linux/idr.h>  
#include <linux/fs.h>  
#include <linux/list.h>
#include <linux/path.h>  
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/atomic.h>

#define FS_ACCESS		0x00000001	 
#define FS_MODIFY		0x00000002	 
#define FS_ATTRIB		0x00000004	 
#define FS_CLOSE_WRITE		0x00000008	 
#define FS_CLOSE_NOWRITE	0x00000010	 
#define FS_OPEN			0x00000020	 
#define FS_MOVED_FROM		0x00000040	 
#define FS_MOVED_TO		0x00000080	 
#define FS_CREATE		0x00000100	 
#define FS_DELETE		0x00000200	 
#define FS_DELETE_SELF		0x00000400	 
#define FS_MOVE_SELF		0x00000800	 

#define FS_UNMOUNT		0x00002000	 
#define FS_Q_OVERFLOW		0x00004000	 
#define FS_IN_IGNORED		0x00008000	 

#define FS_OPEN_PERM		0x00010000	 
#define FS_ACCESS_PERM		0x00020000	 

#define FS_EXCL_UNLINK		0x04000000	 
#define FS_ISDIR		0x40000000	 
#define FS_IN_ONESHOT		0x80000000	 

#define FS_DN_RENAME		0x10000000	 
#define FS_DN_MULTISHOT		0x20000000	 

#define FS_EVENT_ON_CHILD	0x08000000

#define FS_EVENTS_POSS_ON_CHILD   (FS_ACCESS | FS_MODIFY | FS_ATTRIB |\
				   FS_CLOSE_WRITE | FS_CLOSE_NOWRITE | FS_OPEN |\
				   FS_MOVED_FROM | FS_MOVED_TO | FS_CREATE |\
				   FS_DELETE | FS_OPEN_PERM | FS_ACCESS_PERM)

#define FS_MOVE			(FS_MOVED_FROM | FS_MOVED_TO)

#define ALL_FSNOTIFY_PERM_EVENTS (FS_OPEN_PERM | FS_ACCESS_PERM)

#define ALL_FSNOTIFY_EVENTS (FS_ACCESS | FS_MODIFY | FS_ATTRIB | \
			     FS_CLOSE_WRITE | FS_CLOSE_NOWRITE | FS_OPEN | \
			     FS_MOVED_FROM | FS_MOVED_TO | FS_CREATE | \
			     FS_DELETE | FS_DELETE_SELF | FS_MOVE_SELF | \
			     FS_UNMOUNT | FS_Q_OVERFLOW | FS_IN_IGNORED | \
			     FS_OPEN_PERM | FS_ACCESS_PERM | FS_EXCL_UNLINK | \
			     FS_ISDIR | FS_IN_ONESHOT | FS_DN_RENAME | \
			     FS_DN_MULTISHOT | FS_EVENT_ON_CHILD)

struct fsnotify_group;
struct fsnotify_event;
struct fsnotify_mark;
struct fsnotify_event_private_data;
struct fsnotify_fname;

struct fsnotify_ops {
	int (*handle_event)(struct fsnotify_group *group,
			    struct inode *inode,
			    struct fsnotify_mark *inode_mark,
			    struct fsnotify_mark *vfsmount_mark,
			    u32 mask, void *data, int data_type,
			    const unsigned char *file_name, u32 cookie);
	void (*free_group_priv)(struct fsnotify_group *group);
	void (*freeing_mark)(struct fsnotify_mark *mark, struct fsnotify_group *group);
	void (*free_event)(struct fsnotify_event *event);
#ifdef MY_ABC_HERE
	int (*fetch_name)(struct fsnotify_event *event);
#endif  
};

struct fsnotify_event {
	struct list_head list;
	 
	struct inode *inode;	 
	u32 mask;		 
};

struct fsnotify_group {
	 
	atomic_t refcnt;		 

	const struct fsnotify_ops *ops;	 

	struct mutex notification_mutex;	 
	struct list_head notification_list;	 
	wait_queue_head_t notification_waitq;	 
	unsigned int q_len;			 
	unsigned int max_events;		 
	 
	#define FS_PRIO_0	0  
	#define FS_PRIO_1	1  
	#define FS_PRIO_2	2  
	unsigned int priority;
	bool shutdown;		 

	struct mutex mark_mutex;	 
	atomic_t num_marks;		 
	struct list_head marks_list;	 

	struct fasync_struct *fsn_fa;     

	struct fsnotify_event *overflow_event;	 

	union {
		void *private;
#ifdef CONFIG_INOTIFY_USER
		struct inotify_group_private_data {
			spinlock_t	idr_lock;
			struct idr      idr;
			struct user_struct      *user;
		} inotify_data;
#endif
#ifdef MY_ABC_HERE
		struct synotify_group_private_data {
			struct user_struct *user;
			unsigned int max_watchers;
		} synotify_data;
#endif  
#ifdef CONFIG_FANOTIFY
		struct fanotify_group_private_data {
#ifdef CONFIG_FANOTIFY_ACCESS_PERMISSIONS
			 
			spinlock_t access_lock;
			struct list_head access_list;
			wait_queue_head_t access_waitq;
#endif  
			int f_flags;
			unsigned int max_marks;
			struct user_struct *user;
		} fanotify_data;
#endif  
	};
};

#define FSNOTIFY_EVENT_NONE	0
#define FSNOTIFY_EVENT_PATH	1
#define FSNOTIFY_EVENT_INODE	2
#ifdef MY_ABC_HERE
#define FSNOTIFY_EVENT_SYNO	3
#endif  

struct fsnotify_mark {
	 
	__u32 mask;
	 
	atomic_t refcnt;
	 
	struct fsnotify_group *group;
	 
	struct list_head g_list;
	 
	spinlock_t lock;
	 
	struct hlist_node obj_list;
	union {	 
		struct inode *inode;	 
		struct vfsmount *mnt;	 
	};
	 
	__u32 ignored_mask;
#define FSNOTIFY_MARK_FLAG_INODE		0x01
#define FSNOTIFY_MARK_FLAG_VFSMOUNT		0x02
#define FSNOTIFY_MARK_FLAG_OBJECT_PINNED	0x04
#define FSNOTIFY_MARK_FLAG_IGNORED_SURV_MODIFY	0x08
#define FSNOTIFY_MARK_FLAG_ALIVE		0x10
#define FSNOTIFY_MARK_FLAG_ATTACHED		0x20
	unsigned int flags;		 
	void (*free_mark)(struct fsnotify_mark *mark);  
};

#ifdef CONFIG_FSNOTIFY

extern int fsnotify(struct inode *to_tell, __u32 mask, void *data, int data_is,
		    const unsigned char *name, u32 cookie);
extern int __fsnotify_parent(struct path *path, struct dentry *dentry, __u32 mask);
extern void __fsnotify_inode_delete(struct inode *inode);
extern void __fsnotify_vfsmount_delete(struct vfsmount *mnt);
extern u32 fsnotify_get_cookie(void);
#ifdef MY_ABC_HERE
extern int SYNOFsnotify(__u32 mask, void *data, int data_is,
	     const unsigned char *file_name, u32 cookie);
#endif  

static inline int fsnotify_inode_watches_children(struct inode *inode)
{
	 
	if (!(inode->i_fsnotify_mask & FS_EVENT_ON_CHILD))
		return 0;
	 
	return inode->i_fsnotify_mask & FS_EVENTS_POSS_ON_CHILD;
}

static inline void __fsnotify_update_dcache_flags(struct dentry *dentry)
{
	struct dentry *parent;

	assert_spin_locked(&dentry->d_lock);

	parent = dentry->d_parent;
	if (parent->d_inode && fsnotify_inode_watches_children(parent->d_inode))
		dentry->d_flags |= DCACHE_FSNOTIFY_PARENT_WATCHED;
	else
		dentry->d_flags &= ~DCACHE_FSNOTIFY_PARENT_WATCHED;
}

static inline void __fsnotify_d_instantiate(struct dentry *dentry, struct inode *inode)
{
	if (!inode)
		return;

	spin_lock(&dentry->d_lock);
	__fsnotify_update_dcache_flags(dentry);
	spin_unlock(&dentry->d_lock);
}

extern struct fsnotify_group *fsnotify_alloc_group(const struct fsnotify_ops *ops);
 
extern void fsnotify_get_group(struct fsnotify_group *group);
 
extern void fsnotify_put_group(struct fsnotify_group *group);
 
extern void fsnotify_group_stop_queueing(struct fsnotify_group *group);
 
extern void fsnotify_destroy_group(struct fsnotify_group *group);
 
extern int fsnotify_fasync(int fd, struct file *file, int on);
 
extern void fsnotify_destroy_event(struct fsnotify_group *group,
				   struct fsnotify_event *event);
 
extern int fsnotify_add_event(struct fsnotify_group *group,
			      struct fsnotify_event *event,
			      int (*merge)(struct list_head *,
					   struct fsnotify_event *));
 
extern bool fsnotify_notify_queue_is_empty(struct fsnotify_group *group);
 
extern struct fsnotify_event *fsnotify_peek_first_event(struct fsnotify_group *group);
 
extern struct fsnotify_event *fsnotify_remove_first_event(struct fsnotify_group *group);

extern void fsnotify_recalc_vfsmount_mask(struct vfsmount *mnt);
 
extern void fsnotify_recalc_inode_mask(struct inode *inode);
extern void fsnotify_init_mark(struct fsnotify_mark *mark, void (*free_mark)(struct fsnotify_mark *mark));
 
extern struct fsnotify_mark *fsnotify_find_inode_mark(struct fsnotify_group *group, struct inode *inode);
 
extern struct fsnotify_mark *fsnotify_find_vfsmount_mark(struct fsnotify_group *group, struct vfsmount *mnt);
 
extern void fsnotify_duplicate_mark(struct fsnotify_mark *new, struct fsnotify_mark *old);
 
extern void fsnotify_set_mark_ignored_mask_locked(struct fsnotify_mark *mark, __u32 mask);
 
extern void fsnotify_set_mark_mask_locked(struct fsnotify_mark *mark, __u32 mask);
 
extern int fsnotify_add_mark(struct fsnotify_mark *mark, struct fsnotify_group *group,
			     struct inode *inode, struct vfsmount *mnt, int allow_dups);
extern int fsnotify_add_mark_locked(struct fsnotify_mark *mark, struct fsnotify_group *group,
				    struct inode *inode, struct vfsmount *mnt, int allow_dups);
 
extern void fsnotify_destroy_mark(struct fsnotify_mark *mark,
				  struct fsnotify_group *group);
 
extern void fsnotify_detach_mark(struct fsnotify_mark *mark);
 
extern void fsnotify_free_mark(struct fsnotify_mark *mark);
 
extern void fsnotify_clear_vfsmount_marks_by_group(struct fsnotify_group *group);
 
extern void fsnotify_clear_inode_marks_by_group(struct fsnotify_group *group);
 
extern void fsnotify_clear_marks_by_group_flags(struct fsnotify_group *group, unsigned int flags);
 
extern void fsnotify_clear_marks_by_group(struct fsnotify_group *group);
extern void fsnotify_get_mark(struct fsnotify_mark *mark);
extern void fsnotify_put_mark(struct fsnotify_mark *mark);
extern void fsnotify_unmount_inodes(struct super_block *sb);

extern void fsnotify_init_event(struct fsnotify_event *event,
				struct inode *to_tell, u32 mask);

#else

static inline int fsnotify(struct inode *to_tell, __u32 mask, void *data, int data_is,
			   const unsigned char *name, u32 cookie)
{
	return 0;
}

static inline int __fsnotify_parent(struct path *path, struct dentry *dentry, __u32 mask)
{
	return 0;
}

static inline void __fsnotify_inode_delete(struct inode *inode)
{}

static inline void __fsnotify_vfsmount_delete(struct vfsmount *mnt)
{}

static inline void __fsnotify_update_dcache_flags(struct dentry *dentry)
{}

static inline void __fsnotify_d_instantiate(struct dentry *dentry, struct inode *inode)
{}

static inline u32 fsnotify_get_cookie(void)
{
	return 0;
}

static inline void fsnotify_unmount_inodes(struct super_block *sb)
{}

#endif	 

#endif	 

#endif	 
