#ifndef   _ts2ipZqsys_H_
#define   _ts2ipZqsys_H_

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
#include "drxiic.h"

using namespace std;

namespace timing
{
  class ts2ipZqsys  : public ts2ipDrv
  {
    private:
    
    protected:

    public:
      ts2ipZqsys(const char *deviceName);
      ~ts2ipZqsys();

      timing::drxiic    *psiic;

      int idt_clk_id;
      int idt_data_id;

      bool sfp_present;   // slv_reg12[8] <= `DLY sfp1_presentN  ;
      bool sfp_txFault;   // slv_reg12[4] <= `DLY sfp1_txFault   ;
      bool sfp_loss;      // slv_reg12[0] <= `DLY sfp1_Loss      ;
      bool sfp_cs0;       // sfp1_cs      <= `DLY slv_reg32[1:0];
      bool sfp_cs1;              

      int getStat(void);
      int prnStat(void);

  };
};

#endif
