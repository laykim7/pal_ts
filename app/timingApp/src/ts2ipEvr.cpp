//Wrapping class for system calls of timing device driver.
#include <stdlib.h>
#include <termios.h>

#include "ts2ipEvr.h"

namespace timing{

//==============================================================================
//----===@ define
//==============================================================================
//-----------------------------------------------------------------------------
// Status : EVR
//-----------------------------------------------------------------------------
#define A_rx_evCode_cntr          slv_reg20 // {rx_evCodeA_cntr, rx_evCodeB_cntr};
#define A_streamRx_evCode_cntr    slv_reg21 // {streamRx_evCodeA_cnt, streamRx_evCodeB_cnt};
#define A_emergencyStopCntr       slv_reg22 // emergencyStopCntr[15:0]
#define A_pdpRunStt               slv_reg23

#define A_evrW_time0              slv_reg1e // {evrW_d9, evrW_y7[5:0], evrW_h5, evrW_m6, evrW_s6};  
#define A_evrW_time1              slv_reg1f // {evrW_y7[6], 1'h0, evrW_ms10, evrW_cntr20};
#define A_evrRx_linkErr           slv_reg26 // 
#define A_evrRx_linkErrCntr       slv_reg27 // 

//-----------------------------------------------------------------------------
// Control : EVR
//-----------------------------------------------------------------------------
#define A_setEvr                  slv_reg60 //
#define SET_evrW_rst              (BIT31) // evr watch reset
#define SET_pdp_rst               (BIT1)
#define SET_evr_setCtrlReg        (BIT0)
#define SET_evr_getStatReg        (BIT4)

#define A_cfgEvr_enable           slv_reg38 //
#define A_cfgEvr_rx_DBusSel       slv_reg62 //
#define A_cfgEvr_evLogFifo_Enable slv_reg63 // slv_reg63[0];
#define A_cfgEvr_pdpPolarity      slv_reg64 //

#define MemA_EVR_0        0x1000
#define MemB_EVR_1        0x2000
#define MemA_EVR_0n       0x3000
#define MemB_EVR_1n       0x4000

#define MemC_EVR_pdpDelay 0x5400          //ctrl
#define MemC_EVR_pdpWidth 0x5480          //ctrl

#define MemD_EVR_xpc      0x6000

//-----------------------------------------------------------------------------
// Read, Write Fifo 
//-----------------------------------------------------------------------------
#define A_fifo_evLogT0_STAT  A_ipSys_rFifoStat0
#define A_fifo_evLogT1_STAT  A_ipSys_rFifoStat1
#define A_fifo_evLogEv_STAT  A_ipSys_rFifoStat2
#define A_fifo_evLogPdp_STAT A_ipSys_rFifoStat3

#define A_get_evLogT0        A_ipSys_rFifoData0
#define A_get_evLogT1        A_ipSys_rFifoData1
#define A_get_evLogEv        A_ipSys_rFifoData2
#define A_get_evLogPdp       A_ipSys_rFifoData3

//==============================================================================
//----===@ global variable
//==============================================================================
static char evOutSrcName[EV_OUT_SRC_MAX][20] ={
  {"EXTOUT30_xps"},
  {"EXTOUT31_1pps"},
  {"RXDBUS_00"},   {"RXDBUS_01"},   {"RXDBUS_02"},   {"RXDBUS_03"},   
  {"RXDBUS_04"},   {"RXDBUS_05"},   {"RXDBUS_06"},   {"RXDBUS_07"},
  {"PDP_OUT_00"},  {"PDP_OUT_01"},  {"PDP_OUT_02"},  {"PDP_OUT_03"},
  {"PDP_OUT_04"},  {"PDP_OUT_05"},  {"PDP_OUT_06"},  {"PDP_OUT_07"},
  {"PDP_OUT_08"},  {"PDP_OUT_09"},  {"PDP_OUT_10"},  {"PDP_OUT_11"},
  {"PDP_OUT_12"},  {"PDP_OUT_13"},  {"PDP_OUT_14"},  {"PDP_OUT_15"},
  {"PDP_OUT_16"},  {"PDP_OUT_17"},  {"PDP_OUT_18"},  {"PDP_OUT_19"},
  {"PDP_OUT_20"},  {"PDP_OUT_21"},  {"PDP_OUT_22"},  {"PDP_OUT_23"},
  {"PDP_OUT_24"},  {"PDP_OUT_25"},  {"PDP_OUT_26"},  {"PDP_OUT_27"},
  {"PDP_OUT_28"},  {"PDP_OUT_29"},  {"PDP_OUT_30"},  {"PDP_OUT_31"} };

//==============================================================================
//----===@ class
//==============================================================================


//=====================================
//----===@ ts2ipEvr
// Parameters  :
// Description :
ts2ipEvr::ts2ipEvr(const char *deviceName, uint tzone, double tickPeriod)
{
  memset(mappingRAM  , 0, sizeof(mappingRAM  ));

  ip_open(deviceName);

  sprintf(fStat_evLogT0.name  , "logT0");
  sprintf(fStat_evLogT1.name  , "logT1");
  sprintf(fStat_evLogEv.name  , "logEv");
  sprintf(fStat_evLogPdp.name , "logPdp");

  sprintf(evTime.name, "%s", devName);
  evTime.tzone = tzone;
  evTime.tickPeriod = tickPeriod;
  sprintf(logTime.name, "%s log", devName);
  logTime.tzone = tzone;
  logTime.tickPeriod = tickPeriod;
}

//=====================================
//----===@ ~ts2ipEvr
// Parameters  :
// Description :
ts2ipEvr::~ts2ipEvr()
{
  prnM2("~ts2ipEvr();\r\n");
}


//=====================================
//----===@ enable
// Parameters  :
// Description :
int ts2ipEvr::enable(uint exOut, uint rxTNet )
{
  int rtVal;

  if(fd < 0) prnErrRet();
  ifRet(exOut > 1);
  ifRet(rxTNet > 1);

  exOutEnable  = exOut & 0x1;
  rxTNetEnable = rxTNet & 0x1;

  ip_wr(accType_devRaw, slv_reg38, exOutEnable );
  ip_wr(accType_devRaw, slv_reg39, rxTNetEnable );

  return rtVal;
};

//=====================================
//----===@ getStat
// Parameters  :
// Description :
int ts2ipEvr::getStat(void)
{
  uint rdData;
  int i;

  if(fd < 0) prnErrRet();

  // status -----------------------------------------------
  ip_rd(accType_devRaw, slv_reg10, &rdData);
  rx_evCodeA_cntr = rdData;

  ip_rd(accType_devRaw, slv_reg11, &rdData);
  rx_evCodeB_cntr = rdData;

  ip_rd(accType_devRaw, slv_reg16, &rdData);
  streamRx_evCodeA_cntr = rdData;
  
  ip_rd(accType_devRaw, slv_reg17, &rdData);
  streamRx_evCodeB_cntr = rdData;

  ip_rd(accType_devRaw, slv_reg12, &rdData);
  emergencyStop     = ((rdData >> 31) & 0x1);
  emergencyStopCntr = ((rdData >>  0) & 0xffff);

  ip_rd(accType_devRaw, slv_reg13, &rdData);
  for(i=0;i<32;i++)
    pdp[i].isRunning = (rdData >> i) & 0x00000001;

  ip_rd(accType_devRaw, slv_reg14, &rdData);
  evrRx_linkErr = rdData & 0x1;

  ip_rd(accType_devRaw, slv_reg15, &rdData);
  evrRx_linkErrCntr = rdData;

  ip_getFifoStat(A_fifo_evLogT0_STAT , &fStat_evLogT0);
  ip_getFifoStat(A_fifo_evLogT1_STAT , &fStat_evLogT1);
  ip_getFifoStat(A_fifo_evLogEv_STAT , &fStat_evLogEv);
  ip_getFifoStat(A_fifo_evLogPdp_STAT, &fStat_evLogPdp);

  // control ----------------------------------------------
  ip_rd(accType_devRaw, slv_reg32, &rdData);
  extOutSwap   = (rdData>>31)&0x1;
  exOut30_xps  = (rdData>>29)&0x1;
  exOut31_1pps = (rdData>>30)&0x1;

  for(i=0;i<8;i++)
  {
    if( 0 == ((rdData>>i)&0x1) )
      ext_evOut[i]     = (ev_out_src)(PDP_OUT_00+i);
    else
      ext_evOut[i]     = (ev_out_src)(RXDBUS_00+i);
  }

  for(i=8;i<30;i++)
    ext_evOut[i]     = (ev_out_src)(PDP_OUT_00+i);

  if( 1 == exOut30_xps )
    ext_evOut[30]     = EXTOUT30_xps;
  else
    ext_evOut[30]     = PDP_OUT_30;

  if( 1 == exOut31_1pps )
    ext_evOut[31]     = EXTOUT31_1pps;
  else
    ext_evOut[31]     = PDP_OUT_31;

  ip_rd(accType_devRaw, slv_reg33, &rdData);
  evLogFifo_En_user = (rdData)&0x1;

  ip_rd(accType_devRaw, slv_reg34, &rdData);
  exOutInvert = (rdData);

  ip_rd(accType_devRaw, slv_reg38, &rdData);
  exOutEnable       = (rdData)&0x1;

  ip_setCommand(SET_evr_getStatReg, 0);
  for(i=0;i<MAX_PDP_NUM;i++)
  {
    ip_rd(accType_devRaw, MemC_EVR_pdpDelay + i*4, &rdData);
    pdp[i].delay = rdData;

    ip_rd(accType_devRaw, MemC_EVR_pdpWidth + i*4, &rdData);
    pdp[i].width = rdData;

    // ip_rd(accType_devRaw, MemC_EVR_pdpStat  + i*4, &rdData);
    // pdp[i].runCnt = ((rdData >> 16) & 0xffff);
    // pdp[i].errCnt = ((rdData >> 0 ) & 0xffff);
  }

  return RET_OK;
}


//=====================================
//----===@ prnStat
// Parameters  :
// Description :
int ts2ipEvr::prnStat(void)
{
  int rtVal;
  int i = 0;

  prnM2("======================================================\r\n");
  prnM2("++ %s prnStat ++\r\n", devName);
  prnM2("------------------------------------------------------\r\n");
  rtVal = getStat();
  ifRet(RET_ERR == rtVal);

  prnM2_fmtDec("evrRx_linkErr", evrRx_linkErr, "[1:Error]");
  prnM2_fmtDec("evrRx_linkErrCntr", evrRx_linkErrCntr, "counts");
  
  prnM2_fmtDec("evCodeA_cnt", rx_evCodeA_cntr, "counts");
  prnM2_fmtDec("evCodeB_cnt", rx_evCodeB_cntr, "counts");

  prnM2_fmtDec("stream_evCodeA_cnt", streamRx_evCodeA_cntr, "counts");
  prnM2_fmtDec("stream_evCodeB_cnt", streamRx_evCodeB_cntr, "counts");
  
  prnM2_fmtDec("emergencyStop", emergencyStop, "");
  prnM2_fmtDec("emergencyStopCntr", emergencyStopCntr, "counts");

  prnM2_fmtDec("exOutEnable    "   , exOutEnable         , "[High:Enable]");
  prnM2_fmtDec("exOutInvert    "   , exOutInvert         , "[High:Invert]");
  prnM2_fmtDec("evLogFifo_En_user" , evLogFifo_En_user   , "[1:Enable]");
  
  prnM2_fmtDec("extOutSwap"   , extOutSwap   , "1: sl<->sh swap");
  prnM2_fmtDec("exOut31_1pps" , exOut31_1pps , "1: ext out 31 to 1pps");
  prnM2_fmtDec("pdpTrgMode"   , pdpTrgMode   , "1: pdp 16~31 trigger from rx_evCodeA ");

  // if(pLevel > 0)
  {
		prnM2("---------------------------------------------------------------------\r\n");
    for(i=0;i<32;i++)
    {
      prnM2("%s_%02d : %dS/ %5dR/ %5dE/ %5dD/ %5dW/ %dP / evout : %s\r\n",\
                  "pdp",\
                  i,
                  pdp[i].isRunning  ,\
                  pdp[i].runCnt     ,\
                  pdp[i].errCnt     ,\
                  pdp[i].delay      ,\
                  pdp[i].width      ,\
                  pdp[i].polarity   ,\
                  evOutSrcName[ext_evOut[i]]\
                  );
    }
  }

  ip_prnFifoStat(fStat_evLogT0 , 1);
  ip_prnFifoStat(fStat_evLogT1 , 0);
  ip_prnFifoStat(fStat_evLogEv , 0);
  ip_prnFifoStat(fStat_evLogPdp, 0);

  prnM2("\r\n");
  return RET_OK;
}

//=====================================
//----===@ setEvRam
// Parameters  :
// Description :
int ts2ipEvr::setEvRam(uint evCode, uint mapData)
{
  ifRet(fd < 0);
  ifRet(evCode > 0x7ff);

  ip_wr(accType_devRaw, MemA_EVR_0 + evCode * 4 , mapData);
  return RET_OK;
}

//=====================================
//----===@ setEvRamN
// Parameters  :
// Description :
int ts2ipEvr::setEvRamN(uint evCode, uint mapData)
{
  ifRet(fd < 0);
  ifRet(evCode > 0x7ff);

  ip_wr(accType_devRaw, MemA_EVR_0n + evCode * 4 , mapData);
  return RET_OK;
}


//=====================================
//----===@ cfgPdp
// Parameters  :
// Description :
int ts2ipEvr::cfgPdp(uint pdpNum, uint delay, uint width, uint polarity, uint trgFromEvCodeA)
{
  ifRet(fd < 0);
  ifRet(pdpNum > 31);
  ifRet(polarity > 1);
  ifRet(trgFromEvCodeA > 1);

  pdp[pdpNum].delay = delay;
  pdp[pdpNum].width = width;
  pdp[pdpNum].polarity = polarity;

  if( (pdpNum>15) && (pdpNum<31) )
    pdp[pdpNum].trgFromEvCodeA = trgFromEvCodeA;
  else
    pdp[pdpNum].trgFromEvCodeA = 0;

  return RET_OK;
};

//=====================================
//----===@ cfgOut
// Parameters  :
// Description :
int ts2ipEvr::cfgOut(ev_out evOut, ev_out_src evOutSrc)
{
  ifRet(fd < 0);
  ifRet(evOut > EV_OUT_MAX-1);
  ifRet(evOutSrc > EV_OUT_SRC_MAX-1);
  
  int setState = RET_ERR;

  switch(evOut){
    case 0  :  if((evOutSrc == RXDBUS_00) || (evOutSrc == PDP_OUT_00)) setState = RET_OK; break;
    case 1  :  if((evOutSrc == RXDBUS_01) || (evOutSrc == PDP_OUT_01)) setState = RET_OK; break;
    case 2  :  if((evOutSrc == RXDBUS_02) || (evOutSrc == PDP_OUT_02)) setState = RET_OK; break;
    case 3  :  if((evOutSrc == RXDBUS_03) || (evOutSrc == PDP_OUT_03)) setState = RET_OK; break;
    case 4  :  if((evOutSrc == RXDBUS_04) || (evOutSrc == PDP_OUT_04)) setState = RET_OK; break;
    case 5  :  if((evOutSrc == RXDBUS_05) || (evOutSrc == PDP_OUT_05)) setState = RET_OK; break;
    case 6  :  if((evOutSrc == RXDBUS_06) || (evOutSrc == PDP_OUT_06)) setState = RET_OK; break;
    case 7  :  if((evOutSrc == RXDBUS_07) || (evOutSrc == PDP_OUT_07)) setState = RET_OK; break;
    case 8  :  if( evOutSrc == PDP_OUT_08) setState = RET_OK; break;
    case 9  :  if( evOutSrc == PDP_OUT_09) setState = RET_OK; break;
    case 10 :  if( evOutSrc == PDP_OUT_10) setState = RET_OK; break;
    case 11 :  if( evOutSrc == PDP_OUT_11) setState = RET_OK; break;
    case 12 :  if( evOutSrc == PDP_OUT_12) setState = RET_OK; break;
    case 13 :  if( evOutSrc == PDP_OUT_13) setState = RET_OK; break;
    case 14 :  if( evOutSrc == PDP_OUT_14) setState = RET_OK; break;
    case 15 :  if( evOutSrc == PDP_OUT_15) setState = RET_OK; break;
    case 16 :  if( evOutSrc == PDP_OUT_16) setState = RET_OK; break;
    case 17 :  if( evOutSrc == PDP_OUT_17) setState = RET_OK; break;
    case 18 :  if( evOutSrc == PDP_OUT_18) setState = RET_OK; break;
    case 19 :  if( evOutSrc == PDP_OUT_19) setState = RET_OK; break;
    case 20 :  if( evOutSrc == PDP_OUT_20) setState = RET_OK; break;
    case 21 :  if( evOutSrc == PDP_OUT_21) setState = RET_OK; break;
    case 22 :  if( evOutSrc == PDP_OUT_22) setState = RET_OK; break;
    case 23 :  if( evOutSrc == PDP_OUT_23) setState = RET_OK; break;
    case 24 :  if( evOutSrc == PDP_OUT_24) setState = RET_OK; break;
    case 25 :  if( evOutSrc == PDP_OUT_25) setState = RET_OK; break;
    case 26 :  if( evOutSrc == PDP_OUT_26) setState = RET_OK; break;
    case 27 :  if( evOutSrc == PDP_OUT_27) setState = RET_OK; break;
    case 28 :  if( evOutSrc == PDP_OUT_28) setState = RET_OK; break;
    case 29 :  if( evOutSrc == PDP_OUT_29) setState = RET_OK; break;
    case 30 :  if((evOutSrc == EXTOUT30_xps) || (evOutSrc == PDP_OUT_30)){
                  if(evOutSrc == EXTOUT30_xps){
                    exOut30_xps = 1;
                  }
                  else{
                    exOut30_xps = 0;
                  }
                  setState = RET_OK;
                }
                break;
    case 31 :  if((evOutSrc == EXTOUT31_1pps) || (evOutSrc == PDP_OUT_31)){
                  if(evOutSrc == EXTOUT31_1pps){
                    exOut31_1pps = 1;
                  }
                  else{
                    exOut31_1pps = 0;
                  }
                  setState = RET_OK;
                }
                break;
    default   : break;
  }
  
  if(setState == RET_OK){
    prnM2("  cfg_evr_out [evOut] %d - [evOutSrc] %s.\r\n", evOut, evOutSrcName[evOutSrc]);
    ext_evOut[evOut] = evOutSrc;
  }

  return setState;
};

//=====================================
//----===@ setOut
// Parameters  :
// Description :
int ts2ipEvr::setOut(uint ext_OutSwap)
{
  ifRet(fd < 0);
  ifRet(ext_OutSwap > 1);

  int i;
  uint wrData;
  uint wrData_bit;

  wrData = 0 ;
  wrData_bit = 1;

  extOutSwap   = ext_OutSwap;

  for(i=0;i<MAX_PDP_NUM;i++)
  {
    ip_wr(accType_devRaw, MemC_EVR_pdpDelay + 4 * i, pdp[i].delay);
    ip_wr(accType_devRaw, MemC_EVR_pdpWidth + 4 * i, pdp[i].width);

    if(pdp[i].polarity == 1) 
      wrData |= wrData_bit;

    wrData_bit = wrData_bit << 1;
  }
  ip_setCommand(SET_evr_setCtrlReg, 1000);

  ip_wr(accType_devRaw, slv_reg34, wrData); // exOutInvert
  
  wrData = 0 ;
  wrData_bit = 1;
  for(i=0;i<8;i++)
  {
    if((int)ext_evOut[i] < (int)PDP_OUT_00){
      wrData |= wrData_bit;
    }
    wrData_bit = wrData_bit << 1;
  }
  wrData = wrData | (extOutSwap << 31) | (exOut31_1pps << 30) | (exOut30_xps << 29); 
  ip_wr(accType_devRaw, slv_reg32, wrData);
  // //evr control config
  // swapExOut16            <= `DLY slv_reg32[31];
  // extOutSel_1pps         <= `DLY slv_reg32[30];
  // extOutSel_xps          <= `DLY slv_reg32[29];
  // rx_DBusSel8            <= `DLY slv_reg32[7:0];
  prnM2("swap-1pps-xps-dbus8 : 0x%08x.\r\n",wrData);
  
  wrData = 0 ;
  wrData_bit = 1;
  for(i=0;i<15;i++)
  {
    if(pdp[i+16].trgFromEvCodeA == 1) 
      wrData |= wrData_bit;

    wrData_bit = wrData_bit << 1;
  }
  ip_wr(accType_devRaw, slv_reg35, wrData);
  prnM2("evr_pdpTrgSrcSel wrData : 0x%08x.\r\n",wrData);

  return RET_OK;
};

//=====================================
//----===@ prnEvLog
// Parameters  :
// Description :
int ts2ipEvr::prnEvLog(void)
{
  // uint rdData;
  int i;
  uint rdEv;
  uint rdPdp;
  struct s_fifoStat fStat;

  ifRet(fd < 0);

  prnM2("======================================================\r\n");
  prnM2("++ prnEvLog ++\r\n");

  for(i=0;i<1024;i++)
  {
    ip_getFifoStat(A_fifo_evLogPdp_STAT, &fStat);
    //prnM2("%-22s: %d\r\n","fStat.EMPTY",fStat.EMPTY);
    if(fStat.EMPTY == 0)
    {
      //evr status
      ip_getEvTime(A_get_evLogT0, A_get_evLogT0+8, &logTime);

      ip_rd(accType_devRaw, A_get_evLogEv       , &rdEv);
      ip_rd(accType_devRaw, A_get_evLogPdp      , &rdPdp);

      prnM2("%-12s: %04d.%02d.%02d-%02d:%02d:%02d-%09d[ns] ev[0x%08x] pdp[0x%08x]\r\n", \
            logTime.name        , \
            logTime.tmT.tm_year , \
            logTime.tmT.tm_mon  +1, \
            logTime.tmT.tm_mday , \
            logTime.tmT.tm_hour , \
            logTime.tmT.tm_min  , \
            logTime.tmT.tm_sec  , \
            (uint)logTime.nsec  , \
            rdEv                , \
            rdPdp               );
    }
    else
    {
      break;
    }
  }
  return RET_OK;
}


//=====================================
//----===@ get_evTime
// Parameters  :
// Description :
int ts2ipEvr::get_evTime(void)
{
  ifRet(fd < 0);

  ip_getEvTime(slv_reg1e, slv_reg1e+4, &evTime);

  return RET_OK;
}


//=====================================
//----===@ get_evgCommand
// Parameters  :
// Description :
int ts2ipEvr::get_evgCommand(void)
{
  ifRet(fd < 0);

  uint rdData;
  ip_rd(accType_devRaw, slv_reg1a, &rdData);
  evgCmd = rdData & 0xffff;

  ip_rd(accType_devRaw, slv_reg1b, &rdData);
  evgCmdSub = rdData & 0xffff;

  return RET_OK;
}














} //name space end




