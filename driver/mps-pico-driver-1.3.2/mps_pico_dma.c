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

 #include "mps_pico_dma.h"

void dma1_push (
	struct board_data *dev, 
	uint32_t address, 
	uint32_t length, 
	int gen_irq
)
{
	// WRITE DMA BASE ADDRESS
	iowrite32(address, dev->bar0 + DMA_ADDR + DMA_OFFSET_ADDR);
    dev_dbg(&dev->pci_dev->dev,  "   dma_start(): DMA address readback: %08x\n",
		ioread32(dev->bar0 + DMA_ADDR + DMA_OFFSET_ADDR));

	// WRITE DMA LENGTH
	iowrite32(length, dev->bar0 + DMA_ADDR + DMA_OFFSET_LEN);
    dev_dbg(&dev->pci_dev->dev,  "   dma_start(): DMA length readback: %08x\n",
		ioread32(dev->bar0 + DMA_ADDR + DMA_OFFSET_LEN));

	/* make sure that address and length have been written */
	mb();
    dev_dbg(&dev->pci_dev->dev,  "   dma_start(): DMA command go%s!\n", gen_irq ? ", gen irq" : "");
	// WRITE DMA CMD -> CMD GO + IRQ ENABLE BIT if gen_irq == 1
	iowrite32(DMA_CMD_MASK_DMA_GO  | (gen_irq ? DMA_CMD_MASK_GEN_IRQ : 0 ),
		dev->bar0 + DMA_ADDR + DMA_OFFSET_CMD);
}


void dma2_push (
	struct board_data *dev, 
	uint32_t address, 
	uint32_t length, 
	int gen_irq
)
{
	// WRITE DMA BASE ADDRESS
	iowrite32(address, dev->bar0 + DMA2_ADDR + DMA_OFFSET_ADDR);
    dev_dbg(&dev->pci_dev->dev,  "   dma2_start(): DMA2 address readback: %08x\n",
		ioread32(dev->bar0 + DMA2_ADDR + DMA_OFFSET_ADDR));

	// WRITE DMA LENGTH
	iowrite32(length, dev->bar0 + DMA2_ADDR + DMA_OFFSET_LEN);
    dev_dbg(&dev->pci_dev->dev,  "   dma2_start(): DMA2 length readback: %08x\n",
		ioread32(dev->bar0 + DMA2_ADDR + DMA_OFFSET_LEN));

	/* make sure that address and length have been written */
	mb();
    dev_dbg(&dev->pci_dev->dev,  "   dma2_start(): DMA2 command go%s!\n", gen_irq ? ", gen irq" : "");
	// WRITE DMA CMD -> CMD GO + IRQ ENABLE BIT if gen_irq == 1
	iowrite32(DMA_CMD_MASK_DMA_GO  | (gen_irq ? DMA_CMD_MASK_GEN_IRQ : 0 ),
		dev->bar0 + DMA2_ADDR + DMA_OFFSET_CMD);
}

void dma1_enable (
	struct board_data *dev, 
	int enable
)
{
	uint32_t ctrl = enable ? DMA_CTRL_MASK_ENABLE : 0;

	// WRITE DMA ENGINE ENABLE/DISABLE BIT
	iowrite32(ctrl, dev->bar0 + DMA_ADDR + DMA_OFFSET_CONTROL);

/** Register access during DMA sometimes trigger hard lockup of device.
 *  The following is a great way to trigger this.
 *  Resolved w/ DMA controller FW fix in Jan 2016
	ctrl = ioread32(dev->bar0 + DMA_ADDR + DMA_OFFSET_CONTROL);
    dev_dbg(&dev->pci_dev->dev, "   dma_enable(): ctrl read: 0x%08x\n", ctrl);
 */
}

void dma2_enable (
	struct board_data *dev, 
	int enable
)
{
	uint32_t ctrl = enable ? DMA_CTRL_MASK_ENABLE : 0;

	// WRITE DMA ENGINE ENABLE/DISABLE BIT
	iowrite32(ctrl, dev->bar0 + DMA2_ADDR + DMA_OFFSET_CONTROL);

/** Register access during DMA sometimes trigger hard lockup of device.
 *  The following is a great way to trigger this.
 *  Resolved w/ DMA controller FW fix in Jan 2016
	ctrl = ioread32(dev->bar0 + DMA_ADDR + DMA_OFFSET_CONTROL);
    dev_dbg(&dev->pci_dev->dev, "   dma_enable(): ctrl read: 0x%08x\n", ctrl);
 */
}


void dma1_reset (
	struct board_data *dev
)
{
	// WRITE DMA RESET DMA ENGINE BIT
	iowrite32(DMA_CTRL_MASK_RESET, dev->bar0 + DMA_ADDR + DMA_OFFSET_CONTROL);

	/* force write before continuing */
	mb();
}

void dma2_reset (
	struct board_data *dev
)
{
	// WRITE DMA RESET DMA ENGINE BIT
	iowrite32(DMA_CTRL_MASK_RESET, dev->bar0 + DMA2_ADDR + DMA_OFFSET_CONTROL);

	/* force write before continuing */
	mb();
}
