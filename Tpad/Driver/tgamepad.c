#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/uaccess.h>

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

static struct usb_device *device;
static struct usb_class_driver class;
static unsigned char usb_buff[MAX_PKT_SIZE];

/***************************************************/
static int tgamepad_open(struct inode *i, struct file *f)
{
    return 0;
}

static int tgamepad_close(struct inode *i, struct file *f)
{
    return 0;
}

static ssize_t tgamepad_read(struct file *f, char __user *buf, size_t cnt, loff_t *off)
{
    int retval;
    int read_cnt;
    char *kernel_buf = NULL;

    printk("Handle read from %lld, %zu bytes\n", *off, cnt);
    kernel_buf = kzalloc(MAX_PKT_SIZE, GFP_KERNEL);
    if(NULL == kernel_buf)
    {
        return 0;
    }

    /* Read the data from the int endpoint */
    retval = usb_interrupt_msg(device, usb_rcvintpipe(device, INT_EP_IN),
            kernel_buf, MAX_PKT_SIZE, &read_cnt, 5000);
    printk("Debug int buff %s\n", kernel_buf);

    if (retval)
    {
        printk(KERN_ERR "INT message returned %d\n", retval);
        return retval;
    }
    if (copy_to_user(buf, kernel_buf, MAX_PKT_SIZE))
    {
        return -EFAULT;
    }
    return read_cnt;
}

static ssize_t tgamepad_write(struct file *f, const char __user *buf, size_t cnt, loff_t *off)
{
    int retval;
    int wrote_cnt = MIN(cnt, MAX_PKT_SIZE);

    if (copy_from_user(usb_buff, buf, MIN(cnt, MAX_PKT_SIZE)))
    {
        return -EFAULT;
    }

    /* Write the data into the bulk endpoint */
    retval = usb_interrupt_msg(device, usb_sndintpipe(device, INT_EP_OUT),
            usb_buff, MIN(cnt, MAX_PKT_SIZE), &wrote_cnt, 5000);
    if (retval)
    {
        printk(KERN_ERR "Bulk message returned %d\n", retval);
        return retval;
    }
    return wrote_cnt;
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

    device = interface_to_usbdev(interface);

    class.name = "tgamepad%d";
    class.fops = &fops;

    if ((retval = usb_register_dev(interface, &class)) < 0)
    {
        /* Something prevented us from registering this driver */
        printk("Not able to get a minor for this device.");
    }
    else
    {
        printk(KERN_INFO "Minor obtained: %d\n", interface->minor);
    }
    return retval;
}

static void tgamepad_disconnect(struct usb_interface *interface)
{
    usb_deregister_dev(interface, &class);
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
