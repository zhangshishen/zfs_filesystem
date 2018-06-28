#include "ext2.h"
#include <linux/quotaops.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/buffer_head.h>
#include <linux/capability.h>

static inline int zfs_get_value_by_offset_in_block(int *block_buf,int block_offset){

}

static int zfs_get_zero_offset_in_block(int *block_buf){

    int count = 0;
    for(count=0;count<1024;count++){

        if(block_buf[count]+1==0){
            continue;
        }else{
            int one = 1;
            int offset_in_dword = 0;
            while(block_buf[count] & one ){
                one<<=1;
                offset_in_dword++;
            }
            block_buf[count] &= one;
            int zero_bit = offset_in_dword + count * 32;
            return zero_bit;
        }

    }
    return -1;
}

// alloc an empty block and mark the block bitmap
// no empty block if return zero ,or return the block number
static int zfs_new_blocks(struct inode* inode,int goal){       

    struct buffer_head *bitmap_bh = NULL;
    struct super_block* sb= inode->i_sb;
    sb->
    struct zfs_super_block* z_sb = sb->s_fs_info;

    int first_block_bitmap_block = z_sb->s_inode_bitmap_block_number + 2;
    int block_bitmap_block_number = z_sb->s_block_bitmap_number;
    int count = 0;

    while(count < block_bitmap_block_number){
        bitmap_bh = sb_bread(sb,first_block_bitmap_block + count);
        int *block_bit = bitmap_bh->b_data;
        
        int empty_block = zfs_get_zero_offset_in_block(block_bit);

        if(empty_block != -1){  //find empty block
            inode->i_size += 4096;
            mark_buffer_dirty(bitmap_bh);
            return empty_block + count * 4096 * 8;
        }else{
            brelse(bitmap_bh);
        }

    }
    return 0;

}