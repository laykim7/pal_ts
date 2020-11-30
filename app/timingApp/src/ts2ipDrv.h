#ifndef   _ts2ipDrv_H_
#define   _ts2ipDrv_H_

#include <cstdio>
#include <string>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "commonDefine.h"
#include <epicsTime.h>

using namespace std;

//-----------------------------------------------------------------------------
// Status : ipSys 
//-----------------------------------------------------------------------------
#define A_ipSys_vendor            slv_reg00 // 32'h44555255, // ascii "DURU"
#define A_ipSys_info              slv_reg01 // {ipid_h8[7:0], s00_axi_ipid_l8, ipversion[15:0]};
#define A_ipSys_buildTime         slv_reg02 // {d9, y6, h5, m6, s6};
#define A_ipSys_clkCntr           slv_reg03 // 
#define A_ipSys_intrReg           slv_reg04 //

#define A_setReg                  slv_reg30 //

#define A_ipSys_setting           slv_reg08 //
#define A_ipSys_config            slv_reg09 //
#define A_ipSys_intrMask          slv_reg0a //
#define A_ipSys_fifoReset         slv_reg0f //
#define A_ipSys_rFifoStat0        slv_reg70 //
#define A_ipSys_rFifoData0        slv_reg71 //
#define A_ipSys_rFifoStat1        slv_reg72 //
#define A_ipSys_rFifoData1        slv_reg73 //
#define A_ipSys_rFifoStat2        slv_reg74 //
#define A_ipSys_rFifoData2        slv_reg75 //
#define A_ipSys_rFifoStat3        slv_reg76 //
#define A_ipSys_rFifoData3        slv_reg77 //
#define A_ipSys_wFifoStat0        slv_reg78 //
#define A_ipSys_wFifoData0        slv_reg79 //
#define A_ipSys_wFifoStat1        slv_reg7a //
#define A_ipSys_wFifoData1        slv_reg7b //
#define A_ipSys_wFifoStat2        slv_reg7c //
#define A_ipSys_wFifoData2        slv_reg7d //
#define A_ipSys_wFifoStat3        slv_reg7e //
#define A_ipSys_wFifoData3        slv_reg7f //

//-----------------------------------------------------------------------------
// Define : ipSys
//-----------------------------------------------------------------------------
#define C_ipSys_intrEnable          (BIT0)
#define C_ipSys_intrClear           (BIT0)

#define C_ipSys_rFifo_RST_0         (BIT0)
#define C_ipSys_rFifo_RST_1         (BIT1)
#define C_ipSys_rFifo_RST_2         (BIT2)
#define C_ipSys_rFifo_RST_3         (BIT3)
#define C_ipSys_wFifo_RST_0         (BIT4)
#define C_ipSys_wFifo_RST_1         (BIT5)
#define C_ipSys_wFifo_RST_2         (BIT6)
#define C_ipSys_wFifo_RST_3         (BIT7)

//================================================
// data access Type :

#define accType_devRaw     (0)
#define accType_app        (1)
#define accType_devCfg     (2)
#define accType_notDefined (3)



namespace timing
{
  class ts2ipDrv 
  {
    private:
    
    protected:
      int ip_open(const char *devName);

    public:
      ts2ipDrv();
      virtual ~ts2ipDrv();
      int isInit;

      struct s_regRW{
        uint offset;     
        uint val;};

      struct s_fifoStat{
        char name[32];
        int WRCOUNT;
        int RDCOUNT;
        int RDERR;
        int WRERR;
        int EMPTY;
        int ALMOSTEMPTY;
        int FULL;
        int ALMOSTFULL;};

      char devName[64];
      int  fd; 
      uint vendor;
      uint ipID;
      uint ipID_sub;
      uint ipVer_major;
      uint ipVer_miner;
      uint ipVer_rev;
      uint ipTick;

      drTime_T buildTime;

      drTime_T tmpTime;

      tm tm_tmpTime;     // tmep reference time yyyy.1.1
      tm tm_epsTime;     // epics time 기준시 /* seconds since 0000 Jan 1, 1990 */
      time_t tt_epsTimeOffset; // convert from mktime


      int ip_rd(uint accType, uint offset, uint *rdData);
      int ip_wr(uint accType, uint offset, uint wrData);

      int ip_getInfo(void);
      int ip_getTick(void);
      int ip_prnInfo(void);
      int ip_prnTime(drTime_T* drT);

      int ip_getEvTime(uint addr1, uint addr2, drTime_T *drT);
      int ip_convEpsTime(drTime_T* drT, epicsTimeStamp* tgT);

      int ip_intrEn(uint wrData);
      int ip_intrClear(void);
      int ip_intrMask(uint wrData);
      int ip_getIntrVal(uint* rdData);

      int ip_setCommand(uint setCommand, uint setDelay);
      
      int ip_getFifoStat(uint addr, struct s_fifoStat *fStat);
      int ip_prnFifoStat(struct s_fifoStat fStat, int prnLabel);

      //======================================
      virtual int getStat(void);
      virtual int prnStat(void);

  };
};

#endif
