//Wrapping class for system calls of timing device driver.
#include <stdlib.h>
#include <termios.h>

#include "timingData.h"
#include "tsDev_zq9r.h"


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
#define DRV_NAME_mp_x "/dev/mps_pico_0000"
#define DRV_NAME_mp_x_bar2 "/sys/bus/pci/devices/0000"
#define DRV_NAME_mp_x_bar2sub "00.0/resource2"


//=====================================
//----===@ tsDev_zq9r
// Parameters  :
// Description :
tsDev_zq9r::tsDev_zq9r(const int opModeV, const int tsModeV, const int tsClassV, const char* tsNameV, const int tsNumV, uint tzone, uint tickPeriod)
{
  opMode      = opModeV;
  tsMode      = tsModeV;
  tsClass     = tsClassV;
  tsNum       = tsNumV;
  sprintf(tsName,"%s", tsNameV);

  // prnM2("[F:%s]\n",__FUNCTION__);
  int i,j;

  int fd;
  char mpDevName[128];
  char mpDevName_bar2[128];

  for(i=0;i<ipIdCntMax;i++){
    zq[i] = NULL;
    mp[i] = NULL;
  }
  evr[0] = new ts2ipEvr(DRV_NAME_ip_evr0 , tzone, tickPeriod);

  gtp[0] = new ts2ipGtp(DRV_NAME_ip_gtp  );

  zq[0] = new ts2ipZqsys(DRV_NAME_ip_zqsys  );

  ifJstRet(zq[0]->isInit == -1);
  ifJstRet(gtp[0]->isInit == -1);
  ifJstRet(evr[0]->isInit == -1);

  gtp[0]->reset();
  taskDelay(10000);

  //    enable(uint exOut, uint rxTNet );
  evr[0]->enable( 0, 0);
  evr[0]->ip_intrClear();
  evr[0]->ip_intrMask(0 \
      | C_intrMsk_evrW_alarm_sec \
      | C_intrMsk_Emergency \
      | C_intrMsk_evCodeB_7F2 \
      );


  evrMain = evr[0];
  
  j=0;
  for(i=0;i<15;i++){
    sprintf(mpDevName,"%s:%02x:00.0",DRV_NAME_mp_x,i);
    sprintf(mpDevName_bar2,"%s:%02x:%s",DRV_NAME_mp_x_bar2,i,DRV_NAME_mp_x_bar2sub);

    fd  = open(mpDevName,O_RDWR);
    if(fd < 0){
      // prnM3("tsDev_zq9r device open fail: %s\r\n", mpDevName);
    }
    else{
      close(fd);
      fd = -1;
      mp[j] = new drMpsPico(mpDevName,mpDevName_bar2); 
      prnM3("tsDev_zq9r device open : %s\r\n", mpDevName);
      if(j>3)break;
      j++;
    }
  }


  prnM2("tsDev_zq9r init ok %d.\r\n",isInit);
  isInit = 1;
}

//=====================================
//----===@ ~tsDev_zq9r
// Parameters  :
// Description :
tsDev_zq9r::~tsDev_zq9r()
{
  delete zq[0]  ;
  delete gtp[0] ;
  delete evr[0] ;

  isInit = -1;
  prnM2("[F:%s]\n",__FUNCTION__);
}

//=====================================
//----===@ readAppData
// Parameters  :
// Description :
asynStatus tsDev_zq9r::readAppData(uint offset, uint *val)
{
  asynStatus status = asynSuccess;

  *val = 0;
  return status;
}

//=====================================
//----===@ writeAppData
// Parameters  :
// Description :
asynStatus tsDev_zq9r::writeAppData(uint offset, uint val)
{
  asynStatus status = asynSuccess;
  unsigned int addr = offset/4;

  switch(addr)
  {
    case 0x1100   : if(zq[0] == NULL){return asynError;} bist(val); break;
    default       : status = asynError; break;
  }
  return status;
}

//=====================================
//----===@ readReg
// Parameters  :
// Description :
asynStatus tsDev_zq9r::readReg(const RegMap &rmap, int &val)
{
  // prnM2("[F:%s]\n",__FUNCTION__);
  int rdData=-1;
  asynStatus status = asynSuccess;
  timing::ts2ipDrv  *pTsIp = NULL;
  timing::drMpsPico *pMpIp = NULL;

  val = -1;
  // prnM2("readReg %s\r\n", rmap.drvname);

  if(rmap.accType == accType_app){
    readAppData(rmap.address, (unsigned int*)&rdData);
  }
  else{
    switch(rmap.ipId){
      case ipId_zq  : if(zq[rmap.ipCnt]  == NULL){return asynError;}
                      pTsIp = zq[rmap.ipCnt];  
                      break;
      case ipId_gtp : if(gtp[rmap.ipCnt] == NULL){return asynError;}
                      pTsIp = gtp[rmap.ipCnt]; 
                      break;
      case ipId_evr : if(evr[rmap.ipCnt] == NULL){return asynError;}
                      pTsIp = evr[rmap.ipCnt]; 
                      break;
      case ipId_mp  : if(mp[rmap.ipCnt]  == NULL){return asynError;}
                      pMpIp = mp[rmap.ipCnt];  
                      break;
      default : return asynError;
    }

    if(pTsIp != NULL)
      pTsIp->ip_rd(rmap.accType, rmap.address, (unsigned int*)&rdData);
    else if(pMpIp != NULL)
      pMpIp->ip_rd(rmap.accType, rmap.address, (unsigned int*)&rdData);
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
asynStatus tsDev_zq9r::readString(const RegMap &rmap, char *val)
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
asynStatus tsDev_zq9r::writeReg(const RegMap &rmap, const int val)
{
  // prnM2("[F:%s]\n",__FUNCTION__);
  int rdData=-1;
  int value=val;
  int wrData;
  asynStatus status = asynSuccess;
  timing::ts2ipDrv  *pTsIp = NULL;
  timing::drMpsPico *pMpIp = NULL;
  int tmpData=0;

  // prnM2("writeReg %s\r\n", rmap.drvname);

  if(rmap.accType == accType_app){
    writeAppData(rmap.address, (unsigned int)val);
  }
  else{
    switch(rmap.ipId){
      case ipId_zq  : if(zq[rmap.ipCnt]  == NULL){return asynError;}
                      pTsIp = zq[rmap.ipCnt];  
                      break;
      case ipId_gtp : if(gtp[rmap.ipCnt] == NULL){return asynError;}
                      pTsIp = gtp[rmap.ipCnt]; 
                      break;
      case ipId_evr : if(evr[rmap.ipCnt] == NULL){return asynError;}
                      pTsIp = evr[rmap.ipCnt]; 
                      break;
      case ipId_mp  : if(mp[rmap.ipCnt]  == NULL){return asynError;}
                      pMpIp = mp[rmap.ipCnt];  
                      break;
      default : return asynError;
    }

    if(pTsIp != NULL)
      pTsIp->ip_rd(rmap.accType, rmap.address, (unsigned int*)&rdData);
    else if(pMpIp != NULL)
      pMpIp->ip_rd(rmap.accType, rmap.address, (unsigned int*)&rdData);
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
      else if(pMpIp != NULL){
        pMpIp->ip_wr(rmap.accType, rmap.address, (unsigned int)value);
        pMpIp->ip_wr(rmap.accType, rmap.address, 0);
      }
      else
        return asynError;
    }
    else{
      wrData = rdData & tmpData;
      wrData |= value;

      if(pTsIp != NULL)
        pTsIp->ip_wr(rmap.accType, rmap.address, (unsigned int)wrData);
      else if(pMpIp != NULL)
        pMpIp->ip_wr(rmap.accType, rmap.address, (unsigned int)wrData);
      else
        return asynError;
    }
  }
  return status;
}




//=====================================
//----===@ readRegArray
// Parameters  :
// Description :
asynStatus tsDev_zq9r::readRegArray(const RegMap &rmap, void *val)
{
  // prnM2("[F:%s]\n",__FUNCTION__);
  // uint i;
  uint rdData=-1;
  // float* pfData;

  asynStatus status = asynSuccess;
  timing::drMpsPico *pMpIp = NULL;

  switch(rmap.ipId){
    case ipId_mp  : if(mp[rmap.ipCnt]  == NULL){return asynError;}
                    pMpIp = mp[rmap.ipCnt];  
                    break;
    default : return asynError;
  }

  // if(dataType_Waveform_int == rmap.dataType){
  //   else if(pMpIp != NULL){
  //     for(i=0;i<rmap.dataSize;i++){
  //       pMpIp->ip_rd(rmap.accType, rmap.address + i*4, (unsigned int*)&rdData);
  //       ((int*)val)[i] = rdData;
  //     }
  //   }
  //   else{ return asynError;}
  // }
  if(dataType_Waveform_f32 == rmap.dataType){
    printf("dataType_Waveform_f32 %s(%s) : nEle(%d)\n",__FUNCTION__, rmap.drvname, rmap.dataSize);
    // for(i=0;i<rmap.dataSize;i++) 
    //   ((float*)val)[i] = 0.01*i;
    pMpIp->ip_rd(addrType_bar0, 0x29C, &rdData);
    printf("rdData 0x%08x\n",rdData);
    
    if(0x2 == (rdData & 0x2)){
      pMpIp->bar2_rd((off_t) rmap.address, rmap.dataSize, (float*)val);
    }
    else{
      prnErr();
      return asynError;
    }

  }
  else{
    return asynError;
  }

  // else if(dataType_Waveform_f64 == rmap.dataType){
    // printf("dataType_Waveform_f64 %s(%s) : nEle(%d)\n",__FUNCTION__, rmap.drvname, rmap.dataSize);
    // if(pTsIp != NULL){
    //   for(i=0;i<rmap.dataSize;i++){
    //     pTsIp->ip_rd(rmap.accType, rmap.address + i*4, (unsigned int*)&rdData);
    //     pfData = ((float*)&rdData) ;
    //     ((epicsFloat64*)val)[i]  = 1000000000 * (epicsFloat64) (*pfData) ;
    //   }
    // }
    // else if(pMpIp != NULL){
    //   // printf("%s(%s) : nEle(%d)\n",__FUNCTION__, rmap.drvname, rmap.dataSize);

    //   // for(i=0;i<rmap.dataSize;i++){
    //   for(i=0;i<4;i++){
    //     pMpIp->bar2_rd((off_t)rmap.address+i*4, (unsigned int*)&rdData);
    //   }
    //     // pfData = ((float*)&rdData) ;
    //     // printf("%s(%s) : nEle(%d)-------> 0x%08x %f\n",__FUNCTION__, rmap.drvname, rmap.dataSize,rmap.address + i*4, *pfData * 1000000000.0);
    //     // ((epicsFloat64*)val)[i]  = 1000000000 * (epicsFloat64) (*pfData) ;
    //     // ((epicsFloat64*)val)[i]  = (epicsFloat64) i ;
    //   // }
    // }
    
    // if(pMpIp != NULL){
    //   // printf("%s(%s) : nEle(%d)\n",__FUNCTION__, rmap.drvname, rmap.dataSize);

    //   // for(i=0;i<rmap.dataSize;i++){
    //   for(i=0;i<4;i++){
    //     pMpIp->bar2_rd((off_t)rmap.address+i*4, (unsigned int*)&rdData);
    //   }
    //     // pfData = ((float*)&rdData) ;
    //     // printf("%s(%s) : nEle(%d)-------> 0x%08x %f\n",__FUNCTION__, rmap.drvname, rmap.dataSize,rmap.address + i*4, *pfData * 1000000000.0);
    //     // ((epicsFloat64*)val)[i]  = 1000000000 * (epicsFloat64) (*pfData) ;
    //     // ((epicsFloat64*)val)[i]  = (epicsFloat64) i ;
    //   // }
    // }
    // else{ return asynError;}
  // }

  return status;
}

//=====================================
//----===@ writeRegArray
// Parameters  :
// Description :
asynStatus tsDev_zq9r::writeRegArray(const RegMap &rmap, const void *val)
{
  val = 0;
  prnM2("writeRegArray %s\r\n", rmap.drvname);
  return asynSuccess;
}


//=====================================
//----===@ delayCheck_Proc
// Parameters  :
// Description :
void tsDev_zq9r::delayCheck_Proc(void)
{
  evrMain->get_evgCommand();
  // evr[0]->evgCmd;
  // evr[0]->evgCmdSub;
};

//=====================================
//----===@ devProc
// Parameters  :
// Description :
int tsDev_zq9r::devProc(void)
{
  // prnM2("[F:%s]\n",__FUNCTION__);
  get_evTimeAll();

  gtp[0]->getStat();
  evr[0]->getStat();
  
#ifdef Test_drMpsPico
  // mp[4]->readData();
#endif
  // int i;
  // long rtVal;
  // char pname[128];
	// DBADDR pdbAddr;
  // sprintf(pname,"%s:%s","sysDevName","pvname");
	// rtVal=dbNameToAddr(pname, &pdbAddr);

  // long *pfieldLink = (long *)pdbAddr.pfield;
  
  // waveformRecord *precordLink = (waveformRecord *)pdbAddr.precord;

  // for(i=0;i<1024;i++)
  // {
  //   pfieldLink[i] = (unsigned long)1;
  //   // printf("pfieldLink[index] : %ld\r\n", (unsigned long)pfieldLink[index]);
  // };  

	// precordLink->nord = i+1;
	// dbProcess((dbCommon*)precordLink);

	// if (rtVal) {
  //   prnM3("[ERR] Record '%s' not found\n", pname);
  //   return RET_ERR;
	// }


	return RET_OK;
}


//=====================================
//----===@ get_evTimeAll
// Parameters  :
// Description :
int tsDev_zq9r::get_evTimeAll(void)
{
  // prnM2("[F:%s]\n",__FUNCTION__);
  ifRet(RET_ERR == zq[0]->ip_setCommand(SET_getTime, 0));
  evr[0]->get_evTime();
  return RET_OK;
}

//=====================================
//----===@ get_evTimeStamp
// Parameters  :
// Description :
int tsDev_zq9r::get_evTimeStamp(epicsTimeStamp *epsT)
{
  // prnM2("[F:%s]\n",__FUNCTION__);
  ifRet(RET_ERR == zq[0]->ip_setCommand(SET_getTime, 0));
  evrMain->get_evTime();
  evrMain->ip_convEpsTime(&evrMain->evTime, epsT);
  return RET_OK;
}







//=====================================
//----===@ bist
// Parameters  :
// Description : built in self test
int tsDev_zq9r::bist(uint tMode)
{
  // int i;
  prnM2("%-20s: %d.\r\n", "[ Built in Self Test ]" , tMode);

  switch(tMode)
  {
    case 1 :  
              if(zq[0]  != NULL)zq[0]->ip_prnInfo();
              if(gtp[0] != NULL)gtp[0]->ip_prnInfo();
              break;
    case 2 :  
              break;
    case 3 :  
              if(evr[0] != NULL)evr[0]->ip_prnInfo();
              break;
    case 4 :  
              if(zq[0]  != NULL)zq[0]->ip_prnInfo();
              if(gtp[0] != NULL)gtp[0]->ip_prnInfo();
              if(evr[0] != NULL)evr[0]->ip_prnInfo();
              break;

    case 5 :  
              if(evr[0] != NULL)evr[0]->prnEvLog();
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




