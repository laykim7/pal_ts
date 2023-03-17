#ifndef   _drxiic_H_
#define   _drxiic_H_

#include <cstdio>
#include <string>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "commonDefine.h"

using namespace std;

#define drxIIC_maxDrv (16)
#define drxIIC_maxDrvNameL (15)

struct s_drxiic{
  char strName[64];
  uint name;
  char addr;
  char isMux;
  char muxAddress;
  char muxNum;
};


namespace timing
{
  class drxiic
  {
    private:
    
    protected:
      int ip_open(const char *devName);

    public:
      drxiic(const char *deviceName);
      virtual ~drxiic();

      char devName[64];
      int  fd; 

      int isOpen;
      int iic_listN;
      s_drxiic iic_list[drxIIC_maxDrv];

      s_drxiic* iic_getInfo(int slvid);
      int iic_addChip(const char *chipName, char i2cAddr, char muxAddr, char muxNum);
      int iic_prnInfo();

      int iic_setMux(s_drxiic* piic);
      int iic_wr(int id, uint offset, uint offset_size, char* pData, int size);
      int iic_rd(int id, uint offset, uint offset_size, char* pData, int size);

      //======================================
      virtual int getStat(void);
      virtual int prnStat(void);

  };
};

#endif
