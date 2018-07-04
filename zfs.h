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
};


struct zfs_inode_info{
	int i_data[10];
	struct inode vfs_inode;
};


/*
block overall:

| 1 empty block | 1 super block | n block of inode bitmap | n block of block bitmap| n block of inode table | 1 root block | other block of data block|

two block of inode descriptor
*/
struct zfs_super_block {
	int s_inode_count;
	int s_free_blocks_count;
	int s_free_inode_count;
	int s_first_data_block;
	int s_inode_table_block_number;
	int s_inode_bitmap_block_number;
	int s_block_bitmap_number;
	int s_total_block_number;
	
};



struct zfs_sb_info {
	int s_inode_count;
	int s_free_blocks_count;
	int s_free_inode_count;
	int s_first_data_block;
	int s_inode_table_block_number;
	int s_inode_bitmap_block_number;
	int s_block_bitmap_number;
	int s_total_block_number;
	int s_max_inode_num;
	int s_max_block_num;
	int s_inode_table_block_num;
	struct buffer_head *s_bh;
	struct zfs_super_block* s_zs;
};
#define ZFS_MAX_NAME_LEN 28
struct zfs_dir_entry{
	int inode;
	int name_len;
	int rec_len;
	int file_type;
	char name[ZFS_MAX_NAME_LEN];
};

static inline struct zfs_inode_info * ZFS_I(struct inode* inode){
	return container_of(inode,struct zfs_inode_info,vfs_inode);
};