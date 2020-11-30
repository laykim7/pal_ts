#ifndef   _ts2ipEvr_H_
#define   _ts2ipEvr_H_

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

#define MAX_PDP_NUM      32
#define MAX_OUT_NUM      32

#define C_intrMsk_evrW_alarm_sec    (BIT0)

#define C_intrMsk_evCodeA_01        (BIT1)
#define C_intrMsk_evCodeA_02        (BIT2)
#define C_intrMsk_evCodeA_03        (BIT3)
#define C_intrMsk_evCodeA_04        (BIT4)
#define C_intrMsk_evCodeA_05        (BIT5)
#define C_intrMsk_evCodeA_06        (BIT6)
#define C_intrMsk_evCodeA_07        (BIT7)
#define C_intrMsk_evCodeA_08        (BIT8)
#define C_intrMsk_evCodeA_09        (BIT9)
#define C_intrMsk_evCodeA_0A        (BIT10)
#define C_intrMsk_evCodeA_0B        (BIT11)
#define C_intrMsk_evCodeA_0C        (BIT12)
#define C_intrMsk_evCodeA_0D        (BIT13)
#define C_intrMsk_evCodeA_0E        (BIT14)
#define C_intrMsk_evCodeA_0F        (BIT15)

#define C_intrMsk_Emergency         (BIT16)
#define C_intrMsk_evCodeB_7F0       (BIT16)
#define C_intrMsk_evCodeA_10        (BIT16)

#define C_intrMsk_Emergency_Off     (BIT17)
#define C_intrMsk_evCodeB_7F1       (BIT17)

#define C_intrMsk_evCodeB_7F2       (BIT18) //delay check mode command
#define C_intrMsk_evCodeB_7F3       (BIT19)
#define C_intrMsk_evCodeB_7F4       (BIT20)
#define C_intrMsk_evCodeB_7F5       (BIT21)
#define C_intrMsk_evCodeB_7F6       (BIT22)
#define C_intrMsk_evCodeB_7F7       (BIT23)
#define C_intrMsk_evCodeB_7F8       (BIT24)
#define C_intrMsk_evCodeB_7F9       (BIT25)
#define C_intrMsk_evCodeB_7Fa       (BIT26)
#define C_intrMsk_evCodeB_7Fb       (BIT27)
#define C_intrMsk_evCodeB_7Fc       (BIT28)
#define C_intrMsk_evCodeB_7Fd       (BIT29)
#define C_intrMsk_evCodeB_7Fe       (BIT30)
#define C_intrMsk_evCodeB_7Ff       (BIT31)

/*--- A_setEvr                      slv_reg60 ---*/
#define SET_evrW_rst                (BIT31) // evr watch reset
#define SET_pdp_rst                 (BIT1)
#define SET_evr_setCtrlReg          (BIT0)
#define SET_evr_getStatReg          (BIT4)

enum ev_out{
  EXT_OUT_SL_00, EXT_OUT_SL_01, EXT_OUT_SL_02, EXT_OUT_SL_03,
  EXT_OUT_SL_04, EXT_OUT_SL_05, EXT_OUT_SL_06, EXT_OUT_SL_07,
  EXT_OUT_SL_08, EXT_OUT_SL_09, EXT_OUT_SL_10, EXT_OUT_SL_11,
  EXT_OUT_SL_12, EXT_OUT_SL_13, EXT_OUT_SL_14, EXT_OUT_SL_15,
  EXT_OUT_SH_00, EXT_OUT_SH_01, EXT_OUT_SH_02, EXT_OUT_SH_03,
  EXT_OUT_SH_04, EXT_OUT_SH_05, EXT_OUT_SH_06, EXT_OUT_SH_07,
  EXT_OUT_SH_08, EXT_OUT_SH_09, EXT_OUT_SH_10, EXT_OUT_SH_11,
  EXT_OUT_SH_12, EXT_OUT_SH_13, EXT_OUT_SH_14, EXT_OUT_SH_15,
  EV_OUT_MAX
};

enum ev_out_src{
  EXTOUT30_xps,
  EXTOUT31_1pps,

  RXDBUS_00, RXDBUS_01, RXDBUS_02, RXDBUS_03,
  RXDBUS_04, RXDBUS_05, RXDBUS_06, RXDBUS_07,

  PDP_OUT_00, PDP_OUT_01, PDP_OUT_02, PDP_OUT_03,
  PDP_OUT_04, PDP_OUT_05, PDP_OUT_06, PDP_OUT_07,
  PDP_OUT_08, PDP_OUT_09, PDP_OUT_10, PDP_OUT_11,
  PDP_OUT_12, PDP_OUT_13, PDP_OUT_14, PDP_OUT_15,
  PDP_OUT_16, PDP_OUT_17, PDP_OUT_18, PDP_OUT_19,
  PDP_OUT_20, PDP_OUT_21, PDP_OUT_22, PDP_OUT_23,
  PDP_OUT_24, PDP_OUT_25, PDP_OUT_26, PDP_OUT_27,
  PDP_OUT_28, PDP_OUT_29, PDP_OUT_30, PDP_OUT_31,
  EV_OUT_SRC_MAX
};

namespace timing
{
  class ts2ipEvr  : public ts2ipDrv
  {
    private:
    
    protected:


    public:
      ts2ipEvr(const char *deviceName, uint tzone, double tickPeriod);
      ~ts2ipEvr();

      struct s_pdp{
        char isRunning;
        uint runCnt;
        uint errCnt;
        uint delay  ;
        uint width ;
        uint polarity ;
        uint trgFromEvCodeA;
      };

      struct s_pdp pdp[MAX_PDP_NUM];
      ev_out_src   ext_evOut[EV_OUT_MAX];  // slv_reg62[7:0]; if(rx_DBusSel8[7] == 1'b0)begin exOut32[7] <= `DLY rx_DBus[7]; end else begin exOut32[7] <= `DLY pdpOutput[7]; end

      uint evrRx_linkErr;
      uint evrRx_linkErrCntr;
      uint rx_evCodeA_cntr;     
      uint rx_evCodeB_cntr;     
      uint streamRx_evCodeA_cntr;     
      uint streamRx_evCodeB_cntr;     
      uint emergencyStop;
      uint emergencyStopCntr;
      uint rxTNetEnable;  //rx timing network enable
      uint exOutEnable ;
      uint exOutInvert ;

      uint extOutSwap;         // swapExOut16     slv_reg62[31];
      uint exOut31_1pps;       // extOutSel_1pps  slv_reg62[30];
      uint exOut30_xps;        // 
      uint pdpTrgMode;         // if set 1 : pdp 16~31 are triggered from rx_evCodeA.
                               //            evr_pdpTrgSrcSel  slv_reg62[8];
      uint evLogFifo_En_user ; // slv_reg63[0];

      uint mappingRAM[2048];

      uint evgCmd ;
      uint evgCmdSub ;

      struct s_fifoStat fStat_evLogT0 ;
      struct s_fifoStat fStat_evLogT1 ;
      struct s_fifoStat fStat_evLogEv ;
      struct s_fifoStat fStat_evLogPdp;
      
      drTime_T evTime;
      drTime_T logTime;

      int enable(uint exOut, uint rxTNet );
      int getStat(void);
      int prnStat(void);

      int setEvRam(uint evCode, uint mapData);
      int setEvRamN(uint evCode, uint mapData);

      int cfgPdp(uint pdpNum, uint delay, uint width, uint polarity, uint trgFromEvCodeA);
      int cfgOut(ev_out evOut, ev_out_src evOutSrc);
      int setOut(uint extOutSwap);
      int prnEvLog(void);

      int get_evTime(void);
      int get_evgCommand(void);
  };
};

#endif
