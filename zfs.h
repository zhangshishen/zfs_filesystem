#include <linux/fs.h>

struct zfs_inode{
	unsigned short i_uid;
	unsigned short i_mode;
	unsigned int i_size;
	unsigned int i_time;
	unsigned int i_ctime;
	unsigned int i_mtine;
	unsigned int i_flags;
	unsigned int i_nlink
	int i_block[10];
}


struct zfs_inode_info{
	int i_data[10];
	struct inode vfs_inode;
}
struct zfs_super_block {
	int s_inode_count;
	int s_free_blocks_count;
	int s_free_inode_count;
	int s_first_data_block;
	int s_inode_table_block_number;
	int s_inode_bitmap_block_number;
	
}



struct zfs_sb_info {
	int s_max_inode_num;
	int s_max_block_num;
	int s_inode_table_block_num;
	struct buffer_head *s_bh;
	struct zfs_super_block* s_zs;
}

static inline struct zfs_inode_info * ZFS_I(struct inode* inode){
	return container_of(inode,struct zfs_inode_info,vfs_inode);
}