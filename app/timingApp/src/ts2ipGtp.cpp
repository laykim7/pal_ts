//Wrapping class for system calls of timing device driver.
#include <stdlib.h>
#include <termios.h>

#include "ts2ipGtp.h"


namespace timing{

//==============================================================================
//----===@ define
//==============================================================================
//-----------------------------------------------------------------------------
// STATUS : ts2gtp ip
//-----------------------------------------------------------------------------
  //  status check
#define A_sttGtp_ip          slv_reg10
#define A_sttGtp_trackLoss   slv_reg11
#define A_sttGtp_gt0clk_cntr slv_reg12 // {gt0_txClk_cntr,gt0_rxClk_cntr} ;
#define A_sttGtp_gt1clk_cntr slv_reg13 // {gt1_txClk_cntr,gt1_rxClk_cntr} ;

//-----------------------------------------------------------------------------
// Control : ts2gtp ip
//-----------------------------------------------------------------------------
#define A_cfgGtp             slv_reg31

#define SET_gtp_reset        (BIT31)

//==============================================================================
//----===@ global variable
//==============================================================================

//==============================================================================
//----===@ class
//==============================================================================


//=====================================
//----===@ ts2ipGtp
// Parameters  :
// Description :
ts2ipGtp::ts2ipGtp(const char *deviceName)
{
  ip_open(deviceName);
}

//=====================================
//----===@ ~ts2ipGtp
// Parameters  :
// Description :
ts2ipGtp::~ts2ipGtp()
{
  prnM2("~ts2ipGtp();\r\n");
}

//=====================================
//----===@ getStat
// Parameters  :
// Description :
int ts2ipGtp::getStat(void)
{
  ifRet(fd < 0);

  uint rdData;

  ip_rd(accType_devRaw, A_sttGtp_ip, &rdData);  
  stat[0].cpllfbclklost    = ((rdData >>  0) & 0x1) ;
  stat[0].cplllock         = ((rdData >>  1) & 0x1) ;
  stat[0].txresetdone      = ((rdData >>  4) & 0x1) ;
  stat[0].txfsmresetdone   = ((rdData >>  5) & 0x1) ;
  stat[0].rxresetdone      = ((rdData >>  6) & 0x1) ;
  stat[0].rxfsmresetdone   = ((rdData >>  7) & 0x1) ;
  stat[0].track_data       = ((rdData >> 10) & 0x1) ;

  if(0xa == ipVer_rev){
    stat[1].cpllfbclklost    = ((rdData >> 16) & 0x1) ;
    stat[1].cplllock         = ((rdData >> 17) & 0x1) ;
    stat[1].txresetdone      = ((rdData >> 20) & 0x1) ;
    stat[1].txfsmresetdone   = ((rdData >> 21) & 0x1) ;
    stat[1].rxresetdone      = ((rdData >> 22) & 0x1) ;
    stat[1].rxfsmresetdone   = ((rdData >> 23) & 0x1) ;
    stat[1].track_data       = ((rdData >> 26) & 0x1) ;
  }

  ip_rd(accType_devRaw, A_sttGtp_trackLoss, &rdData); 
  stat[0].trackLossCnt     = ((rdData >>  0) & 0xffff)    ;
  if(0xa == ipVer_rev){
    stat[1].trackLossCnt     = ((rdData >> 16) & 0xffff)    ;
  }

  ip_rd(accType_devRaw, A_sttGtp_gt0clk_cntr, &rdData); 
  stat[0].txClk_cntr       = (rdData )      ;

  if(0xa == ipVer_rev){
    ip_rd(accType_devRaw, A_sttGtp_gt1clk_cntr, &rdData); 
    stat[1].txClk_cntr       = (rdData )      ;
  }
  
  return RET_OK;
}

//=====================================
//----===@ prnStat
// Parameters  :
// Description :
int ts2ipGtp::prnStat(void)
{
  prnM2("======================================================\r\n");
  prnM2("++ %s prnStat ++\r\n", devName);
  prnM2("------------------------------------------------------\r\n");
  ifRet(fd < 0);

  getStat();
  
  prnM2_fmtDec("gt0_cplllock      ", stat[0].cplllock       , " ");
  prnM2_fmtDec("gt0_txfsmresetdone", stat[0].txfsmresetdone , " ");
  prnM2_fmtDec("gt0_rxfsmresetdone", stat[0].rxfsmresetdone , " ");
  if(0xa == ipVer_rev){
    prnM2("------------------------------------------------------\r\n");
    prnM2_fmtDec("gt1_cplllock      ", stat[1].cplllock       , " ");
    prnM2_fmtDec("gt1_txfsmresetdone", stat[1].txfsmresetdone , " ");
    prnM2_fmtDec("gt1_rxfsmresetdone", stat[1].rxfsmresetdone , " ");
  }
  prnM2("------------------------------------------------------\r\n");
  prnM2_fmtDec("gt0_trackLossCnt  ", stat[0].trackLossCnt   , " ");
  if(0xa == ipVer_rev){
    prnM2_fmtDec("gt1_trackLossCnt  ", stat[1].trackLossCnt   , " ");
  }
  prnM2("------------------------------------------------------\r\n");
  prnM2_fmtDec("gt0_txClk_cntr    ", stat[0].txClk_cntr   , " ");
  if(0xa == ipVer_rev){
    prnM2_fmtDec("gt1_txClk_cntr    ", stat[1].txClk_cntr   , " ");
  }

  prnM2("------------------------------------------------------\r\n");
  prnM2_fmtDec("gt0_track_data"    , stat[0].track_data     , "<------");
  if(0xa == ipVer_rev){
    prnM2_fmtDec("gt1_track_data"    , stat[1].track_data     , "<------");
  }
  prnM2("\r\n");

  prnM2("\r\n");

  return RET_OK;
}

//=====================================
//----===@ reset
// Parameters  :
// Description :
int ts2ipGtp::reset(void)
{
  ifRet(fd < 0);
  ip_setCommand(SET_gtp_reset, 1000);
  return RET_OK;
}


} //name space end




