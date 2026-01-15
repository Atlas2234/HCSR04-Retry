/* Linux kernel headers */

// Linux kernel headers for creating a kernel module
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

// Linux kernel headers for using GPIO pins in our driver
#include <linux/gpio/consumer.h>

// Linux kernel headers for kernel time calculations
#include <linux/ktime.h>

// Linux kernel headers for delay/sleep
#include <linux/delay.h>


//Linux kernel headers for passing to user space
#include <linux/uaccess.h>

// Linux kernel headers for error codes
#include <linux/errno.h>


/* Global kernel module variables  */

// The major number which will be allocated by our kernel module
static int major;

// Lock variable to prevent multiple access to device
static int hcsr04_lock = 0;

// Two gpio descriptor structs for the pins we want to use
static struct gpio_desc *echo, *trigger;

#define GPIO_OUT	20	// GPIO20 trigger pin
#define GPIO_IN		21	// GPIO21 echo pin

#define GPIO_OFFSET	512	// GPIO offset number

// Variables for kernel time calculations
static ktime_t rising, falling;

// Vairable for HCSR04 write timeout
#define ECHO_TIMEOUT_US 20000 // 20ms


/* Kernel Module Functions */
// File operation callback functions for our character device
static int hcsr04_open(struct inode *inode, struct file *file) {
	int ret = 0;
	pr_info("hcsr04_dev - Open is called\n");

	//Check if device is locked
	if (hcsr04_lock > 0) {
		ret = -EBUSY;
	}
	// Lock device if not locked
	else
		hcsr04_lock++;
	
	return ret;
}

// Close callback function for our character device
static int hcsr04_close(struct inode *inode, struct file *filp) {
	pr_info("hcsr04_dev - Close is called\n");

	// Unlock device when closed
	hcsr04_lock = 0;

	return 0;
}

// Read callback function for our character device
static ssize_t hcsr04_read(struct file *filp, char __user *user_buf, size_t len, loff_t *off) {
	int ret;
	int pulse;

	pr_info("hcsr04_dev - Read is called\n");

	// Calculate pulse width in microseconds
	pulse = (int)ktime_to_us(ktime_sub(falling, rising));
	pr_info("hcsr04_dev - Pulse width is %d\n", pulse);

	// Copy pulse width to user space
	ret = copy_to_user(user_buf, &pulse, 4);

	return 4;
}

// Write callback function for our character device
static ssize_t hcsr04_write(struct file *filp, const char __user *user_buf, size_t len, loff_t *off) {
	pr_info("hcsr04_dev - Write is called\n");
	
	// Variable for timeouts
	ktime_t t0;

	// Send 10us pulse to trigger pin
	// gpiod_set_value(trigger, 0);
	gpiod_set_value(trigger, 1);
	udelay(10);
	// msleep(30);
	gpiod_set_value(trigger, 0);
	//msleep(30);
	
	// Wait for eacho pin to go high and then low
	// Rising edge
	t0 = ktime_get();
	while(gpiod_get_value(echo) == 0) {
		if (ktime_us_delta(ktime_get(), t0) > ECHO_TIMEOUT_US)
			return -ETIMEDOUT;
		cpu_relax();
	}
	// Record time of rising edge
	rising = ktime_get();
	pr_info("hchsr04_dev - Rising timestamp is %d\n", (int)ktime_to_us(rising));

	// Falling edge
	t0 = rising;
	while(gpiod_get_value(echo) == 1){
		if (ktime_us_delta(ktime_get(), t0) > ECHO_TIMEOUT_US)
			return -ETIMEDOUT;
		cpu_relax();
	}
	// Record time of falling edge
	falling = ktime_get();
	pr_info("hchsr04_dev - Falling timestamp is %d\n", (int)ktime_to_us(falling));

	return 1;
}

// File operations structure defining the character device's operations
static struct file_operations fops = {
	.open = hcsr04_open,
	.release = hcsr04_close,
	.read = hcsr04_read,
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
	pr_info("hcsr04_dev - Registered character device with major number %d\n", major);
	
	return 0;
}

/* Kernel module exit function */
static void __exit hcsr04_module_exit(void) {
	pr_info("hcsr04_dev - Cleaning up hcsr04_dev\n");
	
	hcsr04_lock = 0;

	// Unregister the character device using the major number and device name.
	// Delets the character device from the system and frees the device number.
	unregister_chrdev(major, "hcsr04_dev");

	// gpiod_set_value(trigger, 0);
}

// Register module entry and exit points
module_init(hcsr04_module_init);
module_exit(hcsr04_module_exit);

// Moddule information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Atlas");
MODULE_DESCRIPTION("A character device driver to interface with the HCSR04 sensor");


