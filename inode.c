#include <linux/time.h>
#include <linux/highuid.h>
#include <linux/pagemap.h>
#include <linux/dax.h>
#include <linux/blkdev.h>
#include <linux/quotaops.h>
#include <linux/writeback.h>
#include <linux/buffer_head.h>
#include <linux/mpage.h>
#include <linux/fiemap.h>
#include <linux/iomap.h>
#include <linux/namei.h>
#include <linux/uio.h>

#include "zfs.h"


#define SIZEPERINODEENRTY 4	//32 bit max inode number
#define LEVEL0ENRTY 4		//inode block number first level
#define LEVEL1ENTRY 6
//#define LEVEL2ENTRY 2

#define LEVEL0SIZE 4096
#define LEVEL1SIZE ((4096/SIZEPERINODEENRTY)*4096)
//#define LEVEL2SIZE ((4096/SIZEPERINODEENRTY)*(4096/SIZEPERINODEENRTY)*4096)


static int __zfs_write_inode(struct inode *inode, int do_sync)
{

	struct super_block *sb = inode->i_sb;
	ino_t ino = inode->i_ino;
	struct buffer_head *bh;
	struct zfs_inode* raw_inode = zfs_get_inode(sb,ino,&bh);

	struct zfs_inode_info* zi = container_of(inode,struct zfs_inode_info,vfs_inode);
	if(IS_ERR(raw_inode))
		return -EIO;
	raw_inode->i_mode = cpu_to_le16(inode->i_mode);

	raw_inode->i_nlink = inode->i_nlink;
	raw_inode->i_size = inode->i_size;
	int n;
	for(n = 0;n < N_BLOCK;n ++){
		raw_inode->i_block[n] = zi->i_data[n];	
	}
	make_buffer_dirty(bh);
	sync_dirty_buffer(bh);
	brelse(bh);

}

int zfs_write_inode(struct inode *inode, struct writeback_control *wbc)
{
	return __zfs_write_inode(inode, wbc->sync_mode == WB_SYNC_ALL);
}

int get_depth(sector_t iblock, int* offset){
 	int res = 1;
	if(iblock> 4096*1024){
		return 0;	
	}
	if(iblock + 1> LEVEL0ENRTY){
		res++;
	}else{
		index[0] = iblock;
		return res;
	}
	iblock -= LEVEL0ENRTY;
	index[0] = iblock / 1024 + LEVEL0ENRTY;
	if(index[0]>10) return 0; 
	index[1] = iblock % 1024;
	return res;
}

static int zfs_get_block_number(struct inode* inode,int* offset,int depth){
	struct zfs_inode_info* zi = ZFS_I(inode);
	if(zi->i_data[offset[0]] == 0){
		goto no_block;
	}
	if(depth==1){	//only level 1 page
		return zi->i_data[offset[0]];
	}
	struct buffer_head* bh = sb_bread(inode->i_sb,zi->i_data[offset[0]]);
	
	if(!bh){
		goto read_error;
	}

	unsigned int *entry = bh->b_data;
	if(entry[offset[1]]==0)){
		brelse(bh);
		goto no_block;
	}
	return entry[offset[1]];

	

read_error
	printk("cant read buffer while getting block\n");
	return NULL;
no_block:
	offset[0] = -1;
	return NULL;
}

static int zfs_get_block(struct inode* inode,sector_t iblock, struct buffer_head* bh_result, int create){

	int offset[4];
	int depth = get_depth(iblock,depth);
	if(depth == 0)
		return -EIO;
	int num;
	num = zfs_get_block_buf(inode,offset,depth);

	if(!bh){
		if(offset[0]==-1){
			//no block ,alloc block
		}else{
			//read block error
			return -EIO
		}
	}
	
	map_bh(bh_result,inode0>i_sb,num);

	
}

const struct address_space_operations zfs_aops = {
	.readpage		= zfs_readpage,
	.readpages		= zfs_readpages,
	.writepage		= zfs_writepage,
	.write_begin		= zfs_write_begin,
	.write_end		= zfs_write_end,
	.bmap			= zfs_bmap,
	.direct_IO		= zfs_direct_IO,
	.writepages		= zfs_writepages,
	.migratepage		= buffer_migrate_page,
	.is_partially_uptodate	= block_is_partially_uptodate,
	.error_remove_page	= generic_error_remove_page,
};

struct inode * zfs_iget(struct super_block* sb,unsigned long ino){	

	struct inode* inode;
	struct zfs_inode* raw_inode;
	struct buffer_header* bh;
	inode = iget_locked(sb, ino);	// check inode hash that if ino inode has exist

	if (!inode)
		return ERR_PTR(-ENOMEM);
	if (!(inode->i_state & I_NEW))
		return inode;
	raw_inode = zfs_get_inode(sb,ino,&bh);
	//container_of(inode, struct ext2_inode_info, vfs_inode);
	
	inode->i_mode = raw_inode->i_mode;
	inode->i_uid = 0;
	inode->i_gid = 0;
	inode->i_size = raw_inode->i_size;

	inode->__i_nlink = raw_inode->i_nlink;
	inode->i_atime = inode->i_ctime = inode->m_time = 0;

	if(inode->i_nlink = 0 && inode->i_mode = 0){
		brelse(bh);
		ret = -ESTALE;	
		goto bad_inode;
		
	}

	if (S_ISREG(inode->i_mode)) {
		inode->i_op = &zfs_file_inode_operations;
		inode->i_mapping->a_ops = &zfs_aops;
		inode->i_fop = &zfs_file_operations;
		
	} else if (S_ISDIR(inode->i_mode)) {
		inode->i_op = &zfs_dir_inode_operations;
		inode->i_fop = &zfs_dir_operations;
		inode->i_mapping->a_ops = &zfs_aops;
	}else {
		goto bad_inode;	
	}
	brelse(bh);	//release memory of raw_inode, it will not be used any more
	unlock_new_inode(inode);
	return inode;

bad_inode:	
	iget_failed(inode);	//remove inode hash
	return ERR_PTR(ret);

}

//get a zfs inode ,map the memory to the disk with buffer_head
static struct zfs_inode* zfs_get_inode(struct super_block* sb,int ino,struct buffer_head **p)
{
	struct buffer_head *bh;

	struct zfs_sb_info *sbi;
	
	unsigned long block;
	unsigned long offset;
	*p = NULL;

	sbi = sb->s_fs_info;

	if(ino <1 || ino > sbi->s_max_inode_num){
		return ERR_PTR(-EINVAL);	
	}

	int inode_per_block = 4096 / sizeof(zfs_inode);
	int block_num = s_inode_table_block_num + (ino) / inode_per_block;
	int inode_offset = ((ino) % inode_per_block) * sizeof(zfs_inode);

	bh = sb_bread(sb,block_num);
	*p = bh;
	return (struct zfs_inode *)(bh->data + inode_offset);

} 
