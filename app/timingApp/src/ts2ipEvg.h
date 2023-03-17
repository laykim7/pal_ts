#ifndef   _ts2ipEvg_H_
#define   _ts2ipEvg_H_

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

#define MAX_MXC_NUM      14


#define IDECODE_MODE_IGEN  0 // 0 : irigb decode mode
#define IDECODE_MODE_USER  1 // 1 : user time set mode
#define IDECODE_MODE_TNET  2 // 2 : timing network

/*--- A_setSysEvg                   slv_reg30 ---*/
#define SET_evgW_rst                (BIT31)
#define SET_idec_rst                (BIT28)
#define SET_SLH_iic_rst             (BIT8)
#define SET_tx_DBusSel_set          (BIT7)
#define SET_evSrc_set               (BIT6)
#define SET_usrTrg                  (BIT5)
#define SET_sw_evgW_set_time        (BIT2) 
#define SET_seqTrgA_resume          (BIT1)
#define SET_seqTrgB_resume          (BIT0)

/*--- sequence trigger source combination ---*/
#define seqS_CMB_NONE          0 
#define seqS_CMB_OR0           1
#define seqS_CMB_OR1           2
#define seqS_CMB_OR0_and_OR1   3
#define seqS_CMB_OR2           5
#define seqS_CMB_OR3           6
#define seqS_CMB_OR2_and_OR3   7

/*--- sequence trigger source define ---*/
// evSrc = {1'h0, exIn32[15:0], mxc14[13:0], usrTrg};
#define seqS_usrTrg  BIT0 
#define seqS_mxc00   BIT1 
#define seqS_mxc01   BIT2 
#define seqS_mxc02   BIT3 
#define seqS_mxc03   BIT4 
#define seqS_mxc04   BIT5 
#define seqS_mxc05   BIT6 
#define seqS_mxc06   BIT7 
#define seqS_mxc07   BIT8 
#define seqS_mxc08   BIT9 
#define seqS_mxc09   BIT10
#define seqS_mxc10   BIT11
#define seqS_mxc11   BIT12
#define seqS_mxc12   BIT13
#define seqS_mxc13   BIT14
#define seqS_ext00   BIT15
#define seqS_ext01   BIT16
#define seqS_ext02   BIT17
#define seqS_ext03   BIT18
#define seqS_ext04   BIT19
#define seqS_ext05   BIT20
#define seqS_ext06   BIT21
#define seqS_ext07   BIT22
#define seqS_ext08   BIT23
#define seqS_ext09   BIT24
#define seqS_ext10   BIT25
#define seqS_ext11   BIT26
#define seqS_ext12   BIT27
#define seqS_ext13   BIT28
#define seqS_ext14   BIT29
#define seqS_ext15   BIT30

enum ev_src{
  USER_TRIGGER,

  MXC_00, MXC_01, MXC_02, MXC_03,
  MXC_04, MXC_05, MXC_06, MXC_07,
  MXC_08, MXC_09, MXC_10, MXC_11,
  MXC_12, MXC_13,
  
  EXT_IN_SL_00, EXT_IN_SL_01, EXT_IN_SL_02, EXT_IN_SL_03,
  EXT_IN_SL_04, EXT_IN_SL_05, EXT_IN_SL_06, EXT_IN_SL_07,
  EXT_IN_SL_08, EXT_IN_SL_09, EXT_IN_SL_10, EXT_IN_SL_11,
  EXT_IN_SL_12, EXT_IN_SL_13, EXT_IN_SL_14, EXT_IN_SL_15,
  EV_SRC_MAX
};

namespace timing
{
  class ts2ipEvg  : public ts2ipDrv
  {
    private:
    
    protected:

    public:
      ts2ipEvg(const char *deviceName, uint tzone, double tickPeriod);
      ~ts2ipEvg();

      struct s_seqTable{
        uint stopbit  ;
        uint evOption ;
        uint evCodeB  ;
        uint timeStamp;
      } ;

      uint idec_mode; //1: user define time, 0:idec time
      uint idec_status;     
      uint seqA_isRunning;
      uint seqB_isRunning;
      uint tx_evCodeA_cntr;     
      uint tx_evCodeB_cntr;     
      uint evrW_set_terr_cnt;
      uint evgW_set_terr_cnt;
      uint mxc14_prescalerReg[MAX_MXC_NUM];
      uint seqTrgCmbSrcSel[2]; //seq trigger combination source select seqS_CMB_NONE ~ seqS_CMB_OR2_and_OR3
      uint seqSrcOR[4];        //seq trigger source OR value (seqS_usrTrg, seqS_mxc00~13, seqS_ext00~31)
      uint seq_repeatReg[2];
      uint txEn_DBus     ; // slv_reg38[31];
      uint txEn_evCodeA  ; // slv_reg38[30];
      uint txEn_evCodeB  ; // slv_reg38[29];
      uint txEn_mevCodeA ; // slv_reg38[28];
      uint seqTrgA_en    ; // slv_reg38[27];
      uint seqTrgB_en    ; // slv_reg38[26];
      uint mxc14Enable   ; // slv_reg38[25];
      uint evTrgMsk;

      uint seqA_evCodeB[2048];
      uint seqA_timeStamp[2048];
      uint seqB_evCodeB[2048];
      uint seqB_timeStamp[2048];

      float  mxcFreq_Hz[MAX_MXC_NUM];
      ev_src evTrg[32];
      ev_src tx_DBusSel[8];

      struct s_fifoStat fStat_evCodeA;
      struct s_fifoStat fStat_seqA;
      struct s_fifoStat fStat_seqB;
      struct s_fifoStat fStat_evUser;

      drTime_T evTime;
      drTime_T evTimeSub;

      int enable(uint dbus, uint evCodeA, uint evCodeB, uint mevCodeA, uint seqTrgA, uint seqTrgB, uint mxc);
      int getStat(void);
      int prnStat(void);

      int setWatch(uint idecMode, uint year, uint day, uint hour, uint min, uint sec);
      int setMxcFreq(ev_src evSrc, float mxcFreqHz) ;
      int setTrg(uint evTrgNum, ev_src evSrc);
      int setTrgMsk(uint maskVal);
      int setDBusSel(uint dBusNum, ev_src evSrc);
      int setMevCodeA(uint trgNum, uint evCode);
      int setSeqTrg(uint seqSrcListA0, uint seqSrcListA1,uint seqSrcListB0,   uint seqSrcListB1,uint seqA_TrgCmbSrc, uint seqB_TrgCmbSrc);
      int setSeqRepeat(uint seqNum, uint repeat);
      int setSeqTable(uint seqNum, uint offset, uint evCode, uint stopbit, uint timeStamp);
      int setDevInfo(int tsMode, int tsNum, int chkDelay_buf);

      int get_evTime(void);

  };
};

#endif
