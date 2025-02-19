# Kernel build path (this assumes you are working inside the kernel tree)
KERNEL_DIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Kernel module name
obj-m := ioctl.o

# The object files to be linked into the module (make sure the source files are distinct)

# Target to build the kernel module
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

# Clean the generated files
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
