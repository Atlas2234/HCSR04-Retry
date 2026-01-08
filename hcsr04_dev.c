/* Linux kernel headers */

// Linux kernel headers for creating a kernel module
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>


/* Global kernel module variables  */

// The major number which will be allocated by our kernel module
static int major;

// File operations structure defining the character device's operations
static struct file_operations fops = {};


/* Kernel module initialization function */
static int __init hcsr04_module_init(void) {
	// Allocation of the devie number, creates a character device, and links device number to character device.
	major = register_chrdev(0, "hcsr04_dev", &fops);

	// Check if the major number was successfully allocated. If the major number is less than 0, then an error has occurred.
	if (major < 0) {
		// Log an error message to the kernel log
		pr_err("hcsr04_dev - Failed to register character device\n");
		// Return the error code which is the invalid major number.
		return major;
	}

	// Log a message to the kernel log indicating successful registration
	pr_info("hcsr04_dev - Registered character device with ajor number %d\n", major);

	return 0;
}

/* Kernel module exit function */
static void __exit hcsr04_module_exit(void) {
	// Unregister the character device using the major number and device name.
	// Delets the character device from the system and frees the device number.
	unregister_chrdev(major, "hcsr04_dev");
}

// Register module entry and exit points
module_init(hcsr04_module_init);
module_exit(hcsr04_module_exit);

// Moddule information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Atlas");
MODULE_DESCRIPTION("A character device driver to interface with the HCSR04 sensor");


