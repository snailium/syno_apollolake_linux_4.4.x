#ifndef MY_ABC_HERE
#define MY_ABC_HERE
#endif
 
#include <linux/dcache.h>
#include <linux/file.h>
#include <linux/module.h>
#include <linux/namei.h>
#include <linux/skbuff.h>
#include <linux/crypto.h>
#include <linux/mount.h>
#include <linux/pagemap.h>
#include <linux/key.h>
#include <linux/parser.h>
#include <linux/fs_stack.h>
#include <linux/slab.h>
#include <linux/magic.h>
#include "ecryptfs_kernel.h"

int ecryptfs_verbosity = 0;

module_param(ecryptfs_verbosity, int, 0);
MODULE_PARM_DESC(ecryptfs_verbosity,
		 "Initial verbosity level (0 or 1; defaults to "
		 "0, which is Quiet)");

unsigned int ecryptfs_message_buf_len = ECRYPTFS_DEFAULT_MSG_CTX_ELEMS;

module_param(ecryptfs_message_buf_len, uint, 0);
MODULE_PARM_DESC(ecryptfs_message_buf_len,
		 "Number of message buffer elements");

signed long ecryptfs_message_wait_timeout = ECRYPTFS_MAX_MSG_CTX_TTL / HZ;

module_param(ecryptfs_message_wait_timeout, long, 0);
MODULE_PARM_DESC(ecryptfs_message_wait_timeout,
		 "Maximum number of seconds that an operation will "
		 "sleep while waiting for a message response from "
		 "userspace");

unsigned int ecryptfs_number_of_users = ECRYPTFS_DEFAULT_NUM_USERS;

module_param(ecryptfs_number_of_users, uint, 0);
MODULE_PARM_DESC(ecryptfs_number_of_users, "An estimate of the number of "
		 "concurrent users of eCryptfs");

void __ecryptfs_printk(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	if (fmt[1] == '7') {  
		if (ecryptfs_verbosity >= 1)
			vprintk(fmt, args);
	} else
		vprintk(fmt, args);
	va_end(args);
}

static int ecryptfs_init_lower_file(struct dentry *dentry,
				    struct file **lower_file)
{
	const struct cred *cred = current_cred();
	struct path *path = ecryptfs_dentry_to_lower_path(dentry);
	int rc;

	rc = ecryptfs_privileged_open(lower_file, path->dentry, path->mnt,
				      cred);
	if (rc) {
		printk(KERN_ERR "Error opening lower file "
		       "for lower_dentry [0x%p] and lower_mnt [0x%p]; "
		       "rc = [%d]\n", path->dentry, path->mnt, rc);
		(*lower_file) = NULL;
	}
	return rc;
}

int ecryptfs_get_lower_file(struct dentry *dentry, struct inode *inode)
{
	struct ecryptfs_inode_info *inode_info;
	int count, rc = 0;

	inode_info = ecryptfs_inode_to_private(inode);
	mutex_lock(&inode_info->lower_file_mutex);
	count = atomic_inc_return(&inode_info->lower_file_count);
	if (WARN_ON_ONCE(count < 1))
		rc = -EINVAL;
	else if (count == 1) {
		rc = ecryptfs_init_lower_file(dentry,
					      &inode_info->lower_file);
		if (rc)
			atomic_set(&inode_info->lower_file_count, 0);
	}
	mutex_unlock(&inode_info->lower_file_mutex);
	return rc;
}

void ecryptfs_put_lower_file(struct inode *inode)
{
	struct ecryptfs_inode_info *inode_info;

	inode_info = ecryptfs_inode_to_private(inode);
	if (atomic_dec_and_mutex_lock(&inode_info->lower_file_count,
				      &inode_info->lower_file_mutex)) {
		filemap_write_and_wait(inode->i_mapping);
		fput(inode_info->lower_file);
		inode_info->lower_file = NULL;
		mutex_unlock(&inode_info->lower_file_mutex);
	}
}

enum { ecryptfs_opt_sig, ecryptfs_opt_ecryptfs_sig,
       ecryptfs_opt_cipher, ecryptfs_opt_ecryptfs_cipher,
       ecryptfs_opt_ecryptfs_key_bytes,
       ecryptfs_opt_passthrough, ecryptfs_opt_xattr_metadata,
       ecryptfs_opt_encrypted_view, ecryptfs_opt_fnek_sig,
       ecryptfs_opt_fn_cipher, ecryptfs_opt_fn_cipher_key_bytes,
       ecryptfs_opt_unlink_sigs, ecryptfs_opt_mount_auth_tok_only,
       ecryptfs_opt_check_dev_ruid,
#ifdef MY_ABC_HERE
       ecryptfs_opt_no_fast_lookup,
#endif  
       ecryptfs_opt_err };

static const match_table_t tokens = {
	{ecryptfs_opt_sig, "sig=%s"},
	{ecryptfs_opt_ecryptfs_sig, "ecryptfs_sig=%s"},
	{ecryptfs_opt_cipher, "cipher=%s"},
	{ecryptfs_opt_ecryptfs_cipher, "ecryptfs_cipher=%s"},
	{ecryptfs_opt_ecryptfs_key_bytes, "ecryptfs_key_bytes=%u"},
	{ecryptfs_opt_passthrough, "ecryptfs_passthrough"},
	{ecryptfs_opt_xattr_metadata, "ecryptfs_xattr_metadata"},
	{ecryptfs_opt_encrypted_view, "ecryptfs_encrypted_view"},
	{ecryptfs_opt_fnek_sig, "ecryptfs_fnek_sig=%s"},
	{ecryptfs_opt_fn_cipher, "ecryptfs_fn_cipher=%s"},
	{ecryptfs_opt_fn_cipher_key_bytes, "ecryptfs_fn_key_bytes=%u"},
	{ecryptfs_opt_unlink_sigs, "ecryptfs_unlink_sigs"},
	{ecryptfs_opt_mount_auth_tok_only, "ecryptfs_mount_auth_tok_only"},
	{ecryptfs_opt_check_dev_ruid, "ecryptfs_check_dev_ruid"},
#ifdef MY_ABC_HERE
	{ecryptfs_opt_no_fast_lookup, "no_fast_lookup"},
#endif  
	{ecryptfs_opt_err, NULL}
};

static int ecryptfs_init_global_auth_toks(
	struct ecryptfs_mount_crypt_stat *mount_crypt_stat)
{
	struct ecryptfs_global_auth_tok *global_auth_tok;
	struct ecryptfs_auth_tok *auth_tok;
	int rc = 0;

	list_for_each_entry(global_auth_tok,
			    &mount_crypt_stat->global_auth_tok_list,
			    mount_crypt_stat_list) {
		rc = ecryptfs_keyring_auth_tok_for_sig(
			&global_auth_tok->global_auth_tok_key, &auth_tok,
			global_auth_tok->sig);
		if (rc) {
			printk(KERN_ERR "Could not find valid key in user "
			       "session keyring for sig specified in mount "
			       "option: [%s]\n", global_auth_tok->sig);
			global_auth_tok->flags |= ECRYPTFS_AUTH_TOK_INVALID;
			goto out;
		} else {
			global_auth_tok->flags &= ~ECRYPTFS_AUTH_TOK_INVALID;
			up_write(&(global_auth_tok->global_auth_tok_key)->sem);
		}
	}
out:
	return rc;
}

static void ecryptfs_init_mount_crypt_stat(
	struct ecryptfs_mount_crypt_stat *mount_crypt_stat)
{
	memset((void *)mount_crypt_stat, 0,
	       sizeof(struct ecryptfs_mount_crypt_stat));
	INIT_LIST_HEAD(&mount_crypt_stat->global_auth_tok_list);
	mutex_init(&mount_crypt_stat->global_auth_tok_list_mutex);
#ifdef MY_ABC_HERE
	mount_crypt_stat->flags |= ECRYPTFS_GLOBAL_FAST_LOOKUP_ENABLED;
#endif  
	mount_crypt_stat->flags |= ECRYPTFS_MOUNT_CRYPT_STAT_INITIALIZED;
}

static int ecryptfs_parse_options(struct ecryptfs_sb_info *sbi, char *options,
				  uid_t *check_ruid)
{
	char *p;
	int rc = 0;
	int sig_set = 0;
	int cipher_name_set = 0;
	int fn_cipher_name_set = 0;
	int cipher_key_bytes;
	int cipher_key_bytes_set = 0;
	int fn_cipher_key_bytes;
	int fn_cipher_key_bytes_set = 0;
	struct ecryptfs_mount_crypt_stat *mount_crypt_stat =
		&sbi->mount_crypt_stat;
	substring_t args[MAX_OPT_ARGS];
	int token;
	char *sig_src;
	char *cipher_name_dst;
	char *cipher_name_src;
	char *fn_cipher_name_dst;
	char *fn_cipher_name_src;
	char *fnek_dst;
	char *fnek_src;
	char *cipher_key_bytes_src;
	char *fn_cipher_key_bytes_src;
	u8 cipher_code;

	*check_ruid = 0;

	if (!options) {
		rc = -EINVAL;
		goto out;
	}
	ecryptfs_init_mount_crypt_stat(mount_crypt_stat);
	while ((p = strsep(&options, ",")) != NULL) {
		if (!*p)
			continue;
		token = match_token(p, tokens, args);
		switch (token) {
		case ecryptfs_opt_sig:
		case ecryptfs_opt_ecryptfs_sig:
			sig_src = args[0].from;
			rc = ecryptfs_add_global_auth_tok(mount_crypt_stat,
							  sig_src, 0);
			if (rc) {
				printk(KERN_ERR "Error attempting to register "
				       "global sig; rc = [%d]\n", rc);
				goto out;
			}
			sig_set = 1;
			break;
		case ecryptfs_opt_cipher:
		case ecryptfs_opt_ecryptfs_cipher:
			cipher_name_src = args[0].from;
			cipher_name_dst =
				mount_crypt_stat->
				global_default_cipher_name;
			strncpy(cipher_name_dst, cipher_name_src,
				ECRYPTFS_MAX_CIPHER_NAME_SIZE);
			cipher_name_dst[ECRYPTFS_MAX_CIPHER_NAME_SIZE] = '\0';
			cipher_name_set = 1;
			break;
		case ecryptfs_opt_ecryptfs_key_bytes:
			cipher_key_bytes_src = args[0].from;
			cipher_key_bytes =
				(int)simple_strtol(cipher_key_bytes_src,
						   &cipher_key_bytes_src, 0);
			mount_crypt_stat->global_default_cipher_key_size =
				cipher_key_bytes;
			cipher_key_bytes_set = 1;
			break;
		case ecryptfs_opt_passthrough:
			mount_crypt_stat->flags |=
				ECRYPTFS_PLAINTEXT_PASSTHROUGH_ENABLED;
			break;
		case ecryptfs_opt_xattr_metadata:
			mount_crypt_stat->flags |=
				ECRYPTFS_XATTR_METADATA_ENABLED;
			break;
		case ecryptfs_opt_encrypted_view:
			mount_crypt_stat->flags |=
				ECRYPTFS_XATTR_METADATA_ENABLED;
			mount_crypt_stat->flags |=
				ECRYPTFS_ENCRYPTED_VIEW_ENABLED;
			break;
		case ecryptfs_opt_fnek_sig:
			fnek_src = args[0].from;
			fnek_dst =
				mount_crypt_stat->global_default_fnek_sig;
			strncpy(fnek_dst, fnek_src, ECRYPTFS_SIG_SIZE_HEX);
			mount_crypt_stat->global_default_fnek_sig[
				ECRYPTFS_SIG_SIZE_HEX] = '\0';
			rc = ecryptfs_add_global_auth_tok(
				mount_crypt_stat,
				mount_crypt_stat->global_default_fnek_sig,
				ECRYPTFS_AUTH_TOK_FNEK);
			if (rc) {
				printk(KERN_ERR "Error attempting to register "
				       "global fnek sig [%s]; rc = [%d]\n",
				       mount_crypt_stat->global_default_fnek_sig,
				       rc);
				goto out;
			}
			mount_crypt_stat->flags |=
				(ECRYPTFS_GLOBAL_ENCRYPT_FILENAMES
				 | ECRYPTFS_GLOBAL_ENCFN_USE_MOUNT_FNEK);
			break;
		case ecryptfs_opt_fn_cipher:
			fn_cipher_name_src = args[0].from;
			fn_cipher_name_dst =
				mount_crypt_stat->global_default_fn_cipher_name;
			strncpy(fn_cipher_name_dst, fn_cipher_name_src,
				ECRYPTFS_MAX_CIPHER_NAME_SIZE);
			mount_crypt_stat->global_default_fn_cipher_name[
				ECRYPTFS_MAX_CIPHER_NAME_SIZE] = '\0';
			fn_cipher_name_set = 1;
			break;
		case ecryptfs_opt_fn_cipher_key_bytes:
			fn_cipher_key_bytes_src = args[0].from;
			fn_cipher_key_bytes =
				(int)simple_strtol(fn_cipher_key_bytes_src,
						   &fn_cipher_key_bytes_src, 0);
			mount_crypt_stat->global_default_fn_cipher_key_bytes =
				fn_cipher_key_bytes;
			fn_cipher_key_bytes_set = 1;
			break;
		case ecryptfs_opt_unlink_sigs:
			mount_crypt_stat->flags |= ECRYPTFS_UNLINK_SIGS;
			break;
		case ecryptfs_opt_mount_auth_tok_only:
			mount_crypt_stat->flags |=
				ECRYPTFS_GLOBAL_MOUNT_AUTH_TOK_ONLY;
			break;
		case ecryptfs_opt_check_dev_ruid:
			*check_ruid = 1;
			break;
#ifdef MY_ABC_HERE
		case ecryptfs_opt_no_fast_lookup:
			mount_crypt_stat->flags &= (~ECRYPTFS_GLOBAL_FAST_LOOKUP_ENABLED);
			break;
#endif  
		case ecryptfs_opt_err:
		default:
			printk(KERN_WARNING
			       "%s: eCryptfs: unrecognized option [%s]\n",
			       __func__, p);
		}
	}
	if (!sig_set) {
		rc = -EINVAL;
		ecryptfs_printk(KERN_ERR, "You must supply at least one valid "
				"auth tok signature as a mount "
				"parameter; see the eCryptfs README\n");
		goto out;
	}
	if (!cipher_name_set) {
		int cipher_name_len = strlen(ECRYPTFS_DEFAULT_CIPHER);

		BUG_ON(cipher_name_len > ECRYPTFS_MAX_CIPHER_NAME_SIZE);
		strcpy(mount_crypt_stat->global_default_cipher_name,
		       ECRYPTFS_DEFAULT_CIPHER);
	}
	if ((mount_crypt_stat->flags & ECRYPTFS_GLOBAL_ENCRYPT_FILENAMES)
	    && !fn_cipher_name_set)
		strcpy(mount_crypt_stat->global_default_fn_cipher_name,
		       mount_crypt_stat->global_default_cipher_name);
	if (!cipher_key_bytes_set)
		mount_crypt_stat->global_default_cipher_key_size = 0;
	if ((mount_crypt_stat->flags & ECRYPTFS_GLOBAL_ENCRYPT_FILENAMES)
	    && !fn_cipher_key_bytes_set)
		mount_crypt_stat->global_default_fn_cipher_key_bytes =
			mount_crypt_stat->global_default_cipher_key_size;

	cipher_code = ecryptfs_code_for_cipher_string(
		mount_crypt_stat->global_default_cipher_name,
		mount_crypt_stat->global_default_cipher_key_size);
	if (!cipher_code) {
		ecryptfs_printk(KERN_ERR,
				"eCryptfs doesn't support cipher: %s",
				mount_crypt_stat->global_default_cipher_name);
		rc = -EINVAL;
		goto out;
	}

	mutex_lock(&key_tfm_list_mutex);
	if (!ecryptfs_tfm_exists(mount_crypt_stat->global_default_cipher_name,
				 NULL)) {
		rc = ecryptfs_add_new_key_tfm(
			NULL, mount_crypt_stat->global_default_cipher_name,
			mount_crypt_stat->global_default_cipher_key_size);
		if (rc) {
			printk(KERN_ERR "Error attempting to initialize "
			       "cipher with name = [%s] and key size = [%td]; "
			       "rc = [%d]\n",
			       mount_crypt_stat->global_default_cipher_name,
			       mount_crypt_stat->global_default_cipher_key_size,
			       rc);
			rc = -EINVAL;
			mutex_unlock(&key_tfm_list_mutex);
			goto out;
		}
	}
	if ((mount_crypt_stat->flags & ECRYPTFS_GLOBAL_ENCRYPT_FILENAMES)
	    && !ecryptfs_tfm_exists(
		    mount_crypt_stat->global_default_fn_cipher_name, NULL)) {
		rc = ecryptfs_add_new_key_tfm(
			NULL, mount_crypt_stat->global_default_fn_cipher_name,
			mount_crypt_stat->global_default_fn_cipher_key_bytes);
		if (rc) {
			printk(KERN_ERR "Error attempting to initialize "
			       "cipher with name = [%s] and key size = [%td]; "
			       "rc = [%d]\n",
			       mount_crypt_stat->global_default_fn_cipher_name,
			       mount_crypt_stat->global_default_fn_cipher_key_bytes,
			       rc);
			rc = -EINVAL;
			mutex_unlock(&key_tfm_list_mutex);
			goto out;
		}
	}
	mutex_unlock(&key_tfm_list_mutex);
	rc = ecryptfs_init_global_auth_toks(mount_crypt_stat);
	if (rc)
		printk(KERN_WARNING "One or more global auth toks could not "
		       "properly register; rc = [%d]\n", rc);
out:
	return rc;
}

#ifdef MY_ABC_HERE
static int ecryptfs_test_super(struct super_block *s, void *data)
{
	struct ecryptfs_sb_info *p = data;
	struct ecryptfs_sb_info *sb_info = ecryptfs_superblock_to_private(s);

	return sb_info->dentry == p->dentry;
}

static int ecryptfs_set_super(struct super_block *s, void *data)
{
	int err = set_anon_super(s, data);
	if (!err) {
		ecryptfs_set_superblock_private(s, (struct ecryptfs_sb_info *)data);
	}
	return err;
}
#endif  

struct kmem_cache *ecryptfs_sb_info_cache;
static struct file_system_type ecryptfs_fs_type;

static struct dentry *ecryptfs_mount(struct file_system_type *fs_type, int flags,
			const char *dev_name, void *raw_data)
{
	struct super_block *s;
	struct ecryptfs_sb_info *sbi;
	struct ecryptfs_mount_crypt_stat *mount_crypt_stat;
	struct ecryptfs_dentry_info *root_info;
	const char *err = "Getting sb failed";
	struct inode *inode;
	struct path path;
	uid_t check_ruid;
	int rc;

	sbi = kmem_cache_zalloc(ecryptfs_sb_info_cache, GFP_KERNEL);
	if (!sbi) {
		rc = -ENOMEM;
		goto out;
	}

#ifdef MY_ABC_HERE
	rc = kern_path(dev_name, LOOKUP_FOLLOW | LOOKUP_DIRECTORY, &path);
	if (rc) {
		ecryptfs_printk(KERN_WARNING, "kern_path() failed\n");
		goto out;
	}

	sbi->dentry = path.dentry;
	s = sget(fs_type, ecryptfs_test_super, ecryptfs_set_super, flags, sbi);
	if (IS_ERR(s)) {
		rc = PTR_ERR(s);
		goto out1;
	}

	if (s->s_root) {
		kmem_cache_free(ecryptfs_sb_info_cache, sbi);
		path_put(&path);
		return dget(s->s_root);
	}

#endif  

	rc = ecryptfs_parse_options(sbi, raw_data, &check_ruid);
	if (rc) {
		err = "Error parsing options";
#ifdef MY_ABC_HERE
		goto out_free;
#else  
		goto out;
#endif  
	}
	mount_crypt_stat = &sbi->mount_crypt_stat;

#ifdef MY_ABC_HERE
#else  
	s = sget(fs_type, NULL, set_anon_super, flags, NULL);
	if (IS_ERR(s)) {
		rc = PTR_ERR(s);
		goto out;
	}
#endif  

	rc = bdi_setup_and_register(&sbi->bdi, "ecryptfs");
	if (rc)
#ifdef MY_ABC_HERE
		goto out_free;
#else  
		goto out1;
#endif  

	ecryptfs_set_superblock_private(s, sbi);
	s->s_bdi = &sbi->bdi;

	sbi = NULL;
	s->s_op = &ecryptfs_sops;
	s->s_d_op = &ecryptfs_dops;

	err = "Reading sb failed";
#ifdef MY_ABC_HERE
#else  
	rc = kern_path(dev_name, LOOKUP_FOLLOW | LOOKUP_DIRECTORY, &path);
	if (rc) {
		ecryptfs_printk(KERN_WARNING, "kern_path() failed\n");
		goto out1;
	}
#endif  
	if (path.dentry->d_sb->s_type == &ecryptfs_fs_type) {
		rc = -EINVAL;
		printk(KERN_ERR "Mount on filesystem of type "
			"eCryptfs explicitly disallowed due to "
			"known incompatibilities\n");
		goto out_free;
	}

	if (check_ruid && !uid_eq(d_inode(path.dentry)->i_uid, current_uid())) {
		rc = -EPERM;
		printk(KERN_ERR "Mount of device (uid: %d) not owned by "
		       "requested user (uid: %d)\n",
			i_uid_read(d_inode(path.dentry)),
			from_kuid(&init_user_ns, current_uid()));
		goto out_free;
	}

	ecryptfs_set_superblock_lower(s, path.dentry->d_sb);

	s->s_flags = flags & ~MS_POSIXACL;
	s->s_flags |= path.dentry->d_sb->s_flags & MS_POSIXACL;

	if (path.dentry->d_sb->s_flags & MS_RDONLY ||
	    mount_crypt_stat->flags & ECRYPTFS_ENCRYPTED_VIEW_ENABLED)
		s->s_flags |= MS_RDONLY;

	s->s_maxbytes = path.dentry->d_sb->s_maxbytes;
	s->s_blocksize = path.dentry->d_sb->s_blocksize;
	s->s_magic = ECRYPTFS_SUPER_MAGIC;
	s->s_stack_depth = path.dentry->d_sb->s_stack_depth + 1;

	rc = -EINVAL;
	if (s->s_stack_depth > FILESYSTEM_MAX_STACK_DEPTH) {
		pr_err("eCryptfs: maximum fs stacking depth exceeded\n");
		goto out_free;
	}

	inode = ecryptfs_get_inode(d_inode(path.dentry), s);
	rc = PTR_ERR(inode);
	if (IS_ERR(inode))
		goto out_free;

	s->s_root = d_make_root(inode);
	if (!s->s_root) {
		rc = -ENOMEM;
		goto out_free;
	}

	rc = -ENOMEM;
	root_info = kmem_cache_zalloc(ecryptfs_dentry_info_cache, GFP_KERNEL);
	if (!root_info)
		goto out_free;

	ecryptfs_set_dentry_private(s->s_root, root_info);
	root_info->lower_path = path;

	s->s_flags |= MS_ACTIVE;
	return dget(s->s_root);

#ifdef MY_ABC_HERE
out_free:
	deactivate_locked_super(s);
out1:
	path_put(&path);
#else  
out_free:
	path_put(&path);
out1:
	deactivate_locked_super(s);
#endif  
out:
	if (sbi) {
		ecryptfs_destroy_mount_crypt_stat(&sbi->mount_crypt_stat);
		kmem_cache_free(ecryptfs_sb_info_cache, sbi);
	}
	printk(KERN_ERR "%s; rc = [%d]\n", err, rc);
	return ERR_PTR(rc);
}

static void ecryptfs_kill_block_super(struct super_block *sb)
{
	struct ecryptfs_sb_info *sb_info = ecryptfs_superblock_to_private(sb);
	kill_anon_super(sb);
	if (!sb_info)
		return;
	ecryptfs_destroy_mount_crypt_stat(&sb_info->mount_crypt_stat);
	bdi_destroy(&sb_info->bdi);
	kmem_cache_free(ecryptfs_sb_info_cache, sb_info);
}

static struct file_system_type ecryptfs_fs_type = {
	.owner = THIS_MODULE,
	.name = "ecryptfs",
	.mount = ecryptfs_mount,
	.kill_sb = ecryptfs_kill_block_super,
	.fs_flags = 0
};
MODULE_ALIAS_FS("ecryptfs");

static void
inode_info_init_once(void *vptr)
{
	struct ecryptfs_inode_info *ei = (struct ecryptfs_inode_info *)vptr;

	inode_init_once(&ei->vfs_inode);
}

static struct ecryptfs_cache_info {
	struct kmem_cache **cache;
	const char *name;
	size_t size;
	void (*ctor)(void *obj);
} ecryptfs_cache_infos[] = {
	{
		.cache = &ecryptfs_auth_tok_list_item_cache,
		.name = "ecryptfs_auth_tok_list_item",
		.size = sizeof(struct ecryptfs_auth_tok_list_item),
	},
	{
		.cache = &ecryptfs_file_info_cache,
		.name = "ecryptfs_file_cache",
		.size = sizeof(struct ecryptfs_file_info),
	},
	{
		.cache = &ecryptfs_dentry_info_cache,
		.name = "ecryptfs_dentry_info_cache",
		.size = sizeof(struct ecryptfs_dentry_info),
	},
	{
		.cache = &ecryptfs_inode_info_cache,
		.name = "ecryptfs_inode_cache",
		.size = sizeof(struct ecryptfs_inode_info),
		.ctor = inode_info_init_once,
	},
	{
		.cache = &ecryptfs_sb_info_cache,
		.name = "ecryptfs_sb_cache",
		.size = sizeof(struct ecryptfs_sb_info),
	},
	{
		.cache = &ecryptfs_header_cache,
		.name = "ecryptfs_headers",
		.size = PAGE_CACHE_SIZE,
	},
	{
		.cache = &ecryptfs_xattr_cache,
		.name = "ecryptfs_xattr_cache",
		.size = PAGE_CACHE_SIZE,
	},
	{
		.cache = &ecryptfs_key_record_cache,
		.name = "ecryptfs_key_record_cache",
		.size = sizeof(struct ecryptfs_key_record),
	},
	{
		.cache = &ecryptfs_key_sig_cache,
		.name = "ecryptfs_key_sig_cache",
		.size = sizeof(struct ecryptfs_key_sig),
	},
	{
		.cache = &ecryptfs_global_auth_tok_cache,
		.name = "ecryptfs_global_auth_tok_cache",
		.size = sizeof(struct ecryptfs_global_auth_tok),
	},
	{
		.cache = &ecryptfs_key_tfm_cache,
		.name = "ecryptfs_key_tfm_cache",
		.size = sizeof(struct ecryptfs_key_tfm),
	},
};

static void ecryptfs_free_kmem_caches(void)
{
	int i;

	rcu_barrier();

	for (i = 0; i < ARRAY_SIZE(ecryptfs_cache_infos); i++) {
		struct ecryptfs_cache_info *info;

		info = &ecryptfs_cache_infos[i];
		if (*(info->cache))
			kmem_cache_destroy(*(info->cache));
	}
}

static int ecryptfs_init_kmem_caches(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ecryptfs_cache_infos); i++) {
		struct ecryptfs_cache_info *info;

		info = &ecryptfs_cache_infos[i];
		*(info->cache) = kmem_cache_create(info->name, info->size,
				0, SLAB_HWCACHE_ALIGN, info->ctor);
		if (!*(info->cache)) {
			ecryptfs_free_kmem_caches();
			ecryptfs_printk(KERN_WARNING, "%s: "
					"kmem_cache_create failed\n",
					info->name);
			return -ENOMEM;
		}
	}
	return 0;
}

static struct kobject *ecryptfs_kobj;

static ssize_t version_show(struct kobject *kobj,
			    struct kobj_attribute *attr, char *buff)
{
	return snprintf(buff, PAGE_SIZE, "%d\n", ECRYPTFS_VERSIONING_MASK);
}

static struct kobj_attribute version_attr = __ATTR_RO(version);

static struct attribute *attributes[] = {
	&version_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attributes,
};

static int do_sysfs_registration(void)
{
	int rc;

	ecryptfs_kobj = kobject_create_and_add("ecryptfs", fs_kobj);
	if (!ecryptfs_kobj) {
		printk(KERN_ERR "Unable to create ecryptfs kset\n");
		rc = -ENOMEM;
		goto out;
	}
	rc = sysfs_create_group(ecryptfs_kobj, &attr_group);
	if (rc) {
		printk(KERN_ERR
		       "Unable to create ecryptfs version attributes\n");
		kobject_put(ecryptfs_kobj);
	}
out:
	return rc;
}

static void do_sysfs_unregistration(void)
{
	sysfs_remove_group(ecryptfs_kobj, &attr_group);
	kobject_put(ecryptfs_kobj);
}

static int __init ecryptfs_init(void)
{
	int rc;

	if (ECRYPTFS_DEFAULT_EXTENT_SIZE > PAGE_CACHE_SIZE) {
		rc = -EINVAL;
		ecryptfs_printk(KERN_ERR, "The eCryptfs extent size is "
				"larger than the host's page size, and so "
				"eCryptfs cannot run on this system. The "
				"default eCryptfs extent size is [%u] bytes; "
				"the page size is [%lu] bytes.\n",
				ECRYPTFS_DEFAULT_EXTENT_SIZE,
				(unsigned long)PAGE_CACHE_SIZE);
		goto out;
	}
	rc = ecryptfs_init_kmem_caches();
	if (rc) {
		printk(KERN_ERR
		       "Failed to allocate one or more kmem_cache objects\n");
		goto out;
	}
	rc = do_sysfs_registration();
	if (rc) {
		printk(KERN_ERR "sysfs registration failed\n");
		goto out_free_kmem_caches;
	}
	rc = ecryptfs_init_kthread();
	if (rc) {
		printk(KERN_ERR "%s: kthread initialization failed; "
		       "rc = [%d]\n", __func__, rc);
		goto out_do_sysfs_unregistration;
	}
	rc = ecryptfs_init_messaging();
	if (rc) {
		printk(KERN_ERR "Failure occurred while attempting to "
				"initialize the communications channel to "
				"ecryptfsd\n");
		goto out_destroy_kthread;
	}
	rc = ecryptfs_init_crypto();
	if (rc) {
		printk(KERN_ERR "Failure whilst attempting to init crypto; "
		       "rc = [%d]\n", rc);
		goto out_release_messaging;
	}
	rc = register_filesystem(&ecryptfs_fs_type);
	if (rc) {
		printk(KERN_ERR "Failed to register filesystem\n");
		goto out_destroy_crypto;
	}
	if (ecryptfs_verbosity > 0)
		printk(KERN_CRIT "eCryptfs verbosity set to %d. Secret values "
			"will be written to the syslog!\n", ecryptfs_verbosity);

	goto out;
out_destroy_crypto:
	ecryptfs_destroy_crypto();
out_release_messaging:
	ecryptfs_release_messaging();
out_destroy_kthread:
	ecryptfs_destroy_kthread();
out_do_sysfs_unregistration:
	do_sysfs_unregistration();
out_free_kmem_caches:
	ecryptfs_free_kmem_caches();
out:
	return rc;
}

static void __exit ecryptfs_exit(void)
{
	int rc;

	rc = ecryptfs_destroy_crypto();
	if (rc)
		printk(KERN_ERR "Failure whilst attempting to destroy crypto; "
		       "rc = [%d]\n", rc);
	ecryptfs_release_messaging();
	ecryptfs_destroy_kthread();
	do_sysfs_unregistration();
	unregister_filesystem(&ecryptfs_fs_type);
	ecryptfs_free_kmem_caches();
}

MODULE_AUTHOR("Michael A. Halcrow <mhalcrow@us.ibm.com>");
MODULE_DESCRIPTION("eCryptfs");

MODULE_LICENSE("GPL");

module_init(ecryptfs_init)
module_exit(ecryptfs_exit)
