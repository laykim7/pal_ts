//Wrapping class for system calls of timing device driver.

#include <math.h>

#include <stdlib.h>
#include <termios.h>

#include "timingData.h"
#include "tsDev.h"

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
//----===@ tsDev
// Parameters  :
// Description :
tsDev::tsDev()
{
  // prnM2("[F:%s]\n",__FUNCTION__);
  int i;
  isInit = -1;
  
  if(xadc_core_init(EXADC_INIT_READ_ONLY) != 0){
    isInitXADC = -1;
    prnM3("Coudn't Start the XADC Core : Exiting\r\n");
    prnErr();
    return;
  }
  else{
    isInitXADC = 1;
    initXADC();
    prnXADC();
  }

  for(i=0;i<ipIdCntMax;i++){
    gtp[i] = NULL;
    evr[i] = NULL;
  }

  prnM3("tsDev init ok;\r\n");
}

//=====================================
//----===@ ~tsDev
// Parameters  :
// Description :
tsDev::~tsDev()
{
  // prnM2("[F:%s]\n",__FUNCTION__);
  delete gtp[0] ;
  delete evr[0] ;
  // delete evg[0] ;
  prnM3("tsDev finalize ok;\r\n");
}


//=====================================
//----===@ initXADC
// Parameters  :
// Description :
int tsDev::initXADC(void)
{
    sprintf(xadcCmd[EParamVccInt ].name, "VccInt " );    xadcCmd[EParamVccInt ].parameter_id = EParamVccInt ;   sprintf( xadcCmd[EParamVccInt ].unit, "mV");
    sprintf(xadcCmd[EParamVccAux ].name, "VccAux " );    xadcCmd[EParamVccAux ].parameter_id = EParamVccAux ;   sprintf( xadcCmd[EParamVccAux ].unit, "mV");
    sprintf(xadcCmd[EParamVccBRam].name, "VccBRam" );    xadcCmd[EParamVccBRam].parameter_id = EParamVccBRam;   sprintf( xadcCmd[EParamVccBRam].unit, "mV");
    sprintf(xadcCmd[EParamVccpInt].name, "VccpInt" );    xadcCmd[EParamVccpInt].parameter_id = EParamVccpInt;   sprintf( xadcCmd[EParamVccpInt].unit, "mV");
    sprintf(xadcCmd[EParamVccpAux].name, "VccpAux" );    xadcCmd[EParamVccpAux].parameter_id = EParamVccpAux;   sprintf( xadcCmd[EParamVccpAux].unit, "mV");
    sprintf(xadcCmd[EParamVccoddr].name, "Vccoddr" );    xadcCmd[EParamVccoddr].parameter_id = EParamVccoddr;   sprintf( xadcCmd[EParamVccoddr].unit, "mV");
    sprintf(xadcCmd[EParamVrefp  ].name, "Vrefp  " );    xadcCmd[EParamVrefp  ].parameter_id = EParamVrefp  ;   sprintf( xadcCmd[EParamVrefp  ].unit, "mV");
    sprintf(xadcCmd[EParamVrefn  ].name, "Vrefn  " );    xadcCmd[EParamVrefn  ].parameter_id = EParamVrefn  ;   sprintf( xadcCmd[EParamVrefn  ].unit, "mV");
    sprintf(xadcCmd[EParamTemp   ].name, "Temp   " );    xadcCmd[EParamTemp   ].parameter_id = EParamTemp   ;   sprintf( xadcCmd[EParamTemp   ].unit, "Degree Celsius");
    return RET_OK;
}

//=====================================
//----===@ getXADC
// Parameters  :
// Description :
int tsDev::getXADC(void)
{
  ifRet(-1 == isInitXADC);
  int i;

  for (i=0; i < EParamMax; i++)
    xadcCmd[i].value = xadc_touch(xadcCmd[i].parameter_id);
  return RET_OK;
}

//=====================================
//----===@ prnXADC
// Parameters  :
// Description :
int tsDev::prnXADC(void)
{
  ifRet(RET_ERR == getXADC());
  
  prnM2("======================================================\r\n");
  prnM2("++ prnXADC ++\r\n");

  prnM2("VccInt  : %6.1f mV\r\n", xadcCmd[EParamVccInt ].value);
  prnM2("VccAux  : %6.1f mV\r\n", xadcCmd[EParamVccAux ].value);
  prnM2("VccBRam : %6.1f mV\r\n", xadcCmd[EParamVccBRam].value);
  prnM2("VccpInt : %6.1f mV\r\n", xadcCmd[EParamVccpInt].value);
  prnM2("VccpAux : %6.1f mV\r\n", xadcCmd[EParamVccpAux].value);
  prnM2("Vccoddr : %6.1f mV\r\n", xadcCmd[EParamVccoddr].value);
  prnM2("Vrefp   : %6.1f mV\r\n", xadcCmd[EParamVrefp  ].value);
  prnM2("Vrefn   : %6.1f mV\r\n", xadcCmd[EParamVrefn  ].value);
  prnM2("Temp    : %3.1f'  \r\n", xadcCmd[EParamTemp   ].value);
  
  return RET_OK;
}

//=====================================
//----===@ get_strTime
// Parameters  :
// Description :
int tsDev::get_strTime(drTime_T *evTime, char* strT)
{
  // if(evTime->tmT.tm_hour > 23 ) printf("%s.tm_hour %d\r\n",evTime->name, evTime->tmT.tm_hour); 
  // if(evTime->tmT.tm_min > 59 ) printf("%s.tm_min %d\r\n",evTime->name, evTime->tmT.tm_min); 
  // if(evTime->tmT.tm_sec > 59 ) printf("%s.tm_sec %d\r\n",evTime->name, evTime->tmT.tm_sec); 

  sprintf(strT, "%04d.%02d.%02d-%02d.%02d.%02d-%09d",\
          evTime->tmT.tm_year          ,\
          evTime->tmT.tm_mon+1           ,\
          evTime->tmT.tm_mday          ,\
          evTime->tmT.tm_hour          ,\
          evTime->tmT.tm_min           ,\
          evTime->tmT.tm_sec           ,\
          evTime->nsec);


  // printf("get_strTime %s\r\n", strT);
  return RET_OK;
}



// int tsDev::date_from_yday(uint yday)
// {
//   $yr = yday - 1900;
//   $yday = $ARGV[1];
//       $mytime = timelocal(1,0,0,1,1,$yr);
//       $mytime += ( 86400 * $yday );

//   ($sec,$min,$hour,$day,$mon,$yr,$wday,$yday,$whocares) =
//         localtime($mytime);
//   $yr += 1900;
//   printf("%02d/%02d/%d\n", $day, $mon, $yr);

//   #my $now = localtime time;
// }




//=====================================
//----===@ devThread
// Parameters  :
// Description :
int tsDev::devThread(void)
{
  // prnM2("[F:%s]\n",__FUNCTION__);
  ifRet(isInit<0);
  if(isInit)
  {
    int i;
    int rdData=0;
    int rdDataSize =0;
    
    evrMain->ip_intrEn(1);
    
    if((rdDataSize = read(evrMain->fd, &rdData, sizeof(rdData))) < 0){ //evr ip interrupt pending
      return RET_OK;
    }

    if(0 != rdData)
    {
      evrMain->ip_intrClear();

      //2020.01.09 by laykim : need more update 
      if((rdData & C_intrMsk_Emergency) == C_intrMsk_Emergency)
        prnM2("[Emergency]\r\n");

      // if((rdData & C_intrMsk_evCodeB_7F2) == C_intrMsk_evCodeB_7F2)
      //   delayCheck_Proc();
      //   // prnM2("[evCodeB_7F2]\r\n");

      if((rdData & C_intrMsk_evrW_alarm_sec) == C_intrMsk_evrW_alarm_sec)
      {
        devProc();

        for (i=0; i < EParamMax; i++)
          xadcCmd[i].value = xadc_touch(xadcCmd[i].parameter_id);

        // gettimeofday(&current_time, NULL);
        // pt_now = localtime((time_t*)&current_time);

        // if( (pt_now->tm_yday != evrMain->evTime.day) || (pt_now->tm_hour != evrMain->evTime.hour) || (pt_now->tm_min != evrMain->evTime.min) || (pt_now->tm_sec != evrMain->evTime.sec) )
        // {
        //   tm_sTime.tm_year = evrMain->evTime.year + 100 ; //evTime.year is 0~99, 2000 - 1900;
        //   set_time = mktime(&tm_sTime) + evrMain->evTime.day*86400 + evrMain->evTime.hour*3600 + evrMain->evTime.min*60 + evrMain->evTime.sec ;
        //   stime(&set_time);

        //   printf("**************** set time %d-%d.%d[%d]-%d.%d.%d **************** \n", pt_now->tm_year+1900, pt_now->tm_mon, pt_now->tm_mday, pt_now->tm_yday, pt_now->tm_hour, pt_now->tm_min, pt_now->tm_sec);
        // }
        // printf("time : %d-%d.%d[%d]-%d.%d.%d \n", pt_now->tm_year+1900, pt_now->tm_mon, pt_now->tm_mday, pt_now->tm_yday, pt_now->tm_hour, pt_now->tm_min, pt_now->tm_sec);
      }

    }
 }
  return RET_OK;
};


//=====================================
//----===@ devProc
// Parameters  :
// Description :
int tsDev::devProc(void)
{
  // prnM2("[F:%s]\n",__FUNCTION__);
	return RET_OK;
}


//=====================================
//----===@ readAppData
// Parameters  :
// Description :
asynStatus tsDev::readAppData(uint offset, uint *val)
{
  *val = 0;
  prnM1("readAppData\r\n");
  return asynSuccess;
}

//=====================================
//----===@ writeAppData
// Parameters  :
// Description :
asynStatus tsDev::writeAppData(uint offset, uint val)
{
  prnM1("writeAppData\r\n");
  return asynSuccess;
}

//=====================================
//----===@ readReg
// Parameters  :
// Description :
asynStatus tsDev::readReg(const RegMap &rmap, int &val)
{
  val = 0;
  prnM1("readReg %s\r\n", rmap.drvname);
  return asynSuccess;
}

//=====================================
//----===@ readRegArray
// Parameters  :
// Description :
asynStatus tsDev::readRegArray(const RegMap &rmap, void *val)
{
  val = 0;
  prnM1("readRegArray %s\r\n", rmap.drvname);
  return asynSuccess;
}


//=====================================
//----===@ readString
// Parameters  :
// Description :
asynStatus tsDev::readString(const RegMap &rmap, char *val)
{
  asynStatus status = asynSuccess;
  return status;
}


//=====================================
//----===@ writeReg
// Parameters  :
// Description :
asynStatus tsDev::writeReg(const RegMap &rmap, const int val)
{
  prnM1("writeReg\r\n");
  return asynSuccess;
}

//=====================================
//----===@ writeRegArray
// Parameters  :
// Description :
asynStatus tsDev::writeRegArray(const RegMap &rmap, const void *val)
{
  val = 0;
  prnM1("writeRegArray %s\r\n", rmap.drvname);
  return asynSuccess;
}


//=====================================
//----===@ writeString
// Parameters  :
// Description :
asynStatus tsDev::writeString(const RegMap &rmap, const char *val)
{
  prnM1("writeString\r\n");
  return asynSuccess;
}

int tsDev::updateInfo(void)
{
  return RET_OK;
}

int tsDev::timingNet_timeSync()
{
  // struct tm *info;
  // time_t tt;
  // tt = tv.tv_sec;
  // info = localtime(&tt);
  // printf("%d.%d.%d-%d.%d.%d -- %d\r\n", info->tm_year, info->tm_mon+1, info->tm_mday, info->tm_hour, info->tm_min, info->tm_sec, info->tm_yday);
  return RET_OK;
}

//=====================================
//=====================================
} //name space end




