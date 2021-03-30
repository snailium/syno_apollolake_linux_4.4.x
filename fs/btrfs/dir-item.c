#ifndef MY_ABC_HERE
#define MY_ABC_HERE
#endif
 
#include "ctree.h"
#include "disk-io.h"
#include "hash.h"
#include "transaction.h"

static struct btrfs_dir_item *insert_with_overflow(struct btrfs_trans_handle
						   *trans,
						   struct btrfs_root *root,
						   struct btrfs_path *path,
						   struct btrfs_key *cpu_key,
						   u32 data_size,
						   const char *name,
						   int name_len)
{
	int ret;
	char *ptr;
	struct btrfs_item *item;
	struct extent_buffer *leaf;

	ret = btrfs_insert_empty_item(trans, root, path, cpu_key, data_size);
	if (ret == -EEXIST) {
		struct btrfs_dir_item *di;
		di = btrfs_match_dir_item_name(root, path, name, name_len);
		if (di)
			return ERR_PTR(-EEXIST);
		btrfs_extend_item(root, path, data_size);
	} else if (ret < 0)
		return ERR_PTR(ret);
	WARN_ON(ret > 0);
	leaf = path->nodes[0];
	item = btrfs_item_nr(path->slots[0]);
	ptr = btrfs_item_ptr(leaf, path->slots[0], char);
	BUG_ON(data_size > btrfs_item_size(leaf, item));
	ptr += btrfs_item_size(leaf, item) - data_size;
	return (struct btrfs_dir_item *)ptr;
}

int btrfs_insert_xattr_item(struct btrfs_trans_handle *trans,
			    struct btrfs_root *root,
			    struct btrfs_path *path, u64 objectid,
			    const char *name, u16 name_len,
			    const void *data, u16 data_len)
{
	int ret = 0;
	struct btrfs_dir_item *dir_item;
	unsigned long name_ptr, data_ptr;
	struct btrfs_key key, location;
	struct btrfs_disk_key disk_key;
	struct extent_buffer *leaf;
	u32 data_size;

	BUG_ON(name_len + data_len > BTRFS_MAX_XATTR_SIZE(root));

	key.objectid = objectid;
	key.type = BTRFS_XATTR_ITEM_KEY;
	key.offset = btrfs_name_hash(name, name_len);

	data_size = sizeof(*dir_item) + name_len + data_len;
	dir_item = insert_with_overflow(trans, root, path, &key, data_size,
					name, name_len);
	if (IS_ERR(dir_item))
		return PTR_ERR(dir_item);
	memset(&location, 0, sizeof(location));

	leaf = path->nodes[0];
	btrfs_cpu_key_to_disk(&disk_key, &location);
	btrfs_set_dir_item_key(leaf, dir_item, &disk_key);
	btrfs_set_dir_type(leaf, dir_item, BTRFS_FT_XATTR);
	btrfs_set_dir_name_len(leaf, dir_item, name_len);
	btrfs_set_dir_transid(leaf, dir_item, trans->transid);
	btrfs_set_dir_data_len(leaf, dir_item, data_len);
	name_ptr = (unsigned long)(dir_item + 1);
	data_ptr = (unsigned long)((char *)name_ptr + name_len);

	write_extent_buffer(leaf, name, name_ptr, name_len);
	write_extent_buffer(leaf, data, data_ptr, data_len);
	btrfs_mark_buffer_dirty(path->nodes[0]);

	return ret;
}

int btrfs_insert_dir_item(struct btrfs_trans_handle *trans, struct btrfs_root
			  *root, const char *name, int name_len,
			  struct inode *dir, struct btrfs_key *location,
			  u8 type, u64 index)
{
	int ret = 0;
	int ret2 = 0;
	struct btrfs_path *path;
	struct btrfs_dir_item *dir_item;
	struct extent_buffer *leaf;
	unsigned long name_ptr;
	struct btrfs_key key;
	struct btrfs_disk_key disk_key;
	u32 data_size;

	key.objectid = btrfs_ino(dir);
	key.type = BTRFS_DIR_ITEM_KEY;
	key.offset = btrfs_name_hash(name, name_len);

	path = btrfs_alloc_path();
	if (!path)
		return -ENOMEM;
	path->leave_spinning = 1;

	btrfs_cpu_key_to_disk(&disk_key, location);

	data_size = sizeof(*dir_item) + name_len;
	dir_item = insert_with_overflow(trans, root, path, &key, data_size,
					name, name_len);
	if (IS_ERR(dir_item)) {
		ret = PTR_ERR(dir_item);
		if (ret == -EEXIST)
			goto second_insert;
		goto out_free;
	}

	leaf = path->nodes[0];
	btrfs_set_dir_item_key(leaf, dir_item, &disk_key);
	btrfs_set_dir_type(leaf, dir_item, type);
	btrfs_set_dir_data_len(leaf, dir_item, 0);
	btrfs_set_dir_name_len(leaf, dir_item, name_len);
	btrfs_set_dir_transid(leaf, dir_item, trans->transid);
	name_ptr = (unsigned long)(dir_item + 1);

	write_extent_buffer(leaf, name, name_ptr, name_len);
	btrfs_mark_buffer_dirty(leaf);

second_insert:
	 
	if (root == root->fs_info->tree_root) {
		ret = 0;
		goto out_free;
	}
	btrfs_release_path(path);

	ret2 = btrfs_insert_delayed_dir_index(trans, root, name, name_len, dir,
					      &disk_key, type, index);
out_free:
	btrfs_free_path(path);
	if (ret)
		return ret;
	if (ret2)
		return ret2;
	return 0;
}

struct btrfs_dir_item *btrfs_lookup_dir_item(struct btrfs_trans_handle *trans,
					     struct btrfs_root *root,
					     struct btrfs_path *path, u64 dir,
					     const char *name, int name_len,
					     int mod)
{
	int ret;
	struct btrfs_key key;
	int ins_len = mod < 0 ? -1 : 0;
	int cow = mod != 0;

	key.objectid = dir;
	key.type = BTRFS_DIR_ITEM_KEY;

	key.offset = btrfs_name_hash(name, name_len);

	ret = btrfs_search_slot(trans, root, &key, path, ins_len, cow);
	if (ret < 0)
		return ERR_PTR(ret);
	if (ret > 0)
		return NULL;

	return btrfs_match_dir_item_name(root, path, name, name_len);
}

int btrfs_check_dir_item_collision(struct btrfs_root *root, u64 dir,
				   const char *name, int name_len)
{
	int ret;
	struct btrfs_key key;
	struct btrfs_dir_item *di;
	int data_size;
	struct extent_buffer *leaf;
	int slot;
	struct btrfs_path *path;

	path = btrfs_alloc_path();
	if (!path)
		return -ENOMEM;

	key.objectid = dir;
	key.type = BTRFS_DIR_ITEM_KEY;
	key.offset = btrfs_name_hash(name, name_len);

	ret = btrfs_search_slot(NULL, root, &key, path, 0, 0);

	if (ret < 0)
		goto out;

	if (ret > 0) {
		ret = 0;
		goto out;
	}

	di = btrfs_match_dir_item_name(root, path, name, name_len);
	if (di) {
		 
		ret = -EEXIST;
		goto out;
	}

#ifdef MY_ABC_HERE
	 
	data_size = sizeof(*di) + name_len + sizeof(struct btrfs_item);
#else
	data_size = sizeof(*di) + name_len;
#endif  
	leaf = path->nodes[0];
	slot = path->slots[0];
	if (data_size + btrfs_item_size_nr(leaf, slot) +
	    sizeof(struct btrfs_item) > BTRFS_LEAF_DATA_SIZE(root)) {
		ret = -EOVERFLOW;
	} else {
		 
		ret = 0;
	}
out:
	btrfs_free_path(path);
	return ret;
}

struct btrfs_dir_item *
btrfs_lookup_dir_index_item(struct btrfs_trans_handle *trans,
			    struct btrfs_root *root,
			    struct btrfs_path *path, u64 dir,
			    u64 objectid, const char *name, int name_len,
			    int mod)
{
	int ret;
	struct btrfs_key key;
	int ins_len = mod < 0 ? -1 : 0;
	int cow = mod != 0;

	key.objectid = dir;
	key.type = BTRFS_DIR_INDEX_KEY;
	key.offset = objectid;

	ret = btrfs_search_slot(trans, root, &key, path, ins_len, cow);
	if (ret < 0)
		return ERR_PTR(ret);
	if (ret > 0)
		return ERR_PTR(-ENOENT);
	return btrfs_match_dir_item_name(root, path, name, name_len);
}

struct btrfs_dir_item *
btrfs_search_dir_index_item(struct btrfs_root *root,
			    struct btrfs_path *path, u64 dirid,
			    const char *name, int name_len)
{
	struct extent_buffer *leaf;
	struct btrfs_dir_item *di;
	struct btrfs_key key;
	u32 nritems;
	int ret;

	key.objectid = dirid;
	key.type = BTRFS_DIR_INDEX_KEY;
	key.offset = 0;

	ret = btrfs_search_slot(NULL, root, &key, path, 0, 0);
	if (ret < 0)
		return ERR_PTR(ret);

	leaf = path->nodes[0];
	nritems = btrfs_header_nritems(leaf);

	while (1) {
		if (path->slots[0] >= nritems) {
			ret = btrfs_next_leaf(root, path);
			if (ret < 0)
				return ERR_PTR(ret);
			if (ret > 0)
				break;
			leaf = path->nodes[0];
			nritems = btrfs_header_nritems(leaf);
			continue;
		}

		btrfs_item_key_to_cpu(leaf, &key, path->slots[0]);
		if (key.objectid != dirid || key.type != BTRFS_DIR_INDEX_KEY)
			break;

		di = btrfs_match_dir_item_name(root, path, name, name_len);
		if (di)
			return di;

		path->slots[0]++;
	}
	return NULL;
}

struct btrfs_dir_item *btrfs_lookup_xattr(struct btrfs_trans_handle *trans,
					  struct btrfs_root *root,
					  struct btrfs_path *path, u64 dir,
					  const char *name, u16 name_len,
					  int mod)
{
	int ret;
	struct btrfs_key key;
	int ins_len = mod < 0 ? -1 : 0;
	int cow = mod != 0;

	key.objectid = dir;
	key.type = BTRFS_XATTR_ITEM_KEY;
	key.offset = btrfs_name_hash(name, name_len);
	ret = btrfs_search_slot(trans, root, &key, path, ins_len, cow);
	if (ret < 0)
		return ERR_PTR(ret);
	if (ret > 0)
		return NULL;

	return btrfs_match_dir_item_name(root, path, name, name_len);
}

struct btrfs_dir_item *btrfs_match_dir_item_name(struct btrfs_root *root,
						 struct btrfs_path *path,
						 const char *name, int name_len)
{
	struct btrfs_dir_item *dir_item;
	unsigned long name_ptr;
	u32 total_len;
	u32 cur = 0;
	u32 this_len;
	struct extent_buffer *leaf;

	leaf = path->nodes[0];
	dir_item = btrfs_item_ptr(leaf, path->slots[0], struct btrfs_dir_item);
	if (verify_dir_item(root, leaf, dir_item))
		return NULL;

	total_len = btrfs_item_size_nr(leaf, path->slots[0]);
	while (cur < total_len) {
		this_len = sizeof(*dir_item) +
			btrfs_dir_name_len(leaf, dir_item) +
			btrfs_dir_data_len(leaf, dir_item);
		name_ptr = (unsigned long)(dir_item + 1);

		if (btrfs_dir_name_len(leaf, dir_item) == name_len &&
		    memcmp_extent_buffer(leaf, name, name_ptr, name_len) == 0)
			return dir_item;

		cur += this_len;
		dir_item = (struct btrfs_dir_item *)((char *)dir_item +
						     this_len);
	}
	return NULL;
}

int btrfs_delete_one_dir_name(struct btrfs_trans_handle *trans,
			      struct btrfs_root *root,
			      struct btrfs_path *path,
			      struct btrfs_dir_item *di)
{

	struct extent_buffer *leaf;
	u32 sub_item_len;
	u32 item_len;
	int ret = 0;

	leaf = path->nodes[0];
	sub_item_len = sizeof(*di) + btrfs_dir_name_len(leaf, di) +
		btrfs_dir_data_len(leaf, di);
	item_len = btrfs_item_size_nr(leaf, path->slots[0]);
	if (sub_item_len == item_len) {
		ret = btrfs_del_item(trans, root, path);
	} else {
		 
		unsigned long ptr = (unsigned long)di;
		unsigned long start;

		start = btrfs_item_ptr_offset(leaf, path->slots[0]);
		memmove_extent_buffer(leaf, ptr, ptr + sub_item_len,
			item_len - (ptr + sub_item_len - start));
		btrfs_truncate_item(root, path, item_len - sub_item_len, 1);
	}
	return ret;
}

int verify_dir_item(struct btrfs_root *root,
		    struct extent_buffer *leaf,
		    struct btrfs_dir_item *dir_item)
{
	u16 namelen = BTRFS_NAME_LEN;
	u8 type = btrfs_dir_type(leaf, dir_item);

	if (type >= BTRFS_FT_MAX) {
		btrfs_crit(root->fs_info, "invalid dir item type: %d",
		       (int)type);
		return 1;
	}

	if (type == BTRFS_FT_XATTR)
		namelen = XATTR_NAME_MAX;

	if (btrfs_dir_name_len(leaf, dir_item) > namelen) {
		btrfs_crit(root->fs_info, "invalid dir item name len: %u",
		       (unsigned)btrfs_dir_data_len(leaf, dir_item));
		return 1;
	}

	if ((btrfs_dir_data_len(leaf, dir_item) +
	     btrfs_dir_name_len(leaf, dir_item)) > BTRFS_MAX_XATTR_SIZE(root)) {
		btrfs_crit(root->fs_info, "invalid dir item name + data len: %u + %u",
		       (unsigned)btrfs_dir_name_len(leaf, dir_item),
		       (unsigned)btrfs_dir_data_len(leaf, dir_item));
		return 1;
	}

	return 0;
}
