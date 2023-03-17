#ifndef   _ts2ipGtp_H_
#define   _ts2ipGtp_H_

#include <cstdio>
#include <string>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "commonDefine.h"
#include "ts2ipDrv.h"

using namespace std;

namespace timing
{
  class ts2ipGtp  : public ts2ipDrv
  {
    private:
    
    protected:

    public:
      ts2ipGtp(const char *deviceName);
      ~ts2ipGtp();

      struct s_gtpStat{
        int cpllfbclklost  ;
        int cplllock       ;
        int txresetdone    ;
        int txfsmresetdone ;
        int rxresetdone    ;
        int rxfsmresetdone ;
        int track_data     ;
        int trackLossCnt   ;
        int txClk_cntr     ;
      };

      struct s_gtpStat stat[2];

      int getStat(void);
      int prnStat(void);
    
      int reset(void);
  };
};

#endif
