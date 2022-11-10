#include <cstdlib>
#include <cstring>
#include <bitset>
#include <epicsTypes.h>
#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsString.h>
#include <epicsTimer.h>
#include <epicsMutex.h>
#include <epicsEvent.h>
#include <registryFunction.h>
#include <epicsExport.h>
#include <dbCommon.h>
#include <iocsh.h>

/* for DB put *********/
#include "dbDefs.h"
#include "dbBase.h"
#include "dbScan.h"
#include "dbStaticLib.h"
#include <dbAccess.h>
#include "dbAccessDefs.h"
#include "db_access_routines.h"
#include <waveformRecord.h>
/* for DB put *********/

#include "iniparser.hpp"
#include "timingAsynEpics.h"

//#define tUpdate

static void userP_main(void *drvPvt);
static void userP_timeSync(void *drvPvt);

using timing::tsDev_evx;
using timing::tsDev_zq9r;

static int timingDebug = 1;
epicsExportAddress(int, timingDebug);

static int timingPrintCount = 20;
epicsExportAddress(int, timingPrintCount);


timingAsynEpics::timingAsynEpics( const char *portName, int maxSizeSnapshot, int maxSizeBufferMB, int clientMode,                 \
                                  const char* serverPath, const char* localPath,                                                  \
                                  const char *deviceSys, const char *deviceSubSys, const char *deviceName, const char *deviceNum,   \
                                  const int opMode, const int tzone, const int tickPeriod, const int reserved3, const int reserved4, const int reserved5 )
  : asynPortDriver(portName, 
      1,  /*maxAddr */ 
      /*0xC600, maxAddr*/
                    15000,
                    asynInt32Mask | asynUInt32DigitalMask | asynFloat64Mask | asynInt16ArrayMask | asynInt32ArrayMask| asynFloat32ArrayMask| asynFloat64ArrayMask | asynEnumMask | asynOctetMask | asynDrvUserMask, /* Interface mask */
                    asynInt32Mask | asynUInt32DigitalMask | asynFloat64Mask | asynInt16ArrayMask | asynInt32ArrayMask| asynFloat32ArrayMask| asynFloat64ArrayMask | asynEnumMask | asynOctetMask,  /* Interrupt mask */
                    1, /* asynFlags.  This driver does not block and it is not multi-device, so flag is 0 */
                    1, /* Autoconnect */
                    0, /* Default priority */
                    0 /* Default stack size*/
                    ),clientThreadMode(clientMode),driverName("timingAsynEpics"),system_init_ok(1) 
{
  printf("GCC Version-(%d)\n", GCC_VERSION);
  asynStatus status;

  char tsName[128];
  int  tsMode;
  int  tsClass;
  int  tsNum = strtol(deviceNum,NULL,10);
  double tmp_tickPeriod = tickPeriod / 10000000;

  if(     strcmp(deviceName, "EVG"  ) == 0 || strcmp(deviceName, "evg") == 0)  { tsMode = RAON_EVG;    sprintf(tsName, "%s%03d", "EVG"     , tsNum ); tsClass = TS_CLASS_EVX  ; }
  else if(strcmp(deviceName, "EVR"  ) == 0 || strcmp(deviceName, "evr") == 0)  { tsMode = RAON_EVR;    sprintf(tsName, "%s%03d", "EVR"     , tsNum ); tsClass = TS_CLASS_EVX  ; }
  else if(strcmp(deviceName, "EVF"  ) == 0 || strcmp(deviceName, "evf") == 0)  { tsMode = RAON_EVF;    sprintf(tsName, "%s%03d", "EVF"     , tsNum ); tsClass = TS_CLASS_EVX  ; }
  else if(strcmp(deviceName, "EVS"  ) == 0 || strcmp(deviceName, "evs") == 0)  { tsMode = RAON_EVS;    sprintf(tsName, "%s%03d", "EVS"     , tsNum ); tsClass = TS_CLASS_EVX  ; }
  else if(strcmp(deviceName, "EVRUP") == 0 || strcmp(deviceName, "evrup") == 0){ tsMode = RAON_EVRUP;  sprintf(tsName, "%s%03d", "EVRUP"   , tsNum ); tsClass = TS_CLASS_EVX  ; }
  else if(strcmp(deviceName, "ZQ9R" ) == 0 || strcmp(deviceName, "zq9r" ) == 0){ tsMode = RAON_ZQ9R;   sprintf(tsName, "%s%03d", "ZQ9R"    , tsNum ); tsClass = TS_CLASS_ZQ9R ; }
  else{ prnM3("[ERR] tsName[%s_%03d] Setting Error - Exiting\r\n", deviceName, tsNum); return; }

  //const int opMode, const int tsMode, const int tsClass, const char* tsName, const int tsNum, const double firmwareVer, const double softwareVer, 
  if(tsClass == TS_CLASS_EVX)
    pTsDev = new tsDev_evx( opMode, tsMode, tsClass, tsName, tsNum, tzone, tmp_tickPeriod);
  else if(tsClass == TS_CLASS_ZQ9R)
    pTsDev = new tsDev_zq9r(opMode, tsMode, tsClass, tsName, tsNum, tzone, tmp_tickPeriod);
  else
    return;

  if(pTsDev->isInit != 1) return;

  string verFileName_sw;
  string verFileName_boot;
  string verFileName_image;
  string verFileName_pv;
  
  verFileName_sw = serverPath;
  verFileName_boot = localPath;
  verFileName_image = localPath;
  verFileName_pv = localPath;

  verFileName_sw += "/app/vers_sw.sh";
  verFileName_boot += "/verl_boot.sh";
  verFileName_image += "/verl_image.sh";
  verFileName_pv += "/verl_tspv.sh";

  pTsDev->ver_sw    = new verMan("sw", verFileName_sw);
  pTsDev->ver_boot  = new verMan("boot", verFileName_boot);
  pTsDev->ver_image = new verMan("image", verFileName_image);
  pTsDev->ver_pv    = new verMan("pv", verFileName_pv);

  prnM2("\r\n");
  prnM2("======================================================\r\n");
  prnM2("++ %s timingApp ++\r\n", tsName);
  prnM2("======================================================\r\n");
  prnM2("   %-7s : %s\n", pTsDev->ver_sw->verName.c_str() , pTsDev->ver_sw->strVer.c_str());
  prnM2("   %-7s : %s\n", pTsDev->ver_boot->verName.c_str() , pTsDev->ver_boot->strVer.c_str());
  prnM2("   %-7s : %s\n", pTsDev->ver_image->verName.c_str() , pTsDev->ver_image->strVer.c_str());
  prnM2("   %-7s : %s\n", pTsDev->ver_pv->verName.c_str() , pTsDev->ver_pv->strVer.c_str());
  prnM2("------------------------------------------------------\r\n");
  pTsDev->updateInfo();

  // Create database entries
  eventId_ = epicsEventCreate(epicsEventEmpty);  

  // register parameter list from register file.  
  regFileName += localPath;
  regFileName += "/tspv.reg";
  registerParamListFromFile(regFileName);

  string pvInitVal_path;
  pvInitVal_path = serverPath;
  pvInitVal_path += "/local-env";

  sprintf(sysDevName, "%s:%s:%s", deviceSys, deviceSubSys, pTsDev->tsName);
  sprintf(iValFileName, "%s/pv/%s.initValue/%s.%s.%s.", pvInitVal_path.c_str(), deviceSys, deviceSys, deviceSubSys, pTsDev->tsName);

  INI::File ft;

  isInit = -1;
  // OPI Java Script Logic
  if (system_init_ok==1){
    status = (asynStatus)(epicsThreadCreate("timingAsynEpicsUserTask",
                                            epicsThreadPriorityMedium,
                                            epicsThreadGetStackSize(epicsThreadStackMedium),
                                            (EPICSTHREADFUNC)::userP_main,
                                            this) == NULL);
    if (status) {
      printf("%s:%s: epicsThreadCreate failure\n", driverName, __FUNCTION__);
      return;
    }
  }

  if (( (tsMode == RAON_EVG) || (tsMode == RAON_EVS) ) && (system_init_ok==1)){
    status = (asynStatus)(epicsThreadCreate("timingAsynEpicsUserTask2",
                                            epicsThreadPriorityMedium,
                                            epicsThreadGetStackSize(epicsThreadStackMedium),
                                            (EPICSTHREADFUNC)::userP_timeSync,
                                            this) == NULL);
    if (status) {
      printf("%s:%s: epicsThreadCreate failure\n", driverName, __FUNCTION__);
      return;
    }
  }


}

void timingAsynEpics::registerParamListFromFile(string fileName)
{
  ifstream file(fileName.c_str());
  if(file.fail())
  {
    prnM3("[ERR] registerParamListFromFile open error : %s\r\n", fileName.c_str());
    file.close();
    return;
  } 
  
  RegMap regmap;
  string strToken;
  char str[200];
  char *pch;

  regmap.index = 0;

  while(!file.eof())
  {
    getline(file, strToken);
    if(strToken[0] == '#' || strToken.empty()==true) continue;

    strcpy (str, strToken.c_str());
    memset(&regmap,0, sizeof(RegMap));

    if(!(pch = strtok (str,","))) continue;
    strcpy(regmap.drvname, pch);

    if(!(pch = strtok (NULL,","))) continue;
    regmap.pType = getAsynParamType(pch);

    if(!(pch = strtok (NULL,","))) continue;
    regmap.ipId = strtol(pch,NULL,10);

    if(!(pch = strtok (NULL,","))) continue;
    regmap.ipCnt = strtol(pch,NULL,10);

    if(!(pch = strtok (NULL,","))) continue;
    regmap.accType = strtol(pch,NULL,10);

    if(!(pch = strtok (NULL,","))) continue;
    regmap.address = (unsigned int)strtoll(pch,NULL,10);

    if(!(pch = strtok (NULL,","))) continue;
    regmap.addrType = strtol(pch,NULL,10);

    if(!(pch = strtok (NULL,","))) continue;
    regmap.dataType = strtol(pch,NULL,10);

    if(!(pch = strtok (NULL,","))) continue;
    regmap.dataWidth = strtol(pch,NULL,10);

    if(!(pch = strtok (NULL,","))) continue;
    regmap.dataOffset = strtol(pch,NULL,10);

    regmap.dataSize = 1;

    createParam(regmap.drvname, regmap.pType, &regmap.index);
    regmaptable[regmap.index] = regmapfile[regmap.drvname] = regmap;

    // if(regmap.index == 0 && timingDebug)
    // {
    //   printf("%s\r\n", "===================================================================================================");
    //   printf("index, ipId, drvname           , address   , pType, dataType, dataWidth, dataOffset\r\n");
    //   printf("%s\r\n", "---------------------------------------------------------------------------------------------------");
    // }

    // if(timingDebug)
    //   printf("%5d, %4d, %-18s, 0x%08x, %5d, %8d, %9d, %10d\r\n", regmap.index, regmap.ipId, regmap.drvname, regmap.address, regmap.pType, regmap.dataType, regmap.dataWidth, regmap.dataOffset);
    regmap.index++;
  };  
  // if(timingDebug)
    // printf("%s\r\n", "---------------------------------------------------------------------------------------------------");
  printf("\r\n");

  file.close();
}

#if 0
void timingAsynEpics::cfgInitFromFile(string fileName)
{
  ifstream file(fileName.c_str());
  if(file.fail()){
    prnM3("[ERR] cfgInitFromFile open error : %s\r\n", fileName.c_str());
    file.close();
    return;
  }
  
  string strToken;

  char str[200];
  char index[200];
  char pname[100];
  char pvalue[32];
  char *pch;

  while(!file.eof())
  {
    getline(file, strToken);
    if(strToken[0] == '#' || strToken.empty()==true) continue;
    strcpy (str, strToken.c_str());
    
    if(!(pch = strtok (str,","))) continue;
    sprintf(index, "%s", pch);
    if(!(pch = strtok (NULL,","))) continue;
    sprintf(pname, "%s:%s", sysDevName, pch);
    if(!(pch = strtok (NULL,","))) continue;
    sprintf(pvalue, "%s", pch);
    if(!(pch = strtok (NULL,","))) continue;

    printf("cfgInitFromFile -> %s : %s\n", pname, pvalue);
    db_put(pname, pvalue);
  };  

  file.close();
}
#else

void timingAsynEpics::cfgGetValue(const char* pvname)
{
  char pvnameFull[128];
  char pvalue[64];
  sprintf(pvnameFull, "%s:%s", sysDevName, pvname);
  sprintf(pvalue, "%s", iniFt.GetSection("CFG")->GetValue(pvname, -1).AsString().c_str());
  // printf("cfgInitFromFile -> %s = %s\n", pvnameFull, pvalue);
  db_put(pvnameFull, pvalue);
}

void timingAsynEpics::cfgGetValueToSet(const char* pvname)
{
  char pvnameFull[128];
  sprintf(pvnameFull, "%s:%s", sysDevName, pvname);
  // printf("cfgInitFromFile -> %s = %s\n", pvnameFull, pvalue);
  db_put(pvnameFull, "1");
  db_put(pvnameFull, "0");
}

void timingAsynEpics::cfgInitFromFile(void)
{
  if (!iniFt.Load("/mnt/sdcard/pvCfg.ini")){
		printf("There is no config.ini file\r\n");
	}

  cfgGetValue("EVR1_TIMEDELAY");             
  cfgGetValue("EVR1_EXOUTINVERT_0");         
  cfgGetValue("EVR1_EXOUTINVERT_1");         
  cfgGetValue("EVR1_EXOUTINVERT_2");         
  cfgGetValue("EVR1_EXOUTINVERT_3");         
  cfgGetValue("EVR1_EXOUTINVERT_4");         
  cfgGetValue("EVR1_EXOUTINVERT_5");         
  cfgGetValue("EVR1_EXOUTINVERT_6");         
  cfgGetValue("EVR1_EXOUTINVERT_7");         
  cfgGetValue("EVR1_EXOUTINVERT_8");         
  cfgGetValue("EVR1_EXOUTINVERT_9");         
  cfgGetValue("EVR1_EXOUTINVERT_10");        
  cfgGetValue("EVR1_EXOUTINVERT_11");        
  cfgGetValue("EVR1_EXOUTINVERT_12");        
  cfgGetValue("EVR1_EXOUTINVERT_13");        
  cfgGetValue("EVR1_EXOUTINVERT_14");        
  cfgGetValue("EVR1_EXOUTINVERT_15");        
  cfgGetValue("EVR1_EXOUTINVERT_16");        
  cfgGetValue("EVR1_EXOUTINVERT_17");        
  cfgGetValue("EVR1_EXOUTINVERT_18");        
  cfgGetValue("EVR1_EXOUTINVERT_19");        
  cfgGetValue("EVR1_EXOUTINVERT_20");        
  cfgGetValue("EVR1_EXOUTINVERT_21");        
  cfgGetValue("EVR1_EXOUTINVERT_22");        
  cfgGetValue("EVR1_EXOUTINVERT_23");        
  cfgGetValue("EVR1_EXOUTINVERT_24");        
  cfgGetValue("EVR1_EXOUTINVERT_25");        
  cfgGetValue("EVR1_EXOUTINVERT_26");        
  cfgGetValue("EVR1_EXOUTINVERT_27");        
  cfgGetValue("EVR1_EXOUTINVERT_28");        
  cfgGetValue("EVR1_EXOUTINVERT_29");        
  cfgGetValue("EVR1_EXOUTINVERT_30");        
  cfgGetValue("EVR1_EXOUTINVERT_31");        
  cfgGetValue("EVR1_PDPDELAY_0");            
  cfgGetValue("EVR1_PDPDELAY_1");            
  cfgGetValue("EVR1_PDPDELAY_2");            
  cfgGetValue("EVR1_PDPDELAY_3");            
  cfgGetValue("EVR1_PDPDELAY_4");            
  cfgGetValue("EVR1_PDPDELAY_5");            
  cfgGetValue("EVR1_PDPDELAY_6");            
  cfgGetValue("EVR1_PDPDELAY_7");            
  cfgGetValue("EVR1_PDPDELAY_8");            
  cfgGetValue("EVR1_PDPDELAY_9");            
  cfgGetValue("EVR1_PDPDELAY_10");           
  cfgGetValue("EVR1_PDPDELAY_11");           
  cfgGetValue("EVR1_PDPDELAY_12");           
  cfgGetValue("EVR1_PDPDELAY_13");           
  cfgGetValue("EVR1_PDPDELAY_14");           
  cfgGetValue("EVR1_PDPDELAY_15");           
  cfgGetValue("EVR1_PDPDELAY_16");           
  cfgGetValue("EVR1_PDPDELAY_17");           
  cfgGetValue("EVR1_PDPDELAY_18");           
  cfgGetValue("EVR1_PDPDELAY_19");           
  cfgGetValue("EVR1_PDPDELAY_20");           
  cfgGetValue("EVR1_PDPDELAY_21");           
  cfgGetValue("EVR1_PDPDELAY_22");           
  cfgGetValue("EVR1_PDPDELAY_23");           
  cfgGetValue("EVR1_PDPDELAY_24");           
  cfgGetValue("EVR1_PDPDELAY_25");           
  cfgGetValue("EVR1_PDPDELAY_26");           
  cfgGetValue("EVR1_PDPDELAY_27");           
  cfgGetValue("EVR1_PDPDELAY_28");           
  cfgGetValue("EVR1_PDPDELAY_29");           
  cfgGetValue("EVR1_PDPDELAY_30");           
  cfgGetValue("EVR1_PDPDELAY_31");           
  cfgGetValue("EVR1_PDPWIDTH_0");            
  cfgGetValue("EVR1_PDPWIDTH_1");            
  cfgGetValue("EVR1_PDPWIDTH_2");            
  cfgGetValue("EVR1_PDPWIDTH_3");            
  cfgGetValue("EVR1_PDPWIDTH_4");            
  cfgGetValue("EVR1_PDPWIDTH_5");            
  cfgGetValue("EVR1_PDPWIDTH_6");            
  cfgGetValue("EVR1_PDPWIDTH_7");            
  cfgGetValue("EVR1_PDPWIDTH_8");            
  cfgGetValue("EVR1_PDPWIDTH_9");            
  cfgGetValue("EVR1_PDPWIDTH_10");           
  cfgGetValue("EVR1_PDPWIDTH_11");           
  cfgGetValue("EVR1_PDPWIDTH_12");           
  cfgGetValue("EVR1_PDPWIDTH_13");           
  cfgGetValue("EVR1_PDPWIDTH_14");           
  cfgGetValue("EVR1_PDPWIDTH_15");           
  cfgGetValue("EVR1_PDPWIDTH_16");           
  cfgGetValue("EVR1_PDPWIDTH_17");           
  cfgGetValue("EVR1_PDPWIDTH_18");           
  cfgGetValue("EVR1_PDPWIDTH_19");           
  cfgGetValue("EVR1_PDPWIDTH_20");           
  cfgGetValue("EVR1_PDPWIDTH_21");           
  cfgGetValue("EVR1_PDPWIDTH_22");           
  cfgGetValue("EVR1_PDPWIDTH_23");           
  cfgGetValue("EVR1_PDPWIDTH_24");           
  cfgGetValue("EVR1_PDPWIDTH_25");           
  cfgGetValue("EVR1_PDPWIDTH_26");           
  cfgGetValue("EVR1_PDPWIDTH_27");           
  cfgGetValue("EVR1_PDPWIDTH_28");           
  cfgGetValue("EVR1_PDPWIDTH_29");           
  cfgGetValue("EVR1_PDPWIDTH_30");           
  cfgGetValue("EVR1_PDPWIDTH_31");           
  cfgGetValue("EVR1_PDPTRGSEL_0");           
  cfgGetValue("EVR1_PDPTRGSEL_1");           
  cfgGetValue("EVR1_PDPTRGSEL_2");           
  cfgGetValue("EVR1_PDPTRGSEL_3");           
  cfgGetValue("EVR1_PDPTRGSEL_4");           
  cfgGetValue("EVR1_PDPTRGSEL_5");           
  cfgGetValue("EVR1_PDPTRGSEL_6");           
  cfgGetValue("EVR1_PDPTRGSEL_7");           
  cfgGetValue("EVR1_PDPTRGSEL_8");           
  cfgGetValue("EVR1_PDPTRGSEL_9");           
  cfgGetValue("EVR1_PDPTRGSEL_10");          
  cfgGetValue("EVR1_PDPTRGSEL_11");          
  cfgGetValue("EVR1_PDPTRGSEL_12");          
  cfgGetValue("EVR1_PDPTRGSEL_13");          
  cfgGetValue("EVR1_PDPTRGSEL_14");          
  cfgGetValue("EVR1_PDPTRGSEL_15");          
  cfgGetValue("EVR1_PDPTRGSEL_16");          
  cfgGetValue("EVR1_PDPTRGSEL_17");          
  cfgGetValue("EVR1_PDPTRGSEL_18");          
  cfgGetValue("EVR1_PDPTRGSEL_19");          
  cfgGetValue("EVR1_PDPTRGSEL_20");          
  cfgGetValue("EVR1_PDPTRGSEL_21");          
  cfgGetValue("EVR1_PDPTRGSEL_22");          
  cfgGetValue("EVR1_PDPTRGSEL_23");          
  cfgGetValue("EVR1_PDPTRGSEL_24");          
  cfgGetValue("EVR1_PDPTRGSEL_25");          
  cfgGetValue("EVR1_PDPTRGSEL_26");          
  cfgGetValue("EVR1_PDPTRGSEL_27");          
  cfgGetValue("EVR1_PDPTRGSEL_28");          
  cfgGetValue("EVR1_PDPTRGSEL_29");          
  cfgGetValue("EVR1_PDPTRGSEL_30");          
  cfgGetValue("EVR1_PDPTRGSEL_31");          
  cfgGetValue("EVR1_PORTFREQ_0");           
  cfgGetValue("EVR1_PORTFREQ_1");           
  cfgGetValue("EVR1_PORTFREQ_2");           
  cfgGetValue("EVR1_PORTFREQ_3");           
  cfgGetValue("EVR1_PORTFREQ_4");           
  cfgGetValue("EVR1_PORTFREQ_5");           
  cfgGetValue("EVR1_PORTFREQ_6");           
  cfgGetValue("EVR1_PORTFREQ_7");           
  cfgGetValue("EVR1_PORTFREQ_8");           
  cfgGetValue("EVR1_PORTFREQ_9");           
  cfgGetValue("EVR1_PORTFREQ_10");          
  cfgGetValue("EVR1_PORTFREQ_11");          
  cfgGetValue("EVR1_PORTFREQ_12");          
  cfgGetValue("EVR1_PORTFREQ_13");          
  cfgGetValue("EVR1_PORTFREQ_14");          
  cfgGetValue("EVR1_PORTFREQ_15");          
  cfgGetValue("EVR1_PORTFREQ_16");          
  cfgGetValue("EVR1_PORTFREQ_17");          
  cfgGetValue("EVR1_PORTFREQ_18");          
  cfgGetValue("EVR1_PORTFREQ_19");          
  cfgGetValue("EVR1_PORTFREQ_20");          
  cfgGetValue("EVR1_PORTFREQ_21");          
  cfgGetValue("EVR1_PORTFREQ_22");          
  cfgGetValue("EVR1_PORTFREQ_23");          
  cfgGetValue("EVR1_PORTFREQ_24");          
  cfgGetValue("EVR1_PORTFREQ_25");          
  cfgGetValue("EVR1_PORTFREQ_26");          
  cfgGetValue("EVR1_PORTFREQ_27");          
  cfgGetValue("EVR1_PORTFREQ_28");          
  cfgGetValue("EVR1_PORTFREQ_29");          
  cfgGetValue("EVR1_PORTFREQ_30");          
  cfgGetValue("EVR1_PORTFREQ_31");          
  cfgGetValue("EVR1_RX_DBUSSEL_0");          
  cfgGetValue("EVR1_RX_DBUSSEL_1");          
  cfgGetValue("EVR1_RX_DBUSSEL_2");          
  cfgGetValue("EVR1_RX_DBUSSEL_3");          
  cfgGetValue("EVR1_RX_DBUSSEL_4");          
  cfgGetValue("EVR1_RX_DBUSSEL_5");          
  cfgGetValue("EVR1_RX_DBUSSEL_6");          
  cfgGetValue("EVR1_RX_DBUSSEL_7");          
  cfgGetValue("EVR1_EXTOUTSWAP");            
  cfgGetValue("EVR1_EXTOUTSEL_1PPS");        
  cfgGetValue("EVR1_EXTOUTSEL_XPS");         
  cfgGetValue("EVR1_LOGFIFO_ENABLE");        
  cfgGetValue("EVR1_EXOUTENABLE");           
  cfgGetValue("EVR1_RXENABLE");              
  cfgGetValue("EVR0_RXENABLE");              
  cfgGetValue("EVG1_MXC14_PRESCALERREG_0");  
  cfgGetValue("EVG1_MXC14_PRESCALERREG_1");  
  cfgGetValue("EVG1_MXC14_PRESCALERREG_2");  
  cfgGetValue("EVG1_MXC14_PRESCALERREG_3");  
  cfgGetValue("EVG1_MXC14_PRESCALERREG_4");  
  cfgGetValue("EVG1_MXC14_PRESCALERREG_5");  
  cfgGetValue("EVG1_MXC14_PRESCALERREG_6");  
  cfgGetValue("EVG1_MXC14_PRESCALERREG_7");  
  cfgGetValue("EVG1_MXC14_PRESCALERREG_8");  
  cfgGetValue("EVG1_MXC14_PRESCALERREG_9");  
  cfgGetValue("EVG1_MXC14_PRESCALERREG_10"); 
  cfgGetValue("EVG1_MXC14_PRESCALERREG_11"); 
  cfgGetValue("EVG1_MXC14_PRESCALERREG_12"); 
  cfgGetValue("EVG1_MXC14_PRESCALERREG_13"); 
  cfgGetValue("EVG1_TX_DBUS_SEL_0");         
  cfgGetValue("EVG1_TX_DBUS_SEL_1");         
  cfgGetValue("EVG1_TX_DBUS_SEL_2");         
  cfgGetValue("EVG1_TX_DBUS_SEL_3");         
  cfgGetValue("EVG1_TX_DBUS_SEL_4");         
  cfgGetValue("EVG1_TX_DBUS_SEL_5");         
  cfgGetValue("EVG1_TX_DBUS_SEL_6");         
  cfgGetValue("EVG1_TX_DBUS_SEL_7");         
  cfgGetValue("EVG1_SEQ_REPEATREG_0");       
  cfgGetValue("EVG1_SEQ_REPEATREG_1");       
  cfgGetValue("EVG1_SEQTRGSEL_0");           
  cfgGetValue("EVG1_SEQTRGSEL_1");           
  cfgGetValue("EVG1_SEQTRGMSKA0_0");         
  cfgGetValue("EVG1_SEQTRGMSKA0_1");         
  cfgGetValue("EVG1_SEQTRGMSKA0_2");         
  cfgGetValue("EVG1_SEQTRGMSKA0_3");         
  cfgGetValue("EVG1_SEQTRGMSKA0_4");         
  cfgGetValue("EVG1_SEQTRGMSKA0_5");         
  cfgGetValue("EVG1_SEQTRGMSKA0_6");         
  cfgGetValue("EVG1_SEQTRGMSKA0_7");         
  cfgGetValue("EVG1_SEQTRGMSKA0_8");         
  cfgGetValue("EVG1_SEQTRGMSKA0_15");        
  cfgGetValue("EVG1_SEQTRGMSKA0_16");        
  cfgGetValue("EVG1_SEQTRGMSKA0_31");        
  cfgGetValue("EVG1_SEQTRGMSKA1_0");         
  cfgGetValue("EVG1_SEQTRGMSKA1_1");         
  cfgGetValue("EVG1_SEQTRGMSKA1_2");         
  cfgGetValue("EVG1_SEQTRGMSKA1_3");         
  cfgGetValue("EVG1_SEQTRGMSKA1_4");         
  cfgGetValue("EVG1_SEQTRGMSKA1_5");         
  cfgGetValue("EVG1_SEQTRGMSKA1_6");         
  cfgGetValue("EVG1_SEQTRGMSKA1_7");         
  cfgGetValue("EVG1_SEQTRGMSKA1_8");         
  cfgGetValue("EVG1_SEQTRGMSKA1_15");        
  cfgGetValue("EVG1_SEQTRGMSKA1_16");        
  cfgGetValue("EVG1_SEQTRGMSKA1_31");        
  cfgGetValue("EVG1_SEQTRGMSKB0_0");         
  cfgGetValue("EVG1_SEQTRGMSKB0_1");         
  cfgGetValue("EVG1_SEQTRGMSKB0_2");         
  cfgGetValue("EVG1_SEQTRGMSKB0_3");         
  cfgGetValue("EVG1_SEQTRGMSKB0_4");         
  cfgGetValue("EVG1_SEQTRGMSKB0_5");         
  cfgGetValue("EVG1_SEQTRGMSKB0_6");         
  cfgGetValue("EVG1_SEQTRGMSKB0_7");         
  cfgGetValue("EVG1_SEQTRGMSKB0_8");         
  cfgGetValue("EVG1_SEQTRGMSKB0_15");        
  cfgGetValue("EVG1_SEQTRGMSKB0_16");        
  cfgGetValue("EVG1_SEQTRGMSKB0_31");        
  cfgGetValue("EVG1_SEQTRGMSKB1_0");         
  cfgGetValue("EVG1_SEQTRGMSKB1_1");         
  cfgGetValue("EVG1_SEQTRGMSKB1_2");         
  cfgGetValue("EVG1_SEQTRGMSKB1_3");         
  cfgGetValue("EVG1_SEQTRGMSKB1_4");         
  cfgGetValue("EVG1_SEQTRGMSKB1_5");         
  cfgGetValue("EVG1_SEQTRGMSKB1_6");         
  cfgGetValue("EVG1_SEQTRGMSKB1_7");         
  cfgGetValue("EVG1_SEQTRGMSKB1_15");        
  cfgGetValue("EVG1_SEQTRGMSKB1_16");        
  cfgGetValue("EVG1_SEQTRGMSKB1_31");        
  cfgGetValue("EV_CPS_CTL_GT0");             
  cfgGetValue("EV_CPS_CTL_GT1");             
  cfgGetValue("EV_CPS_CTL_FMC1");            
  cfgGetValue("EV_CPS_CTL_FMC2");            
  cfgGetValue("EV_CPS_CTL_SL");              
  cfgGetValue("EV_CPS_CTL_SH");              
  cfgGetValue("EV_CPS_CTL_CTL");             
  cfgGetValue("EV_CPS_SL_CTRL");             
  cfgGetValue("EV_CPS_SL_SH");               
  cfgGetValue("EV_CPS_SL_RSL");              
  cfgGetValue("EV_CPS_SL_RSH");              
  cfgGetValue("EV_CPS_SL_00");               
  cfgGetValue("EV_CPS_SL_01");               
  cfgGetValue("EV_CPS_SL_02");               
  cfgGetValue("EV_CPS_SL_03");               
  cfgGetValue("EV_CPS_SL_04");               
  cfgGetValue("EV_CPS_SL_05");               
  cfgGetValue("EV_CPS_SL_06");               
  cfgGetValue("EV_CPS_SL_07");               
  cfgGetValue("EV_CPS_SL_08");               
  cfgGetValue("EV_CPS_SL_09");               
  cfgGetValue("EV_CPS_SL_10");               
  cfgGetValue("EV_CPS_SL_11");               
  cfgGetValue("EV_CPS_SH_CTRL");             
  cfgGetValue("EV_CPS_SH_SL");               
  cfgGetValue("EV_CPS_SH_RSH");              
  cfgGetValue("EV_CPS_SH_RSL");              
  cfgGetValue("EV_CPS_SH_00");               
  cfgGetValue("EV_CPS_SH_01");               
  cfgGetValue("EV_CPS_SH_02");               
  cfgGetValue("EV_CPS_SH_03");               
  cfgGetValue("EV_CPS_SH_04");               
  cfgGetValue("EV_CPS_SH_05");               
  cfgGetValue("EV_CPS_SH_06");               
  cfgGetValue("EV_CPS_SH_07");               
  cfgGetValue("EV_CPS_SH_08");               
  cfgGetValue("EV_CPS_SH_09");               
  cfgGetValue("EV_CPS_SH_10");               
  cfgGetValue("EV_CPS_SH_11");               
  cfgGetValue("EVG1_TXENABLE_DBUS");         
  cfgGetValue("EVG1_TXENABLE_MEVCODEA");     
  cfgGetValue("EVG1_SEQMODE_0");             
  cfgGetValue("EVG1_SEQMODE_1");             
  cfgGetValue("EVG1_SEQTRG_EN_0");           
  cfgGetValue("EVG1_SEQTRG_EN_1");           
  cfgGetValue("EVG1_TXENABLE_EVCODEA");      
  cfgGetValue("EVG1_TXENABLE_EVCODEB");      
  cfgGetValue("EVG1_MXC14ENABLE");           

  cfgGetValueToSet("EV_CPS_SET");
  cfgGetValueToSet("EVR1_SETCTRLREG");
  cfgGetValueToSet("SYS_SAVE_CFG");
}
#endif


void timingAsynEpics::cfgSetValue(const char* pvname)
{
  char pvnameFull[128];
  char pvalue[64];
  sprintf(pvnameFull, "%s:%s", sysDevName, pvname);
  db_get(pvnameFull, pvalue);
  iniFt.GetSection("CFG")->SetValue(pvname, pvalue);
}

void timingAsynEpics::cfgSaveToFile(void)
{
  cfgSetValue("EVR1_TIMEDELAY");             
  cfgSetValue("EVR1_EXOUTINVERT_0");         
  cfgSetValue("EVR1_EXOUTINVERT_1");         
  cfgSetValue("EVR1_EXOUTINVERT_2");         
  cfgSetValue("EVR1_EXOUTINVERT_3");         
  cfgSetValue("EVR1_EXOUTINVERT_4");         
  cfgSetValue("EVR1_EXOUTINVERT_5");         
  cfgSetValue("EVR1_EXOUTINVERT_6");         
  cfgSetValue("EVR1_EXOUTINVERT_7");         
  cfgSetValue("EVR1_EXOUTINVERT_8");         
  cfgSetValue("EVR1_EXOUTINVERT_9");         
  cfgSetValue("EVR1_EXOUTINVERT_10");        
  cfgSetValue("EVR1_EXOUTINVERT_11");        
  cfgSetValue("EVR1_EXOUTINVERT_12");        
  cfgSetValue("EVR1_EXOUTINVERT_13");        
  cfgSetValue("EVR1_EXOUTINVERT_14");        
  cfgSetValue("EVR1_EXOUTINVERT_15");        
  cfgSetValue("EVR1_EXOUTINVERT_16");        
  cfgSetValue("EVR1_EXOUTINVERT_17");        
  cfgSetValue("EVR1_EXOUTINVERT_18");        
  cfgSetValue("EVR1_EXOUTINVERT_19");        
  cfgSetValue("EVR1_EXOUTINVERT_20");        
  cfgSetValue("EVR1_EXOUTINVERT_21");        
  cfgSetValue("EVR1_EXOUTINVERT_22");        
  cfgSetValue("EVR1_EXOUTINVERT_23");        
  cfgSetValue("EVR1_EXOUTINVERT_24");        
  cfgSetValue("EVR1_EXOUTINVERT_25");        
  cfgSetValue("EVR1_EXOUTINVERT_26");        
  cfgSetValue("EVR1_EXOUTINVERT_27");        
  cfgSetValue("EVR1_EXOUTINVERT_28");        
  cfgSetValue("EVR1_EXOUTINVERT_29");        
  cfgSetValue("EVR1_EXOUTINVERT_30");        
  cfgSetValue("EVR1_EXOUTINVERT_31");        
  cfgSetValue("EVR1_PDPDELAY_0");            
  cfgSetValue("EVR1_PDPDELAY_1");            
  cfgSetValue("EVR1_PDPDELAY_2");            
  cfgSetValue("EVR1_PDPDELAY_3");            
  cfgSetValue("EVR1_PDPDELAY_4");            
  cfgSetValue("EVR1_PDPDELAY_5");            
  cfgSetValue("EVR1_PDPDELAY_6");            
  cfgSetValue("EVR1_PDPDELAY_7");            
  cfgSetValue("EVR1_PDPDELAY_8");            
  cfgSetValue("EVR1_PDPDELAY_9");            
  cfgSetValue("EVR1_PDPDELAY_10");           
  cfgSetValue("EVR1_PDPDELAY_11");           
  cfgSetValue("EVR1_PDPDELAY_12");           
  cfgSetValue("EVR1_PDPDELAY_13");           
  cfgSetValue("EVR1_PDPDELAY_14");           
  cfgSetValue("EVR1_PDPDELAY_15");           
  cfgSetValue("EVR1_PDPDELAY_16");           
  cfgSetValue("EVR1_PDPDELAY_17");           
  cfgSetValue("EVR1_PDPDELAY_18");           
  cfgSetValue("EVR1_PDPDELAY_19");           
  cfgSetValue("EVR1_PDPDELAY_20");           
  cfgSetValue("EVR1_PDPDELAY_21");           
  cfgSetValue("EVR1_PDPDELAY_22");           
  cfgSetValue("EVR1_PDPDELAY_23");           
  cfgSetValue("EVR1_PDPDELAY_24");           
  cfgSetValue("EVR1_PDPDELAY_25");           
  cfgSetValue("EVR1_PDPDELAY_26");           
  cfgSetValue("EVR1_PDPDELAY_27");           
  cfgSetValue("EVR1_PDPDELAY_28");           
  cfgSetValue("EVR1_PDPDELAY_29");           
  cfgSetValue("EVR1_PDPDELAY_30");           
  cfgSetValue("EVR1_PDPDELAY_31");           
  cfgSetValue("EVR1_PDPWIDTH_0");            
  cfgSetValue("EVR1_PDPWIDTH_1");            
  cfgSetValue("EVR1_PDPWIDTH_2");            
  cfgSetValue("EVR1_PDPWIDTH_3");            
  cfgSetValue("EVR1_PDPWIDTH_4");            
  cfgSetValue("EVR1_PDPWIDTH_5");            
  cfgSetValue("EVR1_PDPWIDTH_6");            
  cfgSetValue("EVR1_PDPWIDTH_7");            
  cfgSetValue("EVR1_PDPWIDTH_8");            
  cfgSetValue("EVR1_PDPWIDTH_9");            
  cfgSetValue("EVR1_PDPWIDTH_10");           
  cfgSetValue("EVR1_PDPWIDTH_11");           
  cfgSetValue("EVR1_PDPWIDTH_12");           
  cfgSetValue("EVR1_PDPWIDTH_13");           
  cfgSetValue("EVR1_PDPWIDTH_14");           
  cfgSetValue("EVR1_PDPWIDTH_15");           
  cfgSetValue("EVR1_PDPWIDTH_16");           
  cfgSetValue("EVR1_PDPWIDTH_17");           
  cfgSetValue("EVR1_PDPWIDTH_18");           
  cfgSetValue("EVR1_PDPWIDTH_19");           
  cfgSetValue("EVR1_PDPWIDTH_20");           
  cfgSetValue("EVR1_PDPWIDTH_21");           
  cfgSetValue("EVR1_PDPWIDTH_22");           
  cfgSetValue("EVR1_PDPWIDTH_23");           
  cfgSetValue("EVR1_PDPWIDTH_24");           
  cfgSetValue("EVR1_PDPWIDTH_25");           
  cfgSetValue("EVR1_PDPWIDTH_26");           
  cfgSetValue("EVR1_PDPWIDTH_27");           
  cfgSetValue("EVR1_PDPWIDTH_28");           
  cfgSetValue("EVR1_PDPWIDTH_29");           
  cfgSetValue("EVR1_PDPWIDTH_30");           
  cfgSetValue("EVR1_PDPWIDTH_31");           
  cfgSetValue("EVR1_PDPTRGSEL_0");           
  cfgSetValue("EVR1_PDPTRGSEL_1");           
  cfgSetValue("EVR1_PDPTRGSEL_2");           
  cfgSetValue("EVR1_PDPTRGSEL_3");           
  cfgSetValue("EVR1_PDPTRGSEL_4");           
  cfgSetValue("EVR1_PDPTRGSEL_5");           
  cfgSetValue("EVR1_PDPTRGSEL_6");           
  cfgSetValue("EVR1_PDPTRGSEL_7");           
  cfgSetValue("EVR1_PDPTRGSEL_8");           
  cfgSetValue("EVR1_PDPTRGSEL_9");           
  cfgSetValue("EVR1_PDPTRGSEL_10");          
  cfgSetValue("EVR1_PDPTRGSEL_11");          
  cfgSetValue("EVR1_PDPTRGSEL_12");          
  cfgSetValue("EVR1_PDPTRGSEL_13");          
  cfgSetValue("EVR1_PDPTRGSEL_14");          
  cfgSetValue("EVR1_PDPTRGSEL_15");          
  cfgSetValue("EVR1_PDPTRGSEL_16");          
  cfgSetValue("EVR1_PDPTRGSEL_17");          
  cfgSetValue("EVR1_PDPTRGSEL_18");          
  cfgSetValue("EVR1_PDPTRGSEL_19");          
  cfgSetValue("EVR1_PDPTRGSEL_20");          
  cfgSetValue("EVR1_PDPTRGSEL_21");          
  cfgSetValue("EVR1_PDPTRGSEL_22");          
  cfgSetValue("EVR1_PDPTRGSEL_23");          
  cfgSetValue("EVR1_PDPTRGSEL_24");          
  cfgSetValue("EVR1_PDPTRGSEL_25");          
  cfgSetValue("EVR1_PDPTRGSEL_26");          
  cfgSetValue("EVR1_PDPTRGSEL_27");          
  cfgSetValue("EVR1_PDPTRGSEL_28");          
  cfgSetValue("EVR1_PDPTRGSEL_29");          
  cfgSetValue("EVR1_PDPTRGSEL_30");          
  cfgSetValue("EVR1_PDPTRGSEL_31");          

  cfgSetValue("EVR1_PORTFREQ_0");           
  cfgSetValue("EVR1_PORTFREQ_1");           
  cfgSetValue("EVR1_PORTFREQ_2");           
  cfgSetValue("EVR1_PORTFREQ_3");           
  cfgSetValue("EVR1_PORTFREQ_4");           
  cfgSetValue("EVR1_PORTFREQ_5");           
  cfgSetValue("EVR1_PORTFREQ_6");           
  cfgSetValue("EVR1_PORTFREQ_7");           
  cfgSetValue("EVR1_PORTFREQ_8");           
  cfgSetValue("EVR1_PORTFREQ_9");           
  cfgSetValue("EVR1_PORTFREQ_10");          
  cfgSetValue("EVR1_PORTFREQ_11");          
  cfgSetValue("EVR1_PORTFREQ_12");          
  cfgSetValue("EVR1_PORTFREQ_13");          
  cfgSetValue("EVR1_PORTFREQ_14");          
  cfgSetValue("EVR1_PORTFREQ_15");          
  cfgSetValue("EVR1_PORTFREQ_16");          
  cfgSetValue("EVR1_PORTFREQ_17");          
  cfgSetValue("EVR1_PORTFREQ_18");          
  cfgSetValue("EVR1_PORTFREQ_19");          
  cfgSetValue("EVR1_PORTFREQ_20");          
  cfgSetValue("EVR1_PORTFREQ_21");          
  cfgSetValue("EVR1_PORTFREQ_22");          
  cfgSetValue("EVR1_PORTFREQ_23");          
  cfgSetValue("EVR1_PORTFREQ_24");          
  cfgSetValue("EVR1_PORTFREQ_25");          
  cfgSetValue("EVR1_PORTFREQ_26");          
  cfgSetValue("EVR1_PORTFREQ_27");          
  cfgSetValue("EVR1_PORTFREQ_28");          
  cfgSetValue("EVR1_PORTFREQ_29");          
  cfgSetValue("EVR1_PORTFREQ_30");          
  cfgSetValue("EVR1_PORTFREQ_31");          

  cfgSetValue("EVR1_RX_DBUSSEL_0");          
  cfgSetValue("EVR1_RX_DBUSSEL_1");          
  cfgSetValue("EVR1_RX_DBUSSEL_2");          
  cfgSetValue("EVR1_RX_DBUSSEL_3");          
  cfgSetValue("EVR1_RX_DBUSSEL_4");          
  cfgSetValue("EVR1_RX_DBUSSEL_5");          
  cfgSetValue("EVR1_RX_DBUSSEL_6");          
  cfgSetValue("EVR1_RX_DBUSSEL_7");          
  cfgSetValue("EVR1_EXTOUTSWAP");            
  cfgSetValue("EVR1_EXTOUTSEL_1PPS");        
  cfgSetValue("EVR1_EXTOUTSEL_XPS");         
  cfgSetValue("EVR1_LOGFIFO_ENABLE");        
  cfgSetValue("EVR1_EXOUTENABLE");           
  cfgSetValue("EVR1_RXENABLE");              
  cfgSetValue("EVR0_RXENABLE");              
  cfgSetValue("EVG1_MXC14_PRESCALERREG_0");  
  cfgSetValue("EVG1_MXC14_PRESCALERREG_1");  
  cfgSetValue("EVG1_MXC14_PRESCALERREG_2");  
  cfgSetValue("EVG1_MXC14_PRESCALERREG_3");  
  cfgSetValue("EVG1_MXC14_PRESCALERREG_4");  
  cfgSetValue("EVG1_MXC14_PRESCALERREG_5");  
  cfgSetValue("EVG1_MXC14_PRESCALERREG_6");  
  cfgSetValue("EVG1_MXC14_PRESCALERREG_7");  
  cfgSetValue("EVG1_MXC14_PRESCALERREG_8");  
  cfgSetValue("EVG1_MXC14_PRESCALERREG_9");  
  cfgSetValue("EVG1_MXC14_PRESCALERREG_10"); 
  cfgSetValue("EVG1_MXC14_PRESCALERREG_11"); 
  cfgSetValue("EVG1_MXC14_PRESCALERREG_12"); 
  cfgSetValue("EVG1_MXC14_PRESCALERREG_13"); 
  cfgSetValue("EVG1_TX_DBUS_SEL_0");         
  cfgSetValue("EVG1_TX_DBUS_SEL_1");         
  cfgSetValue("EVG1_TX_DBUS_SEL_2");         
  cfgSetValue("EVG1_TX_DBUS_SEL_3");         
  cfgSetValue("EVG1_TX_DBUS_SEL_4");         
  cfgSetValue("EVG1_TX_DBUS_SEL_5");         
  cfgSetValue("EVG1_TX_DBUS_SEL_6");         
  cfgSetValue("EVG1_TX_DBUS_SEL_7");         
  cfgSetValue("EVG1_SEQ_REPEATREG_0");       
  cfgSetValue("EVG1_SEQ_REPEATREG_1");       
  cfgSetValue("EVG1_SEQTRGSEL_0");           
  cfgSetValue("EVG1_SEQTRGSEL_1");           
  cfgSetValue("EVG1_SEQTRGMSKA0_0");         
  cfgSetValue("EVG1_SEQTRGMSKA0_1");         
  cfgSetValue("EVG1_SEQTRGMSKA0_2");         
  cfgSetValue("EVG1_SEQTRGMSKA0_3");         
  cfgSetValue("EVG1_SEQTRGMSKA0_4");         
  cfgSetValue("EVG1_SEQTRGMSKA0_5");         
  cfgSetValue("EVG1_SEQTRGMSKA0_6");         
  cfgSetValue("EVG1_SEQTRGMSKA0_7");         
  cfgSetValue("EVG1_SEQTRGMSKA0_8");         
  cfgSetValue("EVG1_SEQTRGMSKA0_15");        
  cfgSetValue("EVG1_SEQTRGMSKA0_16");        
  cfgSetValue("EVG1_SEQTRGMSKA0_31");        
  cfgSetValue("EVG1_SEQTRGMSKA1_0");         
  cfgSetValue("EVG1_SEQTRGMSKA1_1");         
  cfgSetValue("EVG1_SEQTRGMSKA1_2");         
  cfgSetValue("EVG1_SEQTRGMSKA1_3");         
  cfgSetValue("EVG1_SEQTRGMSKA1_4");         
  cfgSetValue("EVG1_SEQTRGMSKA1_5");         
  cfgSetValue("EVG1_SEQTRGMSKA1_6");         
  cfgSetValue("EVG1_SEQTRGMSKA1_7");         
  cfgSetValue("EVG1_SEQTRGMSKA1_8");         
  cfgSetValue("EVG1_SEQTRGMSKA1_15");        
  cfgSetValue("EVG1_SEQTRGMSKA1_16");        
  cfgSetValue("EVG1_SEQTRGMSKA1_31");        
  cfgSetValue("EVG1_SEQTRGMSKB0_0");         
  cfgSetValue("EVG1_SEQTRGMSKB0_1");         
  cfgSetValue("EVG1_SEQTRGMSKB0_2");         
  cfgSetValue("EVG1_SEQTRGMSKB0_3");         
  cfgSetValue("EVG1_SEQTRGMSKB0_4");         
  cfgSetValue("EVG1_SEQTRGMSKB0_5");         
  cfgSetValue("EVG1_SEQTRGMSKB0_6");         
  cfgSetValue("EVG1_SEQTRGMSKB0_7");         
  cfgSetValue("EVG1_SEQTRGMSKB0_8");         
  cfgSetValue("EVG1_SEQTRGMSKB0_15");        
  cfgSetValue("EVG1_SEQTRGMSKB0_16");        
  cfgSetValue("EVG1_SEQTRGMSKB0_31");        
  cfgSetValue("EVG1_SEQTRGMSKB1_0");         
  cfgSetValue("EVG1_SEQTRGMSKB1_1");         
  cfgSetValue("EVG1_SEQTRGMSKB1_2");         
  cfgSetValue("EVG1_SEQTRGMSKB1_3");         
  cfgSetValue("EVG1_SEQTRGMSKB1_4");         
  cfgSetValue("EVG1_SEQTRGMSKB1_5");         
  cfgSetValue("EVG1_SEQTRGMSKB1_6");         
  cfgSetValue("EVG1_SEQTRGMSKB1_7");         
  cfgSetValue("EVG1_SEQTRGMSKB1_15");        
  cfgSetValue("EVG1_SEQTRGMSKB1_16");        
  cfgSetValue("EVG1_SEQTRGMSKB1_31");        
  cfgSetValue("EV_CPS_CTL_GT0");             
  cfgSetValue("EV_CPS_CTL_GT1");             
  cfgSetValue("EV_CPS_CTL_FMC1");            
  cfgSetValue("EV_CPS_CTL_FMC2");            
  cfgSetValue("EV_CPS_CTL_SL");              
  cfgSetValue("EV_CPS_CTL_SH");              
  cfgSetValue("EV_CPS_CTL_CTL");             
  cfgSetValue("EV_CPS_SL_CTRL");             
  cfgSetValue("EV_CPS_SL_SH");               
  cfgSetValue("EV_CPS_SL_RSL");              
  cfgSetValue("EV_CPS_SL_RSH");              
  cfgSetValue("EV_CPS_SL_00");               
  cfgSetValue("EV_CPS_SL_01");               
  cfgSetValue("EV_CPS_SL_02");               
  cfgSetValue("EV_CPS_SL_03");               
  cfgSetValue("EV_CPS_SL_04");               
  cfgSetValue("EV_CPS_SL_05");               
  cfgSetValue("EV_CPS_SL_06");               
  cfgSetValue("EV_CPS_SL_07");               
  cfgSetValue("EV_CPS_SL_08");               
  cfgSetValue("EV_CPS_SL_09");               
  cfgSetValue("EV_CPS_SL_10");               
  cfgSetValue("EV_CPS_SL_11");               
  cfgSetValue("EV_CPS_SH_CTRL");             
  cfgSetValue("EV_CPS_SH_SL");               
  cfgSetValue("EV_CPS_SH_RSH");              
  cfgSetValue("EV_CPS_SH_RSL");              
  cfgSetValue("EV_CPS_SH_00");               
  cfgSetValue("EV_CPS_SH_01");               
  cfgSetValue("EV_CPS_SH_02");               
  cfgSetValue("EV_CPS_SH_03");               
  cfgSetValue("EV_CPS_SH_04");               
  cfgSetValue("EV_CPS_SH_05");               
  cfgSetValue("EV_CPS_SH_06");               
  cfgSetValue("EV_CPS_SH_07");               
  cfgSetValue("EV_CPS_SH_08");               
  cfgSetValue("EV_CPS_SH_09");               
  cfgSetValue("EV_CPS_SH_10");               
  cfgSetValue("EV_CPS_SH_11");               
  cfgSetValue("EVG1_TXENABLE_DBUS");         
  cfgSetValue("EVG1_TXENABLE_MEVCODEA");     
  cfgSetValue("EVG1_SEQMODE_0");             
  cfgSetValue("EVG1_SEQMODE_1");             
  cfgSetValue("EVG1_SEQTRG_EN_0");           
  cfgSetValue("EVG1_SEQTRG_EN_1");           
  cfgSetValue("EVG1_TXENABLE_EVCODEA");      
  cfgSetValue("EVG1_TXENABLE_EVCODEB");      
  cfgSetValue("EVG1_MXC14ENABLE");           

  iniFt.Save("/mnt/sdcard/pvCfg.ini");
}


void timingAsynEpics::waveformInitFromFile(const char *pvname)
{
  string fName;
  fName = iValFileName;
  fName += pvname;
  fName += ".init";
  
  ifstream file(fName.c_str());
  if(file.fail()){
    prnM3("[ERR] seqInitFromFile open error : %s\r\n", fName.c_str());
    file.close();
    return;
  }

  string strToken;
  char str[200];
  char pname[128];
  char *pch;
  int index=0;

  long rtVal;
  // unsigned long long tmpVal;
	DBADDR pdbAddr;

  sprintf(pname,"%s:%s",sysDevName,pvname);

	rtVal=dbNameToAddr(pname, &pdbAddr);


  //2020.02.12 코드 수정 필요함.. pv value type 에 따라 달리 처리 해줘야함. by laykim
  long *pfieldLink = (long *)pdbAddr.pfield;
  // switch(pdbAddr.field_type)
  // {
  //   case DBF_CHAR   : char           *pfieldLink = (char           *)pdbAddr.pfield;    break;
  //   case DBF_UCHAR  : unsigned char  *pfieldLink = (unsigned char  *)pdbAddr.pfield;    break;
  //   case DBF_SHORT  : short          *pfieldLink = (short          *)pdbAddr.pfield;    break;
  //   case DBF_USHORT : unsigned short *pfieldLink = (unsigned short *)pdbAddr.pfield;    break;
  //   case DBF_LONG   : long           *pfieldLink = (long           *)pdbAddr.pfield;    break;
  //   case DBF_ULONG  : unsigned long  *pfieldLink = (unsigned long  *)pdbAddr.pfield;    break;
  //   case DBF_FLOAT  : float          *pfieldLink = (float          *)pdbAddr.pfield;    break;
  //   case DBF_DOUBLE : double         *pfieldLink = (double         *)pdbAddr.pfield;    break;
  //   default : prnM3("[ERR] Record '%s' data type is unknown.\n", pname); return;
  // };
  
  waveformRecord *precordLink = (waveformRecord *)pdbAddr.precord;

  while(!file.eof())
  {
    getline(file, strToken);
    if(strToken[0] == '#' || strToken.empty()==true) continue;

    strcpy (str, strToken.c_str());

    if(!(pch = strtok (str,","))) continue;
    index = strtol(pch,NULL,10);
    
    if(index > 2047)
      break;

    if(!(pch = strtok (NULL,","))) continue;
    pfieldLink[index] = (unsigned long)strtoll(pch,NULL,10);
    // printf("pfieldLink[index] : %ld\r\n", (unsigned long)pfieldLink[index]);
  };  

  file.close();

	precordLink->nord = index+1;
	dbProcess((dbCommon*)precordLink);

	if (rtVal) {
    prnM3("[ERR] Record '%s' not found\n", pname);
    return;
	}

  printf("waveformInitFromFile( %s )\r\n", pname);

}





asynParamType timingAsynEpics::getAsynParamType(const char *paramstring)
{
  asynParamType paramtype = asynParamNotDefined;

  if( 0 == strcmp("asynParamNotDefined",paramstring))
    paramtype = asynParamNotDefined;
  else if(0 == strcmp("asynParamInt32",paramstring))
    paramtype = asynParamInt32;
  else if(0 == strcmp("asynParamUInt32Digital",paramstring))
    paramtype = asynParamUInt32Digital;
  else if(0 == strcmp("asynParamFloat64",paramstring))
    paramtype = asynParamFloat64;
  else if(0 == strcmp("asynParamOctet",paramstring))
    paramtype = asynParamOctet;
  else if(0 == strcmp("asynParamInt8Array",paramstring))
    paramtype = asynParamInt8Array;
  else if(0 == strcmp("asynParamInt16Array",paramstring))
    paramtype = asynParamInt16Array;
  else if(0 == strcmp("asynParamInt32Array",paramstring))
    paramtype = asynParamInt32Array;
  else if(0 == strcmp("asynParamFloat32Array",paramstring))
    paramtype = asynParamFloat32Array;
  else if(0 == strcmp("asynParamFloat64Array",paramstring))
    paramtype = asynParamFloat64Array;
  else if(0 == strcmp("asynParamGenericPointer",paramstring))
    paramtype = asynParamGenericPointer;
  
  return (paramtype);
}






















// /* epics time stamp for C interface*/
// typedef struct epicsTimeStamp {
//     epicsUInt32    secPastEpoch;   /* seconds since 0000 Jan 1, 1990 */
//     epicsUInt32    nsec;           /* nanoseconds within second */
// } epicsTimeStamp;

// /*TS_STAMP is deprecated */
// #define TS_STAMP epicsTimeStamp



asynStatus timingAsynEpics::readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int *eomReason)
{
  int function = pasynUser->reason;
  asynStatus status = asynSuccess;
  // const char *paramName;

  epicsTimeStamp timeStamp;
  getTimeStamp(&timeStamp);
  pasynUser->timestamp = timeStamp;
  
  prnM1("%s(%s)\n",__FUNCTION__, regmaptable[function].drvname);

  int addr=0;

  status = getAddress(pasynUser, &addr); 
  if (status != asynSuccess) return(status);
  
  /* We just read the current value of the parameter from the parameter library.
   * Those values are updated whenever anything could cause them to change */
  status = (asynStatus)getStringParam(addr, function, (int)maxChars, value);

  status = pTsDev->readString(regmaptable[function], value);

  if (status) 
      epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                "%s:%s: status=%d, function=%d, value=%s", 
                driverName, __FUNCTION__, status, function, value);
  else        
      asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, 
            "%s:%s: function=%d, value=%s\n", 
            driverName, __FUNCTION__, function, value);
  
  if (eomReason) *eomReason = ASYN_EOM_END;
  *nActual = strlen(value)+1;
  status = (asynStatus) callParamCallbacks();

  return(status);
}


asynStatus timingAsynEpics::readInt32(asynUser *pasynUser, epicsInt32 *value)
{
  int function = pasynUser->reason;
  asynStatus status = asynSuccess;
  // const char *paramName;

  epicsTimeStamp timeStamp;
  getTimeStamp(&timeStamp);
  pasynUser->timestamp = timeStamp;
  
  prnM1("%s(%s)\n",__FUNCTION__, regmaptable[function].drvname);

  int addr=0;
  
  status = getAddress(pasynUser, &addr); 
  if (status != asynSuccess) return(status);

  /* We just read the current value of the parameter from the parameter library.
    * Those values are updated whenever anything could cause them to change */
  status = (asynStatus) getIntegerParam(addr, function, value);

  status = pTsDev->readReg(regmaptable[function], *value);

  if (status)
    epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                "%s:%s: status=%d, function=%d, value=%d", 
                driverName, __FUNCTION__, status, function, *value);
  else
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s:%s: function=%d, value=%d\n", 
                driverName, __FUNCTION__, function, *value);

  status = (asynStatus) callParamCallbacks();

  return(status);
}


asynStatus timingAsynEpics::readFloat64(asynUser *pasynUser, epicsFloat64 *value)
{
  int function = pasynUser->reason;
  asynStatus status = asynSuccess;
  // const char *paramName;

  epicsTimeStamp timeStamp;
  getTimeStamp(&timeStamp);
  pasynUser->timestamp = timeStamp;
  
  prnM1("%s(%s)\n",__FUNCTION__, regmaptable[function].drvname);

  int addr=0;
  float* pfData;
  epicsInt32 iVal = -1;
  epicsInt32 *piVal = &iVal;

  status = getAddress(pasynUser, &addr); 
  if (status != asynSuccess) return(status);

  /* We just read the current value of the parameter from the parameter library.
    * Those values are updated whenever anything could cause them to change */
  status = (asynStatus) getDoubleParam(addr, function, value);

  status = pTsDev->readReg(regmaptable[function], *piVal);
  pfData = ((float*)piVal) ;
  // *value  = 1000000000 * (epicsFloat64) (*pfData) ;
  *value  = (epicsFloat64) (*pfData) ;
  // printf("sizeof(fData), %d, %d\r\n", sizeof(fData), sizeof(rdData));
  // printf("pfData, %f, %lf\r\n", *pfData*1000000000, dVal*1000000000);
  // printf("readFloat64\r\n");

  if (status)
    epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, 
                "%s:%s: status=%d, function=%d, value=%f", 
                driverName, __FUNCTION__, status, function, *value);
  else
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s:%s: function=%d, value=%f\n", 
                driverName, __FUNCTION__, function, *value);

  status = (asynStatus) callParamCallbacks();

  return(status);
}

asynStatus timingAsynEpics::readInt32Array (asynUser *pasynUser, epicsInt32 *value, size_t nElements, size_t *nIn)
{
  int function = pasynUser->reason;
  asynStatus status = asynSuccess;
  // const char *paramName;

  epicsTimeStamp timeStamp;
  getTimeStamp(&timeStamp);
  pasynUser->timestamp = timeStamp;

  prnM2("%s(%s) : nEle(%d)\n",__FUNCTION__, regmaptable[function].drvname, nElements);

  if(checkParam(regmaptable[function].drvname) == -1) return(asynError);

  // for(size_t i = 0; i < (size_t)nElements;i++)
  // {    
  //   status = pTsDev->writeReg(regmaptable[function], (unsigned long)value[i] & 0xffffffff, 0.0, NULL, i);
  //   if (status != asynSuccess) return(status);

  //     //uval.push_back((unsigned long)value[i] & 0xffff);
  //     // uval.push_back((unsigned long)value[i] & 0xffffffff);
  //     // if(timingDebug && i < (size_t)timingPrintCount)
  //     //     printf("%s(%s)-[%ld]:LONG(0x%08x),Value(0x%f)\n",__FUNCTION__,regmap.drvname, i, uval.at(i), value[i]);
  // };   

  return status;
}

asynStatus timingAsynEpics::readFloat32Array(asynUser *pasynUser, epicsFloat32 *value, size_t nElements, size_t *nIn)
{
  int function = pasynUser->reason;
  asynStatus status = asynSuccess;
  // const char *paramName;

  epicsTimeStamp timeStamp;
  getTimeStamp(&timeStamp);
  pasynUser->timestamp = timeStamp;
  *nIn = 0;

  prnM2("%s(%s) : nEle(%d)\n",__FUNCTION__, regmaptable[function].drvname, nElements);

  if(checkParam(regmaptable[function].drvname) == -1) return(asynError);

  regmaptable[function].dataSize=nElements;
  status = pTsDev->readRegArray(regmaptable[function], (void*)value);
  if (status != asynSuccess) return(status);

  *nIn = nElements -1;

  // for(size_t i = 0; i < (size_t)nElements;i++)
  // {    
  //   status = pTsDev->writeReg(regmaptable[function], (unsigned long)value[i] & 0xffffffff, 0.0, NULL, i);
  //   if (status != asynSuccess) return(status);

  //     //uval.push_back((unsigned long)value[i] & 0xffff);
  //     // uval.push_back((unsigned long)value[i] & 0xffffffff);
  //     // if(timingDebug && i < (size_t)timingPrintCount)
  //     //     printf("%s(%s)-[%ld]:LONG(0x%08x),Value(0x%f)\n",__FUNCTION__,regmap.drvname, i, uval.at(i), value[i]);
  // };   

  return status;
}

asynStatus timingAsynEpics::readFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements, size_t *nIn)
{
  int function = pasynUser->reason;
  asynStatus status = asynSuccess;
  // const char *paramName;

  epicsTimeStamp timeStamp;
  getTimeStamp(&timeStamp);
  pasynUser->timestamp = timeStamp;

  // prnM2("%s(%s) : nEle(%d)\n",__FUNCTION__, regmaptable[function].drvname, nElements);
  // printf("%s(%s) : nEle(%d)\n",__FUNCTION__, regmaptable[function].drvname, nElements);

  if(checkParam(regmaptable[function].drvname) == -1) return(asynError);

  regmaptable[function].dataSize = (unsigned int)nElements;

  status = pTsDev->readRegArray(regmaptable[function], (void*)value);
  if (status != asynSuccess) return(status);

  // for(size_t i = 0; i < (size_t)nElements;i++)
  // {    
  //   status = pTsDev->writeReg(regmaptable[function], (unsigned long)value[i] & 0xffffffff, 0.0, NULL, i);
  //   if (status != asynSuccess) return(status);

  //     //uval.push_back((unsigned long)value[i] & 0xffff);
  //     // uval.push_back((unsigned long)value[i] & 0xffffffff);
  //     // if(timingDebug && i < (size_t)timingPrintCount)
  //     //     printf("%s(%s)-[%ld]:LONG(0x%08x),Value(0x%f)\n",__FUNCTION__,regmap.drvname, i, uval.at(i), value[i]);
  // };   

  return status;
}


asynStatus timingAsynEpics::writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual)
{
  int function = pasynUser->reason;
  asynStatus status = asynSuccess;
  const char *paramName;

  epicsTimeStamp timeStamp;
  getTimeStamp(&timeStamp);
  pasynUser->timestamp = timeStamp;
  
  prnM2("%s(%s) : %s\n",__FUNCTION__, regmaptable[function].drvname, value);

  /* Set the parameter in the parameter library. */
  status = (asynStatus) setStringParam(function, value);
  if(status != asynSuccess) return(status);

  if(checkParam(regmaptable[function].drvname) == -1) return(asynError);
  
  if(dataType_pvValSaveCmd != regmaptable[function].dataType){
    status = pTsDev->writeString(regmaptable[function], value);
    if (status != asynSuccess) return(status);
  }
  else{
    printf("call cfgSaveToFile\r\n");
    cfgSaveToFile();
  }
  
  /* Fetch the parameter string name for possible use in debugging */
  getParamName(function, &paramName);
  /* Do callbacks so higher layers see any changes */
  if (status) 
    epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, "%s:%s: status=%d, function=%d, name=%s, value=%d", driverName, __FUNCTION__, status, function, paramName, value);
  else        
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s:%s: function=%d, name=%s, value=%d\n", driverName, __FUNCTION__, function, paramName, value);
    
  *nActual = strlen(value);
  return (status);
}


asynStatus timingAsynEpics::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
  int function = pasynUser->reason;
  asynStatus status = asynSuccess;
  const char *paramName;

  epicsTimeStamp timeStamp;
  getTimeStamp(&timeStamp);
  pasynUser->timestamp = timeStamp;
  
  prnM2("%s(%s) : %d\n",__FUNCTION__, regmaptable[function].drvname, value);

  int addr = 0;
  status = getAddress(pasynUser, &addr); 
  if (status != asynSuccess) return(status);

  /* Set the parameter in the parameter library. */
  status = (asynStatus) setIntegerParam(function, value);
  if (status != asynSuccess) return(status);

  if(checkParam(regmaptable[function].drvname) == -1) return(asynError);

  status = pTsDev->writeReg(regmaptable[function], value);
  if (status != asynSuccess) return(status);

  /* Fetch the parameter string name for possible use in debugging */
  getParamName(function, &paramName);
  /* Do callbacks so higher layers see any changes */
  if (status) 
    epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, "%s:%s: status=%d, function=%d, name=%s, value=%d", driverName, __FUNCTION__, status, function, paramName, value);
  else        
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s:%s: function=%d, name=%s, value=%d\n", driverName, __FUNCTION__, function, paramName, value);

  return status;
}



asynStatus timingAsynEpics::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
  int function = pasynUser->reason;
  asynStatus status = asynSuccess;
  const char *paramName;

  epicsTimeStamp timeStamp;
  getTimeStamp(&timeStamp);
  pasynUser->timestamp = timeStamp;
  
  prnM2("%s(%s) : %lf\n",__FUNCTION__, regmaptable[function].drvname, value);

  int addr = 0;
  status = getAddress(pasynUser, &addr); 
  if (status != asynSuccess) return(status);

  /* Set the parameter in the parameter library. */
  status = (asynStatus) setDoubleParam(function, value);
  if (status != asynSuccess) return(status);

  if(checkParam(regmaptable[function].drvname) == -1) return(asynError);

  float fData;
  fData = (float)value;
  int*  piData = ((int*)&fData);

  // float*  pfData = ((float*)piData);
  // epicsFloat64 dData;
  // dData = (epicsFloat64)*pfData;
  // printf("sizeof(value) %d, sizeof(fData) %d\r\n", sizeof(value), sizeof(fData));
  // //printf("value 0x%016x, fData 0x%08x, piData 0x%08x\r\n", value, fData, *piData);
  // printf("value %.1f, fData %.1f, dData %.1lf, piData 0x%08x\r\n", value, fData, dData, *piData);

  // printf("pfData, %f, %lf\r\n", *pfData*1000000000, fVal*1000000000);
  status = pTsDev->writeReg(regmaptable[function], *piData);
  if (status != asynSuccess) return(status);

  /* Fetch the parameter string name for possible use in debugging */
  getParamName(function, &paramName);
  /* Do callbacks so higher layers see any changes */
  if (status) 
    epicsSnprintf(pasynUser->errorMessage, pasynUser->errorMessageSize, "%s:%s: status=%d, function=%d, name=%s, value=%d", driverName, __FUNCTION__, status, function, paramName, value);
  else        
    asynPrint(pasynUser, ASYN_TRACEIO_DRIVER, "%s:%s: function=%d, name=%s, value=%d\n", driverName, __FUNCTION__, function, paramName, value);

  return status;
}

asynStatus timingAsynEpics::writeInt32Array(asynUser *pasynUser, epicsInt32 *value, size_t nElements)
{
  int function = pasynUser->reason;
  asynStatus status = asynSuccess;
  // const char *paramName;

  epicsTimeStamp timeStamp;
  getTimeStamp(&timeStamp);
  pasynUser->timestamp = timeStamp;

  prnM2("%s(%s) : nEle(%d)\n",__FUNCTION__, regmaptable[function].drvname, nElements);

  if(checkParam(regmaptable[function].drvname) == -1) return(asynError);

  for(size_t i = 0; i < (size_t)nElements;i++)
  { 
    status = pTsDev->writeReg(regmaptable[function], (unsigned long)value[i]);
    if (status != asynSuccess) return(status);
    regmaptable[function].address += 4; 

      //uval.push_back((unsigned long)value[i] & 0xffff);
      // uval.push_back((unsigned long)value[i] & 0xffffffff);
      // if(timingDebug && i < (size_t)timingPrintCount)
      //     printf("%s(%s)-[%ld]:LONG(0x%08x),Value(0x%f)\n",__FUNCTION__,regmap.drvname, i, uval.at(i), value[i]);
  };   

  return status;
}

asynStatus timingAsynEpics::writeFloat32Array(asynUser *pasynUser, epicsFloat32 *value, size_t nElements)
{
  int function = pasynUser->reason;
  asynStatus status = asynSuccess;
  // const char *paramName;

  epicsTimeStamp timeStamp;
  getTimeStamp(&timeStamp);
  pasynUser->timestamp = timeStamp;

  prnM2("%s(%s) : nEle(%d)\n",__FUNCTION__, regmaptable[function].drvname, nElements);

  if(checkParam(regmaptable[function].drvname) == -1) return(asynError);

  // for(size_t i = 0; i < (size_t)nElements;i++)
  // {    
  //   status = pTsDev->writeReg(regmaptable[function], (unsigned long)value[i] & 0xffffffff, 0.0, NULL, i);
  //   if (status != asynSuccess) return(status);

  //     //uval.push_back((unsigned long)value[i] & 0xffff);
  //     // uval.push_back((unsigned long)value[i] & 0xffffffff);
  //     // if(timingDebug && i < (size_t)timingPrintCount)
  //     //     printf("%s(%s)-[%ld]:LONG(0x%08x),Value(0x%f)\n",__FUNCTION__,regmap.drvname, i, uval.at(i), value[i]);
  // };   

  return status;
}

asynStatus timingAsynEpics::writeFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements)
{
  int function = pasynUser->reason;
  asynStatus status = asynSuccess;
  // const char *paramName;

  epicsTimeStamp timeStamp;
  getTimeStamp(&timeStamp);
  pasynUser->timestamp = timeStamp;

  prnM2("%s(%s) : nEle(%d)\n",__FUNCTION__, regmaptable[function].drvname, nElements);

  if(checkParam(regmaptable[function].drvname) == -1) return(asynError);

  // for(size_t i = 0; i < (size_t)nElements;i++)
  // {    
  //   status = pTsDev->writeReg(regmaptable[function], (unsigned long)value[i] & 0xffffffff, 0.0, NULL, i);
  //   if (status != asynSuccess) return(status);

  //     //uval.push_back((unsigned long)value[i] & 0xffff);
  //     // uval.push_back((unsigned long)value[i] & 0xffffffff);
  //     // if(timingDebug && i < (size_t)timingPrintCount)
  //     //     printf("%s(%s)-[%ld]:LONG(0x%08x),Value(0x%f)\n",__FUNCTION__,regmap.drvname, i, uval.at(i), value[i]);
  // };   

  return status;
}



asynStatus timingAsynEpics::db_put(const char *pname, const char *pvalue)
{
  long rtVal;
  long tmpValue;
	asynStatus status = asynSuccess;
	DBADDR addr;

  if (!pname || !*pname || !pvalue) {
		printf("ERROR!!! Usage: db_put \"pv name\", \"value\"\n");
		return asynError;
	}

	rtVal=dbNameToAddr(pname, &addr);
	if (rtVal) {
		printf("ERROR!!! Record '%s' not found\n", pname);
		return asynError;
	}

	/* For enumerated types must allow for ENUM rather than string*/
	/* If entire field is digits then use DBR_ENUM else DBR_STRING*/
	// if (addr.dbr_field_type == DBR_ENUM && !*pvalue && strspn(pvalue,"0123456789") == strlen(pvalue)){
	// 	unsigned short value;
	// 	sscanf(pvalue, "%hu", &value);
	// 	status = dbPutField(&addr, DBR_ENUM, &value, 1L);
	// } else if (addr.dbr_field_type == DBR_CHAR && addr.no_elements > 1) {

  if (addr.dbr_field_type == DBR_CHAR && addr.no_elements > 1) {
    // printf("db_put DBR_CHAR : %s \r\n", pname);
		rtVal = dbPutField(&addr, DBR_CHAR, pvalue, strlen(pvalue) + 1);
	} else if (addr.dbr_field_type == DBR_LONG) {
    tmpValue = strtoul(pvalue , NULL, 10 );     
    // printf("db_put DBR_LONG : %s %d\r\n", pname, tmpValue);
		rtVal = dbPutField(&addr, DBR_LONG, (void*)&tmpValue, 1L);
	} else {
    // printf("db_put ELSE : %s \r\n", pname);
		rtVal = dbPutField(&addr, DBR_STRING, pvalue, 1L);
	}
  
	if (rtVal) {
    printf("ERROR!!! dbPutField '%s':%d\n", pname, status);
		return asynError;
	}

	return asynSuccess;
}


asynStatus timingAsynEpics::db_get(const char *pname, char *pvalue)
{
  long rtVal;
	asynStatus status = asynSuccess;
	DBADDR addr;
	
	char	*pfield_value;
	DBENTRY	dbentry;
	DBENTRY	*pdbentry = &dbentry;
	

	 if (!pname || !*pname || !pvalue) {
    printf("ERROR!!! Usage: db_get \"pv name\", \"value\"\n");
		return asynError;
	}

	rtVal=dbNameToAddr(pname, &addr);
	if (rtVal) {
    printf("ERROR!!! Record '%s' not found\n", pname);
		return asynError;
	}

	dbInitEntry(pdbbase,pdbentry);
	rtVal = dbFindRecord(pdbentry,pname);
	if(rtVal) {
    printf("ERROR!!! dbFindRecord '%s':%d\n", pname, status);
		return asynError;
	}
	
	rtVal = dbFindField(pdbentry,"VAL:");
	pfield_value = (char *)dbGetString(pdbentry);
	sprintf(pvalue, "%s",  (pfield_value ? pfield_value : "null"));

	/*printf("\"%s\":VAL, %s\n", pname, pvalue );*/
	
	return asynSuccess;
}


asynStatus timingAsynEpics::db_get(const char *pname, int &value)
{
  char rdData[256];
	db_get(pname, rdData);

  value = (int)strtoll(rdData,NULL,10);

	return asynSuccess;
}







void timingAsynEpics::userP_timeSync()
{
  struct timeval tv;

  while(1)
  {
    pTsDev->timingNet_timeSync();
    // printf("end time : %09d\r\n",tv.tv_usec);
    gettimeofday(&tv, NULL);
    usleep(4000000 - tv.tv_usec); 
  }
}







void timingAsynEpics::userP_main()
{
  char tmpstr[64];
  char pname[128];
  int run = 1;
  int i;
  int rtVal;
  // int testVal;
  // int testVal2=0;

  printf("userP_main() start ready..........2\r\n");
  sprintf(pname,"%s:%s",sysDevName,"SYS_P_RUN");
  
  for(i=0;i<100;i++){
    printf("xxxxxx  1\r\n");
    epicsEventWaitWithTimeout(eventId_, 1);
    lock();
    rtVal = pTsDev->devThread();
    
    printf("xxxxxx  2\r\n");
    if(rtVal == RET_ERR)
      taskDelay(1000000);
      
    printf("xxxxxx  3\r\n");
    unlock();
    if(asynSuccess == db_get(pname, tmpstr))
      printf("SYS_P_RUN : %s:%d\r\n",tmpstr,i);

    printf("xxxxxx  4\r\n");
    if(strcmp(tmpstr, "RUN") == 0 ){
      isInit = 1;
      break;
    }
    printf(".");
  }
  printf(".\r\n");
  printf("userP_main() ready Done!!\r\n");


#if 1
  string iValFileName_cfg;
  iValFileName_cfg = iValFileName;
  iValFileName_cfg += "config.init";
  // cfgInitFromFile(iValFileName_cfg);
  cfgInitFromFile();

  if( (pTsDev->tsMode == RAON_EVG) || (pTsDev->tsMode == RAON_EVS) || (pTsDev->tsMode == RAON_EVRUP) )
  {
#if 1

    waveformInitFromFile("EVG1_SEQA_CFG");
    waveformInitFromFile("EVG1_SEQB_CFG");
    waveformInitFromFile("EVG1_SEQA_TSTAMP");
    waveformInitFromFile("EVG1_SEQB_TSTAMP");
    // waveformInitFromFile("EVR1_MAP_RAM_N");
    // waveformInitFromFile("EVR1_MAP_RAM");
#endif    
  }
  // else if( (pTsDev->tsMode == RAON_EVR) )
  // {
  //   waveformInitFromFile("EVR1_MAP_RAM_N");
  //   waveformInitFromFile("EVR1_MAP_RAM");
  // }
  // else if( (pTsDev->tsMode == RAON_ZQ9R) )
  // {
  //   waveformInitFromFile("EVR0_MAP_RAM_N");
  //   waveformInitFromFile("EVR0_MAP_RAM");
  // }
  // else if( (pTsDev->tsMode == RAON_EVF) )
  // {
  // }
#endif



  //struct timespec vartime;
  //long time_elapsed_nanos;


  /* Loop forever 
   * If the logic will be able to block into scan thread in IOC, this thread should work in IOC.
   */    
  // lock();

  while (1) { 
    // epicsEventWaitWithTimeout(eventId_, 0.5);
    // lock();
    rtVal = pTsDev->devThread();
    
    if(rtVal == RET_ERR)
    {
      printf("[ERR] epicsEventWaitWithTimeout===\n");
      taskDelay(500000);
    }
    // unlock();

    if(!run)continue;
  }
}

#if 0
//Multi Return Test
tuple < float, int, int, int > timingAsynEpics::cal(int n1, int n2)
{
  return make_tuple((float)n1/n2, n1%n2, n1+n2, n1*n2);
}

void timingAsynEpics::caltest()
{
  float re1;
  int re2, re3, re4;

  tie(re1, re2, re3, re4) = cal(5, 10);

  auto result = cal(5,10);

  printf("re1(%f), re2(%d), re3(%d), re4(%d)\n", re1, re2, re3, re4);
  printf("re1(%f), re2(%d), re3(%d), re4(%d)\n", get<0>(result), get<1>(result), get<2>(result), get<3>(result)); 

}
#endif



int timingAsynEpics::checkParam(const string drvname)
{
  int checkval = 0;
  check_iter = regmapfile.find(drvname);

  if(check_iter == regmapfile.end())
    checkval = -1;

  return (checkval);
}

// int timingAsynEpics::setParamValue(const string drvname, const int ival)
// {
//   int check = checkParam(drvname);
//   if(check == 0)
//     setIntegerParam(regmapfile[drvname].index, ival);
//   return (check);
// }

// int timingAsynEpics::setParamValue(const string drvname, const double dval)
// {
//   int check = checkParam(drvname);
//   if(check == 0)
//     setDoubleParam(regmapfile[drvname].index, dval);
//   return (check);
// }

// int timingAsynEpics::setParamValue(const string drvname, const string svalue)
// {
//   int check = checkParam(drvname);
//   if(check == 0)
//     setStringParam(regmapfile[drvname].index, svalue.c_str());
//   return (check);
// }

// asynStatus timingAsynEpics::getParamValue(const string drvname, int maxChars, char *value )
// {
//   asynStatus status = asynSuccess;
//   getStringParam(regmapfile[drvname].index, maxChars, value);
//   return (status);
// }

// asynStatus timingAsynEpics::getParamValue(const string drvname, int &value)
// {
//   asynStatus status = asynSuccess;
//   getIntegerParam(regmapfile[drvname].index, &value);
//   return (status);
// }

// asynStatus timingAsynEpics::getParamValue(const string drvname, double &value)
// {
//   asynStatus status = asynSuccess;
//   getDoubleParam(regmapfile[drvname].index, &value);
//   return (status);
// }













void userP_timeSync(void *drvPvt)
{
  timingAsynEpics *pPvt = (timingAsynEpics *)drvPvt;

  if (pPvt->clientThreadMode==10) 
  {
    fprintf(stderr, "start snapshot user task (mode real samples)\n");
    pPvt->userP_timeSync();
  }
}


void userP_main(void *drvPvt)
{
  timingAsynEpics *pPvt = (timingAsynEpics *)drvPvt;

  if (pPvt->clientThreadMode==10) 
  {
    fprintf(stderr, "start snapshot user task (mode real samples)\n");
    pPvt->userP_main();
  }
}

extern "C" {

/** EPICS iocsh callable function to call constructor for the testAsynPortDriver class.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] maxSizeSnapshot The maximum  number of sample in one snapshot
  * \param[in] maxNbSnapshot The number of snapshot buffered
 */
epicsShareFunc int timingAsynEpicsConfigure(const char *portName, int maxSizeSnapshot, int maxNbSnapshot, int clientMode,                     \
                                            const char* serverPath, const char* localPath,                                                    \
                                            const char *deviceSys, const char *deviceSubSys, const char *deviceName, const char *deviceNum,     \
                                            const int opMode, const int tzone, const int tickPeriod, const int reserved3, const int reserved4, const int reserved5 )
{
  new timingAsynEpics(portName, maxSizeSnapshot, maxNbSnapshot, clientMode,     \
                      serverPath, localPath,                                    \
                      deviceSys, deviceSubSys, deviceName, deviceNum,           \
                      opMode, tzone, tickPeriod, reserved3, reserved4, reserved5);
  return(asynSuccess);
}


/* EPICS iocsh shell commands */
static const iocshArg initArg0  = { "portName"          ,iocshArgString};
static const iocshArg initArg1  = { "max size snapshot" ,iocshArgInt};
static const iocshArg initArg2  = { "max size buffer MB",iocshArgInt};
static const iocshArg initArg3  = { "client mode"       ,iocshArgInt};
static const iocshArg initArg4  = { "epics server path" ,iocshArgString};
static const iocshArg initArg5  = { "register file path",iocshArgString};
static const iocshArg initArg6  = { "device system"     ,iocshArgString};
static const iocshArg initArg7  = { "device subsystem"  ,iocshArgString};
static const iocshArg initArg8  = { "device name"       ,iocshArgString};
static const iocshArg initArg9  = { "device number"     ,iocshArgString};

static const iocshArg initArg10 = { "operating mode"    ,iocshArgInt};
static const iocshArg initArg11 = { "time zone"         ,iocshArgInt};
static const iocshArg initArg12 = { "tick period"       ,iocshArgInt};
static const iocshArg initArg13 = { "reserverd 3"       ,iocshArgInt};
static const iocshArg initArg14 = { "reserverd 4"       ,iocshArgInt};
static const iocshArg initArg15 = { "reserverd 5"       ,iocshArgInt};

static const iocshArg * const initArgs[] = {&initArg0 ,
                                            &initArg1 ,
                                            &initArg2 ,
                                            &initArg3 ,
                                            &initArg4 ,
                                            &initArg5 ,
                                            &initArg6 ,
                                            &initArg7 ,
                                            &initArg8 ,
                                            &initArg9 ,
                                            &initArg10,
                                            &initArg11,
                                            &initArg12,
                                            &initArg13,
                                            &initArg14,
                                            &initArg15};

static const iocshFuncDef initFuncDef = {"timingAsynEpicsConfigure",16,initArgs};
static void initCallFunc(const iocshArgBuf *args)
{
  timingAsynEpicsConfigure( args[0].sval, args[1].ival, args[2].ival, args[3].ival, args[4].sval, args[5].sval, args[6].sval, args[7].sval, args[8].sval, args[9].sval, \
                            args[10].ival, args[11].ival, args[12].ival, args[13].ival, args[14].ival, args[15].ival);
}

void timingAsynEpicsRegister(void)
{
  iocshRegister(&initFuncDef,initCallFunc);
}

epicsExportRegistrar(timingAsynEpicsRegister);
}//end extern "C"
