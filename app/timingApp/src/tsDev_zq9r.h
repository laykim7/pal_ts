#ifndef   _tsDev_zq9r_H_
#define   _tsDev_zq9r_H_

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
  class tsDev_zq9r  : public tsDev
  {
    private:
    
    protected:

    public:
      tsDev_zq9r(const int opModeV, const int tsModeV, const int tsClassV, const char* tsNameV, const int tsNumV, uint tzone, uint tickPeriod);
      ~tsDev_zq9r();

      timing::ts2ipZqsys *zq[ipIdCntMax];
      timing::drMpsPico  *mp[ipIdCntMax];

      int devProc(void);
      int get_evTimeAll(void);
      int get_evTimeStamp(epicsTimeStamp *epsT);

      asynStatus readAppData(uint offset, uint *val);
      asynStatus writeAppData(uint offset, uint val);

      asynStatus readReg(const RegMap &rmap, epicsInt32 &iVal);
      asynStatus readString(const RegMap &rmap, char *cVal);
      asynStatus readRegArray(const RegMap &rmap, void *Val);
      asynStatus writeReg(const RegMap &rmap, const epicsInt32 iVal);
      asynStatus writeRegArray(const RegMap &rmap, const void *Val);

      void delayCheck_Proc(void);
      int  bist(uint tMode);

  };
};

#endif
