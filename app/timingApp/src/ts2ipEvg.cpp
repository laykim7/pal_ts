//Wrapping class for system calls of timing device driver.
#include <stdlib.h>
#include <termios.h>

#include "ts2ipEvg.h"


namespace timing{

//==============================================================================
//----===@ define
//==============================================================================
#define TS2_REF_CLK_FREQ    81250000

#define A_seqTrg_runState         slv_reg10 // idec_status[30], seqTrgA_runState[1:0], seqTrgB_runState[1:0]
#define A_fifo_evCodeA_STAT       slv_reg11 
#define A_fifo_seqA_STAT          slv_reg12 
#define A_fifo_seqB_STAT          slv_reg13 
#define A_fifo_evUser_STAT        A_ipSys_wFifoStat0

#define A_tx_evCode_cntr          slv_reg14 // {tx_evCodeA_cntr, tx_evCodeB_cntr};
#define A_evgW_time0              slv_reg1e // {evgW_d9, evgW_y7[5:0], evgW_h5, evgW_m6, evgW_s6};  
#define A_evgW_time1              slv_reg1f // {evgW_y7[6], 1'h0, evgW_ms10, evgW_cntr20};

#define A_cfgEvg_sw_evgW0         slv_reg36
#define A_cfgEvg_sw_evgW1         slv_reg37
#define A_cfgEvg_enable           slv_reg38
#define A_cfgEvg_evTrgMsk         slv_reg41
#define A_cfgEvg_seqTrgSel        slv_reg43
#define A_cfgEvg_seqTrgMskA0      slv_reg44
#define A_cfgEvg_seqTrgMskA1      slv_reg45
#define A_cfgEvg_seqTrgMskB0      slv_reg46
#define A_cfgEvg_seqTrgMskB1      slv_reg47
#define A_cfgEvg_seqA_repeatReg   slv_reg48
#define A_cfgEvg_seqB_repeatReg   slv_reg49
#define A_cfgEvg_mxcFreq_00       slv_reg50
#define A_cfgEvg_mxcFreq_01       slv_reg51
#define A_cfgEvg_mxcFreq_02       slv_reg52
#define A_cfgEvg_mxcFreq_03       slv_reg53
#define A_cfgEvg_mxcFreq_04       slv_reg54
#define A_cfgEvg_mxcFreq_05       slv_reg55
#define A_cfgEvg_mxcFreq_06       slv_reg56
#define A_cfgEvg_mxcFreq_07       slv_reg57
#define A_cfgEvg_mxcFreq_08       slv_reg58
#define A_cfgEvg_mxcFreq_09       slv_reg59
#define A_cfgEvg_mxcFreq_10       slv_reg5a
#define A_cfgEvg_mxcFreq_11       slv_reg5b
#define A_cfgEvg_mxcFreq_12       slv_reg5c
#define A_cfgEvg_mxcFreq_13       slv_reg5d

#define A_cfgEvg_evTrgSelA        slv_reg65
#define A_cfgEvg_evTrgSelB        slv_reg66
#define A_cfgEvg_evTrgSelC        slv_reg67
#define A_cfgEvg_evTrgSelD        slv_reg68
#define A_cfgEvg_evTrgSelE        slv_reg69
#define A_cfgEvg_evTrgSelF        slv_reg6a
#define A_cfgEvg_evTrgSelG        slv_reg6b
#define A_cfgEvg_evTrgSelH        slv_reg6c

#define A_cfgEvg_tx_DBusSelA      slv_reg6d
#define A_cfgEvg_tx_DBusSelB      slv_reg6e

#define Mem1_mevCodeA 0x1000
#define Mem2_seqA_0A  0x2000
#define Mem3_seqA_0B  0x3000
#define Mem4_seqA_1A  0x4000
#define Mem5_seqA_1B  0x5000
#define Mem6_seqB_0A  0x6000
#define Mem7_seqB_0B  0x7000
#define Mem8_seqB_1A  0x8000
#define Mem9_seqB_1B  0x9000

#define Mem_evCodeA          Mem1_mevCodeA
#define Mem_seqA_Config      Mem2_seqA_0A
#define Mem_seqA_TimeStamp   Mem4_seqA_1A
#define Mem_seqB_Config      Mem6_seqB_0A
#define Mem_seqB_TimeStamp   Mem8_seqB_1A

#define C_seq_stop(sv)              ((sv & 0x1) << 15)
#define C_seq_evCodeB(sv)           ((sv & 0x7ff) << 0)

//==============================================================================
//----===@ global variable
//==============================================================================
static char evSrcName[EV_SRC_MAX][20] =
{
  {"USER_TRIGGER"},
  {"MXC_00"},    {"MXC_01"},    {"MXC_02"},    {"MXC_03"},
  {"MXC_04"},    {"MXC_05"},    {"MXC_06"},    {"MXC_07"},
  {"MXC_08"},    {"MXC_09"},    {"MXC_10"},    {"MXC_11"},
  {"MXC_12"},    {"MXC_13"},
  {"EXT_IN_SL_00"},    {"EXT_IN_SL_01"},    {"EXT_IN_SL_02"},    {"EXT_IN_SL_03"},
  {"EXT_IN_SL_04"},    {"EXT_IN_SL_05"},    {"EXT_IN_SL_06"},    {"EXT_IN_SL_07"},
  {"EXT_IN_SL_08"},    {"EXT_IN_SL_09"},    {"EXT_IN_SL_10"},    {"EXT_IN_SL_11"},
  {"EXT_IN_SL_12"},    {"EXT_IN_SL_13"},    {"EXT_IN_SL_14"},    {"EXT_IN_SL_15"}
};


//==============================================================================
//----===@ class
//==============================================================================


//=====================================
//----===@ ts2ipEvg
// Parameters  :
// Description :
ts2ipEvg::ts2ipEvg(const char *deviceName, uint tzone, double tickPeriod)
{
  memset(seqA_evCodeB  , 0, sizeof(seqA_evCodeB  ));
  memset(seqA_timeStamp, 0, sizeof(seqA_timeStamp));
  memset(seqB_evCodeB  , 0, sizeof(seqB_evCodeB  ));
  memset(seqB_timeStamp, 0, sizeof(seqB_timeStamp));

  ip_open(deviceName);

  sprintf(evTime.name, "%s_org", devName);
  sprintf(evTimeSub.name, "%s_sub", devName);

  evTime.tzone = tzone;
  evTime.tickPeriod = tickPeriod;
  evTimeSub.tzone = tzone;
  evTimeSub.tickPeriod = tickPeriod;
}

//=====================================
//----===@ ~ts2ipEvg
// Parameters  :
// Description :
ts2ipEvg::~ts2ipEvg()
{
  prnM2("~ts2ipEvg();\r\n");
}

//=====================================
//----===@ enable
// Parameters  :
// Description :
int ts2ipEvg::enable(uint dbus, uint evCodeA, uint evCodeB, uint mevCodeA, uint seqTrgA, uint seqTrgB, uint mxc)
{
  ifRet(fd < 0);

  ifRet(dbus     > 1);
  ifRet(evCodeA  > 1);
  ifRet(evCodeB  > 1);
  ifRet(mevCodeA > 1);
  ifRet(seqTrgA  > 1);
  ifRet(seqTrgB  > 1);
  ifRet(mxc > 1);

  txEn_DBus     = dbus     & 0x01; // slv_reg38[31];
  txEn_evCodeA  = evCodeA  & 0x01; // slv_reg38[30];
  txEn_evCodeB  = evCodeB  & 0x01; // slv_reg38[29];
  txEn_mevCodeA = mevCodeA & 0x01; // slv_reg38[28];
  seqTrgA_en    = seqTrgA  & 0x01; // slv_reg38[27];
  seqTrgB_en    = seqTrgB  & 0x01; // slv_reg38[26];
  mxc14Enable   = mxc      & 0x01; // slv_reg38[25];

  ip_wr(accType_devRaw, A_cfgEvg_enable, (txEn_DBus<<31) | (txEn_evCodeA<<30) | (txEn_evCodeB<<29) | (txEn_mevCodeA<<28)  | (seqTrgA_en<<27)  | (seqTrgB_en<<26)  | (mxc14Enable<<25) );

  return RET_OK;
};


//=====================================
//----===@ getStat
// Parameters  :
// Description :
int ts2ipEvg::getStat(void)
{
  int i;
  uint rdData;

  ifRet(fd < 0);

  //evg status
  ip_rd(accType_devRaw, slv_reg10, &rdData);
  idec_status    = ((rdData >> 30) & 0x1   );
  seqA_isRunning = ((rdData >>  0) & 0x3   );
  seqB_isRunning = ((rdData >>  2) & 0x3   );

  ip_getFifoStat(A_fifo_evCodeA_STAT, &fStat_evCodeA);
  ip_getFifoStat(A_fifo_seqA_STAT   , &fStat_seqA   );
  ip_getFifoStat(A_fifo_seqB_STAT   , &fStat_seqB   );
  ip_getFifoStat(A_fifo_evUser_STAT , &fStat_evUser );

  ip_rd(accType_devRaw, slv_reg16, &rdData);
  tx_evCodeA_cntr = rdData;

  ip_rd(accType_devRaw, slv_reg17, &rdData);
  tx_evCodeB_cntr = rdData;

  ip_rd(accType_devRaw, slv_reg15, &rdData);
  evgW_set_terr_cnt = ((rdData >>  0) & 0xffff);

  ip_rd(accType_devRaw, slv_reg36, &rdData);
  idec_mode    =(rdData>> 0)&0x3; //

  ip_rd(accType_devRaw, slv_reg38, &rdData);
  txEn_DBus     = (rdData>>31)&0x1; //
  txEn_evCodeA  = (rdData>>30)&0x1; //
  txEn_evCodeB  = (rdData>>29)&0x1; //
  txEn_mevCodeA = (rdData>>28)&0x1; //
  seqTrgA_en    = (rdData>>27)&0x1; //
  seqTrgB_en    = (rdData>>26)&0x1; //
  mxc14Enable   = (rdData>>25)&0x1; //

  ip_rd(accType_devRaw, slv_reg41, &rdData);
  evTrgMsk = rdData;

  ip_rd(accType_devRaw, slv_reg43, &rdData);
  seqTrgCmbSrcSel[0] = ((rdData >> 0) & 0xf);
  seqTrgCmbSrcSel[1] = ((rdData >> 4) & 0xf);

  for(i=0;i<4;i++){
    ip_rd(accType_devRaw, slv_reg44 + i*4, &rdData); 
    seqSrcOR[i] = rdData & 0x7fffffff;
  }

  ip_rd(accType_devRaw, slv_reg48, &rdData); seq_repeatReg[0] = rdData;
  ip_rd(accType_devRaw, slv_reg49, &rdData); seq_repeatReg[1] = rdData;

  for(i=0;i<MAX_MXC_NUM;i++){
    ip_rd(accType_devRaw, slv_reg50 + i*4, &rdData);
    mxc14_prescalerReg[i] = rdData;

    if(rdData<2)
      mxcFreq_Hz[i] = 40.625;
    else
      mxcFreq_Hz[i] = (uint)(TS2_REF_CLK_FREQ / rdData);
      // mxcFreq_Hz[i] = (float)(1000000.0f/rdData);
  }

  for(i=0;i<8;i++){
    ip_rd(accType_devRaw, slv_reg65 + i*4, &rdData);
    evTrg[i*4]     = (ev_src)((rdData & 0x000000ff)    );
    evTrg[i*4 + 1] = (ev_src)((rdData & 0x0000ff00)>>8 );
    evTrg[i*4 + 2] = (ev_src)((rdData & 0x00ff0000)>>16);
    evTrg[i*4 + 3] = (ev_src)((rdData & 0xff000000)>>24);
  }

  for(i=0;i<2;i++){
    ip_rd(accType_devRaw, slv_reg6d + i*4, &rdData);
    tx_DBusSel[i*4]     = (ev_src)((rdData & 0x000000ff)    );
    tx_DBusSel[i*4 + 1] = (ev_src)((rdData & 0x0000ff00)>>8 );
    tx_DBusSel[i*4 + 2] = (ev_src)((rdData & 0x00ff0000)>>16);
    tx_DBusSel[i*4 + 3] = (ev_src)((rdData & 0xff000000)>>24);
  }
  return RET_OK;
}


//=====================================
//----===@ prnStat
// Parameters  :
// Description :
int ts2ipEvg::prnStat(void)
{
  // int rtVal;

  ifRet(fd < 0);
  uint i,j;
  uint tmpData;
  uint cmbTmp  = 0 ;
  uint cmbTmp2 = 0 ;

  prnM2("======================================================\r\n");
  prnM2("++ %s prnStat ++\r\n", devName);
  prnM2("------------------------------------------------------\r\n");
  // rtVal = getStat();
  // ifRet(RET_ERR == rtVal);

  prnM2_fmtDec("idec_mode"        , idec_mode, "0:irigb,1:user,2:timingNet");
  prnM2_fmtDec("idec_status      ", idec_status      , "[0:Loss]");
  prnM2_fmtDec("seqA_isRunning   ", seqA_isRunning   , "[0:stop, 1:idle, 2:run, 3:pending]");
  prnM2_fmtDec("seqB_isRunning   ", seqB_isRunning   , "[0:stop, 1:idle, 2:run, 3:pending]");
  prnM2_fmtDec("tx_evCodeA_cntr  ", tx_evCodeA_cntr  , "");
  prnM2_fmtDec("tx_evCodeB_cntr  ", tx_evCodeB_cntr  , "");
  prnM2_fmtDec("evgW_set_terr_cnt", evgW_set_terr_cnt, "counts");
  

  prnM2_fmtDec("txEn_DBus        ", txEn_DBus     , "[1:Enable]");
  prnM2_fmtDec("txEn_evCodeA     ", txEn_evCodeA  , "[1:Enable]");
  prnM2_fmtDec("txEn_evCodeB     ", txEn_evCodeB  , "[1:Enable]");
  prnM2_fmtDec("txEn_mevCodeA    ", txEn_mevCodeA , "[1:Enable]");
  prnM2_fmtDec("seqTrgA_en       ", seqTrgA_en    , "[1:Enable]");
  prnM2_fmtDec("seqTrgB_en       ", seqTrgB_en    , "[1:Enable]");
  prnM2_fmtDec("mxc14Enable      ", mxc14Enable   , "[1:Enable]");

  prnM2("------------------------------------------------------\r\n");
  prnM2("%s\r\n", "mxcFreq_Hz");
  for(i=0;i<MAX_MXC_NUM/2;i++)
    prnM2("[%02d] %10d clk, %10.3lf Hz / [%02d] %10d clk, %10.3lf Hz\r\n"\
    , i    , mxc14_prescalerReg[i    ], mxcFreq_Hz[i    ]\
    , i+7*1, mxc14_prescalerReg[i+7*1], mxcFreq_Hz[i+7*1]\
    );

  prnM2("------------------------------------------------------\r\n");
  prnM2("%s\r\n", "tx_DBusSel");
  for(i=0;i<2;i++)
    prnM2("[%02d] %-16s / [%02d] %-16s / [%02d] %-16s / [%02d] %-16s\r\n"\
    , i    , evSrcName[tx_DBusSel[i    ]]\
    , i+2*1, evSrcName[tx_DBusSel[i+2*1]]\
    , i+2*2, evSrcName[tx_DBusSel[i+2*2]]\
    , i+2*3, evSrcName[tx_DBusSel[i+2*3]]\
    );
    
  
  prnM2("------------------------------------------------------\r\n");
  prnM2("%s\r\n", "evTrg");
  for(i=0;i<7;i++)
    prnM2("[%02d] %-16s / [%02d] %-16s / [%02d] %-16s / [%02d] %-16s\r\n"\
    , i    , evSrcName[evTrg[(i    )]] \
    , i+8*1, evSrcName[evTrg[(i+8*1)]] \
    , i+8*2, evSrcName[evTrg[(i+8*2)]] \
    , i+8*3, evSrcName[evTrg[(i+8*3)]] \
    );
    prnM2("[%02d] %-16s / [%02d] %-16s / [%02d] %-16s\r\n"\
    , i    , evSrcName[evTrg[(i    )]] \
    , i+8*1, evSrcName[evTrg[(i+8*1)]] \
    , i+8*2, evSrcName[evTrg[(i+8*2)]] \
    );

  prnM2("------------------------------------------------------\r\n");
  for(j=0;j<2;j++)
  {
    prnM2("%-16s[%c]      : %d\r\n", "seqTrgCmbSrcSel", "A"+j, seqTrgCmbSrcSel[j]);
    switch(seqTrgCmbSrcSel[j])
    {
      // case seqS_CMB_NONE        :
      case seqS_CMB_OR0         : cmbTmp  = 0;
                                  cmbTmp2 = 4;
                                  break;
      case seqS_CMB_OR1         : cmbTmp  = 1;
                                  cmbTmp2 = 4;
                                  break;
      case seqS_CMB_OR2         : cmbTmp  = 2;
                                  cmbTmp2 = 4;
                                  break; 
      case seqS_CMB_OR3         : cmbTmp  = 3;
                                  cmbTmp2 = 4;
                                  break;
      case seqS_CMB_OR0_and_OR1 : cmbTmp  = 0;
                                  cmbTmp2 = 1;
                                  break;
      case seqS_CMB_OR2_and_OR3 : cmbTmp  = 2;
                                  cmbTmp2 = 3;
                                  break;
      default :                   cmbTmp  = 4;
                                  cmbTmp2 = 4;
                                  break;
    }
    
    prnM2("       (");

    if(cmbTmp<4) {
      tmpData = 1;
      for(i=0;i<31;i++){
        if((tmpData & seqSrcOR[cmbTmp]) > 0) prnM2(" %s |", evSrcName[i]);
        tmpData = tmpData << 1;
      }
    }

    prnM2(")\r\n     & (");

    if(cmbTmp2<4) {
      tmpData = 1;
      for(i=0;i<31;i++){
        if((tmpData & seqSrcOR[cmbTmp2]) > 0) prnM2(" %s |", evSrcName[i]);
        tmpData = tmpData << 1;
      }
    }
    prnM2(")\r\n");
  }
    // Parameters  : seqX_TrgCmbSrc : 0~7 
//                                0 : evg_seq disable
//                                1 : seqSrcOR[0]
//                                2 : seqSrcOR[1]
//                                3 : seqSrcOR[0] & seqSrcOR[1]
//                                4 : evg_seq disable
//                                5 : seqSrcOR[2]
//                                6 : seqSrcOR[3]
//                                7 : seqSrcOR[2] & seqSrcOR[3]
  
  // if(pLevel > 1)
  {
    ip_prnFifoStat(fStat_evCodeA, 1);
    ip_prnFifoStat(fStat_seqA   , 0);
    ip_prnFifoStat(fStat_seqB   , 0);
    ip_prnFifoStat(fStat_evUser , 0);
  }

  prnM2("\r\n");
  return RET_OK;
}




//=====================================
//----===@ setWatch
// Parameters  :
// Description : idec_mode (1:by user, 0:by gen or ext irig-b )
int ts2ipEvg::setWatch(uint idecMode, uint year, uint day, uint hour, uint min, uint sec)
{
  ifRet(fd < 0);

  uint wrData = 0;
  idec_mode = idecMode;

  wrData |= ((year & 0x0000003F) << 17); // 17
  wrData |= ((day  & 0x000001FF) << 23); // 27 bit
  wrData |= ((hour & 0x0000001F) << 12); // 12
  wrData |= ((min  & 0x0000003F) <<  6); // 6
  wrData |= ((sec  & 0x0000003F) <<  0); // 0

  ip_wr(accType_devRaw, A_cfgEvg_sw_evgW0, (idec_mode & 3) | ((year & 0x00000040) << 25) );
  ip_wr(accType_devRaw, A_cfgEvg_sw_evgW1, wrData);

  // if(idec_mode == 1)ip_setCommand(SET_sw_evgW_set_time, 0);
  return RET_OK;
};

//=====================================
//----===@ setTrgMsk
// Parameters  :
// Description :
int ts2ipEvg::setTrgMsk(uint maskVal)
{
  ifRet(fd < 0);

  evTrgMsk = maskVal;
  ip_wr(accType_devRaw, slv_reg41, evTrgMsk & 0x7fffffff);    
  return RET_OK;
};





//=====================================
//----===@ setMxcFreq
// Parameters  :
// Description :
int ts2ipEvg::setMxcFreq(ev_src evSrc, float mxcFreqHz) 
{
  ifRet(fd < 0);
  ifRet(mxcFreqHz < 1.0);
  ifRet(mxcFreqHz > TS2_REF_CLK_FREQ/2);

  uint mxcNum = evSrc - MXC_00;
  ifRet(mxcNum > MAX_MXC_NUM-1);

  //81.250000 MHz
  mxcFreq_Hz[mxcNum] = mxcFreqHz;
  mxc14_prescalerReg[mxcNum] = (uint)(TS2_REF_CLK_FREQ / mxcFreq_Hz[mxcNum]);

  prnM1("setMxcFreq [%d] : %f Hz, %d x12.3ns clock period.\r\n", mxcNum, mxcFreq_Hz[mxcNum], mxc14_prescalerReg[mxcNum]);
  ip_wr(accType_devRaw, slv_reg50 + mxcNum*4, mxc14_prescalerReg[mxcNum]);

  return RET_OK;
};

//=====================================
//----===@ setTrg
// Parameters  :
// Description :
int ts2ipEvg::setTrg(uint evTrgNum, ev_src evSrc)
{
  ifRet(fd < 0);
  ifRet(evTrgNum > 31);
  ifRet(evSrc > EV_SRC_MAX-1);
  
  evTrg[evTrgNum] = evSrc;
  prnM1("set_evg_evTrg : 0x%08x\r\n",(uint)(evTrgNum<<8) | (evSrc & 0x1f));
  
  int offset;
  switch(evTrgNum)
  {
    case  0 : case  1 : case  2 : case  3 : offset =  0; ip_wr(accType_devRaw, A_cfgEvg_evTrgSelA, (evTrg[offset+3]<< 24) | (evTrg[offset+2]<< 16) | (evTrg[offset+1]<< 8) | evTrg[offset]); break;
    case  4 : case  5 : case  6 : case  7 : offset =  4; ip_wr(accType_devRaw, A_cfgEvg_evTrgSelB, (evTrg[offset+3]<< 24) | (evTrg[offset+2]<< 16) | (evTrg[offset+1]<< 8) | evTrg[offset]); break;
    case  8 : case  9 : case 10 : case 11 : offset =  8; ip_wr(accType_devRaw, A_cfgEvg_evTrgSelC, (evTrg[offset+3]<< 24) | (evTrg[offset+2]<< 16) | (evTrg[offset+1]<< 8) | evTrg[offset]); break;
    case 12 : case 13 : case 14 : case 15 : offset = 12; ip_wr(accType_devRaw, A_cfgEvg_evTrgSelD, (evTrg[offset+3]<< 24) | (evTrg[offset+2]<< 16) | (evTrg[offset+1]<< 8) | evTrg[offset]); break;
    case 16 : case 17 : case 18 : case 19 : offset = 16; ip_wr(accType_devRaw, A_cfgEvg_evTrgSelE, (evTrg[offset+3]<< 24) | (evTrg[offset+2]<< 16) | (evTrg[offset+1]<< 8) | evTrg[offset]); break;
    case 20 : case 21 : case 22 : case 23 : offset = 20; ip_wr(accType_devRaw, A_cfgEvg_evTrgSelF, (evTrg[offset+3]<< 24) | (evTrg[offset+2]<< 16) | (evTrg[offset+1]<< 8) | evTrg[offset]); break;
    case 24 : case 25 : case 26 : case 27 : offset = 24; ip_wr(accType_devRaw, A_cfgEvg_evTrgSelG, (evTrg[offset+3]<< 24) | (evTrg[offset+2]<< 16) | (evTrg[offset+1]<< 8) | evTrg[offset]); break;
    case 28 : case 29 : case 30 : case 31 : offset = 28; ip_wr(accType_devRaw, A_cfgEvg_evTrgSelH, (evTrg[offset+3]<< 24) | (evTrg[offset+2]<< 16) | (evTrg[offset+1]<< 8) | evTrg[offset]); break;
    default  : return RET_ERR;
  };

  return RET_OK;
};

//=====================================
//----===@ setDBusSel
// Parameters  :
// Description :
int ts2ipEvg::setDBusSel(uint dBusNum, ev_src evSrc)
{
  ifRet(fd < 0);
  ifRet(dBusNum > 7);
  ifRet(evSrc > EV_SRC_MAX-1);
  
  tx_DBusSel[dBusNum] = evSrc;
  // prnM1("set_evg_tx_DBusSel : 0x%08x\r\n",(uint)(dBusNum<<8) | (evSrc & 0x1f));
  
  int offset;
  switch(dBusNum)
  {
    case  0 : case  1 : case  2 : case  3 : offset =  0; ip_wr(accType_devRaw, slv_reg6d, (tx_DBusSel[offset+3]<< 24) | (tx_DBusSel[offset+2]<< 16) | (tx_DBusSel[offset+1]<< 8) | tx_DBusSel[offset]); break;
    case  4 : case  5 : case  6 : case  7 : offset =  4; ip_wr(accType_devRaw, slv_reg6e, (tx_DBusSel[offset+3]<< 24) | (tx_DBusSel[offset+2]<< 16) | (tx_DBusSel[offset+1]<< 8) | tx_DBusSel[offset]); break;
    default  : return RET_ERR;
  };

  return RET_OK;
};

//=====================================
//----===@ setMevCodeA
// Parameters  :
// Description :
int ts2ipEvg::setMevCodeA(uint trgNum, uint evCode)
{
  ifRet(fd < 0);
  ifRet(trgNum > 30);
  ifRet(evCode > 0x7ff);

  ip_wr(accType_devRaw, Mem_evCodeA + (trgNum+1) * 4 , evCode);
  return RET_OK;
}

//=====================================
//----===@ setSeqTrg
// Parameters  : seqX_TrgCmbSrc : 0~7 
//                                0 : evg_seq disable
//                                1 : seqSrcOR[0]
//                                2 : seqSrcOR[1]
//                                3 : seqSrcOR[0] & seqSrcOR[1]
//                                4 : evg_seq disable
//                                5 : seqSrcOR[2]
//                                6 : seqSrcOR[3]
//                                7 : seqSrcOR[2] & seqSrcOR[3]
//               seqSrcList    : seq trigger source OR value 
//                                 seqS_usrTrg, seqS_mxc00~13, seqS_ext00~15
// Description :
int ts2ipEvg::setSeqTrg( uint seqSrcListA0,   uint seqSrcListA1, \
                         uint seqSrcListB0,   uint seqSrcListB1, \
                         uint seqA_TrgCmbSrc, uint seqB_TrgCmbSrc)
{
  ifRet(fd < 0);
  ifRet(seqA_TrgCmbSrc > seqS_CMB_OR2_and_OR3);
  ifRet(seqB_TrgCmbSrc > seqS_CMB_OR2_and_OR3);
  
  seqTrgCmbSrcSel[0] = seqA_TrgCmbSrc;
  seqTrgCmbSrcSel[1] = seqB_TrgCmbSrc;

  seqSrcOR[0] = seqSrcListA0 & 0x7fffffff;
  seqSrcOR[1] = seqSrcListA1 & 0x7fffffff;
  seqSrcOR[2] = seqSrcListB0 & 0x7fffffff;
  seqSrcOR[3] = seqSrcListB1 & 0x7fffffff;

  ip_wr(accType_devRaw, slv_reg43, (seqTrgCmbSrcSel[0] & 0xf) | ((seqTrgCmbSrcSel[1] & 0xf)<<4) ) ;
  
  ip_wr(accType_devRaw, slv_reg44, seqSrcOR[0]);
  ip_wr(accType_devRaw, slv_reg45, seqSrcOR[1]);
  ip_wr(accType_devRaw, slv_reg46, seqSrcOR[2]);
  ip_wr(accType_devRaw, slv_reg47, seqSrcOR[3]);
  return RET_OK;
};


//=====================================
//----===@ setSeqRepeat
// Parameters  : seqNum 0-A, 1-B
// Description : 
int ts2ipEvg::setSeqRepeat(uint seqNum, uint repeat)
{
  ifRet(fd < 0);
  ifRet(seqNum > 1);

  seq_repeatReg[seqNum] = repeat;
  ip_wr(accType_devRaw, slv_reg48 + seqNum*4, seq_repeatReg[seqNum]);
  return RET_OK;
}

//=====================================
//----===@ setSeqTable
// Parameters  : seqNum 0-A, 1-B
// Description : 
int ts2ipEvg::setSeqTable(uint seqNum, uint offset, uint evCode, uint stopbit, uint timeStamp)
{
  ifRet(fd < 0);
  ifRet(seqNum > 1);
  ifRet(evCode > 0x7ff);
  ifRet(stopbit > 1);

  uint seqMemOffset;
  
  if(seqNum == 0)
    seqMemOffset = 0;
  else
    seqMemOffset = 0x4000;

  ip_wr(accType_devRaw, Mem_seqA_Config + seqMemOffset + (offset*4), C_seq_stop(stopbit) | C_seq_evCodeB(evCode));
  ip_wr(accType_devRaw, Mem_seqA_Config + seqMemOffset + 0x2000 + (offset*4), timeStamp);

  return RET_OK;
}

//=====================================
//----===@ setDevInfo
// Parameters  : 
// Description : 
int ts2ipEvg::setDevInfo(int tsMode, int tsNum, int chkDelay_buf)
{
  ifRet(fd < 0);

  ip_wr(accType_devRaw, slv_reg3f, ((tsMode & 0xf) << 28) | ((tsNum & 0xfff) << 16) | (chkDelay_buf & 0xffff)) ;

  return RET_OK;
}



//=====================================
//----===@ get_evTime
// Parameters  :
// Description :
int ts2ipEvg::get_evTime(void)
{
  ifRet(fd < 0);

  ip_getEvTime(slv_reg1e, slv_reg1e+4, &evTime);
  ip_getEvTime(slv_reg1c, slv_reg1c+4, &evTimeSub);
  
  return RET_OK;
}





} //name space end




