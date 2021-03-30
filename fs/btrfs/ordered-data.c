#ifndef MY_ABC_HERE
#define MY_ABC_HERE
#endif
 
#include <linux/slab.h>
#include <linux/blkdev.h>
#include <linux/writeback.h>
#include <linux/pagevec.h>
#include "ctree.h"
#include "transaction.h"
#include "btrfs_inode.h"
#include "extent_io.h"
#include "disk-io.h"
#include "compression.h"

static struct kmem_cache *btrfs_ordered_extent_cache;

static u64 entry_end(struct btrfs_ordered_extent *entry)
{
	if (entry->file_offset + entry->len < entry->file_offset)
		return (u64)-1;
	return entry->file_offset + entry->len;
}

static struct rb_node *tree_insert(struct rb_root *root, u64 file_offset,
				   struct rb_node *node)
{
	struct rb_node **p = &root->rb_node;
	struct rb_node *parent = NULL;
	struct btrfs_ordered_extent *entry;

	while (*p) {
		parent = *p;
		entry = rb_entry(parent, struct btrfs_ordered_extent, rb_node);

		if (file_offset < entry->file_offset)
			p = &(*p)->rb_left;
		else if (file_offset >= entry_end(entry))
			p = &(*p)->rb_right;
		else
			return parent;
	}

	rb_link_node(node, parent, p);
	rb_insert_color(node, root);
	return NULL;
}

static void ordered_data_tree_panic(struct inode *inode, int errno,
					       u64 offset)
{
	struct btrfs_fs_info *fs_info = btrfs_sb(inode->i_sb);
	btrfs_panic(fs_info, errno, "Inconsistency in ordered tree at offset "
		    "%llu", offset);
}

static struct rb_node *__tree_search(struct rb_root *root, u64 file_offset,
				     struct rb_node **prev_ret)
{
	struct rb_node *n = root->rb_node;
	struct rb_node *prev = NULL;
	struct rb_node *test;
	struct btrfs_ordered_extent *entry;
	struct btrfs_ordered_extent *prev_entry = NULL;

	while (n) {
		entry = rb_entry(n, struct btrfs_ordered_extent, rb_node);
		prev = n;
		prev_entry = entry;

		if (file_offset < entry->file_offset)
			n = n->rb_left;
		else if (file_offset >= entry_end(entry))
			n = n->rb_right;
		else
			return n;
	}
	if (!prev_ret)
		return NULL;

	while (prev && file_offset >= entry_end(prev_entry)) {
		test = rb_next(prev);
		if (!test)
			break;
		prev_entry = rb_entry(test, struct btrfs_ordered_extent,
				      rb_node);
		if (file_offset < entry_end(prev_entry))
			break;

		prev = test;
	}
	if (prev)
		prev_entry = rb_entry(prev, struct btrfs_ordered_extent,
				      rb_node);
	while (prev && file_offset < entry_end(prev_entry)) {
		test = rb_prev(prev);
		if (!test)
			break;
		prev_entry = rb_entry(test, struct btrfs_ordered_extent,
				      rb_node);
		prev = test;
	}
	*prev_ret = prev;
	return NULL;
}

static int offset_in_entry(struct btrfs_ordered_extent *entry, u64 file_offset)
{
	if (file_offset < entry->file_offset ||
	    entry->file_offset + entry->len <= file_offset)
		return 0;
	return 1;
}

static int range_overlaps(struct btrfs_ordered_extent *entry, u64 file_offset,
			  u64 len)
{
	if (file_offset + len <= entry->file_offset ||
	    entry->file_offset + entry->len <= file_offset)
		return 0;
	return 1;
}

static inline struct rb_node *tree_search(struct btrfs_ordered_inode_tree *tree,
					  u64 file_offset)
{
	struct rb_root *root = &tree->tree;
	struct rb_node *prev = NULL;
	struct rb_node *ret;
	struct btrfs_ordered_extent *entry;

	if (tree->last) {
		entry = rb_entry(tree->last, struct btrfs_ordered_extent,
				 rb_node);
		if (offset_in_entry(entry, file_offset))
			return tree->last;
	}
	ret = __tree_search(root, file_offset, &prev);
	if (!ret)
		ret = prev;
	if (ret)
		tree->last = ret;
	return ret;
}

static int __btrfs_add_ordered_extent(struct inode *inode, u64 file_offset,
				      u64 start, u64 len, u64 disk_len,
				      int type, int dio, int compress_type)
{
	struct btrfs_root *root = BTRFS_I(inode)->root;
	struct btrfs_ordered_inode_tree *tree;
	struct rb_node *node;
	struct btrfs_ordered_extent *entry;

	tree = &BTRFS_I(inode)->ordered_tree;
	entry = kmem_cache_zalloc(btrfs_ordered_extent_cache, GFP_NOFS);
	if (!entry)
		return -ENOMEM;

	entry->file_offset = file_offset;
	entry->start = start;
	entry->len = len;
	entry->disk_len = disk_len;
	entry->bytes_left = len;
	entry->inode = igrab(inode);
	entry->compress_type = compress_type;
	entry->truncated_len = (u64)-1;
	if (type != BTRFS_ORDERED_IO_DONE && type != BTRFS_ORDERED_COMPLETE)
		set_bit(type, &entry->flags);

	if (dio)
		set_bit(BTRFS_ORDERED_DIRECT, &entry->flags);

	atomic_set(&entry->refs, 1);
	init_waitqueue_head(&entry->wait);
	INIT_LIST_HEAD(&entry->list);
	INIT_LIST_HEAD(&entry->root_extent_list);
	INIT_LIST_HEAD(&entry->work_list);
	init_completion(&entry->completion);
	INIT_LIST_HEAD(&entry->log_list);
	INIT_LIST_HEAD(&entry->trans_list);

	trace_btrfs_ordered_extent_add(inode, entry);

	spin_lock_irq(&tree->lock);
	node = tree_insert(&tree->tree, file_offset,
			   &entry->rb_node);
	if (node)
		ordered_data_tree_panic(inode, -EEXIST, file_offset);
	spin_unlock_irq(&tree->lock);

	spin_lock(&root->ordered_extent_lock);
	list_add_tail(&entry->root_extent_list,
		      &root->ordered_extents);

	root->nr_ordered_extents++;
#ifdef MY_ABC_HERE
	atomic_inc(&root->fs_info->syno_ordered_extent_nr);
#endif  
	if (root->nr_ordered_extents == 1) {
		spin_lock(&root->fs_info->ordered_root_lock);
		BUG_ON(!list_empty(&root->ordered_root));
		list_add_tail(&root->ordered_root,
			      &root->fs_info->ordered_roots);
		spin_unlock(&root->fs_info->ordered_root_lock);
	}
	spin_unlock(&root->ordered_extent_lock);

	return 0;
}

int btrfs_add_ordered_extent(struct inode *inode, u64 file_offset,
			     u64 start, u64 len, u64 disk_len, int type)
{
	return __btrfs_add_ordered_extent(inode, file_offset, start, len,
					  disk_len, type, 0,
					  BTRFS_COMPRESS_NONE);
}

int btrfs_add_ordered_extent_dio(struct inode *inode, u64 file_offset,
				 u64 start, u64 len, u64 disk_len, int type)
{
	return __btrfs_add_ordered_extent(inode, file_offset, start, len,
					  disk_len, type, 1,
					  BTRFS_COMPRESS_NONE);
}

int btrfs_add_ordered_extent_compress(struct inode *inode, u64 file_offset,
				      u64 start, u64 len, u64 disk_len,
				      int type, int compress_type)
{
	return __btrfs_add_ordered_extent(inode, file_offset, start, len,
					  disk_len, type, 0,
					  compress_type);
}

void btrfs_add_ordered_sum(struct inode *inode,
			   struct btrfs_ordered_extent *entry,
			   struct btrfs_ordered_sum *sum)
{
	struct btrfs_ordered_inode_tree *tree;

	tree = &BTRFS_I(inode)->ordered_tree;
	spin_lock_irq(&tree->lock);
	list_add_tail(&sum->list, &entry->list);
	spin_unlock_irq(&tree->lock);
}

int btrfs_dec_test_first_ordered_pending(struct inode *inode,
				   struct btrfs_ordered_extent **cached,
				   u64 *file_offset, u64 io_size, int uptodate)
{
	struct btrfs_ordered_inode_tree *tree;
	struct rb_node *node;
	struct btrfs_ordered_extent *entry = NULL;
	int ret;
	unsigned long flags;
	u64 dec_end;
	u64 dec_start;
	u64 to_dec;

	tree = &BTRFS_I(inode)->ordered_tree;
	spin_lock_irqsave(&tree->lock, flags);
	node = tree_search(tree, *file_offset);
	if (!node) {
		ret = 1;
		goto out;
	}

	entry = rb_entry(node, struct btrfs_ordered_extent, rb_node);
	if (!offset_in_entry(entry, *file_offset)) {
		ret = 1;
		goto out;
	}

	dec_start = max(*file_offset, entry->file_offset);
	dec_end = min(*file_offset + io_size, entry->file_offset +
		      entry->len);
	*file_offset = dec_end;
	if (dec_start > dec_end) {
		btrfs_crit(BTRFS_I(inode)->root->fs_info,
			"bad ordering dec_start %llu end %llu", dec_start, dec_end);
	}
	to_dec = dec_end - dec_start;
	if (to_dec > entry->bytes_left) {
		btrfs_crit(BTRFS_I(inode)->root->fs_info,
			"bad ordered accounting left %llu size %llu",
			entry->bytes_left, to_dec);
	}
	entry->bytes_left -= to_dec;
	if (!uptodate)
		set_bit(BTRFS_ORDERED_IOERR, &entry->flags);

	if (entry->bytes_left == 0) {
		ret = test_and_set_bit(BTRFS_ORDERED_IO_DONE, &entry->flags);
		 
		if (waitqueue_active(&entry->wait))
			wake_up(&entry->wait);
	} else {
		ret = 1;
	}
out:
	if (!ret && cached && entry) {
		*cached = entry;
		atomic_inc(&entry->refs);
	}
	spin_unlock_irqrestore(&tree->lock, flags);
	return ret == 0;
}

int btrfs_dec_test_ordered_pending(struct inode *inode,
				   struct btrfs_ordered_extent **cached,
				   u64 file_offset, u64 io_size, int uptodate)
{
	struct btrfs_ordered_inode_tree *tree;
	struct rb_node *node;
	struct btrfs_ordered_extent *entry = NULL;
	unsigned long flags;
	int ret;

	tree = &BTRFS_I(inode)->ordered_tree;
	spin_lock_irqsave(&tree->lock, flags);
	if (cached && *cached) {
		entry = *cached;
		goto have_entry;
	}

	node = tree_search(tree, file_offset);
	if (!node) {
		ret = 1;
		goto out;
	}

	entry = rb_entry(node, struct btrfs_ordered_extent, rb_node);
have_entry:
	if (!offset_in_entry(entry, file_offset)) {
		ret = 1;
		goto out;
	}

	if (io_size > entry->bytes_left) {
		btrfs_crit(BTRFS_I(inode)->root->fs_info,
			   "bad ordered accounting left %llu size %llu",
		       entry->bytes_left, io_size);
	}
	entry->bytes_left -= io_size;
	if (!uptodate)
		set_bit(BTRFS_ORDERED_IOERR, &entry->flags);

	if (entry->bytes_left == 0) {
		ret = test_and_set_bit(BTRFS_ORDERED_IO_DONE, &entry->flags);
		 
		if (waitqueue_active(&entry->wait))
			wake_up(&entry->wait);
	} else {
		ret = 1;
	}
out:
	if (!ret && cached && entry) {
		*cached = entry;
		atomic_inc(&entry->refs);
	}
	spin_unlock_irqrestore(&tree->lock, flags);
	return ret == 0;
}

void btrfs_get_logged_extents(struct inode *inode,
			      struct list_head *logged_list,
			      const loff_t start,
			      const loff_t end)
{
	struct btrfs_ordered_inode_tree *tree;
	struct btrfs_ordered_extent *ordered;
	struct rb_node *n;
	struct rb_node *prev;

	tree = &BTRFS_I(inode)->ordered_tree;
	spin_lock_irq(&tree->lock);
	n = __tree_search(&tree->tree, end, &prev);
	if (!n)
		n = prev;
	for (; n; n = rb_prev(n)) {
		ordered = rb_entry(n, struct btrfs_ordered_extent, rb_node);
		if (ordered->file_offset > end)
			continue;
		if (entry_end(ordered) <= start)
			break;
		if (test_and_set_bit(BTRFS_ORDERED_LOGGED, &ordered->flags))
			continue;
		list_add(&ordered->log_list, logged_list);
		atomic_inc(&ordered->refs);
	}
	spin_unlock_irq(&tree->lock);
}

void btrfs_put_logged_extents(struct list_head *logged_list)
{
	struct btrfs_ordered_extent *ordered;

	while (!list_empty(logged_list)) {
		ordered = list_first_entry(logged_list,
					   struct btrfs_ordered_extent,
					   log_list);
		list_del_init(&ordered->log_list);
		btrfs_put_ordered_extent(ordered);
	}
}

void btrfs_submit_logged_extents(struct list_head *logged_list,
				 struct btrfs_root *log)
{
	int index = log->log_transid % 2;

	spin_lock_irq(&log->log_extents_lock[index]);
	list_splice_tail(logged_list, &log->logged_list[index]);
	spin_unlock_irq(&log->log_extents_lock[index]);
}

void btrfs_wait_logged_extents(struct btrfs_trans_handle *trans,
			       struct btrfs_root *log, u64 transid)
{
	struct btrfs_ordered_extent *ordered;
	int index = transid % 2;

	spin_lock_irq(&log->log_extents_lock[index]);
	while (!list_empty(&log->logged_list[index])) {
		struct inode *inode;
		ordered = list_first_entry(&log->logged_list[index],
					   struct btrfs_ordered_extent,
					   log_list);
		list_del_init(&ordered->log_list);
		inode = ordered->inode;
		spin_unlock_irq(&log->log_extents_lock[index]);

		if (!test_bit(BTRFS_ORDERED_IO_DONE, &ordered->flags) &&
		    !test_bit(BTRFS_ORDERED_DIRECT, &ordered->flags)) {
			u64 start = ordered->file_offset;
			u64 end = ordered->file_offset + ordered->len - 1;

			WARN_ON(!inode);
			filemap_fdatawrite_range(inode->i_mapping, start, end);
		}
		wait_event(ordered->wait, test_bit(BTRFS_ORDERED_IO_DONE,
						   &ordered->flags));

		if (!test_bit(BTRFS_ORDERED_COMPLETE, &ordered->flags)) {
			struct btrfs_ordered_inode_tree *tree;

			tree = &BTRFS_I(inode)->ordered_tree;
			spin_lock_irq(&tree->lock);
			if (!test_bit(BTRFS_ORDERED_COMPLETE, &ordered->flags)) {
				set_bit(BTRFS_ORDERED_PENDING, &ordered->flags);
				atomic_inc(&trans->transaction->pending_ordered);
			}
			spin_unlock_irq(&tree->lock);
		}
		btrfs_put_ordered_extent(ordered);
		spin_lock_irq(&log->log_extents_lock[index]);
	}
	spin_unlock_irq(&log->log_extents_lock[index]);
}

void btrfs_free_logged_extents(struct btrfs_root *log, u64 transid)
{
	struct btrfs_ordered_extent *ordered;
	int index = transid % 2;

	spin_lock_irq(&log->log_extents_lock[index]);
	while (!list_empty(&log->logged_list[index])) {
		ordered = list_first_entry(&log->logged_list[index],
					   struct btrfs_ordered_extent,
					   log_list);
		list_del_init(&ordered->log_list);
		spin_unlock_irq(&log->log_extents_lock[index]);
		btrfs_put_ordered_extent(ordered);
		spin_lock_irq(&log->log_extents_lock[index]);
	}
	spin_unlock_irq(&log->log_extents_lock[index]);
}

void btrfs_put_ordered_extent(struct btrfs_ordered_extent *entry)
{
	struct list_head *cur;
	struct btrfs_ordered_sum *sum;

	trace_btrfs_ordered_extent_put(entry->inode, entry);

	if (atomic_dec_and_test(&entry->refs)) {
		ASSERT(list_empty(&entry->log_list));
		ASSERT(list_empty(&entry->trans_list));
		ASSERT(list_empty(&entry->root_extent_list));
		ASSERT(RB_EMPTY_NODE(&entry->rb_node));
		if (entry->inode)
			btrfs_add_delayed_iput(entry->inode);
		while (!list_empty(&entry->list)) {
			cur = entry->list.next;
			sum = list_entry(cur, struct btrfs_ordered_sum, list);
			list_del(&sum->list);
			kfree(sum);
		}
		kmem_cache_free(btrfs_ordered_extent_cache, entry);
	}
}

void btrfs_remove_ordered_extent(struct inode *inode,
				 struct btrfs_ordered_extent *entry)
{
	struct btrfs_ordered_inode_tree *tree;
	struct btrfs_root *root = BTRFS_I(inode)->root;
	struct rb_node *node;
	bool dec_pending_ordered = false;

	tree = &BTRFS_I(inode)->ordered_tree;
	spin_lock_irq(&tree->lock);
	node = &entry->rb_node;
	rb_erase(node, &tree->tree);
	RB_CLEAR_NODE(node);
	if (tree->last == node)
		tree->last = NULL;
	set_bit(BTRFS_ORDERED_COMPLETE, &entry->flags);
	if (test_and_clear_bit(BTRFS_ORDERED_PENDING, &entry->flags))
		dec_pending_ordered = true;
	spin_unlock_irq(&tree->lock);

	if (dec_pending_ordered) {
		struct btrfs_transaction *trans;

		spin_lock(&root->fs_info->trans_lock);
		trans = root->fs_info->running_transaction;
		if (trans)
			atomic_inc(&trans->use_count);
		spin_unlock(&root->fs_info->trans_lock);

		ASSERT(trans);
		if (trans) {
			if (atomic_dec_and_test(&trans->pending_ordered))
				wake_up(&trans->pending_wait);
			btrfs_put_transaction(trans);
		}
	}

	spin_lock(&root->ordered_extent_lock);
	list_del_init(&entry->root_extent_list);

	root->nr_ordered_extents--;
#ifdef MY_ABC_HERE
	atomic_dec(&root->fs_info->syno_ordered_extent_nr);
#endif  

	trace_btrfs_ordered_extent_remove(inode, entry);

	if (!root->nr_ordered_extents) {
		spin_lock(&root->fs_info->ordered_root_lock);
		BUG_ON(list_empty(&root->ordered_root));
		list_del_init(&root->ordered_root);
		spin_unlock(&root->fs_info->ordered_root_lock);
	}
	spin_unlock(&root->ordered_extent_lock);
	wake_up(&entry->wait);
#ifdef MY_ABC_HERE
	if (waitqueue_active(&root->fs_info->syno_ordered_queue_wait))
		wake_up(&root->fs_info->syno_ordered_queue_wait);
#endif  
}

static void btrfs_run_ordered_extent_work(struct btrfs_work *work)
{
	struct btrfs_ordered_extent *ordered;

	ordered = container_of(work, struct btrfs_ordered_extent, flush_work);
#ifdef MY_ABC_HERE
	btrfs_start_ordered_extent(ordered->inode, ordered, 2);
#else
	btrfs_start_ordered_extent(ordered->inode, ordered, 1);
#endif  
	complete(&ordered->completion);
}

int btrfs_wait_ordered_extents(struct btrfs_root *root, int nr,
			       const u64 range_start, const u64 range_len)
{
	LIST_HEAD(splice);
	LIST_HEAD(skipped);
	LIST_HEAD(works);
	struct btrfs_ordered_extent *ordered, *next;
	int count = 0;
	const u64 range_end = range_start + range_len;

	mutex_lock(&root->ordered_extent_mutex);
	spin_lock(&root->ordered_extent_lock);
	list_splice_init(&root->ordered_extents, &splice);
	while (!list_empty(&splice) && nr) {
		ordered = list_first_entry(&splice, struct btrfs_ordered_extent,
					   root_extent_list);

		if (range_end <= ordered->start ||
		    ordered->start + ordered->disk_len <= range_start) {
			list_move_tail(&ordered->root_extent_list, &skipped);
			cond_resched_lock(&root->ordered_extent_lock);
			continue;
		}

		list_move_tail(&ordered->root_extent_list,
			       &root->ordered_extents);
		atomic_inc(&ordered->refs);
		spin_unlock(&root->ordered_extent_lock);

		btrfs_init_work(&ordered->flush_work,
				btrfs_flush_delalloc_helper,
				btrfs_run_ordered_extent_work, NULL, NULL);
		list_add_tail(&ordered->work_list, &works);
		btrfs_queue_work(root->fs_info->flush_workers,
				 &ordered->flush_work);

		cond_resched();
		spin_lock(&root->ordered_extent_lock);
		if (nr != -1)
			nr--;
		count++;
	}
	list_splice_tail(&skipped, &root->ordered_extents);
	list_splice_tail(&splice, &root->ordered_extents);
	spin_unlock(&root->ordered_extent_lock);

	list_for_each_entry_safe(ordered, next, &works, work_list) {
		list_del_init(&ordered->work_list);
		wait_for_completion(&ordered->completion);
		btrfs_put_ordered_extent(ordered);
		cond_resched();
	}
	mutex_unlock(&root->ordered_extent_mutex);

	return count;
}

void btrfs_wait_ordered_roots(struct btrfs_fs_info *fs_info, int nr,
			      const u64 range_start, const u64 range_len)
{
	struct btrfs_root *root;
	struct list_head splice;
	int done;

	INIT_LIST_HEAD(&splice);

	mutex_lock(&fs_info->ordered_operations_mutex);
	spin_lock(&fs_info->ordered_root_lock);
	list_splice_init(&fs_info->ordered_roots, &splice);
	while (!list_empty(&splice) && nr) {
		root = list_first_entry(&splice, struct btrfs_root,
					ordered_root);
		root = btrfs_grab_fs_root(root);
		BUG_ON(!root);
		list_move_tail(&root->ordered_root,
			       &fs_info->ordered_roots);
		spin_unlock(&fs_info->ordered_root_lock);

		done = btrfs_wait_ordered_extents(root, nr,
						  range_start, range_len);
		btrfs_put_fs_root(root);

		spin_lock(&fs_info->ordered_root_lock);
		if (nr != -1) {
			nr -= done;
			WARN_ON(nr < 0);
		}
	}
	list_splice_tail(&splice, &fs_info->ordered_roots);
	spin_unlock(&fs_info->ordered_root_lock);
	mutex_unlock(&fs_info->ordered_operations_mutex);
}

void btrfs_start_ordered_extent(struct inode *inode,
				       struct btrfs_ordered_extent *entry,
				       int wait)
{
#ifdef MY_ABC_HERE
	struct btrfs_root *root = BTRFS_I(entry->inode)->root;
#endif  
	u64 start = entry->file_offset;
	u64 end = start + entry->len - 1;

	trace_btrfs_ordered_extent_start(inode, entry);

#ifdef MY_ABC_HERE
	if (wait) {
		entry->high_priority = 1;
		if (test_bit(BTRFS_ORDERED_WORK_INITIALIZED, &entry->flags) && work_pending(&entry->work.normal_work) &&
			!test_and_set_bit(BTRFS_ORDERED_HIGH_PRIORITY, &entry->flags)) {
			if (cancel_work_sync(&entry->work.normal_work)) {
				btrfs_queue_work(BTRFS_I(inode)->root->fs_info->syno_high_priority_endio_workers, &entry->work);
			}
		}
	}
#endif  

	if (!test_bit(BTRFS_ORDERED_DIRECT, &entry->flags))
#ifdef MY_ABC_HERE
	{
		if (2 == wait)mutex_lock(&root->ordered_extent_worker_mutex);
		filemap_fdatawrite_range(inode->i_mapping, start, end);
		if (2 == wait)mutex_unlock(&root->ordered_extent_worker_mutex);
	}
#else
		filemap_fdatawrite_range(inode->i_mapping, start, end);
#endif  
	if (wait) {
		wait_event(entry->wait, test_bit(BTRFS_ORDERED_COMPLETE,
						 &entry->flags));
	}
}

int btrfs_wait_ordered_range(struct inode *inode, u64 start, u64 len)
{
	int ret = 0;
	int ret_wb = 0;
	u64 end;
	u64 orig_end;
	struct btrfs_ordered_extent *ordered;

	if (start + len < start) {
		orig_end = INT_LIMIT(loff_t);
	} else {
		orig_end = start + len - 1;
		if (orig_end > INT_LIMIT(loff_t))
			orig_end = INT_LIMIT(loff_t);
	}

	ret = btrfs_fdatawrite_range(inode, start, orig_end);
	if (ret)
		return ret;

	ret_wb = filemap_fdatawait_range(inode->i_mapping, start, orig_end);

	end = orig_end;
	while (1) {
		ordered = btrfs_lookup_first_ordered_extent(inode, end);
		if (!ordered)
			break;
		if (ordered->file_offset > orig_end) {
			btrfs_put_ordered_extent(ordered);
			break;
		}
		if (ordered->file_offset + ordered->len <= start) {
			btrfs_put_ordered_extent(ordered);
			break;
		}
		btrfs_start_ordered_extent(inode, ordered, 1);
		end = ordered->file_offset;
		if (test_bit(BTRFS_ORDERED_IOERR, &ordered->flags))
			ret = -EIO;
		btrfs_put_ordered_extent(ordered);
		if (ret || end == 0 || end == start)
			break;
		end--;
	}
	return ret_wb ? ret_wb : ret;
}

struct btrfs_ordered_extent *btrfs_lookup_ordered_extent(struct inode *inode,
							 u64 file_offset)
{
	struct btrfs_ordered_inode_tree *tree;
	struct rb_node *node;
	struct btrfs_ordered_extent *entry = NULL;

	tree = &BTRFS_I(inode)->ordered_tree;
	spin_lock_irq(&tree->lock);
	node = tree_search(tree, file_offset);
	if (!node)
		goto out;

	entry = rb_entry(node, struct btrfs_ordered_extent, rb_node);
	if (!offset_in_entry(entry, file_offset))
		entry = NULL;
	if (entry)
		atomic_inc(&entry->refs);
out:
	spin_unlock_irq(&tree->lock);
	return entry;
}

struct btrfs_ordered_extent *btrfs_lookup_ordered_range(struct inode *inode,
							u64 file_offset,
							u64 len)
{
	struct btrfs_ordered_inode_tree *tree;
	struct rb_node *node;
	struct btrfs_ordered_extent *entry = NULL;

	tree = &BTRFS_I(inode)->ordered_tree;
	spin_lock_irq(&tree->lock);
	node = tree_search(tree, file_offset);
	if (!node) {
		node = tree_search(tree, file_offset + len);
		if (!node)
			goto out;
	}

	while (1) {
		entry = rb_entry(node, struct btrfs_ordered_extent, rb_node);
		if (range_overlaps(entry, file_offset, len))
			break;

		if (entry->file_offset >= file_offset + len) {
			entry = NULL;
			break;
		}
		entry = NULL;
		node = rb_next(node);
		if (!node)
			break;
	}
out:
	if (entry)
		atomic_inc(&entry->refs);
	spin_unlock_irq(&tree->lock);
	return entry;
}

bool btrfs_have_ordered_extents_in_range(struct inode *inode,
					 u64 file_offset,
					 u64 len)
{
	struct btrfs_ordered_extent *oe;

	oe = btrfs_lookup_ordered_range(inode, file_offset, len);
	if (oe) {
		btrfs_put_ordered_extent(oe);
		return true;
	}
	return false;
}

struct btrfs_ordered_extent *
btrfs_lookup_first_ordered_extent(struct inode *inode, u64 file_offset)
{
	struct btrfs_ordered_inode_tree *tree;
	struct rb_node *node;
	struct btrfs_ordered_extent *entry = NULL;

	tree = &BTRFS_I(inode)->ordered_tree;
	spin_lock_irq(&tree->lock);
	node = tree_search(tree, file_offset);
	if (!node)
		goto out;

	entry = rb_entry(node, struct btrfs_ordered_extent, rb_node);
	atomic_inc(&entry->refs);
out:
	spin_unlock_irq(&tree->lock);
	return entry;
}

int btrfs_ordered_update_i_size(struct inode *inode, u64 offset,
				struct btrfs_ordered_extent *ordered)
{
	struct btrfs_ordered_inode_tree *tree = &BTRFS_I(inode)->ordered_tree;
	u64 disk_i_size;
	u64 new_i_size;
	u64 i_size = i_size_read(inode);
	struct rb_node *node;
	struct rb_node *prev = NULL;
	struct btrfs_ordered_extent *test;
	int ret = 1;
	u64 orig_offset = offset;

	spin_lock_irq(&tree->lock);
	if (ordered) {
		offset = entry_end(ordered);
		if (test_bit(BTRFS_ORDERED_TRUNCATED, &ordered->flags))
			offset = min(offset,
				     ordered->file_offset +
				     ordered->truncated_len);
	} else {
		offset = ALIGN(offset, BTRFS_I(inode)->root->sectorsize);
	}
	disk_i_size = BTRFS_I(inode)->disk_i_size;

	if (disk_i_size > i_size) {
		BTRFS_I(inode)->disk_i_size = orig_offset;
		ret = 0;
		goto out;
	}

	if (disk_i_size == i_size)
		goto out;

	if (offset <= disk_i_size &&
	    (!ordered || ordered->outstanding_isize <= disk_i_size))
		goto out;

	if (ordered) {
		node = rb_prev(&ordered->rb_node);
	} else {
		prev = tree_search(tree, offset);
		 
		if (prev) {
			test = rb_entry(prev, struct btrfs_ordered_extent,
					rb_node);
			BUG_ON(offset_in_entry(test, offset));
		}
		node = prev;
	}
	for (; node; node = rb_prev(node)) {
		test = rb_entry(node, struct btrfs_ordered_extent, rb_node);

		if (test_bit(BTRFS_ORDERED_UPDATED_ISIZE, &test->flags))
			continue;
		if (test->file_offset + test->len <= disk_i_size)
			break;
		if (test->file_offset >= i_size)
			break;
		if (entry_end(test) > disk_i_size) {
			 
			if (test->outstanding_isize < offset)
				test->outstanding_isize = offset;
			if (ordered &&
			    ordered->outstanding_isize >
			    test->outstanding_isize)
				test->outstanding_isize =
						ordered->outstanding_isize;
			goto out;
		}
	}
	new_i_size = min_t(u64, offset, i_size);

	if (ordered && ordered->outstanding_isize > new_i_size)
		new_i_size = min_t(u64, ordered->outstanding_isize, i_size);
	BTRFS_I(inode)->disk_i_size = new_i_size;
	ret = 0;
out:
	 
	if (ordered)
		set_bit(BTRFS_ORDERED_UPDATED_ISIZE, &ordered->flags);
	spin_unlock_irq(&tree->lock);
	return ret;
}

int btrfs_find_ordered_sum(struct inode *inode, u64 offset, u64 disk_bytenr,
			   u32 *sum, int len)
{
	struct btrfs_ordered_sum *ordered_sum;
	struct btrfs_ordered_extent *ordered;
	struct btrfs_ordered_inode_tree *tree = &BTRFS_I(inode)->ordered_tree;
	unsigned long num_sectors;
	unsigned long i;
	u32 sectorsize = BTRFS_I(inode)->root->sectorsize;
	int index = 0;

	ordered = btrfs_lookup_ordered_extent(inode, offset);
	if (!ordered)
		return 0;

	spin_lock_irq(&tree->lock);
	list_for_each_entry_reverse(ordered_sum, &ordered->list, list) {
		if (disk_bytenr >= ordered_sum->bytenr &&
		    disk_bytenr < ordered_sum->bytenr + ordered_sum->len) {
			i = (disk_bytenr - ordered_sum->bytenr) >>
			    inode->i_sb->s_blocksize_bits;
			num_sectors = ordered_sum->len >>
				      inode->i_sb->s_blocksize_bits;
			num_sectors = min_t(int, len - index, num_sectors - i);
			memcpy(sum + index, ordered_sum->sums + i,
			       num_sectors);

			index += (int)num_sectors;
			if (index == len)
				goto out;
			disk_bytenr += num_sectors * sectorsize;
		}
	}
out:
	spin_unlock_irq(&tree->lock);
	btrfs_put_ordered_extent(ordered);
	return index;
}

int __init ordered_data_init(void)
{
	btrfs_ordered_extent_cache = kmem_cache_create("btrfs_ordered_extent",
				     sizeof(struct btrfs_ordered_extent), 0,
				     SLAB_RECLAIM_ACCOUNT | SLAB_MEM_SPREAD,
				     NULL);
	if (!btrfs_ordered_extent_cache)
		return -ENOMEM;

	return 0;
}

void ordered_data_exit(void)
{
	kmem_cache_destroy(btrfs_ordered_extent_cache);
}