
GNU_TARGET = arm-linux-gnueabihf
GNU_DIR = /opt/pkg/petalinux20182/tools/linux-i386/gcc-arm-linux-gnueabi
GNU_CC = $(GNU_DIR)/bin/arm-linux-gnueabihf-gcc



export SHELL=/bin/bash
export ARCH=arm
export BUILDARCH:=$(shell uname -m)
export TARGET=arm-linux-gnueabihf
export CROSS_COMPILE=$(GNU_DIR)/bin/arm-linux-gnueabihf-

obj-m := ts2ip.o

KERNEL_SRC ?= /home/ctrluser/wpl/zq900/build/tmp/work/plnx_zynq7-xilinx-linux-gnueabi/linux-xlnx/4.14-xilinx-v2018.2+gitAUTOINC+ad4cd988ba-r0/linux-plnx_zynq7-standard-build
SRC := $(shell pwd)

all: default

default:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules


clean:
	rm -f *.o *~ core .depend .*.cmd *.ko *.mod.c
	rm -f Module.markers Module.symvers modules.order
	rm -rf .tmp_versions Modules.symvers
