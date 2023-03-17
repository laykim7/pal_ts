PCI Driver init and exit Functions:
===================================

function called at driver initialization

```C
static int __init init_function(void) 
{
    mps_pico8_class = class_create(THIS_MODULE, MOD_NAME);
    return pci_register_driver(&pci_driver);
}
```
relative macro
```C
module_init(init_function);
```

```C
static void __exit exit_function(void) 
{
    pci_unregister_driver(&pci_driver);
    class_destroy(mps_pico8_class);
}
```
relative macro
```C
module_exit(exit_function);
```

Secondary Functions:
====================

After the init function, the first function to be called is the probe() function.

```C
static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
```
Actually enable the device
```C
    pci_enable_device(dev);
```
    
Mark all PCI regions associated with PCI device pdev as being reserved by owner res_name. // Do not access any address inside the PCI regions unless this call returns successfully.
Returns 0 on success, or EBUSY on error. A warning message is also printed on failure. 
```C
    pci_request_regions(dev, DRV_NAME);
```
map bars
```C
    board->bar0 = pci_ioremap_bar(dev, 0);
    board->bar2 = pci_ioremap_bar(dev, 2);
```
enables bus-mastering for device dev
```C
    pci_set_master(dev);
```
inform the kernel the addresses that the device (FPGA) can address in this case using a macro
```C
    pci_set_dma_mask(dev, DMA_BIT_MASK(32));
```
allocate memory for the 8 buffers (8 channels)
for each buffer allocate DMA_BUF_SIZE = 4 MByte
```C
    for (i = 0; i < DMA_BUF_COUNT; i++) 
    {
```
cpu_addr = pci_alloc_consistent(dev, size, &dma_handle);
This routine will allocate RAM for that region, so it acts similarly to __get_free_pages (but takes size instead of a page order).  If your driver needs regions sized smaller than a page, you may prefer using the pci_pool interface, described below.

It returns two values: the virtual address which you can use to access it from the CPU and dma_handle which you pass to the card.

The cpu return address and the DMA bus master address are both guaranteed to be aligned to the smallest PAGE_SIZE order which is greater than or equal to the requested size.  This invariant exists (for example) to guarantee that if you allocate a chunk which is smaller than or equal to 64 kilobytes, the extent of the buffer you receive will not cross a 64K boundary.
```C
        board->kernel_mem_buf[i] = pci_alloc_consistent(dev, DMA_BUF_SIZE, &board->dma_buf[i]);
    }
```
if requested, enable MSI interrupts
```C
    if (board->irqmode==dmac_irq_msi) {
```
Setup the MSI capability structure of device function with a single MSI vector upon its software driver call to request for MSI mode enabled on its hardware device function. A return of zero indicates the successful setup of an entry zero with the new MSI vector or non−zero for otherwise
```C
        ret = pci_enable_msi(dev);
    }
```
if requested, allocate an interrupt line
```C
    if (board->irqmode!=dmac_irq_poll) {
```C
> dev->irq: Interrupt line to allocate
> &amc_isr: Function to be called when the IRQ occurs
> 0: Interrupt type flags
> "pico_acq": An ascii name for the claiming device
> board: A cookie passed back to the handler function
```C
        ret = request_irq(dev->irq, &amc_isr, 0, "pico_acq", board);
    }
```
At this point when an interrupt is generated the driver will call the amc_isr() function
At this point the probe() function is not completed, it remains to setup the CHAR device

```C
ret = sysfs_create_groups(&dev->dev.kobj, pico_groups);
```


```C
ret = alloc_chrdev_region(&board->cdevno, 0, 1, MOD_NAME);
```


```C
cdev_init(&board->cdev, &mps_pico_fops);
board->cdev.owner = THIS_MODULE;
```


```C
ret = cdev_add(&board->cdev, board->cdevno, 1);
```


```C
cdev = device_create(mps_pico8_class, &dev->dev, board->cdevno,
                         NULL, MOD_NAME "_%s", pci_name(dev));
ret = -ENOMEM;
```


```C
}
```

At this point we can define char_open, char_release, char_read, ecc..




Relevant Data Structures:
=========================

this structure indicates all the devices that are supported by the driver
```C
static const struct pci_device_id ids[] = {
	{ .vendor = PCI_VENDOR_ID_XILINX, .device = 0x0007,
	  .subvendor = MPS_PICO_SUBVENDOR_ID, .subdevice = MPS_PICO_SUBDEVICE_ID
	},
	{ 0, }
};
```

```C
static struct pci_driver pci_driver = {
    .name = MOD_NAME,   // unique name that identifies the driver
	.id_table = ids,    // pointer to the pci_device_id structure
	.probe = probe,     // pointer to the probe() function
	.remove = remove,   // pointer to the remove() function
};
```

```C
 enum dmac_irqmode_t irqmode;
```

> 0 - polled  (debugging)
> 1 - classic PCI level IRQ
> 2 - PCI MSI





CHAR Driver Functions:
======================




Secondary Functions:
====================



Relevant Data Structures:
=========================

```C
const struct file_operations mps_pico_fops = {
	.owner		= THIS_MODULE,
	.open		= char_open,     // pointer to the function called when opening device file
	.release	= char_release,  // pointer to the function called when closing device file
	.read		= char_read,     // pointer to the function called when reading from device file
    .write      = char_write,    // pointer to the function called when writing to device file
    .llseek     = char_llseek,
	.unlocked_ioctl = char_ioctl
};
```