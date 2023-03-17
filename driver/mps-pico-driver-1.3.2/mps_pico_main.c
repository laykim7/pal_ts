/*
 * MPS-Pico8 Linux Driver
 *
 *  Copyright (C) 2018 CAEN ELS s.r.l.
 *
 *  Paolo Scarbolo <p.scarbolo@caenels.com>
 *
 *  Copyright 2016 Board of Trustees of Michigan State University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file
 * \brief Register the module with PCIe subsytem
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/dma-mapping.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <asm/ioctl.h>

#include "mps_pico.h"
#include "mps_pico_regs.h"
#include "mps_pico_dma.h"
#include "mps_pico_char.h"
#include "mps_pico_bist.h"
#include "mps_pico_version.h"

#define DRV_NAME "MPS-PICO8 Driver"


#if LINUX_VERSION_CODE<KERNEL_VERSION(3,12,0)

#define __ATTRIBUTE_GROUPS(_name)				\
static const struct attribute_group *_name##_groups[] = {	\
    &_name##_group,						\
    NULL,							\
}

#define ATTRIBUTE_GROUPS(_name)					\
static const struct attribute_group _name##_group = {		\
    .attrs = _name##_attrs,					\
};								\
__ATTRIBUTE_GROUPS(_name)

static inline int sysfs_create_groups(struct kobject *kobj, const struct attribute_group **groups)
{
    int error = 0;
    int i;

    if (!groups)
        return 0;

    for (i = 0; groups[i]; i++) {
        error = sysfs_create_group(kobj, groups[i]);
        if (error) {
            while (--i >= 0)
                sysfs_remove_group(kobj, groups[i]);
            break;
        }
    }
    return error;
}

static inline void sysfs_remove_groups(struct kobject *kobj, const struct attribute_group **groups)
{
    int i;

    if (!groups)
        return;
    for (i = 0; groups[i]; i++)
        sysfs_remove_group(kobj, groups[i]);
}
#endif /* LINUX_VERSION_CODE<KERNEL_VERSION(3,16,0) */

static
int version[3] = {1, 3, 3};

static
struct class *mps_pico8_class;

/* allow DMA buffer size to be selected at load time.
 * May be reduced for testing.
 * Increasing this will at some point cause allocation failures
 * in probe().
 * The limit will be host specific.
 */
static
ulong damc_req_dma_buf_len = 4*1024*1024;
module_param_named(dma_buf_len, damc_req_dma_buf_len, ulong, 0444);

unsigned long damc_dma_buf_len;

/* 0 - polled  (debugging)
 * 1 - classic PCI level IRQ
 * 2 - PCI MSI
 */
uint dmac_irqmode = 2;
module_param_named(irqmode, dmac_irqmode, uint, 0444);

/* select source of site specific FW customization. */
static
char *dmac_site_name =
#ifdef CONFIG_MPS_PICO_SITE_DEFAULT
        CONFIG_MPS_PICO_SITE_DEFAULT
#else
        NULL
#endif
        ;
uint32_t dmac_site;
module_param_named(site, dmac_site_name, charp, 0444);


/** List of devices this driver recognizes */
static const struct pci_device_id ids[] = {
	{ .vendor = PCI_VENDOR_ID_XILINX, .device = 0x0007,
	  .subvendor = MPS_PICO_SUBVENDOR_ID, .subdevice = MPS_PICO_SUBDEVICE_ID
	},
	{ 0, }
};

MODULE_DEVICE_TABLE(pci, ids);


static
void calib_cycles(uint64_t *cycles, uint64_t *nano)
{
    struct timespec tA, tB;
    cycles_t cA, cB;

    tA = current_kernel_time();
    cA = get_cycles();

    msleep(10);

    tB = current_kernel_time();
    cB = get_cycles();

    *cycles = cB-cA;
    tA = timespec_sub(tB, tA);
    *nano = timespec_to_ns(&tA);
}

// This function is called to handle an interrupt (dmac_irq_msi)
// thus it's called when the DMA transfer is completed (asserted by the DMA engine in the FPGA)
irqreturn_t amc_isr(int irq, void *dev_id)
{
    cycles_t tstart;
    struct board_data *board;
    uint32_t active;

    tstart = get_cycles();

    board = (struct board_data *)dev_id;
    dev_dbg(&board->pci_dev->dev, "Inside ISR irq: 0x%x\n", irq);

    WARN_ONCE(board==NULL, "mps_pico ISR had board==NULL\n");
    if (board == NULL)
        return IRQ_NONE;

    // verify the actual presence of an interrupt in the interrupt latch on FPGA reg
    active = ioread32(board->bar0+INTR_LATCH);
    dev_dbg(&board->pci_dev->dev, "IRQ in MSI mode %08x\n", (unsigned)active);
    if (unlikely(active&~INTR_MASK)) 
    {
        /* Maybe some new FW feature has signaled an interrupt we don't know
         * how to handle, and can't mask out.
         * So clear it and hope for the best...
         */
        dev_warn(&board->pci_dev->dev, "Device signaling unknown IRQ %08x\n", (unsigned)active);
    }

    // if latch not active -> signal spurious IRQ and exit
    if (!active) 
    {
        if (unlikely(board->irqmode==dmac_irq_msi)) 
        {
            dev_warn(&board->pci_dev->dev, "Spurious IRQ in MSI mode %08x, irq: 0x%x\n", (unsigned)active, irq);
        }
        return IRQ_NONE;
    }

    if (active & INTR_DMA_DONE) // check interrupt generated by DMA(1) buffer 1 MHz
    {
        size_t nsent = 0;
        unsigned long flags;
        unsigned cycles = 0;
        int op = 1;

        // check number of elements in the response queue (axi_dma)
        uint32_t count = (ioread32(board->bar0 + DMA_ADDR + DMA_OFFSET_STATUS) >> 16) & 0x7FF;
        //dev_dbg(&board->pci_dev->dev, "ISR: irq: 0x%x %u\n", irq, (unsigned)count);
        dev_dbg(&board->pci_dev->dev, "ISR (DMA1): irq: 0x%x %u\n", irq, (unsigned)count);

        if ( unlikely(count==0) ) 
        {
            dev_warn(&board->pci_dev->dev, "DMA DONE w/ response fifo empty\n");
        } 
        else 
        {
            while (count > 0) 
            {
                if ( unlikely(count == 0xFFFFFFFFUL) ) 
                {
                    dev_err(&board->pci_dev->dev, "something wrong when reading from DMA\n");
                    break;

                } 
                else if ( unlikely(cycles++ > 100) ) 
                {
                    dev_err(&board->pci_dev->dev, "FIFO ran away, stopping\n");
                    op = 2;
                    break;
                }

                // check DMA response tranfer length
                nsent += ioread32(board->bar0 + DMA_ADDR + DMA_OFFSET_RESP_LEN);
                dev_dbg(&board->pci_dev->dev, "   ISR (DMA1): resp count: %08x\n", count);
                dev_dbg(&board->pci_dev->dev, "   ISR (DMA1): resp len: %08x\n", (unsigned)nsent);
                dev_dbg(&board->pci_dev->dev, "   ISR (DMA1): resp addr: %08x\n",
                        ioread32(board->bar0 + DMA_ADDR + DMA_OFFSET_RESP_ADDR));

                /* pop from axi_dma resp fifo */
                iowrite32(0, board->bar0 + DMA_ADDR + DMA_OFFSET_RESP_LEN);
                // mb() function guarantees that every read/write operation are concluded 
                mb();
                // check again number of elements in the response queue (axi_dma)
                // this should be -1 than before
                count = (ioread32(board->bar0 + DMA_ADDR + DMA_OFFSET_STATUS) >> 16) & 0x7FF;
            }

            // once the response queue is empty
            // safely update board-dma variables
            // by laykim. 2019.08.21
            //spin_lock_irqsave(&board->dma_queue.lock, flags);
            board->dma1_irq_flag = op;
            board->dma1_bytes_trans = nsent;
            wake_up_locked(&board->dma_queue);
            // by laykim. 2019.08.21
            //spin_unlock_irqrestore(&board->dma_queue.lock, flags);

            dev_dbg(&board->pci_dev->dev, "ISR (DMA1): waked up dma_queue\n");
        }
        iowrite32(INTR_DMA_DONE, board->bar0+INTR_CLEAR);
    }    
    else if (active & INTR_DMA2_DONE) // check interrupt generated by DMA(2) buffer 1 kHz
    {
        size_t nsent = 0;
        unsigned long flags;
        unsigned cycles = 0;
        int op = 1;

        // check number of elements in the response queue (axi_dma)
        uint32_t count = (ioread32(board->bar0 + DMA2_ADDR + DMA_OFFSET_STATUS) >> 16) & 0x7FF;
        //dev_dbg(&board->pci_dev->dev, "ISR (DMA2): irq: 0x%x %u\n", irq, (unsigned)count);
        dev_dbg(&board->pci_dev->dev, "ISR (DMA2): irq: 0x%x %u\n", irq, (unsigned)count);

        if ( unlikely(count==0) ) 
        {
            dev_warn(&board->pci_dev->dev, "DMA2 DONE w/ response fifo empty\n");
        } 
        else 
        {
            while (count > 0) 
            {
                if ( unlikely(count == 0xFFFFFFFFUL) ) 
                {
                    dev_err(&board->pci_dev->dev, "something wrong when reading from DMA2\n");
                    break;

                } 
                else if ( unlikely(cycles++ > 100) ) 
                {
                    dev_err(&board->pci_dev->dev, "DMA2 FIFO ran away, stopping\n");
                    op = 2;
                    break;
                }

                // check DMA response tranfer length
                nsent += ioread32(board->bar0 + DMA2_ADDR + DMA_OFFSET_RESP_LEN);
                dev_dbg(&board->pci_dev->dev, "   ISR (DMA2): resp count: %08x\n", count);
                dev_dbg(&board->pci_dev->dev, "   ISR (DMA2): resp len: %08x\n", (unsigned)nsent);
                dev_dbg(&board->pci_dev->dev, "   ISR (DMA2): resp addr: %08x\n",
                        ioread32(board->bar0 + DMA2_ADDR + DMA_OFFSET_RESP_ADDR));

                /* pop from axi_dma resp fifo */
                iowrite32(0, board->bar0 + DMA2_ADDR + DMA_OFFSET_RESP_LEN);
                // mb() function guarantees that every read/write operation are concluded 
                mb();
                // check again number of elements in the response queue (axi_dma)
                // this should be -1 than before
                count = (ioread32(board->bar0 + DMA2_ADDR + DMA_OFFSET_STATUS) >> 16) & 0x7FF;
            }

            // once the response queue is empty
            // safely update board-dma variables
            // by laykim. 2019.08.21
            //spin_lock_irqsave(&board->dma_queue.lock, flags);
            board->dma2_irq_flag = op;
            board->dma2_bytes_trans = nsent;
            wake_up_locked(&board->dma_queue);
            // by laykim. 2019.08.21
            //spin_unlock_irqrestore(&board->dma_queue.lock, flags);

            dev_dbg(&board->pci_dev->dev, "ISR (DMA2): waked up dma_queue\n");
        }
        iowrite32(INTR_DMA2_DONE, board->bar0+INTR_CLEAR);
    }

    // who was active will now be cleared
    //active_before_clear = ioread32(board->bar0+INTR_LATCH);
    //dev_info(&board->pci_dev->dev, "IRQ active before clear %08x\n", (unsigned)active_before_clear);
    //iowrite32(active, board->bar0+INTR_CLEAR);

    {
        cycles_t tdelta = get_cycles() - tstart;

        ACCESS_ONCE(board->last_isr) = tdelta;
        if (tdelta > ACCESS_ONCE(board->longest_isr)) 
        {
            ACCESS_ONCE(board->longest_isr) = tdelta;
        }

        atomic_inc(&board->num_isr);
    }


    return IRQ_HANDLED;
}


static
int pico_pci_setup(struct pci_dev *dev, struct board_data *board)
{
#define ERR(COND, LBL, MSG, ...) if(COND) { dev_err(&dev->dev, MSG, ##__VA_ARGS__); goto LBL; }

    unsigned i;
    int ret;

    ret = pci_enable_device(dev);
    ERR(ret, done, "Failed to enable\n");

    ret = pci_request_regions(dev, DRV_NAME);
    ERR(ret, pcidisable, "Failed to configure BARs\n");

    ret = -EIO;

    board->bar0 = pci_ioremap_bar(dev, 0);
    ERR(!board->bar0, release, "Failed to map BAR0\n");

    board->bar2 = pci_ioremap_bar(dev, 2);
    ERR(!board->bar2, unmap0, "Failed to map BAR21\n");

    pci_set_master(dev);

    ret = pci_set_dma_mask(dev, DMA_BIT_MASK(32));
    if(!ret)
    {
        // if the ret == 0 try with the "consistent" function
        ret = pci_set_consistent_dma_mask(dev, DMA_BIT_MASK(32));
    }
    ERR(ret, unmap2, "Failed to set DMA masks\n");

    ret = -ENOMEM;
    // allocate RAM for DMA1 (1 MHz)
    for (i = 0; i < DMA_BUF_COUNT; i++) 
    {
        board->kernel_mem1_buf[i] = pci_alloc_consistent(dev, DMA_BUF_SIZE, &board->dma1_buf[i]);
        ERR(!board->kernel_mem1_buf[i], freebufs, "Failed to allocate DMA1 buffer %u\n", i);

        dev_dbg(&dev->dev, "DMA1 pci_alloc() virt addr: %p\tsize: %u, phys addr: 0x%08llx\n",
            board->kernel_mem1_buf[i], (unsigned)DMA_BUF_SIZE, board->dma1_buf[i]);
    }
    // allocate RAM for DMA2 (1 kHz)
    board->kernel_mem2_buf = pci_alloc_consistent(dev, DMA_BUF_SIZE, &board->dma2_buf);
    ERR(!board->kernel_mem2_buf, freebuf2, "Failed to allocate DMA2 buffer\n");

    dev_dbg(&dev->dev, "DMA2 pci_alloc() virt addr: %p\tsize: %u, phys addr: 0x%08llx\n",
        board->kernel_mem2_buf, (unsigned)DMA_BUF_SIZE, board->dma2_buf);


    if (board->irqmode==dmac_irq_msi) {
        ret = pci_enable_msi(dev);
        ERR(ret, freebuf2, "Failed to enable any MSI interrupts\n");
        
        // int status; 
        // status = pci_enable_msi_block(dev, 2);
        // if (status < 0) 
        // {
 		//     ERR(status, freebuf2, "Failed to enable any MSI interrupts\n");
        // }
        // else
        // {
        //     dev_info(&dev->dev, "MSIX range success, n irqs: %d\n", status);    
        // }
    }

    if (board->irqmode!=dmac_irq_poll) {
        ret = request_irq(dev->irq, &amc_isr, 0, "pico_acq", board);
        ERR(ret, msidisable, "Failed to attach acquire ISR(first irq)\n");
        // ret = request_irq(dev->irq+1, &amc_isr_dma2, 0, "pico_acq_dma2", board);
        // ERR(ret, msidisable, "Failed to attach acquire ISR(second irq)\n");
    }

    return 0;
//stopirq:
//    if (board->irqmode!=dmac_irq_poll) free_irq(dev->irq, &board);
msidisable:
    if (board->irqmode==dmac_irq_msi) pci_disable_msi(dev);
freebuf2:
    for (i = 0; i < 1; i++) 
    {
        if(!board->kernel_mem2_buf) continue;
            pci_free_consistent( dev,
                    DMA_BUF_SIZE,
                    board->kernel_mem2_buf,
                    board->dma2_buf);
    }
freebufs:
    for (i = 0; i < DMA_BUF_COUNT; i++) 
    {
        if(!board->kernel_mem1_buf[i]) continue;
        pci_free_consistent( dev,
                    DMA_BUF_SIZE,
                    board->kernel_mem1_buf[i],
                    board->dma1_buf[i]);
    }
unmap2:
    pci_iounmap(dev, board->bar2);
unmap0:
    pci_iounmap(dev, board->bar0);
release:
    pci_release_regions(dev);
pcidisable:
    pci_disable_device(dev);
done:
    return ret;
#undef ERR
}

static
int pico_pci_cleanup(struct pci_dev *dev, struct board_data *board)
{
    unsigned i;
    if (board->irqmode != dmac_irq_poll) 
    {
        free_irq(dev->irq, board);
        // free_irq(dev->irq+1, board);
    }
    if (board->irqmode == dmac_irq_msi) 
    {
        pci_disable_msi(dev);
    }
    for (i = 0; i < 1; i++) 
    {
        if(!board->kernel_mem2_buf) continue;
        pci_free_consistent( dev,
                DMA_BUF_SIZE,
                board->kernel_mem2_buf,
                board->dma2_buf);
    }
    for (i = 0; i < DMA_BUF_COUNT; i++) 
    {
        if(!board->kernel_mem1_buf[i]) continue;
        pci_free_consistent(	dev,
                    DMA_BUF_SIZE,
                    board->kernel_mem1_buf[i],
                    board->dma1_buf[i]);
    }

    pci_iounmap(dev, board->bar2);
    pci_iounmap(dev, board->bar0);

    pci_release_regions(dev);

    pci_disable_device(dev);
    return 0;
}

static
void pico_wait_for_op(struct board_data *board)
{
    // by laykim. 2019.08.21
    //spin_lock_irq(&board->dma_queue.lock);
    if(board->read1_in_progress) {
        board->dma1_irq_flag = 2;
        wake_up_locked(&board->dma_queue);
    }
    if(board->read2_in_progress) {
        board->dma2_irq_flag = 2;
        wake_up_locked(&board->dma_queue);
    }
    // by laykim. 2019.08.21
    //spin_unlock_irq(&board->dma_queue.lock);
}

static
ssize_t lastisr_show(struct device *dev, struct device_attribute *attr,
                     char *buf)
{
    struct board_data *board = dev_get_drvdata(dev);
    cycles_t value = ACCESS_ONCE(board->last_isr);
    return sprintf(buf, "%llu\n", (unsigned long long)value);
}

static
DEVICE_ATTR(lastisr, 0444, lastisr_show, NULL);

static
ssize_t numisr_store(struct device *dev, struct device_attribute *attr,
                     const char *buf, size_t count)
{
    struct board_data *board = dev_get_drvdata(dev);
    atomic_set(&board->num_isr, 0);
    return count;
}

static
ssize_t numisr_show(struct device *dev, struct device_attribute *attr,
                     char *buf)
{
    struct board_data *board = dev_get_drvdata(dev);
    unsigned num = atomic_read(&board->num_isr);
    return sprintf(buf, "%u\n", num);
}

static
DEVICE_ATTR(numisr, 0644, numisr_show, numisr_store);

static
ssize_t longestisr_store(struct device *dev, struct device_attribute *attr,
                         const char *buf, size_t count)
{
    struct board_data *board = dev_get_drvdata(dev);
    ACCESS_ONCE(board->longest_isr) = 0;
    return count;
}

static
ssize_t longestisr_show(struct device *dev, struct device_attribute *attr,
                     char *buf)
{
    struct board_data *board = dev_get_drvdata(dev);
    cycles_t num = ACCESS_ONCE(board->longest_isr);
    return sprintf(buf, "%lu\n", (unsigned long)num);
}

static
DEVICE_ATTR(longestisr, 0644, longestisr_show, longestisr_store);

static
ssize_t cyclescal_show(struct device *dev, struct device_attribute *attr,
                     char *buf)
{
    uint64_t cycles, nsec;
    calib_cycles(&cycles, &nsec);
    return sprintf(buf, "%lu cycles %lu ns\n",
                   (unsigned long)cycles, (unsigned long)nsec);
}

static
DEVICE_ATTR(cyclescal, 0444, cyclescal_show, NULL);

static
struct attribute * pico_attrs[] = {
    &dev_attr_lastisr.attr,
    &dev_attr_numisr.attr,
    &dev_attr_longestisr.attr,
    &dev_attr_cyclescal.attr,
    NULL
};
ATTRIBUTE_GROUPS(pico);

static
void pico_release(struct kobject *obj)
{
    struct board_data *board = container_of(obj, struct board_data, kobj);

    /* Free allocated memory */
#ifdef CONFIG_MPS_PICO_FRIB
    kfree(board->capture_buf);
#endif
    kfree(board);
}

static
struct kobj_type pico_ktype = {
    .release = &pico_release,
};

static
int pico_cdev_setup(struct pci_dev *dev, struct board_data *board)
{
#define ERR(COND, LBL, MSG, ...) if(COND) { dev_err(&dev->dev, MSG, ##__VA_ARGS__); goto LBL; }

    struct device *cdev;
    int ret;

    ret = sysfs_create_groups(&dev->dev.kobj, pico_groups);
    //ret = pico_attrs_setup(dev, board);
    ERR(ret, done, "Failed to add sysfs attrs\n");

    ret = alloc_chrdev_region(&board->cdevno, 0, 1, MOD_NAME);
    ERR(ret, unsysfs, "Failed to allocate chrdev number\n");

    cdev_init(&board->cdev, &mps_pico_fops);
    board->cdev.owner = THIS_MODULE;

    ret = cdev_add(&board->cdev, board->cdevno, 1);
    ERR(ret, cfree, "Failed to add chrdev\n")

    cdev = device_create(mps_pico8_class, &dev->dev, board->cdevno,
                         NULL, MOD_NAME "_%s", pci_name(dev));
    ret = -ENOMEM;
    ERR(IS_ERR(cdev), cdel, "Failed to allocate device\n");

    return 0;
//devdtor:
//    device_destroy(mps_pico8_class, board->cdevno);
cdel:
    cdev_del(&board->cdev);
    pico_wait_for_op(board);
cfree:
    unregister_chrdev_region(board->cdevno, 1);
unsysfs:
    sysfs_remove_groups(&dev->dev.kobj, pico_groups);
done:
    return ret;
#undef ERR
}

static
void pico_cdev_cleanup(struct pci_dev *dev, struct board_data *board)
{
    device_destroy(mps_pico8_class, board->cdevno);
    cdev_del(&board->cdev);
    pico_wait_for_op(board);
    unregister_chrdev_region(board->cdevno, 1);
    sysfs_remove_groups(&dev->dev.kobj, pico_groups);
}

/**
 * \brief Claims control of PCI device
 * \param dev   PCI device (bus, ...)
 * \param id    Device data (vendor, device, subvendor, subdevice...)
 * \return      0 on success, negative on fail
 */

static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    int ret;
    struct board_data *board = NULL;

    dev_info(&dev->dev, "probe() with slot '%s'\n", dev->slot ? pci_slot_name(dev->slot) : "<no slot>");

	/* Allocate memory for board structure */
	board = kzalloc(sizeof(struct board_data), GFP_KERNEL);
	if (!board) {
		return -ENOMEM;
	}

    ret = kobject_init_and_add(&board->kobj, &pico_ktype, &dev->dev.kobj, "pico_internal");
    if(ret) {
        kfree(board);
        return ret;
    }
    /* henceforth must call kobject_put(board) for cleanup */

	board->pci_dev = dev;
    board->irqmode = dmac_irqmode;
    if (board->irqmode > 2)
    {
        board->irqmode = 2;
    }

	/* store our data (like global variable) */
	dev_set_drvdata(&dev->dev, board);

    init_waitqueue_head(&board->dma_queue);

    ret = pico_pci_setup(dev, board);
    if (!ret) 
    {
        dev_info(&dev->dev, "FPGA HW version = %08x\n",
            ioread32(board->bar0 + PICO_ADDR + FPGA_VER_OFFSET));
        dev_info(&dev->dev, "FPGA HW timestamp = %d\n",
            ioread32(board->bar0 + PICO_ADDR + FPGA_TS_OFFSET));

        dma1_reset(board);
        dma2_reset(board);

        ret = pico_cdev_setup(dev, board);
        if (ret) 
        {
            pico_pci_cleanup(dev, board);
        }
    }

    if (!ret) 
    {
        if (0) 
        {}
#ifdef CONFIG_MPS_PICO_FRIB
        else if(dmac_site==USER_SITE_FRIB) 
        {
            init_waitqueue_head(&board->capture_queue);

            board->capture_length = FRIB_CAP_LAST-FRIB_CAP_FIRST+4;
            board->capture_buf = kmalloc(4*board->capture_length, GFP_KERNEL);
            if(!board->capture_buf) {
                board->capture_length = 0;
                dev_err(&dev->dev, "FRIB capture buffer alloc fails.  Capture disabled.\n");
            }

            mb();
            iowrite32(INTR_DMA_DONE|INTR_USER, board->bar0+INTR_CLEAR);
            iowrite32(INTR_DMA_DONE|INTR_USER, board->bar0+INTR_ENABLE);
        }
#endif
        else 
        {
            mb();
            iowrite32(INTR_DMA_DONE|INTR_DMA2_DONE, board->bar0+INTR_CLEAR);
            iowrite32(INTR_DMA_DONE|INTR_DMA2_DONE, board->bar0+INTR_ENABLE);
        }
    }

    if (ret) 
    {
        kobject_put(&board->kobj);
    }
    return ret;
}

/**
 * \brief  Cleans PCI device things
 * \param	dev	PCI device (bus, ...)
 */

static void remove(struct pci_dev *dev)
{
	struct board_data *board = dev_get_drvdata(&dev->dev);

    iowrite32(0, board->bar0+INTR_ENABLE);
	dev_info(&dev->dev, " remove()\n");
    pico_cdev_cleanup(dev, board);
    pico_pci_cleanup(dev, board);

    kobject_put(&board->kobj);
}


/* PCI driver structure */
// this must be created in order to be regitered by the kernel properly
static struct pci_driver pci_driver = {
	.name = MOD_NAME,
	.id_table = ids,
	.probe = probe,
	.remove = remove,
};

static
void print_all_ioctls(void){
	printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: SET_RANGE = 0x%08x\n", (unsigned int)SET_RANGE);
	printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: GET_RANGE = 0x%08x\n", (unsigned int)GET_RANGE);
	printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: SET_FSAMP = 0x%08x\n", (unsigned int)SET_FSAMP);
	printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: GET_FSAMP = 0x%08x\n", (unsigned int)GET_FSAMP);
	printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: GET_B_TRANS = 0x%08x\n", (unsigned int)GET_B_TRANS);
	printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: SET_TRG   = 0x%08x\n", (unsigned int)SET_TRG);
	printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: SET_RING_BUF = 0x%08x\n", (unsigned int)SET_RING_BUF);
	printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: SET_GATE_MUX = 0x%08x\n", (unsigned int)SET_GATE_MUX);
	printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: SET_CONV_MUX = 0x%08x\n", (unsigned int)SET_CONV_MUX);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: GET_THRESHOLD_FLAG = 0x%08x\n", (unsigned int)GET_THRESHOLD_FLAG);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: GET_THRESHOLD = 0x%08x\n", (unsigned int)GET_THRESHOLD);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: SET_THRESHOLD = 0x%08x\n", (unsigned int)SET_THRESHOLD);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: GET_NSAMP = 0x%08x\n", (unsigned int)GET_NSAMP);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: SET_NSAMP = 0x%08x\n", (unsigned int)SET_NSAMP);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: GET_MPS_MANUAL = 0x%08x\n", (unsigned int)GET_MPS_MANUAL);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: SET_MPS_MANUAL = 0x%08x\n", (unsigned int)SET_MPS_MANUAL);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: GET_MPS_CONFIG = 0x%08x\n", (unsigned int)GET_MPS_CONFIG);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: SET_MPS_CONFIG = 0x%08x\n", (unsigned int)SET_MPS_CONFIG);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: GET_MPS_STATUS = 0x%08x\n", (unsigned int)GET_MPS_STATUS);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: SET_MPS_CTRL = 0x%08x\n", (unsigned int)SET_MPS_CTRL);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: GET_RT_DATA_1M = 0x%08x\n", (unsigned int)GET_RT_DATA_1M);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: GET_RT_DATA_1K = 0x%08x\n", (unsigned int)GET_RT_DATA_1K);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: GET_TRG_CNTR_1M = 0x%08x\n", (unsigned int)GET_TRG_CNTR_1M);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: GET_TRG_CNTR_1K = 0x%08x\n", (unsigned int)GET_TRG_CNTR_1K);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: SET_TRG_NRSAMP = 0x%08x\n", (unsigned int)SET_TRG_NRSAMP);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: GET_TRG_NRSAMP = 0x%08x\n", (unsigned int)GET_TRG_NRSAMP);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: SET_TRG_DELAY = 0x%08x\n", (unsigned int)SET_TRG_DELAY);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: GET_TRG_DELAY = 0x%08x\n", (unsigned int)GET_TRG_DELAY);
    printk(KERN_DEBUG MOD_NAME
		": supported IOCTL: ABORT_READ = 0x%08x\n", (unsigned int)ABORT_READ);
}

/**
 * \brief Registers driver to kernel
 * \returns 0 on success, negative on fail
 */

static int __init damc_fmc25_pcie_init(void)
{
	int rc = 0;

    if(!dmac_site_name || dmac_site_name[0]=='\0' || strcmp(dmac_site_name, "caen")==0) 
    {
        dmac_site = USER_SITE_NONE;
#ifdef CONFIG_MPS_PICO_FRIB
    } 
    else if(strcmp(dmac_site_name, "frib")==0) 
    {
        dmac_site = USER_SITE_FRIB;
#endif
    } 
    else 
    {
        printk(KERN_ERR "mps_pico has not site '%s'\n", dmac_site_name);
        return -EINVAL;
    }

	damc_dma_buf_len = damc_req_dma_buf_len;

	printk(KERN_DEBUG "===============================================\n");
	printk(KERN_DEBUG "              CAEN ELS MPS-PICO8               \n");
	printk(KERN_DEBUG "               version: %d.%d.%d               \n", 
        version[0], version[1], version[2]);
    printk(KERN_DEBUG MOD_NAME " init(), built " MPS_PICO_VERSION "\n");
#ifdef CONFIG_MPS_PICO_FRIB
    printk(KERN_DEBUG "Includes \"frib\" site FW support.\n");
#endif
#ifdef CONFIG_MPS_PICO_SITE_DEFAULT
    printk(KERN_DEBUG "Defaults to " CONFIG_MPS_PICO_SITE_DEFAULT " site\n");
#endif
    if(dmac_site!=USER_SITE_NONE)
        printk(KERN_DEBUG "Enabling site mods for \"%s\"\n", dmac_site_name);
    printk(KERN_DEBUG "===============================================\n");

	print_all_ioctls();

    {
        uint64_t cycles, nsec;
        calib_cycles(&cycles, &nsec);

        printk(KERN_DEBUG "get_cycles() calibration for msleep(10) %llu/%llu\n", 
            (unsigned long long)cycles, (unsigned long long)nsec);
    }

	mps_pico8_class = class_create(THIS_MODULE, MOD_NAME);
	if(!mps_pico8_class) return -ENOMEM;

	rc = pci_register_driver(&pci_driver);
	if(rc)
		class_destroy(mps_pico8_class);
	return rc;
}

/**
 * \brief Removes driver from kernel
 */
static void __exit damc_fmc25_pcie_exit(void)
{
	printk(KERN_DEBUG MOD_NAME " exit()\n");
	pci_unregister_driver(&pci_driver);
	class_destroy(mps_pico8_class);
}


module_init(damc_fmc25_pcie_init);
module_exit(damc_fmc25_pcie_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paolo Scarbolo <p.scarbolo@caenels.com>");
MODULE_DESCRIPTION(DRV_NAME);
