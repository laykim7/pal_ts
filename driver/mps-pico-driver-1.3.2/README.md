Build
=====

```sh
cp config.example .config
make mps_pico_version.h
make gen_py
make test/picodefs.py
make
```

Installation
============

* to install and load the driver:
```sh
sudo make install
```

* to uninstall:
```sh
sudo make uninstall
make clean
```

Build time configuration
========================

Several build time options are available.
See [config.example](config.example) for a full listing.
The provided config.example is a generally useful default.

Device Discovery
================

The simplest way for a user application to find a pico8 device
is to directly provide a character device path (```/dev/mps_pico_*```).

Several additional methods are available, though these depend on the
charactor device not having been re-named (manually or by udev).
Symlinks are ok though.

By PCI ID.  For example,  Bus 1, device 0, function 0 under the default domain (0000).
Look for a sub-directory matching ```/sys/bus/pci/devices/0000:01:00.0/mps_pico/mps_pico_*```.
The name of the final sub-directory will be the charactor device name.

By PCIe slot label (not necessarily a simple number).
Read all files ```/sys/bus/pci/slots/*/address``` to build a mapping between slot label
and PCI ID.
For example ```/sys/bus/pci/slots/1-2/address``` contains ```0000:01:00.0```
so slot "1-2" is device "0000:01:00.0".

Debugging
=========

To enable debug printing for kernels build with CONFIG_DYNAMIC_DEBUG,
after module is loaded.

echo "module mps_pico +p" > /sys/kernel/debug/dynamic_debug/control

See https://www.kernel.org/doc/Documentation/dynamic-debug-howto.txt

sudo sh -c "cat /sys/kernel/debug/dynamic_debug/control | tee /tmp/dynamic_log.log"

ABI
===

One chardev is created for each pico8 card.
By default this is named with the PCI identifier as
for example ```/dev/mps_pico_0000:03:00.0```.

read()
------

This device may be read(), which arms the card
for acquisition, and returns when the requested number
of samples have been received.

The read() buffer size should be a multiple 32 bytes,
which is one 4 byte sample from each of eight channels.

A read will block until the acquisition logic is triggered,
or until ```ioctl(..., ABORT_READ)``` is issued (see ioctl section).

Returns the number of bytes read, or sets ```errno==ECANCELED```.

```errno==ERESTARTSYS``` may also be encountered if the syscall is
interrupted for other reasons.

Only one concurrent read() is allowed on each device.

ioctl()
-------

The header [mps_pico.h](mps_pico.h) defines several
macros for use with ioctl().

```
uint32_t ver = 0;
ioctl(fd, GET_VERSION, &ver);
if(ver!=GET_VERSION_CURRENT)
{ /* oops, kernel module version is different.  Can't proceed */ }
```

The GET_VERSION ioctl() should be the first one issued.
The result should be compared with GET_VERSION_CURRENT,
and/or other versions supported,
to determine if a compatible kernel module is loaded.

```
ioctl(fd, ABORT_READ);
```

Cause read() to return w/ errno==ECANCELED.
If a read() is in progress it returns immediately.
If no read() is in progress, then the next read() will return immediately.

After one read() has returned with errno==ECANCELED, subsequent read() calls
will block as normal.


```
SET_RANGE
GET_RANGE
SET_FSAMP
GET_FSAMP
SET_TRG
SET_RING_BUF
SET_GATE_MUX
SET_CONV_MUX
GET_THRESHOLD_FLAG
GET_THRESHOLD
SET_THRESHOLD
GET_NSAMP
SET_NSAMP
GET_MPS_CONFIG
SET_MPS_CONFIG
SET_MPS_CTRL
```

This provides a means for user applications
to detect and make use of custom firmware features.

Dump Measurements
=================

./dump_meas --devfile /dev/mps_pico_0000:03:00.0 --nrsamp 1024 --out meas10.csv
./dump_meas --devfile /dev/mps_pico_0000:05:00.0 --nrsamp 1024 --out meas11.csv

output should be something like:
> Read time: 1035 micro seconds
