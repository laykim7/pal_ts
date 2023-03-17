#ifndef   _tsDev_evx_H_
#define   _tsDev_evx_H_

#include <cstdio>
#include <string>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "commonDefine.h"
#include "tsDev.h"

using namespace std;


namespace timing
{
  class tsDev_evx  : public tsDev
  {
    private:
    
    protected:

    public:
      tsDev_evx(const int opModeV, const int tsModeV, const int tsClassV, const char* tsNameV, const int tsNumV, uint tzone, uint tickPeriod);
      ~tsDev_evx();
    
      timing::ts2ipEvsys *ev[ipIdCntMax];
      timing::ts2ipEvg   *evg[ipIdCntMax];

      int devProc(void);
      int get_evTimeAll(void);
      int get_evTimeStamp(epicsTimeStamp *epsT);

      asynStatus readAppData(uint offset, uint *val);
      asynStatus writeAppData(uint offset, uint val);

      asynStatus readReg(const RegMap &rmap, epicsInt32 &iVal);
      asynStatus readString(const RegMap &rmap, char *cVal);
      asynStatus writeReg(const RegMap &rmap, const epicsInt32 iVal);
      asynStatus writeString(const RegMap &rmap, const char *cVal);

      int  updateInfo(void);
      int  timingNet_timeSync(void);

      void delayCheck_Proc(void);
      int  bist(uint tMode);

  };
};

#endif
