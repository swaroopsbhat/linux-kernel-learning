#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h> // For copy_from_user and copy_to_user
#include <linux/slab.h>    // For kmalloc and kfree

#define DEVICE_NAME "example_device"

static int major; // Major number for the device

// Open function for the device
static int dev_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "example_device: Device opened\n");
    return 0;
}

// Read function for the device
static ssize_t dev_read(struct file *file, char __user *buffer, size_t len, loff_t *offset) {
    char *kernel_buffer;
    ssize_t ret;

    // Allocate a buffer to hold data to be read
    kernel_buffer = kmalloc(len, GFP_KERNEL);
    if (!kernel_buffer) {
        printk(KERN_ERR "example_device: Failed to allocate memory\n");
        return -ENOMEM;
    }

    // Fill the kernel buffer with some data (for example, "Hello from kernel")
    snprintf(kernel_buffer, len, "Hello from kernel");

    // Copy data from kernel space to user space
    ret = copy_to_user(buffer, kernel_buffer, len);
    if (ret) {
        kfree(kernel_buffer);
        return -EFAULT;
    }

    kfree(kernel_buffer);
    return len;
}

// Write function for the device (just a placeholder in this example)
static ssize_t dev_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset) {
    char *kernel_buffer;
    ssize_t ret;

    // Allocate a buffer to hold data from user space
    kernel_buffer = kmalloc(len, GFP_KERNEL);
    if (!kernel_buffer) {
        printk(KERN_ERR "example_device: Failed to allocate memory\n");
        return -ENOMEM;
    }

    // Copy data from user space to kernel space
    ret = copy_from_user(kernel_buffer, buffer, len);
    if (ret) {
        kfree(kernel_buffer);
        return -EFAULT;
    }

    printk(KERN_INFO "example_device: Received from user: %s\n", kernel_buffer);

    kfree(kernel_buffer);
    return len;
}

// File operations structure
static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
};

// Module initialization function
static int __init example_device_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "example_device: Failed to register a major number\n");
        return major;
    }

    printk(KERN_INFO "example_device: Registered with major number %d\n", major);
    return 0;
}

// Module exit function
static void __exit example_device_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "example_device: Unregistered device\n");
}

module_init(example_device_init);
module_exit(example_device_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple example device driver");
MODULE_AUTHOR("Your Name");

