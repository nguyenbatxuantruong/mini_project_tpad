#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/uaccess.h>

#define debug 0
#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define T_VENDOR_ID     0x045e
#define T_PRODUCT_ID    0x02ea
#define INT_EP_OUT      0x02
#define INT_EP_IN       0x82
#define ISOC_EP_OUT     0x03
#define ISOC_EP_IN      0x83
#define BULK_EP_OUT     0x04
#define BULK_EP_IN      0x84
#define MAX_PKT_SIZE    32

static struct usb_device *tgamepad_device;
static struct usb_class_driver tgamepad_class;
// static unsigned char usb_buff[MAX_PKT_SIZE];

static int tgamepad_hw_read(struct usb_device *dev, char *kbuf)
{
    int retval;
    int read_cnt;

    #if debug
    int i;
    #endif

    /* Read the data from the int endpoint */
    retval = usb_interrupt_msg(dev, usb_rcvintpipe(dev, INT_EP_IN),
            kbuf, MAX_PKT_SIZE, &read_cnt, 1000);
    
    #if debug
    if (read_cnt > 0)
    {
        printk("Debug int buff\n");
        for(i = 0; i < read_cnt; i++)
        {
            printk(" %d,",kernel_buf[i]);
        }
        printk(" -- %d\n", read_cnt);
    }
    #endif

    if (retval < 0)
    {
        printk(KERN_ERR "usb_interrupt_msg return %d\n", retval);
        return retval;
    }
    else
    {
        return read_cnt;
    }
}

static int tgamepad_hw_write(struct usb_device *dev, char *kbuf)
{
    int retval;
    int write_cnt;

    /* Write the data into the interrupt endpoint */
    retval = usb_interrupt_msg(dev, usb_sndintpipe(dev, INT_EP_OUT),
            kbuf, MAX_PKT_SIZE, &write_cnt, 1000);
    if (retval < 0)
    {
        printk(KERN_ERR "usb_interrupt_msg return %d\n", retval);
        return retval;
    }
    else
    {
        return 0;
    }
}

/***************************************************/
static int tgamepad_open(struct inode *i, struct file *f)
{
    return 0;
}

static int tgamepad_close(struct inode *i, struct file *f)
{
    return 0;
}

static ssize_t tgamepad_read(struct file *f, char __user *buf, size_t req_cnt, loff_t *off)
{
    int read_cnt;

    char *kernel_buf = NULL;

    kernel_buf = kzalloc(MAX_PKT_SIZE, GFP_KERNEL);
    if(NULL == kernel_buf)
    {
        return 0;
    }

    read_cnt = tgamepad_hw_read(tgamepad_device, kernel_buf);
    if (read_cnt < 0)
    {
        printk(KERN_ERR "tgamepad_hw_read\n");
        kfree(kernel_buf);
        return read_cnt;
    }

    if (copy_to_user(buf, kernel_buf, read_cnt))
    {
        printk(KERN_ERR "ERROR Copy to user\n");
        kfree(kernel_buf);
        return -EFAULT;
    }
    kfree(kernel_buf);
    return MIN(req_cnt, read_cnt);;
}

static ssize_t tgamepad_write(struct file *f, const char __user *req_buf, size_t cnt, loff_t *off)
{
    int retval;
    int write_cnt = MIN(cnt, MAX_PKT_SIZE);
    char *kernel_buf = NULL;

    kernel_buf = kzalloc(MAX_PKT_SIZE, GFP_KERNEL);
    if(NULL == kernel_buf)
    {
        return 0;
    }

    if (copy_from_user(kernel_buf, req_buf, MIN(cnt, MAX_PKT_SIZE)))
    {
        printk(KERN_ERR "ERROR Copy from user\n");
        kfree(kernel_buf);
        return -EFAULT;
    }

    /* Write the data */
    retval = tgamepad_hw_write(tgamepad_device, kernel_buf);
    if (retval < 0)
    {
        printk(KERN_ERR "tgamepad_hw_write\n");
        kfree(kernel_buf);
        return retval;
    }
    kfree(kernel_buf);
    return write_cnt;
}

static struct file_operations fops =
{
    .open = tgamepad_open,
    .release = tgamepad_close,
    .read = tgamepad_read,
    .write = tgamepad_write,
};

/***************************************************/
static int tgamepad_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    int retval;

    tgamepad_device = interface_to_usbdev(interface);

    tgamepad_class.name = "tgamepad%d";
    tgamepad_class.fops = &fops;

    if ((retval = usb_register_dev(interface, &tgamepad_class)) < 0)
    {
        /* Something prevented us from registering this driver */
        printk("Not able to get a minor for this device.");
    }
    else
    {
        printk(KERN_INFO "Minor tgamepad obtained: %d\n", interface->minor);
    }
    return retval;
}

static void tgamepad_disconnect(struct usb_interface *interface)
{
    usb_deregister_dev(interface, &tgamepad_class);
}

/* Table of devices that work with this driver */
static struct usb_device_id tgamepad_table[] =
{
    { USB_DEVICE(T_VENDOR_ID, T_PRODUCT_ID) },
    // { USB_DEVICE(0x090c, 0x1000) },
    {} /* Terminating entry */
};

MODULE_DEVICE_TABLE (usb, tgamepad_table);

static struct usb_driver tgamepad_driver =
{
    .name = "tgamepad_driver",
    .probe = tgamepad_probe,
    .disconnect = tgamepad_disconnect,
    .id_table = tgamepad_table,
};

/***************************************************/
static int __init tgamepad_init(void)
{
    int result;

    /* Register this driver with the USB subsystem */
    if ((result = usb_register(&tgamepad_driver)))
    {
        printk("usb_register failed. Error number %d", result);
    }
    return result;
}

static void __exit tgamepad_exit(void)
{
    /* Deregister this driver with the USB subsystem */
    usb_deregister(&tgamepad_driver);
}

module_init(tgamepad_init);
module_exit(tgamepad_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TruongNBX - TaiCT");
MODULE_DESCRIPTION("USB tgamepad Device Driver");
