#ifndef   _ts2ipEvsys_H_
#define   _ts2ipEvsys_H_

#include <cstdio>
#include <string>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "commonDefine.h"
#include "ts2ipDrv.h"

using namespace std;

#define REF_CLK_Ext     0
#define REF_CLK_ob25MHz 1

#define IIC_maxDrv (4)
#define IIC_maxDrvNameL (15)
#define IIC_maxSlv_bp (1)

#define LCD_COLOR_RED    0xe0
#define LCD_COLOR_GREEN  0x5c
#define LCD_COLOR_BLUE   0x1f
#define LCD_COLOR_WHITE  0xff

#define IGEN_MODE_EXT   1
#define IGEN_MODE_USER  0
#define SET_igen_rst                (BIT29)
#define SET_igen_set_time           (BIT3)

enum iicBus{ IIC_BUS_bp, IIC_BUS_ob, IIC_BUS_sl, IIC_BUS_sh, IIC_BUS_MAX};

enum cpsOnboard{
        CPS_OB_GT0  =  0, CPS_OB_GT1  =  1,
        CPS_OB_FMC0 =  8, CPS_OB_FMC1 =  9, CPS_OB_FMC2 = 10, CPS_OB_FMC3 = 11,
        CPS_OB_SL   = 12, CPS_OB_SH   = 13,
        CPS_OB_CPS  = 14}; // ctrl <-> ctrl

enum cpsFanoutModule{
        CPS_SW_CTRL,
        CPS_SW_SW, //SL <-> SH
        CPS_SW_FR, //front <-> rear (same floor)
        CPS_SW_X , //front <-> rear (cross floor)
        CPS_SW_00, CPS_SW_01, CPS_SW_02, CPS_SW_03, CPS_SW_04, CPS_SW_05, CPS_SW_06, CPS_SW_07, CPS_SW_08, CPS_SW_09, CPS_SW_10, CPS_SW_11};

struct s_ts2iic{
  char strName[64];
  uint name;
  char addr;
  char isMux;
  char muxAddress;
  char muxNum;
};

struct s_ts2slv{
  int   busId;
  char  id  ;  
  char  name[32];
  char  cpsOut[16]   ;
  char  cpsOutEn[16] ;
  float temp;
  uint  io  ;  
};


namespace timing
{
  class ts2ipEvsys  : public ts2ipDrv
  {
    private:
    
    protected:

    public:
      ts2ipEvsys(const char *deviceName, int* ts_mode, char* ts_name, int* ts_opMode);
      ~ts2ipEvsys();

      int lcd_fd;
      char lcdStr[100];

      struct s_ts2slv ob;
      struct s_ts2slv sl;
      struct s_ts2slv sh;

      int*  ptsOpMode;
      int*  ptsMode;
      char* ptsName;
      
      uint fanA;
      uint fanB;
      uint fanC;
      uint fanD;
      uint isMaster;     
      uint PRSNT_M2C_L;
      uint f_SFP_LossA;
      uint f_SFP_LossB;
      uint f_SFP_prsntA;
      uint f_SFP_prsntB;
      uint slio;
      uint shio;
      uint chkDelay_buf;
      uint igen_enable;
      uint igen_mode_ext;

      int getStat(void);
      int prnStat(void);

      int init_bp(void);
      int init_clk(int clkSrc);
      int init_cps(int busId);
      int init_system();

      int setIgen(uint igenEnable, uint igenModeExt, uint year, uint day, uint hour, uint min, uint sec);
      int set_cpsByMode();
      int set_cpsConfig(struct s_ts2slv* pCfg);

      int set_slaveIO_byMode();
      int set_slaveIO_sub(s_ts2slv* pSlv, uint setVal);

      int set_sys_IO(uint lsbV, uint msbV);
      int set_sys_FMC_LED(uint link0, uint link1);
      int set_fanSpeed(char pwmFront, char pwmRear );
      int set_fp_led(unsigned char pwrOK, unsigned char fanERR, unsigned char epicsOK, unsigned char timingNetworkOK);

      int get_slaveBoardID(int busId);
      int get_slaveBoardName(struct s_ts2slv* pSlv );
      int get_slaveTemp(void);

      //===========================================================
      ssize_t lcdWrite(char value);
      void lcdWriteStr(const char *s);
      void lcdWrite2B(uint v);
      void lcdDrawStr(uint x, uint y, const char *s);
      void lcdClear(void);
      void lcdSetColor(unsigned char color);
      void lcdSetColRow(unsigned char col, unsigned char row);
      void lcdSetBgColor(unsigned char color);
      void lcdSetFont(unsigned char font);

      int updateLCD_Ver(char* cVer);
      int updateLCD_FAN();
      int updateLCD_Time(drTime_T* evTime);
      int updateLCD_Temperature(void);
      int updateLCD_rxCount(uint evCodeA_cntr, uint evCodeB_cntr);
      int updateLCD_Message( char color, const char* message);
      int updateFrontPanel_LED(char pwrOK, char fanERR, char epicsOK, char timingNetOK);

      //===========================================================
      int iic_fd[IIC_maxDrv];
      int iic_isOpen[IIC_maxDrv];
      int iic_slvNum[IIC_maxDrv];
      struct s_ts2iic* iic_piic[IIC_maxDrv];

      struct s_ts2iic* iic_getInfo(int busId, int slvid);
      int iic_setMux(int lfd, s_ts2iic* piic);
      int iic_wr(int busId, int slvId, uint offset, uint offset_size, char* pData, int size);
      int iic_rd(int busId, int slvId, uint offset, uint offset_size, char* pData, int size);
      int iic_prnInfo();

      //===========================================================
      int max7313_configIO(int busId, int slvId, char PF8, char P70 );
      int max7313_setOutput(int busId, int slvId, char PF8, char P70 );

      //===========================================================
      unsigned char si5338_read(unsigned char r_addr);
      uint si5338_write(unsigned char w_addr, unsigned char w_data);
      uint si5338_write_mask(unsigned char Addr, unsigned char Data, unsigned char Mask);


  };
};

#endif
