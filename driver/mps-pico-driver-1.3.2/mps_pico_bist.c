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


#include "mps_pico_bist.h"

/* time measurement */
static ktime_t t0;
static ktime_t t1;


int BIST(struct pci_dev *dev)
{
	const int buf_size = 4*1024*1024;
	struct board_data *board = NULL;
	void *tmp_buf = NULL;
	dma_addr_t tmp_buf_dma;
	uint32_t mux_tmp;
	uint64_t time_needed, throughput;
	int rc;

	board = dev_get_drvdata(&dev->dev);

	printk(KERN_DEBUG MOD_NAME ": Performing BIST routine...\n");

	/* allocate buffer */
	tmp_buf = pci_alloc_consistent(dev, buf_size, &tmp_buf_dma);

	/* read mux setting */
	mux_tmp = ioread32(board->bar0 + MUX_ADDR);
	dev_info(&dev->dev, "MUX now %08x\n", (unsigned)mux_tmp);

	/* mux to PRBS */
	iowrite32(1, board->bar0 + MUX_ADDR);

	/* DMA transfer */
    board->dma_irq_flag = 0;
	t0 = ktime_get();
	// disable DMA
	dma_enable(board, 0);
	// set address and length of the data transfer
	// with buf_size = 4*1024*1024, means transfer_time = (buf_size/4 bytes/8 channels)*1MHz = 0.132072 [s]
	dma_push(board, tmp_buf_dma, buf_size, 1);
	// enable DMA to start the transfer
	dma_enable(board, 1);
	// DMA is transferring: wait on interrupt when transfer done or timeout
    rc = wait_event_interruptible_timeout(board->dma_queue, board->dma_irq_flag != 0, msecs_to_jiffies(1000));
	// wait_event_interruptible_timeout - sleep until a condition gets true or a timeout elapses 
	t1 = ktime_get();

	dev_info(&dev->dev, "wait_event %d\n", rc);
	{
		// check if response fifo (DMA response fifo) is empty
		uint32_t val = ioread32(board->bar0+DMA_ADDR+DMA_OFFSET_STATUS);
		if(val)
			dev_warn(&dev->dev, "Response fifo not empty after BIST %08x\n", (unsigned)val);
	}

	// in any case disable again DMA
	dma_enable(board, 0);
	// and clear the interrupt flag (THIS MUST BE ALWAYS DONE BY DRIVER)
    board->dma_irq_flag = 0;

	if (rc <= 0) {
		// if wait event interruptible exited on timeout, DMA transfer problem
		dev_err(&dev->dev, "DMA was unable to finish\n");
		rc = -1;
	} else {
		// if DMA finished before timeout, DMA transfer done -> good
		time_needed = ktime_to_ns(ktime_sub(t1, t0));

		/* 953 = ns->s, b->Mb */
		throughput = (buf_size*8)/do_div(time_needed,953);

		printk(KERN_DEBUG MOD_NAME ":  DMA trans size: %d bytes\n",
			buf_size);
		printk(KERN_DEBUG MOD_NAME ":  DMA trans time: %llu ns\n",
			time_needed);
		printk(KERN_DEBUG MOD_NAME ":  DMA throughput: %llu Mbps\n",
			throughput);
		rc = 0;
	}

	/* reset DMA engine for good measure */
	dma_reset(board);

	/* return mux to previous value */
	iowrite32(mux_tmp, board->bar0 + MUX_ADDR);

	/* free buffer */
	pci_free_consistent(dev, buf_size, tmp_buf, tmp_buf_dma);

	return rc;
}
