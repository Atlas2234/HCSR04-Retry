/* Linux kernel headers */

// Linux kernel headers for creating a kernel module
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

// Linux kernel headers for using GPIO pins in our driver
#include <linux/gpio/consumer.h>

/* Global kernel module variables  */

// The major number which will be allocated by our kernel module
static int major;



// Two gpio descriptor structs for the pins we want to use
static struct gpio_desc *echo, *trigger;

#define GPIO_OUT	20	// GPIO20 trigger pin
#define GPIO_IN		21	// GPIO21 echo pin

#define GPIO_OFFSET	512	// GPIO offset number

ssize_t hcsr04_write(struct file *filp, const char __user *buffer, size_t len, loff_t *off) {
	pr_info("hcsr04_dev - Write is called");

	// Set gpio value
	gpiod_set_value(trigger, 1);

	return 0;

}

// File operations structure defining the character device's operations
static struct file_operations fops = {
	.write = hcsr04_write
};

/* Kernel module initialization function */
static int __init hcsr04_module_init(void) {
	// Allocation of the devie number, creates a character device, and links device number to character device.
	major = register_chrdev(0, "hcsr04_dev", &fops);
	
	// A status variable to save error codes
	int status;

	// Check if the major number was successfully allocated. If the major number is less than 0, then an error has occurred.
	if (major < 0) {
		// Log an error message to the kernel log
		pr_err("hcsr04_dev - Failed to register character device\n");
		// Return the error code which is the invalid major number.
		return major;
	}
	
	// Convert trigger and echo GPIO numbers to it's descriptor
	trigger = gpio_to_desc(GPIO_OUT + GPIO_OFFSET);
	if (!trigger) {
		pr_err("hcsr04_dev - Error getting pin 20\n");
		return -ENODEV;
	}

	echo = gpio_to_desc(GPIO_IN + GPIO_OFFSET);
	if (!echo) {
		pr_err("hcsr04_dev - Error getting pin 21\n");
		return -ENODEV;
	}

	// Set
	status = gpiod_direction_output(trigger, 0);
	if (status) {
		pr_err("hcsr04_dev = Error setting pin 20 to output\n");
		return status;
	}

	status = gpiod_direction_input(echo);
	if (status) {
		pr_err("hcsr04_dev = Error setting pin 21 to intput\n");
		return status;
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
	
	gpiod_set_value(trigger, 0);
}

// Register module entry and exit points
module_init(hcsr04_module_init);
module_exit(hcsr04_module_exit);

// Moddule information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Atlas");
MODULE_DESCRIPTION("A character device driver to interface with the HCSR04 sensor");


