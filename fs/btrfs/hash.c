 
#include <crypto/hash.h>
#include <linux/err.h>
#include "hash.h"

static struct crypto_shash *tfm;

int __init btrfs_hash_init(void)
{
	tfm = crypto_alloc_shash("crc32c", 0, 0);

	return PTR_ERR_OR_ZERO(tfm);
}

const char* btrfs_crc32c_impl(void)
{
	return crypto_tfm_alg_driver_name(crypto_shash_tfm(tfm));
}

void btrfs_hash_exit(void)
{
	crypto_free_shash(tfm);
}

u32 btrfs_crc32c(u32 crc, const void *address, unsigned int length)
{
	SHASH_DESC_ON_STACK(shash, tfm);
	u32 *ctx = (u32 *)shash_desc_ctx(shash);
	int err;

	shash->tfm = tfm;
	shash->flags = 0;
	*ctx = crc;

	err = crypto_shash_update(shash, address, length);
	BUG_ON(err);

	return *ctx;
}
