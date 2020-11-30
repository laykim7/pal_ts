
GNU_TARGET = arm-linux-gnueabihf
GNU_DIR = /opt/pkg/petalinux20182/tools/linux-i386/gcc-arm-linux-gnueabi
GNU_CC = $(GNU_DIR)/bin/arm-linux-gnueabihf-gcc


export SHELL=/bin/bash
export ARCH=arm
export BUILDARCH:=$(shell uname -m)
export TARGET=arm-linux-gnueabihf
export CROSS_COMPILE=$(GNU_DIR)/bin/arm-linux-gnueabihf-


obj-m	:= mps_pico.o


# BIST removed because it need to be updated with DMA2
mps_pico-objs := 		\
	mps_pico_main.o 	\
	mps_pico_char.o 	\
	mps_pico_dma.o

KERNEL_SRC ?= /home/ctrluser/wpl/zq900/build/tmp/work/plnx_zynq7-xilinx-linux-gnueabi/linux-xlnx/4.14-xilinx-v2018.2+gitAUTOINC+ad4cd988ba-r0/linux-plnx_zynq7-standard-build
SRC := $(shell pwd)
PERL := perl

all: default

default:
	svn update
	rm -f mps_pico_version.h
	$(PERL) genVersionHeader.pl -v -t . -N MPS_PICO_VERSION $(PWD)/mps_pico_version.h
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules

ifdef msg
	svn ci -m "$(msg)"
else
	svn ci -m "-"
endif
	
debug:
	KCPPFLAGS="-DDEBUG_SYS=1 -DDEBUG_CHAR=1 -DDEBUG_DMA=1 -DDEBUG_IRQ=1 -DDEBUG_FULL=1" make all

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) clean
	rm -f mps_pico_version.h
	rm -f gen_py

install:
	echo "KERNEL==\"mps_pico*\", MODE=\"0666\"\n" > /etc/udev/rules.d/10-CAENels-AMC-Pico.rules
	udevadm control --reload-rules
	cp mps_pico.ko /lib/modules/$(shell uname -r)/extra
	echo mps_pico >> /etc/modules
	#echo "module mps_pico +p" > /sys/kernel/debug/dynamic_debug/control
	depmod -a
	modprobe mps_pico
	echo "Installation done"

uninstall:
	modprobe -r mps_pico
	rm /etc/udev/rules.d/10-CAENels-AMC-Pico.rules
	udevadm control --reload-rules
	rm /lib/modules/$(shell uname -r)/extra/mps_pico.ko
	sed -i '/mps_pico/d' /etc/modules
	depmod -a
	echo "Driver uninstalled"

gen_py: gen_py.c mps_pico.h mps_pico_version.h
	$(GNU_CC) -o $@ -g -Wall $<

mps_pico_version.h:
	$(PERL) genVersionHeader.pl -t . -N MPS_PICO_VERSION $(PWD)/mps_pico_version.h

test/picodefs.py: gen_py
	./$< $@
