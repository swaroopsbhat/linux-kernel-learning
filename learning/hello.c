#include<linux/module.h>
#include<linux/kernel.h>

static int __init my_module_init(void){
	printk(KERN_INFO "KERNEL MODULE LOADED");
return 0;
}


static void __exit my_module_exit(void){
	printk(KERN_INFO "KERNEL MODULE UNLOADED");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SWAROOPS");
MODULE_DESCRIPTION("SIMPLE_KERNEL_MODULE");

