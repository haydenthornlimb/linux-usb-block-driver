#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/skbuff.h>
#include <linux/freezer.h>
#include <linux/moduleparam.h>
#include <linux/mutex.h>

#include <linux/fs.h>
#include <linux/bio.h>
#include <linux/buffer_head.h>
#include <linux/blkdev.h>
#include <linux/version.h>
#include <linux/blkpg.h>
#include <linux/namei.h>
#include <linux/vmalloc.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hayden Thornlimb");
MODULE_DESCRIPTION("A Block Abstraction Read/Write for a USB device.");
MODULE_VERSION("1.0");

char* device = NULL;
module_param(device, charp, S_IRUGO);

unsigned int cur_dev_sector = 0;
static struct block_device *bdevice = NULL;
static struct bio *usb_bio = NULL;
static struct file *usb_file = NULL;

bool kmod_ioctl_init(void);
void kmod_ioctl_teardown(void);
long rw_usb(char* data, unsigned int size, unsigned int offset, bool flag);

static bool open_usb(void)
{
    if (!device) {
        printk("error: device parameter is NULL. Use insmod kmod.ko device=/dev/sdX\n");
        return false;
    }

    usb_file = bdev_file_open_by_path(device, BLK_OPEN_READ | BLK_OPEN_WRITE, NULL, NULL);
    if (IS_ERR(usb_file)) {
        printk("error: failed to open block device %s\n", device);
        usb_file = NULL;
        return false;
    }

    bdevice = file_bdev(usb_file);
    if (!bdevice) {
        printk("error: failed to get block device\n");
        return false;
    }

    cur_dev_sector = 0;
    printk("opened USB block device: %s\n", device);
    return true;
}

long rw_usb(char* data, unsigned int size, unsigned int offset, bool flag)
{
    unsigned int remaining = size;
    unsigned int processed = 0;
    unsigned int page_offset;
    int len;
    int bio_add_page_result;
    int ret;

    if (IS_ERR(data) || data == NULL) {
        printk("error: message not correct.\n");
        return -1;
    }

    while (remaining > 0) {
        len = remaining > 512 ? 512 : remaining;

        usb_bio = bio_alloc(bdevice, 1, flag ? REQ_OP_WRITE : REQ_OP_READ, GFP_NOIO);
        if (!usb_bio) {
            printk("error: bio_alloc failed\n");
            return -ENOMEM;
        }

        bio_set_dev(usb_bio, bdevice);

        if (offset == (unsigned int)-1) {
            usb_bio->bi_iter.bi_sector = cur_dev_sector;
        } else {
            printk("offset is provided (bytes = %d, sector = %d)\n", offset, offset / 512);
            usb_bio->bi_iter.bi_sector = offset / 512;
            cur_dev_sector = offset / 512;
            offset = -1;
        }

        if (flag == true) {
            usb_bio->bi_opf = REQ_OP_WRITE;
            printk("WRITE (size = %d, offset = %lld)\n",
                size, usb_bio->bi_iter.bi_sector);
        } else {
            usb_bio->bi_opf = REQ_OP_READ;
            printk("READ (size = %d, offset = %lld)\n",
                size, usb_bio->bi_iter.bi_sector);
        }

        page_offset = offset_in_page(data + processed);

        bio_add_page_result = bio_add_page(
            usb_bio,
            vmalloc_to_page(data + processed),
            len,
            page_offset
        );

        if (bio_add_page_result != len) {
            printk("error: bio_add_page failed\n");
            bio_put(usb_bio);
            return -EIO;
        }

        ret = submit_bio_wait(usb_bio);
        if (ret < 0) {
            printk("error: submit_bio_wait failed\n");
            bio_put(usb_bio);
            return ret;
        }

        bio_put(usb_bio);
        usb_bio = NULL;

        remaining -= len;
        processed += len;
        cur_dev_sector++;
    }

    return processed;
}

static void close_usb(void)
{
    if (usb_file) {
        bdev_fput(usb_file);
        usb_file = NULL;
        bdevice = NULL;
    }
}

static int __init kmod_init(void)
{
    pr_info("Hello World!\n");

    if (!open_usb()) {
        pr_err("Failed to open USB block device\n");
        return -ENODEV;
    }

    kmod_ioctl_init();
    return 0;
}

static void __exit kmod_fini(void)
{
    close_usb();
    kmod_ioctl_teardown();
    printk("Goodbye, World!\n");
}

module_init(kmod_init);
module_exit(kmod_fini);