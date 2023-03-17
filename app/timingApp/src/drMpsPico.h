#ifndef   _drMpsPico_H_
#define   _drMpsPico_H_

#include <cstdio>
#include <string>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "commonDefine.h"

//================================================
// data access Type :
#define accType_devRaw     (0)
#define accType_app        (1)
#define accType_devCfg     (2)
#define accType_notDefined (3)



using namespace std;

namespace timing
{
  class drMpsPico
  {
    private:
    
    protected:
      int ip_open(const char *devName, const char *deviceNameBar2);

    public:
      drMpsPico(const char *deviceName, const char *deviceNameBar2);
      virtual ~drMpsPico();
      
      struct s_regRW{
        uint offset;     
        uint val;};      

      int ip_rd(uint accType, uint offset, uint *rdData);
      int ip_wr(uint accType, uint offset, uint wrData);

      char devName[64];
      char devNameBar2[128];
      int fd; 
      void *bar2, *virt_bar2;
      size_t mapSize;
      unsigned long read_result;
      float read_float;


      int isOpen;

      void *readDataBuf;
      size_t tot_readDataBytes;


      int bar2_rd(off_t offset, uint nr_samp, float *rdData);
      int readData(void);

      //======================================
      virtual int getStat(void);
      virtual int prnStat(void);

  };
};

#endif
