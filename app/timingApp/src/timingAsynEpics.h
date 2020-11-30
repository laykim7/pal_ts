#ifndef    _TIMING_ASYN_EPICS_H
#define    _TIMING_ASYN_EPICS_H

#define GCC_VERSION (__GNUC__ * 10000 \
    + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)

#if GCC_VERSION >= 40300
#include <tr1/unordered_map>
#define hash_map std::tr1::unordered_map
#else
#include <ext/hash_map>
#define hash_map __gnu_cxx::hash_map
namespace __gnu_cxx {
  template <>
    struct hash<std::string> {
      size_t operator() (const std::string& x) const {
        return hash<const char*>()(x.c_str());
      }
    };
};
#endif

#include <iostream>
#include <fstream>
#include <vector>
#include <tuple>

#include "asynPortDriver.h"

#include "timingData.h"
#include "tsDev.h"
#include "tsDev_evx.h"
#include "tsDev_zq9r.h"

extern "C" {
}

class timingAsynEpics : public asynPortDriver 
{
public:
  timingAsynEpics(const char *portName, int maxSizeSnapshot, int maxNbSnapshot, int clientMode,                     \
                  const char* serverPath, const char* localPath,                                                    \
                  const char *deviceSys, const char *deviceSubSys, const char *deviceName, const char *deviceNum,     \
                  const int opMode, const int tzone, const int tickPeriod, const int reserved3, const int reserved4, const int reserved5 );
  void userP_main();
  void userP_timeSync();
  int clientThreadMode;

protected:
  /** Values used for pasynUser->reason, and indexes into the parameter library. */

private:
  char firmwareVer[128];
  char softwareVer[128];

  string regFileName;

  const char *driverName;
  epicsEventId eventId_;
  int system_init_ok;
  int isInit;

  timing::tsDev *pTsDev;

  char iocStartTime[25];
  char sysDevName[128];
  char iValFileName[128];

  //GCC Version > 4.3, unordered_map
  hash_map<string, RegMap> regmapfile;
  hash_map<int,   RegMap> regmaptable;
  hash_map<string, RegMap>::const_iterator check_iter;

  typedef  vector<unsigned long> vecCode;
  hash_map<string, vecCode> vecCodeMap;

  int tsMode(const char* mode);
  int getIpFd(unsigned int IP_Index);
  void registerParamListFromFile(string fileName);
  asynParamType getAsynParamType(const char *paramstring);
  int checkParam(const string drvname);

  asynStatus writeOctet(asynUser *pasynUser, const char *value, size_t maxChars, size_t *nActual);
  asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
  asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);

  asynStatus readOctet(asynUser *pasynUser, char *value, size_t maxChars, size_t *nActual, int *eomReason);
  asynStatus readInt32(asynUser *pasynUser, epicsInt32 *value);
  asynStatus readFloat64(asynUser *pasynUser, epicsFloat64 *value);

  //New 
  void cfgInitFromFile(string fileName);
  void waveformInitFromFile(const char *pvname);

  asynStatus db_put(const char *pname, const char *pvalue);
  asynStatus db_get(const char *pname, char *pvalue);
  asynStatus db_get(const char *pname, int &value);

  //WaveformPV Setup
  asynStatus readInt32Array (asynUser *pasynUser, epicsInt32 *value, size_t nElements, size_t *nIn);
  asynStatus writeInt32Array(asynUser *pasynUser, epicsInt32 *value, size_t nElements);
  
  asynStatus readFloat32Array (asynUser *pasynUser, epicsFloat32 *value, size_t nElements, size_t *nIn);
  asynStatus writeFloat32Array(asynUser *pasynUser, epicsFloat32 *value, size_t nElements);

  asynStatus readFloat64Array (asynUser *pasynUser, epicsFloat64 *value, size_t nElements, size_t *nIn);
  asynStatus writeFloat64Array(asynUser *pasynUser, epicsFloat64 *value, size_t nElements);

  // int setParamValue(const string drvname, const string svalue);
  // int setParamValue(const string drvname, const int ival);
  // int setParamValue(const string drvname, const double dval);
  // asynStatus getParamValue(const string drvname, int maxChars, char *value );
  // asynStatus getParamValue(const string drvname, int &value);
  // asynStatus getParamValue(const string drvname, double &value);


  //tuple < float, int, int, int > cal(int n1, int n2);
  //void caltest();
};

#endif
