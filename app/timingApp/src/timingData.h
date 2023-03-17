/*******************************************************************************
 *                                                                             *
 *  Copyright (c) 2014 ~ by DURUTRONIX. All Rights Reserved.                   *
 *                                                                             *
 ******************************************************************************/

/*==============================================================================
                        EDIT HISTORY FOR MODULE

when                who            what, where, why
------------------- -------------  ---------------------------------------------
2018-05-07 14:53:53 laykim         Create
==============================================================================*/
#if !defined( __timingData_h__ )
#define __timingData_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "asynDriver.h"
#include "asynParamType.h"
#include "commonDefine.h"

#define RAON_VER      1.4f

#define RAON_EVG              (0x001)
#define RAON_EVR              (0x002)
#define RAON_EVF              (0x003)
#define RAON_EVS              (0x004) // ev stand alone
#define RAON_EVRUP            (0x005) // evr uplink
#define RAON_ZQ9R             (0x100) // evr embedded zq 900

#define TS_OP_MODE_NORMAL     (0)    // timing system operation mode - normal.
#define TS_OP_MODE_DELAYCHECK (1)    // timing system operation mode - delay check.
#define TS_OP_MODE_FANCHECK   (2)    // timing system operation mode - fan status check & display front LCD.

#define TS_CLASS_EVX          (1)
#define TS_CLASS_ZQ9R         (2)

#define SET_getTime (BIT9)

struct RegMap{
  int             index;
  char            drvname[64];
  asynParamType   pType;  //parameter type
  int             ipId;
  int             ipCnt;
  int             accType;  //access type
  unsigned int    address;
  int             addrType;
  int             dataType;
  int             dataWidth;
  int             dataOffset;
  unsigned int    dataSize;
};

#ifdef __cplusplus
}
#endif
#endif
/*==============================================================================
                                  END OF FILE
==============================================================================*/
