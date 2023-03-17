/*****************************************************************************
 *
 *     Author: Xilinx, Inc. (c) Copyright 2012 Xilinx Inc.
 *     All rights reserved.
 *
 *     This file may be used under the terms of the GNU General Public
 *     License version 3.0 as published by the Free Software Foundation
 *     and appearing in the file LICENSE.GPL included in the packaging of
 *     this file.  Please review the following information to ensure the
 *     GNU General Public License version 3.0 requirements will be met.
 *     http://www.gnu.org/copyleft/gpl.html.
 *
 *     With respect to any license that requires Xilinx to make available to
 *     recipients of object code distributed by Xilinx pursuant to such
 *     license the corresponding source code, and if you desire to receive
 *     such source code from Xilinx and cannot access the internet to obtain
 *     a copy thereof, then Xilinx hereby offers (which offer is valid for as
 *     long as required by the applicable license; and we may charge you the
 *     cost thereof unless prohibited by the license) to provide you with a
 *     copy of such source code; and to accept such offer send a letter
 *     requesting such source code (please be specific by identifying the
 *     particular Xilinx Software you are inquiring about (name and version
 *     number), to:  Xilinx, Inc., Legal Department, Attention: Software
 *     Compliance Officer, 2100 Logic Drive, San Jose, CA U.S.A. 95124.
 *
 *     XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
 *     AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
 *     SOLUTIONS FOR XILINX DEVICES.  BY PROVIDING THIS DESIGN, CODE,
 *     OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
 *     APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
 *     THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
 *     AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
 *     FOR YOUR IMPLEMENTATION.  XILINX EXPRESSLY DISCLAIMS ANY
 *     WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
 *     IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
 *     REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
 *     INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *     FOR A PARTICULAR PURPOSE.
 *
 *     CRITICAL APPLICATIONS WARRANTIES
 *     Xilinx products are not designed or intended to be fail-safe, or
 *     for use in any application requiring fail-safe performance, such as
 *     life-support or safety devices or systems, Class III medical devices,
 *     nuclear facilities, applications related to the deployment of airbags,
 *     or any other applications that could lead to death, personal injury,
 *     or severe property or environmental damage (individually and
 *     collectively,  "Critical Applications"). Customer assumes the sole
 *     risk and liability  of any use of Xilinx products in Critical
 *     Applications, subject only to applicable laws and regulations
 *     governing limitations on product liability.
 *
 *     THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF
 *     THIS FILE AT ALL TIMES.
 *
 *     This file is a part of sobel_qt application, which is based in part
 *     on the work of the Qwt project (http://qwt.sf.net).
 *
 *****************************************************************************/


#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ctype.h>
#include <assert.h>

/*
 * events.h is suppose to be part of cross compile tool's linux includes
 * But as its under development, the file is in ../linux_include
 * once it is in the main, once should include
 * #include <linux/iio/events.h>
 * This file should be always in sync with <kernel>/include/linux/iio/events.h
 *
 */
// #include "events.h"
#include "xadc_core.h"

//todo: provide isInit assertion for all the functions.

#define MULTIPLIER_12_BIT

#ifdef MULTIPLIER_12_BIT
  static const int multiplier = 1 << 12;
#elif MULTIPLIER_16_BIT
  static const int multiplier = 1 << 16;
#endif
  static const int mV_mul = 1000;

//function prototype
float conv_temperature(float input, enum EConvType conv_direction);
float conv_voltage(float input, enum EConvType conv_direction);
float conv_voltage_ext_ch(float input, enum EConvType conv_direction);

static int line_from_file(char* filename, char* linebuf);
// static int line_to_file(char* filename, char* linebuf);

static char gDevicePath[MAX_PATH_SIZE];
static char gNodeName[MAX_NAME_SIZE];
static bool gExitNow = false;
static enum XADC_Init_Type gInit_state = EXADC_NOT_INITIALIZED;

struct XadcParameter gXadcData[EParamMax] = {
  [EParamVccInt ] = { "in_voltage0_vccint_raw"  ,   0, conv_voltage    },
  [EParamVccAux ] = { "in_voltage1_vccaux_raw"  ,   0, conv_voltage    },
  [EParamVccBRam] = { "in_voltage2_vccbram_raw" ,   0, conv_voltage    },
  [EParamVccpInt] = { "in_voltage3_vccpint_raw" ,   0, conv_voltage    },
  [EParamVccpAux] = { "in_voltage4_vccpaux_raw" ,   0, conv_voltage    },
  [EParamVccoddr] = { "in_voltage5_vccoddr_raw" ,   0, conv_voltage    },
  [EParamVrefp  ] = { "in_voltage6_vrefp_raw"   ,   0, conv_voltage    },
  [EParamVrefn  ] = { "in_voltage7_vrefn_raw"   ,   0, conv_voltage    },
  [EParamTemp   ] = { "in_temp0_raw"            ,   0, conv_temperature}
};



static int read_xadc_param(struct XadcParameter *param)
{
  char filename[MAX_PATH_SIZE];
  char read_value[MAX_VALUE_SIZE];

  memset(filename, 0, sizeof(filename) );
  sprintf(filename, "%s/%s", gDevicePath, param->name );

  if (line_from_file(filename,read_value) == RET_SUCCESS)
  {
    param->value = param->conv_fn(atof(read_value), EConvType_Raw_to_Scale);
  }
  else
  {
    printf("\n***Error: reading file %s\n",filename);
    param->value = 0;
    return RET_FILE_READ_FAILED;
  }

  return RET_SUCCESS;
}


float xadc_touch(enum XADC_Param parameter)
{
  assert(gInit_state != EXADC_NOT_INITIALIZED);
  assert((parameter >= 0) && (parameter < EParamMax));
  if(read_xadc_param(gXadcData + parameter) != RET_SUCCESS)
  {
    perror("Error Updating the statistic \n");
    return 0;
  }
  return gXadcData[parameter].value;
}

static int get_iio_node(const char * deviceName)
{
  struct dirent **namelist;
  char file[MAX_PATH_SIZE];
  char name[MAX_NAME_SIZE];
  int i,n;
  int flag = 0;

  n = scandir(SYS_PATH_IIO, &namelist, 0, alphasort);
  if (n < 0)
    return RET_ERR_DEV_NOT_FOUND;

  for (i=0; i < n; i++)
  {
    sprintf(file, "%s/%s/name", SYS_PATH_IIO, namelist[i]->d_name);
    if ((line_from_file(file,name) == 0) && (strcmp(name,deviceName) ==  0))
    {
      flag =1;
      strcpy(gNodeName, namelist[i]->d_name);
      sprintf(gDevicePath, "%s/%s", SYS_PATH_IIO, gNodeName);
      break;
    }
  }

  if(flag == 0) return RET_ERR_DEV_NOT_FOUND;

  return RET_SUCCESS;
}

int xadc_core_init(enum XADC_Init_Type init_type)
{
  int ret = 0;
  gExitNow = false;  //just reassuring

  assert(gInit_state == EXADC_NOT_INITIALIZED);  // Make sure it is only called once.
  assert(init_type >= 0 && init_type < EXADC_NOT_INITIALIZED);

  if (get_iio_node(DEVICE_NAME) != RET_SUCCESS)
  {
    perror(DEVICE_NAME " Device Not Found");
    ret = -1;
    goto EXIT;
  }

  gInit_state = init_type;

EXIT:
  return ret;
}

int xadc_core_deinit(void)
{
  assert(gInit_state != EXADC_NOT_INITIALIZED);
  gExitNow = true;
  gInit_state = EXADC_NOT_INITIALIZED;
  return 0;
}


//utility functions
float conv_voltage(float input, enum EConvType conv_direction)
{
  float result=0;

  switch(conv_direction)
  {
  case EConvType_Raw_to_Scale:
    result = ((input * 3.0 * mV_mul)/multiplier);
    break;
  case EConvType_Scale_to_Raw:
    result = (input/(3.0 * mV_mul))*multiplier;
    break;
  default:
    printf("Convertion type incorrect... Doing no conversion\n");
    //  intentional no break;
  case EConvType_None:
      result = input;
      break;
  }

  return result;
}

float conv_voltage_ext_ch(float input, enum EConvType conv_direction)
{
  float result=0;

  switch(conv_direction)
  {
  case EConvType_Raw_to_Scale:
    result = ((input * mV_mul)/multiplier);
    break;
  case EConvType_Scale_to_Raw:
    result = (input/mV_mul)*multiplier;
    break;
  default:
    printf("Convertion type incorrect... Doing no conversion\n");
    //  intentional no break;
  case EConvType_None:
      result = input;
      break;
  }

  return result;
}

float conv_temperature(float input, enum EConvType conv_direction)
{
  float result=0;

  switch(conv_direction)
  {
  case EConvType_Raw_to_Scale:
    result = ((input * 503.975)/multiplier) - 273.15;
    break;
  case EConvType_Scale_to_Raw:
    result = (input + 273.15)*multiplier/503.975;
    break;
  default:
    printf("Conversion type incorrect... Doing no conversion\n");
    //  intentional no break;
  case EConvType_None:
      result = input;
      break;
  }

  return result;
}

static int line_from_file(char* filename, char* linebuf)
{
    char* s;
    int i;
    FILE* fp = fopen(filename, "r");
    if (!fp) return RET_CANNOT_OPEN_FILE;
    s = fgets(linebuf, MAX_VALUE_SIZE, fp);
    fclose(fp);
    if (!s) return RET_FILE_READ_FAILED;

    for (i=0; (*s)&&(i<MAX_VALUE_SIZE); i++) {
        if (*s == '\n') *s = 0;
        s++;
    }
    return RET_SUCCESS;
}

// static int line_to_file(char* filename, char* linebuf)
// {
//     int fd;
//     fd = open(filename, O_WRONLY);
//     if (fd < 0) return RET_CANNOT_OPEN_FILE;
//     write(fd, linebuf, strlen(linebuf));
//     close(fd);
//     return RET_SUCCESS;
// }

































