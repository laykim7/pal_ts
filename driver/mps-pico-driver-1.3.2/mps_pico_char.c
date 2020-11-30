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

 #include <linux/uaccess.h> 
#include "mps_pico_char.h"

static int char_open (
    struct inode *inode,
    struct file *file
)
{
    struct file_data *fdata;
    
    struct board_data *board = container_of(inode->i_cdev, struct board_data, cdev);

    dev_info(&board->pci_dev->dev, "char_open()\n");

    if (!try_module_get(THIS_MODULE))
    {
        return -ENODEV;
    }

    fdata = kzalloc(sizeof(*fdata), GFP_KERNEL);

    if (!fdata) 
    {
        module_put(THIS_MODULE);
        return -ENOMEM;
    }
    
    fdata->board = board;

    file->private_data = fdata;

    return 0;
}

static int char_release (
    struct inode *inode, 
    struct file *file
)
{
    struct file_data *fdata = (struct file_data *)file->private_data;
    struct board_data *board = container_of(inode->i_cdev, struct board_data, cdev);

    dev_info(&board->pci_dev->dev, "char_release()\n");

    kfree(fdata);

    module_put(THIS_MODULE);
    return 0;
}

#ifdef CONFIG_MPS_PICO_FRIB
static ssize_t frib_write_reg(struct board_data *board,
                             const char __user *buf,
                             size_t count,
                             loff_t *pos);
static ssize_t frib_read_reg(struct board_data *board,
                             char __user *buf,
                             size_t count,
                             loff_t *pos);
static ssize_t frib_read_capture(struct board_data *board,
                                 char __user *buf,
                                 size_t count,
                                 loff_t *pos);
#endif




////////////////////////////////////////////////
// Read from DMA1 and DMA2
////////////////////////////////////////////////
static ssize_t char_read (
    struct file *filp,
    char __user *buf,
    size_t count,
    loff_t *pos
)
{
    struct file_data *fdata = (struct file_data *)filp->private_data;
    struct board_data *board = fdata->board;
    int rc, cond1, cond2;
    size_t tmp_count, dma1_count, dma2_count, dma1_count_copy;
    const int ratio = 1000;
    int i;

    dev_dbg(&board->pci_dev->dev, "  read(), site_mode=%u count %zd\n", fdata->site_mode, count);
    if ( fdata->site_mode != 0 )
    {
        return -EINVAL;
    }

    // if read bytes larger than MAX, exit
    if (count > DMA_BUF_COUNT*DMA_BUF_SIZE) 
    {
        return -EINVAL;
    }

    // check if DMA1 read in progress
    spin_lock_irq(&board->dma_queue.lock);
    if (board->read1_in_progress) 
    {
        spin_unlock_irq(&board->dma_queue.lock);
        dev_warn(&board->pci_dev->dev, "  DMA1 read(), concurrent read()s not allowed\n");
        return -EIO;
    }
    board->read1_in_progress = 1;
    // check if DMA2 read in progress
    if (board->read2_in_progress) 
    {
        spin_unlock_irq(&board->dma_queue.lock);
        dev_warn(&board->pci_dev->dev, "  DMA2 read(), concurrent read()s not allowed\n");
        return -EIO;
    }
    board->read2_in_progress = 1;

    // PREPARE DMA1 FOR TRANSFER
    i = 0;
    tmp_count = count;
    dma2_count = ((tmp_count / ratio) / DMA_BYTES_PER_LINE) * DMA_BYTES_PER_LINE;
    dma1_count = tmp_count - dma2_count;
    dma1_count_copy = dma1_count;
    // disable DMA engine
    dma1_enable(board, 0);
    dma2_enable(board, 0);
    while (dma1_count > DMA_BUF_SIZE) 
    {
        dma1_push(board, (uint32_t)board->dma1_buf[i++], DMA_BUF_SIZE, 0);
        dma1_count -= DMA_BUF_SIZE;
    }
    // write ADDRESS, LENGTH AND RISE IRQ
    dma1_push(board, (uint32_t)board->dma1_buf[i], dma1_count, 1);
    dma2_push(board, (uint32_t)board->dma2_buf, dma2_count, 1);
    mb();
    dev_info(&board->pci_dev->dev, "  dma1_push(): length DMA1/2 transfer %lu %lu\n", (long unsigned int) dma1_count, (long unsigned int) dma2_count);
    // ENABLE DMA engine
    dma1_enable(board, 1);
    dma2_enable(board, 1);
    mb();


    //////////////////////////////////////////////////////////
    // check DMA1
    //////////////////////////////////////////////////////////
    if (likely(board->irqmode != dmac_irq_poll)) 
    {
        //const unsigned long twait2 = msecs_to_jiffies(2);
        // TODO: For the moment I check only the dma1_queue, check also dma2_queue??
        // wait_event_interruptible_locked_irq - sleep until a condition gets true 
        // The function will return -ERESTARTSYS if it was interrupted by a signal and 0 if condition evaluated to true.
        dev_dbg(&board->pci_dev->dev, "  start wait_event_interruptible_locked_irq on DMA1\n");
        rc = wait_event_interruptible_locked_irq(board->dma_queue, board->dma1_irq_flag!=0);
        dev_dbg(&board->pci_dev->dev, "  wait_event_interruptible_locked_irq on DMA1 rc = %d\n", rc);
    } 
    else 
    {
        const unsigned long twait1 = msecs_to_jiffies(1);
        do 
        {
            spin_unlock_irq(&board->dma_queue.lock);
            /* must unlock for call to amc_isr() as spin locks aren't recursive */
            if (amc_isr(board->pci_dev->irq, board) == IRQ_NONE)
            {
                // wait_event_interruptible_timeout - sleep until a condition gets true or a timeout elapses
                // 0 if the condition evaluated to false after the timeout elapsed, 1 if the condition evaluated 
                // to true after the timeout elapsed, the remaining jiffies (at least 1) if the condition evaluated 
                // to true before the timeout elapsed, or -ERESTARTSYS if it was interrupted by a signal
                rc = wait_event_interruptible_timeout(board->dma_queue, board->dma1_irq_flag!=0, twait1);
                dev_dbg(&board->pci_dev->dev, "  wait_event_interruptible_timeout on DMA1 rc = %d\n", rc);
            }
            else
            {
                rc = 0;
            }
            spin_lock_irq(&board->dma_queue.lock);
            /* continue while no "IRQ" signaled, and wait not interrupted */
        } while(board->dma1_irq_flag==0 && rc>=0);

        if (rc > 0)
        {
            rc = 0;
        }

    }

    // re-save the initial value of dma1_count
    dma1_count = dma1_count_copy;
    /*
     * dma_irq_flag==1 && rc==0 is normal completion
     * rc==-ERESTARTSYS is user abort
     * other cases not to happen, but are treated as -ECANCELED
     */
    cond1 = board->dma1_irq_flag;
    board->dma1_irq_flag = 0;
    board->read1_in_progress = 0;
    dev_dbg(&board->pci_dev->dev, "read() DMA1 wait complete w/ rc=%d cond1=%d\n", rc, cond1);

    // TODO: not understood
    if ((rc != 0) || (cond1 != 1 )) 
    { /* interrupted or aborted */
        if(cond1!=1) rc = -ECANCELED;
        /* reset DMA engine */
        dma1_reset(board);
        board->dma1_bytes_trans = 0;
        spin_unlock_irq(&board->dma_queue.lock);

        dev_warn(&board->pci_dev->dev, "DMA1 read(): interrupt failed: %d\n", rc);
        return rc;
    } 
    else 
    {
        // If everithing went smooth till now
        spin_unlock_irq(&board->dma_queue.lock);

        i = 0;
        dev_dbg(&board->pci_dev->dev, "  read(): DMA1 returned from sleep %lu\n", (long unsigned int) dma1_count);
        rc = 0;
        // copy data from RAM to User memory (in the case when tmp_count > DMA_BUF_SIZE)
        while ((dma1_count > DMA_BUF_SIZE) && (rc == 0)) 
        {
        // rc = copy_to_user(buf + DMA_BUF_SIZE*i, board->kernel_mem1_buf[i], DMA_BUF_SIZE);
      rc = copy_to_user(buf + DMA_BUF_SIZE*i, board->kernel_mem1_buf[i], DMA_BUF_SIZE);
            dev_dbg(&board->pci_dev->dev, "  copy_to_user() DMA1: n bytes not copied %d\n", rc);
            dma1_count -= DMA_BUF_SIZE;
            /* sometimes the DMA done interrupt comes even though nothing has been
             * transfered.  Fill our buffer with a test pattern so that this is more
             * obvious.
             */
            memset(board->kernel_mem1_buf[i], 0xf0, DMA_BUF_SIZE);
            i++;
        }
        // copy DMA1 data from RAM to User memory (in the case when tmp_count < DMA_BUF_SIZE)
        if (rc == 0) 
        {
            // copy_to_user return the number of bytes that could NOT be copied
            // rc = copy_to_user(buf + DMA_BUF_SIZE*i,    board->kernel_mem1_buf[i], dma1_count);
            rc = copy_to_user(buf + DMA_BUF_SIZE*i,    board->kernel_mem1_buf[i], dma1_count);
            memset(board->kernel_mem1_buf[i], 0xf0, dma1_count);
        }

        if (rc) 
        {
            return rc;
        }

    }

    //////////////////////////////////////////////////////////
    // check DMA2
    //////////////////////////////////////////////////////////
    spin_lock_irq(&board->dma_queue.lock);
    if (likely(board->irqmode != dmac_irq_poll)) 
    {
        //const unsigned long twait2 = msecs_to_jiffies(2);
        // TODO: For the moment I check only the dma1_queue, check also dma2_queue??
        // wait_event_interruptible_locked_irq - sleep until a condition gets true 
        // The function will return -ERESTARTSYS if it was interrupted by a signal and 0 if condition evaluated to true.
        dev_dbg(&board->pci_dev->dev, "  start wait_event_interruptible_locked_irq on DMA2\n");
        rc = wait_event_interruptible_locked_irq(board->dma_queue, board->dma2_irq_flag!=0);
        dev_dbg(&board->pci_dev->dev, "  wait_event_interruptible_locked_irq on DMA2 rc = %d\n", rc);
    } 
    else 
    {
        const unsigned long twait1 = msecs_to_jiffies(1);
        do 
        {
            spin_unlock_irq(&board->dma_queue.lock);
            /* must unlock for call to amc_isr() as spin locks aren't recursive */
            if (amc_isr(board->pci_dev->irq, board) == IRQ_NONE)
            {
                // wait_event_interruptible_timeout - sleep until a condition gets true or a timeout elapses
                // 0 if the condition evaluated to false after the timeout elapsed, 1 if the condition evaluated 
                // to true after the timeout elapsed, the remaining jiffies (at least 1) if the condition evaluated 
                // to true before the timeout elapsed, or -ERESTARTSYS if it was interrupted by a signal
                rc = wait_event_interruptible_timeout(board->dma_queue, board->dma2_irq_flag!=0, twait1);
                dev_warn(&board->pci_dev->dev, "  wait_event_interruptible_timeout on DMA2 rc = %d\n", rc);
            }
            else
            {
                rc = 0;
            }
            spin_lock_irq(&board->dma_queue.lock);
            /* continue while no "IRQ" signaled, and wait not interrupted */
        } while(board->dma2_irq_flag==0 && rc>=0);

        if (rc > 0)
        {
            rc = 0;
        }

    }
    /*
     * dma_irq_flag==1 && rc==0 is normal completion
     * rc==-ERESTARTSYS is user abort
     * other cases not to happen, but are treated as -ECANCELED
     */

    cond2 = board->dma2_irq_flag;
    board->dma2_irq_flag = 0;
    board->read2_in_progress = 0;
    dev_dbg(&board->pci_dev->dev, "read() DMA2 wait complete w/ rc=%d cond2=%d\n", rc, cond2);

    // TODO: not understood
    if ((rc != 0) || (cond2 != 1 )) 
    { /* interrupted or aborted */
        if(cond2!=1) rc = -ECANCELED;
        /* reset DMA engine */
        dma2_reset(board);
        board->dma2_bytes_trans = 0;
        spin_unlock_irq(&board->dma_queue.lock);

        dev_warn(&board->pci_dev->dev, "DMA2 read(): interrupt failed: %d\n", rc);
        return rc;
    } 
    else 
    {
        // If everithing went smooth till now
        spin_unlock_irq(&board->dma_queue.lock);

        dev_dbg(&board->pci_dev->dev, "  read(): DMA2 returned from sleep %d\n", dma2_count);
        dev_info(&board->pci_dev->dev, "  read(): DMA2 returned from sleep %lu, %lu, %lu\n", (long unsigned int) dma2_count, (long unsigned int) DMA_BUF_SIZE, (long unsigned int) i);
        rc = 0;
        // copy data from RAM to User memory (in the case when tmp_count > DMA_BUF_SIZE)
        //rc = copy_to_user(buf + DMA_BUF_SIZE*i + dma1_count, board->kernel_mem2_buf, dma2_count);
        rc = copy_to_user(buf + DMA_BUF_SIZE*i + dma1_count, board->kernel_mem2_buf, dma2_count);
        memset(board->kernel_mem2_buf, 0xf0, dma2_count);
        
        if (rc) 
        {
            return rc;
        }

    }

    *pos += count;

    return count;
}


    // by laykim 20180902
struct st_regRW
{
    unsigned int offset;     
    unsigned int val;
};

// #define MAGIC_NUM 0xDB
#define IOCTL_R  _IOWR(MPS_PICO_MAGIC, 1, struct st_regRW)
#define IOCTL_W  _IOWR(MPS_PICO_MAGIC, 2, struct st_regRW)
#define IOCTL_R2 _IOWR(MPS_PICO_MAGIC, 3, struct st_regRW)
#define IOCTL_W2 _IOWR(MPS_PICO_MAGIC, 4, struct st_regRW)
    // by laykim 20180902 -- end


/* all possible ioctl() value types */
union ioctl_value {
    uint8_t u8;
    uint32_t u32;
    struct trg_ctrl trg;
    struct pico_float_coeff float_coeff;

    // by laykim 20180902
    struct st_regRW reg;
};

static
long char_ioctl(
    struct file *filp,
    unsigned int cmd,
    unsigned long arg
)
{
    union ioctl_value uval;
    long ret =0;
    struct file_data *fdata = (struct file_data *)filp->private_data;
    struct board_data *board = fdata->board;
    size_t tocpy = sizeof(uval);

    long abort_flag = 0;
    uint32_t scaler;
    uint32_t sver;

    /* copy some bytes to/from user space, others are zero'd */
    memset(&uval, 0, sizeof(uval));
    if(_IOC_SIZE(cmd)<sizeof(uval))
        tocpy = _IOC_SIZE(cmd);

    // dev_info(&board->pci_dev->dev, "%s: 0x%08x size=%u\n", __PRETTY_FUNCTION__, cmd, (unsigned)tocpy);

    // if(_IOC_DIR(cmd)&_IOC_WRITE) {
    //     /* copy in all provided bytes. based on IOCTL code. */
    //     //ret = copy_from_user(&uval, (const void*)arg, tocpy);
    //     ret = copy_from_user(&uval, (const void*)arg, tocpy);
    //     if(ret) return ret;
    // }

    /* validate cmd and copy in values from user before locking
     * can't access board->
     */
    //  printk("cmd : %d r%d w%d r2_%d w%d\r\n", cmd, IOCTL_R, IOCTL_W, IOCTL_R2, IOCTL_W2);

    if( copy_from_user(&uval, (const void*)arg, tocpy) ){
      printk("Error copy_from_user");
      return -EINVAL;
    }

    /* locking here to protect RMW register operations.
     * Use dma_queue.lock for convinience
     */
    spin_lock_irq(&board->dma_queue.lock); /* enter critical section, can't sleep */
    // Uncomment these two lines to use DMA1 or DMA2 singularly
    // spin_lock_irq(&board->dma1_queue.lock); /* enter critical section, can't sleep */
    // spin_lock_irq(&board->dma2_queue.lock); /* enter critical section, can't sleep */

    uint32_t ctrl_tmp;

    switch (cmd) 
    {
      case IOCTL_W :  // printk("IOCTL_W.\n"); 
                    iowrite32( uval.reg.val, (u32 *)(board->bar0 + uval.reg.offset) );
                    break;

      case IOCTL_R :  // printk("IOCTL_R.\n"); 
                      // printk("IOCTL_R addr : 0x%08x.\n", (u32 *)(board->bar0 + uval.reg.offset)); 
                    uval.reg.val = ioread32((u32 *)(board->bar0 + uval.reg.offset));
                    if( copy_to_user((void*)arg, &uval, tocpy) ){
                      printk("Error copy_to_user");
                      ret = -EINVAL;
                    } 
                    break;
     case IOCTL_W2 :  // printk("IOCTL_W2.\n"); 
                    switch (uval.reg.offset)
                    {
                      case 0x40 : //printk("SET_FSAMP.\n");
                                  scaler = PICO_CLK_FREQ / uval.reg.val - 1;
                                  if ((uval.reg.val > PICO_ADC_MAX_FREQ) || (scaler > (PICO_CONV_MAX-1))){
                                    ret = -EINVAL;
                                  } 
                                  else{
                                    iowrite32(scaler, board->bar0 + PICO_CONV_GEN);
                                  }
                                  break;
                      case 0x44 : //printk("SET_SITE_MODE.\n");//SET_SITE_MODE
                                  if(0) {}
                                #ifdef CONFIG_MPS_PICO_FRIB
                                  else if(dmac_site==USER_SITE_FRIB) {
                                      if(uval.reg.val>2){ret = -EINVAL;}
                                  }
                                #endif
                                  else if(uval.reg.val!=0){ret = -EINVAL;}
                                  else {fdata->site_mode = uval.reg.val;}
                                  break;
                      case 0x50 : //printk("SET_GATE_MUX.\n");//SET_GATE_MUX
                                  uval.reg.val &= MUX_TRG_MASK;
                                  uval.reg.val <<= MUX_TRG_SHIFT;
                                  ctrl_tmp = ioread32(board->bar0 + PICO_ADDR + PICO_CONV_TRG);
                                  ctrl_tmp &= ~(MUX_TRG_MASK << MUX_TRG_SHIFT);
                                  ctrl_tmp |= uval.reg.val;
                                  iowrite32(ctrl_tmp, board->bar0 + PICO_ADDR + PICO_CONV_TRG);
                                  break;
                      case 0x54 : //printk("SET_CONV_MUX.\n");//SET_CONV_MUX
                                  uval.reg.val &= MUX_CONV_MASK;
                                  uval.reg.val <<= MUX_CONV_SHIFT;
                                  ctrl_tmp = ioread32(board->bar0 + PICO_ADDR + PICO_CONV_TRG);
                                  ctrl_tmp &= ~(MUX_CONV_MASK << MUX_CONV_SHIFT);
                                  ctrl_tmp |= uval.reg.val;
                                  iowrite32(ctrl_tmp, board->bar0 + PICO_ADDR + PICO_CONV_TRG);
                                  break;
                      case 0x58 : //printk("ABORT_READ.\n");//ABORT_READ
                                  board->dma1_irq_flag = 2;
                                  board->dma2_irq_flag = 2;
                                  wake_up_locked(&board->dma_queue);
                                  abort_flag = 1;
                                  break;

                      default   : printk("Error IOCTL_W2\r\n");
                                  ret = -EINVAL;
                                  break;
                    }
                    break;

     case IOCTL_R2 :  // printk("IOCTL_R2.\n"); 
                    switch (uval.reg.offset)
                    {
                      case 0x00 : //printk("GET_VERSION.\n");//GET_VERSION
                                  uval.reg.val = GET_VERSION_CURRENT;
                                  break;
                      case 0x04 : //printk("GET_SITE_ID.\n");//GET_SITE_ID
                                  uval.reg.val = dmac_site;
                                  break;
                      case 0x08 : //printk("GET_SITE_VERSION.\n");//GET_SITE_VERSION
                                  switch(dmac_site) {
                                #ifdef CONFIG_MPS_PICO_FRIB
                                  case USER_SITE_FRIB: sver = 0; break;
                                #endif
                                  default: sver = 0; break;
                                  }
                                  uval.reg.val = sver;
                                  break;
                      case 0x0C : //printk("GET_B_TRANS.\n");//GET_B_TRANS
                                  uval.reg.val = board->dma1_bytes_trans + board->dma2_bytes_trans; // - DMA_BYTES_PER_LINE;  // TODO:
                                  break;

                      case 0x40 : //printk("SET_FSAMP.\n");
                                  uval.reg.val = scaler;
                                  break;
                      case 0x44 : //printk("SET_SITE_MODE.\n");//SET_SITE_MODE
                                  uval.reg.val = -1;
                                  break;
                      case 0x50 : //printk("SET_GATE_MUX.\n");//SET_GATE_MUX
                                  ctrl_tmp = ioread32(board->bar0 + PICO_ADDR + PICO_CONV_TRG);
                                  ctrl_tmp &= ~(MUX_TRG_MASK << MUX_TRG_SHIFT);
                                  uval.reg.val = ctrl_tmp;
                                  break;
                      case 0x54 : //printk("SET_CONV_MUX.\n");//SET_CONV_MUX
                                  ctrl_tmp = ioread32(board->bar0 + PICO_ADDR + PICO_CONV_TRG);
                                  ctrl_tmp &= ~(MUX_CONV_MASK << MUX_CONV_SHIFT);
                                  uval.reg.val = ctrl_tmp;
                                  break;
                      case 0x58 : //printk("ABORT_READ.\n");//ABORT_READ
                                  uval.reg.val = -1;
                                  break;                                  
                      default   : printk("Error IOCTL_R2 0x%08x\r\n", uval.reg.offset);
                                  ret = -EINVAL;
                                  break;
                    }
                    
                    if( copy_to_user((void*)arg, &uval, tocpy) ){
                      printk("Error copy_to_user");
                      ret = -EINVAL;
                    } 
                    break;
      default:      dev_info(&board->pci_dev->dev, "%s: unknown ioctl\n", __PRETTY_FUNCTION__);
                    printk("Error cmd");
                    break;
    }

    spin_unlock_irq(&board->dma_queue.lock);

#ifdef CONFIG_MPS_PICO_FRIB
    // if(cmd==ABORT_READ) {
    if(1==abort_flag) {
        /* abort any waiting for capture buffer */
        spin_lock_irq(&board->capture_queue.lock);
        board->capture_ready = 2;
        wake_up_locked(&board->capture_queue);
        spin_unlock_irq(&board->capture_queue.lock);
    }
#endif

    return ret;
}

static
ssize_t char_write(struct file *filp, const char __user *buf, size_t count, loff_t *pos)
{
    struct file_data *fdata = (struct file_data *)filp->private_data;
    struct board_data *board = fdata->board;
    (void)board;
    if(0) {}
#ifdef CONFIG_MPS_PICO_FRIB
    else if(dmac_site==USER_SITE_FRIB) {
        switch(fdata->site_mode) {
        case 1:  return frib_write_reg(board, buf, count, pos);
        default: return -EINVAL;
        }
    }
#endif
    else
        return -EINVAL;
}

static
loff_t char_llseek(struct file *filp, loff_t pos, int whence)
{
    struct file_data *fdata = (struct file_data *)filp->private_data;
    //struct board_data *board = fdata->board;
    loff_t npos;

    if(dmac_site!=USER_SITE_FRIB || fdata->site_mode==0)
        return -EINVAL;
#ifdef CONFIG_MPS_PICO_FRIB

    switch(whence) {
    case 0: npos = pos; break;
    case 1: npos = filp->f_pos + pos; break;
    case 2: npos = 0x40000 + pos; break;
    default: return -EINVAL;
    }

    if(npos<0)
        return -EINVAL;
    else if(npos>0x40000)
        npos = 0x40000;

    filp->f_pos = npos;
    return npos;

#else
    //(void)board;
    (void)npos;
    return -EINVAL;
#endif
}

const struct file_operations mps_pico_fops = {
    .owner        = THIS_MODULE,
    .open        = char_open,
    .release    = char_release,
    .read        = char_read,
    .write      = char_write,
    .llseek     = char_llseek,
    .unlocked_ioctl = char_ioctl
};

#ifdef CONFIG_MPS_PICO_FRIB

static ssize_t frib_write_reg (
    struct board_data *board,
    const char __user *buf,
    size_t count,
    loff_t *pos)
{
    ssize_t ret=0;
    unsigned offset, end;
    const uint32_t __user *ibuf = (const uint32_t __user *)buf;

    if(count%4) return -EINVAL;
    if(!pos || *pos<USER_ADDR) return -EINVAL;

    offset = *pos-USER_ADDR;

    if(offset>=0x10000) return -EINVAL;

    if(count>0x10000-offset)
        count = 0x10000-offset;

    end = offset+count;

    for(; offset<count; offset+=4, ibuf+=4)
    {
        uint32_t val;
        ret = get_user(val, ibuf);
        if(unlikely(ret)) break;
        iowrite32(val, board->bar0 + USER_ADDR + offset);
    }

    if(!ret) ret=count;
    return ret;
}

static ssize_t frib_read_reg (
    struct board_data *board,
    char __user *buf,
    size_t count,
    loff_t *pos)
{
    ssize_t ret=0;
    unsigned offset, end;
    uint32_t __user *ibuf = (uint32_t __user *)buf;

    if(count%4) return -EINVAL;
    if(!pos || *pos<USER_ADDR) return -EINVAL;

    offset = *pos-USER_ADDR;

    if(offset>=0x10000) return 0;

    if(count>0x10000-offset)
        count = 0x10000-offset;

    end = offset+count;

    for(; ret && offset<count; offset+=4, ibuf+=4)
    {
        uint32_t val = ioread32(board->bar0 + USER_ADDR + offset);
        ret = put_user(val, ibuf);
    }

    if(!ret) ret=count;
    return ret;
    /* *pos not updated */
}

static ssize_t frib_read_capture (
    struct board_data *board,
    char __user *buf,
    size_t count,
    loff_t *pos)
{
    ssize_t ret;
    unsigned offset;

    if(count%4) return -EINVAL;
    if(!pos || *pos<USER_ADDR+0x100) return -EINVAL;

    offset = *pos-(USER_ADDR+0x100);

    if(offset>=board->capture_length) {
        dev_info(&board->pci_dev->dev, "Capture read starts past end of buffer %u %u\n",
                offset, board->capture_length);
        return 0;
    }

    if(count > board->capture_length-offset)
        count = board->capture_length-offset;

    spin_lock_irq(&board->capture_queue.lock);

    ret = wait_event_interruptible_locked_irq(board->capture_queue, board->capture_ready!=0);

    if(!ret && board->capture_ready!=1)
        ret = -ECANCELED;
    board->capture_ready = 0;

    spin_unlock_irq(&board->capture_queue.lock);

    if(ret) return ret;

    ret = copy_to_user(buf, board->capture_buf, count);
    if(!ret) ret=count;
    return ret;
    /* *pos not updated */
}

#endif
