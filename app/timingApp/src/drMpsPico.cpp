//Wrapping class for system calls of mpsPico device driver.
#include <stdlib.h>
#include <termios.h>
#include <malloc.h>
#include <sys/mman.h>

#include "drMpsPico.h"
#include "mps_pico.h"


namespace timing{

//==============================================================================
//----===@ define
//==============================================================================
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#define BYTES_PER_LINE  (64)    // this must be a multiple of 32 otherwise it is not compatible with LINUX PAGE SIZE (usually 4kByte)  

// #define DRV_NAME_mpsPico  "/dev/mps_pico" //_0000:05:00.0"
// cdev = device_create(mps_pico8_class, &dev->dev, board->cdevno,
//                      NULL, MOD_NAME "_%s", pci_name(dev));

//==============================================================================
//----===@ global variable
//==============================================================================

//==============================================================================
//----===@ class
//==============================================================================

//=====================================
//----===@ drMpsPico
// Parameters  :
// Description :
drMpsPico::drMpsPico(const char *deviceName, const char *deviceNameBar2)
{
  isOpen = -1;
  ip_open(deviceName, deviceNameBar2);

  tot_readDataBytes = BYTES_PER_LINE*1024;
  readDataBuf = malloc(tot_readDataBytes);

  if (readDataBuf == NULL) {
    perror("malloc()");
    return;
  }
  size_t allocated_bytes = malloc_usable_size(readDataBuf);
  printf("Byte requested: %lu, Allocated: %lu\n", (long unsigned int) tot_readDataBytes, (long unsigned int) allocated_bytes);
}

//=====================================
//----===@ ~drMpsPico
// Parameters  :
// Description :
drMpsPico::~drMpsPico()
{
  prnM2("~drMpsPico();\r\n");
  close(fd);
  isOpen = -1;
  free(readDataBuf);
}

//=====================================
//----===@ ip_open
// Parameters  :
// Description :
int drMpsPico::ip_open(const char *deviceName, const char *deviceNameBar2)
{
  sprintf(devName,"%s",deviceName);

  fd  = open(devName, O_RDWR | O_SYNC);
  if(fd < 0){
    prnM3("[ERR] drMpsPico::ip_open : %s\n", devName);
    return RET_ERR;
  }
  prnM3("drMpsPico::ip_open : %s\n", devName);

  sprintf(devNameBar2,"%s",deviceNameBar2);

  isOpen = 1;
  return RET_OK;
}

//=====================================
//----===@ readData
// Parameters  :
// Description :
int drMpsPico::readData(void)
{
  ifRet(fd < 0);

  ssize_t bytes_read = read(fd, readDataBuf, tot_readDataBytes);
  printf("bytes_read : %08d/ tot_readDataBytes : %08d\r\n", bytes_read, tot_readDataBytes);

  return RET_OK;
}

//=====================================
//----===@ ip_rd
// Parameters  :
// Description :
int drMpsPico::bar2_rd(off_t offset, uint nr_samp, float *rdData)
{
  void *map_base, *virt_addr;
  //off_t base_address;
  int fdBar2; 
  unsigned long read_result;
  // float read_float;

  //base_address = strtoul(BUFFER1_OFFSET, 0, 0);
  //target = strtoul(BUFFER1_OFFSET, 0, 0);
  fdBar2 = open(devNameBar2, O_RDWR | O_SYNC);
  if (fdBar2<0) {
    perror("open()");
    return RET_ERR;
  }
    printf("%s opened.\n", devNameBar2);

    /* Map one page */
    printf("mmap(%d, %lu, 0x%x, 0x%x, %d, 0x%lx)\n", 0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fdBar2,  offset & ~MAP_MASK);
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fdBar2, offset & ~MAP_MASK);
    if(map_base == (void *) -1) prnErrRet();

    virt_addr = map_base + (offset & MAP_MASK);
    printf("PCI Memory mapped to address %p_%p.\n", map_base, virt_addr);

    // read
    read_result = *((uint32_t *) virt_addr);
    printf("Value at virtual address %p: 0x%lX\n", virt_addr, read_result);

    // read_float = *((float *) virt_addr);
    // printf("Value at virtual address %p: %+0.6e\n", virt_addr, read_float);

  for (unsigned long i = 0; i < 8; i++)
    printf("Value at virtual address %p: %+0.6e\n", virt_addr+i*4, *((float *) (virt_addr+i*4)));


  for (unsigned long i = 0; i < nr_samp; i++)
    *rdData++ = *((float *) (virt_addr+i*4));
  // memcpy((void*)rdData,virt_addr,nr_samp);

  if(munmap(map_base, MAP_SIZE) == -1) perror("munmap");;

  close(fdBar2);


  // fd_bar2  = open(deviceNameBar2, O_RDWR | O_SYNC);
  // if(fd_bar2 < 0){
  //   prnM3("[ERR] drMpsPico::ip_open[fd_bar2] : %s\n", deviceNameBar2);
  //   return RET_ERR;
  // }
  // prnM3("drMpsPico::ip_open[fd_bar2] : %s\n", deviceNameBar2);
  

  // printf("offset & ~MAP_MASK : 0x%08x - 0x%08x\r\n", 0x8000 & ~MAP_MASK , 0x8000 & MAP_MASK);
  // printf("offset & ~MAP_MASK : 0x%08x - 0x%08x\r\n", 0x18000 & ~MAP_MASK , 0x18000 & MAP_MASK);

  // mapSize = MAP_SIZE;
  // // // bar2 = mmap(NULL, MAP_SIZE, PROT_READ ,  MAP_PRIVATE | MAP_ANON, fd_bar2, 0);
  // bar2 = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bar2, 0x8000);
  // if(bar2 == (void *) -1)
  // {
  //   perror("mmap");
  // } 



  // struct s_regRW reg;
  // reg.offset = offset;
  // reg.val    = -1;
  
  // if (ioctl(fd, _IOWR(MPS_PICO_MAGIC, 5, struct s_regRW), &reg) < 0) 
  //   prnErrRet();

  // *rdData = reg.val;
  // prnM2("drMpsPico::ip_rd : %d, %+0.6e\r\n", *rdData, *(float*)rdData);


  // prnM3("drMpsPico::bar2_rd offset : 0x%08x, 0x%08x\r\n", offset, bar2+offset);
  // ifRet(fd_bar2 < 0);

  // *rdData = *((uint *) (bar2));
  // prnM2("drMpsPico::ip_rd : %d, %+0.6e\r\n", *rdData, *(float*)rdData);

  // void *map_base, *virt_addr;
  // offset = 0x8000;

  // /* Map one page */
  // printf("mmap(%d, %lu, 0x%x, 0x%x, %d, 0x%lx)\n", 0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_bar2, offset);
  // map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, fd_bar2, offset & ~MAP_MASK);
  // if(map_base == (void *) -1) printf("map_base error 1\r\n");
  // printf("PCI Memory mapped to address %p.\n", map_base);

  // virt_addr = map_base + (offset & MAP_MASK);

  // // read
  // read_result = *((uint32_t *) virt_addr);
  // printf("Value at virtual address %p: 0x%lX\n", virt_addr, read_result);

  // read_float = *((float *) virt_addr);
  // printf("Value at virtual address %p: %+0.6e\n", virt_addr, read_float);

  // for (unsigned long i = 0; i < 10; i++)
  // {
  //   // *buf++ = *((float *) (virt_addr+i*4));
  //   read_float = *((float *) virt_addr);
  //   printf("Value at virtual address %p: %+0.6e\n", virt_addr, read_float);
  // }

  // if(munmap(map_base, MAP_SIZE) == -1) printf("map_base error 2\r\n");






  return RET_OK;
}

//=====================================
//----===@ ip_rd
// Parameters  :
// Description :
int drMpsPico::ip_rd(uint accType, uint offset, uint *rdData)
{
  // prnM2("drMpsPico::ip_rd offset : %d / 0x%08x\r\n", accType, offset);
  ifRet(fd < 0);
  ifRet(accType > 2);
  ifRet(accType == 1);

  struct s_regRW reg;
  reg.offset = offset;
  reg.val    = -1;
  
  if (ioctl(fd, _IOWR(MPS_PICO_MAGIC, 1 + accType, struct s_regRW), &reg) < 0) 
    prnErrRet();

  *rdData = reg.val;
  // prnM2("drMpsPico::ip_rd : %d / 0x%08x / %d\r\n", accType, offset, reg.val);
  return RET_OK;
}

//=====================================
//----===@ ip_wr
// Parameters  :
// Description :
int drMpsPico::ip_wr(uint accType, uint offset, uint wrData)
{
  ifRet(fd < 0);
  ifRet(accType > 2);
  ifRet(accType == 1);

  struct s_regRW reg;
  reg.offset = offset;
  reg.val    = wrData;

  if (ioctl(fd, _IOWR(MPS_PICO_MAGIC, 2 + accType, struct s_regRW), &reg) < 0) 
    prnErrRet();

  return RET_OK;
}

//=====================================
//----===@ getStat
// Parameters  :
// Description :
int drMpsPico::getStat(void)
{
  ifRet(fd < 0);
  // uint rdData;
  
  return RET_OK;
}

//=====================================
//----===@ prnStat
// Parameters  :
// Description :
int drMpsPico::prnStat(void)
{
  prnM2("======================================================\r\n");
  prnM2("++ %s prnStat ++\r\n", devName);
  prnM2("------------------------------------------------------\r\n");
  
  ifRet(fd < 0);
  getStat();

  prnM2("\r\n");

  return RET_OK;
}


} //name space end





// #define  ABORT_READ  _IO  MPS_PICO_MAGIC  81  
// #define  "GET_VERSION  "  _IOR  MPS_PICO_MAGIC  10  uint32_t
// #define  "GET_FSAMP  "  _IOR  MPS_PICO_MAGIC  13  uint32_t
// #define  "GET_RANGE  "  _IOR  MPS_PICO_MAGIC  14  uint8_t
// #define  "GET_MPS_CONFIG  "  _IOR  MPS_PICO_MAGIC  25  uint32_t
// #define  "GET_B_TRANS  "  _IOR  MPS_PICO_MAGIC  40  uint32_t
// #define  GET_SITE_ID  _IOR  MPS_PICO_MAGIC  91  uint32_t
// #define  GET_SITE_VERSION  _IOR  MPS_PICO_MAGIC  92  uint32_t
          
// #define  "SET_TRG_NRSAMP    "  _IOW  MPS_PICO_MAGIC  54  uint32_t
// #define  "SET_TRG_DELAY    "  _IOW  MPS_PICO_MAGIC  56  uint32_t
// #define  "SET_RING_BUF  "  _IOW  MPS_PICO_MAGIC  60  uint32_t
// #define  SET_SITE_MODE  _IOW  MPS_PICO_MAGIC  92  uint32_t
// #define  "SET_RANGE  "  _IOWR  MPS_PICO_MAGIC  11  uint8_t
// #define  "SET_FSAMP  "  _IOWR  MPS_PICO_MAGIC  12  uint32_t
// #define  "SET_MPS_CONFIG  "  _IOWR  MPS_PICO_MAGIC  26  uint32_t
// #define  SET_MPS_CTRL  _IOWR  MPS_PICO_MAGIC  27  uint32_t
// #define  "GET_TRG_CNTR_1M    "  _IOWR  MPS_PICO_MAGIC  52  uint32_t
// #define  "GET_TRG_CNTR_1K    "  _IOWR  MPS_PICO_MAGIC  53  uint32_t
// #define  "GET_TRG_NRSAMP    "  _IOWR  MPS_PICO_MAGIC  55  uint32_t
// #define  "GET_TRG_DELAY    "  _IOWR  MPS_PICO_MAGIC  57  uint32_t
// #define  "SET_GATE_MUX  "  _IOWR  MPS_PICO_MAGIC  70  uint32_t
// #define  "SET_CONV_MUX  "  _IOWR  MPS_PICO_MAGIC  80  uint32_t
