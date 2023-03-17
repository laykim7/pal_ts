/*

* Copyright (C) 2013 - 2016 Xilinx, Inc
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.

*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License along
*   with this program. If not, see <http://www.gnu.org/licenses/>.

*/


#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/interrupt.h>

#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>

#include <linux/irq.h>
#include <asm/io.h>
#include <linux/fs.h>		//required for fops
#include <linux/uaccess.h>	//required for 'cpoy_from_user' and 'copy_to_user'
#include <linux/signal.h>	//required for kernel-to-userspace signals

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h> /* register_chrdev, unregister_chrdev */
#include <linux/seq_file.h> /* seq_read, seq_lseek, single_release */

#include "dd_comdef.h"
#include "ts2ip.h"
#include "ts2ip_version.h"

/* Standard module information, edit as appropriate */
MODULE_LICENSE("GPL");
MODULE_AUTHOR ("durutronix");
MODULE_DESCRIPTION("Driver for ts2ip embedded evr.");
MODULE_ALIAS("custom:ts2ip");

#define DRIVER_NAME "ts2ip"   // timing system 2 ips[evg, evr, gtp, evsys]


#define IPID_MAX 32

#define IPID_GTP      0x40
#define IPID_EVSYS    0x41
#define IPID_EVG      0x42
#define IPID_EVR      0x43
#define IPID_ZQSYS    0x44

struct st_ipid {
  unsigned long id;
  char          devName[16];
};

static struct st_ipid ipidList[IPID_MAX] =
{
  {IPID_GTP  , "gtp"  },
  {IPID_EVSYS, "evsys"},
  {IPID_EVG  , "evg"  },
  {IPID_EVR  , "evr"  },
  {IPID_ZQSYS, "zqsys"},
  {0,""}
}; 


#define RET_OK  0
#define RET_ERR -1

ssize_t _write(struct file *fp, const char *buf, size_t length, loff_t *offset);
ssize_t _read(struct file *fp, char *buf, size_t length, loff_t *offset);
long _ioctl (struct file *fp, unsigned int cmd, unsigned long arg);
static int _open(struct inode *inode, struct file *fp);
static int _close(struct inode *inode, struct file *fp);

static char get_ipidListN(char ipid);
static unsigned char get_idByMajor(char major);

wait_queue_head_t read_wait_queue;
static int devCount;

struct st_devInfo {
  char   ipid;
  char   ipidSub;
  u32    isCreate;
  u32    isOpen;
  char   devName[16];
  int irq;
  dev_t  devt;        // variable for the device number
  struct cdev c_dev;  // variable for the character device structure
  struct class *pCl;  // variable for the device class
  unsigned long mem_start;
  unsigned long mem_end;
  unsigned long mem_size;
  unsigned long mem_base;
  
  u32    devNum;
};

static struct st_devInfo dInfo[IPID_MAX];

// device init and file operations
struct file_operations _fops = {
  .owner          = THIS_MODULE, //Taba
  .read           = _read,    // read()
  .write          = _write,   // write()
  .open           = _open,    // open()
  .release        = _close,   // close()
  .unlocked_ioctl = _ioctl,     // ioctl()
};






//===============================================================================
//  write
//===============================================================================
ssize_t _write(struct file *fp, const char *buf, size_t length, loff_t *offset)
{
  struct st_devInfo *pdi = fp->private_data;
  printk("%s _write.\n",pdi->devName);

  return RET_OK;
}

//===============================================================================
//  read
//===============================================================================
static int flag = 0;

ssize_t _read(struct file *fp, char *buf, size_t length, loff_t *offset)
{
  //unsigned int vBTGR = 0;
  struct st_devInfo *pdi = fp->private_data;
  // printk("%s _read.\n",pdi->devName);
  
  unsigned int rData = 0;
  if( wait_event_interruptible(read_wait_queue, flag != 0 ))
  {
    printk("[err-k] %s _read: wait_event_interruptible\r\n",pdi->devName);
    return (RET_ERR);
  }

  rData = ioread32((u32 *)(pdi->mem_base + 4*4));  // read interrupt register value

  if(copy_to_user((unsigned int *)buf, &rData, sizeof(unsigned int)))
  {
    printk("[err-k] %s _read: copy_to_user",pdi->devName);
    return (RET_ERR);
  } 

  // printk("[k]_read 0x%08x\r\n", rData);
  flag = 0;

  return sizeof(unsigned int);
}

//===============================================================================
//  ioctl
//===============================================================================
long _ioctl (struct file *fp, unsigned int cmd, unsigned long arg)
{

  struct st_regRW reg;
  struct st_devInfo *pdi = fp->private_data;

  switch (cmd) {

    case IOCTL_W :
      //printk("IOCTL_W.\n"); 
      if( copy_from_user(&reg, (struct st_regRW *)arg, sizeof(struct st_regRW)))
      {
        printk("Error copy_from_user");
        return (RET_ERR);
      }

      //printk("offset     = %d",  reg.offset);
      //printk("val        = %d",  reg.val);
      // write here
      iowrite32(reg.val, (u32 *)(pdi->mem_base + reg.offset) );
      
      break;

    case IOCTL_R :
      //printk("IOCTL_R.\n"); 

      if( copy_from_user(&reg, (struct st_regRW *)arg, sizeof(struct st_regRW)))
      {
        printk("Error copy_from_user");
        return (RET_ERR);
      }

      //getUpRegConfig(p_card->baseSpiUpdateIp, &reg);
      reg.val = ioread32((u32 *)(pdi->mem_base + reg.offset));

      //printk("offset     = %d",  reg.offset);
      //printk("val        = %d",  reg.val);

      if(copy_to_user((struct st_regRW *)arg, &reg, sizeof(struct st_regRW)))
      {
        printk("Error copy_to_user");
        return (RET_ERR);
      } 
      break;

    default:
      break;
  }

  return (RET_OK);
}

//===============================================================================
//  open
//===============================================================================
static int _open(struct inode *pI, struct file *fp)
{
  struct st_devInfo *pdi;
  char major = 0;
  unsigned char id;

  major = imajor(pI);
  if(major < 1){
    printk("[ERR] _open get major.\n");
    return RET_ERR;
  }

  id = get_idByMajor(major);
  if(id >= IPID_MAX){
    printk("[ERR] _open get_idByMajor\n");
    return RET_ERR;
  }
  
  pdi = &dInfo[id];
  fp->private_data = pdi;

  if(pdi->isCreate != 1)
  {
    printk("[ERR] %s : isCreate %d\n", pdi->devName, pdi->isCreate );
    return RET_ERR;
  }

  if (pdi->isOpen)          // if the device is already open,
    return -EBUSY;          // return with an error

  pdi->isOpen++;            // 'open' device
  
  // printk("%s device open done.\n",pdi->devName);

  return RET_OK;
}

//===============================================================================
//  close
//===============================================================================
static int _close(struct inode *pI, struct file *fp)
{
  struct st_devInfo *pdi;
  char major = 0;
  unsigned char id;

  major = imajor(pI);
  if(major < 1){
    printk("[ERR] _close get major.\n");
    return RET_ERR;
  }

  id = get_idByMajor(major);
  if(id >= IPID_MAX){
    printk("[ERR] _close get_idByMajor\n");
    return RET_ERR;
  }
  
  pdi = &dInfo[id];
  fp->private_data = pdi;

  if(pdi->isCreate != 1)
  {
    printk("[ERR] %s : isCreate %d\n", pdi->devName, pdi->isCreate );
    return RET_ERR;
  }

  if (pdi->isOpen < 1)
    return RET_ERR;          // return with an error

  pdi->isOpen--;            // 'close' device
  
  printk("%s device close done.\n",pdi->devName);

  return RET_OK;
}

//===============================================================================
//  cleanup
//===============================================================================
static void cleanup(struct st_devInfo *pdi)
{
  printk("%s cleanup...\n", pdi->devName);
  
  if (pdi->isCreate){
    printk("%s isCreate.\n", pdi->devName);
    device_destroy(pdi->pCl, pdi->devt);
    cdev_del(&(pdi->c_dev));
  }

  if (pdi->pCl){
    printk("%s pCl.\n", pdi->devName);
    class_destroy(pdi->pCl);
  }
  
  if ( MAJOR(pdi->devt) != 0){
    printk("%s  MAJOR(pdi->devt).\n", pdi->devName);
    unregister_chrdev_region( pdi->devt, 1);
  }

  printk("%s cleanup done.\n", pdi->devName);
}


//===============================================================================
//  irqHandler
//===============================================================================
static irqreturn_t irqHandler(int irq, void *val)
{
#if 0
  // Clear Interrupt Enable
  iowrite32(0, (u32 *)(ioMemBaseAddress + A_intrEnable));
	kill_fasync(&_async_queue, SIGIO, POLL_IN);
#else
  // static unsigned int irqTime;
  struct st_devInfo *pdi = (struct st_devInfo *)val;

  iowrite32(0, (u32 *)(pdi->mem_base + 9*4));             // ipSys_intrEnable -> disable
  //printk(KERN_ALERT "%s local interrupt %d.\n",DEVICE_NAME, irqTime++);
  flag = 1;
  wake_up_interruptible(&read_wait_queue);
#endif

  return IRQ_HANDLED;
}


//===============================================================================
//  _probe
//===============================================================================

static int _probe(struct platform_device *pdev)
{
  struct device *dev = &pdev->dev;
  struct st_devInfo *pdi;

  struct resource *r_irq; /* Interrupt resources */
  struct resource *r_mem; /* IO mem resources */

  int rc = 0;

  unsigned long rData;
  unsigned long ipidListN;

  dev_info(dev, "Device Tree Probing\n");
  init_waitqueue_head(&read_wait_queue);  

  /* Get iospace for the device */
  r_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
  if (!r_mem){
    dev_err(dev, "invalid address\n");
    return -ENODEV;
  }

  pdi = &dInfo[devCount];

  pdi->mem_start = r_mem->start;
  pdi->mem_end   = r_mem->end;
  pdi->mem_size  = r_mem->end - r_mem->start + 1;
  pdi->mem_base = (unsigned long)ioremap(pdi->mem_start, pdi->mem_size);

  rData = ioread32((u32 *)(pdi->mem_base + 1*4));  // read interrupt register value
  pdi->ipid    = (unsigned char)((rData & 0xff000000) >> 24);
  pdi->ipidSub = (rData & 0x00ff0000) >> 16;

  ipidListN = (unsigned long)get_ipidListN(pdi->ipid);

  if (ipidListN == -1){
    dev_info(dev, "Could not find ipid in ipidList[%ld] / ipid[0x%02x].\n", ipidListN, pdi->ipid);
    return -ENODEV;
  }

  sprintf(pdi->devName,"%s%d",ipidList[ipidListN].devName, (int)pdi->ipidSub);
  printk("ipid_val[0x%02x], ipidSub_val[0x%02x]\r\n", pdi->ipid, pdi->ipidSub);

  dev_set_drvdata(dev, (void*)pdi);
  // dev_info(dev, "dev_set_drvdata OK.\n");

  if (!request_mem_region(pdi->mem_start, pdi->mem_size, pdi->devName)){
    dev_err(dev, "Couldn't lock memory region at %p\n", (void *)pdi->mem_start);
    rc = -EBUSY;
    goto error1;
  }
  // dev_info(dev, "request_mem_region OK.\n");

    /* Get IRQ for the device */
  r_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
  if (!r_irq){
    dev_info(dev, "no IRQ found\n");
    goto error2;
  }
  // dev_info(dev, "platform_get_resource OK.\n");

  rc = request_irq(r_irq->start, &irqHandler, 0, pdi->devName, pdi);
  if (rc){
    dev_err(dev, "Could not allocate interrupt %d.\n", r_irq->start);
    goto error3;
  }
  // dev_info(dev, "request_irq OK.\n");
  dev_info(dev,"%s mapped to 0x%08x, irq=%d\n", pdi->devName, (int)pdi->mem_base, r_irq->start);
  pdi->irq = r_irq->start;

  /* cat /proc/devices */
  if (alloc_chrdev_region(&pdi->devt, 0, 1, pdi->devName) < 0)
    goto cdev_error;

  printk("0_sub_probe...pdi->major :  %d \r\n", MAJOR(pdi->devt));
  printk("0_sub_probe...pdi->minor :  %d \r\n", MINOR(pdi->devt));

  /* ls /sys/class */
  if ((pdi->pCl = class_create(THIS_MODULE, pdi->devName)) == NULL)
    goto cdev_error;

  /* ls /dev/ */
  if (device_create(pdi->pCl, NULL, pdi->devt, NULL, pdi->devName) == NULL)
    goto cdev_error;

  pdi->isCreate = 1;
  cdev_init(&(pdi->c_dev), &_fops);

  if (cdev_add(&(pdi->c_dev), pdi->devt, 1) == -1)
      goto cdev_error;

  devCount++;

  return 0;

error3:
  printk("[error3] _probe\r\n");
  free_irq(r_irq->start, pdi);
error2:
  printk("[error2] _probe\r\n");
  release_mem_region(pdi->mem_start, pdi->mem_size);
error1:
  printk("[error1] _probe\r\n");
  dev_set_drvdata(dev, NULL);
  return rc;
cdev_error:
  printk("[cdev_error] _probe\r\n");
  cleanup(pdi);
  return -1;
}


//===============================================================================
//  _remove
//===============================================================================
static int _remove(struct platform_device *pdev)
{
  struct st_devInfo *pdi = dev_get_drvdata(&pdev->dev);
  devCount = 0;
  
  cleanup(pdi);
  free_irq(pdi->irq, pdi);
  release_mem_region(pdi->mem_start, pdi->mem_size);
  dev_set_drvdata(&pdev->dev, NULL);

  printk("remove[%s] : 0x%08x\n",pdi->devName , (int)pdi->mem_start);

  return RET_OK;
}

//===============================================================================
//  of_device_id
//===============================================================================
#ifdef CONFIG_OF
static struct of_device_id device_of_match[] = {
  { .compatible = "xlnx,gtp-1.0", },
  { .compatible = "xlnx,evsys-1.0", },
  { .compatible = "xlnx,evg-1.0", },
  { .compatible = "xlnx,evr-1.0", },
  { .compatible = "xlnx,syszq-1.0", },
  { /* end of list */ },
};
MODULE_DEVICE_TABLE(of, device_of_match);
#else
# define device_of_match
#endif

//===============================================================================
//  platform_driver
//===============================================================================
static struct platform_driver pf_driver = {
  .driver = {
    .name  = DRIVER_NAME,
    .owner = THIS_MODULE,
    .of_match_table = device_of_match,
  },
  .probe    = _probe,
  .remove   = _remove,
};


//===============================================================================
//  get_ipidListN
//===============================================================================
static char get_ipidListN(char ipid)
{
  unsigned char i;

  for(i=0; i<IPID_MAX; i++)
  {
    // printk("ipidList[%d] : 0x%02x - %s\n", i, ipidList[i].id, ipidList[i].devName);
    if(ipidList[i].id == 0)
      return -1;

    if(ipidList[i].id == ipid)
      return i;
  }
  return -1;
}



//===============================================================================
//  get_idByMajor
//===============================================================================
static unsigned char get_idByMajor(char major)
{
  unsigned char i;

  for(i=0; i<IPID_MAX; i++)
  {
    if(MAJOR(dInfo[i].devt) == major)
      return i;
  }
  return -1;
}

//===============================================================================
//  mod_init
//===============================================================================
static int __init mod_init(void)
{
  devCount = 0;

  memset (dInfo, 0, sizeof(dInfo));

  printk(KERN_ALERT "Init module [%s ver.%s] !!!!!\n", DRIVER_NAME, TS2IP_VERSION);
  return platform_driver_register(&pf_driver);
}

//===============================================================================
//  mod_exit
//===============================================================================
static void __exit mod_exit(void)
{
  platform_driver_unregister(&pf_driver);
  printk(KERN_ALERT "Exit module [%s ver.%s] !!!!!\n", DRIVER_NAME, TS2IP_VERSION);
}

//===============================================================================
//  module_init, module_exit
//===============================================================================
module_init(mod_init);
module_exit(mod_exit);
