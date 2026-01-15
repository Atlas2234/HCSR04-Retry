obj-m += hcsr04_dev.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

# --- MAP files ---
# User-space program map
TEST_MAP := test.map

# Kernel module map (map of the final .ko link)
MODULE_MAP := hcsr04_dev.ko.map
# Kbuild reads EXTRA_LDFLAGS for module link flags
EXTRA_LDFLAGS += -Map=$(PWD)/$(MODULE_MAP)

.PHONY: all modules test clean

all: modules test

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

test: test.c
	$(CC) -Wall -Wextra -O2 -Wl,-Map=$(TEST_MAP) -o test test.c

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(RM) -f test $(TEST_MAP) $(MODULE_MAP)
