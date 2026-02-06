#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/serial.h>

#define DEVICE_NAME "example_serial"
#define SERIAL_PORT "/dev/ttyACM0" // Update this with your serial port

static int major;
static char *message;
static ssize_t message_size = 0;
static struct file *serial_file;

// Open function for the device
static int dev_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "example_serial: Device opened\n");
    return 0;
}

// Read function for the device
static ssize_t dev_read(struct file *file, char __user *buffer, size_t len, loff_t *offset) {
    ssize_t ret;

    // Ensure we do not exceed the buffer size
    if (*offset >= message_size)
        return 0;

    if (len > message_size - *offset)
        len = message_size - *offset;

    ret = copy_to_user(buffer, message + *offset, len);
    if (ret == 0) {
        *offset += len;
        printk(KERN_INFO "example_serial: Sent %zu characters to the user\n", len);
        return len;
    } else {
        printk(KERN_ERR "example_serial: Failed to send %zu characters to the user\n", ret);
        return -EFAULT;
    }
}

// Write function for the device
static ssize_t dev_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset) {
    ssize_t ret;

    if (len > message_size) {
        kfree(message);
        message = kmalloc(len, GFP_KERNEL);
        if (!message) {
            printk(KERN_ERR "example_serial: Failed to allocate memory\n");
            return -ENOMEM;
        }
        message_size = len;
    }

    ret = copy_from_user(message, buffer, len);
    if (ret == 0) {
        printk(KERN_INFO "example_serial: Received %zu characters from the user\n", len);
        return len;
    } else {
        printk(KERN_ERR "example_serial: Failed to receive %zu characters from the user\n", ret);
        return -EFAULT;
    }
}

// File operations structure
static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
};

// Module initialization function
static int __init example_serial_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "example_serial: Failed to register a major number\n");
        return major;
    }

    message = kmalloc(1024, GFP_KERNEL);
    if (!message) {
        unregister_chrdev(major, DEVICE_NAME);
        printk(KERN_ALERT "example_serial: Failed to allocate memory\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "example_serial: Registered with major number %d\n", major);

    // Open the serial port
    serial_file = filp_open(SERIAL_PORT, O_RDWR | O_NOCTTY | O_NDELAY, 0);
    if (IS_ERR(serial_file)) {
        printk(KERN_ERR "example_serial: Failed to open serial port\n");
        unregister_chrdev(major, DEVICE_NAME);
        kfree(message);
        return PTR_ERR(serial_file);
    }

    printk(KERN_INFO "example_serial: Serial port %s opened\n", SERIAL_PORT);

    return 0;
}

// Module exit function
static void __exit example_serial_exit(void) {
    if (serial_file) {
        filp_close(serial_file, NULL);
    }
    kfree(message);
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "example_serial: Unregistered device\n");
}

module_init(example_serial_init);
module_exit(example_serial_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("example_serial device driver");
MODULE_AUTHOR("Mr. Anonymous");

