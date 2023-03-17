#ifndef   _tsDev_H_
#define   _tsDev_H_

#include <cstdio>
#include <string>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "xadc_core.h"

#include "verMan.h"

#include "ts2ipEvsys.h"
#include "ts2ipZqsys.h"
#include "ts2ipGtp.h"
#include "ts2ipEvg.h"
#include "ts2ipEvr.h"
#include "drMpsPico.h"
#include "commonDefine.h"

using namespace std;

//================================================
// device driver name
#define DRV_NAME_ip_evsys  "/dev/evsys0"
#define DRV_NAME_ip_zqsys  "/dev/zqsys0"
#define DRV_NAME_ip_gtp    "/dev/gtp0"
#define DRV_NAME_ip_evg0   "/dev/evg0"
#define DRV_NAME_ip_evg1   "/dev/evg1"
#define DRV_NAME_ip_evr0   "/dev/evr0"
#define DRV_NAME_ip_evr1   "/dev/evr1"

// #define DRV_NAME_mp_4 "/dev/mps_pico_0000:05:00.0"
// #define DRV_NAME_mp_x "/dev/mps_pico_0000:"

//================================================
// ipId :
#define ipId_sys    0
#define ipId_gtp    1
#define ipId_evr    2
#define ipId_evg    3
#define ipId_ev     4
#define ipId_zq     5
#define ipId_mp     6

#define ipIdCntMax  5

//================================================
// dataType :
#define dataType_Nomal         0	
#define dataType_Config_CPS    1	
#define dataType_Trg_Seq       2	
#define dataType_Pulse         3	
#define dataType_GetSet        4	
#define dataType_TimeYDMS      5     //(year day min sec)
#define dataType_TimeYDMS64    6     //(year day min sec ms ns, 64bit time)
#define dataType_TimeYMDMS     7     //(year month day min sec)
#define dataType_Waveform_int  8	
#define dataType_Waveform_f32  9	
#define dataType_Waveform_f64  10	
#define dataType_PortFreq      11	   // port freq config type add 2022.11.08
#define dataType_pvValSaveCmd  12	   // pv value save to ini file command add 2022.11.08

//================================================
// addrType :
#define addrType_bar0   0	
#define addrType_bar1   1	
#define addrType_bar2   2	
#define addrType_dma    3	

namespace timing
{
  class tsDev
  {
    private:
    
    protected:

    public:
      tsDev();
      virtual ~tsDev();
      int isInit;

      verMan *ver_sw;
      verMan *ver_boot;
      verMan *ver_image;
      verMan *ver_pv;

      int    opMode;
      int    tsMode;
      int    tsClass;
      char   tsName[64];
      int    tsNum;

      
      struct timeval current_time;

      struct xadc_cmdList xadcCmd[EParamMax];
      int isInitXADC;
      int initXADC(void);
      int getXADC(void);
      int prnXADC(void);

      int get_strTime(drTime_T *evTime, char* strT);

      timing::ts2ipGtp *gtp[ipIdCntMax];
      timing::ts2ipEvg *evg[ipIdCntMax];
      timing::ts2ipEvr *evr[ipIdCntMax];
      timing::ts2ipEvr *evrMain;
      timing::ts2ipEvr *evrSub;

      int devThread(void);
      virtual int devProc(void);

      virtual asynStatus readAppData(uint offset, uint *val);
      virtual asynStatus writeAppData(uint offset, uint val);

      virtual asynStatus readReg(const RegMap &rmap, epicsInt32 &iVal);
      virtual asynStatus readRegArray(const RegMap &rmap, void *Val);
      virtual asynStatus readString(const RegMap &rmap, char *cVal);
      virtual asynStatus writeReg(const RegMap &rmap, const epicsInt32 iVal);
      virtual asynStatus writeRegArray(const RegMap &rmap, const void *Val);
      virtual asynStatus writeString(const RegMap &rmap, const char *cVal);

      virtual int updateInfo(void);
      virtual int timingNet_timeSync(void);

  };
};

#endif
