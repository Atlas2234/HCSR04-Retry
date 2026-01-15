#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char **argv) {
	// Set application and character device names
	char *app_name = argv[0];
	char *dev_name = "/dev/hcsr04_dev";
	int fd = -1;
	char c;
	int d;

	// Open character device
	if((fd = open(dev_name, O_RDWR)) < 0) {
		fprintf(stderr, "%s: unable to open %s: %s\n", app_name, dev_name, strerror(errno));
		return 1;
	}

	c = 1;
	// Write to device to trigger measurement
	ssize_t n = write(fd, &c, 1);
	if (n < 0) {
		if(errno == ETIMEDOUT) {
			fprintf(stderr, "Measurement timed out (ETIMEDOUT)\n");
		} else {
			fprintf(stderr, "write failed: %s\n", strerror(errno));
		}
		close(fd);
		return 1;
	}
	
	// Read measurement result from device
	read(fd, &d, 4);

	// Print distance in cm (pulse width divided by 58)
	printf("Pulse width is %d. Distance is %fcm\n", d, d/58.0);

	//Close character device
	close(fd);

	return 0;
}
