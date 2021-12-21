//Wrapping class for system calls of timing device driver.
#include <stdlib.h>
#include <termios.h>

#include "timingData.h"
#include "tsDev_evx.h"

namespace timing{

//==============================================================================
//----===@ define
//==============================================================================

//==============================================================================
//----===@ global variable
//==============================================================================

//==============================================================================
//----===@ class
//==============================================================================


//=====================================
//----===@ tsDev_evx
// Parameters  :
// Description :
tsDev_evx::tsDev_evx(const int opModeV, const int tsModeV, const int tsClassV, const char* tsNameV, const int tsNumV, uint tzone, uint tickPeriod)
{
  opMode      = opModeV;
  tsMode      = tsModeV;
  tsClass     = tsClassV;
  tsNum       = tsNumV;
  sprintf(tsName,"%s", tsNameV);

  // prnM2("[F:%s]\n",__FUNCTION__);
  int i;
  for(i=0;i<ipIdCntMax;i++){
    ev[i]  = NULL;
    evg[i] = NULL;
    evr[i] = NULL;
    gtp[i] = NULL;
  }
  evr[0] = new ts2ipEvr(DRV_NAME_ip_evr0 , tzone, tickPeriod);
  evr[1] = new ts2ipEvr(DRV_NAME_ip_evr1 , tzone, tickPeriod);
  evg[0] = new ts2ipEvg(DRV_NAME_ip_evg0 , tzone, tickPeriod);
  evg[1] = new ts2ipEvg(DRV_NAME_ip_evg1 , tzone, tickPeriod);

  gtp[0] = new ts2ipGtp(DRV_NAME_ip_gtp  );
  ev[0]  = new ts2ipEvsys(DRV_NAME_ip_evsys, &tsMode, tsName, &opMode);

  ifJstRet(ev[0]->isInit == -1);
  ifJstRet(gtp[0]->isInit == -1);
  // ifJstRet(evr[1]->isInit == -1);
  // ifJstRet(evg[0]->isInit == -1);

  prnM2("\r\n");

  // taskDelay(500000);
  //-- init gtp clock and then reset gtp
  gtp[0]->reset();
  taskDelay(10000);
  
  evg[0]->enable(0, 0, 0, 0, 0, 0, 0);
  evg[1]->enable(0, 0, 0, 0, 0, 0, 0);

  evg[0]->setTrgMsk(0x7fffffff);
  evg[1]->setTrgMsk(0x7fffffff);

  if(tsMode == RAON_EVG){
    // ev[0]->setIgen  (DR_ENABLE, IGEN_MODE_EXT , 19, 126, 0, 0, 0);
    evg[0]->setWatch(IDECODE_MODE_USER        , 20, 366, 14, 59, 0);
    evg[1]->setWatch(IDECODE_MODE_USER        , 20, 366, 14, 59, 0);
    ev[0]->setIgen  (DR_ENABLE, IGEN_MODE_USER, 20, 366, 14, 59, 0);
  }
  else if(tsMode == RAON_EVRUP){
    evg[0]->setWatch(IDECODE_MODE_TNET        , 19, 126, 0, 0, 0);
    evg[1]->setWatch(IDECODE_MODE_TNET        , 19, 126, 0, 0, 0);
    ev[0]->setIgen  (DR_ENABLE, IGEN_MODE_USER, 19, 126, 0, 0, 0);
  }
  else{
    evg[0]->setWatch(IDECODE_MODE_IGEN        , 20, 366, 14, 59, 0);
    evg[1]->setWatch(IDECODE_MODE_IGEN        , 20, 366, 14, 59, 0);
    ev[0]->setIgen  (DR_ENABLE, IGEN_MODE_USER, 20, 366, 14, 59, 0);
  }

  //    enable(uint exOut, uint rxTNet );
  evr[0]->enable( 0, 0);
  evr[1]->enable( 0, 0);

  evr[0]->ip_intrClear();
  evr[1]->ip_intrClear();

  evr[0]->ip_intrMask(0 \
      );
  
  evr[1]->ip_intrMask(0 \
      | C_intrMsk_evrW_alarm_sec \
      | C_intrMsk_Emergency \
      | C_intrMsk_evCodeB_7F2 \
      );

  evrMain = evr[1];
  evrSub  = evr[0];

  prnM2("tsDev_evx init ok %d.\r\n",isInit);
  isInit = 1;
}

//=====================================
//----===@ ~tsDev_evx
// Parameters  :
// Description :
tsDev_evx::~tsDev_evx()
{
  delete ev[0]  ;
  delete gtp[0] ;
  delete evg[0] ;
  delete evg[1] ;
  delete evr[0] ;
  delete evr[1] ;

  isInit = -1;
  prnM2("[F:%s]\n",__FUNCTION__);
}

//=====================================
//----===@ readAppData
// Parameters  :
// Description :
asynStatus tsDev_evx::readAppData(uint offset, uint *val)
{
  asynStatus status = asynSuccess;

  *val = 0;
  return status;
}

//=====================================
//----===@ writeAppData
// Parameters  :
// Description :
asynStatus tsDev_evx::writeAppData(uint offset, uint val)
{
  asynStatus status = asynSuccess;
  unsigned int addr = offset/4;

  switch(addr)
  {
    case 0x1000   : if(ev[0] == NULL){return asynError;} ev[0]->ob.cpsOut[CPS_OB_GT0 ] = val; break;
    case 0x1001   : if(ev[0] == NULL){return asynError;} ev[0]->ob.cpsOut[CPS_OB_GT1 ] = val; break;
    case 0x1002   : if(ev[0] == NULL){return asynError;} ev[0]->ob.cpsOut[CPS_OB_FMC1] = val; break;
    case 0x1003   : if(ev[0] == NULL){return asynError;} ev[0]->ob.cpsOut[CPS_OB_FMC2] = val; break;
    case 0x1004   : if(ev[0] == NULL){return asynError;} ev[0]->ob.cpsOut[CPS_OB_SL  ] = val; break;
    case 0x1005   : if(ev[0] == NULL){return asynError;} ev[0]->ob.cpsOut[CPS_OB_SH  ] = val; break;
    case 0x1006   : if(ev[0] == NULL){return asynError;} ev[0]->ob.cpsOut[CPS_OB_CPS ] = val; break;

    case 0x1010   : if(ev[0] == NULL){return asynError;} ev[0]->sl.cpsOut[CPS_SW_CTRL] = val; break;
    case 0x1011   : if(ev[0] == NULL){return asynError;} ev[0]->sl.cpsOut[CPS_SW_SW  ] = val; break;
    case 0x1012   : if(ev[0] == NULL){return asynError;} ev[0]->sl.cpsOut[CPS_SW_FR  ] = val; break;
    case 0x1013   : if(ev[0] == NULL){return asynError;} ev[0]->sl.cpsOut[CPS_SW_X   ] = val; break;
    case 0x1014   : if(ev[0] == NULL){return asynError;} ev[0]->sl.cpsOut[CPS_SW_00  ] = val; break;
    case 0x1015   : if(ev[0] == NULL){return asynError;} ev[0]->sl.cpsOut[CPS_SW_01  ] = val; break;
    case 0x1016   : if(ev[0] == NULL){return asynError;} ev[0]->sl.cpsOut[CPS_SW_02  ] = val; break;
    case 0x1017   : if(ev[0] == NULL){return asynError;} ev[0]->sl.cpsOut[CPS_SW_03  ] = val; break;
    case 0x1018   : if(ev[0] == NULL){return asynError;} ev[0]->sl.cpsOut[CPS_SW_04  ] = val; break;
    case 0x1019   : if(ev[0] == NULL){return asynError;} ev[0]->sl.cpsOut[CPS_SW_05  ] = val; break;
    case 0x101a   : if(ev[0] == NULL){return asynError;} ev[0]->sl.cpsOut[CPS_SW_06  ] = val; break;
    case 0x101b   : if(ev[0] == NULL){return asynError;} ev[0]->sl.cpsOut[CPS_SW_07  ] = val; break;
    case 0x101c   : if(ev[0] == NULL){return asynError;} ev[0]->sl.cpsOut[CPS_SW_08  ] = val; break;
    case 0x101d   : if(ev[0] == NULL){return asynError;} ev[0]->sl.cpsOut[CPS_SW_09  ] = val; break;
    case 0x101e   : if(ev[0] == NULL){return asynError;} ev[0]->sl.cpsOut[CPS_SW_10  ] = val; break;
    case 0x101f   : if(ev[0] == NULL){return asynError;} ev[0]->sl.cpsOut[CPS_SW_11  ] = val; break;
    
    case 0x1020   : if(ev[0] == NULL){return asynError;} ev[0]->sh.cpsOut[CPS_SW_CTRL] = val; break;
    case 0x1021   : if(ev[0] == NULL){return asynError;} ev[0]->sh.cpsOut[CPS_SW_SW  ] = val; break;
    case 0x1022   : if(ev[0] == NULL){return asynError;} ev[0]->sh.cpsOut[CPS_SW_FR  ] = val; break;
    case 0x1023   : if(ev[0] == NULL){return asynError;} ev[0]->sh.cpsOut[CPS_SW_X   ] = val; break;
    case 0x1024   : if(ev[0] == NULL){return asynError;} ev[0]->sh.cpsOut[CPS_SW_00  ] = val; break;
    case 0x1025   : if(ev[0] == NULL){return asynError;} ev[0]->sh.cpsOut[CPS_SW_01  ] = val; break;
    case 0x1026   : if(ev[0] == NULL){return asynError;} ev[0]->sh.cpsOut[CPS_SW_02  ] = val; break;
    case 0x1027   : if(ev[0] == NULL){return asynError;} ev[0]->sh.cpsOut[CPS_SW_03  ] = val; break;
    case 0x1028   : if(ev[0] == NULL){return asynError;} ev[0]->sh.cpsOut[CPS_SW_04  ] = val; break;
    case 0x1029   : if(ev[0] == NULL){return asynError;} ev[0]->sh.cpsOut[CPS_SW_05  ] = val; break;
    case 0x102a   : if(ev[0] == NULL){return asynError;} ev[0]->sh.cpsOut[CPS_SW_06  ] = val; break;
    case 0x102b   : if(ev[0] == NULL){return asynError;} ev[0]->sh.cpsOut[CPS_SW_07  ] = val; break;
    case 0x102c   : if(ev[0] == NULL){return asynError;} ev[0]->sh.cpsOut[CPS_SW_08  ] = val; break;
    case 0x102d   : if(ev[0] == NULL){return asynError;} ev[0]->sh.cpsOut[CPS_SW_09  ] = val; break;
    case 0x102e   : if(ev[0] == NULL){return asynError;} ev[0]->sh.cpsOut[CPS_SW_10  ] = val; break;
    case 0x102f   : if(ev[0] == NULL){return asynError;} ev[0]->sh.cpsOut[CPS_SW_11  ] = val; break;

    case 0x1030   : if(val == 0){return asynSuccess;} 
                    if(ev[0] == NULL){return asynError;} 
                    
                    ev[0]->set_cpsConfig(&ev[0]->ob); 
                    if(ev[0]->sl.id==0x02)
                      ev[0]->set_cpsConfig(&ev[0]->sl); 
                    if(ev[0]->sh.id==0x02) 
                      ev[0]->set_cpsConfig(&ev[0]->sh); 
                    break;

    case 0x1100   : if(ev[0] == NULL){return asynError;} bist(val); break;

    default    : status = asynError; break;
  }
  return status;
}

//=====================================
//----===@ readReg
// Parameters  :
// Description :
asynStatus tsDev_evx::readReg(const RegMap &rmap, int &val)
{
  // prnM2("[F:%s]\n",__FUNCTION__);
  int rdData=-1;
  asynStatus status = asynSuccess;
  timing::ts2ipDrv  *pTsIp = NULL;

  val = -1;
  // prnM2("readReg %s\r\n", rmap.drvname);

  if(rmap.accType == accType_app){
    readAppData(rmap.address, (unsigned int*)&rdData);
  }
  else{
    switch(rmap.ipId){
      case ipId_ev  : if(ev[rmap.ipCnt]  == NULL){return asynError;}
                      pTsIp = ev[rmap.ipCnt];  
                      break;
      case ipId_gtp : if(gtp[rmap.ipCnt] == NULL){return asynError;}
                      pTsIp = gtp[rmap.ipCnt]; 
                      break;
      case ipId_evg : if(evg[rmap.ipCnt] == NULL){return asynError;}
                      pTsIp = evg[rmap.ipCnt]; 
                      break;
      case ipId_evr : if(evr[rmap.ipCnt] == NULL){return asynError;}
                      pTsIp = evr[rmap.ipCnt]; 
                      break;
      default : return asynError;
    }

    if(pTsIp != NULL)
      pTsIp->ip_rd(rmap.accType, rmap.address, (unsigned int*)&rdData);
    else
      return asynError;

    switch(rmap.dataWidth){
      case 32 : break;
      case 16 : rdData = (rdData >> rmap.dataOffset);  rdData &= 0xFFFF;  break;
      case  8 : rdData = (rdData >> rmap.dataOffset);  rdData &= 0xFF;    break;
      case  4 : rdData = (rdData >> rmap.dataOffset);  rdData &= 0xF;     break;
      case  2 : rdData = (rdData >> rmap.dataOffset);  rdData &= 0x3;     break;
      case  1 : rdData = (rdData >> rmap.dataOffset);  rdData &= 0x1;     break;
      default : rdData = -1; status = asynError; break;
    }
  }

  val = rdData;
  return status;
}

//=====================================
//----===@ readString
// Parameters  :
// Description :
asynStatus tsDev_evx::readString(const RegMap &rmap, char *val)
{
  asynStatus status = asynSuccess;
  unsigned int addr = rmap.address/4;
  char strbuf[128];
  sprintf(strbuf,"ERROR"); 
  // prnM2("[%d-%d] 0x%08x, %d, %d\r\n",rmap.ipId,rmap.ipCnt, addr, rmap.dataWidth, rmap.dataOffset);

  if(rmap.accType == accType_app){
    switch(addr){
      case 0x000 : sprintf(strbuf,"%s", tsName); 
                    // printf("%s\r\n",strbuf);
                    break;
      case 0x001 : epicsTimeStamp current;
                    epicsTimeGetCurrent (&current);
                    epicsTimeToStrftime(strbuf,25,"%Y.%m.%d-%H.%M.%S", &current);
                    break;

      case 0x004 : get_strTime(&gtp[0]->buildTime, strbuf); break;
      
      case 0x005 : sprintf(strbuf,"%s", ver_sw->chVer ); break;
      case 0x006 : sprintf(strbuf,"%s", ver_boot->chVer ); break;
      case 0x007 : sprintf(strbuf,"%s", ver_image->chVer ); break;
      case 0x008 : sprintf(strbuf,"%s", ver_pv->chVer ); break;

      case 0x010 : if(gtp[0]->stat[0].cplllock == 1)       sprintf(strbuf,"Lock"); break;
      case 0x011 : if(gtp[0]->stat[0].rxfsmresetdone == 1) sprintf(strbuf,"Done"); break;
      case 0x012 : if(gtp[0]->stat[0].track_data == 1)     sprintf(strbuf,"OK");   break;
      case 0x013 : sprintf(strbuf,"%d",gtp[0]->stat[0].trackLossCnt); break;
      case 0x014 : get_strTime(&evrMain->evTime, strbuf); break;

      case 0x100 : sprintf(strbuf,"%6.1f mV",(epicsFloat64)xadcCmd[EParamVccInt ].value); break;
      case 0x101 : sprintf(strbuf,"%6.1f mV",(epicsFloat64)xadcCmd[EParamVccAux ].value); break;
      case 0x102 : sprintf(strbuf,"%6.1f mV",(epicsFloat64)xadcCmd[EParamVccBRam].value); break;
      case 0x103 : sprintf(strbuf,"%6.1f mV",(epicsFloat64)xadcCmd[EParamVccpInt].value); break;
      case 0x104 : sprintf(strbuf,"%6.1f mV",(epicsFloat64)xadcCmd[EParamVccpAux].value); break;
      case 0x105 : sprintf(strbuf,"%6.1f mV",(epicsFloat64)xadcCmd[EParamVccoddr].value); break;
      case 0x106 : sprintf(strbuf,"%6.1f mV",(epicsFloat64)xadcCmd[EParamVrefp  ].value); break;
      case 0x107 : sprintf(strbuf,"%6.1f mV",(epicsFloat64)xadcCmd[EParamVrefn  ].value); break;
      case 0x108 : sprintf(strbuf,"%3.1f'  ",(epicsFloat64)xadcCmd[EParamTemp   ].value); break;

      case 0x200 : if(ev[0]->isMaster == 1)
                      sprintf(strbuf,"Master Floor");
                    else
                      sprintf(strbuf,"Slave Floor"); 
                    break;
      case 0x210 : if(gtp[0]->stat[1].cplllock == 1)       sprintf(strbuf,"Lock"); break;
      case 0x211 : if(gtp[0]->stat[1].rxfsmresetdone == 1) sprintf(strbuf,"Done"); break;
      case 0x212 : if(gtp[0]->stat[1].track_data == 1)     sprintf(strbuf,"OK");   break;
      case 0x213 : sprintf(strbuf,"%d",gtp[0]->stat[1].trackLossCnt); break;

      case 0x214 : get_strTime(&evrSub->evTime, strbuf); break;
      case 0x220 : get_strTime(&evg[0]->evTime, strbuf); break;
      case 0x221 : get_strTime(&evg[0]->evTimeSub, strbuf); break;
      case 0x222 : get_strTime(&evg[1]->evTime, strbuf); break;
      case 0x223 : get_strTime(&evg[1]->evTimeSub, strbuf); break;

      case 0x224 : sprintf(strbuf,"%10s %3.1f'", ev[0]->sl.name, ev[0]->sl.temp); break;
      case 0x225 : sprintf(strbuf,"%10s %3.1f'", ev[0]->sh.name, ev[0]->sh.temp); break;

      case 0x226 : sprintf(strbuf,"%d", ev[0]->fanA); break;
      case 0x227 : sprintf(strbuf,"%d", ev[0]->fanB); break;
      case 0x228 : sprintf(strbuf,"%d", ev[0]->fanC); break;
      case 0x229 : sprintf(strbuf,"%d", ev[0]->fanD); break;
      case 0x230 : sprintf(strbuf,"%d", ev[0]->chkDelay_buf); break;


      default    : status = asynError; break;
    }
  }
  else{
    return asynError;
  }

  strcpy(val, (char*)strbuf); 

  return status;
}



//=====================================
//----===@ writeReg
// Parameters  :
// Description :
asynStatus tsDev_evx::writeReg(const RegMap &rmap, const int val)
{
  // prnM2("[F:%s]\n",__FUNCTION__);
  int rdData=-1;
  int value=val;
  int wrData;
  asynStatus status = asynSuccess;
  timing::ts2ipDrv  *pTsIp = NULL;
  int tmpData=0;

  // prnM2("writeReg %s\r\n", rmap.drvname);

  if(rmap.accType == accType_app){
    writeAppData(rmap.address, (unsigned int)val);
  }
  else{
    switch(rmap.ipId){
      case ipId_ev  : if(ev[rmap.ipCnt]  == NULL){return asynError;}
                      pTsIp = ev[rmap.ipCnt];  
                      break;
      case ipId_gtp : if(gtp[rmap.ipCnt] == NULL){return asynError;}
                      pTsIp = gtp[rmap.ipCnt]; 
                      break;
      case ipId_evr : if(evr[rmap.ipCnt] == NULL){return asynError;}
                      pTsIp = evr[rmap.ipCnt]; 
                      break;
      case ipId_evg  : if(evg[rmap.ipCnt]  == NULL){return asynError;}
                      pTsIp = evg[rmap.ipCnt];  
                      break;
      default : return asynError;
    }

    if(pTsIp != NULL)
      pTsIp->ip_rd(rmap.accType, rmap.address, (unsigned int*)&rdData);
    else
      return asynError;

    switch(rmap.dataWidth)
    {
      case 16 : value &= 0xFFFF; value = (value << rmap.dataOffset);  tmpData = ~(0xFFFF << rmap.dataOffset);  break;
      case  8 : value &= 0xFF;   value = (value << rmap.dataOffset);  tmpData = ~(0xFF   << rmap.dataOffset);  break;
      case  4 : value &= 0xF;    value = (value << rmap.dataOffset);  tmpData = ~(0xF    << rmap.dataOffset);  break;
      case  2 : value &= 0x3;    value = (value << rmap.dataOffset);  tmpData = ~(0x3    << rmap.dataOffset);  break;
      case  1 : value &= 0x1;    value = (value << rmap.dataOffset);  tmpData = ~(0x1    << rmap.dataOffset);  break;
      default : break;
    };

    if(dataType_Pulse == rmap.dataType){
      if(pTsIp != NULL){
        pTsIp->ip_wr(rmap.accType, rmap.address, (unsigned int)value);
        pTsIp->ip_wr(rmap.accType, rmap.address, 0);
      }
      else
        return asynError;
    }
    else{
      wrData = rdData & tmpData;
      wrData |= value;

      if(pTsIp != NULL)
        pTsIp->ip_wr(rmap.accType, rmap.address, (unsigned int)wrData);
      else
        return asynError;
    }
  }
  return status;
}


//=====================================
//----===@ writeString
// Parameters  :
// Description :
asynStatus tsDev_evx::writeString(const RegMap &rmap, const char *val)
{
  int value = 0;

  if(dataType_Trg_Seq == rmap.dataType)
  {
    if      (strcmp(val, "User Trigger"  ) == 0 ) value =  0;
    else if (strcmp(val, "MXC_00"        ) == 0 ) value =  1;
    else if (strcmp(val, "MXC_01"        ) == 0 ) value =  2;
    else if (strcmp(val, "MXC_02"        ) == 0 ) value =  3;
    else if (strcmp(val, "MXC_03"        ) == 0 ) value =  4;
    else if (strcmp(val, "MXC_04"        ) == 0 ) value =  5;
    else if (strcmp(val, "MXC_05"        ) == 0 ) value =  6;
    else if (strcmp(val, "MXC_06"        ) == 0 ) value =  7;
    else if (strcmp(val, "MXC_07"        ) == 0 ) value =  8;
    else if (strcmp(val, "EXT_IN_SL_00"  ) == 0 ) value = 15;
    else if (strcmp(val, "EXT_IN_SL_01"  ) == 0 ) value = 16;
    else if (strcmp(val, "EXT_SYNC_01"   ) == 0 ) value = 31;
    else if (strcmp(val, "none"          ) == 0 ) value =  0;
    else if (strcmp(val, "MASK A0"       ) == 0 ) value =  1;
    else if (strcmp(val, "MASK A1"       ) == 0 ) value =  2;
    else if (strcmp(val, "MASK A0 and A1") == 0 ) value =  3;
    else if (strcmp(val, "MASK B0"       ) == 0 ) value =  5;
    else if (strcmp(val, "MASK B1"       ) == 0 ) value =  6;
    else if (strcmp(val, "MASK B0 and B1") == 0 ) value =  7;
    else if (strcmp(val, "evCode"        ) == 0 ) value =  0;
    else if (strcmp(val, "dBus0"         ) == 0 ) value =  1;
    else if (strcmp(val, "dBus1"         ) == 0 ) value =  2;
    else if (strcmp(val, "dBus2"         ) == 0 ) value =  3;
    else if (strcmp(val, "dBus3"         ) == 0 ) value =  4;
    else if (strcmp(val, "dBus4"         ) == 0 ) value =  5;
    else if (strcmp(val, "dBus5"         ) == 0 ) value =  6;
    else if (strcmp(val, "dBus6"         ) == 0 ) value =  7;
    else if (strcmp(val, "dBus7"         ) == 0 ) value =  8;
  }
  else if(dataType_Config_CPS == rmap.dataType)
  {
    if      (strcmp(val, "GT0"  ) == 0 ) value = 0 ;
    else if (strcmp(val, "GT1"  ) == 0 ) value = 1 ;
    else if (strcmp(val, "FMC1" ) == 0 ) value = 9 ;
    else if (strcmp(val, "FMC2" ) == 0 ) value = 10;
    else if (strcmp(val, "SL"   ) == 0 ) value = 12;
    else if (strcmp(val, "SH"   ) == 0 ) value = 13;
    else if (strcmp(val, "OCTRL") == 0 ) value = 14; // other ctrl board 
    else if (strcmp(val, "CTRL" ) == 0 ) value = 0 ;
    else if (strcmp(val, "SW"   ) == 0 ) value = 1 ;
    else if (strcmp(val, "FR"   ) == 0 ) value = 2 ;
    else if (strcmp(val, "X"    ) == 0 ) value = 3 ;
    else if (strcmp(val, "00"   ) == 0 ) value = 4 ;
    else if (strcmp(val, "01"   ) == 0 ) value = 5 ;
    else if (strcmp(val, "02"   ) == 0 ) value = 6 ;
    else if (strcmp(val, "03"   ) == 0 ) value = 7 ;
    else if (strcmp(val, "04"   ) == 0 ) value = 8 ;
    else if (strcmp(val, "05"   ) == 0 ) value = 9 ;
    else if (strcmp(val, "06"   ) == 0 ) value = 10;
    else if (strcmp(val, "07"   ) == 0 ) value = 11;
    else if (strcmp(val, "08"   ) == 0 ) value = 12;
    else if (strcmp(val, "09"   ) == 0 ) value = 13;
    else if (strcmp(val, "10"   ) == 0 ) value = 14;
    else if (strcmp(val, "11"   ) == 0 ) value = 15;
  }

  prnM2("writeString : %s - %d\r\n", val, value);

  writeReg(rmap, value);

  return asynSuccess;
}


//=====================================
//----===@ delayCheck_Proc
// Parameters  :
// Description :
void tsDev_evx::delayCheck_Proc(void)
{
  evrMain->get_evgCommand();
  // evr[0]->evgCmd;
  // evr[0]->evgCmdSub;
};

//=====================================
//----===@ devProc
// Parameters  :
// Description :
int tsDev_evx::devProc(void)
{
  // prnM2("[F:%s]\n",__FUNCTION__);
  static uint runCount =0;
  static uint evTimeCheckCount =0;
  static int  evg_evTime_sec  =60;
  static int  gt1_tLoss_pre  =-1;
  static int  rx_evCode_miss =-1;
  static char pwrOK=0, fanERR=0, epicsOK=0, timingNetOK=0 ;
  
  static int  pre_opMode;

  get_evTimeAll();

  ev[0]->getStat();
  ev[0]->get_slaveTemp();

  evg[0]->setDevInfo(tsMode, tsNum, ev[0]->chkDelay_buf);
  evg[1]->setDevInfo(tsMode, tsNum, ev[0]->chkDelay_buf);

  gtp[0]->getStat();
  evg[0]->getStat();
  evg[1]->getStat();
  evr[0]->getStat();
  evr[1]->getStat();

  // if(tsMode == RAON_EVRUP)
  {
    if( (evg_evTime_sec == evg[1]->evTime.tmT.tm_sec) )
      evTimeCheckCount++;
    else
      evTimeCheckCount = 0;

    if( evTimeCheckCount > 5 )
    {
      gtp[0]->reset();
      prnM2("[ERROR] === gtp reset === !!!!!!!!!\r\n");
      taskDelay(1000);
    }
  }
  evg_evTime_sec  = evg[1]->evTime.tmT.tm_sec;

  if(pre_opMode != opMode)
    ev[0]->init_system();

  pre_opMode = opMode;

  if(TS_OP_MODE_FANCHECK == opMode){
    ev[0]->updateLCD_FAN();
  }
  else{
    ev[0]->updateLCD_rxCount(evr[1]->rx_evCodeA_cntr, evr[1]->rx_evCodeB_cntr);
    ev[0]->updateLCD_Time(&evr[1]->evTime);
    ev[0]->updateLCD_Temperature();
  }

  if((gt1_tLoss_pre != gtp[0]->stat[1].trackLossCnt) || (gtp[0]->stat[1].track_data == 0)){
    timingNetOK = 0;
    if(tsMode != RAON_EVF){
      prnM2("[ERR]gt1 trackLoss : %d   ", gtp[0]->stat[1].trackLossCnt - gt1_tLoss_pre);
      evr[1]->ip_prnTime(&evr[1]->evTime);
    }

    if(runCount%2 == 0)
      ev[0]->updateLCD_Message(LCD_COLOR_RED ,"[ERR] Timing Net !!");
    else
      ev[0]->updateLCD_Message(LCD_COLOR_BLUE,"[ERR] Timing Net !!");

    rx_evCode_miss = 1;
  }
  else{
    if(timingNetOK == 0){
      ev[0]->updateLCD_Message(LCD_COLOR_GREEN,"Timing Net OK");
      rx_evCode_miss = 0;
    }
    timingNetOK = 1;
  }
  gt1_tLoss_pre  = gtp[0]->stat[1].trackLossCnt;

  pwrOK = 1;
  fanERR = 0;
  epicsOK = 1;
  ev[0]->updateFrontPanel_LED(pwrOK, fanERR, epicsOK, timingNetOK);

  // ev[0]->set_sys_FMC_LED( (!gtp[0]->stat[0].track_data & 1) & ev[0]->f_SFP_LossA, (!gtp[0]->stat[1].track_data & 1) & ev[0]->f_SFP_LossB );
  // ev[0]->set_sys_FMC_LED( (gtp[0]->stat[0].track_data & 1) & !ev[0]->f_SFP_LossA, (gtp[0]->stat[1].track_data & 1) & !ev[0]->f_SFP_LossB );
  ev[0]->set_sys_FMC_LED( gtp[0]->stat[0].track_data , gtp[0]->stat[1].track_data );

  // evg[0]->ip_prnTime(&evg[0]->evTime);
  // evg[1]->ip_prnTime(&evg[1]->evTime);
  // evr[0]->ip_prnTime(&evr[0]->evTime);
  // evr[1]->ip_prnTime(&evr[1]->evTime);
  runCount++;

	return RET_OK;
}


//=====================================
//----===@ get_evTimeAll
// Parameters  :
// Description :
int tsDev_evx::get_evTimeAll(void)
{
  // prnM2("[F:%s]\n",__FUNCTION__);
  ifRet(RET_ERR == ev[0]->ip_setCommand(SET_getTime, 0));

  evg[0]->get_evTime();
  evg[1]->get_evTime();
  evr[0]->get_evTime();
  evr[1]->get_evTime();
  return RET_OK;
}

//=====================================
//----===@ get_evTimeStamp
// Parameters  :
// Description :
int tsDev_evx::get_evTimeStamp(epicsTimeStamp *epsT)
{
  // prnM2("[F:%s]\n",__FUNCTION__);
  ifRet(RET_ERR == ev[0]->ip_setCommand(SET_getTime, 0));
  evrMain->get_evTime();
  evrMain->ip_convEpsTime(&evrMain->evTime, epsT);
  return RET_OK;
}



//=====================================
//----===@ get_evTimeStamp
// Parameters  :
// Description :
int tsDev_evx::updateInfo()
{
  char strbuf[256];
  sprintf(strbuf,"ver%.3s.%.3s.%.3s.%.3s\r\n", ver_sw->chVer, ver_boot->chVer, ver_image->chVer, ver_pv->chVer );
  // printf("%s",strbuf);
  ev[0]->updateLCD_Ver(strbuf);
  return RET_OK;
}

int tsDev_evx::timingNet_timeSync(void)
{
  static struct timeval tv;
  static struct tm *info;
  static time_t tt;
  static int diff_time = 0;
  static int tmpV;

  gettimeofday(&tv, NULL);
  tmpV = 500000 - tv.tv_usec;
  if(tmpV<0) 
    tmpV = 500000;
  usleep(tmpV); 

  ifRet(RET_ERR == ev[0]->ip_setCommand(SET_getTime, 0));
  gettimeofday(&tv, NULL);
  evg[1]->get_evTime();

  diff_time = ((uint) evg[1]->evTime.nsec/1000) - (tv.tv_usec);

  if((diff_time > 250) || (diff_time < -250))
  {
    tt = tv.tv_sec;
    info = gmtime(&tt);
    evg[1]->setWatch(IDECODE_MODE_USER        , info->tm_year-100 , info->tm_yday+1, info->tm_hour, info->tm_min, info->tm_sec); //irig-b 1~366 //tm type 0~365
    evg[0]->setWatch(IDECODE_MODE_USER        , info->tm_year-100 , info->tm_yday+1, info->tm_hour, info->tm_min, info->tm_sec); //irig-b 1~366 //tm type 0~365
    usleep(300000); 

    gettimeofday(&tv, NULL);
    tmpV = 810970 - tv.tv_usec;
    if((tmpV<0) || (tmpV > 100000))
      tmpV = 100000;
    usleep(tmpV); 
    ev[0]->ip_setCommand(SET_igen_set_time, 5);
    // printf("+ %09d___",tv.tv_usec);
    printf("timingNet_timeSync : [diff] %09d : [evg] %09d : [sys] %09d - ",diff_time, evg[1]->evTime.nsec/1000, tv.tv_usec);
    
    // ev[0]->setIgen  (DR_ENABLE, IGEN_MODE_USER, info->tm_year-100 , info->tm_yday+1, info->tm_hour, info->tm_min, info->tm_sec); //irig-b 1~366 //tm type 0~365


    // ev[0]->setIgen  (DR_ENABLE, IGEN_MODE_USER, 20, 366, 14, 59, 0);
    // printf("timingNet_timeSync..................%09d\r\n", diff_time);
    // printf("2 %04d.%02d.%02d-%02d.%02d.%02d -- %09d\r\n", info->tm_year+1900, info->tm_mon+1, info->tm_mday, info->tm_hour, info->tm_min, info->tm_sec, tv.tv_usec);
  }
  // if(info->tm_year-100 evg[1]->evTime.tmp_year, info->tm_yday, info->tm_hour, info->tm_min, info->tm_sec
  // ev[0]->setIgen  (DR_ENABLE, IGEN_MODE_USER, info->tm_year-100 , info->tm_yday, info->tm_hour, info->tm_min, info->tm_sec);
  // printf("1 %04d.%02d.%02d-%02d.%02d.%02d -- %09d\r\n", evg[1]->evTime.tmT.tm_year, evg[1]->evTime.tmT.tm_mon, evg[1]->evTime.tmT.tm_mday, evg[1]->evTime.tmT.tm_hour, evg[1]->evTime.tmT.tm_min, evg[1]->evTime.tmT.tm_sec, evg[1]->evTime.nsec);
  // printf("2 %04d.%02d.%02d-%02d.%02d.%02d -- %09d\r\n", info->tm_year+1900, info->tm_mon+1, info->tm_mday, info->tm_hour, info->tm_min, info->tm_sec, tv.tv_usec);
  // info = localtime(&tt);


  return RET_OK;
}



//=====================================
//----===@ bist
// Parameters  :
// Description : built in self test
int tsDev_evx::bist(uint tMode)
{
  // int i;
  prnM2("%-20s: %d.\r\n", "[ Built in Self Test ]" , tMode);

  switch(tMode)
  {
    case 1 :  
              if(ev[0]  != NULL)ev[0]->ip_prnInfo();
              if(gtp[0] != NULL)gtp[0]->ip_prnInfo();
              break;
    case 2 :  
              if(evg[0] != NULL)evg[0]->ip_prnInfo();
              if(evg[1] != NULL)evg[1]->ip_prnInfo();
              break;
    case 3 :  
              if(evr[0] != NULL)evr[0]->ip_prnInfo();
              if(evr[1] != NULL)evr[1]->ip_prnInfo();
              break;
    case 4 :  
              if(ev[0]  != NULL)ev[0]->ip_prnInfo();
              if(gtp[0] != NULL)gtp[0]->ip_prnInfo();
              if(evg[0] != NULL)evg[0]->ip_prnInfo();
              if(evg[1] != NULL)evg[1]->ip_prnInfo();
              if(evr[0] != NULL)evr[0]->ip_prnInfo();
              if(evr[1] != NULL)evr[1]->ip_prnInfo();
              break;

    case 5 :  
              if(evr[0] != NULL)evr[0]->prnEvLog();
              if(evr[1] != NULL)evr[1]->prnEvLog();
              break;
              
#if 0
    case 10 : // [EVG0] mxc0:1Hz -> evCodeA[3] -> mevCodeA[0x100]
              // [EVR0] evCodeB[0x100] -> pdp Trigger All [delay:0~31, width:0~31]
              if(evr[0] != NULL)evr[0]->ip_setCommand(SET_pdp_rst, 1000);

              if(((tsMode == RAON_EVG) || (tsMode == RAON_EVS)) && (evg[0] != NULL))
              {
                // EVG --------------------------------------------------------
                //mxc0~7:1KHz~14KHz
                for(i=0;i<14;i++)evg[0]->setMxcFreq((ev_src)(MXC_00 + i), 1000.0 * (i+1));
                //mxc0:1Hz
                evg[0]->setMxcFreq((ev_src)(MXC_00), 1);
                //dbus soure signal select [ USER_TRIGGER ]
                for(i=0;i<8;i++)evg[0]->setDBusSel(i, USER_TRIGGER);
                //init evTrg source to user trigger
                for(i=0;i<31;i++)evg[0]->setTrg(i, USER_TRIGGER);
                //init mevCodeA bram
                for(i=0;i<31;i++)evg[0]->setMevCodeA(i, 0);
                //sequence repeat 1
                evg[0]->setSeqRepeat(0, 0);
                evg[0]->setSeqRepeat(1, 0);
                i=0; 
                evg[0]->setSeqTable(0, i, 0x100 +i, 1, i*500);
                evg[0]->setSeqTrg( seqS_usrTrg, seqS_usrTrg, \
                                 seqS_usrTrg, seqS_usrTrg, \
                                 seqS_CMB_OR0, seqS_CMB_NONE);

                //mxc0 -> evCodeA[3]
                evg[0]->setTrg(3, MXC_00 );

                //evCodeA[3] -> mevCodeA[0x100]
                evg[0]->setMevCodeA(3, 0x100);
              }

              // EVR --------------------------------------------------------
              //init evCodeB mapped bram
              if(evr[0] != NULL)
              {
                for(i=0;i<2048;i++)evr[0]->setEvRam(i, 0);
                for(i=0;i<2048;i++)evr[0]->setEvRamN(i, 0);

                //init pdp
                //set pdp config & pdp trigger from evCodeB & exOut noSwap
                for(i=0;i<32;i++){
                  //           pdpNum, delay,    width,    pol, trgFromEvCodeA
                  evr[0]->cfgPdp(i,      812500*i, 40625000, 0,   0             ); // pdp [delay:0~31 ms, width:500ms] pol high
                  if(i<8)
                    evr[0]->cfgOut((ev_out)(EXT_OUT_SL_00 + i), (ev_out_src)(PDP_OUT_00 + i));
                  else
                    evr[0]->cfgOut((ev_out)(EXT_OUT_SL_00 + i), (ev_out_src)(PDP_OUT_00 + i));
                }
                evr[0]->setOut(0); //setOut(uint extOutSwap)

                //evCodeB[0x100] -> pdp Trigger All
                evr[0]->setEvRam(0x100, 0xffffffff);
              }

              if(((tsMode == RAON_EVG) || (tsMode == RAON_EVS)) && (evg[0] != NULL))
              {
                // Enable --------------------------------------------------------
                // evg                dbus, evCodeA, evCodeB, mevCodeA, seqA, seqB, mxc
                evg[0]->enable(0,    0,       1,       1,        0,    0,    1); 
              }

              // evr                exOutEnable, rxEnable
              if(evr[0] != NULL)evr[0]->enable(1,           1);

              for(i=0;i<2;i++){
                taskDelay(1000000);
                bist(3);
              }
              break;

    case 11 : // [EVG0] mxc0~7:1Hz~8Hz -> txDBus
              // [EVR0] rxDBus
              if(evr[0] != NULL)evr[0]->ip_setCommand(SET_pdp_rst, 1000);

              if((tsMode == RAON_EVG) || (tsMode == RAON_EVS))
              {
                // EVG --------------------------------------------------------
                //mxc0~7:1Hz~14Hz
                for(i=0;i<14;i++)evg[0]->setMxcFreq((ev_src)(MXC_00 + i), (i+1));
                //dbus soure signal select [MXC00 ~ 14]
                for(i=0;i<8 ;i++)evg[0]->setDBusSel(i, (ev_src)(MXC_00 + i));
                //init evTrg source to user trigger
                for(i=0;i<31;i++)evg[0]->setTrg(i, USER_TRIGGER);
                //init mevCodeA bram
                for(i=0;i<31;i++)evg[0]->setMevCodeA(i, 0);

                //sequence repeat 1
                evg[0]->setSeqRepeat(0, 0);
                evg[0]->setSeqRepeat(1, 0);
                i=0; 
                evg[0]->setSeqTable(0, i, 0x100 +i, 1, i*500);
                evg[0]->setSeqTrg(  seqS_usrTrg, seqS_usrTrg, \
                                  seqS_usrTrg, seqS_usrTrg, \
                                  seqS_CMB_OR0, seqS_CMB_NONE);
              }

              // EVR --------------------------------------------------------
              //init evCodeB mapped bram
              if(evr[0] != NULL)
              {
                for(i=0;i<2048;i++)evr[0]->setEvRam(i, 0);
                for(i=0;i<2048;i++)evr[0]->setEvRamN(i, 0);

                //init pdp
                //set pdp config & pdp trigger from evCodeB & exOut noSwap
                for(i=0;i<32;i++){
                  //           pdpNum, delay,    width,    pol, trgFromEvCodeA
                  evr[0]->cfgPdp(i,      i,        i,        0,   0             ); // pdp [delay:0~31, width:0~31] pol high

                  if(i<8)
                    evr[0]->cfgOut((ev_out)(EXT_OUT_SL_00 + i), (ev_out_src)(RXDBUS_00 + i));
                  else
                    evr[0]->cfgOut((ev_out)(EXT_OUT_SL_00 + i), (ev_out_src)(PDP_OUT_00 + i));
                }
                evr[0]->setOut(0); //setOut(uint extOutSwap)
              }
              
              // Enable --------------------------------------------------------
              if(((tsMode == RAON_EVG) || (tsMode == RAON_EVS)) && (evg[0] != NULL))
              {
                // evg       dbus, evCodeA, evCodeB, mevCodeA, seqA, seqB, mxc
                evg[0]->enable(1,    0,       0,       0,        0,    0,    1); 
              }

              // evr       exOutEnable, rxEnable
              if(evr[0] != NULL)evr[0]->enable(1,           1);

              for(i=0;i<2;i++){
                taskDelay(1000000);
                bist(3);
              }
              break;

    case 12 : // [EVG0] MXC00 [1] trigger sequence & 0x100 ~ 0x103 evCode Tx(interval 500)
              // [EVR0] evCodeB[0x100~103] -> pdp Trigger 0~3
              if(evr[0] != NULL)evr[0]->ip_setCommand(SET_pdp_rst, 1000);

              if(((tsMode == RAON_EVG) || (tsMode == RAON_EVS)) && (evg[0] != NULL))
              {
                // EVG --------------------------------------------------------
                //mxc0~7:1~14
                for(i=0;i<14;i++)evg[0]->setMxcFreq((ev_src)(MXC_00 + i), (i+1));

                //dbus soure signal select [MXC00 ~ 14]
                for(i=0;i<8;i++)evg[0]->setDBusSel(i, USER_TRIGGER);

                //init evTrg source to user trigger
                for(i=0;i<31;i++)evg[0]->setTrg(  i, USER_TRIGGER);

                //init mevCodeA bram
                for(i=0;i<31;i++)evg[0]->setMevCodeA(i, 0);

                //sequence repeat infinite 
                evg[0]->setSeqRepeat(0, -1);
                evg[0]->setSeqRepeat(1, -1);

                i=0; evg[0]->setSeqTable(0, i, 0x100 +i, 0, i*500);
                i++; evg[0]->setSeqTable(0, i, 0x100 +i, 0, i*500);
                i++; evg[0]->setSeqTable(0, i, 0x100 +i, 0, i*500);
                i++; evg[0]->setSeqTable(0, i,        0, 1, i*500);

                evg[0]->setSeqTrg(  seqS_usrTrg | seqS_mxc00, \
                                  seqS_usrTrg, \
                                  seqS_usrTrg | seqS_mxc00, \
                                  seqS_usrTrg, \
                                  seqS_CMB_OR0, seqS_CMB_NONE);
              }

              // EVR --------------------------------------------------------
              //init evCodeB mapped bram
              if(evr[0] != NULL)
              {
                for(i=0;i<2048;i++)evr[0]->setEvRam(i, 0);
                for(i=0;i<2048;i++)evr[0]->setEvRamN(i, 0);

                //init pdp
                //set pdp config & pdp trigger from evCodeB & exOut noSwap
                for(i=0;i<32;i++){
                  //           pdpNum, delay,    width,    pol, trgFromEvCodeA
                  evr[0]->cfgPdp(i,      812500*i, 40625000, 0,   0             ); // pdp [delay:0~31 ms, width:500ms] pol high
                  if(i<8)
                    evr[0]->cfgOut((ev_out)(EXT_OUT_SL_00 + i), (ev_out_src)(RXDBUS_00 + i));
                  else
                    evr[0]->cfgOut((ev_out)(EXT_OUT_SL_00 + i), (ev_out_src)(PDP_OUT_00 + i));
                }
                evr[0]->setOut(0); //setOut(uint extOutSwap)

                //evCodeB[0x100] -> pdp Trigger All
                evr[0]->setEvRam(0x100, 0x10000 << 0);
                evr[0]->setEvRam(0x101, 0x10000 << 1);
                evr[0]->setEvRam(0x102, 0x10000 << 2);
                evr[0]->setEvRam(0x103, 0x10000 << 3);
              }

              // Enable --------------------------------------------------------
              if(((tsMode == RAON_EVG) || (tsMode == RAON_EVS)) && (evg[0] != NULL))
              {
                // evg       dbus, evCodeA, evCodeB, mevCodeA, seqA, seqB, mxc
                evg[0]->enable(0,    0,       1,       0,        1,    0,    1); 
              }

              // evr       exOutEnable, rxEnable
              if(evr[0] != NULL)evr[0]->enable(1,           1);

              for(i=0;i<2;i++){
                taskDelay(1000000);
                bist(3);
              }
              break;

    case 13 : // latch test
              if(evr[0] != NULL)evr[0]->ip_setCommand(SET_pdp_rst, 1000);

              if(((tsMode == RAON_EVG) || (tsMode == RAON_EVS)) && (evg[0] != NULL))
              {
                // EVG --------------------------------------------------------
                //mxc0~7:1~14
                for(i=0;i<14;i++)evg[0]->setMxcFreq((ev_src)(MXC_00 + i), (i+1));

                //dbus soure signal select [MXC00 ~ 14]
                for(i=0;i<8;i++)evg[0]->setDBusSel(i, USER_TRIGGER);

                //init evTrg source to user trigger
                for(i=0;i<31;i++)evg[0]->setTrg(  i, USER_TRIGGER);

                //init mevCodeA bram
                for(i=0;i<31;i++)evg[0]->setMevCodeA(i, 0);

                //sequence repeat infinite 
                evg[0]->setSeqRepeat(0, 0);
                evg[0]->setSeqRepeat(1, 0);

                evg[0]->setSeqTrg(  seqS_usrTrg, \
                                  seqS_usrTrg, \
                                  seqS_usrTrg, \
                                  seqS_usrTrg, \
                                  seqS_CMB_OR0, seqS_CMB_NONE);
              }

              // EVR --------------------------------------------------------
              //init evCodeB mapped bram
              if(evr[0] != NULL)
              {
                for(i=0;i<2048;i++)evr[0]->setEvRam(i, 0);
                for(i=0;i<2048;i++)evr[0]->setEvRamN(i, 0);

                evr[0]->setEvRam (1, 0x0000ffff);
                evr[0]->setEvRamN(2, 0x0000ffff);
                evr[0]->setEvRam (3, 0xffff0000);
                evr[0]->setEvRamN(4, 0xffff0000);

                //init pdp
                //set pdp config & pdp trigger from evCodeB & exOut noSwap
                for(i=0;i<32;i++){
                  //           pdpNum, delay,    width,      pol, trgFromEvCodeA
                  evr[0]->cfgPdp(i,      812500*i, 0xffffffff, 0,   0             ); // pdp [delay:0~31 ms, width:500ms] pol high

                  evr[0]->cfgOut((ev_out)(EXT_OUT_SL_00 + i), (ev_out_src)(PDP_OUT_00 + i));
                }
                evr[0]->setOut(0); //setOut(uint extOutSwap)
              }
              
              // Enable --------------------------------------------------------
              if(((tsMode == RAON_EVG) || (tsMode == RAON_EVS)) && (evg[0] != NULL))
              {
                // evg                dbus, evCodeA, evCodeB, mevCodeA, seqA, seqB, mxc
                evg[0]->enable(0,    0,       1,       0,        0,    0,    0); 
              }

              // evr                exOutEnable, rxEnable
              if(evr[0] != NULL)evr[0]->enable(1,           1);

              break;              

    case 19 : // default set
              if(evr[0] != NULL)evr[0]->ip_setCommand(SET_pdp_rst, 1000);

              if(((tsMode == RAON_EVG) || (tsMode == RAON_EVS)) && (evg[0] != NULL))
              {
                // EVG --------------------------------------------------------
                //mxc0~7:1~14
                for(i=0;i<14;i++)evg[0]->setMxcFreq((ev_src)(MXC_00 + i), (i+1));

                //dbus soure signal select [MXC00 ~ 14]
                for(i=0;i<8;i++)evg[0]->setDBusSel(i, USER_TRIGGER);

                //init evTrg source to user trigger
                for(i=0;i<31;i++)evg[0]->setTrg(  i, USER_TRIGGER);

                //init mevCodeA bram
                for(i=0;i<31;i++)evg[0]->setMevCodeA(i, 0);

                //sequence repeat infinite 
                evg[0]->setSeqRepeat(0, 0);
                evg[0]->setSeqRepeat(1, 0);

                evg[0]->setSeqTrg(  seqS_usrTrg, \
                                  seqS_usrTrg, \
                                  seqS_usrTrg, \
                                  seqS_usrTrg, \
                                  seqS_CMB_OR0, seqS_CMB_NONE);
              }

              // EVR --------------------------------------------------------
              //init evCodeB mapped bram
              if(evr[0] != NULL)
              {
                for(i=0;i<2048;i++)evr[0]->setEvRam(i, 0);

                //init pdp
                //set pdp config & pdp trigger from evCodeB & exOut noSwap
                for(i=0;i<32;i++){
                  //           pdpNum, delay,    width,    pol, trgFromEvCodeA
                  evr[0]->cfgPdp(i,      812500*i, 40625000, 0,   0             ); // pdp [delay:0~31 ms, width:500ms] pol high
                  if(i<8)
                    evr[0]->cfgOut((ev_out)(EXT_OUT_SL_00 + i), (ev_out_src)(RXDBUS_00 + i));
                  else
                    evr[0]->cfgOut((ev_out)(EXT_OUT_SL_00 + i), (ev_out_src)(PDP_OUT_00 + i));
                }
                evr[0]->setOut(0); //setOut(uint extOutSwap)
              }
              
              // Enable --------------------------------------------------------
              if(((tsMode == RAON_EVG) || (tsMode == RAON_EVS)) && (evg[0] != NULL))
              {
                // evg                dbus, evCodeA, evCodeB, mevCodeA, seqA, seqB, mxc
                evg[0]->enable(0,    0,       0,       0,        0,    0,    0); 
              }

              // evr                exOutEnable, rxEnable
              if(evr[0] != NULL)evr[0]->enable(0,           0);

              break;              
#endif
    default:
              break;
  }

  return RET_OK;
}  




//=====================================
//=====================================
} //name space end




