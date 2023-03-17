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


#ifndef MPS_PICO_INTERNAL_H_
#define MPS_PICO_INTERNAL_H_

#include <linux/kobject.h>
#include <linux/spinlock.h>
#include <linux/cdev.h>
#include <linux/irqreturn.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/atomic.h>

#include "mps_pico.h"
#include "mps_pico_regs.h"


/** Driver name (shows in lsmod and dmesg) */
#define MOD_NAME "mps_pico"

/** Number of buffers allocated for DMA */
#define DMA_BUF_COUNT		(8)
#define DMA_BYTES_PER_LINE  (64)   // this must be a multiple of 32 otherwise it is not compatible with LINUX PAGE SIZE (usually 4kByte)  

/** Buffer size allocated (should be <= 4MB) */
#define DMA_BUF_SIZE		damc_dma_buf_len
extern unsigned long damc_dma_buf_len;

extern uint32_t dmac_site;

irqreturn_t amc_isr(int irq, void *dev_id);

irqreturn_t amc_isr_dma2(int irq, void *dev_id);

enum dmac_irqmode_t {
    dmac_irq_poll,
    dmac_irq_level,
    dmac_irq_msi
};

/**
 * \struct board_data
 *
 * Keeps state of the PCIe core.
 */

struct board_data {
    /* our own kobj, so we may outlive cdev */
    struct kobject kobj;

	/** the kernel pci device data structure provided by probe() */
    struct pci_dev *pci_dev;

	/** kernel virtual address of the mapped BAR memory and IO regions of
	 *  the End Point. Used by map_bars()/unmap_bars().
	 */
	void * __iomem bar0;
	void * __iomem bar2;

    enum dmac_irqmode_t irqmode;

	/* number of interrupts */
    uint32_t irq_count;

	/** character device number */
	dev_t cdevno;

	/** character device */
	struct cdev cdev;

	/** pointer to DMA buffer for mSGDMA on FPGA */
	void *kernel_mem1_buf[DMA_BUF_COUNT];
    void *kernel_mem2_buf;

	/** physical address of buffers */
	dma_addr_t dma1_buf[DMA_BUF_COUNT];
    dma_addr_t dma2_buf;

	unsigned read1_in_progress;
    unsigned read2_in_progress;
    wait_queue_head_t dma_queue;
    wait_queue_head_t dma1_queue;
    wait_queue_head_t dma2_queue;
    unsigned dma1_irq_flag;
    unsigned dma2_irq_flag;
    uint32_t dma1_bytes_trans;
    uint32_t dma2_bytes_trans;

#ifdef CONFIG_MPS_PICO_FRIB
    unsigned capture_ready;
    unsigned capture_length;
    uint32_t *capture_buf;
    wait_queue_head_t capture_queue;
#endif

    atomic_t num_isr;
    cycles_t last_isr;
    cycles_t longest_isr;
};

#endif /* MPS_PICO_INTERNAL_H_ */
