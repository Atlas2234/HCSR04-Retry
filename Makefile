obj-m += hcsr04_dev.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: modules test

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

test: test.c
	$(CC) -Wall -Wextra -O2 -o test test.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
