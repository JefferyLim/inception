#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/string.h>
#include <linux/init.h>

static unsigned long kernel_load_address = 0xffffffff81000000;
module_param(kernel_load_address, ulong, 0);
MODULE_PARM_DESC(kernel_load_address, "Kernel load address (for debugging)");

#define SECRET_SIZE 4096
static char secret[SECRET_SIZE];

// Buffer to receive a user-supplied secret string
static char *secret_input = NULL;
module_param(secret_input, charp, 0);
MODULE_PARM_DESC(secret_input, "Secret string to store in kernel memory");

static int __init my_init(void)
{
    size_t len;

    if (secret_input) {
        len = strnlen(secret_input, SECRET_SIZE - 1);
        memset(secret, 0, SECRET_SIZE);
        memcpy(secret, secret_input, len);
        printk(KERN_INFO "Secret copied (%zu bytes)\n", len);
    } else {
        memset(secret, 'X', SECRET_SIZE);
        printk(KERN_INFO "No secret string provided, filled with 'X'\n");
    }

    printk(KERN_INFO "secret_ptr: %px\n", secret);
    return 0;
}

static void __exit my_exit(void)
{
    // Optionally zero out secret before module unload
    memset(secret, 0, SECRET_SIZE);
    printk(KERN_INFO "Secret memory cleared\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("JefferyLim");
MODULE_DESCRIPTION("Kernel module with configurable secret string");
