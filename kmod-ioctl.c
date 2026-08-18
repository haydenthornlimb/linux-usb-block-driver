#include <linux/blkdev.h>
#include <linux/completion.h>
#include <linux/dcache.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/kref.h>
#include <linux/kthread.h>
#include <linux/limits.h>
#include <linux/rwsem.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/freezer.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/composite.h>
#include <linux/cdev.h>
#include <linux/nospec.h>
#include <linux/vmalloc.h>

#include "../ioctl-defines.h"

static dev_t            dev = 0;
static struct class*    kmod_class;
static struct cdev      kmod_cdev;

struct block_rw_ops rw_request;
struct block_rwoffset_ops rwoffset_request;

long rw_usb(char* data, unsigned int size, unsigned int offset, bool flag);

bool kmod_ioctl_init(void);
void kmod_ioctl_teardown(void);

static long kmod_ioctl(struct file *f, unsigned int cmd, unsigned long arg) {
    char* kernbuf;
    long ret;

    switch (cmd)
    {
        case BREAD:
            if (copy_from_user(&rw_request, (void __user *)arg, sizeof(rw_request)))
                return -EFAULT;

            kernbuf = vmalloc(rw_request.size);
            if (!kernbuf)
                return -ENOMEM;

            ret = rw_usb(kernbuf, rw_request.size, -1, false);

            if (ret >= 0) {
                if (copy_to_user(rw_request.data, kernbuf, rw_request.size))
                    ret = -EFAULT;
            }

            vfree(kernbuf);
            return ret;

        case BWRITE:
            if (copy_from_user(&rw_request, (void __user *)arg, sizeof(rw_request)))
                return -EFAULT;

            kernbuf = vmalloc(rw_request.size);
            if (!kernbuf)
                return -ENOMEM;

            if (copy_from_user(kernbuf, rw_request.data, rw_request.size)) {
                vfree(kernbuf);
                return -EFAULT;
            }

            ret = rw_usb(kernbuf, rw_request.size, -1, true);
            vfree(kernbuf);
            return ret;

        case BREADOFFSET:
            if (copy_from_user(&rwoffset_request, (void __user *)arg, sizeof(rwoffset_request)))
                return -EFAULT;

            kernbuf = vmalloc(rwoffset_request.size);
            if (!kernbuf)
                return -ENOMEM;

            ret = rw_usb(kernbuf, rwoffset_request.size, rwoffset_request.offset, false);

            if (ret >= 0) {
                if (copy_to_user(rwoffset_request.data, kernbuf, rwoffset_request.size))
                    ret = -EFAULT;
            }

            vfree(kernbuf);
            return ret;

        case BWRITEOFFSET:
            if (copy_from_user(&rwoffset_request, (void __user *)arg, sizeof(rwoffset_request)))
                return -EFAULT;

            kernbuf = vmalloc(rwoffset_request.size);
            if (!kernbuf)
                return -ENOMEM;

            if (copy_from_user(kernbuf, rwoffset_request.data, rwoffset_request.size)) {
                vfree(kernbuf);
                return -EFAULT;
            }

            ret = rw_usb(kernbuf, rwoffset_request.size, rwoffset_request.offset, true);
            vfree(kernbuf);
            return ret;

        default:
            printk("Error: incorrect operation requested, returning.\n");
            return -EINVAL;
    }

    return 0;
}

static int kmod_open(struct inode* inode, struct file* file) {
    printk("Opened kmod. \n");
    return 0;
}

static int kmod_release(struct inode* inode, struct file* file) {
    printk("Closed kmod. \n");
    return 0;
}

static struct file_operations fops =
{
    .owner          = THIS_MODULE,
    .open           = kmod_open,
    .release        = kmod_release,
    .unlocked_ioctl = kmod_ioctl,
};

bool kmod_ioctl_init(void) {
    if (alloc_chrdev_region(&dev, 0, 1, "usbaccess") < 0) {
        printk("error: couldn't allocate 'usbaccess' character device.\n");
        return false;
    }

    cdev_init(&kmod_cdev, &fops);
    if (cdev_add(&kmod_cdev, dev, 1) < 0) {
        printk("error: couldn't add kmod_cdev.\n");
        goto cdevfailed;
    }

#if LINUX_VERSION_CODE <= KERNEL_VERSION(6,2,16)
    if ((kmod_class = class_create(THIS_MODULE, "kmod_class")) == NULL) {
#else
    if ((kmod_class = class_create("kmod_class")) == NULL) {
#endif
        printk("error: couldn't create kmod_class.\n");
        goto cdevfailed;
    }

    if ((device_create(kmod_class, NULL, dev, NULL, "kmod")) == NULL) {
        printk("error: couldn't create device.\n");
        goto classfailed;
    }

    printk("[*] IOCTL device initialization complete.\n");
    return true;

classfailed:
    class_destroy(kmod_class);
cdevfailed:
    unregister_chrdev_region(dev, 1);
    return false;
}

void kmod_ioctl_teardown(void) {
    if (kmod_class) {
        device_destroy(kmod_class, dev);
        class_destroy(kmod_class);
    }
    cdev_del(&kmod_cdev);
    unregister_chrdev_region(dev,1);

    printk("[*] IOCTL device teardown complete.\n");
}