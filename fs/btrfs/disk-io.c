#ifndef MY_ABC_HERE
#define MY_ABC_HERE
#endif
 
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/scatterlist.h>
#include <linux/swap.h>
#include <linux/radix-tree.h>
#include <linux/writeback.h>
#include <linux/buffer_head.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/migrate.h>
#include <linux/ratelimit.h>
#include <linux/uuid.h>
#include <linux/semaphore.h>
#include <asm/unaligned.h>
#include "ctree.h"
#include "disk-io.h"
#include "hash.h"
#include "transaction.h"
#include "btrfs_inode.h"
#include "volumes.h"
#include "print-tree.h"
#include "locking.h"
#include "tree-log.h"
#include "free-space-cache.h"
#include "free-space-tree.h"
#include "inode-map.h"
#include "check-integrity.h"
#include "rcu-string.h"
#include "dev-replace.h"
#include "raid56.h"
#include "sysfs.h"
#include "qgroup.h"
#include "compression.h"

#ifdef CONFIG_X86
#include <asm/cpufeature.h>
#endif

#define BTRFS_SUPER_FLAG_SUPP	(BTRFS_HEADER_FLAG_WRITTEN |\
				 BTRFS_HEADER_FLAG_RELOC |\
				 BTRFS_SUPER_FLAG_ERROR |\
				 BTRFS_SUPER_FLAG_SEEDING |\
				 BTRFS_SUPER_FLAG_METADUMP)

static const struct extent_io_ops btree_extent_io_ops;
static void end_workqueue_fn(struct btrfs_work *work);
static void free_fs_root(struct btrfs_root *root);
static void btrfs_destroy_ordered_extents(struct btrfs_root *root);
static int btrfs_destroy_delayed_refs(struct btrfs_transaction *trans,
				      struct btrfs_root *root);
static void btrfs_destroy_delalloc_inodes(struct btrfs_root *root);
static int btrfs_destroy_marked_extents(struct btrfs_root *root,
					struct extent_io_tree *dirty_pages,
					int mark);
static int btrfs_destroy_pinned_extent(struct btrfs_root *root,
				       struct extent_io_tree *pinned_extents);
static int btrfs_cleanup_transaction(struct btrfs_root *root);
static void btrfs_error_commit_super(struct btrfs_root *root);

struct btrfs_end_io_wq {
	struct bio *bio;
	bio_end_io_t *end_io;
	void *private;
	struct btrfs_fs_info *info;
	int error;
	enum btrfs_wq_endio_type metadata;
	struct list_head list;
	struct btrfs_work work;
};

static struct kmem_cache *btrfs_end_io_wq_cache;

int __init btrfs_end_io_wq_init(void)
{
	btrfs_end_io_wq_cache = kmem_cache_create("btrfs_end_io_wq",
					sizeof(struct btrfs_end_io_wq),
					0,
					SLAB_RECLAIM_ACCOUNT | SLAB_MEM_SPREAD,
					NULL);
	if (!btrfs_end_io_wq_cache)
		return -ENOMEM;
	return 0;
}

void btrfs_end_io_wq_exit(void)
{
	kmem_cache_destroy(btrfs_end_io_wq_cache);
}

struct async_submit_bio {
	struct inode *inode;
	struct bio *bio;
	struct list_head list;
	extent_submit_bio_hook_t *submit_bio_start;
	extent_submit_bio_hook_t *submit_bio_done;
	int rw;
	int mirror_num;
	unsigned long bio_flags;
	 
	u64 bio_offset;
	struct btrfs_work work;
	int error;
#ifdef MY_ABC_HERE
	int throttle;
#endif  
};

#ifdef CONFIG_DEBUG_LOCK_ALLOC
# if BTRFS_MAX_LEVEL != 8
#  error
# endif

static struct btrfs_lockdep_keyset {
	u64			id;		 
	const char		*name_stem;	 
	char			names[BTRFS_MAX_LEVEL + 1][20];
	struct lock_class_key	keys[BTRFS_MAX_LEVEL + 1];
} btrfs_lockdep_keysets[] = {
	{ .id = BTRFS_ROOT_TREE_OBJECTID,	.name_stem = "root"	},
	{ .id = BTRFS_EXTENT_TREE_OBJECTID,	.name_stem = "extent"	},
	{ .id = BTRFS_CHUNK_TREE_OBJECTID,	.name_stem = "chunk"	},
	{ .id = BTRFS_DEV_TREE_OBJECTID,	.name_stem = "dev"	},
	{ .id = BTRFS_FS_TREE_OBJECTID,		.name_stem = "fs"	},
	{ .id = BTRFS_CSUM_TREE_OBJECTID,	.name_stem = "csum"	},
	{ .id = BTRFS_QUOTA_TREE_OBJECTID,	.name_stem = "quota"	},
	{ .id = BTRFS_TREE_LOG_OBJECTID,	.name_stem = "log"	},
	{ .id = BTRFS_TREE_RELOC_OBJECTID,	.name_stem = "treloc"	},
	{ .id = BTRFS_DATA_RELOC_TREE_OBJECTID,	.name_stem = "dreloc"	},
	{ .id = BTRFS_UUID_TREE_OBJECTID,	.name_stem = "uuid"	},
	{ .id = BTRFS_FREE_SPACE_TREE_OBJECTID,	.name_stem = "free-space" },
#ifdef MY_ABC_HERE
	{ .id = BTRFS_BLOCK_GROUP_HINT_TREE_OBJECTID,   .name_stem = "block-group-hint" },
	{ .id = BTRFS_BLOCK_GROUP_CACHE_TREE_OBJECTID,   .name_stem = "block-group-cache-tree" },
#endif  
	{ .id = 0,				.name_stem = "tree"	},
};

void __init btrfs_init_lockdep(void)
{
	int i, j;

	for (i = 0; i < ARRAY_SIZE(btrfs_lockdep_keysets); i++) {
		struct btrfs_lockdep_keyset *ks = &btrfs_lockdep_keysets[i];

		for (j = 0; j < ARRAY_SIZE(ks->names); j++)
			snprintf(ks->names[j], sizeof(ks->names[j]),
				 "btrfs-%s-%02d", ks->name_stem, j);
	}
}

void btrfs_set_buffer_lockdep_class(u64 objectid, struct extent_buffer *eb,
				    int level)
{
	struct btrfs_lockdep_keyset *ks;

	BUG_ON(level >= ARRAY_SIZE(ks->keys));

	for (ks = btrfs_lockdep_keysets; ks->id; ks++)
		if (ks->id == objectid)
			break;

	lockdep_set_class_and_name(&eb->lock,
				   &ks->keys[level], ks->names[level]);
}

#endif

static struct extent_map *btree_get_extent(struct inode *inode,
		struct page *page, size_t pg_offset, u64 start, u64 len,
		int create)
{
	struct extent_map_tree *em_tree = &BTRFS_I(inode)->extent_tree;
	struct extent_map *em;
	int ret;

	read_lock(&em_tree->lock);
	em = lookup_extent_mapping(em_tree, start, len);
	if (em) {
		em->bdev =
			BTRFS_I(inode)->root->fs_info->fs_devices->latest_bdev;
		read_unlock(&em_tree->lock);
		goto out;
	}
	read_unlock(&em_tree->lock);

	em = alloc_extent_map();
	if (!em) {
		em = ERR_PTR(-ENOMEM);
		goto out;
	}
	em->start = 0;
	em->len = (u64)-1;
	em->block_len = (u64)-1;
	em->block_start = 0;
	em->bdev = BTRFS_I(inode)->root->fs_info->fs_devices->latest_bdev;

	write_lock(&em_tree->lock);
	ret = add_extent_mapping(em_tree, em, 0);
	if (ret == -EEXIST) {
		free_extent_map(em);
		em = lookup_extent_mapping(em_tree, start, len);
		if (!em)
			em = ERR_PTR(-EIO);
	} else if (ret) {
		free_extent_map(em);
		em = ERR_PTR(ret);
	}
	write_unlock(&em_tree->lock);

out:
	return em;
}

u32 btrfs_csum_data(char *data, u32 seed, size_t len)
{
	return btrfs_crc32c(seed, data, len);
}

void btrfs_csum_final(u32 crc, char *result)
{
	put_unaligned_le32(~crc, result);
}

static int csum_tree_block(struct btrfs_fs_info *fs_info,
			   struct extent_buffer *buf,
			   int verify)
{
	u16 csum_size = btrfs_super_csum_size(fs_info->super_copy);
	char *result = NULL;
	unsigned long len;
	unsigned long cur_len;
	unsigned long offset = BTRFS_CSUM_SIZE;
	char *kaddr;
	unsigned long map_start;
	unsigned long map_len;
	int err;
	u32 crc = ~(u32)0;
	unsigned long inline_result;

	len = buf->len - offset;
	while (len > 0) {
		err = map_private_extent_buffer(buf, offset, 32,
					&kaddr, &map_start, &map_len);
		if (err)
			return err;
		cur_len = min(len, map_len - (offset - map_start));
		crc = btrfs_csum_data(kaddr + offset - map_start,
				      crc, cur_len);
		len -= cur_len;
		offset += cur_len;
	}
	if (csum_size > sizeof(inline_result)) {
		result = kzalloc(csum_size, GFP_NOFS);
		if (!result)
			return -ENOMEM;
	} else {
		result = (char *)&inline_result;
	}

	btrfs_csum_final(crc, result);

	if (verify) {
		if (memcmp_extent_buffer(buf, result, 0, csum_size)) {
			u32 val;
			u32 found = 0;
			memcpy(&found, result, csum_size);

			read_extent_buffer(buf, &val, 0, csum_size);
			btrfs_warn_rl(fs_info,
				"%s checksum verify failed on %llu wanted %X found %X "
				"level %d",
				fs_info->sb->s_id, buf->start,
				val, found, btrfs_header_level(buf));
			if (result != (char *)&inline_result)
				kfree(result);
			return -EUCLEAN;
		}
	} else {
		write_extent_buffer(buf, result, 0, csum_size);
	}
	if (result != (char *)&inline_result)
		kfree(result);
	return 0;
}

static int verify_parent_transid(struct extent_io_tree *io_tree,
				 struct extent_buffer *eb, u64 parent_transid,
				 int atomic)
{
	struct extent_state *cached_state = NULL;
	int ret;
	bool need_lock = (current->journal_info == BTRFS_SEND_TRANS_STUB);

	if (!parent_transid || btrfs_header_generation(eb) == parent_transid)
		return 0;

	if (atomic)
		return -EAGAIN;

	if (need_lock) {
		btrfs_tree_read_lock(eb);
		btrfs_set_lock_blocking_rw(eb, BTRFS_READ_LOCK);
	}

	lock_extent_bits(io_tree, eb->start, eb->start + eb->len - 1,
			 &cached_state);
	if (extent_buffer_uptodate(eb) &&
	    btrfs_header_generation(eb) == parent_transid) {
		ret = 0;
		goto out;
	}
	btrfs_err_rl(eb->fs_info,
		"parent transid verify failed on %llu wanted %llu found %llu",
			eb->start,
			parent_transid, btrfs_header_generation(eb));
	ret = 1;

	if (!extent_buffer_under_io(eb))
		clear_extent_buffer_uptodate(eb);
out:
	unlock_extent_cached(io_tree, eb->start, eb->start + eb->len - 1,
			     &cached_state, GFP_NOFS);
	if (need_lock)
		btrfs_tree_read_unlock_blocking(eb);
	return ret;
}

static int btrfs_check_super_csum(char *raw_disk_sb)
{
	struct btrfs_super_block *disk_sb =
		(struct btrfs_super_block *)raw_disk_sb;
	u16 csum_type = btrfs_super_csum_type(disk_sb);
	int ret = 0;

	if (csum_type == BTRFS_CSUM_TYPE_CRC32) {
		u32 crc = ~(u32)0;
		const int csum_size = sizeof(crc);
		char result[csum_size];

		crc = btrfs_csum_data(raw_disk_sb + BTRFS_CSUM_SIZE,
				crc, BTRFS_SUPER_INFO_SIZE - BTRFS_CSUM_SIZE);
		btrfs_csum_final(crc, result);

		if (memcmp(raw_disk_sb, result, csum_size))
			ret = 1;
	}

	if (csum_type >= ARRAY_SIZE(btrfs_csum_sizes)) {
		printk(KERN_ERR "BTRFS: unsupported checksum algorithm %u\n",
				csum_type);
		ret = 1;
	}

	return ret;
}

static int btree_read_extent_buffer_pages(struct btrfs_root *root,
					  struct extent_buffer *eb,
					  u64 start, u64 parent_transid)
{
	struct extent_io_tree *io_tree;
	int failed = 0;
	int ret;
	int num_copies = 0;
	int mirror_num = 0;
	int failed_mirror = 0;

	clear_bit(EXTENT_BUFFER_CORRUPT, &eb->bflags);
	io_tree = &BTRFS_I(root->fs_info->btree_inode)->io_tree;
	while (1) {
		ret = read_extent_buffer_pages(io_tree, eb, start,
					       WAIT_COMPLETE,
					       btree_get_extent, mirror_num);
		if (!ret) {
			if (!verify_parent_transid(io_tree, eb,
						   parent_transid, 0))
				break;
			else
				ret = -EIO;
		}

		if (test_bit(EXTENT_BUFFER_CORRUPT, &eb->bflags))
			break;

		num_copies = btrfs_num_copies(root->fs_info,
					      eb->start, eb->len);
		if (num_copies == 1)
			break;

		if (!failed_mirror) {
			failed = 1;
			failed_mirror = eb->read_mirror;
		}

		mirror_num++;
		if (mirror_num == failed_mirror)
			mirror_num++;

		if (mirror_num > num_copies)
			break;
	}

	if (failed && !ret && failed_mirror)
		repair_eb_io_failure(root, eb, failed_mirror);
#ifdef MY_ABC_HERE
	if (unlikely(test_bit(EXTENT_BUFFER_CORRUPT, &eb->bflags) && !ret))
		repair_eb_io_failure(root, eb, 1);
#endif  

	return ret;
}

static int csum_dirty_buffer(struct btrfs_fs_info *fs_info, struct page *page)
{
	u64 start = page_offset(page);
	u64 found_start;
	struct extent_buffer *eb;

	eb = (struct extent_buffer *)page->private;
	if (page != eb->pages[0])
		return 0;

	found_start = btrfs_header_bytenr(eb);
	 
	if (WARN_ON(found_start != start))
		return -EUCLEAN;
	if (WARN_ON(!PageUptodate(page)))
		return -EUCLEAN;

	ASSERT(memcmp_extent_buffer(eb, fs_info->fsid,
			btrfs_header_fsid(), BTRFS_FSID_SIZE) == 0);

	return csum_tree_block(fs_info, eb, 0);
}

static int check_tree_block_fsid(struct btrfs_fs_info *fs_info,
				 struct extent_buffer *eb)
{
	struct btrfs_fs_devices *fs_devices = fs_info->fs_devices;
	u8 fsid[BTRFS_UUID_SIZE];
	int ret = 1;

	read_extent_buffer(eb, fsid, btrfs_header_fsid(), BTRFS_FSID_SIZE);
	while (fs_devices) {
		if (!memcmp(fsid, fs_devices->fsid, BTRFS_FSID_SIZE)) {
			ret = 0;
			break;
		}
		fs_devices = fs_devices->seed;
	}
	return ret;
}

#define CORRUPT(reason, eb, root, slot)				\
	btrfs_crit(root->fs_info, "corrupt %s, %s: block=%llu,"	\
		   " root=%llu, slot=%d",			\
		   btrfs_header_level(eb) == 0 ? "leaf" : "node",\
		   reason, btrfs_header_bytenr(eb), root->objectid, slot)

#ifdef MY_ABC_HERE
 
static int fix_item_offset_size(struct btrfs_root *root, struct extent_buffer *leaf, int slot)
{
	u32 offset, size;
	struct btrfs_item *item = btrfs_item_nr(slot);

	if (slot >= btrfs_header_nritems(leaf) - 1)
		return -EIO;

	if (btrfs_item_offset_nr(leaf, slot + 1) > BTRFS_LEAF_DATA_SIZE(root) ||
			btrfs_item_size_nr(leaf, slot + 1) > BTRFS_LEAF_DATA_SIZE(root))
		return -EIO;

	if (slot + 2 < btrfs_header_nritems(leaf) &&
			btrfs_item_offset_nr(leaf, slot + 1) != btrfs_item_end_nr(leaf, slot + 2))
		return -EIO;

	offset = btrfs_item_end_nr(leaf, slot + 1);

	if (slot != 0) {
		if (offset >= btrfs_item_offset_nr(leaf, slot - 1))
			return -EIO;
		size = btrfs_item_offset_nr(leaf, slot - 1) - offset;
	} else {
		if (offset >= BTRFS_LEAF_DATA_SIZE(root))
			return -EIO;
		size = BTRFS_LEAF_DATA_SIZE(root) - offset;
	}

	btrfs_set_item_offset(leaf, item, offset);
	btrfs_set_item_size(leaf, item, size);

	return 0;
}

static void fix_item_key(struct btrfs_fs_info *fs_info, struct extent_buffer *leaf, int slot, struct btrfs_key bad_key)
{
	struct btrfs_extent_item *ei;
	struct btrfs_extent_inline_ref *iref;
	struct btrfs_extent_data_ref *dref;
	struct btrfs_root *root;
	struct btrfs_key key;
	struct btrfs_path *path = NULL;
	struct extent_buffer *buf;
	struct btrfs_file_extent_item *item;
	struct btrfs_disk_key disk_key;
	u64 flags;
	int type;
	int ret;

	if (btrfs_header_owner(leaf) != 2)
		return;

	if (bad_key.type == BTRFS_BLOCK_GROUP_ITEM_KEY)
		return;

	ei = btrfs_item_ptr(leaf, slot, struct btrfs_extent_item);
	flags = btrfs_extent_flags(leaf, ei);

	if (flags != BTRFS_EXTENT_FLAG_DATA)
		return;

	iref = (struct btrfs_extent_inline_ref *)(ei + 1);
	type = btrfs_extent_inline_ref_type(leaf, iref);

	if (type != BTRFS_EXTENT_DATA_REF_KEY)
		return;

	if (cmpxchg(&fs_info->can_fix_meta_key, CAN_FIX_META_KEY, DOING_FIX_META_KEY) != CAN_FIX_META_KEY)
		return;

	dref = (struct btrfs_extent_data_ref *)(&iref->offset);
	key.objectid = btrfs_extent_data_ref_root(leaf, dref);
	key.type = BTRFS_ROOT_ITEM_KEY;
	key.offset = 0;
	root = btrfs_read_fs_root_no_name(fs_info, &key);
	if (IS_ERR(root))
		goto err;

	path = btrfs_alloc_path();
	if (!path)
		goto err;

	key.objectid = btrfs_extent_data_ref_objectid(leaf, dref);
	key.offset = btrfs_extent_data_ref_offset(leaf, dref);
	key.type = BTRFS_EXTENT_DATA_KEY;
	ret = btrfs_search_slot(NULL, root, &key, path, 0, 0);
	if (ret)
		goto err;

	buf = path->nodes[0];
	item = btrfs_item_ptr(buf, path->slots[0], struct btrfs_file_extent_item);
	type = btrfs_file_extent_type(buf, item);
	if (type != BTRFS_FILE_EXTENT_REG && type != BTRFS_FILE_EXTENT_PREALLOC)
		goto err;

	key.objectid = btrfs_file_extent_disk_bytenr(buf, item);
	key.type = BTRFS_EXTENT_ITEM_KEY;
	key.offset = btrfs_file_extent_disk_num_bytes(buf, item);
	btrfs_cpu_key_to_disk(&disk_key, &key);
	btrfs_set_item_key(leaf, &disk_key, slot);

err:
	fs_info->can_fix_meta_key = CAN_FIX_META_KEY;
	btrfs_free_path(path);
	return;
}
#endif  

static noinline int check_leaf(struct btrfs_root *root,
			       struct extent_buffer *leaf)
{
	struct btrfs_key key;
	struct btrfs_key leaf_key;
	u32 nritems = btrfs_header_nritems(leaf);
	int slot;

	if (nritems == 0)
		return 0;

	if (btrfs_item_offset_nr(leaf, 0) + btrfs_item_size_nr(leaf, 0) !=
	    BTRFS_LEAF_DATA_SIZE(root)) {
#ifdef MY_ABC_HERE
		if (fix_item_offset_size(root, leaf, 0)) {
			btrfs_crit(root->fs_info, "invalid leaf item offset size pair, "
					"block=%llu, root=%llu, slot=%d", btrfs_header_bytenr(leaf), btrfs_header_owner(leaf), 0);
			return -EIO;
		}
		btrfs_warn(root->fs_info, "corrupt leaf fixed, invalid item offset size pair, "
				"block=%llu, root=%llu, slot=%d", btrfs_header_bytenr(leaf), btrfs_header_owner(leaf), 0);
		set_bit(EXTENT_BUFFER_CORRUPT, &leaf->bflags);
#else
		CORRUPT("invalid item offset size pair", leaf, root, 0);
		return -EIO;
#endif  
	}

	for (slot = 0; slot < nritems - 1; slot++) {
		btrfs_item_key_to_cpu(leaf, &leaf_key, slot);
		btrfs_item_key_to_cpu(leaf, &key, slot + 1);

#ifdef MY_ABC_HERE
		 
#else
		 
		if (btrfs_comp_cpu_keys(&leaf_key, &key) >= 0) {
			CORRUPT("bad key order", leaf, root, slot);
			return -EIO;
		}
#endif  

		if (btrfs_item_offset_nr(leaf, slot) !=
			btrfs_item_end_nr(leaf, slot + 1)) {
#ifdef MY_ABC_HERE
			if (fix_item_offset_size(root, leaf, slot + 1)) {
				btrfs_crit(root->fs_info, "leaf slot offset bad, "
						"block=%llu, root=%llu, slot=%d", btrfs_header_bytenr(leaf), btrfs_header_owner(leaf), slot + 1);
				return -EIO;
			}
			btrfs_warn(root->fs_info, "corrupt leaf fixed, slot offset bad, "
					"block=%llu, root=%llu, slot=%d", btrfs_header_bytenr(leaf), btrfs_header_owner(leaf), slot + 1);
			set_bit(EXTENT_BUFFER_CORRUPT, &leaf->bflags);
#else
			CORRUPT("slot offset bad", leaf, root, slot);
			return -EIO;
#endif  
		}

#ifdef MY_ABC_HERE
		 
		if (btrfs_comp_cpu_keys(&leaf_key, &key) >= 0) {
			 
			fix_item_key(root->fs_info, leaf, slot, leaf_key);
			fix_item_key(root->fs_info, leaf, slot + 1, key);

			btrfs_item_key_to_cpu(leaf, &leaf_key, slot);
			btrfs_item_key_to_cpu(leaf, &key, slot + 1);

			if (btrfs_comp_cpu_keys(&leaf_key, &key) >= 0) {
				btrfs_crit(root->fs_info, "leaf bad key order, "
						"block=%llu, root=%llu, slot=%d", btrfs_header_bytenr(leaf), btrfs_header_owner(leaf), slot);
				return -EIO;
			}
			btrfs_warn(root->fs_info, "corrupt leaf fixed, bad key order, "
					"block=%llu, root=%llu, slot=%d", btrfs_header_bytenr(leaf), btrfs_header_owner(leaf), slot);
			set_bit(EXTENT_BUFFER_CORRUPT, &leaf->bflags);
		}

#else
		 
		if (btrfs_item_end_nr(leaf, slot) >
		    BTRFS_LEAF_DATA_SIZE(root)) {
			CORRUPT("slot end outside of leaf", leaf, root, slot);
			return -EIO;
		}
#endif  
	}

	return 0;
}

static int check_node(struct btrfs_root *root, struct extent_buffer *node)
{
	unsigned long nr = btrfs_header_nritems(node);
	struct btrfs_key key, next_key;
	int slot;
	u64 bytenr;
	int ret = 0;

	if (nr == 0 || nr > BTRFS_NODEPTRS_PER_BLOCK(root)) {
		btrfs_crit(root->fs_info,
			   "corrupt node: block %llu root %llu nritems %lu",
			   node->start, root->objectid, nr);
		return -EIO;
	}

	for (slot = 0; slot < nr - 1; slot++) {
		bytenr = btrfs_node_blockptr(node, slot);
		btrfs_node_key_to_cpu(node, &key, slot);
		btrfs_node_key_to_cpu(node, &next_key, slot + 1);

		if (!bytenr) {
			CORRUPT("invalid item slot", node, root, slot);
			ret = -EIO;
			goto out;
		}

		if (btrfs_comp_cpu_keys(&key, &next_key) >= 0) {
			CORRUPT("bad key order", node, root, slot);
			ret = -EIO;
			goto out;
		}
	}
out:
	return ret;
}

static int btree_readpage_end_io_hook(struct btrfs_io_bio *io_bio,
				      u64 phy_offset, struct page *page,
				      u64 start, u64 end, int mirror)
{
	u64 found_start;
	int found_level;
	struct extent_buffer *eb;
	struct btrfs_root *root = BTRFS_I(page->mapping->host)->root;
	struct btrfs_fs_info *fs_info = root->fs_info;
	int ret = 0;
	int reads_done;

	if (!page->private)
		goto out;

	eb = (struct extent_buffer *)page->private;

	extent_buffer_get(eb);

	reads_done = atomic_dec_and_test(&eb->io_pages);
	if (!reads_done)
		goto err;

	eb->read_mirror = mirror;

	if (test_bit(EXTENT_BUFFER_READ_ERR, &eb->bflags)) {
		ret = -EIO;
		goto err;
	}

	found_start = btrfs_header_bytenr(eb);
	if (found_start != eb->start) {
		btrfs_err_rl(fs_info, "bad tree block start %llu %llu",
			     found_start, eb->start);
		ret = -EIO;
		goto err;
	}
	if (check_tree_block_fsid(fs_info, eb)) {
		btrfs_err_rl(fs_info, "bad fsid on block %llu",
			     eb->start);
		ret = -EIO;
		goto err;
	}
	found_level = btrfs_header_level(eb);
	if (found_level >= BTRFS_MAX_LEVEL) {
		btrfs_err(fs_info, "bad tree block level %d",
			  (int)btrfs_header_level(eb));
		ret = -EIO;
		goto err;
	}

	btrfs_set_buffer_lockdep_class(btrfs_header_owner(eb),
				       eb, found_level);

	ret = csum_tree_block(fs_info, eb, 1);
	if (ret) {
		ret = -EIO;
		goto err;
	}

	if (found_level == 0 && check_leaf(root, eb)) {
		set_bit(EXTENT_BUFFER_CORRUPT, &eb->bflags);
		ret = -EIO;
	}

	if (found_level > 0 && check_node(root, eb))
		ret = -EIO;

	if (!ret)
		set_extent_buffer_uptodate(eb);
err:
	if (reads_done &&
	    test_and_clear_bit(EXTENT_BUFFER_READAHEAD, &eb->bflags))
		btree_readahead_hook(fs_info, eb, eb->start, ret);

	if (ret) {
		 
		atomic_inc(&eb->io_pages);
		clear_extent_buffer_uptodate(eb);
	}
	free_extent_buffer(eb);
out:
	return ret;
}

static int btree_io_failed_hook(struct page *page, int failed_mirror)
{
	struct extent_buffer *eb;

	eb = (struct extent_buffer *)page->private;
	set_bit(EXTENT_BUFFER_READ_ERR, &eb->bflags);

	eb->read_mirror = failed_mirror;
	atomic_dec(&eb->io_pages);
	if (test_and_clear_bit(EXTENT_BUFFER_READAHEAD, &eb->bflags))
		btree_readahead_hook(eb->fs_info, eb, eb->start, -EIO);
	return -EIO;	 
}

static void end_workqueue_bio(struct bio *bio)
{
	struct btrfs_end_io_wq *end_io_wq = bio->bi_private;
	struct btrfs_fs_info *fs_info;
	struct btrfs_workqueue *wq;
	btrfs_work_func_t func;

	fs_info = end_io_wq->info;
	end_io_wq->error = bio->bi_error;

	if (bio->bi_rw & REQ_WRITE) {
		if (end_io_wq->metadata == BTRFS_WQ_ENDIO_METADATA) {
			wq = fs_info->endio_meta_write_workers;
			func = btrfs_endio_meta_write_helper;
		} else if (end_io_wq->metadata == BTRFS_WQ_ENDIO_FREE_SPACE) {
			wq = fs_info->endio_freespace_worker;
			func = btrfs_freespace_write_helper;
		} else if (end_io_wq->metadata == BTRFS_WQ_ENDIO_RAID56) {
			wq = fs_info->endio_raid56_workers;
			func = btrfs_endio_raid56_helper;
		} else {
			wq = fs_info->endio_write_workers;
			func = btrfs_endio_write_helper;
		}
	} else {
		if (unlikely(end_io_wq->metadata ==
			     BTRFS_WQ_ENDIO_DIO_REPAIR)) {
			wq = fs_info->endio_repair_workers;
			func = btrfs_endio_repair_helper;
		} else if (end_io_wq->metadata == BTRFS_WQ_ENDIO_RAID56) {
			wq = fs_info->endio_raid56_workers;
			func = btrfs_endio_raid56_helper;
		} else if (end_io_wq->metadata) {
#ifdef MY_ABC_HERE
			if (unlikely(fs_info->can_fix_meta_key == DOING_FIX_META_KEY)) {
				wq = fs_info->endio_meta_fix_workers;
				func = btrfs_endio_meta_fix_helper;
			} else {
				wq = fs_info->endio_meta_workers;
				func = btrfs_endio_meta_helper;
			}
#else
			wq = fs_info->endio_meta_workers;
			func = btrfs_endio_meta_helper;
#endif  
		} else {
			wq = fs_info->endio_workers;
			func = btrfs_endio_helper;
		}
	}

	btrfs_init_work(&end_io_wq->work, func, end_workqueue_fn, NULL, NULL);
	btrfs_queue_work(wq, &end_io_wq->work);
}

int btrfs_bio_wq_end_io(struct btrfs_fs_info *info, struct bio *bio,
			enum btrfs_wq_endio_type metadata)
{
	struct btrfs_end_io_wq *end_io_wq;

	end_io_wq = kmem_cache_alloc(btrfs_end_io_wq_cache, GFP_NOFS);
	if (!end_io_wq)
		return -ENOMEM;

	end_io_wq->private = bio->bi_private;
	end_io_wq->end_io = bio->bi_end_io;
	end_io_wq->info = info;
	end_io_wq->error = 0;
	end_io_wq->bio = bio;
	end_io_wq->metadata = metadata;

	bio->bi_private = end_io_wq;
	bio->bi_end_io = end_workqueue_bio;
	return 0;
}

unsigned long btrfs_async_submit_limit(struct btrfs_fs_info *info)
{
	unsigned long limit = min_t(unsigned long,
				    info->thread_pool_size,
				    info->fs_devices->open_devices);
	return 256 * limit;
}

static void run_one_async_start(struct btrfs_work *work)
{
	struct async_submit_bio *async;
	int ret;

	async = container_of(work, struct  async_submit_bio, work);
	ret = async->submit_bio_start(async->inode, async->rw, async->bio,
				      async->mirror_num, async->bio_flags,
				      async->bio_offset);
	if (ret)
		async->error = ret;
}

static void run_one_async_done(struct btrfs_work *work)
{
	struct btrfs_fs_info *fs_info;
	struct async_submit_bio *async;
	int limit;

	async = container_of(work, struct  async_submit_bio, work);
	fs_info = BTRFS_I(async->inode)->root->fs_info;

	limit = btrfs_async_submit_limit(fs_info);
	limit = limit * 2 / 3;

	if (atomic_dec_return(&fs_info->nr_async_submits) < limit &&
	    waitqueue_active(&fs_info->async_submit_wait))
		wake_up(&fs_info->async_submit_wait);

	if (async->error) {
		async->bio->bi_error = async->error;
		bio_endio(async->bio);
		return;
	}

	async->submit_bio_done(async->inode, async->rw, async->bio,
			       async->mirror_num, async->bio_flags,
			       async->bio_offset);
}

static void run_one_async_free(struct btrfs_work *work)
{
	struct async_submit_bio *async;
#ifdef MY_ABC_HERE
	struct btrfs_fs_info *fs_info;
#endif  

	async = container_of(work, struct  async_submit_bio, work);
#ifdef MY_ABC_HERE
	if (async->throttle) {
		fs_info = BTRFS_I(async->inode)->root->fs_info;
		if (atomic_dec_return(&fs_info->syno_async_submit_nr) < fs_info->syno_async_submit_throttle &&
			waitqueue_active(&fs_info->syno_async_submit_queue_wait))
			wake_up(&fs_info->syno_async_submit_queue_wait);
	}
#endif  
	kfree(async);
}

#ifdef MY_ABC_HERE
int btrfs_wq_submit_bio(struct btrfs_fs_info *fs_info, struct inode *inode,
			int rw, struct bio *bio, int mirror_num,
			unsigned long bio_flags,
			u64 bio_offset,
			extent_submit_bio_hook_t *submit_bio_start,
			extent_submit_bio_hook_t *submit_bio_done, int throttle)
#else
int btrfs_wq_submit_bio(struct btrfs_fs_info *fs_info, struct inode *inode,
			int rw, struct bio *bio, int mirror_num,
			unsigned long bio_flags,
			u64 bio_offset,
			extent_submit_bio_hook_t *submit_bio_start,
			extent_submit_bio_hook_t *submit_bio_done)
#endif  
{
	struct async_submit_bio *async;

	async = kmalloc(sizeof(*async), GFP_NOFS);
	if (!async)
		return -ENOMEM;

	async->inode = inode;
	async->rw = rw;
	async->bio = bio;
	async->mirror_num = mirror_num;
	async->submit_bio_start = submit_bio_start;
	async->submit_bio_done = submit_bio_done;
#ifdef MY_ABC_HERE
	async->throttle = throttle;
#endif  

	btrfs_init_work(&async->work, btrfs_worker_helper, run_one_async_start,
			run_one_async_done, run_one_async_free);

	async->bio_flags = bio_flags;
	async->bio_offset = bio_offset;

	async->error = 0;

	atomic_inc(&fs_info->nr_async_submits);
#ifdef MY_ABC_HERE
	if (async->throttle) {
		atomic_inc(&fs_info->syno_async_submit_nr);
	}
#endif  

	if (rw & REQ_SYNC)
		btrfs_set_work_high_priority(&async->work);

	btrfs_queue_work(fs_info->workers, &async->work);

	while (atomic_read(&fs_info->async_submit_draining) &&
	      atomic_read(&fs_info->nr_async_submits)) {
		wait_event(fs_info->async_submit_wait,
			   (atomic_read(&fs_info->nr_async_submits) == 0));
	}

	return 0;
}

static int btree_csum_one_bio(struct bio *bio)
{
	struct bio_vec *bvec;
	struct btrfs_root *root;
	int i, ret = 0;

	bio_for_each_segment_all(bvec, bio, i) {
		root = BTRFS_I(bvec->bv_page->mapping->host)->root;
		ret = csum_dirty_buffer(root->fs_info, bvec->bv_page);
		if (ret)
			break;
	}

	return ret;
}

static int __btree_submit_bio_start(struct inode *inode, int rw,
				    struct bio *bio, int mirror_num,
				    unsigned long bio_flags,
				    u64 bio_offset)
{
	 
	return btree_csum_one_bio(bio);
}

static int __btree_submit_bio_done(struct inode *inode, int rw, struct bio *bio,
				 int mirror_num, unsigned long bio_flags,
				 u64 bio_offset)
{
	int ret;

	ret = btrfs_map_bio(BTRFS_I(inode)->root, rw, bio, mirror_num, 1);
	if (ret) {
		bio->bi_error = ret;
		bio_endio(bio);
	}
	return ret;
}

static int check_async_write(struct inode *inode, unsigned long bio_flags)
{
	if (bio_flags & EXTENT_BIO_TREE_LOG)
		return 0;
#ifdef CONFIG_X86
	if (static_cpu_has(X86_FEATURE_XMM4_2))
		return 0;
#endif
	return 1;
}

static int btree_submit_bio_hook(struct inode *inode, int rw, struct bio *bio,
				 int mirror_num, unsigned long bio_flags,
				 u64 bio_offset)
{
	int async = check_async_write(inode, bio_flags);
	int ret;

	if (!(rw & REQ_WRITE)) {
		 
		ret = btrfs_bio_wq_end_io(BTRFS_I(inode)->root->fs_info,
					  bio, BTRFS_WQ_ENDIO_METADATA);
		if (ret)
			goto out_w_error;
		ret = btrfs_map_bio(BTRFS_I(inode)->root, rw, bio,
				    mirror_num, 0);
	} else if (!async) {
		ret = btree_csum_one_bio(bio);
		if (ret)
			goto out_w_error;
		ret = btrfs_map_bio(BTRFS_I(inode)->root, rw, bio,
				    mirror_num, 0);
	} else {
		 
#ifdef MY_ABC_HERE
		ret = btrfs_wq_submit_bio(BTRFS_I(inode)->root->fs_info,
					  inode, rw, bio, mirror_num, 0,
					  bio_offset,
					  __btree_submit_bio_start,
					  __btree_submit_bio_done, 0);
#else
		ret = btrfs_wq_submit_bio(BTRFS_I(inode)->root->fs_info,
					  inode, rw, bio, mirror_num, 0,
					  bio_offset,
					  __btree_submit_bio_start,
					  __btree_submit_bio_done);
#endif  
	}

	if (ret)
		goto out_w_error;
	return 0;

out_w_error:
	bio->bi_error = ret;
	bio_endio(bio);
	return ret;
}

#ifdef CONFIG_MIGRATION
static int btree_migratepage(struct address_space *mapping,
			struct page *newpage, struct page *page,
			enum migrate_mode mode)
{
	 
	if (PageDirty(page))
		return -EAGAIN;
	 
	if (page_has_private(page) &&
	    !try_to_release_page(page, GFP_KERNEL))
		return -EAGAIN;
	return migrate_page(mapping, newpage, page, mode);
}
#endif

static int btree_writepages(struct address_space *mapping,
			    struct writeback_control *wbc)
{
	struct btrfs_fs_info *fs_info;
	int ret;

	if (wbc->sync_mode == WB_SYNC_NONE) {

		if (wbc->for_kupdate)
			return 0;

		fs_info = BTRFS_I(mapping->host)->root->fs_info;
		 
		ret = __percpu_counter_compare(&fs_info->dirty_metadata_bytes,
					     BTRFS_DIRTY_METADATA_THRESH,
					     fs_info->dirty_metadata_batch);
		if (ret < 0)
			return 0;
	}
	return btree_write_cache_pages(mapping, wbc);
}

static int btree_readpage(struct file *file, struct page *page)
{
	struct extent_io_tree *tree;
	tree = &BTRFS_I(page->mapping->host)->io_tree;
	return extent_read_full_page(tree, page, btree_get_extent, 0);
}

static int btree_releasepage(struct page *page, gfp_t gfp_flags)
{
	if (PageWriteback(page) || PageDirty(page))
		return 0;

	return try_release_extent_buffer(page);
}

static void btree_invalidatepage(struct page *page, unsigned int offset,
				 unsigned int length)
{
	struct extent_io_tree *tree;
	tree = &BTRFS_I(page->mapping->host)->io_tree;
	extent_invalidatepage(tree, page, offset);
	btree_releasepage(page, GFP_NOFS);
	if (PagePrivate(page)) {
		btrfs_warn(BTRFS_I(page->mapping->host)->root->fs_info,
			   "page private not zero on page %llu",
			   (unsigned long long)page_offset(page));
		ClearPagePrivate(page);
		set_page_private(page, 0);
		page_cache_release(page);
	}
}

static int btree_set_page_dirty(struct page *page)
{
#ifdef DEBUG
	struct extent_buffer *eb;

	BUG_ON(!PagePrivate(page));
	eb = (struct extent_buffer *)page->private;
	BUG_ON(!eb);
	BUG_ON(!test_bit(EXTENT_BUFFER_DIRTY, &eb->bflags));
	BUG_ON(!atomic_read(&eb->refs));
	btrfs_assert_tree_locked(eb);
#endif
	return __set_page_dirty_nobuffers(page);
}

static const struct address_space_operations btree_aops = {
	.readpage	= btree_readpage,
	.writepages	= btree_writepages,
	.releasepage	= btree_releasepage,
	.invalidatepage = btree_invalidatepage,
#ifdef CONFIG_MIGRATION
	.migratepage	= btree_migratepage,
#endif
	.set_page_dirty = btree_set_page_dirty,
};

void readahead_tree_block(struct btrfs_root *root, u64 bytenr)
{
	struct extent_buffer *buf = NULL;
	struct inode *btree_inode = root->fs_info->btree_inode;

	buf = btrfs_find_create_tree_block(root, bytenr);
	if (IS_ERR(buf))
		return;
	read_extent_buffer_pages(&BTRFS_I(btree_inode)->io_tree,
				 buf, 0, WAIT_NONE, btree_get_extent, 0);
	free_extent_buffer(buf);
}

int reada_tree_block_flagged(struct btrfs_root *root, u64 bytenr,
			 int mirror_num, struct extent_buffer **eb)
{
	struct extent_buffer *buf = NULL;
	struct inode *btree_inode = root->fs_info->btree_inode;
	struct extent_io_tree *io_tree = &BTRFS_I(btree_inode)->io_tree;
	int ret;

	buf = btrfs_find_create_tree_block(root, bytenr);
	if (IS_ERR(buf))
		return 0;

	set_bit(EXTENT_BUFFER_READAHEAD, &buf->bflags);

	ret = read_extent_buffer_pages(io_tree, buf, 0, WAIT_PAGE_LOCK,
				       btree_get_extent, mirror_num);
	if (ret) {
		free_extent_buffer(buf);
		return ret;
	}

	if (test_bit(EXTENT_BUFFER_CORRUPT, &buf->bflags)) {
		free_extent_buffer(buf);
		return -EIO;
	} else if (extent_buffer_uptodate(buf)) {
		*eb = buf;
	} else {
		free_extent_buffer(buf);
	}
	return 0;
}

#ifdef MY_ABC_HERE
struct extent_buffer *btrfs_find_tree_block(struct btrfs_root *root,
					    u64 bytenr)
{
	return find_extent_buffer(root, bytenr);
}
#else
struct extent_buffer *btrfs_find_tree_block(struct btrfs_fs_info *fs_info,
					    u64 bytenr)
{
	return find_extent_buffer(fs_info, bytenr);
}
#endif  

struct extent_buffer *btrfs_find_create_tree_block(struct btrfs_root *root,
						 u64 bytenr)
{
	if (btrfs_test_is_dummy_root(root))
		return alloc_test_extent_buffer(root->fs_info, bytenr);
#ifdef MY_ABC_HERE
	return alloc_extent_buffer(root, bytenr);
#else
	return alloc_extent_buffer(root->fs_info, bytenr);
#endif  
}

int btrfs_write_tree_block(struct extent_buffer *buf)
{
	return filemap_fdatawrite_range(buf->pages[0]->mapping, buf->start,
					buf->start + buf->len - 1);
}

int btrfs_wait_tree_block_writeback(struct extent_buffer *buf)
{
	return filemap_fdatawait_range(buf->pages[0]->mapping,
				       buf->start, buf->start + buf->len - 1);
}

struct extent_buffer *read_tree_block(struct btrfs_root *root, u64 bytenr,
				      u64 parent_transid)
{
	struct extent_buffer *buf = NULL;
	int ret;

	buf = btrfs_find_create_tree_block(root, bytenr);
	if (IS_ERR(buf))
		return buf;

	ret = btree_read_extent_buffer_pages(root, buf, 0, parent_transid);
	if (ret) {
		free_extent_buffer(buf);
		return ERR_PTR(ret);
	}
	return buf;

}

void clean_tree_block(struct btrfs_trans_handle *trans,
		      struct btrfs_fs_info *fs_info,
		      struct extent_buffer *buf)
{
	if (btrfs_header_generation(buf) ==
	    fs_info->running_transaction->transid) {
		btrfs_assert_tree_locked(buf);

		if (test_and_clear_bit(EXTENT_BUFFER_DIRTY, &buf->bflags)) {
			__percpu_counter_add(&fs_info->dirty_metadata_bytes,
					     -buf->len,
					     fs_info->dirty_metadata_batch);
			 
			btrfs_set_lock_blocking(buf);
			clear_extent_buffer_dirty(buf);
		}
	}
}

static struct btrfs_subvolume_writers *btrfs_alloc_subvolume_writers(void)
{
	struct btrfs_subvolume_writers *writers;
	int ret;

	writers = kmalloc(sizeof(*writers), GFP_NOFS);
	if (!writers)
		return ERR_PTR(-ENOMEM);

	ret = percpu_counter_init(&writers->counter, 0, GFP_KERNEL);
	if (ret < 0) {
		kfree(writers);
		return ERR_PTR(ret);
	}

	init_waitqueue_head(&writers->wait);
	return writers;
}

static void
btrfs_free_subvolume_writers(struct btrfs_subvolume_writers *writers)
{
	percpu_counter_destroy(&writers->counter);
	kfree(writers);
}

static void __setup_root(u32 nodesize, u32 sectorsize, u32 stripesize,
			 struct btrfs_root *root, struct btrfs_fs_info *fs_info,
			 u64 objectid)
{
	root->node = NULL;
	root->commit_root = NULL;
	root->sectorsize = sectorsize;
	root->nodesize = nodesize;
	root->stripesize = stripesize;
	root->state = 0;
	root->orphan_cleanup_state = 0;

	root->objectid = objectid;
	root->last_trans = 0;
	root->highest_objectid = 0;
	root->nr_delalloc_inodes = 0;
	root->nr_ordered_extents = 0;
	root->name = NULL;
	root->inode_tree = RB_ROOT;
	INIT_RADIX_TREE(&root->delayed_nodes_tree, GFP_ATOMIC);
	root->block_rsv = NULL;
	root->orphan_block_rsv = NULL;

	INIT_LIST_HEAD(&root->dirty_list);
	INIT_LIST_HEAD(&root->root_list);
	INIT_LIST_HEAD(&root->delalloc_inodes);
	INIT_LIST_HEAD(&root->delalloc_root);
	INIT_LIST_HEAD(&root->ordered_extents);
	INIT_LIST_HEAD(&root->ordered_root);
	INIT_LIST_HEAD(&root->logged_list[0]);
	INIT_LIST_HEAD(&root->logged_list[1]);
	spin_lock_init(&root->orphan_lock);
	spin_lock_init(&root->inode_lock);
	spin_lock_init(&root->delalloc_lock);
	spin_lock_init(&root->ordered_extent_lock);
	spin_lock_init(&root->accounting_lock);
	spin_lock_init(&root->log_extents_lock[0]);
	spin_lock_init(&root->log_extents_lock[1]);
	mutex_init(&root->objectid_mutex);
	mutex_init(&root->log_mutex);
	mutex_init(&root->ordered_extent_mutex);
#ifdef MY_ABC_HERE
	mutex_init(&root->ordered_extent_worker_mutex);
#endif  
	mutex_init(&root->delalloc_mutex);
	init_waitqueue_head(&root->log_writer_wait);
	init_waitqueue_head(&root->log_commit_wait[0]);
	init_waitqueue_head(&root->log_commit_wait[1]);
	INIT_LIST_HEAD(&root->log_ctxs[0]);
	INIT_LIST_HEAD(&root->log_ctxs[1]);
	atomic_set(&root->log_commit[0], 0);
	atomic_set(&root->log_commit[1], 0);
	atomic_set(&root->log_writers, 0);
	atomic_set(&root->log_batch, 0);
	atomic_set(&root->orphan_inodes, 0);
	atomic_set(&root->refs, 1);
	atomic_set(&root->will_be_snapshoted, 0);
	atomic_set(&root->qgroup_meta_rsv, 0);
	root->log_transid = 0;
#ifdef MY_ABC_HERE
#else
	root->log_transid_committed = -1;
#endif  
	root->last_log_commit = 0;
	if (fs_info)
		extent_io_tree_init(&root->dirty_log_pages,
				     fs_info->btree_inode->i_mapping);

	memset(&root->root_key, 0, sizeof(root->root_key));
	memset(&root->root_item, 0, sizeof(root->root_item));
	memset(&root->defrag_progress, 0, sizeof(root->defrag_progress));
	if (fs_info)
		root->defrag_trans_start = fs_info->generation;
	else
		root->defrag_trans_start = 0;
	root->root_key.objectid = objectid;
	root->anon_dev = 0;

	spin_lock_init(&root->root_item_lock);
}

#ifdef MY_ABC_HERE
static struct btrfs_root *btrfs_alloc_root(struct btrfs_fs_info *fs_info,
		gfp_t flags, int monitor)
#else
static struct btrfs_root *btrfs_alloc_root(struct btrfs_fs_info *fs_info,
		gfp_t flags)
#endif  
{
	struct btrfs_root *root = kzalloc(sizeof(*root), flags);
	if (root)
		root->fs_info = fs_info;
#ifdef MY_ABC_HERE
	if (!root || !monitor)
		return root;

	if (percpu_counter_init(&root->eb_hit, 0, flags) ||
			percpu_counter_init(&root->eb_miss, 0, flags)) {
		btrfs_free_root_eb_monitor(root);
		kfree(root);
		root = NULL;
	}
#endif  
	return root;
}

#ifdef CONFIG_BTRFS_FS_RUN_SANITY_TESTS
 
struct btrfs_root *btrfs_alloc_dummy_root(void)
{
	struct btrfs_root *root;

	root = btrfs_alloc_root(NULL, GFP_KERNEL);
	if (!root)
		return ERR_PTR(-ENOMEM);
	__setup_root(4096, 4096, 4096, root, NULL, 1);
	set_bit(BTRFS_ROOT_DUMMY_ROOT, &root->state);
	root->alloc_bytenr = 0;

	return root;
}
#endif

struct btrfs_root *btrfs_create_tree(struct btrfs_trans_handle *trans,
				     struct btrfs_fs_info *fs_info,
				     u64 objectid)
{
	struct extent_buffer *leaf;
	struct btrfs_root *tree_root = fs_info->tree_root;
	struct btrfs_root *root;
	struct btrfs_key key;
	int ret = 0;
	uuid_le uuid;

#ifdef MY_ABC_HERE
	root = btrfs_alloc_root(fs_info, GFP_KERNEL,
			(objectid >= BTRFS_FIRST_FREE_OBJECTID && objectid <= BTRFS_LAST_FREE_OBJECTID) ||
			objectid == BTRFS_EXTENT_TREE_OBJECTID);
#else
	root = btrfs_alloc_root(fs_info, GFP_KERNEL);
#endif  
	if (!root)
		return ERR_PTR(-ENOMEM);

	__setup_root(tree_root->nodesize, tree_root->sectorsize,
		tree_root->stripesize, root, fs_info, objectid);
	root->root_key.objectid = objectid;
	root->root_key.type = BTRFS_ROOT_ITEM_KEY;
	root->root_key.offset = 0;

	leaf = btrfs_alloc_tree_block(trans, root, 0, objectid, NULL, 0, 0, 0);
	if (IS_ERR(leaf)) {
		ret = PTR_ERR(leaf);
		leaf = NULL;
		goto fail;
	}

	memset_extent_buffer(leaf, 0, 0, sizeof(struct btrfs_header));
	btrfs_set_header_bytenr(leaf, leaf->start);
	btrfs_set_header_generation(leaf, trans->transid);
	btrfs_set_header_backref_rev(leaf, BTRFS_MIXED_BACKREF_REV);
	btrfs_set_header_owner(leaf, objectid);
	root->node = leaf;

	write_extent_buffer(leaf, fs_info->fsid, btrfs_header_fsid(),
			    BTRFS_FSID_SIZE);
	write_extent_buffer(leaf, fs_info->chunk_tree_uuid,
			    btrfs_header_chunk_tree_uuid(leaf),
			    BTRFS_UUID_SIZE);
	btrfs_mark_buffer_dirty(leaf);

	root->commit_root = btrfs_root_node(root);
	set_bit(BTRFS_ROOT_TRACK_DIRTY, &root->state);

	root->root_item.flags = 0;
	root->root_item.byte_limit = 0;
	btrfs_set_root_bytenr(&root->root_item, leaf->start);
	btrfs_set_root_generation(&root->root_item, trans->transid);
	btrfs_set_root_level(&root->root_item, 0);
	btrfs_set_root_refs(&root->root_item, 1);
	btrfs_set_root_used(&root->root_item, leaf->len);
	btrfs_set_root_last_snapshot(&root->root_item, 0);
	btrfs_set_root_dirid(&root->root_item, 0);
	uuid_le_gen(&uuid);
	memcpy(root->root_item.uuid, uuid.b, BTRFS_UUID_SIZE);
	root->root_item.drop_level = 0;

	key.objectid = objectid;
	key.type = BTRFS_ROOT_ITEM_KEY;
	key.offset = 0;
	ret = btrfs_insert_root(trans, tree_root, &key, &root->root_item);
	if (ret)
		goto fail;

	btrfs_tree_unlock(leaf);

	return root;

fail:
	if (leaf) {
		btrfs_tree_unlock(leaf);
		free_extent_buffer(root->commit_root);
		free_extent_buffer(leaf);
	}
#ifdef MY_ABC_HERE
	btrfs_free_root_eb_monitor(root);
#endif  
	kfree(root);

	return ERR_PTR(ret);
}

static struct btrfs_root *alloc_log_tree(struct btrfs_trans_handle *trans,
					 struct btrfs_fs_info *fs_info)
{
	struct btrfs_root *root;
	struct btrfs_root *tree_root = fs_info->tree_root;
	struct extent_buffer *leaf;

#ifdef MY_ABC_HERE
	root = btrfs_alloc_root(fs_info, GFP_NOFS, 0);
#else
	root = btrfs_alloc_root(fs_info, GFP_NOFS);
#endif  
	if (!root)
		return ERR_PTR(-ENOMEM);

	__setup_root(tree_root->nodesize, tree_root->sectorsize,
		     tree_root->stripesize, root, fs_info,
		     BTRFS_TREE_LOG_OBJECTID);

	root->root_key.objectid = BTRFS_TREE_LOG_OBJECTID;
	root->root_key.type = BTRFS_ROOT_ITEM_KEY;
	root->root_key.offset = BTRFS_TREE_LOG_OBJECTID;

	leaf = btrfs_alloc_tree_block(trans, root, 0, BTRFS_TREE_LOG_OBJECTID,
			NULL, 0, 0, 0);
	if (IS_ERR(leaf)) {
#ifdef MY_ABC_HERE
		btrfs_free_root_eb_monitor(root);
#endif  
		kfree(root);
		return ERR_CAST(leaf);
	}

	memset_extent_buffer(leaf, 0, 0, sizeof(struct btrfs_header));
	btrfs_set_header_bytenr(leaf, leaf->start);
	btrfs_set_header_generation(leaf, trans->transid);
	btrfs_set_header_backref_rev(leaf, BTRFS_MIXED_BACKREF_REV);
	btrfs_set_header_owner(leaf, BTRFS_TREE_LOG_OBJECTID);
	root->node = leaf;

	write_extent_buffer(root->node, root->fs_info->fsid,
			    btrfs_header_fsid(), BTRFS_FSID_SIZE);
	btrfs_mark_buffer_dirty(root->node);
	btrfs_tree_unlock(root->node);
	return root;
}

int btrfs_init_log_root_tree(struct btrfs_trans_handle *trans,
			     struct btrfs_fs_info *fs_info)
{
	struct btrfs_root *log_root;

	log_root = alloc_log_tree(trans, fs_info);
	if (IS_ERR(log_root))
		return PTR_ERR(log_root);
	WARN_ON(fs_info->log_root_tree);
	fs_info->log_root_tree = log_root;
	return 0;
}

int btrfs_add_log_tree(struct btrfs_trans_handle *trans,
		       struct btrfs_root *root)
{
	struct btrfs_root *log_root;
	struct btrfs_inode_item *inode_item;

	log_root = alloc_log_tree(trans, root->fs_info);
	if (IS_ERR(log_root))
		return PTR_ERR(log_root);

	log_root->last_trans = trans->transid;
	log_root->root_key.offset = root->root_key.objectid;

	inode_item = &log_root->root_item.inode;
	btrfs_set_stack_inode_generation(inode_item, 1);
	btrfs_set_stack_inode_size(inode_item, 3);
	btrfs_set_stack_inode_nlink(inode_item, 1);
	btrfs_set_stack_inode_nbytes(inode_item, root->nodesize);
	btrfs_set_stack_inode_mode(inode_item, S_IFDIR | 0755);

	btrfs_set_root_node(&log_root->root_item, log_root->node);

	WARN_ON(root->log_root);
	root->log_root = log_root;
	root->log_transid = 0;
#ifdef MY_ABC_HERE
#else
	root->log_transid_committed = -1;
#endif  
	root->last_log_commit = 0;
	return 0;
}

static struct btrfs_root *btrfs_read_tree_root(struct btrfs_root *tree_root,
					       struct btrfs_key *key)
{
	struct btrfs_root *root;
	struct btrfs_fs_info *fs_info = tree_root->fs_info;
	struct btrfs_path *path;
	u64 generation;
	int ret;

	path = btrfs_alloc_path();
	if (!path)
		return ERR_PTR(-ENOMEM);

#ifdef MY_ABC_HERE
	root = btrfs_alloc_root(fs_info, GFP_NOFS,
			(key->objectid >= BTRFS_FIRST_FREE_OBJECTID && key->objectid <= BTRFS_LAST_FREE_OBJECTID) ||
			key->objectid == BTRFS_EXTENT_TREE_OBJECTID);
#else
	root = btrfs_alloc_root(fs_info, GFP_NOFS);
#endif  
	if (!root) {
		ret = -ENOMEM;
		goto alloc_fail;
	}

	__setup_root(tree_root->nodesize, tree_root->sectorsize,
		tree_root->stripesize, root, fs_info, key->objectid);

	ret = btrfs_find_root(tree_root, key, path,
			      &root->root_item, &root->root_key);
	if (ret) {
		if (ret > 0)
			ret = -ENOENT;
		goto find_fail;
	}

	generation = btrfs_root_generation(&root->root_item);
	root->node = read_tree_block(root, btrfs_root_bytenr(&root->root_item),
				     generation);
	if (IS_ERR(root->node)) {
		ret = PTR_ERR(root->node);
		goto find_fail;
	} else if (!btrfs_buffer_uptodate(root->node, generation, 0)) {
		ret = -EIO;
		free_extent_buffer(root->node);
		goto find_fail;
	}
	root->commit_root = btrfs_root_node(root);
out:
	btrfs_free_path(path);
	return root;

find_fail:
#ifdef MY_ABC_HERE
	btrfs_free_root_eb_monitor(root);
#endif  
	kfree(root);
alloc_fail:
	root = ERR_PTR(ret);
	goto out;
}

struct btrfs_root *btrfs_read_fs_root(struct btrfs_root *tree_root,
				      struct btrfs_key *location)
{
	struct btrfs_root *root;

	root = btrfs_read_tree_root(tree_root, location);
	if (IS_ERR(root))
		return root;

	if (root->root_key.objectid != BTRFS_TREE_LOG_OBJECTID) {
		set_bit(BTRFS_ROOT_REF_COWS, &root->state);
		btrfs_check_and_init_root_item(&root->root_item);
	}

	return root;
}

int btrfs_init_fs_root(struct btrfs_root *root)
{
	int ret;
	struct btrfs_subvolume_writers *writers;

	root->free_ino_ctl = kzalloc(sizeof(*root->free_ino_ctl), GFP_NOFS);
	root->free_ino_pinned = kzalloc(sizeof(*root->free_ino_pinned),
					GFP_NOFS);
	if (!root->free_ino_pinned || !root->free_ino_ctl) {
		ret = -ENOMEM;
		goto fail;
	}

	writers = btrfs_alloc_subvolume_writers();
	if (IS_ERR(writers)) {
		ret = PTR_ERR(writers);
		goto fail;
	}
	root->subv_writers = writers;

	btrfs_init_free_ino_ctl(root);
	spin_lock_init(&root->ino_cache_lock);
	init_waitqueue_head(&root->ino_cache_wait);

#ifdef MY_ABC_HERE
	ret = get_anon_bdev_with_gfp(&root->anon_dev, GFP_NOFS);
#else
	ret = get_anon_bdev(&root->anon_dev);
#endif  
	if (ret)
		goto fail;

	mutex_lock(&root->objectid_mutex);
	ret = btrfs_find_highest_objectid(root,
					&root->highest_objectid);
	if (ret) {
		mutex_unlock(&root->objectid_mutex);
		goto fail;
	}

	ASSERT(root->highest_objectid <= BTRFS_LAST_FREE_OBJECTID);

	mutex_unlock(&root->objectid_mutex);

	return 0;
fail:
	 
	return ret;
}

static struct btrfs_root *btrfs_lookup_fs_root(struct btrfs_fs_info *fs_info,
					       u64 root_id)
{
	struct btrfs_root *root;

	spin_lock(&fs_info->fs_roots_radix_lock);
	root = radix_tree_lookup(&fs_info->fs_roots_radix,
				 (unsigned long)root_id);
	spin_unlock(&fs_info->fs_roots_radix_lock);
	return root;
}

int btrfs_insert_fs_root(struct btrfs_fs_info *fs_info,
			 struct btrfs_root *root)
{
	int ret;

	ret = radix_tree_preload(GFP_NOFS);
	if (ret)
		return ret;

	spin_lock(&fs_info->fs_roots_radix_lock);
	ret = radix_tree_insert(&fs_info->fs_roots_radix,
				(unsigned long)root->root_key.objectid,
				root);
	if (ret == 0)
		set_bit(BTRFS_ROOT_IN_RADIX, &root->state);
	spin_unlock(&fs_info->fs_roots_radix_lock);
	radix_tree_preload_end();

	return ret;
}

struct btrfs_root *btrfs_get_fs_root(struct btrfs_fs_info *fs_info,
				     struct btrfs_key *location,
				     bool check_ref)
{
	struct btrfs_root *root;
	struct btrfs_path *path;
	struct btrfs_key key;
	int ret;

	if (location->objectid == BTRFS_ROOT_TREE_OBJECTID)
		return fs_info->tree_root;
	if (location->objectid == BTRFS_EXTENT_TREE_OBJECTID)
		return fs_info->extent_root;
	if (location->objectid == BTRFS_CHUNK_TREE_OBJECTID)
		return fs_info->chunk_root;
	if (location->objectid == BTRFS_DEV_TREE_OBJECTID)
		return fs_info->dev_root;
	if (location->objectid == BTRFS_CSUM_TREE_OBJECTID)
		return fs_info->csum_root;
	if (location->objectid == BTRFS_QUOTA_TREE_OBJECTID)
		return fs_info->quota_root ? fs_info->quota_root :
					     ERR_PTR(-ENOENT);
	if (location->objectid == BTRFS_UUID_TREE_OBJECTID)
		return fs_info->uuid_root ? fs_info->uuid_root :
					    ERR_PTR(-ENOENT);
	if (location->objectid == BTRFS_FREE_SPACE_TREE_OBJECTID)
		return fs_info->free_space_root ? fs_info->free_space_root :
						  ERR_PTR(-ENOENT);
#ifdef MY_ABC_HERE
	if (location->objectid == BTRFS_BLOCK_GROUP_HINT_TREE_OBJECTID)
		return fs_info->block_group_hint_root ? fs_info->block_group_hint_root :
					    ERR_PTR(-ENOENT);
	if (location->objectid == BTRFS_BLOCK_GROUP_CACHE_TREE_OBJECTID)
		return fs_info->block_group_cache_root ? fs_info->block_group_cache_root :
					    ERR_PTR(-ENOENT);
#endif  

again:
	root = btrfs_lookup_fs_root(fs_info, location->objectid);
	if (root) {
		if (check_ref && btrfs_root_refs(&root->root_item) == 0)
			return ERR_PTR(-ENOENT);
		return root;
	}

	root = btrfs_read_fs_root(fs_info->tree_root, location);
	if (IS_ERR(root))
		return root;

	if (check_ref && btrfs_root_refs(&root->root_item) == 0) {
		ret = -ENOENT;
		goto fail;
	}

	ret = btrfs_init_fs_root(root);
	if (ret)
		goto fail;

	path = btrfs_alloc_path();
	if (!path) {
		ret = -ENOMEM;
		goto fail;
	}
	key.objectid = BTRFS_ORPHAN_OBJECTID;
	key.type = BTRFS_ORPHAN_ITEM_KEY;
	key.offset = location->objectid;

	ret = btrfs_search_slot(NULL, fs_info->tree_root, &key, path, 0, 0);
	btrfs_free_path(path);
	if (ret < 0)
		goto fail;
	if (ret == 0)
		set_bit(BTRFS_ROOT_ORPHAN_ITEM_INSERTED, &root->state);

	ret = btrfs_insert_fs_root(fs_info, root);
	if (ret) {
		if (ret == -EEXIST) {
			free_fs_root(root);
			goto again;
		}
		goto fail;
	}
	return root;
fail:
	free_fs_root(root);
	return ERR_PTR(ret);
}

static int btrfs_congested_fn(void *congested_data, int bdi_bits)
{
	struct btrfs_fs_info *info = (struct btrfs_fs_info *)congested_data;
	int ret = 0;
	struct btrfs_device *device;
	struct backing_dev_info *bdi;

	rcu_read_lock();
	list_for_each_entry_rcu(device, &info->fs_devices->devices, dev_list) {
		if (!device->bdev)
			continue;
		bdi = blk_get_backing_dev_info(device->bdev);
		if (bdi_congested(bdi, bdi_bits)) {
			ret = 1;
			break;
		}
	}
	rcu_read_unlock();
	return ret;
}

static int setup_bdi(struct btrfs_fs_info *info, struct backing_dev_info *bdi)
{
	int err;

	err = bdi_setup_and_register(bdi, "btrfs");
	if (err)
		return err;

	bdi->ra_pages = VM_MAX_READAHEAD * 1024 / PAGE_CACHE_SIZE;
	bdi->congested_fn	= btrfs_congested_fn;
	bdi->congested_data	= info;
	bdi->capabilities |= BDI_CAP_CGROUP_WRITEBACK;
	return 0;
}

static void end_workqueue_fn(struct btrfs_work *work)
{
	struct bio *bio;
	struct btrfs_end_io_wq *end_io_wq;

	end_io_wq = container_of(work, struct btrfs_end_io_wq, work);
	bio = end_io_wq->bio;

	bio->bi_error = end_io_wq->error;
	bio->bi_private = end_io_wq->private;
	bio->bi_end_io = end_io_wq->end_io;
	kmem_cache_free(btrfs_end_io_wq_cache, end_io_wq);
	bio_endio(bio);
}

static int cleaner_kthread(void *arg)
{
	struct btrfs_root *root = arg;
	int again;
	struct btrfs_trans_handle *trans;

	do {
		again = 0;

		if (btrfs_need_cleaner_sleep(root))
			goto sleep;

		if (!root->fs_info->open)
			goto sleep;

		if (!mutex_trylock(&root->fs_info->cleaner_mutex))
			goto sleep;

		if (btrfs_need_cleaner_sleep(root)) {
			mutex_unlock(&root->fs_info->cleaner_mutex);
			goto sleep;
		}

		mutex_lock(&root->fs_info->cleaner_delayed_iput_mutex);
		btrfs_run_delayed_iputs(root);
		mutex_unlock(&root->fs_info->cleaner_delayed_iput_mutex);

#ifdef MY_ABC_HERE
		if (root->fs_info->snapshot_cleaner)
			again = btrfs_clean_one_deleted_snapshot(root);
#else
		again = btrfs_clean_one_deleted_snapshot(root);
#endif  
		mutex_unlock(&root->fs_info->cleaner_mutex);

		btrfs_run_defrag_inodes(root->fs_info);

		btrfs_delete_unused_bgs(root->fs_info);
sleep:
		if (!again) {
			set_current_state(TASK_INTERRUPTIBLE);
			if (!kthread_should_stop())
				schedule();
			__set_current_state(TASK_RUNNING);
		}
	} while (!kthread_should_stop());

	trans = btrfs_attach_transaction(root);
	if (IS_ERR(trans)) {
		if (PTR_ERR(trans) != -ENOENT)
			btrfs_err(root->fs_info,
				  "cleaner transaction attach returned %ld",
				  PTR_ERR(trans));
	} else {
		int ret;

		ret = btrfs_commit_transaction(trans, root);
		if (ret)
			btrfs_err(root->fs_info,
				  "cleaner open transaction commit returned %d",
				  ret);
	}

	return 0;
}

static int transaction_kthread(void *arg)
{
	struct btrfs_root *root = arg;
	struct btrfs_trans_handle *trans;
	struct btrfs_transaction *cur;
	u64 transid;
	unsigned long now;
	unsigned long delay;
	bool cannot_commit;

	do {
		cannot_commit = false;
		delay = HZ * root->fs_info->commit_interval;
		mutex_lock(&root->fs_info->transaction_kthread_mutex);

		spin_lock(&root->fs_info->trans_lock);
		cur = root->fs_info->running_transaction;
		if (!cur) {
			spin_unlock(&root->fs_info->trans_lock);
			goto sleep;
		}

		now = get_seconds();
		if (cur->state < TRANS_STATE_BLOCKED &&
		    (now < cur->start_time ||
		     now - cur->start_time < root->fs_info->commit_interval)) {
			spin_unlock(&root->fs_info->trans_lock);
#ifdef MY_ABC_HERE
			 
			if (root->fs_info->commit_interval <= 5)
				delay = HZ * 1;
			else
#endif  
			delay = HZ * 5;
			goto sleep;
		}
		transid = cur->transid;
		spin_unlock(&root->fs_info->trans_lock);

		trans = btrfs_attach_transaction(root);
		if (IS_ERR(trans)) {
			if (PTR_ERR(trans) != -ENOENT)
				cannot_commit = true;
			goto sleep;
		}
		if (transid == trans->transid) {
			btrfs_commit_transaction(trans, root);
		} else {
			btrfs_end_transaction(trans, root);
		}
sleep:
		wake_up_process(root->fs_info->cleaner_kthread);
		mutex_unlock(&root->fs_info->transaction_kthread_mutex);

		if (unlikely(test_bit(BTRFS_FS_STATE_ERROR,
				      &root->fs_info->fs_state)))
			btrfs_cleanup_transaction(root);
		set_current_state(TASK_INTERRUPTIBLE);
		if (!kthread_should_stop() &&
				(!btrfs_transaction_blocked(root->fs_info) ||
				 cannot_commit))
			schedule_timeout(delay);
		__set_current_state(TASK_RUNNING);
	} while (!kthread_should_stop());
	return 0;
}

static int find_newest_super_backup(struct btrfs_fs_info *info, u64 newest_gen)
{
	u64 cur;
	int newest_index = -1;
	struct btrfs_root_backup *root_backup;
	int i;

	for (i = 0; i < BTRFS_NUM_BACKUP_ROOTS; i++) {
		root_backup = info->super_copy->super_roots + i;
		cur = btrfs_backup_tree_root_gen(root_backup);
		if (cur == newest_gen)
			newest_index = i;
	}

	if (newest_index == BTRFS_NUM_BACKUP_ROOTS - 1) {
		root_backup = info->super_copy->super_roots;
		cur = btrfs_backup_tree_root_gen(root_backup);
		if (cur == newest_gen)
			newest_index = 0;
	}
	return newest_index;
}

static void find_oldest_super_backup(struct btrfs_fs_info *info,
				     u64 newest_gen)
{
	int newest_index = -1;

	newest_index = find_newest_super_backup(info, newest_gen);
	 
	if (newest_index == -1) {
		info->backup_root_index = 0;
	} else {
		info->backup_root_index = (newest_index + 1) % BTRFS_NUM_BACKUP_ROOTS;
	}
}

static void backup_super_roots(struct btrfs_fs_info *info)
{
	int next_backup;
	struct btrfs_root_backup *root_backup;
	int last_backup;

	next_backup = info->backup_root_index;
	last_backup = (next_backup + BTRFS_NUM_BACKUP_ROOTS - 1) %
		BTRFS_NUM_BACKUP_ROOTS;

	root_backup = info->super_for_commit->super_roots + last_backup;
	if (btrfs_backup_tree_root_gen(root_backup) ==
	    btrfs_header_generation(info->tree_root->node))
		next_backup = last_backup;

	root_backup = info->super_for_commit->super_roots + next_backup;

	memset(root_backup, 0, sizeof(*root_backup));

	info->backup_root_index = (next_backup + 1) % BTRFS_NUM_BACKUP_ROOTS;

	btrfs_set_backup_tree_root(root_backup, info->tree_root->node->start);
	btrfs_set_backup_tree_root_gen(root_backup,
			       btrfs_header_generation(info->tree_root->node));

	btrfs_set_backup_tree_root_level(root_backup,
			       btrfs_header_level(info->tree_root->node));

	btrfs_set_backup_chunk_root(root_backup, info->chunk_root->node->start);
	btrfs_set_backup_chunk_root_gen(root_backup,
			       btrfs_header_generation(info->chunk_root->node));
	btrfs_set_backup_chunk_root_level(root_backup,
			       btrfs_header_level(info->chunk_root->node));

	btrfs_set_backup_extent_root(root_backup, info->extent_root->node->start);
	btrfs_set_backup_extent_root_gen(root_backup,
			       btrfs_header_generation(info->extent_root->node));
	btrfs_set_backup_extent_root_level(root_backup,
			       btrfs_header_level(info->extent_root->node));

	if (info->fs_root && info->fs_root->node) {
		btrfs_set_backup_fs_root(root_backup,
					 info->fs_root->node->start);
		btrfs_set_backup_fs_root_gen(root_backup,
			       btrfs_header_generation(info->fs_root->node));
		btrfs_set_backup_fs_root_level(root_backup,
			       btrfs_header_level(info->fs_root->node));
	}

	btrfs_set_backup_dev_root(root_backup, info->dev_root->node->start);
	btrfs_set_backup_dev_root_gen(root_backup,
			       btrfs_header_generation(info->dev_root->node));
	btrfs_set_backup_dev_root_level(root_backup,
				       btrfs_header_level(info->dev_root->node));

	btrfs_set_backup_csum_root(root_backup, info->csum_root->node->start);
	btrfs_set_backup_csum_root_gen(root_backup,
			       btrfs_header_generation(info->csum_root->node));
	btrfs_set_backup_csum_root_level(root_backup,
			       btrfs_header_level(info->csum_root->node));

	btrfs_set_backup_total_bytes(root_backup,
			     btrfs_super_total_bytes(info->super_copy));
	btrfs_set_backup_bytes_used(root_backup,
			     btrfs_super_bytes_used(info->super_copy));
	btrfs_set_backup_num_devices(root_backup,
			     btrfs_super_num_devices(info->super_copy));

	memcpy(&info->super_copy->super_roots,
	       &info->super_for_commit->super_roots,
	       sizeof(*root_backup) * BTRFS_NUM_BACKUP_ROOTS);
}

static noinline int next_root_backup(struct btrfs_fs_info *info,
				     struct btrfs_super_block *super,
				     int *num_backups_tried, int *backup_index)
{
	struct btrfs_root_backup *root_backup;
	int newest = *backup_index;

	if (*num_backups_tried == 0) {
		u64 gen = btrfs_super_generation(super);

		newest = find_newest_super_backup(info, gen);
		if (newest == -1)
			return -1;

		*backup_index = newest;
		*num_backups_tried = 1;
	} else if (*num_backups_tried == BTRFS_NUM_BACKUP_ROOTS) {
		 
		return -1;
	} else {
		 
		newest = (*backup_index + BTRFS_NUM_BACKUP_ROOTS - 1) %
			BTRFS_NUM_BACKUP_ROOTS;
		*backup_index = newest;
		*num_backups_tried += 1;
	}
	root_backup = super->super_roots + newest;

	btrfs_set_super_generation(super,
				   btrfs_backup_tree_root_gen(root_backup));
	btrfs_set_super_root(super, btrfs_backup_tree_root(root_backup));
	btrfs_set_super_root_level(super,
				   btrfs_backup_tree_root_level(root_backup));
	btrfs_set_super_bytes_used(super, btrfs_backup_bytes_used(root_backup));

	btrfs_set_super_total_bytes(super, btrfs_backup_total_bytes(root_backup));
	btrfs_set_super_num_devices(super, btrfs_backup_num_devices(root_backup));
	return 0;
}

static void btrfs_stop_all_workers(struct btrfs_fs_info *fs_info)
{
	btrfs_destroy_workqueue(fs_info->fixup_workers);
	btrfs_destroy_workqueue(fs_info->delalloc_workers);
	btrfs_destroy_workqueue(fs_info->workers);
	btrfs_destroy_workqueue(fs_info->endio_workers);
	btrfs_destroy_workqueue(fs_info->endio_meta_workers);
#ifdef MY_ABC_HERE
	btrfs_destroy_workqueue(fs_info->endio_meta_fix_workers);
#endif  
	btrfs_destroy_workqueue(fs_info->endio_raid56_workers);
	btrfs_destroy_workqueue(fs_info->endio_repair_workers);
	btrfs_destroy_workqueue(fs_info->rmw_workers);
	btrfs_destroy_workqueue(fs_info->endio_meta_write_workers);
	btrfs_destroy_workqueue(fs_info->endio_write_workers);
	btrfs_destroy_workqueue(fs_info->endio_freespace_worker);
	btrfs_destroy_workqueue(fs_info->submit_workers);
	btrfs_destroy_workqueue(fs_info->delayed_workers);
	btrfs_destroy_workqueue(fs_info->caching_workers);
	btrfs_destroy_workqueue(fs_info->readahead_workers);
#ifdef MY_ABC_HERE
	btrfs_destroy_workqueue(fs_info->reada_path_workers);
#endif  
	btrfs_destroy_workqueue(fs_info->flush_workers);
#ifdef MY_ABC_HERE
	btrfs_destroy_workqueue(fs_info->flush_meta_workers);
#endif  
	btrfs_destroy_workqueue(fs_info->qgroup_rescan_workers);
	btrfs_destroy_workqueue(fs_info->extent_workers);
#ifdef MY_ABC_HERE
	btrfs_destroy_workqueue(fs_info->syno_nocow_endio_workers);
	btrfs_destroy_workqueue(fs_info->syno_high_priority_endio_workers);
#endif  
}

static void free_root_extent_buffers(struct btrfs_root *root)
{
	if (root) {
		free_extent_buffer(root->node);
		free_extent_buffer(root->commit_root);
		root->node = NULL;
		root->commit_root = NULL;
	}
}

static void free_root_pointers(struct btrfs_fs_info *info, int chunk_root)
{
	free_root_extent_buffers(info->tree_root);

	free_root_extent_buffers(info->dev_root);
#ifdef MY_ABC_HERE
	 
	btrfs_free_root_eb_monitor(info->extent_root);
#endif  
	free_root_extent_buffers(info->extent_root);
	free_root_extent_buffers(info->csum_root);
	free_root_extent_buffers(info->quota_root);
	free_root_extent_buffers(info->uuid_root);
#ifdef MY_ABC_HERE
	free_root_extent_buffers(info->block_group_hint_root);
	free_root_extent_buffers(info->block_group_cache_root);
#endif  
	if (chunk_root)
		free_root_extent_buffers(info->chunk_root);
	free_root_extent_buffers(info->free_space_root);
}

void btrfs_free_fs_roots(struct btrfs_fs_info *fs_info)
{
	int ret;
	struct btrfs_root *gang[8];
	int i;

	while (!list_empty(&fs_info->dead_roots)) {
		gang[0] = list_entry(fs_info->dead_roots.next,
				     struct btrfs_root, root_list);
		list_del(&gang[0]->root_list);

		if (test_bit(BTRFS_ROOT_IN_RADIX, &gang[0]->state)) {
			btrfs_drop_and_free_fs_root(fs_info, gang[0]);
		} else {
			free_extent_buffer(gang[0]->node);
			free_extent_buffer(gang[0]->commit_root);
			btrfs_put_fs_root(gang[0]);
		}
	}

	while (1) {
		ret = radix_tree_gang_lookup(&fs_info->fs_roots_radix,
					     (void **)gang, 0,
					     ARRAY_SIZE(gang));
		if (!ret)
			break;
		for (i = 0; i < ret; i++)
			btrfs_drop_and_free_fs_root(fs_info, gang[i]);
	}

	if (test_bit(BTRFS_FS_STATE_ERROR, &fs_info->fs_state)) {
		btrfs_free_log_root_tree(NULL, fs_info);
		btrfs_destroy_pinned_extent(fs_info->tree_root,
					    fs_info->pinned_extents);
	}
}

static void btrfs_init_scrub(struct btrfs_fs_info *fs_info)
{
	mutex_init(&fs_info->scrub_lock);
	atomic_set(&fs_info->scrubs_running, 0);
	atomic_set(&fs_info->scrub_pause_req, 0);
	atomic_set(&fs_info->scrubs_paused, 0);
	atomic_set(&fs_info->scrub_cancel_req, 0);
	init_waitqueue_head(&fs_info->scrub_pause_wait);
	fs_info->scrub_workers_refcnt = 0;
}

static void btrfs_init_balance(struct btrfs_fs_info *fs_info)
{
	spin_lock_init(&fs_info->balance_lock);
	mutex_init(&fs_info->balance_mutex);
	atomic_set(&fs_info->balance_running, 0);
	atomic_set(&fs_info->balance_pause_req, 0);
	atomic_set(&fs_info->balance_cancel_req, 0);
	fs_info->balance_ctl = NULL;
	init_waitqueue_head(&fs_info->balance_wait_q);
}

static void btrfs_init_btree_inode(struct btrfs_fs_info *fs_info,
				   struct btrfs_root *tree_root)
{
	fs_info->btree_inode->i_ino = BTRFS_BTREE_INODE_OBJECTID;
	set_nlink(fs_info->btree_inode, 1);
	 
	fs_info->btree_inode->i_size = OFFSET_MAX;
	fs_info->btree_inode->i_mapping->a_ops = &btree_aops;

	RB_CLEAR_NODE(&BTRFS_I(fs_info->btree_inode)->rb_node);
	extent_io_tree_init(&BTRFS_I(fs_info->btree_inode)->io_tree,
			     fs_info->btree_inode->i_mapping);
	BTRFS_I(fs_info->btree_inode)->io_tree.track_uptodate = 0;
	extent_map_tree_init(&BTRFS_I(fs_info->btree_inode)->extent_tree);

	BTRFS_I(fs_info->btree_inode)->io_tree.ops = &btree_extent_io_ops;

	BTRFS_I(fs_info->btree_inode)->root = tree_root;
	memset(&BTRFS_I(fs_info->btree_inode)->location, 0,
	       sizeof(struct btrfs_key));
	set_bit(BTRFS_INODE_DUMMY,
		&BTRFS_I(fs_info->btree_inode)->runtime_flags);
	btrfs_insert_inode_hash(fs_info->btree_inode);
}

static void btrfs_init_dev_replace_locks(struct btrfs_fs_info *fs_info)
{
	fs_info->dev_replace.lock_owner = 0;
	atomic_set(&fs_info->dev_replace.nesting_level, 0);
	mutex_init(&fs_info->dev_replace.lock_finishing_cancel_unmount);
	rwlock_init(&fs_info->dev_replace.lock);
	atomic_set(&fs_info->dev_replace.read_locks, 0);
	atomic_set(&fs_info->dev_replace.blocking_readers, 0);
	init_waitqueue_head(&fs_info->replace_wait);
	init_waitqueue_head(&fs_info->dev_replace.read_lock_wq);
}

static void btrfs_init_qgroup(struct btrfs_fs_info *fs_info)
{
	spin_lock_init(&fs_info->qgroup_lock);
	mutex_init(&fs_info->qgroup_ioctl_lock);
	fs_info->qgroup_tree = RB_ROOT;
	fs_info->qgroup_op_tree = RB_ROOT;
	INIT_LIST_HEAD(&fs_info->dirty_qgroups);
	fs_info->qgroup_seq = 1;
	fs_info->quota_enabled = 0;
	fs_info->pending_quota_state = 0;
	fs_info->qgroup_ulist = NULL;
	fs_info->qgroup_rescan_running = false;
	mutex_init(&fs_info->qgroup_rescan_lock);
}

static int btrfs_init_workqueues(struct btrfs_fs_info *fs_info,
		struct btrfs_fs_devices *fs_devices)
{
	int max_active = fs_info->thread_pool_size;
	unsigned int flags = WQ_MEM_RECLAIM | WQ_FREEZABLE | WQ_UNBOUND;

	fs_info->workers =
		btrfs_alloc_workqueue("worker", flags | WQ_HIGHPRI,
				      max_active, 16);

	fs_info->delalloc_workers =
		btrfs_alloc_workqueue("delalloc", flags, max_active, 2);

	fs_info->flush_workers =
		btrfs_alloc_workqueue("flush_delalloc", flags, max_active, 0);

#ifdef MY_ABC_HERE
	fs_info->flush_meta_workers =
		btrfs_alloc_workqueue("flush_meta", flags, max_active, 0);
#endif  

	fs_info->caching_workers =
		btrfs_alloc_workqueue("cache", flags, max_active, 0);

	fs_info->submit_workers =
		btrfs_alloc_workqueue("submit", flags,
				      min_t(u64, fs_devices->num_devices,
					    max_active), 64);

	fs_info->fixup_workers =
		btrfs_alloc_workqueue("fixup", flags, 1, 0);

	fs_info->endio_workers =
		btrfs_alloc_workqueue("endio", flags, max_active, 4);
	fs_info->endio_meta_workers =
		btrfs_alloc_workqueue("endio-meta", flags, max_active, 4);
#ifdef MY_ABC_HERE
	fs_info->endio_meta_fix_workers =
		btrfs_alloc_workqueue("endio-meta-fix", flags, max_active, 4);
#endif  
	fs_info->endio_meta_write_workers =
		btrfs_alloc_workqueue("endio-meta-write", flags, max_active, 2);
	fs_info->endio_raid56_workers =
		btrfs_alloc_workqueue("endio-raid56", flags, max_active, 4);
	fs_info->endio_repair_workers =
		btrfs_alloc_workqueue("endio-repair", flags, 1, 0);
	fs_info->rmw_workers =
		btrfs_alloc_workqueue("rmw", flags, max_active, 2);
	fs_info->endio_write_workers =
#ifdef MY_ABC_HERE
		btrfs_alloc_workqueue("endio-write", flags, min_t(unsigned long, 4, max_active), 2);
#else
		btrfs_alloc_workqueue("endio-write", flags, max_active, 2);
#endif  
	fs_info->endio_freespace_worker =
		btrfs_alloc_workqueue("freespace-write", flags, max_active, 0);
	fs_info->delayed_workers =
		btrfs_alloc_workqueue("delayed-meta", flags, max_active, 0);
	fs_info->readahead_workers =
		btrfs_alloc_workqueue("readahead", flags, max_active, 2);
#ifdef MY_ABC_HERE
	fs_info->reada_path_workers =
		btrfs_alloc_workqueue("reada-path", flags, max_active, 2);
#endif  
	fs_info->qgroup_rescan_workers =
		btrfs_alloc_workqueue("qgroup-rescan", flags, 1, 0);
	fs_info->extent_workers =
		btrfs_alloc_workqueue("extent-refs", flags,
#ifdef MY_ABC_HERE
				      min_t(u64, 4,
#else
				      min_t(u64, fs_devices->num_devices,
#endif  
					    max_active), 8);
#ifdef MY_ABC_HERE
	fs_info->syno_nocow_endio_workers =
		btrfs_alloc_workqueue("syno_nocow", flags, max_active, 2);
	fs_info->syno_high_priority_endio_workers =
		btrfs_alloc_workqueue("syno_high_priority", flags, max_active, 2);
#endif  

	if (!(fs_info->workers && fs_info->delalloc_workers &&
	      fs_info->submit_workers && fs_info->flush_workers &&
	      fs_info->endio_workers && fs_info->endio_meta_workers &&
	      fs_info->endio_meta_write_workers &&
	      fs_info->endio_repair_workers &&
	      fs_info->endio_write_workers && fs_info->endio_raid56_workers &&
	      fs_info->endio_freespace_worker && fs_info->rmw_workers &&
	      fs_info->caching_workers && fs_info->readahead_workers &&
	      fs_info->fixup_workers && fs_info->delayed_workers &&
	      fs_info->extent_workers &&
#ifdef MY_ABC_HERE
	      fs_info->flush_meta_workers &&
#endif  
#ifdef MY_ABC_HERE
	      fs_info->syno_nocow_endio_workers && fs_info->syno_high_priority_endio_workers &&
#endif  
	      fs_info->qgroup_rescan_workers)) {
		return -ENOMEM;
	}

	return 0;
}

static int btrfs_replay_log(struct btrfs_fs_info *fs_info,
			    struct btrfs_fs_devices *fs_devices)
{
	int ret;
	struct btrfs_root *tree_root = fs_info->tree_root;
	struct btrfs_root *log_tree_root;
	struct btrfs_super_block *disk_super = fs_info->super_copy;
	u64 bytenr = btrfs_super_log_root(disk_super);

	if (fs_devices->rw_devices == 0) {
		btrfs_warn(fs_info, "log replay required on RO media");
		return -EIO;
	}

#ifdef MY_ABC_HERE
	log_tree_root = btrfs_alloc_root(fs_info, GFP_KERNEL, 0);
#else
	log_tree_root = btrfs_alloc_root(fs_info, GFP_KERNEL);
#endif  
	if (!log_tree_root)
		return -ENOMEM;

	__setup_root(tree_root->nodesize, tree_root->sectorsize,
			tree_root->stripesize, log_tree_root, fs_info,
			BTRFS_TREE_LOG_OBJECTID);

	log_tree_root->node = read_tree_block(tree_root, bytenr,
			fs_info->generation + 1);
	if (IS_ERR(log_tree_root->node)) {
		btrfs_warn(fs_info, "failed to read log tree");
		ret = PTR_ERR(log_tree_root->node);
#ifdef MY_ABC_HERE
		btrfs_free_root_eb_monitor(log_tree_root);
#endif  
		kfree(log_tree_root);
		return ret;
	} else if (!extent_buffer_uptodate(log_tree_root->node)) {
		btrfs_err(fs_info, "failed to read log tree");
		free_extent_buffer(log_tree_root->node);
#ifdef MY_ABC_HERE
		btrfs_free_root_eb_monitor(log_tree_root);
#endif  
		kfree(log_tree_root);
		return -EIO;
	}
	 
	ret = btrfs_recover_log_trees(log_tree_root);
	if (ret) {
		btrfs_handle_fs_error(tree_root->fs_info, ret,
			    "Failed to recover log tree");
		free_extent_buffer(log_tree_root->node);
		kfree(log_tree_root);
		return ret;
	}

	if (fs_info->sb->s_flags & MS_RDONLY) {
		ret = btrfs_commit_super(tree_root);
		if (ret)
			return ret;
	}

	return 0;
}

static int btrfs_read_roots(struct btrfs_fs_info *fs_info,
			    struct btrfs_root *tree_root)
{
	struct btrfs_root *root;
	struct btrfs_key location;
	int ret;

	location.objectid = BTRFS_EXTENT_TREE_OBJECTID;
	location.type = BTRFS_ROOT_ITEM_KEY;
	location.offset = 0;

	root = btrfs_read_tree_root(tree_root, &location);
	if (IS_ERR(root))
		return PTR_ERR(root);
	set_bit(BTRFS_ROOT_TRACK_DIRTY, &root->state);
	fs_info->extent_root = root;

	location.objectid = BTRFS_DEV_TREE_OBJECTID;
	root = btrfs_read_tree_root(tree_root, &location);
	if (IS_ERR(root))
		return PTR_ERR(root);
	set_bit(BTRFS_ROOT_TRACK_DIRTY, &root->state);
	fs_info->dev_root = root;
	btrfs_init_devices_late(fs_info);

	location.objectid = BTRFS_CSUM_TREE_OBJECTID;
	root = btrfs_read_tree_root(tree_root, &location);
	if (IS_ERR(root))
		return PTR_ERR(root);
	set_bit(BTRFS_ROOT_TRACK_DIRTY, &root->state);
	fs_info->csum_root = root;

	location.objectid = BTRFS_QUOTA_TREE_OBJECTID;
#ifdef MY_ABC_HERE
	if (btrfs_test_opt(tree_root, NO_QUOTA_TREE))
		root = ERR_PTR(-ENOENT);
	else
#endif  
	root = btrfs_read_tree_root(tree_root, &location);
	if (!IS_ERR(root)) {
		set_bit(BTRFS_ROOT_TRACK_DIRTY, &root->state);
		fs_info->quota_enabled = 1;
		fs_info->pending_quota_state = 1;
		fs_info->quota_root = root;
	}

	location.objectid = BTRFS_UUID_TREE_OBJECTID;
	root = btrfs_read_tree_root(tree_root, &location);
	if (IS_ERR(root)) {
		ret = PTR_ERR(root);
		if (ret != -ENOENT)
			return ret;
	} else {
		set_bit(BTRFS_ROOT_TRACK_DIRTY, &root->state);
		fs_info->uuid_root = root;
	}

#ifdef MY_ABC_HERE
	if (!fs_info->no_block_group_hint) {
		mutex_init(&fs_info->block_group_hint_tree_mutex);
		location.objectid = BTRFS_BLOCK_GROUP_HINT_TREE_OBJECTID;
		root = btrfs_read_tree_root(tree_root, &location);
		if (IS_ERR_OR_NULL(root))
			fs_info->block_group_hint_root = NULL;
		else {
			set_bit(BTRFS_ROOT_TRACK_DIRTY, &root->state);
			fs_info->block_group_hint_root = root;
		}
	}
	if (btrfs_fs_compat(fs_info, BLOCK_GROUP_CACHE_TREE)) {
		location.objectid = BTRFS_BLOCK_GROUP_CACHE_TREE_OBJECTID;
		root = btrfs_read_tree_root(tree_root, &location);
		if (IS_ERR(root)) {
			return PTR_ERR(root);
		}
		set_bit(BTRFS_ROOT_TRACK_DIRTY, &root->state);
		fs_info->block_group_cache_root = root;
		 
		ret = btrfs_check_syno_block_group_cache_tree(fs_info);
		if (ret) {
			fs_info->block_group_cache_tree_broken = 1;
			WARN_ON_ONCE(1);
		}
	}
#endif  

	if (btrfs_fs_compat_ro(fs_info, FREE_SPACE_TREE)) {
		location.objectid = BTRFS_FREE_SPACE_TREE_OBJECTID;
		root = btrfs_read_tree_root(tree_root, &location);
		if (IS_ERR(root))
			return PTR_ERR(root);
		set_bit(BTRFS_ROOT_TRACK_DIRTY, &root->state);
		fs_info->free_space_root = root;
	}

	return 0;
}

static int validate_super(struct btrfs_fs_info *fs_info,
			    struct btrfs_super_block *sb, int mirror_num)
{
	u64 nodesize = btrfs_super_nodesize(sb);
	u64 sectorsize = btrfs_super_sectorsize(sb);
	int ret = 0;

	if (btrfs_super_magic(sb) != BTRFS_MAGIC) {
		printk(KERN_ERR "BTRFS: no valid FS found\n");
		ret = -EINVAL;
	}
	if (btrfs_super_flags(sb) & ~BTRFS_SUPER_FLAG_SUPP)
		printk(KERN_WARNING "BTRFS: unrecognized super flag: %llu\n",
				btrfs_super_flags(sb) & ~BTRFS_SUPER_FLAG_SUPP);
	if (btrfs_super_root_level(sb) >= BTRFS_MAX_LEVEL) {
		printk(KERN_ERR "BTRFS: tree_root level too big: %d >= %d\n",
				btrfs_super_root_level(sb), BTRFS_MAX_LEVEL);
		ret = -EINVAL;
	}
	if (btrfs_super_chunk_root_level(sb) >= BTRFS_MAX_LEVEL) {
		printk(KERN_ERR "BTRFS: chunk_root level too big: %d >= %d\n",
				btrfs_super_chunk_root_level(sb), BTRFS_MAX_LEVEL);
		ret = -EINVAL;
	}
	if (btrfs_super_log_root_level(sb) >= BTRFS_MAX_LEVEL) {
		printk(KERN_ERR "BTRFS: log_root level too big: %d >= %d\n",
				btrfs_super_log_root_level(sb), BTRFS_MAX_LEVEL);
		ret = -EINVAL;
	}

	if (!is_power_of_2(sectorsize) || sectorsize < 4096 ||
	    sectorsize > BTRFS_MAX_METADATA_BLOCKSIZE) {
		printk(KERN_ERR "BTRFS: invalid sectorsize %llu\n", sectorsize);
		ret = -EINVAL;
	}
	 
	if (sectorsize != PAGE_CACHE_SIZE) {
		printk(KERN_ERR "BTRFS: sectorsize %llu not supported yet, only support %lu\n",
				sectorsize, PAGE_CACHE_SIZE);
		ret = -EINVAL;
	}
	if (!is_power_of_2(nodesize) || nodesize < sectorsize ||
	    nodesize > BTRFS_MAX_METADATA_BLOCKSIZE) {
		printk(KERN_ERR "BTRFS: invalid nodesize %llu\n", nodesize);
		ret = -EINVAL;
	}
	if (nodesize != le32_to_cpu(sb->__unused_leafsize)) {
		printk(KERN_ERR "BTRFS: invalid leafsize %u, should be %llu\n",
				le32_to_cpu(sb->__unused_leafsize),
				nodesize);
		ret = -EINVAL;
	}

	if (!IS_ALIGNED(btrfs_super_root(sb), sectorsize)) {
		printk(KERN_WARNING "BTRFS: tree_root block unaligned: %llu\n",
				btrfs_super_root(sb));
		ret = -EINVAL;
	}
	if (!IS_ALIGNED(btrfs_super_chunk_root(sb), sectorsize)) {
		printk(KERN_WARNING "BTRFS: chunk_root block unaligned: %llu\n",
				btrfs_super_chunk_root(sb));
		ret = -EINVAL;
	}
	if (!IS_ALIGNED(btrfs_super_log_root(sb), sectorsize)) {
		printk(KERN_WARNING "BTRFS: log_root block unaligned: %llu\n",
				btrfs_super_log_root(sb));
		ret = -EINVAL;
	}

	if (memcmp(fs_info->fsid, sb->dev_item.fsid, BTRFS_UUID_SIZE) != 0) {
		printk(KERN_ERR "BTRFS: dev_item UUID does not match fsid: %pU != %pU\n",
				fs_info->fsid, sb->dev_item.fsid);
		ret = -EINVAL;
	}

	if (btrfs_super_bytes_used(sb) < 6 * btrfs_super_nodesize(sb)) {
		btrfs_err(fs_info, "bytes_used is too small %llu",
		       btrfs_super_bytes_used(sb));
		ret = -EINVAL;
	}
	if (!is_power_of_2(btrfs_super_stripesize(sb)) ||
		((btrfs_super_stripesize(sb) != sectorsize) &&
			(btrfs_super_stripesize(sb) != 4096))) {
		btrfs_err(fs_info, "invalid stripesize %u",
		       btrfs_super_stripesize(sb));
		ret = -EINVAL;
	}
	if (btrfs_super_num_devices(sb) > (1UL << 31))
		printk(KERN_WARNING "BTRFS: suspicious number of devices: %llu\n",
				btrfs_super_num_devices(sb));
	if (btrfs_super_num_devices(sb) == 0) {
		printk(KERN_ERR "BTRFS: number of devices is 0\n");
		ret = -EINVAL;
	}

	if (mirror_num >= 0 &&
	    btrfs_super_bytenr(sb) != btrfs_sb_offset(mirror_num)) {
		printk(KERN_ERR "BTRFS: super offset mismatch %llu != %u\n",
				btrfs_super_bytenr(sb), BTRFS_SUPER_INFO_OFFSET);
		ret = -EINVAL;
	}

	if (btrfs_super_sys_array_size(sb) > BTRFS_SYSTEM_CHUNK_ARRAY_SIZE) {
		printk(KERN_ERR "BTRFS: system chunk array too big %u > %u\n",
				btrfs_super_sys_array_size(sb),
				BTRFS_SYSTEM_CHUNK_ARRAY_SIZE);
		ret = -EINVAL;
	}
	if (btrfs_super_sys_array_size(sb) < sizeof(struct btrfs_disk_key)
			+ sizeof(struct btrfs_chunk)) {
		printk(KERN_ERR "BTRFS: system chunk array too small %u < %zu\n",
				btrfs_super_sys_array_size(sb),
				sizeof(struct btrfs_disk_key)
				+ sizeof(struct btrfs_chunk));
		ret = -EINVAL;
	}

	if (btrfs_super_generation(sb) < btrfs_super_chunk_root_generation(sb))
		printk(KERN_WARNING
			"BTRFS: suspicious: generation < chunk_root_generation: %llu < %llu\n",
			btrfs_super_generation(sb), btrfs_super_chunk_root_generation(sb));
	if (btrfs_super_generation(sb) < btrfs_super_cache_generation(sb)
	    && btrfs_super_cache_generation(sb) != (u64)-1)
		printk(KERN_WARNING
			"BTRFS: suspicious: generation < cache_generation: %llu < %llu\n",
			btrfs_super_generation(sb), btrfs_super_cache_generation(sb));

	return ret;
}

static int btrfs_validate_mount_super(struct btrfs_fs_info *fs_info)
{
	return validate_super(fs_info, fs_info->super_copy, 0);
}

static int btrfs_validate_write_super(struct btrfs_fs_info *fs_info,
				      struct btrfs_super_block *sb)
{
	int ret;

	ret = validate_super(fs_info, sb, -1);
	if (ret < 0)
		goto out;
	if (btrfs_super_csum_type(sb) != BTRFS_CSUM_TYPE_CRC32) {
		ret = -EUCLEAN;
		btrfs_err(fs_info, "invalid csum type, has %u want %u",
			  btrfs_super_csum_type(sb), BTRFS_CSUM_TYPE_CRC32);
		goto out;
	}
	if (btrfs_super_incompat_flags(sb) & ~BTRFS_FEATURE_INCOMPAT_SUPP) {
		ret = -EUCLEAN;
		btrfs_err(fs_info,
		"invalid incompat flags, has 0x%llx valid mask 0x%llx",
			  btrfs_super_incompat_flags(sb),
			  (unsigned long long)BTRFS_FEATURE_INCOMPAT_SUPP);
		goto out;
	}
out:
	if (ret < 0)
		btrfs_err(fs_info,
		"super block corruption detected before writing it to disk");
	return ret;
}

int open_ctree(struct super_block *sb,
	       struct btrfs_fs_devices *fs_devices,
	       char *options)
{
	u32 sectorsize;
	u32 nodesize;
	u32 stripesize;
	u64 generation;
	u64 features;
	struct btrfs_key location;
	struct buffer_head *bh;
	struct btrfs_super_block *disk_super;
	struct btrfs_fs_info *fs_info = btrfs_sb(sb);
	struct btrfs_root *tree_root;
	struct btrfs_root *chunk_root;
	int ret;
	int err = -EINVAL;
	int num_backups_tried = 0;
	int backup_index = 0;
	int max_active;
	int clear_free_space_tree = 0;

#ifdef MY_ABC_HERE
	fs_info->snapshot_cleaner = 1;
#endif  

#ifdef MY_ABC_HERE
	ret = percpu_counter_init(&fs_info->eb_hit, 0, GFP_KERNEL);
	if (ret) {
		err = ret;
		goto fail;
	}

	ret = percpu_counter_init(&fs_info->eb_miss, 0, GFP_KERNEL);
	if (ret) {
		err = ret;
		goto fail;
	}

	ret = percpu_counter_init(&fs_info->meta_write_pages, 0, GFP_KERNEL);
	if (ret) {
		err = ret;
		goto fail;
	}

	ret = percpu_counter_init(&fs_info->data_write_pages, 0, GFP_KERNEL);
	if (ret) {
		err = ret;
		goto fail;
	}

	ret = percpu_counter_init(&fs_info->delayed_meta_ref, 0, GFP_KERNEL);
	if (ret) {
		err = ret;
		goto fail;
	}

	ret = percpu_counter_init(&fs_info->delayed_data_ref, 0, GFP_KERNEL);
	if (ret) {
		err = ret;
		goto fail;
	}

	ret = percpu_counter_init(&fs_info->write_flush, 0, GFP_KERNEL);
	if (ret) {
		err = ret;
		goto fail;
	}

	ret = percpu_counter_init(&fs_info->write_fua, 0, GFP_KERNEL);
	if (ret) {
		err = ret;
		goto fail;
	}
#endif  

#ifdef MY_ABC_HERE
	tree_root = fs_info->tree_root = btrfs_alloc_root(fs_info, GFP_KERNEL, 0);
	chunk_root = fs_info->chunk_root = btrfs_alloc_root(fs_info, GFP_KERNEL, 0);
#else
	tree_root = fs_info->tree_root = btrfs_alloc_root(fs_info, GFP_KERNEL);
	chunk_root = fs_info->chunk_root = btrfs_alloc_root(fs_info, GFP_KERNEL);
#endif  
	if (!tree_root || !chunk_root) {
		err = -ENOMEM;
		goto fail;
	}

	ret = init_srcu_struct(&fs_info->subvol_srcu);
	if (ret) {
		err = ret;
		goto fail;
	}

	ret = setup_bdi(fs_info, &fs_info->bdi);
	if (ret) {
		err = ret;
		goto fail_srcu;
	}

	ret = percpu_counter_init(&fs_info->dirty_metadata_bytes, 0, GFP_KERNEL);
	if (ret) {
		err = ret;
		goto fail_bdi;
	}
	fs_info->dirty_metadata_batch = PAGE_CACHE_SIZE *
					(1 + ilog2(nr_cpu_ids));

#ifdef MY_ABC_HERE
	atomic_set(&fs_info->btree_flusher, 0);
#endif  

	ret = percpu_counter_init(&fs_info->delalloc_bytes, 0, GFP_KERNEL);
	if (ret) {
		err = ret;
		goto fail_dirty_metadata_bytes;
	}

	ret = percpu_counter_init(&fs_info->bio_counter, 0, GFP_KERNEL);
	if (ret) {
		err = ret;
		goto fail_delalloc_bytes;
	}

	fs_info->btree_inode = new_inode(sb);
	if (!fs_info->btree_inode) {
		err = -ENOMEM;
		goto fail_bio_counter;
	}

	mapping_set_gfp_mask(fs_info->btree_inode->i_mapping, GFP_NOFS);

	INIT_RADIX_TREE(&fs_info->fs_roots_radix, GFP_ATOMIC);
	INIT_RADIX_TREE(&fs_info->buffer_radix, GFP_ATOMIC);
	INIT_LIST_HEAD(&fs_info->trans_list);
	INIT_LIST_HEAD(&fs_info->dead_roots);
	INIT_LIST_HEAD(&fs_info->delayed_iputs);
	INIT_LIST_HEAD(&fs_info->delalloc_roots);
	INIT_LIST_HEAD(&fs_info->caching_block_groups);
	spin_lock_init(&fs_info->delalloc_root_lock);
	spin_lock_init(&fs_info->trans_lock);
	spin_lock_init(&fs_info->fs_roots_radix_lock);
	spin_lock_init(&fs_info->delayed_iput_lock);
	spin_lock_init(&fs_info->defrag_inodes_lock);
	spin_lock_init(&fs_info->free_chunk_lock);
	spin_lock_init(&fs_info->tree_mod_seq_lock);
	spin_lock_init(&fs_info->super_lock);
	spin_lock_init(&fs_info->qgroup_op_lock);
	spin_lock_init(&fs_info->buffer_lock);
	spin_lock_init(&fs_info->unused_bgs_lock);
	rwlock_init(&fs_info->tree_mod_log_lock);
	mutex_init(&fs_info->unused_bg_unpin_mutex);
	mutex_init(&fs_info->delete_unused_bgs_mutex);
	mutex_init(&fs_info->reloc_mutex);
	mutex_init(&fs_info->delalloc_root_mutex);
	mutex_init(&fs_info->cleaner_delayed_iput_mutex);
	seqlock_init(&fs_info->profiles_lock);

	INIT_LIST_HEAD(&fs_info->dirty_cowonly_roots);
	INIT_LIST_HEAD(&fs_info->space_info);
	INIT_LIST_HEAD(&fs_info->tree_mod_seq_list);
	INIT_LIST_HEAD(&fs_info->unused_bgs);
	btrfs_mapping_init(&fs_info->mapping_tree);
#ifdef MY_ABC_HERE
	atomic_set(&fs_info->nr_extent_maps, 0);
	INIT_LIST_HEAD(&fs_info->extent_map_inode_list);
	spin_lock_init(&fs_info->extent_map_inode_list_lock);
#endif  
	btrfs_init_block_rsv(&fs_info->global_block_rsv,
			     BTRFS_BLOCK_RSV_GLOBAL);
	btrfs_init_block_rsv(&fs_info->delalloc_block_rsv,
			     BTRFS_BLOCK_RSV_DELALLOC);
	btrfs_init_block_rsv(&fs_info->trans_block_rsv, BTRFS_BLOCK_RSV_TRANS);
	btrfs_init_block_rsv(&fs_info->chunk_block_rsv, BTRFS_BLOCK_RSV_CHUNK);
	btrfs_init_block_rsv(&fs_info->empty_block_rsv, BTRFS_BLOCK_RSV_EMPTY);
	btrfs_init_block_rsv(&fs_info->delayed_block_rsv,
			     BTRFS_BLOCK_RSV_DELOPS);
	atomic_set(&fs_info->nr_async_submits, 0);
	atomic_set(&fs_info->async_delalloc_pages, 0);
	atomic_set(&fs_info->async_submit_draining, 0);
	atomic_set(&fs_info->nr_async_bios, 0);
	atomic_set(&fs_info->defrag_running, 0);
	atomic_set(&fs_info->qgroup_op_seq, 0);
	atomic_set(&fs_info->reada_works_cnt, 0);
	atomic64_set(&fs_info->tree_mod_seq, 0);
	fs_info->fs_frozen = 0;
	fs_info->sb = sb;
	fs_info->max_inline = BTRFS_DEFAULT_MAX_INLINE;
	fs_info->metadata_ratio = 0;
	fs_info->defrag_inodes = RB_ROOT;
#ifdef MY_ABC_HERE
	INIT_LIST_HEAD(&fs_info->defrag_inodes_list[0]);
	INIT_LIST_HEAD(&fs_info->defrag_inodes_list[1]);
	fs_info->reclaim_space_entry_count = 0;
#endif  
#ifdef MY_ABC_HERE
	fs_info->block_group_cnt = 0;
#endif  
	fs_info->free_chunk_space = 0;
	fs_info->tree_mod_log = RB_ROOT;
	fs_info->commit_interval = BTRFS_DEFAULT_COMMIT_INTERVAL;
	fs_info->avg_delayed_ref_runtime = NSEC_PER_SEC >> 6;  
	 
	INIT_RADIX_TREE(&fs_info->reada_tree, GFP_NOFS & ~__GFP_DIRECT_RECLAIM);
	spin_lock_init(&fs_info->reada_lock);
#ifdef MY_ABC_HERE
	spin_lock_init(&fs_info->syno_delayed_ref_throttle_lock);
	INIT_LIST_HEAD(&fs_info->syno_delayed_ref_throttle_tickets);
#endif  

	fs_info->thread_pool_size = min_t(unsigned long,
					  num_online_cpus() + 2, 8);

	INIT_LIST_HEAD(&fs_info->ordered_roots);
	spin_lock_init(&fs_info->ordered_root_lock);
	fs_info->delayed_root = kmalloc(sizeof(struct btrfs_delayed_root),
					GFP_KERNEL);
	if (!fs_info->delayed_root) {
		err = -ENOMEM;
		goto fail_iput;
	}
	btrfs_init_delayed_root(fs_info->delayed_root);

	btrfs_init_scrub(fs_info);
#ifdef CONFIG_BTRFS_FS_CHECK_INTEGRITY
	fs_info->check_integrity_print_mask = 0;
#endif
	btrfs_init_balance(fs_info);
	btrfs_init_async_reclaim_work(&fs_info->async_reclaim_work);
#ifdef MY_ABC_HERE
	btrfs_init_async_delayed_ref_work(&fs_info->async_delayed_ref_work);
#endif  
#ifdef MY_ABC_HERE
	atomic_set(&fs_info->syno_writeback_thread_count, 0);
	fs_info->syno_writeback_thread_max = 0;
	fs_info->dev_replace_may_start = 0;
#endif  
#ifdef MY_ABC_HERE
	atomic_set(&fs_info->syno_async_submit_nr, 0);
	fs_info->syno_async_submit_throttle = 128;
	init_waitqueue_head(&fs_info->syno_async_submit_queue_wait);
#endif  
#ifdef MY_ABC_HERE
	atomic_set(&fs_info->syno_ordered_extent_nr, 0);
	fs_info->syno_max_ordered_queue_size = 65536;
	init_waitqueue_head(&fs_info->syno_ordered_queue_wait);
#endif  
#ifdef MY_ABC_HERE
	fs_info->avoid_fs_root_null_pointer_dereference = 1;
#endif  

#ifdef MY_ABC_HERE
	atomic64_set(&fs_info->fsync_cnt, 0);
	atomic64_set(&fs_info->fsync_full_commit_cnt, 0);
	fs_info->commit_time_debug = 0;
#endif  

	sb->s_blocksize = 4096;
	sb->s_blocksize_bits = blksize_bits(4096);
	sb->s_bdi = &fs_info->bdi;

	btrfs_init_btree_inode(fs_info, tree_root);

	spin_lock_init(&fs_info->block_group_cache_lock);
	fs_info->block_group_cache_tree = RB_ROOT;
	fs_info->first_logical_byte = (u64)-1;

	extent_io_tree_init(&fs_info->freed_extents[0],
			     fs_info->btree_inode->i_mapping);
	extent_io_tree_init(&fs_info->freed_extents[1],
			     fs_info->btree_inode->i_mapping);
	fs_info->pinned_extents = &fs_info->freed_extents[0];
	fs_info->do_barriers = 1;

	mutex_init(&fs_info->ordered_operations_mutex);
	mutex_init(&fs_info->tree_log_mutex);
	mutex_init(&fs_info->chunk_mutex);
	mutex_init(&fs_info->transaction_kthread_mutex);
	mutex_init(&fs_info->cleaner_mutex);
	mutex_init(&fs_info->volume_mutex);
	mutex_init(&fs_info->ro_block_group_mutex);
	init_rwsem(&fs_info->commit_root_sem);
	init_rwsem(&fs_info->cleanup_work_sem);
	init_rwsem(&fs_info->subvol_sem);
	sema_init(&fs_info->uuid_tree_rescan_sem, 1);

	btrfs_init_dev_replace_locks(fs_info);

#ifdef MY_ABC_HERE
	fs_info->metadata_ratio = 50;
#endif  

	btrfs_init_qgroup(fs_info);

	btrfs_init_free_cluster(&fs_info->meta_alloc_cluster);
	btrfs_init_free_cluster(&fs_info->data_alloc_cluster);

	init_waitqueue_head(&fs_info->transaction_throttle);
	init_waitqueue_head(&fs_info->transaction_wait);
	init_waitqueue_head(&fs_info->transaction_blocked_wait);
	init_waitqueue_head(&fs_info->async_submit_wait);

	INIT_LIST_HEAD(&fs_info->pinned_chunks);

	ret = btrfs_alloc_stripe_hash_table(fs_info);
	if (ret) {
		err = ret;
		goto fail_alloc;
	}

	__setup_root(4096, 4096, 4096, tree_root,
		     fs_info, BTRFS_ROOT_TREE_OBJECTID);

	invalidate_bdev(fs_devices->latest_bdev);

	bh = btrfs_read_dev_super(fs_devices->latest_bdev);
	if (IS_ERR(bh)) {
		err = PTR_ERR(bh);
		goto fail_alloc;
	}

	if (btrfs_check_super_csum(bh->b_data)) {
		btrfs_err(fs_info, "superblock checksum mismatch");
		err = -EINVAL;
		brelse(bh);
		goto fail_alloc;
	}

	memcpy(fs_info->super_copy, bh->b_data, sizeof(*fs_info->super_copy));
	memcpy(fs_info->super_for_commit, fs_info->super_copy,
	       sizeof(*fs_info->super_for_commit));
	brelse(bh);

	memcpy(fs_info->fsid, fs_info->super_copy->fsid, BTRFS_FSID_SIZE);

	ret = btrfs_validate_mount_super(fs_info);
	if (ret) {
		btrfs_err(fs_info, "superblock contains fatal errors");
		err = -EINVAL;
		goto fail_alloc;
	}

	disk_super = fs_info->super_copy;
	if (!btrfs_super_root(disk_super))
		goto fail_alloc;

#ifdef MY_ABC_HERE
	 
	if (btrfs_super_total_bytes(disk_super) < 1024ULL * 1024 * 1024 * 1024)
		fs_info->data_alloc_cluster.empty_cluster = 512ULL * 1024 * 1024;
#endif  

	if (btrfs_super_flags(disk_super) & BTRFS_SUPER_FLAG_ERROR)
		set_bit(BTRFS_FS_STATE_ERROR, &fs_info->fs_state);

	generation = btrfs_super_generation(disk_super);
	find_oldest_super_backup(fs_info, generation);

#ifdef MY_ABC_HERE
	fs_info->compress_type = BTRFS_COMPRESS_DEFAULT;
#else
	fs_info->compress_type = BTRFS_COMPRESS_ZLIB;
#endif  

	ret = btrfs_parse_options(tree_root, options, sb->s_flags);
	if (ret) {
		err = ret;
		goto fail_alloc;
	}

	features = btrfs_super_incompat_flags(disk_super) &
		~BTRFS_FEATURE_INCOMPAT_SUPP;
	if (features) {
		btrfs_err(fs_info,
		    "cannot mount because of unsupported optional features (%llx)",
		    features);
		err = -EINVAL;
		goto fail_alloc;
	}

	features = btrfs_super_incompat_flags(disk_super);
	features |= BTRFS_FEATURE_INCOMPAT_MIXED_BACKREF;
	if (tree_root->fs_info->compress_type == BTRFS_COMPRESS_LZO)
		features |= BTRFS_FEATURE_INCOMPAT_COMPRESS_LZO;

	if (features & BTRFS_FEATURE_INCOMPAT_SKINNY_METADATA)
		btrfs_info(fs_info, "has skinny extents");

	if (btrfs_super_nodesize(disk_super) > PAGE_CACHE_SIZE) {
		if (!(features & BTRFS_FEATURE_INCOMPAT_BIG_METADATA))
			btrfs_info(fs_info,
				"flagging fs with big metadata feature");
		features |= BTRFS_FEATURE_INCOMPAT_BIG_METADATA;
	}

	nodesize = btrfs_super_nodesize(disk_super);
	sectorsize = btrfs_super_sectorsize(disk_super);
	stripesize = btrfs_super_stripesize(disk_super);
	fs_info->dirty_metadata_batch = nodesize * (1 + ilog2(nr_cpu_ids));
	fs_info->delalloc_batch = sectorsize * 512 * (1 + ilog2(nr_cpu_ids));

	if ((features & BTRFS_FEATURE_INCOMPAT_MIXED_GROUPS) &&
	    (sectorsize != nodesize)) {
		btrfs_err(fs_info,
"unequal nodesize/sectorsize (%u != %u) are not allowed for mixed block groups",
			nodesize, sectorsize);
		goto fail_alloc;
	}

	btrfs_set_super_incompat_flags(disk_super, features);

	features = btrfs_super_compat_ro_flags(disk_super) &
		~BTRFS_FEATURE_COMPAT_RO_SUPP;
	if (!(sb->s_flags & MS_RDONLY) && features) {
		btrfs_err(fs_info,
	"cannot mount read-write because of unsupported optional features (%llx)",
		       features);
		err = -EINVAL;
		goto fail_alloc;
	}

	max_active = fs_info->thread_pool_size;

	ret = btrfs_init_workqueues(fs_info, fs_devices);
	if (ret) {
		err = ret;
		goto fail_sb_buffer;
	}

	fs_info->bdi.ra_pages *= btrfs_super_num_devices(disk_super);
	fs_info->bdi.ra_pages = max(fs_info->bdi.ra_pages,
				    SZ_4M / PAGE_CACHE_SIZE);

	tree_root->nodesize = nodesize;
	tree_root->sectorsize = sectorsize;
	tree_root->stripesize = stripesize;

	sb->s_blocksize = sectorsize;
	sb->s_blocksize_bits = blksize_bits(sectorsize);

	mutex_lock(&fs_info->chunk_mutex);
	ret = btrfs_read_sys_array(tree_root);
	mutex_unlock(&fs_info->chunk_mutex);
	if (ret) {
		btrfs_err(fs_info, "failed to read the system array: %d", ret);
		goto fail_sb_buffer;
	}

	generation = btrfs_super_chunk_root_generation(disk_super);

	__setup_root(nodesize, sectorsize, stripesize, chunk_root,
		     fs_info, BTRFS_CHUNK_TREE_OBJECTID);

	chunk_root->node = read_tree_block(chunk_root,
					   btrfs_super_chunk_root(disk_super),
					   generation);
	if (IS_ERR(chunk_root->node) ||
	    !extent_buffer_uptodate(chunk_root->node)) {
		btrfs_err(fs_info, "failed to read chunk root");
		if (!IS_ERR(chunk_root->node))
			free_extent_buffer(chunk_root->node);
		chunk_root->node = NULL;
		goto fail_tree_roots;
	}
	btrfs_set_root_node(&chunk_root->root_item, chunk_root->node);
	chunk_root->commit_root = btrfs_root_node(chunk_root);

	read_extent_buffer(chunk_root->node, fs_info->chunk_tree_uuid,
	   btrfs_header_chunk_tree_uuid(chunk_root->node), BTRFS_UUID_SIZE);

	ret = btrfs_read_chunk_tree(chunk_root);
	if (ret) {
		btrfs_err(fs_info, "failed to read chunk tree: %d", ret);
		goto fail_tree_roots;
	}

	btrfs_close_extra_devices(fs_devices, 0);

	if (!fs_devices->latest_bdev) {
		btrfs_err(fs_info, "failed to read devices");
		goto fail_tree_roots;
	}

retry_root_backup:
	generation = btrfs_super_generation(disk_super);

	tree_root->node = read_tree_block(tree_root,
					  btrfs_super_root(disk_super),
					  generation);
	if (IS_ERR(tree_root->node) ||
	    !extent_buffer_uptodate(tree_root->node)) {
		btrfs_warn(fs_info, "failed to read tree root");
		if (!IS_ERR(tree_root->node))
			free_extent_buffer(tree_root->node);
		tree_root->node = NULL;
		goto recovery_tree_root;
	}

	btrfs_set_root_node(&tree_root->root_item, tree_root->node);
	tree_root->commit_root = btrfs_root_node(tree_root);
	btrfs_set_root_refs(&tree_root->root_item, 1);

	mutex_lock(&tree_root->objectid_mutex);
	ret = btrfs_find_highest_objectid(tree_root,
					&tree_root->highest_objectid);
	if (ret) {
		mutex_unlock(&tree_root->objectid_mutex);
		goto recovery_tree_root;
	}

	ASSERT(tree_root->highest_objectid <= BTRFS_LAST_FREE_OBJECTID);

	mutex_unlock(&tree_root->objectid_mutex);

	ret = btrfs_read_roots(fs_info, tree_root);
	if (ret)
		goto recovery_tree_root;

	fs_info->generation = generation;
	fs_info->last_trans_committed = generation;

	ret = btrfs_recover_balance(fs_info);
	if (ret) {
		btrfs_err(fs_info, "failed to recover balance: %d", ret);
		goto fail_block_groups;
	}

	ret = btrfs_init_dev_stats(fs_info);
	if (ret) {
		btrfs_err(fs_info, "failed to init dev_stats: %d", ret);
		goto fail_block_groups;
	}

	ret = btrfs_init_dev_replace(fs_info);
	if (ret) {
		btrfs_err(fs_info, "failed to init dev_replace: %d", ret);
		goto fail_block_groups;
	}

	btrfs_close_extra_devices(fs_devices, 1);

#ifdef MY_ABC_HERE
	ret = btrfs_debugfs_add_mounted(fs_info);
	if (ret) {
		pr_err("BTRFS: failed to init debugfs interface: %d\n", ret);
		goto fail_block_groups;
	}
#endif  

	ret = btrfs_sysfs_add_fsid(fs_devices, NULL);
	if (ret) {
		btrfs_err(fs_info, "failed to init sysfs fsid interface: %d",
				ret);
#ifdef MY_ABC_HERE
		goto fail_debugfs;
#else
		goto fail_block_groups;
#endif  
	}

	ret = btrfs_sysfs_add_device(fs_devices);
	if (ret) {
		btrfs_err(fs_info, "failed to init sysfs device interface: %d",
				ret);
		goto fail_fsdev_sysfs;
	}

	ret = btrfs_sysfs_add_mounted(fs_info);
	if (ret) {
		btrfs_err(fs_info, "failed to init sysfs interface: %d", ret);
		goto fail_fsdev_sysfs;
	}

	ret = btrfs_init_space_info(fs_info);
	if (ret) {
		btrfs_err(fs_info, "failed to initialize space info: %d", ret);
		goto fail_sysfs;
	}

#ifdef MY_ABC_HERE
	fs_info->can_fix_meta_key = CAN_FIX_META_KEY;
#endif  

#ifdef MY_ABC_HERE
       if (btrfs_test_opt(tree_root, NO_BLOCK_GROUP)) {
               ret = 0;
       } else {
               ret = btrfs_read_block_groups(fs_info->extent_root);
       }
#else
	ret = btrfs_read_block_groups(fs_info->extent_root);
#endif  

#ifdef MY_ABC_HERE
	btrfs_destroy_workqueue(fs_info->reada_path_workers);
	fs_info->reada_path_workers = NULL;
#endif  
#ifdef MY_ABC_HERE
	fs_info->can_fix_meta_key = STOP_FIX_META_KEY;
	btrfs_destroy_workqueue(fs_info->endio_meta_fix_workers);
	fs_info->endio_meta_fix_workers = NULL;
#endif  

	if (ret) {
		btrfs_err(fs_info, "failed to read block groups: %d", ret);
		goto fail_sysfs;
	}
	fs_info->num_tolerated_disk_barrier_failures =
		btrfs_calc_num_tolerated_disk_barrier_failures(fs_info);
	if (fs_info->fs_devices->missing_devices >
	     fs_info->num_tolerated_disk_barrier_failures &&
	    !(sb->s_flags & MS_RDONLY)) {
		btrfs_warn(fs_info,
"missing devices (%llu) exceeds the limit (%d), writeable mount is not allowed",
			fs_info->fs_devices->missing_devices,
			fs_info->num_tolerated_disk_barrier_failures);
		goto fail_sysfs;
	}

	fs_info->cleaner_kthread = kthread_run(cleaner_kthread, tree_root,
					       "btrfs-cleaner");
	if (IS_ERR(fs_info->cleaner_kthread))
		goto fail_sysfs;

	fs_info->transaction_kthread = kthread_run(transaction_kthread,
						   tree_root,
						   "btrfs-transaction");
	if (IS_ERR(fs_info->transaction_kthread))
		goto fail_cleaner;

	if (!btrfs_test_opt(tree_root, SSD) &&
	    !btrfs_test_opt(tree_root, NOSSD) &&
	    !fs_info->fs_devices->rotating) {
		btrfs_info(fs_info, "detected SSD devices, enabling SSD mode");
		btrfs_set_opt(fs_info->mount_opt, SSD);
	}

	btrfs_apply_pending_changes(fs_info);

#ifdef CONFIG_BTRFS_FS_CHECK_INTEGRITY
	if (btrfs_test_opt(tree_root, CHECK_INTEGRITY)) {
		ret = btrfsic_mount(tree_root, fs_devices,
				    btrfs_test_opt(tree_root,
					CHECK_INTEGRITY_INCLUDING_EXTENT_DATA) ?
				    1 : 0,
				    fs_info->check_integrity_print_mask);
		if (ret)
			btrfs_warn(fs_info,
				"failed to initialize integrity check module: %d",
				ret);
	}
#endif
	ret = btrfs_read_qgroup_config(fs_info);
	if (ret)
		goto fail_trans_kthread;

	if (btrfs_super_log_root(disk_super) != 0 &&
	    !btrfs_test_opt(tree_root, NOLOGREPLAY)) {
		ret = btrfs_replay_log(fs_info, fs_devices);
		if (ret) {
			err = ret;
			goto fail_qgroup;
		}
	}

	ret = btrfs_find_orphan_roots(tree_root);
	if (ret)
		goto fail_qgroup;

	if (!(sb->s_flags & MS_RDONLY)) {
		ret = btrfs_cleanup_fs_roots(fs_info);
		if (ret)
			goto fail_qgroup;

		mutex_lock(&fs_info->cleaner_mutex);
		ret = btrfs_recover_relocation(tree_root);
		mutex_unlock(&fs_info->cleaner_mutex);
		if (ret < 0) {
			btrfs_warn(fs_info, "failed to recover relocation: %d",
					ret);
			err = -EINVAL;
			goto fail_qgroup;
		}
	}

	location.objectid = BTRFS_FS_TREE_OBJECTID;
	location.type = BTRFS_ROOT_ITEM_KEY;
	location.offset = 0;

	fs_info->fs_root = btrfs_read_fs_root_no_name(fs_info, &location);
	if (IS_ERR(fs_info->fs_root)) {
		err = PTR_ERR(fs_info->fs_root);
		goto fail_qgroup;
	}
#ifdef MY_ABC_HERE
	fs_info->avoid_fs_root_null_pointer_dereference = 0;
#endif  

	if (sb->s_flags & MS_RDONLY)
		return 0;

#ifdef MY_ABC_HERE
	if (fs_info->block_group_cache_tree_broken || (!btrfs_test_opt(tree_root, BLOCK_GROUP_CACHE_TREE) && btrfs_fs_compat(fs_info, BLOCK_GROUP_CACHE_TREE))) {
		ret = btrfs_clean_block_group_cache_tree(fs_info);
		if (ret) {
			pr_err("BTRFS: failed to clean the block group cache tree %d\n", ret);
			close_ctree(tree_root);
			return ret;
		}
	}
	if (btrfs_test_opt(tree_root, BLOCK_GROUP_CACHE_TREE) && !btrfs_fs_compat(fs_info, BLOCK_GROUP_CACHE_TREE) && !fs_info->block_group_cache_tree_broken) {
		ret = btrfs_create_block_group_cache_tree(fs_info);
		if (ret) {
			pr_err("BTRFS: failed to create the block group cache tree %d\n", ret);
			close_ctree(tree_root);
			return ret;
		}
	}
#endif  

	if (btrfs_test_opt(tree_root, CLEAR_CACHE) &&
	    btrfs_fs_compat_ro(fs_info, FREE_SPACE_TREE)) {
		clear_free_space_tree = 1;
	} else if (btrfs_fs_compat_ro(fs_info, FREE_SPACE_TREE) &&
		   !btrfs_fs_compat_ro(fs_info, FREE_SPACE_TREE_VALID)) {
		btrfs_warn(fs_info, "free space tree is invalid");
		clear_free_space_tree = 1;
	}

	if (clear_free_space_tree) {
		btrfs_info(fs_info, "clearing free space tree");
		ret = btrfs_clear_free_space_tree(fs_info);
		if (ret) {
			btrfs_warn(fs_info,
				   "failed to clear free space tree: %d", ret);
			close_ctree(tree_root);
			return ret;
		}
	}

	if (btrfs_test_opt(tree_root, FREE_SPACE_TREE) &&
	    !btrfs_fs_compat_ro(fs_info, FREE_SPACE_TREE)) {
		pr_info("BTRFS: creating free space tree\n");
		ret = btrfs_create_free_space_tree(fs_info);
#ifdef MY_ABC_HERE
		if (fs_info->abort_free_space_tree) {
			btrfs_clear_opt(tree_root->fs_info->mount_opt, FREE_SPACE_TREE);
			pr_warn("BTRFS: abort to create free space tree \n");
		}
#endif  
		if (ret) {
			pr_warn("BTRFS: failed to create free space tree %d\n",
				ret);
			close_ctree(tree_root);
			return ret;
		}
	}

	down_read(&fs_info->cleanup_work_sem);
	if ((ret = btrfs_orphan_cleanup(fs_info->fs_root)) ||
	    (ret = btrfs_orphan_cleanup(fs_info->tree_root))) {
		up_read(&fs_info->cleanup_work_sem);
		close_ctree(tree_root);
		return ret;
	}
	up_read(&fs_info->cleanup_work_sem);

	ret = btrfs_resume_balance_async(fs_info);
	if (ret) {
		btrfs_warn(fs_info, "failed to resume balance: %d", ret);
		close_ctree(tree_root);
		return ret;
	}

	ret = btrfs_resume_dev_replace_async(fs_info);
	if (ret) {
		btrfs_warn(fs_info, "failed to resume device replace: %d", ret);
		close_ctree(tree_root);
		return ret;
	}

	btrfs_qgroup_rescan_resume(fs_info);

	if (!fs_info->uuid_root) {
		btrfs_info(fs_info, "creating UUID tree");
		ret = btrfs_create_uuid_tree(fs_info);
		if (ret) {
			btrfs_warn(fs_info,
				"failed to create the UUID tree: %d", ret);
			close_ctree(tree_root);
			return ret;
		}
	} else if (btrfs_test_opt(tree_root, RESCAN_UUID_TREE) ||
		   fs_info->generation !=
				btrfs_super_uuid_tree_generation(disk_super)) {
		btrfs_info(fs_info, "checking UUID tree");
		ret = btrfs_check_uuid_tree(fs_info);
		if (ret) {
			btrfs_warn(fs_info,
				"failed to check the UUID tree: %d", ret);
			close_ctree(tree_root);
			return ret;
		}
	} else {
		fs_info->update_uuid_tree_gen = 1;
	}

	fs_info->open = 1;

	return 0;

fail_qgroup:
	btrfs_free_qgroup_config(fs_info);
fail_trans_kthread:
	kthread_stop(fs_info->transaction_kthread);
	btrfs_cleanup_transaction(fs_info->tree_root);
	btrfs_free_fs_roots(fs_info);
fail_cleaner:
	kthread_stop(fs_info->cleaner_kthread);

	filemap_write_and_wait(fs_info->btree_inode->i_mapping);

fail_sysfs:
	btrfs_sysfs_remove_mounted(fs_info);

fail_fsdev_sysfs:
	btrfs_sysfs_remove_fsid(fs_info->fs_devices);

#ifdef MY_ABC_HERE
fail_debugfs:
	btrfs_debugfs_remove_mounted(fs_info);
#endif  

fail_block_groups:
	btrfs_put_block_group_cache(fs_info);
	btrfs_free_block_groups(fs_info);

fail_tree_roots:
	free_root_pointers(fs_info, 1);
	invalidate_inode_pages2(fs_info->btree_inode->i_mapping);

fail_sb_buffer:
	btrfs_stop_all_workers(fs_info);
fail_alloc:
fail_iput:
	btrfs_mapping_tree_free(&fs_info->mapping_tree);

	iput(fs_info->btree_inode);
fail_bio_counter:
	percpu_counter_destroy(&fs_info->bio_counter);
fail_delalloc_bytes:
	percpu_counter_destroy(&fs_info->delalloc_bytes);
fail_dirty_metadata_bytes:
	percpu_counter_destroy(&fs_info->dirty_metadata_bytes);
fail_bdi:
	bdi_destroy(&fs_info->bdi);
fail_srcu:
	cleanup_srcu_struct(&fs_info->subvol_srcu);
fail:
#ifdef MY_ABC_HERE
	perf_stats_monitor_destroy(fs_info);
#endif  
	btrfs_free_stripe_hash_table(fs_info);
	btrfs_close_devices(fs_info->fs_devices);

	return err;

recovery_tree_root:
	if (!btrfs_test_opt(tree_root, RECOVERY))
		goto fail_tree_roots;

	free_root_pointers(fs_info, 0);

	btrfs_set_super_log_root(disk_super, 0);

	btrfs_set_opt(fs_info->mount_opt, CLEAR_CACHE);

	ret = next_root_backup(fs_info, fs_info->super_copy,
			       &num_backups_tried, &backup_index);
	if (ret == -1)
		goto fail_block_groups;
	goto retry_root_backup;
}

static void btrfs_end_buffer_write_sync(struct buffer_head *bh, int uptodate)
{
	if (uptodate) {
		set_buffer_uptodate(bh);
	} else {
		struct btrfs_device *device = (struct btrfs_device *)
			bh->b_private;

		btrfs_warn_rl_in_rcu(device->dev_root->fs_info,
				"lost page write due to IO error on %s",
					  rcu_str_deref(device->name));
		 
		clear_buffer_uptodate(bh);
		btrfs_dev_stat_inc_and_print(device, BTRFS_DEV_STAT_WRITE_ERRS);
	}
	unlock_buffer(bh);
	put_bh(bh);
}

int btrfs_read_dev_one_super(struct block_device *bdev, int copy_num,
			struct buffer_head **bh_ret)
{
	struct buffer_head *bh;
	struct btrfs_super_block *super;
	u64 bytenr;

	bytenr = btrfs_sb_offset(copy_num);
	if (bytenr + BTRFS_SUPER_INFO_SIZE >= i_size_read(bdev->bd_inode))
		return -EINVAL;

	bh = __bread(bdev, bytenr / 4096, BTRFS_SUPER_INFO_SIZE);
	 
	if (!bh)
		return -EIO;

	super = (struct btrfs_super_block *)bh->b_data;
	if (btrfs_super_bytenr(super) != bytenr ||
		    btrfs_super_magic(super) != BTRFS_MAGIC) {
		brelse(bh);
		return -EINVAL;
	}

	*bh_ret = bh;
	return 0;
}

struct buffer_head *btrfs_read_dev_super(struct block_device *bdev)
{
	struct buffer_head *bh;
	struct buffer_head *latest = NULL;
	struct btrfs_super_block *super;
	int i;
	u64 transid = 0;
	int ret = -EINVAL;

	for (i = 0; i < 1; i++) {
		ret = btrfs_read_dev_one_super(bdev, i, &bh);
		if (ret)
			continue;

		super = (struct btrfs_super_block *)bh->b_data;

		if (!latest || btrfs_super_generation(super) > transid) {
			brelse(latest);
			latest = bh;
			transid = btrfs_super_generation(super);
		} else {
			brelse(bh);
		}
	}

	if (!latest)
		return ERR_PTR(ret);

	return latest;
}

static int write_dev_supers(struct btrfs_device *device,
			    struct btrfs_super_block *sb,
			    int do_barriers, int wait, int max_mirrors)
{
	struct buffer_head *bh;
	int i;
	int ret;
	int errors = 0;
	u32 crc;
	u64 bytenr;

	if (max_mirrors == 0)
		max_mirrors = BTRFS_SUPER_MIRROR_MAX;

	for (i = 0; i < max_mirrors; i++) {
		bytenr = btrfs_sb_offset(i);
		if (bytenr + BTRFS_SUPER_INFO_SIZE >=
		    device->commit_total_bytes)
			break;

		if (wait) {
			bh = __find_get_block(device->bdev, bytenr / 4096,
					      BTRFS_SUPER_INFO_SIZE);
			if (!bh) {
				errors++;
				continue;
			}
			wait_on_buffer(bh);
			if (!buffer_uptodate(bh))
				errors++;

			brelse(bh);

			brelse(bh);
			continue;
		} else {
			btrfs_set_super_bytenr(sb, bytenr);

			crc = ~(u32)0;
			crc = btrfs_csum_data((char *)sb +
					      BTRFS_CSUM_SIZE, crc,
					      BTRFS_SUPER_INFO_SIZE -
					      BTRFS_CSUM_SIZE);
			btrfs_csum_final(crc, sb->csum);

			bh = __getblk(device->bdev, bytenr / 4096,
				      BTRFS_SUPER_INFO_SIZE);
			if (!bh) {
				btrfs_err(device->dev_root->fs_info,
				    "couldn't get super buffer head for bytenr %llu",
				    bytenr);
				errors++;
				continue;
			}

			memcpy(bh->b_data, sb, BTRFS_SUPER_INFO_SIZE);

			get_bh(bh);

			set_buffer_uptodate(bh);
			lock_buffer(bh);
			bh->b_end_io = btrfs_end_buffer_write_sync;
			bh->b_private = device;
		}

		if (i == 0)
			ret = btrfsic_submit_bh(WRITE_FUA, bh);
		else
			ret = btrfsic_submit_bh(WRITE_SYNC, bh);
		if (ret)
			errors++;
#ifdef MY_ABC_HERE
		if (i == 0)
			__percpu_counter_add(&device->dev_root->fs_info->write_fua, 1, SZ_128M);
#endif  
	}
	return errors < i ? 0 : -1;
}

static void btrfs_end_empty_barrier(struct bio *bio)
{
	if (bio->bi_private)
		complete(bio->bi_private);
	bio_put(bio);
}

static int write_dev_flush(struct btrfs_device *device, int wait)
{
	struct bio *bio;
	int ret = 0;

	if (device->nobarriers)
		return 0;

	if (wait) {
		bio = device->flush_bio;
		if (!bio)
			return 0;

		wait_for_completion(&device->flush_wait);

		if (bio->bi_error) {
			ret = bio->bi_error;
			btrfs_dev_stat_inc_and_print(device,
				BTRFS_DEV_STAT_FLUSH_ERRS);
		}

		bio_put(bio);
		device->flush_bio = NULL;

		return ret;
	}

	device->flush_bio = NULL;
	bio = btrfs_io_bio_alloc(GFP_NOFS, 0);
	if (!bio)
		return -ENOMEM;

	bio->bi_end_io = btrfs_end_empty_barrier;
	bio->bi_bdev = device->bdev;
	init_completion(&device->flush_wait);
	bio->bi_private = &device->flush_wait;
	device->flush_bio = bio;

	bio_get(bio);
	btrfsic_submit_bio(WRITE_FLUSH, bio);
#ifdef MY_ABC_HERE
	__percpu_counter_add(&device->dev_root->fs_info->write_flush, 1, SZ_128M);
#endif  

	return 0;
}

static int barrier_all_devices(struct btrfs_fs_info *info)
{
	struct list_head *head;
	struct btrfs_device *dev;
	int errors_send = 0;
	int errors_wait = 0;
	int ret;

	head = &info->fs_devices->devices;
	list_for_each_entry_rcu(dev, head, dev_list) {
		if (dev->missing)
			continue;
		if (!dev->bdev) {
			errors_send++;
			continue;
		}
		if (!dev->in_fs_metadata || !dev->writeable)
			continue;

		ret = write_dev_flush(dev, 0);
		if (ret)
			errors_send++;
	}

	list_for_each_entry_rcu(dev, head, dev_list) {
		if (dev->missing)
			continue;
		if (!dev->bdev) {
			errors_wait++;
			continue;
		}
		if (!dev->in_fs_metadata || !dev->writeable)
			continue;

		ret = write_dev_flush(dev, 1);
		if (ret)
			errors_wait++;
	}
	if (errors_send > info->num_tolerated_disk_barrier_failures ||
	    errors_wait > info->num_tolerated_disk_barrier_failures)
		return -EIO;
	return 0;
}

int btrfs_get_num_tolerated_disk_barrier_failures(u64 flags)
{
	int raid_type;
	int min_tolerated = INT_MAX;

	if ((flags & BTRFS_BLOCK_GROUP_PROFILE_MASK) == 0 ||
	    (flags & BTRFS_AVAIL_ALLOC_BIT_SINGLE))
		min_tolerated = min(min_tolerated,
				    btrfs_raid_array[BTRFS_RAID_SINGLE].
				    tolerated_failures);

	for (raid_type = 0; raid_type < BTRFS_NR_RAID_TYPES; raid_type++) {
		if (raid_type == BTRFS_RAID_SINGLE)
			continue;
		if (!(flags & btrfs_raid_group[raid_type]))
			continue;
		min_tolerated = min(min_tolerated,
				    btrfs_raid_array[raid_type].
				    tolerated_failures);
	}

	if (min_tolerated == INT_MAX) {
		pr_warn("BTRFS: unknown raid flag: %llu\n", flags);
		min_tolerated = 0;
	}

	return min_tolerated;
}

int btrfs_calc_num_tolerated_disk_barrier_failures(
	struct btrfs_fs_info *fs_info)
{
	struct btrfs_ioctl_space_info space;
	struct btrfs_space_info *sinfo;
	u64 types[] = {BTRFS_BLOCK_GROUP_DATA,
		       BTRFS_BLOCK_GROUP_SYSTEM,
		       BTRFS_BLOCK_GROUP_METADATA,
		       BTRFS_BLOCK_GROUP_DATA | BTRFS_BLOCK_GROUP_METADATA};
	int i;
	int c;
	int num_tolerated_disk_barrier_failures =
		(int)fs_info->fs_devices->num_devices;

	for (i = 0; i < ARRAY_SIZE(types); i++) {
		struct btrfs_space_info *tmp;

		sinfo = NULL;
		rcu_read_lock();
		list_for_each_entry_rcu(tmp, &fs_info->space_info, list) {
			if (tmp->flags == types[i]) {
				sinfo = tmp;
				break;
			}
		}
		rcu_read_unlock();

		if (!sinfo)
			continue;

		down_read(&sinfo->groups_sem);
		for (c = 0; c < BTRFS_NR_RAID_TYPES; c++) {
			u64 flags;

			if (list_empty(&sinfo->block_groups[c]))
				continue;

			btrfs_get_block_group_info(&sinfo->block_groups[c],
						   &space);
			if (space.total_bytes == 0 || space.used_bytes == 0)
				continue;
			flags = space.flags;

			num_tolerated_disk_barrier_failures = min(
				num_tolerated_disk_barrier_failures,
				btrfs_get_num_tolerated_disk_barrier_failures(
					flags));
		}
		up_read(&sinfo->groups_sem);
	}

	return num_tolerated_disk_barrier_failures;
}

static int write_all_supers(struct btrfs_root *root, int max_mirrors)
{
	struct list_head *head;
	struct btrfs_device *dev;
	struct btrfs_super_block *sb;
	struct btrfs_dev_item *dev_item;
	int ret;
	int do_barriers;
	int max_errors;
	int total_errors = 0;
	u64 flags;

	do_barriers = !btrfs_test_opt(root, NOBARRIER);
	backup_super_roots(root->fs_info);

	sb = root->fs_info->super_for_commit;
	dev_item = &sb->dev_item;

	mutex_lock(&root->fs_info->fs_devices->device_list_mutex);
	head = &root->fs_info->fs_devices->devices;
	max_errors = btrfs_super_num_devices(root->fs_info->super_copy) - 1;

	if (do_barriers) {
		ret = barrier_all_devices(root->fs_info);
		if (ret) {
			mutex_unlock(
				&root->fs_info->fs_devices->device_list_mutex);
			btrfs_handle_fs_error(root->fs_info, ret,
				    "errors while submitting device barriers.");
			return ret;
		}
	}

	list_for_each_entry_rcu(dev, head, dev_list) {
		if (!dev->bdev) {
			total_errors++;
			continue;
		}
		if (!dev->in_fs_metadata || !dev->writeable)
			continue;

		btrfs_set_stack_device_generation(dev_item, 0);
		btrfs_set_stack_device_type(dev_item, dev->type);
		btrfs_set_stack_device_id(dev_item, dev->devid);
		btrfs_set_stack_device_total_bytes(dev_item,
						   dev->commit_total_bytes);
		btrfs_set_stack_device_bytes_used(dev_item,
						  dev->commit_bytes_used);
		btrfs_set_stack_device_io_align(dev_item, dev->io_align);
		btrfs_set_stack_device_io_width(dev_item, dev->io_width);
		btrfs_set_stack_device_sector_size(dev_item, dev->sector_size);
		memcpy(dev_item->uuid, dev->uuid, BTRFS_UUID_SIZE);
		memcpy(dev_item->fsid, dev->fs_devices->fsid, BTRFS_UUID_SIZE);

		flags = btrfs_super_flags(sb);
		btrfs_set_super_flags(sb, flags | BTRFS_HEADER_FLAG_WRITTEN);

		ret = btrfs_validate_write_super(root->fs_info, sb);
		if (ret < 0) {
			mutex_unlock(&root->fs_info->fs_devices->device_list_mutex);
			btrfs_handle_fs_error(root->fs_info, -EUCLEAN,
				"unexpected superblock corruption detected");
			return -EUCLEAN;
		}

		ret = write_dev_supers(dev, sb, do_barriers, 0, max_mirrors);
		if (ret)
			total_errors++;
	}
	if (total_errors > max_errors) {
		btrfs_err(root->fs_info, "%d errors while writing supers",
		       total_errors);
		mutex_unlock(&root->fs_info->fs_devices->device_list_mutex);

		btrfs_handle_fs_error(root->fs_info, -EIO,
			    "%d errors while writing supers", total_errors);
		return -EIO;
	}

	total_errors = 0;
	list_for_each_entry_rcu(dev, head, dev_list) {
		if (!dev->bdev)
			continue;
		if (!dev->in_fs_metadata || !dev->writeable)
			continue;

		ret = write_dev_supers(dev, sb, do_barriers, 1, max_mirrors);
		if (ret)
			total_errors++;
	}
	mutex_unlock(&root->fs_info->fs_devices->device_list_mutex);
	if (total_errors > max_errors) {
		btrfs_handle_fs_error(root->fs_info, -EIO,
			    "%d errors while writing supers", total_errors);
		return -EIO;
	}
	return 0;
}

int write_ctree_super(struct btrfs_trans_handle *trans,
		      struct btrfs_root *root, int max_mirrors)
{
	return write_all_supers(root, max_mirrors);
}

void btrfs_drop_and_free_fs_root(struct btrfs_fs_info *fs_info,
				  struct btrfs_root *root)
{
	spin_lock(&fs_info->fs_roots_radix_lock);
	radix_tree_delete(&fs_info->fs_roots_radix,
			  (unsigned long)root->root_key.objectid);
	spin_unlock(&fs_info->fs_roots_radix_lock);

	if (btrfs_root_refs(&root->root_item) == 0)
		synchronize_srcu(&fs_info->subvol_srcu);

	if (test_bit(BTRFS_FS_STATE_ERROR, &fs_info->fs_state))
		btrfs_free_log(NULL, root);

	if (root->free_ino_pinned)
		__btrfs_remove_free_space_cache(root->free_ino_pinned);
	if (root->free_ino_ctl)
		__btrfs_remove_free_space_cache(root->free_ino_ctl);
	free_fs_root(root);
}

static void free_fs_root(struct btrfs_root *root)
{
	iput(root->ino_cache_inode);
	WARN_ON(!RB_EMPTY_ROOT(&root->inode_tree));
	btrfs_free_block_rsv(root, root->orphan_block_rsv);
	root->orphan_block_rsv = NULL;
	if (root->anon_dev)
		free_anon_bdev(root->anon_dev);
	if (root->subv_writers)
		btrfs_free_subvolume_writers(root->subv_writers);
	free_extent_buffer(root->node);
	free_extent_buffer(root->commit_root);
	kfree(root->free_ino_ctl);
	kfree(root->free_ino_pinned);
	kfree(root->name);
	btrfs_put_fs_root(root);
}

void btrfs_free_fs_root(struct btrfs_root *root)
{
	free_fs_root(root);
}

int btrfs_cleanup_fs_roots(struct btrfs_fs_info *fs_info)
{
	u64 root_objectid = 0;
	struct btrfs_root *gang[8];
	int i = 0;
	int err = 0;
	unsigned int ret = 0;
	int index;

	while (1) {
		index = srcu_read_lock(&fs_info->subvol_srcu);
		ret = radix_tree_gang_lookup(&fs_info->fs_roots_radix,
					     (void **)gang, root_objectid,
					     ARRAY_SIZE(gang));
		if (!ret) {
			srcu_read_unlock(&fs_info->subvol_srcu, index);
			break;
		}
		root_objectid = gang[ret - 1]->root_key.objectid + 1;

		for (i = 0; i < ret; i++) {
			 
			if (btrfs_root_refs(&gang[i]->root_item) == 0) {
				gang[i] = NULL;
				continue;
			}
			 
			gang[i] = btrfs_grab_fs_root(gang[i]);
		}
		srcu_read_unlock(&fs_info->subvol_srcu, index);

		for (i = 0; i < ret; i++) {
			if (!gang[i])
				continue;
			root_objectid = gang[i]->root_key.objectid;
			err = btrfs_orphan_cleanup(gang[i]);
			if (err)
				break;
			btrfs_put_fs_root(gang[i]);
		}
		root_objectid++;
	}

	for (; i < ret; i++) {
		if (gang[i])
			btrfs_put_fs_root(gang[i]);
	}
	return err;
}

int btrfs_commit_super(struct btrfs_root *root)
{
	struct btrfs_trans_handle *trans;

	mutex_lock(&root->fs_info->cleaner_mutex);
	btrfs_run_delayed_iputs(root);
	mutex_unlock(&root->fs_info->cleaner_mutex);
	wake_up_process(root->fs_info->cleaner_kthread);

	down_write(&root->fs_info->cleanup_work_sem);
	up_write(&root->fs_info->cleanup_work_sem);

	trans = btrfs_join_transaction(root);
	if (IS_ERR(trans))
		return PTR_ERR(trans);
	return btrfs_commit_transaction(trans, root);
}

void close_ctree(struct btrfs_root *root)
{
	struct btrfs_fs_info *fs_info = root->fs_info;
	int ret;

	fs_info->closing = 1;
	smp_mb();

	btrfs_qgroup_wait_for_completion(fs_info, false);

	down(&fs_info->uuid_tree_rescan_sem);
	 
	up(&fs_info->uuid_tree_rescan_sem);

	btrfs_pause_balance(fs_info);

	btrfs_dev_replace_suspend_for_unmount(fs_info);

	btrfs_scrub_cancel(fs_info);

	wait_event(fs_info->transaction_wait,
		   (atomic_read(&fs_info->defrag_running) == 0));

	btrfs_cleanup_defrag_inodes(fs_info);

	cancel_work_sync(&fs_info->async_reclaim_work);
#ifdef MY_ABC_HERE
	cancel_work_sync(&fs_info->async_delayed_ref_work);
#endif  

	if (!(fs_info->sb->s_flags & MS_RDONLY)) {
		 
		btrfs_delete_unused_bgs(root->fs_info);

		ret = btrfs_commit_super(root);
		if (ret)
			btrfs_err(fs_info, "commit super ret %d", ret);
	}

	if (test_bit(BTRFS_FS_STATE_ERROR, &fs_info->fs_state))
		btrfs_error_commit_super(root);

	kthread_stop(fs_info->transaction_kthread);
	kthread_stop(fs_info->cleaner_kthread);

	fs_info->closing = 2;
	smp_mb();

	btrfs_free_qgroup_config(fs_info);

	if (percpu_counter_sum(&fs_info->delalloc_bytes)) {
		btrfs_info(fs_info, "at unmount delalloc count %lld",
		       percpu_counter_sum(&fs_info->delalloc_bytes));
	}

#ifdef MY_ABC_HERE
	btrfs_debugfs_remove_mounted(fs_info);
#endif  

	btrfs_sysfs_remove_mounted(fs_info);
	btrfs_sysfs_remove_fsid(fs_info->fs_devices);

	btrfs_free_fs_roots(fs_info);

	btrfs_put_block_group_cache(fs_info);

	btrfs_free_block_groups(fs_info);

	invalidate_inode_pages2(fs_info->btree_inode->i_mapping);
	btrfs_stop_all_workers(fs_info);

	fs_info->open = 0;
	free_root_pointers(fs_info, 1);

	iput(fs_info->btree_inode);

#ifdef CONFIG_BTRFS_FS_CHECK_INTEGRITY
	if (btrfs_test_opt(root, CHECK_INTEGRITY))
		btrfsic_unmount(root, fs_info->fs_devices);
#endif

	btrfs_close_devices(fs_info->fs_devices);
	btrfs_mapping_tree_free(&fs_info->mapping_tree);

	percpu_counter_destroy(&fs_info->dirty_metadata_bytes);
	percpu_counter_destroy(&fs_info->delalloc_bytes);
	percpu_counter_destroy(&fs_info->bio_counter);
	bdi_destroy(&fs_info->bdi);
	cleanup_srcu_struct(&fs_info->subvol_srcu);

	btrfs_free_stripe_hash_table(fs_info);

	__btrfs_free_block_rsv(root->orphan_block_rsv);
	root->orphan_block_rsv = NULL;

	lock_chunks(root);
	while (!list_empty(&fs_info->pinned_chunks)) {
		struct extent_map *em;

		em = list_first_entry(&fs_info->pinned_chunks,
				      struct extent_map, list);
		list_del_init(&em->list);
		free_extent_map(em);
	}
	unlock_chunks(root);
}

int btrfs_buffer_uptodate(struct extent_buffer *buf, u64 parent_transid,
			  int atomic)
{
	int ret;
	struct inode *btree_inode = buf->pages[0]->mapping->host;

	ret = extent_buffer_uptodate(buf);
	if (!ret)
		return ret;

	ret = verify_parent_transid(&BTRFS_I(btree_inode)->io_tree, buf,
				    parent_transid, atomic);
	if (ret == -EAGAIN)
		return ret;
	return !ret;
}

void btrfs_mark_buffer_dirty(struct extent_buffer *buf)
{
	struct btrfs_root *root;
	u64 transid = btrfs_header_generation(buf);
	int was_dirty;

#ifdef CONFIG_BTRFS_FS_RUN_SANITY_TESTS
	 
	if (unlikely(test_bit(EXTENT_BUFFER_DUMMY, &buf->bflags)))
		return;
#endif
	root = BTRFS_I(buf->pages[0]->mapping->host)->root;
	btrfs_assert_tree_locked(buf);
	if (transid != root->fs_info->generation)
		WARN(1, KERN_CRIT "btrfs transid mismatch buffer %llu, "
		       "found %llu running %llu\n",
			buf->start, transid, root->fs_info->generation);
	was_dirty = set_extent_buffer_dirty(buf);
	if (!was_dirty)
		__percpu_counter_add(&root->fs_info->dirty_metadata_bytes,
				     buf->len,
				     root->fs_info->dirty_metadata_batch);
#ifdef CONFIG_BTRFS_FS_CHECK_INTEGRITY
	if (btrfs_header_level(buf) == 0 && check_leaf(root, buf)) {
		btrfs_print_leaf(root, buf);
		ASSERT(0);
	}
#endif
}

#ifdef MY_ABC_HERE
#define FLUSH_DIRTY_BTREE_PAGES_BATCH 2048  

struct flush_meta_work {
	struct btrfs_fs_info *fs_info;
	struct btrfs_work work;
};

static void __btrfs_async_btree_balance_dirty(struct btrfs_work *work)
{
	struct flush_meta_work *flush_work;
	struct writeback_control wbc = {
		.sync_mode      = WB_SYNC_NONE,
		.nr_to_write    = FLUSH_DIRTY_BTREE_PAGES_BATCH,
		.range_cyclic   = 1,
	};

	flush_work = container_of(work, struct flush_meta_work, work);
	btree_write_cache_pages(flush_work->fs_info->btree_inode->i_mapping, &wbc);
	atomic_dec(&flush_work->fs_info->btree_flusher);
	kfree(flush_work);
}

static int btrfs_start_async_btree_balance_dirty(struct btrfs_fs_info *fs_info)
{
	struct flush_meta_work *flush_work;

	flush_work = kmalloc(sizeof(*flush_work), GFP_NOFS);
	if (!flush_work)
		return -ENOMEM;
	flush_work->fs_info = fs_info;

	btrfs_init_work(&flush_work->work, btrfs_flush_meta_helper,
			__btrfs_async_btree_balance_dirty, NULL, NULL);

	atomic_inc(&fs_info->btree_flusher);
	btrfs_queue_work(fs_info->flush_meta_workers, &flush_work->work);

	return 0;
}

void btrfs_async_btree_balance_dirty(struct btrfs_fs_info *fs_info)
{
	unsigned long background_thresh, dirty_thresh;

	if (__percpu_counter_compare(&fs_info->dirty_metadata_bytes,
				BTRFS_DIRTY_METADATA_THRESH,
				fs_info->dirty_metadata_batch) < 0)
		return;

	if (percpu_counter_sum_positive(&fs_info->dirty_metadata_bytes)
			- (atomic_read(&fs_info->btree_flusher) * FLUSH_DIRTY_BTREE_PAGES_BATCH * PAGE_CACHE_SIZE)
			< BTRFS_DIRTY_METADATA_THRESH)
		return;

	global_dirty_limits(&background_thresh, &dirty_thresh);
	if (global_page_state(NR_FILE_DIRTY) + global_page_state(NR_UNSTABLE_NFS)
			- (atomic_read(&fs_info->btree_flusher) * FLUSH_DIRTY_BTREE_PAGES_BATCH) <
			dirty_thresh + BTRFS_DIRTY_METADATA_THRESH / PAGE_CACHE_SIZE)
		return;

	btrfs_start_async_btree_balance_dirty(fs_info);
}
#endif  

static void __btrfs_btree_balance_dirty(struct btrfs_root *root,
					int flush_delayed)
{
	 
	int ret;

	if (current->flags & PF_MEMALLOC)
		return;

	if (flush_delayed)
		btrfs_balance_delayed_items(root);

	ret = __percpu_counter_compare(&root->fs_info->dirty_metadata_bytes,
				     BTRFS_DIRTY_METADATA_THRESH,
					 root->fs_info->dirty_metadata_batch);
	if (ret > 0) {
		balance_dirty_pages_ratelimited(
				   root->fs_info->btree_inode->i_mapping);
	}
}

void btrfs_btree_balance_dirty(struct btrfs_root *root)
{
	__btrfs_btree_balance_dirty(root, 1);
}

void btrfs_btree_balance_dirty_nodelay(struct btrfs_root *root)
{
	__btrfs_btree_balance_dirty(root, 0);
}

int btrfs_read_buffer(struct extent_buffer *buf, u64 parent_transid)
{
	struct btrfs_root *root = BTRFS_I(buf->pages[0]->mapping->host)->root;
	return btree_read_extent_buffer_pages(root, buf, 0, parent_transid);
}

static void btrfs_error_commit_super(struct btrfs_root *root)
{
	mutex_lock(&root->fs_info->cleaner_mutex);
	btrfs_run_delayed_iputs(root);
	mutex_unlock(&root->fs_info->cleaner_mutex);

	down_write(&root->fs_info->cleanup_work_sem);
	up_write(&root->fs_info->cleanup_work_sem);

	btrfs_cleanup_transaction(root);
}

static void btrfs_destroy_ordered_extents(struct btrfs_root *root)
{
	struct btrfs_ordered_extent *ordered;

	spin_lock(&root->ordered_extent_lock);
	 
	list_for_each_entry(ordered, &root->ordered_extents,
			    root_extent_list)
		set_bit(BTRFS_ORDERED_IOERR, &ordered->flags);
	spin_unlock(&root->ordered_extent_lock);
}

static void btrfs_destroy_all_ordered_extents(struct btrfs_fs_info *fs_info)
{
	struct btrfs_root *root;
	struct list_head splice;

	INIT_LIST_HEAD(&splice);

	spin_lock(&fs_info->ordered_root_lock);
	list_splice_init(&fs_info->ordered_roots, &splice);
	while (!list_empty(&splice)) {
		root = list_first_entry(&splice, struct btrfs_root,
					ordered_root);
		list_move_tail(&root->ordered_root,
			       &fs_info->ordered_roots);

		spin_unlock(&fs_info->ordered_root_lock);
		btrfs_destroy_ordered_extents(root);

		cond_resched();
		spin_lock(&fs_info->ordered_root_lock);
	}
	spin_unlock(&fs_info->ordered_root_lock);
}

static int btrfs_destroy_delayed_refs(struct btrfs_transaction *trans,
				      struct btrfs_root *root)
{
	struct rb_node *node;
	struct btrfs_delayed_ref_root *delayed_refs;
	struct btrfs_delayed_ref_node *ref;
	int ret = 0;

	delayed_refs = &trans->delayed_refs;

	spin_lock(&delayed_refs->lock);
	if (atomic_read(&delayed_refs->num_entries) == 0) {
		spin_unlock(&delayed_refs->lock);
		btrfs_info(root->fs_info, "delayed_refs has NO entry");
		return ret;
	}

	while ((node = rb_first(&delayed_refs->href_root)) != NULL) {
		struct btrfs_delayed_ref_head *head;
		struct btrfs_delayed_ref_node *tmp;
		bool pin_bytes = false;

		head = rb_entry(node, struct btrfs_delayed_ref_head,
				href_node);
		if (!mutex_trylock(&head->mutex)) {
			atomic_inc(&head->node.refs);
			spin_unlock(&delayed_refs->lock);

			mutex_lock(&head->mutex);
			mutex_unlock(&head->mutex);
			btrfs_put_delayed_ref(&head->node);
			spin_lock(&delayed_refs->lock);
			continue;
		}
		spin_lock(&head->lock);
		list_for_each_entry_safe_reverse(ref, tmp, &head->ref_list,
						 list) {
			ref->in_tree = 0;
			list_del(&ref->list);
			if (!list_empty(&ref->add_list))
				list_del(&ref->add_list);
			atomic_dec(&delayed_refs->num_entries);
			btrfs_put_delayed_ref(ref);
		}
		if (head->must_insert_reserved)
			pin_bytes = true;
		btrfs_free_delayed_extent_op(head->extent_op);
		delayed_refs->num_heads--;
		if (head->processing == 0)
			delayed_refs->num_heads_ready--;
		atomic_dec(&delayed_refs->num_entries);
		head->node.in_tree = 0;
		rb_erase(&head->href_node, &delayed_refs->href_root);
		spin_unlock(&head->lock);
		spin_unlock(&delayed_refs->lock);
		mutex_unlock(&head->mutex);

		if (pin_bytes)
			btrfs_pin_extent(root, head->node.bytenr,
					 head->node.num_bytes, 1);
		btrfs_put_delayed_ref(&head->node);
		cond_resched();
		spin_lock(&delayed_refs->lock);
	}

	spin_unlock(&delayed_refs->lock);

	return ret;
}

static void btrfs_destroy_delalloc_inodes(struct btrfs_root *root)
{
	struct btrfs_inode *btrfs_inode;
	struct list_head splice;

	INIT_LIST_HEAD(&splice);

	spin_lock(&root->delalloc_lock);
	list_splice_init(&root->delalloc_inodes, &splice);

	while (!list_empty(&splice)) {
		btrfs_inode = list_first_entry(&splice, struct btrfs_inode,
					       delalloc_inodes);

		list_del_init(&btrfs_inode->delalloc_inodes);
		clear_bit(BTRFS_INODE_IN_DELALLOC_LIST,
			  &btrfs_inode->runtime_flags);
		spin_unlock(&root->delalloc_lock);

		btrfs_invalidate_inodes(btrfs_inode->root);

		spin_lock(&root->delalloc_lock);
	}

	spin_unlock(&root->delalloc_lock);
}

static void btrfs_destroy_all_delalloc_inodes(struct btrfs_fs_info *fs_info)
{
	struct btrfs_root *root;
	struct list_head splice;

	INIT_LIST_HEAD(&splice);

	spin_lock(&fs_info->delalloc_root_lock);
	list_splice_init(&fs_info->delalloc_roots, &splice);
	while (!list_empty(&splice)) {
		root = list_first_entry(&splice, struct btrfs_root,
					 delalloc_root);
		list_del_init(&root->delalloc_root);
		root = btrfs_grab_fs_root(root);
		BUG_ON(!root);
		spin_unlock(&fs_info->delalloc_root_lock);

		btrfs_destroy_delalloc_inodes(root);
		btrfs_put_fs_root(root);

		spin_lock(&fs_info->delalloc_root_lock);
	}
	spin_unlock(&fs_info->delalloc_root_lock);
}

static int btrfs_destroy_marked_extents(struct btrfs_root *root,
					struct extent_io_tree *dirty_pages,
					int mark)
{
	int ret;
	struct extent_buffer *eb;
	u64 start = 0;
	u64 end;

	while (1) {
		ret = find_first_extent_bit(dirty_pages, start, &start, &end,
					    mark, NULL);
		if (ret)
			break;

		clear_extent_bits(dirty_pages, start, end, mark);
		while (start <= end) {
#ifdef MY_ABC_HERE
			eb = btrfs_find_tree_block(root, start);
#else
			eb = btrfs_find_tree_block(root->fs_info, start);
#endif  
			start += root->nodesize;
			if (!eb)
				continue;
			wait_on_extent_buffer_writeback(eb);

			if (test_and_clear_bit(EXTENT_BUFFER_DIRTY,
					       &eb->bflags))
				clear_extent_buffer_dirty(eb);
			free_extent_buffer_stale(eb);
		}
	}

	return ret;
}

static int btrfs_destroy_pinned_extent(struct btrfs_root *root,
				       struct extent_io_tree *pinned_extents)
{
	struct extent_io_tree *unpin;
	u64 start;
	u64 end;
	int ret;
	bool loop = true;

	unpin = pinned_extents;
again:
	while (1) {
		ret = find_first_extent_bit(unpin, 0, &start, &end,
					    EXTENT_DIRTY, NULL);
		if (ret)
			break;

		clear_extent_dirty(unpin, start, end);
		btrfs_error_unpin_extent_range(root, start, end);
		cond_resched();
	}

	if (loop) {
		if (unpin == &root->fs_info->freed_extents[0])
			unpin = &root->fs_info->freed_extents[1];
		else
			unpin = &root->fs_info->freed_extents[0];
		loop = false;
		goto again;
	}

	return 0;
}

void btrfs_cleanup_one_transaction(struct btrfs_transaction *cur_trans,
				   struct btrfs_root *root)
{
	btrfs_destroy_delayed_refs(cur_trans, root);

	cur_trans->state = TRANS_STATE_COMMIT_START;
	wake_up(&root->fs_info->transaction_blocked_wait);

	cur_trans->state = TRANS_STATE_UNBLOCKED;
	wake_up(&root->fs_info->transaction_wait);

	btrfs_destroy_delayed_inodes(root);
	btrfs_assert_delayed_root_empty(root);

	btrfs_destroy_marked_extents(root, &cur_trans->dirty_pages,
				     EXTENT_DIRTY);
	btrfs_destroy_pinned_extent(root,
				    root->fs_info->pinned_extents);

	cur_trans->state =TRANS_STATE_COMPLETED;
	wake_up(&cur_trans->commit_wait);

}

static int btrfs_cleanup_transaction(struct btrfs_root *root)
{
	struct btrfs_transaction *t;

	mutex_lock(&root->fs_info->transaction_kthread_mutex);

	spin_lock(&root->fs_info->trans_lock);
	while (!list_empty(&root->fs_info->trans_list)) {
		t = list_first_entry(&root->fs_info->trans_list,
				     struct btrfs_transaction, list);
		if (t->state >= TRANS_STATE_COMMIT_START) {
			atomic_inc(&t->use_count);
			spin_unlock(&root->fs_info->trans_lock);
			btrfs_wait_for_commit(root, t->transid);
			btrfs_put_transaction(t);
			spin_lock(&root->fs_info->trans_lock);
			continue;
		}
		if (t == root->fs_info->running_transaction) {
			t->state = TRANS_STATE_COMMIT_DOING;
			spin_unlock(&root->fs_info->trans_lock);
			 
			wait_event(t->writer_wait,
				   atomic_read(&t->num_writers) == 0);
		} else {
			spin_unlock(&root->fs_info->trans_lock);
		}
		btrfs_cleanup_one_transaction(t, root);

		spin_lock(&root->fs_info->trans_lock);
		if (t == root->fs_info->running_transaction)
			root->fs_info->running_transaction = NULL;
		list_del_init(&t->list);
		spin_unlock(&root->fs_info->trans_lock);

		btrfs_put_transaction(t);
		trace_btrfs_transaction_commit(root);
		spin_lock(&root->fs_info->trans_lock);
	}
	spin_unlock(&root->fs_info->trans_lock);
	btrfs_destroy_all_ordered_extents(root->fs_info);
	btrfs_destroy_delayed_inodes(root);
	btrfs_assert_delayed_root_empty(root);
	btrfs_destroy_pinned_extent(root, root->fs_info->pinned_extents);
	btrfs_destroy_all_delalloc_inodes(root->fs_info);
	mutex_unlock(&root->fs_info->transaction_kthread_mutex);

	return 0;
}

static const struct extent_io_ops btree_extent_io_ops = {
	.readpage_end_io_hook = btree_readpage_end_io_hook,
	.readpage_io_failed_hook = btree_io_failed_hook,
	.submit_bio_hook = btree_submit_bio_hook,
	 
	.merge_bio_hook = btrfs_merge_bio_hook,
};
