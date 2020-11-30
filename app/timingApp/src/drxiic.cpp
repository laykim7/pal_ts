//Wrapping class for system calls of timing device driver.
#include <stdlib.h>
#include <termios.h>

#include "drxiic.h"

namespace timing{

//==============================================================================
//----===@ define
//==============================================================================

/************************** Constant Definitions *****************************/
#define IIC_SLAVE_FORCE 0x0706
#define IIC_SLAVE       0x0703  /* Change slave address     */
#define IIC_FUNCS       0x0705  /* Get the adapter functionality */
#define IIC_RDWR        0x0707  /* Combined R/W transfer (one stop only)*/
#define MAX_IIC_RW_BUF_SIZE (512)

//==============================================================================
//----===@ global variable
//==============================================================================

//==============================================================================
//----===@ class
//==============================================================================

//=====================================
//----===@ drxiic
// Parameters  :
// Description :
drxiic::drxiic(const char *deviceName)
{
  isOpen = -1;
  ip_open(deviceName);
}

//=====================================
//----===@ ~drxiic
// Parameters  :
// Description :
drxiic::~drxiic()
{
  prnM2("~drxiic();\r\n");
  close(fd);
  isOpen = -1;
}

//=====================================
//----===@ ip_open
// Parameters  :
// Description :
int drxiic::ip_open(const char *deviceName)
{
  sprintf(devName,"%s",deviceName);

  fd  = open(devName,O_RDWR);
  if(fd < 0){
    prnM3("[ERR] drxiic::ip_open : %s\n", devName);
    return RET_ERR;
  }
  prnM3("drxiic::ip_open : %s\n", devName);

  isOpen = 1;
  iic_listN = 0;
  return RET_OK;
}



//=====================================
//----===@ iic_getInfo
// Parameters  :
// Description :
s_drxiic* drxiic::iic_getInfo(int slvid)
{
  // return iic_piic[busId]+slvid;
  return NULL;
};

//=====================================
//----===@ iic_addChip
// Parameters  : 
//               chipName : 
//               i2cAddr  : iic address 
//               muxAddr  : 중간에 mux 칩이 없다면 '0'으로 설정 할 것.
// Description :
int drxiic::iic_addChip(const char *chipName, char i2cAddr, char muxAddr, char muxNum)
{
  if(iic_listN >= drxIIC_maxDrv)
  {
    prnM0("[ERR] L%d [%s] [F:%s]\n", __LINE__ , __FILE__ , __FUNCTION__ );
    return RET_ERR;
  }

  sprintf(iic_list[iic_listN].strName,"%s",chipName);
  printf("iic_list[iic_listN].strName : %s\r\n",iic_list[iic_listN].strName);
  iic_list[iic_listN].name = iic_listN;
  iic_list[iic_listN].addr = i2cAddr;
  iic_list[iic_listN].muxAddress = muxAddr;
  iic_list[iic_listN].muxNum = muxNum;

  return iic_listN++;
};

//=====================================
//----===@ iic_setMux
// Parameters  :
// Description :
int drxiic::iic_setMux(s_drxiic* piic)
{
  // int status;
  // unsigned char WriteBuffer[2];
  // unsigned char BytesWritten;

  // unsigned char ReadBuffer = 0x0; /* Buffer to hold data read.*/
  // unsigned short int BytesToRead;

  // if(piic == NULL)prnErrRet();

  // status = ioctl(lfd, IIC_SLAVE_FORCE, piic->muxAddress);
  // if(status < 0 )prnErrRet();

  // WriteBuffer[0] = piic->muxNum;
  // BytesWritten = write(lfd, WriteBuffer, 1);
  // if(BytesWritten != 1)
  // {
  //   // prnErrRet();
  //   prnM0("[ERR] L%d [%s] [F:%s] W-%s\n", __LINE__ , __FILE__ , __FUNCTION__ , piic->strName);
  //   ip_setCommand(SET_SLH_iic_rst, 100000);// set_sLH_iicReset(100000);
  //   return RET_ERR;
  // }

  // BytesToRead = read(lfd, &ReadBuffer, 1);
  // // if(BytesToRead != 1 )prnErrRet();
  // if(BytesToRead != 1)
  // {
  //   // prnErrRet();
  //   prnM0("[ERR] L%d [%s] [F:%s] R-%s\n", __LINE__ , __FILE__ , __FUNCTION__ , piic->strName);
  //   ip_setCommand(SET_SLH_iic_rst, 100000);// set_sLH_iicReset(100000);
  //   return RET_ERR;
  // }

  return RET_OK;
};


//=====================================
//----===@ iic_wr
// Parameters  :
// Description :
int drxiic::iic_wr(int id, uint offset, uint offset_size, char* pData, int size)
{
  // int i;
  // int lfd;
  // int found = 0;
  int Status = 0;
  unsigned char WriteBuffer[MAX_IIC_RW_BUF_SIZE + 2];
  unsigned char BytesWritten;

  if(pData == NULL       ) prnErrRet();
  if(size == 0           ) prnErrRet();

  // /* check is valid slave.. */
  // for(i = 0; i < iic_slvNum[busId]; i++)
  // {
  //   piic = iic_getInfo(busId, i);
  //   prnM0("   %d.%d \r\t  0x%02x \r\t\t   %d \r\t\t\t   0x%02x \r\t\t\t\t   %d\n", 
  //         i, piic->name, piic->addr, piic->isMux, piic->muxAddress, piic->muxNum );

  //   if(piic->name == (unsigned)slvId)
  //   {
  //     lfd = iic_fd[busId];
  //     found = 1;
  //     break;
  //   }   
  // }

  // /* if found wirte data to iic */
  // if(0 == found)
  //   prnErrRet();

  // if(piic->isMux)
  // {
  //   /* do iic mux control */  
  //   if(!iic_setMux(lfd, piic))
  //     prnErrRet();
  // }

  if(size > MAX_IIC_RW_BUF_SIZE )prnErrRet();

  Status = ioctl(fd, IIC_SLAVE_FORCE, iic_list[id].addr);

  if(Status < 0)prnErrRet();

  if(offset_size > 2)prnErrRet();

  if( offset_size == 1)
  {
    WriteBuffer[0] = (unsigned char)(offset);
  }
  else if( offset_size == 2)
  {
    WriteBuffer[0] = (unsigned char)(offset>>8);
    WriteBuffer[1] = (unsigned char)(offset);
  }

  memcpy(&WriteBuffer[offset_size], pData, size); 
  BytesWritten = write(fd, WriteBuffer, size + offset_size);

  // prnM0("iic_wr : %d,%d,%d,%d,\r\n", lfd, busId, slvId, BytesWritten);

  return BytesWritten-offset_size;
};


//=====================================
//----===@ iic_rd
// Parameters  :
// Description :
int drxiic::iic_rd(int id, uint offset, uint offset_size, char* pData, int size)
{
  // int i;
  // int lfd;
  // int found = 0;
  int Status = 0;
  unsigned char WriteBuffer[2];
  unsigned char BytesWritten = 0;
  unsigned char BytesRead=0;
  // s_drxiic* piic;

  // if(size == 0           ) prnErrRet();

  // /* check is valid slave.. */
  // for(i = 0; i < iic_slvNum[busId]; i++)
  // {
  //   piic = iic_getInfo(busId, i);
  //   if(piic->name == (unsigned)slvId)
  //   {
  //     lfd = iic_fd[busId];
  //     found = 1;
  //     break;
  //   }   
  // }

  // /* if found wirte data to iic */
  // if(!found)
  //   prnErrRet();

  // if(piic->isMux)
  // {
  //   /* do iic mux control */  
  //   if( iic_setMux(lfd, piic ) == RET_ERR )
  //   {
  //     return RET_ERR;
  //   }
  // }

  if(size > MAX_IIC_RW_BUF_SIZE )prnErrRet();

  Status = ioctl(fd, IIC_SLAVE_FORCE, iic_list[id].addr);
  if(Status < 0)prnErrRet();

  if( offset_size > 2)prnErrRet();

  if( offset_size == 1)
  {
    WriteBuffer[0] = (unsigned char)(offset);
  }
  else if( offset_size == 2)
  {
    WriteBuffer[0] = (unsigned char)(offset>>8);
    WriteBuffer[1] = (unsigned char)(offset);
  }

  if( offset_size != 0)
    BytesWritten = write(fd, WriteBuffer, offset_size);

  if(BytesWritten != offset_size)
  {
    // prnErrRet();
    prnM3("[ERR] L%d [%s] [F:%s] %s\n", __LINE__ , __FILE__ , __FUNCTION__ , iic_list[id].strName);
    return RET_ERR;
  }

  BytesRead = read(fd, pData, size);
  return BytesRead;
};


//=====================================
//----===@ iic_prnInfo
// Parameters  :
// Description :
int drxiic::iic_prnInfo()
{
  int i;

  prnM2("-----------------------------------------------\n");
  prnM2("   listN / name / iicAddr / MuxAddr / MuxCh\r\n");
  prnM2("-----------------------------------------------\n");

  for(i = 0; i < iic_listN; i++)
  {
    prnM2("   %-4d / %s / 0x%02x /   0x%02x /   0x%02x /   %d\r\n", 
        i, iic_list[i].strName, iic_list[i].name, iic_list[i].addr, iic_list[i].muxAddress, iic_list[i].muxNum );
  };
  prnM2("\n");

  return RET_OK;
};













//=====================================
//----===@ getStat
// Parameters  :
// Description :
int drxiic::getStat(void)
{
  ifRet(fd < 0);
  // uint rdData;
  
  return RET_OK;
}

//=====================================
//----===@ prnStat
// Parameters  :
// Description :
int drxiic::prnStat(void)
{
  prnM2("======================================================\r\n");
  prnM2("++ %s prnStat ++\r\n", devName);
  prnM2("------------------------------------------------------\r\n");
  
  ifRet(fd < 0);
  getStat();

  // prnM2("%-12s: %d\r\n", "sfp_present", sfp_present);
  // prnM2("%-12s: %d\r\n", "sfp_txFault", sfp_txFault);
  // prnM2("%-12s: %d\r\n", "sfp_loss   ", sfp_loss   );
  // prnM2("%-12s: %d\r\n", "sfp_cs0    ", sfp_cs0    );
  // prnM2("%-12s: %d\r\n", "sfp_cs1    ", sfp_cs1    );

  prnM2("\r\n");

  return RET_OK;
}


} //name space end





