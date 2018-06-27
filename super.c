#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/parser.h>
#include <linux/random.h>
#include <linux/buffer_head.h>
#include <linux/exportfs.h>
#include <linux/vfs.h>
#include <linux/seq_file.h>
#include <linux/mount.h>
#include <linux/log2.h>
#include <linux/quotaops.h>
#include <linux/uaccess.h>
#include <linux/dax.h>




static struct kmem_cache* zfs_inode_cachep;

static struct inode *zfs_alloc_inode(struct super_block *sb){
	
	return &(kmem_cache_alloc(zfs_inode_cachep,GFP_KERNEL)->vfs_inode);

}





static const struct super_operations zfs_sops = {
	.alloc_inode	= zfs_alloc_inode,
	.destroy_inode	= zfs_destroy_inode,
	.write_inode	= zfs_write_inode,
	//.evict_inode	= zfs_evict_inode,
	.put_super	= zfs_put_super,


};

static void zfs_sync_super(struct zfs_sb_info* sbi){
	mark_buffer_dirty(sbi->s_bh);
	sync_dirty_buffer(sbi->s_bh);
	sb->s_dirt = 0;
}


static int zfs_put_super(struct super_block *sb){
	struct zfs_sb_info* sbi = sb->s_fs_info;
	zfs_sync_super(sb,ts);
	brelse(sbi->s_bh);
	kfree(sbi);
	
}

static int zfs_fill_super(struct super_block *sb, void *data, int silent){

	struct buffer_head* bh;
	struct zfs_sb_info* sbi;

	struct zfs_super_block* zs;
	struct inode *root;
	
	int blocksize = 4096;
	sbi = kzalloc(sizeof(*sbi),GFP_KERNEL);

	sb->s_fs_info = sbi;
	sb->s_blocksize = 4096;

	bh = sb_bread(sb,1);	//super_block in disk 1st block
	sbi->s_bh = bh;
	zs = (struct zfs_super_block *)((char *)bh->b_data);
	sbi->s_zs = zs;
	
	sb->s_op = &zfs_sops;
	
	root = zfs_iget(sb,1);	//root inode is inode 1
	if(IS_ERR(root)){
		ret = PTR_ERR(root);
		printk("cant find root inode");	
	}	
	sb->s_root = d_make_root(root);

	if(sb->s_root){
	}else{
		printk("cant create root dentry")
		ret = -ENOMEM;	
		brelse(bh);
		kfree(sbi);
	}
	
	return 0;

}








static struct dentry *zfs_mount(struct file_system_type *fs_type,
	int flags, const char *dev_name, void *data)
{
	return mount_bdev(fs_type, flags, dev_name, data, zfs_fill_super);
}



static struct file_system_type zss_fs_type = {
	.owner		= THIS_MODULE,
	.name		= "zfs",
	.mount		= zfs_mount,
	.kill_sb	= kill_block_super,
	.fs_flags	= FS_REQUIRES_DEV,
};

static int init_inodecache(void)
{
	zfs_inode_cachep = kmem_cache_create("zfs_inode_cache",
						sizeof(struct zfs_inode_info),
						0, (SLAB_RECLAIM_ACCOUNT|SLAB_MEM_SPREAD),
						init_once);
	if(!zfs_inode_cachep)
		return -ENOMEM;
	return 0;

}

static void destroy_inode_cache(void)
{
	kmem_cache_destroy(zfs_inode_cachep);
}

static int __init init_zfs_fs(void){
	
	init_inodecache();
	register_filesystem(zss_fs_type);
	return 0;

}

static void __exit exit_zfs_fs(void){

	unregister_filesystem(zss_fs_type);
	destroy_inodecache();
}













MODULE_AUTHOR("Zhang Shishen");
MODULE_DESCRIPTION("ZFS Filesystem");
MODULE_LICENSE("GPL");
module_init(init_zfs_fs)
module_exit(exit_zfs_fs)
