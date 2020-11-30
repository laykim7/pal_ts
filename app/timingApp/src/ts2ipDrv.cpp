//Wrapping class for system calls of timing device driver.
#include <stdlib.h>
#include <termios.h>

#include "ts2ipDrv.h"

#define MAGIC_NUM 0xDB


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
//----===@ ts2ipDrv
// Parameters  :
// Description :
ts2ipDrv::ts2ipDrv()
{
  tm_tmpTime.tm_mon  = 0;
  tm_tmpTime.tm_mday = 1;
  tm_tmpTime.tm_hour = 0;
  tm_tmpTime.tm_min  = 0;
  tm_tmpTime.tm_sec  = 0;
  
  // epics time 기준시 seconds since 0000 Jan 1, 1990
  tm_epsTime.tm_year = 90; //90 = 1990 - 1900;
  tm_epsTime.tm_mon  = 0;
  tm_epsTime.tm_mday = 1;
  tm_epsTime.tm_hour = 0;
  tm_epsTime.tm_min  = 0;
  tm_epsTime.tm_sec  = 0;

  tt_epsTimeOffset = mktime(&tm_epsTime);

  prnM0("ts2ipDrv()\r\n");
}

//=====================================
//----===@ ~ts2ipDrv
// Parameters  :
// Description :
ts2ipDrv::~ts2ipDrv()
{
  prnM0("ts2ipDrv();\r\n");
  close(fd);
  fd = -1;
  isInit = -1;
}

//=====================================
//----===@ ip_open
// Parameters  :
// Description :
int ts2ipDrv::ip_open(const char *deviceName)
{
  sprintf(devName,"%s",deviceName);

  fd  = open(devName,O_RDWR);
  if(fd < 0){
    prnM3("[ERR] ts2ipDrv::ip_open : %s\n", devName);
    return RET_ERR;
  }
  prnM3("ts2ipDrv::ip_open : %s\n", devName);
  
  sprintf(buildTime.name,"build time");
  buildTime.nsec = 0;

  ip_getInfo();
  isInit = 1;

  return RET_OK;
}


//=====================================
//----===@ ip_rd
// Parameters  :
// Description :
int ts2ipDrv::ip_rd(uint accType, uint offset, uint *rdData)
{
  ifRet(fd < 0);
  ifRet(accType > 2);
  ifRet(accType == 1);

  struct s_regRW reg;
  reg.offset = offset;
  reg.val    = 0;

  //prnM1("ip_rd offset : 0x%08x\r\n",offset);

  if (ioctl(fd, _IOWR(MAGIC_NUM, 1 + accType, struct s_regRW), &reg) < 0) 
    prnErrRet();

  *rdData = reg.val;
  return RET_OK;
}


//=====================================
//----===@ ip_wr
// Parameters  :
// Description :
int ts2ipDrv::ip_wr(uint accType, uint offset, uint wrData)
{
  ifRet(fd < 0);
  ifRet(accType > 2);
  ifRet(accType == 1);

  struct s_regRW reg;
  reg.offset = offset;
  reg.val    = wrData;

  if (ioctl(fd, _IOWR(MAGIC_NUM, 2 + accType, struct s_regRW), &reg) < 0) 
    prnErrRet();

  return RET_OK;
}




//=====================================
//----===@ ip_getInfo
// Parameters  :
// Description :
int ts2ipDrv::ip_getInfo(void)
{
  ifRet(fd < 0);
  uint rdData;

  ip_rd(accType_devRaw, A_ipSys_vendor, &rdData); 
  vendor = rdData;

  ip_rd(accType_devRaw, A_ipSys_info, &rdData);
  ipID        = ((rdData >>  24) & 0xff);
  ipID_sub    = ((rdData >>  16) & 0xff);  
  ipVer_major = ((rdData >>  12) & 0xf);
  ipVer_miner = ((rdData >>   4) & 0xff);
  ipVer_rev   = ((rdData >>   0) & 0xf);

  ip_rd(accType_devRaw, A_ipSys_buildTime, &rdData);
  buildTime.tmT.tm_year  = ((rdData & 0x007E0000) >> 17) + 2000; // 17
  buildTime.tmT.tm_mon   = ((rdData & 0x07800000) >> 23) - 1; // 23
  buildTime.tmT.tm_mday  = ((rdData & 0xF8000000) >> 27); // 27 bit
  buildTime.tmT.tm_hour  = ((rdData & 0x0001F000) >> 12); // 12
  buildTime.tmT.tm_min   = ((rdData & 0x00000FC0) >>  6); // 6
  buildTime.tmT.tm_sec   = ((rdData & 0x0000003F) >>  0); // 0

  return RET_OK;
}

//=====================================
//----===@ ip_getTick
// Parameters  :
// Description :
int ts2ipDrv::ip_getTick(void)
{
  ifRet(fd < 0);
  uint rdData;
  ip_rd(accType_devRaw, A_ipSys_clkCntr, &rdData);  
  return RET_OK;
}

//=====================================
//----===@ ip_prnInfo
// Parameters  :
// Description :
int ts2ipDrv::ip_prnInfo(void)
{
  prnM2("======================================================\r\n");
  prnM2("++ %s ip_prnInfo ++\r\n", devName);
  prnM2("------------------------------------------------------\r\n");
  ifRet(fd < 0);

  prnM2("%-12s: 0x%08x\r\n"    , "vendor", vendor);
  prnM2("%-12s: %02x.%02x\r\n" , "ipID  ", ipID, ipID_sub);
  prnM2("%-12s: %x.%02x.%x\r\n", "ipVer ", ipVer_major, ipVer_miner, ipVer_rev);
  ip_prnTime(&buildTime);
  prnM2("\r\n");

  prnStat();
  return RET_OK;
}

//=====================================
//----===@ ip_prnTime
// Parameters  :
// Description :
int ts2ipDrv::ip_prnTime(drTime_T* pevTime)
{
  ifRet(fd < 0);
  prnM2("%-12s: %04d.%02d.%02d-%02d:%02d:%02d-%09d[ns]", \
              pevTime->name                            , \
              pevTime->tmT.tm_year                     , \
              pevTime->tmT.tm_mon +1                     , \
              pevTime->tmT.tm_mday                     , \
              pevTime->tmT.tm_hour                     , \
              pevTime->tmT.tm_min                      , \
              pevTime->tmT.tm_sec                      , \
              (int)pevTime->nsec);

  return RET_OK;
}




//=====================================
//----===@ ip_intrEn
// Parameters  :
// Description :
int ts2ipDrv::ip_intrEn(uint wrData)
{
  ifRet(fd < 0);
  ip_wr(accType_devRaw, A_ipSys_config, wrData & 1);
  return RET_OK;
}

//=====================================
//----===@ ip_intrClear
// Parameters  :
// Description :
int ts2ipDrv::ip_intrClear(void)
{
  ifRet(fd < 0);
  ip_wr(accType_devRaw, A_ipSys_setting, 1);
  taskDelay(0);
  ip_wr(accType_devRaw, A_ipSys_setting, 0);
  return RET_OK;
}

//=====================================
//----===@ ip_intrMask
// Parameters  :
// Description :
int ts2ipDrv::ip_intrMask(uint wrData)
{
  ifRet(fd < 0);
  ip_wr(accType_devRaw, A_ipSys_intrMask, wrData);
  return RET_OK;
}


//=====================================
//----===@ ip_getIntrVal
// Parameters  :
// Description :
int ts2ipDrv::ip_getIntrVal(uint* rdData)
{
  ifRet(fd < 0);
  ip_rd(accType_devRaw, A_ipSys_intrReg, rdData);
  return RET_OK;
}

//=====================================
//----===@ ip_setCommand
// Parameters  :
// Description :
int ts2ipDrv::ip_setCommand(uint setCommand, uint setDelay)
{
  ifRet(fd < 0);
  static uint isRunning = 0; //mutex control
  int i;
  // printf("ip_setCommand %d, %d\r\n", setCommand, setDelay);
  
  for(i=0;i<10000;i++){
    if(isRunning == 0){
      break;
    }
    else{
      taskDelay(10);
    }
  }

  ifRet(1 == isRunning);
  isRunning = 1;

  ip_wr(accType_devRaw, A_setReg, setCommand);
  
  if(setDelay > 0)
    taskDelay(setDelay);

  ip_wr(accType_devRaw, A_setReg, 0);

  isRunning = 0;
  return RET_OK;
};

//=====================================
//----===@ ip_getFifoStat
// Parameters  :
// Description :
int ts2ipDrv::ip_getFifoStat(uint addr, struct s_fifoStat *fStat)
{
  ifRet(fd < 0);
  uint rdData;
  ip_rd(accType_devRaw, addr, &rdData);
  fStat->WRCOUNT     = ((rdData >> 16) & 0x3ff );
  fStat->RDCOUNT     = ((rdData >>  6) & 0x3ff );
  fStat->RDERR       = ((rdData >>  5) & 0x1 );
  fStat->WRERR       = ((rdData >>  4) & 0x1 );
  fStat->EMPTY       = ((rdData >>  3) & 0x1 );
  fStat->ALMOSTEMPTY = ((rdData >>  2) & 0x1 );
  fStat->FULL        = ((rdData >>  1) & 0x1 );
  fStat->ALMOSTFULL  = ((rdData >>  0) & 0x1 );

  return RET_OK;
}

//=====================================
//----===@ ip_prnFifoStat
// Parameters  :
// Description :
int ts2ipDrv::ip_prnFifoStat(struct s_fifoStat fStat, int prnLabel)
{
	if(prnLabel == 1)
	{
		prnM2("---------------------------------------------------------------------\r\n");
		prnM2("++ %s ip_prnFifoStat ++\r\n", devName);
		prnM2(" %-6s | %-5s | %-5s | %-4s | %-4s | %-5s | %-6s | %-4s | %-6s\r\n",\
				"name","wrcnt","rdcnt","werr","rerr","empty","Aempty","full","Afull");
		prnM2("---------------------------------------------------------------------\r\n");
	}

	prnM2(" %-6s | %-5d | %-5d | %-4d | %-4d | %-5d | %-6d | %-4d | %-6d\r\n",\
			fStat.name        ,\
			fStat.WRCOUNT     ,\
			fStat.RDCOUNT     ,\
			fStat.WRERR       ,\
			fStat.RDERR       ,\
			fStat.EMPTY       ,\
			fStat.ALMOSTEMPTY ,\
			fStat.FULL        ,\
			fStat.ALMOSTFULL  );
	return RET_OK;
}



// Return if year is leap year or not. 
static inline int isLeap(int y) 
{ 
    if ( ((y%100 != 0) && (y%4 == 0)) || y %400 == 0) 
        return 1; 
    return 0; 
};

const unsigned short int __mon_yday[2][13] =
{
    /* Normal years.  */
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    /* Leap years.  */
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};


//=====================================
//----===@ ip_getEvTime
// Parameters  :
// Description :
int ts2ipDrv::ip_getEvTime(uint addr1, uint addr2, drTime_T *drT)
{
  ifRet(fd < 0);

  uint rdData;
  uint tmp_year;
  uint tmp_day ;
  uint tmp_hour;
  uint tmp_min ;
  uint tmp_sec ;
  uint tmp_ms  ;
  uint tmp_tick;

  ip_rd(accType_devRaw, addr1, &rdData);
  tmp_year = ((rdData & 0x007E0000) >> 17); // 17 bit
  tmp_day  = ((rdData & 0xFF800000) >> 23); // 27 (1-366 day)
  tmp_hour = ((rdData & 0x0001F000) >> 12); // 12 (0-23 hour)
  tmp_min  = ((rdData & 0x00000FC0) >>  6); // 6  (0-59 min)
  tmp_sec  = ((rdData & 0x0000003F) >>  0); // 0  (0-59 sec)

  ip_rd(accType_devRaw, addr2, &rdData);
  tmp_year  = tmp_year + ((rdData & 0x80000000) >> 25) + 2000;
  tmp_ms    = ((rdData & 0x7FE00000) >> 21); // 0
  tmp_tick  = ((rdData & 0x001FFFFF) >>  0); // 0

  int leapYearIndex = isLeap(tmp_year);
  int secInYear;
  int month = 0;
  int mon;

  if(leapYearIndex == 0) 
    secInYear = 31536000;
  else
    secInYear = 31622400;

  int tmp_secOfYear = (tmp_day-1)*86400 + tmp_hour*3600 + tmp_min*60 + tmp_sec + drT->tzone;

  if (secInYear <= tmp_secOfYear) {
    tmp_year++;
    tmp_secOfYear -= secInYear;
  }

  uint conv_yday = ((uint) (tmp_secOfYear / 86400));   // 0-365
  uint conv_hour = (uint) (tmp_secOfYear % 86400) / 3600; // 0-23
  uint conv_min  = (uint) (tmp_secOfYear % 3600) / 60;    // 0-59
  uint dayOfMonth = 0; // (0 ~ 30)

  for (mon = 0; mon < 12; mon++) {
    if (conv_yday <= __mon_yday[leapYearIndex][mon+1]) {
      month = mon;
      dayOfMonth = conv_yday - __mon_yday[leapYearIndex][mon];
      break;
    }
  }

  if(tmp_year > 9999)
    tmp_year = 1900;
  
  drT->tmT.tm_year = tmp_year;
  
  if(month > 11) //(0~11)
    drT->tmT.tm_mon = 11; // (0~11)
  else
    drT->tmT.tm_mon = month;

  if(dayOfMonth > 30)
    drT->tmT.tm_mday = 31; // (1~31)
  else
    drT->tmT.tm_mday = dayOfMonth +1;

  if(conv_hour > 23)
    drT->tmT.tm_hour = 23; // (0~23)
  else
    drT->tmT.tm_hour = conv_hour;

  if(conv_min > 59)
    drT->tmT.tm_min = 59; // (0~59)
  else
    drT->tmT.tm_min = conv_min;

  if(tmp_sec > 59)
    drT->tmT.tm_sec = 59; // (0~59)
  else
    drT->tmT.tm_sec = tmp_sec;

  tm tm_tmpT; 
  tm_tmpT.tm_year = tmp_year -1900 ; //evTime.year is 0~99 [from irig-b control field PID6];
  tm_tmpT.tm_mon  = 0;
  tm_tmpT.tm_mday = 1;
  tm_tmpT.tm_hour = 0;
  tm_tmpT.tm_min  = 0;
  tm_tmpT.tm_sec  = 0;

  drT->secPastEpoch = mktime(&tm_tmpT) + tmp_secOfYear;
  drT->nsec         = (uint) (tmp_ms * 1000000.0 + tmp_tick * drT->tickPeriod);

  if(drT->nsec > 999999999)
    drT->nsec = 999999999;
  
  // if((tmp_hour > 23) || (tmp_min > 59) || (tmp_sec > 59))
    // printf("%s. Y_%08x D_%08x H_%08x M_%08x S_%08x ms_%08x t_%08x\r\n",drT->name, tmp_year, tmp_day, tmp_hour, tmp_min, tmp_sec, tmp_ms, tmp_tick); 

  // prnM2("%d, %f\r\n", drT->tzone, drT->tickPeriod);
  // prnM2("%08d, %08d, %f\r\n", tmp_ms, tmp_tick, drT->nsec );
  // prnM2("%08d, %08d\r\n", tmp_ms, tmp_tick);
  // prnM2("secPastEpoch %012d\r\n", drT->secPastEpoch);
  // prnM2("%d.%d.%d-%d.%d.%d : %d.%d\r\n", drT->tmT.tm_year, drT->tmT.tm_mon, drT->tmT.tm_mday, drT->tmT.tm_hour, drT->tmT.tm_min, drT->tmT.tm_sec, leapYearIndex, mon);
  
  return RET_OK;
}



















  // tm tm_tmpT; 
  // tm* tmpT;
  // time_t rawtime;

  // uint tmp_year;
  // uint tmp_day ;
  // uint tmp_hour;
  // uint tmp_min ;
  // uint tmp_sec ;
  // uint tmp_ms  ;
  // uint tmp_tick;

  // tm_tmpT.tm_year = tmp_year + 100 ; //evTime.year is 0~99 [from irig-b control field PID6];
  // tm_tmpT.tm_mon  = 0;
  // tm_tmpT.tm_mday = 1;
  // tm_tmpT.tm_hour = 0;
  // tm_tmpT.tm_min  = 0;
  // tm_tmpT.tm_sec  = 0;

  // drT->secPastEpoch = mktime(&tm_tmpT) + drT->tzone + (tmp_day-1)*86400 + tmp_hour*3600 + tmp_min*60 + tmp_sec;
  // tmpT = gmtime(&drT->secPastEpoch);
 
  // drT->tmT.tm_year = tmpT->tm_year +1900;
  // drT->tmT.tm_mon  = tmpT->tm_mon ;
  // drT->tmT.tm_mday = tmpT->tm_mday;
  // drT->tmT.tm_hour = tmpT->tm_hour;
  // drT->tmT.tm_min  = tmpT->tm_min ;
  // drT->tmT.tm_sec  = tmpT->tm_sec ;
  // drT->nsec        = tmp_ms * 1000000 + tmp_tick * drT->tickPeriod;

  // prnM2("%d, %lf\r\n", drT->tzone, drT->tickPeriod);


//=====================================
//----===@ ip_convEpsTime
// Parameters  :
// Description :
// typedef struct epicsTimeStamp {
//     epicsUInt32    secPastEpoch;   /* seconds since 0000 Jan 1, 1990 */
//     epicsUInt32    nsec;           /* nanoseconds within second */
// } epicsTimeStamp;
int ts2ipDrv::ip_convEpsTime(drTime_T* drT, epicsTimeStamp* tgT)
{
  ifRet(fd < 0);

  tgT->secPastEpoch = drT->secPastEpoch - tt_epsTimeOffset;
  tgT->nsec         = drT->nsec;

  return RET_OK;
}




//=====================================
//----===@ getStat
// Parameters  :
// Description :
int ts2ipDrv::getStat(void)
{
	return RET_OK;
}

//=====================================
//----===@ prnStat
// Parameters  :
// Description :
int ts2ipDrv::prnStat(void)
{
	return RET_OK;
}

} //name space end




