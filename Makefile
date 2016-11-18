# Tic Tac Toe Makefile
# for building an out-of-tree Linux module.

SRC = ttt.c

ifneq ($(KERNELRELEASE),)
# Kbuild part
include Kbuild

else
# normal Makefile
# Debian requirements:
# $ sudo aptitude install linux-headers


# Red Hat 7 / CentOS 7
#KDIR = /usr/src/kernels/3.10.0-123.9.3.el7.x86_64

# User source on ithaqua
#KDIR = /work/linux-output-3.10

# Debian 7 / 8
KDIR = /lib/modules/$(shell uname -r)/build


default:
	make -C $(KDIR) M=$(PWD)

.PHONY: clean
clean:
	make -C $(KDIR) M=$(PWD) clean
	@rm -f tags

tags: $(SRC)
	@ctags $(SRC)

.PHONY: help
help:
	make -C $(KDIR) M=$(PWD) help

# Example using $(value ...) to display variable contents
.PHONY: path
path:
	@echo "PATH = $(value PATH)"
	@echo "SRC = $(value SRC)"

endif
# END OF MAKEFILE

