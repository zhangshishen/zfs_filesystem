#include<linux/fs.h>
#include<linux/buffer_head.h>
#include<linux/pagemap.h>
#include<linux/swap.h>
#include "zfs.h"

static struct page *zfs_get_page(struct inode *dir,int n){
    struct address_space *as = dir->i_mapping;
    struct page *page = read_mapping_page(as,n,NULL);
    if(!IS_ERR(page)){
        kmap(page);
    }
    return page;
}

static inline void zfs_put_page(struct page *page){
    kunmap(page);
    page_cache_release(page);
}
static int zfs_readdir(struct file* file,struct dir_context *ctx)
{
    loff_t pos = ctx->pos;
    struct inode *inode = file_inode(file);
    struct super_block *sb = inode->i_sb;
    unsigned int offset = pos & ~PAGE_CACHE_MASK;
    unsigned int n = pos >> PAGE_CACHE_SHIFT;
    unsigned int npages = dir_pages(inode);
    
    if(pos > inode->i_size){
        return 0;
    }
    if(;n<npages;n++,offset = 0){
        char *kaddr,*limit;
        zfs_dir_entry* de;
        struct page *page = zfs_get_page(inode,n);

        if(IS_ERR(page)){
            printk("bad page \n");
            ctx->pos += PAGE_CACHE_SIZE; - offset;
            return PTR_ERR(page);
        }
        kaddr = page_address(page);
        de = (struct zfs_dir_entry *)(kaddr + offset);
        limit = kaddr + 4096;
        int d_entry_size = sizeof(zfs_dir_entry);
        
        for(;de<limit;de+= d_entry_size){
            if(de->rec_len == 0){

            }
            if(de->inode){
                unsigned char d_type = DT_UNKNOWN;
                int over;
                if(!dir_emit(ctx,de->name,de->name_len,de->inode,de->file_type)){
                    zfs_put_page(page);
                    return 0;
                }
            }
            ctx->pos += d_entry_size;
        }
    }
    return 0;

}

const struct file_operations zfs_dir_operations = {
	.llseek		= generic_file_llseek,
	.read		= generic_read_dir,
	.iterate	= zfs_readdir,
};