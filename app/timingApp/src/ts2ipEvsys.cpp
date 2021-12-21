//Wrapping class for system calls of timing device driver.
#include <stdlib.h>
#include <termios.h>

#include "timingData.h"
#include "ts2ipEvsys.h"

namespace timing{

//==============================================================================
//----===@ define
//==============================================================================
#define TS2Floor_SL  (0)
#define TS2Floor_SH  (1)

#define TS2IO_ModeInput  (0)
#define TS2IO_ModeOutput (1)

#define A_ifanStt4_CntrFront      slv_reg10
#define A_ifanStt4_CntrRear       slv_reg11
#define A_boardStat               slv_reg12
#define A_sttGtp_chkDelayCnt      slv_reg14

#define A_cfgSys_IO_LSB           slv_reg31 // m_st[15: 0]
#define A_cfgSys_IO_MSB           slv_reg32 // m_st[31:16]
#define A_cfgSys_FMC              slv_reg33
#define A_cfgEvg_igen_0           slv_reg34
#define A_cfgEvg_igen_1           slv_reg35
#define A_cfgSys_fanHLCntrMax     slv_reg37

#define SET_SLH_iic_rst           (BIT8)

#define DRV_NAME_iic_bp "/dev/i2c-0"
#define DRV_NAME_iic_ob "/dev/i2c-1"
#define DRV_NAME_iic_sh "/dev/i2c-3"
#define DRV_NAME_iic_sl "/dev/i2c-4"
#define DRV_NAME_lcd    "/dev/ttyUL1"

#define CPS_PORT_EN  (0x30)
#define CPS_PORT_DIS (0x00)

enum lcdLineNumber{
        LCD_LINE_1 = 1, LCD_LINE_2 = 2, LCD_LINE_3 = 3, LCD_LINE_4 = 4,
        LCD_LINE_5 = 5, LCD_LINE_6 = 6, LCD_LINE_7 = 7, LCD_LINE_8 = 8,
        LCD_LINE_9 = 9};

/************************** Constant Definitions *****************************/
#define IIC_SLAVE_FORCE 0x0706
#define IIC_SLAVE       0x0703  /* Change slave address     */
#define IIC_FUNCS       0x0705  /* Get the adapter functionality */
#define IIC_RDWR        0x0707  /* Combined R/W transfer (one stop only)*/

#define MAX_IIC_RW_BUF_SIZE (512)

/* Slave IIC Device Number... exclude IIC MUX */
#define IIC_maxSlv_bp (1)
#define IIC_maxSlv_ob (4)
#define IIC_maxSlv_sl (4)
#define IIC_maxSlv_sh (4)

/* slv iic name by num*/
#define SLV_MAX7313_A (0)
#define SLV_SI5338A   (1)
#define SLV_SI570     (2)
#define SLV_ADN4604   (3)
#define SLV_INA226A   (4)
#define SLV_MAX7313_B (5)
#define SLV_TMP112A   (6)

#define IIC_MUX_ADDR  (0x74) // PCA9546

/************************** si5338 *****************************/
#define IIC_SCLK_RATE       400000
#define IIC_ADDR            0x70
#define IIC_BUS             0

#define LOS_MASK_IN1IN2IN3  0x04
#define LOS_MASK            LOS_MASK_IN1IN2IN3
#define PLL_LOL             0x10
#define LOS_FDBK            0x08
#define LOS_CLKIN           0x04
#define SYS_CAL             0x01
#define LOCK_MASK           (PLL_LOL | LOS_CLKIN | SYS_CAL)
#define FCAL_OVRD_EN        0x80
#define SOFT_RESET          0x02
#define EOB_ALL             0x10
#define DIS_LOL             0x80

/************************** LCD *****************************/
#define LCD_LINE_1 1
#define LCD_LINE_2 2
#define LCD_LINE_3 3
#define LCD_LINE_4 4
#define LCD_LINE_5 5
#define LCD_LINE_6 6
#define LCD_LINE_7 7
#define LCD_LINE_8 8
#define LCD_LINE_9 9

//==============================================================================
//----===@ global variable
//==============================================================================
#define NUM_REGS_MAX 350

typedef struct Reg_Data{
  unsigned char Reg_Addr;
  unsigned char Reg_Val;
  unsigned char Reg_Mask;
} Reg_Data;


//#define REF_81_25MHZ
#define REF_125_MHZ

#ifdef REF_81_25MHZ
static Reg_Data Reg_ob25MHz[NUM_REGS_MAX] = {
{  0,0x00,0x00},{  1,0x00,0x00},{  2,0x00,0x00},{  3,0x00,0x00},{  4,0x00,0x00},{  5,0x00,0x00},{  6,0x08,0x1D},{  7,0x00,0x00},{  8,0x70,0x00},{  9,0x0F,0x00},{ 10,0x00,0x00},{ 11,0x00,0x00},{ 12,0x00,0x00},{ 13,0x00,0x00},{ 14,0x00,0x00},{ 15,0x00,0x00},
{ 16,0x00,0x00},{ 17,0x00,0x00},{ 18,0x00,0x00},{ 19,0x00,0x00},{ 20,0x00,0x00},{ 21,0x00,0x00},{ 22,0x00,0x00},{ 23,0x00,0x00},{ 24,0x00,0x00},{ 25,0x00,0x00},{ 26,0x00,0x00},{ 27,0x70,0x80},{ 28,0x0B,0xFF},{ 29,0x08,0xFF},{ 30,0xB0,0xFF},{ 31,0xC0,0xFF},
{ 32,0xC0,0xFF},{ 33,0xC0,0xFF},{ 34,0xC0,0xFF},{ 35,0xAA,0xFF},{ 36,0x06,0x1F},{ 37,0x06,0x1F},{ 38,0x06,0x1F},{ 39,0x06,0x1F},{ 40,0x84,0xFF},{ 41,0x10,0x7F},{ 42,0x24,0x3F},{ 43,0x00,0x00},{ 44,0x00,0x00},{ 45,0x00,0xFF},{ 46,0x00,0xFF},{ 47,0x14,0x3F},
{ 48,0x3D,0xFF},{ 49,0x00,0xFF},{ 50,0xC4,0xFF},{ 51,0x07,0xFF},{ 52,0x10,0xFF},{ 53,0x00,0xFF},{ 54,0x0D,0xFF},{ 55,0x00,0xFF},{ 56,0x00,0xFF},{ 57,0x00,0xFF},{ 58,0x00,0xFF},{ 59,0x01,0xFF},{ 60,0x00,0xFF},{ 61,0x00,0xFF},{ 62,0x00,0x3F},{ 63,0x10,0xFF},
{ 64,0x00,0xFF},{ 65,0x0D,0xFF},{ 66,0x00,0xFF},{ 67,0x00,0xFF},{ 68,0x00,0xFF},{ 69,0x00,0xFF},{ 70,0x01,0xFF},{ 71,0x00,0xFF},{ 72,0x00,0xFF},{ 73,0x00,0x3F},{ 74,0x10,0xFF},{ 75,0x00,0xFF},{ 76,0x0D,0xFF},{ 77,0x00,0xFF},{ 78,0x00,0xFF},{ 79,0x00,0xFF},
{ 80,0x00,0xFF},{ 81,0x01,0xFF},{ 82,0x00,0xFF},{ 83,0x00,0xFF},{ 84,0x00,0x3F},{ 85,0x10,0xFF},{ 86,0x00,0xFF},{ 87,0x0D,0xFF},{ 88,0x00,0xFF},{ 89,0x00,0xFF},{ 90,0x00,0xFF},{ 91,0x00,0xFF},{ 92,0x01,0xFF},{ 93,0x00,0xFF},{ 94,0x00,0xFF},{ 95,0x00,0x3F},
{ 96,0x10,0x00},{ 97,0xC0,0xFF},{ 98,0x2E,0xFF},{ 99,0x00,0xFF},{100,0x00,0xFF},{101,0x00,0xFF},{102,0x00,0xFF},{103,0x02,0xFF},{104,0x00,0xFF},{105,0x00,0xFF},{106,0x80,0xBF},{107,0x00,0xFF},{108,0x00,0xFF},{109,0x00,0xFF},{110,0x00,0xFF},{111,0x00,0xFF},
{112,0x00,0xFF},{113,0x00,0xFF},{114,0xC0,0xFF},{115,0x00,0xFF},{116,0x80,0xFF},{117,0x00,0xFF},{118,0xC0,0xFF},{119,0x00,0xFF},{120,0x00,0xFF},{121,0x00,0xFF},{122,0x00,0xFF},{123,0x00,0xFF},{124,0x00,0xFF},{125,0x00,0xFF},{126,0x00,0xFF},{127,0x00,0xFF},
{128,0x00,0xFF},{129,0x00,0x0F},{130,0x00,0x0F},{131,0x00,0xFF},{132,0x00,0xFF},{133,0x00,0xFF},{134,0x00,0xFF},{135,0x00,0xFF},{136,0x00,0xFF},{137,0x00,0xFF},{138,0x00,0xFF},{139,0x00,0xFF},{140,0x00,0xFF},{141,0x00,0xFF},{142,0x00,0xFF},{143,0x00,0xFF},
{144,0x00,0xFF},{145,0x00,0x00},{146,0xFF,0x00},{147,0x00,0x00},{148,0x00,0x00},{149,0x00,0x00},{150,0x00,0x00},{151,0x00,0x00},{152,0x00,0xFF},{153,0x00,0xFF},{154,0x00,0xFF},{155,0x00,0xFF},{156,0x00,0xFF},{157,0x00,0xFF},{158,0x00,0x0F},{159,0x00,0x0F},
{160,0x00,0xFF},{161,0x00,0xFF},{162,0x00,0xFF},{163,0x00,0xFF},{164,0x00,0xFF},{165,0x00,0xFF},{166,0x00,0xFF},{167,0x00,0xFF},{168,0x00,0xFF},{169,0x00,0xFF},{170,0x00,0xFF},{171,0x00,0xFF},{172,0x00,0xFF},{173,0x00,0xFF},{174,0x00,0xFF},{175,0x00,0xFF},
{176,0x00,0xFF},{177,0x00,0xFF},{178,0x00,0xFF},{179,0x00,0xFF},{180,0x00,0xFF},{181,0x00,0x0F},{182,0x00,0xFF},{183,0x00,0xFF},{184,0x00,0xFF},{185,0x00,0xFF},{186,0x00,0xFF},{187,0x00,0xFF},{188,0x00,0xFF},{189,0x00,0xFF},{190,0x00,0xFF},{191,0x00,0xFF},
{192,0x00,0xFF},{193,0x00,0xFF},{194,0x00,0xFF},{195,0x00,0xFF},{196,0x00,0xFF},{197,0x00,0xFF},{198,0x00,0xFF},{199,0x00,0xFF},{200,0x00,0xFF},{201,0x00,0xFF},{202,0x00,0xFF},{203,0x00,0x0F},{204,0x00,0xFF},{205,0x00,0xFF},{206,0x00,0xFF},{207,0x00,0xFF},
{208,0x00,0xFF},{209,0x00,0xFF},{210,0x00,0xFF},{211,0x00,0xFF},{212,0x00,0xFF},{213,0x00,0xFF},{214,0x00,0xFF},{215,0x00,0xFF},{216,0x00,0xFF},{217,0x00,0xFF},{218,0x00,0x00},{219,0x00,0x00},{220,0x00,0x00},{221,0x0D,0x00},{222,0x00,0x00},{223,0x00,0x00},
{224,0xF4,0x00},{225,0xF0,0x00},{226,0x00,0x00},{227,0x00,0x00},{228,0x00,0x00},{229,0x00,0x00},{231,0x00,0x00},{232,0x00,0x00},{233,0x00,0x00},{234,0x00,0x00},{235,0x00,0x00},{236,0x00,0x00},{237,0x00,0x00},{238,0x14,0x00},{239,0x00,0x00},{240,0x00,0x00},
{242,0x00,0x02},{243,0xF0,0x00},{244,0x00,0x00},{245,0x00,0x00},{247,0x00,0x00},{248,0x00,0x00},{249,0xA8,0x00},{250,0x00,0x00},{251,0x84,0x00},{252,0x00,0x00},{253,0x00,0x00},{254,0x00,0x00},{255, 1, 0xFF}, // set page bit to 1 
{  0,0x00,0x00},{  1,0x00,0x00},{  2,0x00,0x00},{  3,0x00,0x00},{  4,0x00,0x00},{  5,0x00,0x00},{  6,0x00,0x00},{  7,0x00,0x00},{  8,0x00,0x00},{  9,0x00,0x00},{ 10,0x00,0x00},{ 11,0x00,0x00},{ 12,0x00,0x00},{ 13,0x00,0x00},{ 14,0x00,0x00},{ 15,0x00,0x00},
{ 16,0x00,0x00},{ 17,0x01,0x00},{ 18,0x00,0x00},{ 19,0x00,0x00},{ 20,0x90,0x00},{ 21,0x31,0x00},{ 22,0x00,0x00},{ 23,0x00,0x00},{ 24,0x01,0x00},{ 25,0x00,0x00},{ 26,0x00,0x00},{ 27,0x00,0x00},{ 28,0x00,0x00},{ 29,0x00,0x00},{ 30,0x00,0x00},{ 31,0x00,0xFF},
{ 32,0x00,0xFF},{ 33,0x01,0xFF},{ 34,0x00,0xFF},{ 35,0x00,0xFF},{ 36,0x90,0xFF},{ 37,0x31,0xFF},{ 38,0x00,0xFF},{ 39,0x00,0xFF},{ 40,0x01,0xFF},{ 41,0x00,0xFF},{ 42,0x00,0xFF},{ 43,0x00,0x0F},{ 44,0x00,0x00},{ 45,0x00,0x00},{ 46,0x00,0x00},{ 47,0x00,0xFF},
{ 48,0x00,0xFF},{ 49,0x01,0xFF},{ 50,0x00,0xFF},{ 51,0x00,0xFF},{ 52,0x90,0xFF},{ 53,0x31,0xFF},{ 54,0x00,0xFF},{ 55,0x00,0xFF},{ 56,0x01,0xFF},{ 57,0x00,0xFF},{ 58,0x00,0xFF},{ 59,0x00,0x0F},{ 60,0x00,0x00},{ 61,0x00,0x00},{ 62,0x00,0x00},{ 63,0x00,0xFF},
{ 64,0x00,0xFF},{ 65,0x01,0xFF},{ 66,0x00,0xFF},{ 67,0x00,0xFF},{ 68,0x90,0xFF},{ 69,0x31,0xFF},{ 70,0x00,0xFF},{ 71,0x00,0xFF},{ 72,0x01,0xFF},{ 73,0x00,0xFF},{ 74,0x00,0xFF},{ 75,0x00,0x0F},{ 76,0x00,0x00},{ 77,0x00,0x00},{ 78,0x00,0x00},{ 79,0x00,0xFF},
{ 80,0x00,0xFF},{ 81,0x00,0xFF},{ 82,0x00,0xFF},{ 83,0x00,0xFF},{ 84,0x90,0xFF},{ 85,0x31,0xFF},{ 86,0x00,0xFF},{ 87,0x00,0xFF},{ 88,0x01,0xFF},{ 89,0x00,0xFF},{ 90,0x00,0xFF},{ 91,0x00,0x0F},{ 92,0x00,0x00},{ 93,0x00,0x00},{ 94,0x00,0x00},{255, 0, 0xFF} }; // set page bit to 0
static Reg_Data Reg_extClk[NUM_REGS_MAX] = {
{  0,0x00,0x00},{  1,0x00,0x00},{  2,0x00,0x00},{  3,0x00,0x00},{  4,0x00,0x00},{  5,0x00,0x00},{  6,0x08,0x1D},{  7,0x00,0x00},{  8,0x70,0x00},{  9,0x0F,0x00},{ 10,0x00,0x00},{ 11,0x00,0x00},{ 12,0x00,0x00},{ 13,0x00,0x00},{ 14,0x00,0x00},{ 15,0x00,0x00},
{ 16,0x00,0x00},{ 17,0x00,0x00},{ 18,0x00,0x00},{ 19,0x00,0x00},{ 20,0x00,0x00},{ 21,0x00,0x00},{ 22,0x00,0x00},{ 23,0x00,0x00},{ 24,0x00,0x00},{ 25,0x00,0x00},{ 26,0x00,0x00},{ 27,0x70,0x80},{ 28,0x03,0xFF},{ 29,0x42,0xFF},{ 30,0xB0,0xFF},{ 31,0xC0,0xFF},
{ 32,0xC0,0xFF},{ 33,0xC0,0xFF},{ 34,0xC0,0xFF},{ 35,0xAA,0xFF},{ 36,0x06,0x1F},{ 37,0x06,0x1F},{ 38,0x06,0x1F},{ 39,0x06,0x1F},{ 40,0x84,0xFF},{ 41,0x10,0x7F},{ 42,0x24,0x3F},{ 43,0x00,0x00},{ 44,0x00,0x00},{ 45,0x00,0xFF},{ 46,0x00,0xFF},{ 47,0x14,0x3F},
{ 48,0x4B,0xFF},{ 49,0x00,0xFF},{ 50,0xC4,0xFF},{ 51,0x07,0xFF},{ 52,0x10,0xFF},{ 53,0x00,0xFF},{ 54,0x0D,0xFF},{ 55,0x00,0xFF},{ 56,0x00,0xFF},{ 57,0x00,0xFF},{ 58,0x00,0xFF},{ 59,0x01,0xFF},{ 60,0x00,0xFF},{ 61,0x00,0xFF},{ 62,0x00,0x3F},{ 63,0x10,0xFF},
{ 64,0x00,0xFF},{ 65,0x0D,0xFF},{ 66,0x00,0xFF},{ 67,0x00,0xFF},{ 68,0x00,0xFF},{ 69,0x00,0xFF},{ 70,0x01,0xFF},{ 71,0x00,0xFF},{ 72,0x00,0xFF},{ 73,0x00,0x3F},{ 74,0x10,0xFF},{ 75,0x00,0xFF},{ 76,0x0D,0xFF},{ 77,0x00,0xFF},{ 78,0x00,0xFF},{ 79,0x00,0xFF},
{ 80,0x00,0xFF},{ 81,0x01,0xFF},{ 82,0x00,0xFF},{ 83,0x00,0xFF},{ 84,0x00,0x3F},{ 85,0x10,0xFF},{ 86,0x00,0xFF},{ 87,0x0D,0xFF},{ 88,0x00,0xFF},{ 89,0x00,0xFF},{ 90,0x00,0xFF},{ 91,0x00,0xFF},{ 92,0x01,0xFF},{ 93,0x00,0xFF},{ 94,0x00,0xFF},{ 95,0x00,0x3F},
{ 96,0x10,0x00},{ 97,0x00,0xFF},{ 98,0x3A,0xFF},{ 99,0x00,0xFF},{100,0x00,0xFF},{101,0x00,0xFF},{102,0x00,0xFF},{103,0x01,0xFF},{104,0x00,0xFF},{105,0x00,0xFF},{106,0x80,0xBF},{107,0x00,0xFF},{108,0x00,0xFF},{109,0x00,0xFF},{110,0x00,0xFF},{111,0x00,0xFF},
{112,0x00,0xFF},{113,0x00,0xFF},{114,0xC0,0xFF},{115,0x00,0xFF},{116,0x80,0xFF},{117,0x00,0xFF},{118,0xC0,0xFF},{119,0x00,0xFF},{120,0x00,0xFF},{121,0x00,0xFF},{122,0x00,0xFF},{123,0x00,0xFF},{124,0x00,0xFF},{125,0x00,0xFF},{126,0x00,0xFF},{127,0x00,0xFF},
{128,0x00,0xFF},{129,0x00,0x0F},{130,0x00,0x0F},{131,0x00,0xFF},{132,0x00,0xFF},{133,0x00,0xFF},{134,0x00,0xFF},{135,0x00,0xFF},{136,0x00,0xFF},{137,0x00,0xFF},{138,0x00,0xFF},{139,0x00,0xFF},{140,0x00,0xFF},{141,0x00,0xFF},{142,0x00,0xFF},{143,0x00,0xFF},
{144,0x00,0xFF},{145,0x00,0x00},{146,0xFF,0x00},{147,0x00,0x00},{148,0x00,0x00},{149,0x00,0x00},{150,0x00,0x00},{151,0x00,0x00},{152,0x00,0xFF},{153,0x00,0xFF},{154,0x00,0xFF},{155,0x00,0xFF},{156,0x00,0xFF},{157,0x00,0xFF},{158,0x00,0x0F},{159,0x00,0x0F},
{160,0x00,0xFF},{161,0x00,0xFF},{162,0x00,0xFF},{163,0x00,0xFF},{164,0x00,0xFF},{165,0x00,0xFF},{166,0x00,0xFF},{167,0x00,0xFF},{168,0x00,0xFF},{169,0x00,0xFF},{170,0x00,0xFF},{171,0x00,0xFF},{172,0x00,0xFF},{173,0x00,0xFF},{174,0x00,0xFF},{175,0x00,0xFF},
{176,0x00,0xFF},{177,0x00,0xFF},{178,0x00,0xFF},{179,0x00,0xFF},{180,0x00,0xFF},{181,0x00,0x0F},{182,0x00,0xFF},{183,0x00,0xFF},{184,0x00,0xFF},{185,0x00,0xFF},{186,0x00,0xFF},{187,0x00,0xFF},{188,0x00,0xFF},{189,0x00,0xFF},{190,0x00,0xFF},{191,0x00,0xFF},
{192,0x00,0xFF},{193,0x00,0xFF},{194,0x00,0xFF},{195,0x00,0xFF},{196,0x00,0xFF},{197,0x00,0xFF},{198,0x00,0xFF},{199,0x00,0xFF},{200,0x00,0xFF},{201,0x00,0xFF},{202,0x00,0xFF},{203,0x00,0x0F},{204,0x00,0xFF},{205,0x00,0xFF},{206,0x00,0xFF},{207,0x00,0xFF},
{208,0x00,0xFF},{209,0x00,0xFF},{210,0x00,0xFF},{211,0x00,0xFF},{212,0x00,0xFF},{213,0x00,0xFF},{214,0x00,0xFF},{215,0x00,0xFF},{216,0x00,0xFF},{217,0x00,0xFF},{218,0x00,0x00},{219,0x00,0x00},{220,0x00,0x00},{221,0x0D,0x00},{222,0x00,0x00},{223,0x00,0x00},
{224,0xF4,0x00},{225,0xF0,0x00},{226,0x00,0x00},{227,0x00,0x00},{228,0x00,0x00},{229,0x00,0x00},{231,0x00,0x00},{232,0x00,0x00},{233,0x00,0x00},{234,0x00,0x00},{235,0x00,0x00},{236,0x00,0x00},{237,0x00,0x00},{238,0x14,0x00},{239,0x00,0x00},{240,0x00,0x00},
{242,0x00,0x02},{243,0xF0,0x00},{244,0x00,0x00},{245,0x00,0x00},{247,0x00,0x00},{248,0x00,0x00},{249,0xA8,0x00},{250,0x00,0x00},{251,0x84,0x00},{252,0x00,0x00},{253,0x00,0x00},{254,0x00,0x00},{255, 1, 0xFF}, // set page bit to 1 
{  0,0x00,0x00},{  1,0x00,0x00},{  2,0x00,0x00},{  3,0x00,0x00},{  4,0x00,0x00},{  5,0x00,0x00},{  6,0x00,0x00},{  7,0x00,0x00},{  8,0x00,0x00},{  9,0x00,0x00},{ 10,0x00,0x00},{ 11,0x00,0x00},{ 12,0x00,0x00},{ 13,0x00,0x00},{ 14,0x00,0x00},{ 15,0x00,0x00},
{ 16,0x00,0x00},{ 17,0x01,0x00},{ 18,0x00,0x00},{ 19,0x00,0x00},{ 20,0x90,0x00},{ 21,0x31,0x00},{ 22,0x00,0x00},{ 23,0x00,0x00},{ 24,0x01,0x00},{ 25,0x00,0x00},{ 26,0x00,0x00},{ 27,0x00,0x00},{ 28,0x00,0x00},{ 29,0x00,0x00},{ 30,0x00,0x00},{ 31,0x00,0xFF},
{ 32,0x00,0xFF},{ 33,0x01,0xFF},{ 34,0x00,0xFF},{ 35,0x00,0xFF},{ 36,0x90,0xFF},{ 37,0x31,0xFF},{ 38,0x00,0xFF},{ 39,0x00,0xFF},{ 40,0x01,0xFF},{ 41,0x00,0xFF},{ 42,0x00,0xFF},{ 43,0x00,0x0F},{ 44,0x00,0x00},{ 45,0x00,0x00},{ 46,0x00,0x00},{ 47,0x00,0xFF},
{ 48,0x00,0xFF},{ 49,0x01,0xFF},{ 50,0x00,0xFF},{ 51,0x00,0xFF},{ 52,0x90,0xFF},{ 53,0x31,0xFF},{ 54,0x00,0xFF},{ 55,0x00,0xFF},{ 56,0x01,0xFF},{ 57,0x00,0xFF},{ 58,0x00,0xFF},{ 59,0x00,0x0F},{ 60,0x00,0x00},{ 61,0x00,0x00},{ 62,0x00,0x00},{ 63,0x00,0xFF},
{ 64,0x00,0xFF},{ 65,0x01,0xFF},{ 66,0x00,0xFF},{ 67,0x00,0xFF},{ 68,0x90,0xFF},{ 69,0x31,0xFF},{ 70,0x00,0xFF},{ 71,0x00,0xFF},{ 72,0x01,0xFF},{ 73,0x00,0xFF},{ 74,0x00,0xFF},{ 75,0x00,0x0F},{ 76,0x00,0x00},{ 77,0x00,0x00},{ 78,0x00,0x00},{ 79,0x00,0xFF},
{ 80,0x00,0xFF},{ 81,0x00,0xFF},{ 82,0x00,0xFF},{ 83,0x00,0xFF},{ 84,0x90,0xFF},{ 85,0x31,0xFF},{ 86,0x00,0xFF},{ 87,0x00,0xFF},{ 88,0x01,0xFF},{ 89,0x00,0xFF},{ 90,0x00,0xFF},{ 91,0x00,0x0F},{ 92,0x00,0x00},{ 93,0x00,0x00},{ 94,0x00,0x00},{255, 0, 0xFF} }; // set page bit to 0
#endif

#ifdef REF_125_MHZ
static Reg_Data Reg_ob25MHz[NUM_REGS_MAX] = {
{  0,0x00,0x00},{  1,0x00,0x00},{  2,0x00,0x00},{  3,0x00,0x00},{  4,0x00,0x00},{  5,0x00,0x00},{  6,0x08,0x1D},{  7,0x00,0x00},{  8,0x70,0x00},{  9,0x0F,0x00},{ 10,0x00,0x00},{ 11,0x00,0x00},{ 12,0x00,0x00},{ 13,0x00,0x00},{ 14,0x00,0x00},{ 15,0x00,0x00},
{ 16,0x00,0x00},{ 17,0x00,0x00},{ 18,0x00,0x00},{ 19,0x00,0x00},{ 20,0x00,0x00},{ 21,0x00,0x00},{ 22,0x00,0x00},{ 23,0x00,0x00},{ 24,0x00,0x00},{ 25,0x00,0x00},{ 26,0x00,0x00},{ 27,0x70,0x80},{ 28,0x0B,0xFF},{ 29,0x08,0xFF},{ 30,0xB0,0xFF},{ 31,0xE3,0xFF},
{ 32,0xC0,0xFF},{ 33,0xC0,0xFF},{ 34,0xE3,0xFF},{ 35,0xAA,0xFF},{ 36,0x00,0x1F},{ 37,0x06,0x1F},{ 38,0x06,0x1F},{ 39,0x00,0x1F},{ 40,0x84,0xFF},{ 41,0x10,0x7F},{ 42,0x24,0x3F},{ 43,0x00,0x00},{ 44,0x00,0x00},{ 45,0x00,0xFF},{ 46,0x00,0xFF},{ 47,0x14,0x3F},
{ 48,0x3A,0xFF},{ 49,0x00,0xFF},{ 50,0xC4,0xFF},{ 51,0x07,0xFF},{ 52,0x10,0xFF},{ 53,0x00,0xFF},{ 54,0x00,0xFF},{ 55,0x00,0xFF},{ 56,0x00,0xFF},{ 57,0x00,0xFF},{ 58,0x00,0xFF},{ 59,0x00,0xFF},{ 60,0x00,0xFF},{ 61,0x00,0xFF},{ 62,0x00,0x3F},{ 63,0x10,0xFF},
{ 64,0x00,0xFF},{ 65,0x08,0xFF},{ 66,0x00,0xFF},{ 67,0x00,0xFF},{ 68,0x00,0xFF},{ 69,0x00,0xFF},{ 70,0x01,0xFF},{ 71,0x00,0xFF},{ 72,0x00,0xFF},{ 73,0x00,0x3F},{ 74,0x10,0xFF},{ 75,0x00,0xFF},{ 76,0x08,0xFF},{ 77,0x00,0xFF},{ 78,0x00,0xFF},{ 79,0x00,0xFF},
{ 80,0x00,0xFF},{ 81,0x01,0xFF},{ 82,0x00,0xFF},{ 83,0x00,0xFF},{ 84,0x00,0x3F},{ 85,0x10,0xFF},{ 86,0x00,0xFF},{ 87,0x00,0xFF},{ 88,0x00,0xFF},{ 89,0x00,0xFF},{ 90,0x00,0xFF},{ 91,0x00,0xFF},{ 92,0x00,0xFF},{ 93,0x00,0xFF},{ 94,0x00,0xFF},{ 95,0x00,0x3F},
{ 96,0x10,0x00},{ 97,0x00,0xFF},{ 98,0x30,0xFF},{ 99,0x00,0xFF},{100,0x00,0xFF},{101,0x00,0xFF},{102,0x00,0xFF},{103,0x01,0xFF},{104,0x00,0xFF},{105,0x00,0xFF},{106,0x80,0xBF},{107,0x00,0xFF},{108,0x00,0xFF},{109,0x00,0xFF},{110,0x00,0xFF},{111,0x00,0xFF},
{112,0x00,0xFF},{113,0x00,0xFF},{114,0xC0,0xFF},{115,0x00,0xFF},{116,0x80,0xFF},{117,0x00,0xFF},{118,0xC0,0xFF},{119,0x00,0xFF},{120,0x00,0xFF},{121,0x00,0xFF},{122,0x00,0xFF},{123,0x00,0xFF},{124,0x00,0xFF},{125,0x00,0xFF},{126,0x00,0xFF},{127,0x00,0xFF},
{128,0x00,0xFF},{129,0x00,0x0F},{130,0x00,0x0F},{131,0x00,0xFF},{132,0x00,0xFF},{133,0x00,0xFF},{134,0x00,0xFF},{135,0x00,0xFF},{136,0x00,0xFF},{137,0x00,0xFF},{138,0x00,0xFF},{139,0x00,0xFF},{140,0x00,0xFF},{141,0x00,0xFF},{142,0x00,0xFF},{143,0x00,0xFF},
{144,0x00,0xFF},{145,0x00,0x00},{146,0xFF,0x00},{147,0x00,0x00},{148,0x00,0x00},{149,0x00,0x00},{150,0x00,0x00},{151,0x00,0x00},{152,0x00,0xFF},{153,0x00,0xFF},{154,0x00,0xFF},{155,0x00,0xFF},{156,0x00,0xFF},{157,0x00,0xFF},{158,0x00,0x0F},{159,0x00,0x0F},
{160,0x00,0xFF},{161,0x00,0xFF},{162,0x00,0xFF},{163,0x00,0xFF},{164,0x00,0xFF},{165,0x00,0xFF},{166,0x00,0xFF},{167,0x00,0xFF},{168,0x00,0xFF},{169,0x00,0xFF},{170,0x00,0xFF},{171,0x00,0xFF},{172,0x00,0xFF},{173,0x00,0xFF},{174,0x00,0xFF},{175,0x00,0xFF},
{176,0x00,0xFF},{177,0x00,0xFF},{178,0x00,0xFF},{179,0x00,0xFF},{180,0x00,0xFF},{181,0x00,0x0F},{182,0x00,0xFF},{183,0x00,0xFF},{184,0x00,0xFF},{185,0x00,0xFF},{186,0x00,0xFF},{187,0x00,0xFF},{188,0x00,0xFF},{189,0x00,0xFF},{190,0x00,0xFF},{191,0x00,0xFF},
{192,0x00,0xFF},{193,0x00,0xFF},{194,0x00,0xFF},{195,0x00,0xFF},{196,0x00,0xFF},{197,0x00,0xFF},{198,0x00,0xFF},{199,0x00,0xFF},{200,0x00,0xFF},{201,0x00,0xFF},{202,0x00,0xFF},{203,0x00,0x0F},{204,0x00,0xFF},{205,0x00,0xFF},{206,0x00,0xFF},{207,0x00,0xFF},
{208,0x00,0xFF},{209,0x00,0xFF},{210,0x00,0xFF},{211,0x00,0xFF},{212,0x00,0xFF},{213,0x00,0xFF},{214,0x00,0xFF},{215,0x00,0xFF},{216,0x00,0xFF},{217,0x00,0xFF},{218,0x00,0x00},{219,0x00,0x00},{220,0x00,0x00},{221,0x0D,0x00},{222,0x00,0x00},{223,0x00,0x00},
{224,0xF4,0x00},{225,0xF0,0x00},{226,0x00,0x00},{227,0x00,0x00},{228,0x00,0x00},{229,0x00,0x00},{231,0x00,0x00},{232,0x00,0x00},{233,0x00,0x00},{234,0x00,0x00},{235,0x00,0x00},{236,0x00,0x00},{237,0x00,0x00},{238,0x14,0x00},{239,0x00,0x00},{240,0x00,0x00},
{242,0x00,0x02},{243,0xF0,0x00},{244,0x00,0x00},{245,0x00,0x00},{247,0x00,0x00},{248,0x00,0x00},{249,0xA8,0x00},{250,0x00,0x00},{251,0x84,0x00},{252,0x00,0x00},{253,0x00,0x00},{254,0x00,0x00},{255, 1, 0xFF}, // set page bit to 1 
{  0,0x00,0x00},{  1,0x00,0x00},{  2,0x00,0x00},{  3,0x00,0x00},{  4,0x00,0x00},{  5,0x00,0x00},{  6,0x00,0x00},{  7,0x00,0x00},{  8,0x00,0x00},{  9,0x00,0x00},{ 10,0x00,0x00},{ 11,0x00,0x00},{ 12,0x00,0x00},{ 13,0x00,0x00},{ 14,0x00,0x00},{ 15,0x00,0x00},
{ 16,0x00,0x00},{ 17,0x01,0x00},{ 18,0x00,0x00},{ 19,0x00,0x00},{ 20,0x90,0x00},{ 21,0x31,0x00},{ 22,0x00,0x00},{ 23,0x00,0x00},{ 24,0x01,0x00},{ 25,0x00,0x00},{ 26,0x00,0x00},{ 27,0x00,0x00},{ 28,0x00,0x00},{ 29,0x00,0x00},{ 30,0x00,0x00},{ 31,0x00,0xFF},
{ 32,0x00,0xFF},{ 33,0x01,0xFF},{ 34,0x00,0xFF},{ 35,0x00,0xFF},{ 36,0x90,0xFF},{ 37,0x31,0xFF},{ 38,0x00,0xFF},{ 39,0x00,0xFF},{ 40,0x01,0xFF},{ 41,0x00,0xFF},{ 42,0x00,0xFF},{ 43,0x00,0x0F},{ 44,0x00,0x00},{ 45,0x00,0x00},{ 46,0x00,0x00},{ 47,0x00,0xFF},
{ 48,0x00,0xFF},{ 49,0x01,0xFF},{ 50,0x00,0xFF},{ 51,0x00,0xFF},{ 52,0x90,0xFF},{ 53,0x31,0xFF},{ 54,0x00,0xFF},{ 55,0x00,0xFF},{ 56,0x01,0xFF},{ 57,0x00,0xFF},{ 58,0x00,0xFF},{ 59,0x00,0x0F},{ 60,0x00,0x00},{ 61,0x00,0x00},{ 62,0x00,0x00},{ 63,0x00,0xFF},
{ 64,0x00,0xFF},{ 65,0x01,0xFF},{ 66,0x00,0xFF},{ 67,0x00,0xFF},{ 68,0x90,0xFF},{ 69,0x31,0xFF},{ 70,0x00,0xFF},{ 71,0x00,0xFF},{ 72,0x01,0xFF},{ 73,0x00,0xFF},{ 74,0x00,0xFF},{ 75,0x00,0x0F},{ 76,0x00,0x00},{ 77,0x00,0x00},{ 78,0x00,0x00},{ 79,0x00,0xFF},
{ 80,0x00,0xFF},{ 81,0x00,0xFF},{ 82,0x00,0xFF},{ 83,0x00,0xFF},{ 84,0x90,0xFF},{ 85,0x31,0xFF},{ 86,0x00,0xFF},{ 87,0x00,0xFF},{ 88,0x01,0xFF},{ 89,0x00,0xFF},{ 90,0x00,0xFF},{ 91,0x00,0x0F},{ 92,0x00,0x00},{ 93,0x00,0x00},{ 94,0x00,0x00},{255, 0, 0xFF} }; // set page bit to 0
//End of file

static Reg_Data Reg_extClk[NUM_REGS_MAX] = {
{  0,0x00,0x00},{  1,0x00,0x00},{  2,0x00,0x00},{  3,0x00,0x00},{  4,0x00,0x00},{  5,0x00,0x00},{  6,0x08,0x1D},{  7,0x00,0x00},{  8,0x70,0x00},{  9,0x0F,0x00},{ 10,0x00,0x00},{ 11,0x00,0x00},{ 12,0x00,0x00},{ 13,0x00,0x00},{ 14,0x00,0x00},{ 15,0x00,0x00},
{ 16,0x00,0x00},{ 17,0x00,0x00},{ 18,0x00,0x00},{ 19,0x00,0x00},{ 20,0x00,0x00},{ 21,0x00,0x00},{ 22,0x00,0x00},{ 23,0x00,0x00},{ 24,0x00,0x00},{ 25,0x00,0x00},{ 26,0x00,0x00},{ 27,0x70,0x80},{ 28,0x03,0xFF},{ 29,0x42,0xFF},{ 30,0xB0,0xFF},{ 31,0xE3,0xFF},
{ 32,0xC0,0xFF},{ 33,0xC0,0xFF},{ 34,0xE3,0xFF},{ 35,0xAA,0xFF},{ 36,0x00,0x1F},{ 37,0x06,0x1F},{ 38,0x06,0x1F},{ 39,0x00,0x1F},{ 40,0x84,0xFF},{ 41,0x10,0x7F},{ 42,0x24,0x3F},{ 43,0x00,0x00},{ 44,0x00,0x00},{ 45,0x00,0xFF},{ 46,0x00,0xFF},{ 47,0x14,0x3F},
{ 48,0x2E,0xFF},{ 49,0x00,0xFF},{ 50,0xC4,0xFF},{ 51,0x07,0xFF},{ 52,0x10,0xFF},{ 53,0x00,0xFF},{ 54,0x00,0xFF},{ 55,0x00,0xFF},{ 56,0x00,0xFF},{ 57,0x00,0xFF},{ 58,0x00,0xFF},{ 59,0x00,0xFF},{ 60,0x00,0xFF},{ 61,0x00,0xFF},{ 62,0x00,0x3F},{ 63,0x10,0xFF},
{ 64,0x00,0xFF},{ 65,0x08,0xFF},{ 66,0x00,0xFF},{ 67,0x00,0xFF},{ 68,0x00,0xFF},{ 69,0x00,0xFF},{ 70,0x01,0xFF},{ 71,0x00,0xFF},{ 72,0x00,0xFF},{ 73,0x00,0x3F},{ 74,0x10,0xFF},{ 75,0x00,0xFF},{ 76,0x08,0xFF},{ 77,0x00,0xFF},{ 78,0x00,0xFF},{ 79,0x00,0xFF},
{ 80,0x00,0xFF},{ 81,0x01,0xFF},{ 82,0x00,0xFF},{ 83,0x00,0xFF},{ 84,0x00,0x3F},{ 85,0x10,0xFF},{ 86,0x00,0xFF},{ 87,0x00,0xFF},{ 88,0x00,0xFF},{ 89,0x00,0xFF},{ 90,0x00,0xFF},{ 91,0x00,0xFF},{ 92,0x00,0xFF},{ 93,0x00,0xFF},{ 94,0x00,0xFF},{ 95,0x00,0x3F},
{ 96,0x10,0x00},{ 97,0x00,0xFF},{ 98,0x26,0xFF},{ 99,0x00,0xFF},{100,0x00,0xFF},{101,0x00,0xFF},{102,0x00,0xFF},{103,0x01,0xFF},{104,0x00,0xFF},{105,0x00,0xFF},{106,0x80,0xBF},{107,0x00,0xFF},{108,0x00,0xFF},{109,0x00,0xFF},{110,0x00,0xFF},{111,0x00,0xFF},
{112,0x00,0xFF},{113,0x00,0xFF},{114,0xC0,0xFF},{115,0x00,0xFF},{116,0x80,0xFF},{117,0x00,0xFF},{118,0xC0,0xFF},{119,0x00,0xFF},{120,0x00,0xFF},{121,0x00,0xFF},{122,0x00,0xFF},{123,0x00,0xFF},{124,0x00,0xFF},{125,0x00,0xFF},{126,0x00,0xFF},{127,0x00,0xFF},
{128,0x00,0xFF},{129,0x00,0x0F},{130,0x00,0x0F},{131,0x00,0xFF},{132,0x00,0xFF},{133,0x00,0xFF},{134,0x00,0xFF},{135,0x00,0xFF},{136,0x00,0xFF},{137,0x00,0xFF},{138,0x00,0xFF},{139,0x00,0xFF},{140,0x00,0xFF},{141,0x00,0xFF},{142,0x00,0xFF},{143,0x00,0xFF},
{144,0x00,0xFF},{145,0x00,0x00},{146,0xFF,0x00},{147,0x00,0x00},{148,0x00,0x00},{149,0x00,0x00},{150,0x00,0x00},{151,0x00,0x00},{152,0x00,0xFF},{153,0x00,0xFF},{154,0x00,0xFF},{155,0x00,0xFF},{156,0x00,0xFF},{157,0x00,0xFF},{158,0x00,0x0F},{159,0x00,0x0F},
{160,0x00,0xFF},{161,0x00,0xFF},{162,0x00,0xFF},{163,0x00,0xFF},{164,0x00,0xFF},{165,0x00,0xFF},{166,0x00,0xFF},{167,0x00,0xFF},{168,0x00,0xFF},{169,0x00,0xFF},{170,0x00,0xFF},{171,0x00,0xFF},{172,0x00,0xFF},{173,0x00,0xFF},{174,0x00,0xFF},{175,0x00,0xFF},
{176,0x00,0xFF},{177,0x00,0xFF},{178,0x00,0xFF},{179,0x00,0xFF},{180,0x00,0xFF},{181,0x00,0x0F},{182,0x00,0xFF},{183,0x00,0xFF},{184,0x00,0xFF},{185,0x00,0xFF},{186,0x00,0xFF},{187,0x00,0xFF},{188,0x00,0xFF},{189,0x00,0xFF},{190,0x00,0xFF},{191,0x00,0xFF},
{192,0x00,0xFF},{193,0x00,0xFF},{194,0x00,0xFF},{195,0x00,0xFF},{196,0x00,0xFF},{197,0x00,0xFF},{198,0x00,0xFF},{199,0x00,0xFF},{200,0x00,0xFF},{201,0x00,0xFF},{202,0x00,0xFF},{203,0x00,0x0F},{204,0x00,0xFF},{205,0x00,0xFF},{206,0x00,0xFF},{207,0x00,0xFF},
{208,0x00,0xFF},{209,0x00,0xFF},{210,0x00,0xFF},{211,0x00,0xFF},{212,0x00,0xFF},{213,0x00,0xFF},{214,0x00,0xFF},{215,0x00,0xFF},{216,0x00,0xFF},{217,0x00,0xFF},{218,0x00,0x00},{219,0x00,0x00},{220,0x00,0x00},{221,0x0D,0x00},{222,0x00,0x00},{223,0x00,0x00},
{224,0xF4,0x00},{225,0xF0,0x00},{226,0x00,0x00},{227,0x00,0x00},{228,0x00,0x00},{229,0x00,0x00},{231,0x00,0x00},{232,0x00,0x00},{233,0x00,0x00},{234,0x00,0x00},{235,0x00,0x00},{236,0x00,0x00},{237,0x00,0x00},{238,0x14,0x00},{239,0x00,0x00},{240,0x00,0x00},
{242,0x00,0x02},{243,0xF0,0x00},{244,0x00,0x00},{245,0x00,0x00},{247,0x00,0x00},{248,0x00,0x00},{249,0xA8,0x00},{250,0x00,0x00},{251,0x84,0x00},{252,0x00,0x00},{253,0x00,0x00},{254,0x00,0x00},{255, 1, 0xFF}, // set page bit to 1 
{  0,0x00,0x00},{  1,0x00,0x00},{  2,0x00,0x00},{  3,0x00,0x00},{  4,0x00,0x00},{  5,0x00,0x00},{  6,0x00,0x00},{  7,0x00,0x00},{  8,0x00,0x00},{  9,0x00,0x00},{ 10,0x00,0x00},{ 11,0x00,0x00},{ 12,0x00,0x00},{ 13,0x00,0x00},{ 14,0x00,0x00},{ 15,0x00,0x00},
{ 16,0x00,0x00},{ 17,0x01,0x00},{ 18,0x00,0x00},{ 19,0x00,0x00},{ 20,0x90,0x00},{ 21,0x31,0x00},{ 22,0x00,0x00},{ 23,0x00,0x00},{ 24,0x01,0x00},{ 25,0x00,0x00},{ 26,0x00,0x00},{ 27,0x00,0x00},{ 28,0x00,0x00},{ 29,0x00,0x00},{ 30,0x00,0x00},{ 31,0x00,0xFF},
{ 32,0x00,0xFF},{ 33,0x01,0xFF},{ 34,0x00,0xFF},{ 35,0x00,0xFF},{ 36,0x90,0xFF},{ 37,0x31,0xFF},{ 38,0x00,0xFF},{ 39,0x00,0xFF},{ 40,0x01,0xFF},{ 41,0x00,0xFF},{ 42,0x00,0xFF},{ 43,0x00,0x0F},{ 44,0x00,0x00},{ 45,0x00,0x00},{ 46,0x00,0x00},{ 47,0x00,0xFF},
{ 48,0x00,0xFF},{ 49,0x01,0xFF},{ 50,0x00,0xFF},{ 51,0x00,0xFF},{ 52,0x90,0xFF},{ 53,0x31,0xFF},{ 54,0x00,0xFF},{ 55,0x00,0xFF},{ 56,0x01,0xFF},{ 57,0x00,0xFF},{ 58,0x00,0xFF},{ 59,0x00,0x0F},{ 60,0x00,0x00},{ 61,0x00,0x00},{ 62,0x00,0x00},{ 63,0x00,0xFF},
{ 64,0x00,0xFF},{ 65,0x01,0xFF},{ 66,0x00,0xFF},{ 67,0x00,0xFF},{ 68,0x90,0xFF},{ 69,0x31,0xFF},{ 70,0x00,0xFF},{ 71,0x00,0xFF},{ 72,0x01,0xFF},{ 73,0x00,0xFF},{ 74,0x00,0xFF},{ 75,0x00,0x0F},{ 76,0x00,0x00},{ 77,0x00,0x00},{ 78,0x00,0x00},{ 79,0x00,0xFF},
{ 80,0x00,0xFF},{ 81,0x00,0xFF},{ 82,0x00,0xFF},{ 83,0x00,0xFF},{ 84,0x90,0xFF},{ 85,0x31,0xFF},{ 86,0x00,0xFF},{ 87,0x00,0xFF},{ 88,0x01,0xFF},{ 89,0x00,0xFF},{ 90,0x00,0xFF},{ 91,0x00,0x0F},{ 92,0x00,0x00},{ 93,0x00,0x00},{ 94,0x00,0x00},{255, 0, 0xFF} }; // set page bit to 0
//End of file
#endif



static char iicDrvName[IIC_maxDrv][IIC_maxDrvNameL] ={  {DRV_NAME_iic_bp},  {DRV_NAME_iic_ob},  {DRV_NAME_iic_sl},  {DRV_NAME_iic_sh}};
/* Back plane IIC Slave Information */
static s_ts2iic iicSlvs_bp[IIC_maxSlv_bp] = {
  {"bp_7313", SLV_MAX7313_A, 0x18, 0, 0, 0}        // MAX7313(0x18)
};

/* on board IIC Slave Information */
static s_ts2iic iicSlvs_ob[IIC_maxSlv_ob] = {
  {"ob_5338", SLV_SI5338A, 0x70, 0,            0, 0}, 
  {"ob_570" , SLV_SI570  , 0x5D, 1, IIC_MUX_ADDR, 1},
  {"ob_4604", SLV_ADN4604, 0x4B, 1, IIC_MUX_ADDR, 8},
  {"ob_226A", SLV_INA226A, 0x40, 1, IIC_MUX_ADDR, 4}
};

/* slave board Low floor IIC Slave Information */
static s_ts2iic iicSlvs_sl[IIC_maxSlv_sl] = {
  {"sl_GA_7313", SLV_MAX7313_A, 0x18, 1, IIC_MUX_ADDR ,1},  
  {"sl_IO_7313", SLV_MAX7313_B, 0x19, 1, IIC_MUX_ADDR ,2} ,
  {"sl_112A"   , SLV_TMP112A,   0x48, 1, IIC_MUX_ADDR ,4} ,
  {"sl_4604"   , SLV_ADN4604,   0x4B, 1, IIC_MUX_ADDR ,8}
};

/* slave board High floor IIC Slave Information */
static s_ts2iic iicSlvs_sh[IIC_maxSlv_sh] = {
  {"sh_GA_7313", SLV_MAX7313_A, 0x18, 1, IIC_MUX_ADDR ,1},  
  {"sh_IO_7313", SLV_MAX7313_B, 0x19, 1, IIC_MUX_ADDR ,2} ,
  {"sh_112A"   , SLV_TMP112A,   0x48, 1, IIC_MUX_ADDR ,4} ,
  {"sh_4604"   , SLV_ADN4604,   0x4B, 1, IIC_MUX_ADDR ,8}
};

/* System information */
static s_ts2iic* piicBus[IIC_maxDrv] = {(s_ts2iic*) &iicSlvs_bp,  (s_ts2iic*) &iicSlvs_ob,  (s_ts2iic*) &iicSlvs_sl,  (s_ts2iic*) &iicSlvs_sh};
static int iicSlvNum[IIC_maxDrv] = { IIC_maxSlv_bp,  IIC_maxSlv_ob,  IIC_maxSlv_sl,  IIC_maxSlv_sh};

//==============================================================================
//----===@ class
//==============================================================================


//=====================================
//----===@ ts2ipEvsys
// Parameters  :
// Description :
ts2ipEvsys::ts2ipEvsys(const char *deviceName, int* ts_mode, char* ts_name, int* ts_opMode)
{
  ptsMode   = ts_mode;
  ptsOpMode = ts_opMode;
  ptsName   = ts_name;

  ip_open(deviceName);

  int i;
  for(i = 0; i < IIC_maxDrv; i++){
    /* Open the device. */
    iic_fd[i] = open(iicDrvName[i], O_RDWR);
    
    if(iic_fd[i] < 0){
      prnErr();
      prnM3("[ERR] iic dev open error : %s\r\n",iicDrvName[i]);
    }
    else{
      /* set configuration */
      iic_piic[i]   = piicBus[i];
      iic_isOpen[i] = 1;
      iic_slvNum[i] = iicSlvNum[i];
    }
  }

#ifdef __DEBUG_1__
  iic_prnInfo();
#endif

  struct termios newtio;
  lcd_fd = open( DRV_NAME_lcd, O_RDWR | O_NOCTTY );
  if(lcd_fd<0){
    prnM3("[ERR] lcd dev open error : %s\r\n",DRV_NAME_lcd);
    prnErr();
  }
  else{
    memset( &newtio, 0, sizeof(newtio) );
    newtio.c_cflag = B115200;
    newtio.c_cflag |= CS8;
    newtio.c_cflag |= CLOCAL;
    newtio.c_cflag |= CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 0;

    tcflush(lcd_fd, TCIFLUSH );
    tcsetattr(lcd_fd, TCSANOW, &newtio );
  }

  if(RAON_EVG == *ptsMode)
    init_clk(REF_CLK_Ext);
  else
    init_clk(REF_CLK_ob25MHz);

  ip_wr(accType_devRaw, A_cfgSys_fanHLCntrMax, 5000);

//  taskDelay(1000000);
  taskDelay(1);

  getStat();

  //-- get slave boards ID, sl & sh ID
  ob.busId = IIC_BUS_ob;
  sl.busId = IIC_BUS_sl;
  sh.busId = IIC_BUS_sh;
  
  ob.id = 0;
  sprintf(ob.name,"%s","Onboard I2C");

  get_slaveBoardID(IIC_BUS_sl);
  get_slaveBoardID(IIC_BUS_sh);
  set_slaveIO_byMode();

  if(isMaster == 1){
    prnM2("This is master 1st floor.\r\n");
    init_bp();
  }
  else{
    prnM2("This is slave 2nd floor.\r\n");
  }
  
  init_system();
  init_cps(ob.busId);
  set_cpsByMode();
}

//=====================================
//----===@ ~ts2ipEvsys
// Parameters  :
// Description :
ts2ipEvsys::~ts2ipEvsys()
{
  prnM2("~ts2ipEvsys();\r\n");
}


//=====================================
//----===@ getStat
// Parameters  :
// Description :
int ts2ipEvsys::getStat(void)
{
  ifRet(fd < 0);
  uint rdData;

  //system status
  ip_rd(accType_devRaw, slv_reg12, &rdData);
  isMaster       = (((~rdData) >> 31) & 0x1   );
  PRSNT_M2C_L    = (((~rdData) >> 28) & 0x1)   ;
  f_SFP_LossA    = ((rdData >>  1) & 0x1 )     ;
  f_SFP_LossB    = ((rdData >>  0) & 0x1 )     ;
  f_SFP_prsntA   = (((~rdData) >>  4) & 0x1 )  ;
  f_SFP_prsntB   = (((~rdData) >>  5) & 0x1 )  ;

  ip_rd(accType_devRaw, slv_reg34, &rdData);
  igen_enable    =(rdData>> 1)&0x1; //
  igen_mode_ext  =(rdData>> 0)&0x1; //

  ip_rd(accType_devRaw, slv_reg13, &rdData);
  chkDelay_buf   = rdData;

  if(isMaster == 1){
    ip_rd(accType_devRaw, slv_reg10, &rdData);
    fanA = ((rdData >> 16) & 0xffff);
    fanB = ((rdData >>  0) & 0xffff);

    ip_rd(accType_devRaw, slv_reg11, &rdData);
    fanC = ((rdData >> 16) & 0xffff);
    fanD = ((rdData >>  0) & 0xffff);
  }
  else
  {
    fanA = -1;
    fanB = -1;
    fanC = -1;
    fanD = -1;
  }
  return RET_OK;
}


//=====================================
//----===@ prnStat
// Parameters  :
// Description :
int ts2ipEvsys::prnStat(void)
{
  int rtVal;

  ifRet(fd < 0);

  prnM2("======================================================\r\n");
  prnM2("++ %s prnStat ++\r\n", devName);
  prnM2("------------------------------------------------------\r\n");
  rtVal = getStat();
  ifRet(RET_ERR == rtVal);

  prnM2_fmtDec("isMaster       ", isMaster    , "[1:master (1st floor), 0:2nd floor]");
  prnM2_fmtDec("fan A", fanA      , " ");
  prnM2_fmtDec("fan B", fanB      , " ");
  prnM2_fmtDec("fan C", fanC      , " ");
  prnM2_fmtDec("fan D", fanD      , " ");

  prnM2_fmtDec("FMC PRSNT_M2C  ", PRSNT_M2C_L , "[0:Loss]");
  prnM2_fmtDec("f_SFP_Loss A   ", f_SFP_LossA , "[1:Loss]");
  prnM2_fmtDec("f_SFP_Loss B   ", f_SFP_LossB , "[1:Loss]");
  prnM2_fmtDec("f_SFP_present A", f_SFP_prsntA, "[1:Present]");
  prnM2_fmtDec("f_SFP_present B", f_SFP_prsntB, "[1:Present]");

  prnM2("%-22s: %s\r\n", "sh Name", sh.name);
  prnM2("%-22s: %s\r\n", "sl Name", sl.name);

  prnM2("%-22s: 0x%04x    , %s\r\n", "sh io", shio, "[high=input, low=output]");
  prnM2("%-22s: 0x%04x    , %s\r\n", "sl io", slio, "[high=input, low=output]");
  
  prnM2("%-22s: 0x%08x    \r\n", "chkDelay_buf", chkDelay_buf);
  
  prnM2("\r\n");
 
  return RET_OK;
}






//=====================================
//----===@ init_bp
// Parameters  :
// Description :
int ts2ipEvsys::init_bp(void)
{
  unsigned char txBuf;
  uint txReg;
  int bytesWrite;

  prnM2("----- %-16s -----\r\n", __FUNCTION__);

  //---------------------------------------------------------------------------
  // port I/O Config
  // max7313 port I/O config, 1:input, 0:output 
  if(max7313_configIO(IIC_BUS_bp, SLV_MAX7313_A, 0xf0, 0xf0 )!= 1) 
    prnErr();

  //---------------------------------------------------------------------------
  // PWM config 
  // Master, O16 Intensity Register -> Master intensity duty cycle is 15/15 (full), O16 intensity duty cycle is 1/16
  txReg = 0x0e; txBuf = 0xf0;
  bytesWrite = iic_wr(IIC_BUS_bp, SLV_MAX7313_A, txReg, 1, (char*)&txBuf, 1 );
  if(bytesWrite != 1) 
    prnErr();

  // Configuration Register -> Enable blink
  txReg = 0x0f; txBuf = 0x01;
  bytesWrite = iic_wr(IIC_BUS_bp, SLV_MAX7313_A, txReg, 1, (char*)&txBuf, 1 );
  if(bytesWrite != 1) 
    prnErr();

  // Outputs intensity P9, P8 
  if(set_fanSpeed( 12, 0x4)!= 1) 
    prnErr();

  //---------------------------------------------------------------------------
  // port Output set to high or low
  if(max7313_setOutput(IIC_BUS_bp, SLV_MAX7313_A, 0x03, 0x00 )!= 1) 
    prnErr();

  prnM2("%s done...\r\n", __FUNCTION__);
  return RET_OK;
};


//=====================================
//----===@ init_clk
// Parameters  :
// Description : si5338 initialize
int ts2ipEvsys::init_clk(int clkSrc)
{
  uint ret;

  int i;
  Reg_Data rd;
  unsigned char reg_val, test_data;
  Reg_Data* pRegData;

  // Check chip by reading Revision reg 0x00
  prnM2("----- %-16s -----\r\n", __FUNCTION__);

  test_data = si5338_read(0);
  if(test_data != 1)
  {
    prnM3("[ERR] si5338_init test_data read\r\n");
    return RET_ERR;
  };

  // I2C Programming Procedure
  prnM1("si5338_init I2C Programming Procedure...\r\n");
  si5338_write(246, 0x01);          //Hard reset
  // Disable Outputs
  prnM1("si5338_init Disable Outputs...\r\n");
  si5338_write_mask(230, EOB_ALL, EOB_ALL); // EOB_ALL = 1
  // Pause LOL
  prnM1("si5338_init Pause LOL...\r\n");
  si5338_write_mask(241, DIS_LOL, DIS_LOL); // DIS_LOL = 1
  // Write new configuration to device accounting for the write-allowed mask
  prnM1("si5338_init Write new configuration...\r\n");

  if(REF_CLK_Ext == clkSrc)
    pRegData = Reg_extClk;
  else  
    pRegData = Reg_ob25MHz;

  for(i=0; i<NUM_REGS_MAX; i++)
  {
    rd = pRegData[i];
    ret = si5338_write_mask(rd.Reg_Addr, rd.Reg_Val, rd.Reg_Mask);
    if(ret != 1)
      return RET_ERR;
  }
  // Validate clock input status
  i=0;
  prnM1("si5338_init Validate clock input status...\r\n");
  reg_val = si5338_read(218) & LOS_MASK;
  while(reg_val != 0){
    i++;
    if(i>10000)
    {
      prnM1("[ERR] si5338_init Validate clock input status...\r\n");
      return RET_ERR;
    }
    reg_val = si5338_read(218) & LOS_MASK;
  }
  // Configure PLL for locking
  prnM1("si5338_init Configure PLL for locking...\r\n");
  si5338_write_mask(49, 0, FCAL_OVRD_EN); //FCAL_OVRD_EN = 0
  // Initiate Locking of PLL
  prnM1("si5338_init Initiate Locking of PLL...\r\n");
  si5338_write(246, SOFT_RESET);      //SOFT_RESET = 1
  usleep(25000);                // Wait 25 ms
  // Restart LOL
  prnM1("si5338_init Restart LOL...\r\n");
  si5338_write_mask(241, 0, DIS_LOL);   // DIS_LOL = 0
  si5338_write(241, 0x65);          // Set reg 241 = 0x65

  // Confirm PLL lock status
  prnM1("si5338_init Confirm PLL lock status...\r\n");
  i=0;
  reg_val = si5338_read(218) & LOCK_MASK;
  while(reg_val != 0){
    i++;
    if(i>10000)
    {
      prnM1("[ERR] si5338_init Confirm PLL lock status...\r\n");
      return RET_ERR;
    }
    reg_val = si5338_read(218) & LOCK_MASK;
  }
  //copy FCAL values to active registers
  prnM1("si5338_init copy FCAL values to active registers...\r\n");
  si5338_write_mask( 47, si5338_read( 237), 0x03);  // 237[1:0] to 47[1:0]
  si5338_write(46, si5338_read( 236));  // 236[7:0] to 46[7:0]
  si5338_write(45, si5338_read( 235));  // 235[7:0] to 45[7:0]
  si5338_write_mask( 47, 0x14, 0xFC);   // Set 47[7:2] = 000101b
  // Set PLL to use FCAL values
  prnM1("si5338_init Set PLL to use FCAL values...\r\n");
  si5338_write_mask( 49, FCAL_OVRD_EN, FCAL_OVRD_EN); //FCAL_OVRD_EN = 1
  // Enable Outputs
  prnM1("si5338_init Enable Outputs...\r\n");
  si5338_write(230, 0x00);          //EOB_ALL = 0

  prnM2("%s done...\r\n", __FUNCTION__);
  return RET_OK;
};


//=====================================
//----===@ init_cps
// Parameters  :
// Description :
int ts2ipEvsys::init_cps(int busId)
{
  unsigned char txBuf;
  uint txReg;
  int bytesWrite;

  ifRet(busId > IIC_BUS_MAX-1);
  
  prnM2("----- %-16s -----\r\n", __FUNCTION__);
  prnM2("cps : [%d] ",busId);

  switch (busId) 
  {
    case 1  : prnM2("Onboard.\r\n"); break;
    case 2  : prnM2("Lower Layer Slave Board.\r\n"); break;
    case 3  : prnM2("Upper Layer Slave board.\r\n"); break;
    default : prnM3("[ERR] . \r\n"); break;
  }

  //adn4604 reset
  txReg = 0x00; txBuf = 1;
  bytesWrite = iic_wr(busId, SLV_ADN4604, txReg, 1, (char*)&txBuf, 1 );
  if(bytesWrite != 1)
    prnErrRet(); 

  //adn4604 mapselect 0
  txReg = 0x81; txBuf = 0x00;
  bytesWrite = iic_wr(busId, SLV_ADN4604, txReg, 1, (char*)&txBuf, 1 );
  if(bytesWrite != 1)
    prnErrRet(); 

  //adn4604 termination - All Termination Enable
  txReg = 0xF0; txBuf = 0x00;
  bytesWrite = iic_wr(busId, SLV_ADN4604, txReg, 1, (char*)&txBuf, 1 );
  if(bytesWrite != 1)
    prnErrRet(); 

  //adn4604 update
  txReg =0x80;  txBuf = 0x01;
  bytesWrite = iic_wr(busId, SLV_ADN4604, txReg, 1, (char*)&txBuf, 1 );
  if(bytesWrite != 1)
    prnErrRet(); 

  prnM2("%s done...\r\n", __FUNCTION__);

  return RET_OK;
};

//=====================================
//----===@ init_system
// Parameters  :
// Description :
int ts2ipEvsys::init_system()
{
  int rtVal;

	prnM2("init_system\r\n");

	lcdClear();

  // Timing System v1.4
  // ==================
  // Event TimingSystem 
  // v.562.562.562.562
  // 
  // EVS0001
  // 021.11.21-05.21.32
  // Temp
  // EV
  // timing network ok

  sprintf(lcdStr, "Event TimingSystem\n");
	lcdSetColor(LCD_COLOR_BLUE);
	lcdDrawStr(1,LCD_LINE_1, lcdStr);

  sprintf(lcdStr, "%s\n", ptsName);
	lcdSetColor(LCD_COLOR_GREEN);
	lcdDrawStr(1,LCD_LINE_4,lcdStr);

  if(TS_OP_MODE_FANCHECK == *ptsOpMode)
  {
    lcdSetColor(LCD_COLOR_WHITE);
    lcdDrawStr(1,LCD_LINE_4,"Fan A : \n");
    lcdDrawStr(1,LCD_LINE_5,"Fan B : \n");
    lcdDrawStr(1,LCD_LINE_6,"Fan C : \n");
    lcdDrawStr(1,LCD_LINE_7,"Fan D : \n");
  }
  else
  {
    lcdSetColor(LCD_COLOR_WHITE);
    // lcdDrawStr(1,LCD_LINE_4,"Time:\n");
    // lcdDrawStr(1,LCD_LINE_5,"Run :\n");
    lcdDrawStr(1,LCD_LINE_6,"Temp:\n");
    lcdDrawStr(1,LCD_LINE_7,"EV  :\n");
  }

	set_fp_led(1,0,1,1);

  if(sl.id == 0x02){
    if(RET_ERR == init_cps(IIC_BUS_sl) )
      rtVal = RET_ERR;
  }

  if(sh.id == 0x02){
    if(RET_ERR == init_cps(IIC_BUS_sh) )
      rtVal = RET_ERR;
  }

	return rtVal;
};


//=====================================
//----===@ setIgen
// Parameters  :
// Description :
    //evg control config
    // igen_enable            <= `DLY slv_reg34[1];
    // igen_mode_ext          <= `DLY slv_reg34[0];  // 0 : internal irigb gen, 1 : external irigb
    // igen_set_y7            <= `DLY slv_reg34[31]; 
    // igen_set_d9            <= `DLY slv_reg35[31:23]; 
    // igen_set_y7[5:0]       <= `DLY slv_reg35[22:17]; 
    // igen_set_h5            <= `DLY slv_reg35[16:12]; 
    // igen_set_m6            <= `DLY slv_reg35[11: 6]; 
    // igen_set_s6            <= `DLY slv_reg35[ 5: 0]; 
int ts2ipEvsys::setIgen(uint igenEnable, uint igenModeExt, uint year, uint day, uint hour, uint min, uint sec)
{
  ifRet(fd < 0);

  uint wrData = 0;

  igen_mode_ext = igenModeExt;
  igen_enable   = igenEnable;

  wrData |= ((year & 0x0000003F) << 17); // 17
  wrData |= ((day  & 0x000001FF) << 23); // 27 bit
  wrData |= ((hour & 0x0000001F) << 12); // 12
  wrData |= ((min  & 0x0000003F) <<  6); // 6
  wrData |= ((sec  & 0x0000003F) <<  0); // 0

  ip_wr(accType_devRaw, A_cfgEvg_igen_0, igen_mode_ext | (igen_enable << 1) | ((year & 0x00000040) << 25) );
  ip_wr(accType_devRaw, A_cfgEvg_igen_1, wrData);

  if(igen_enable == 1)ip_setCommand(SET_igen_set_time, 0);

  return RET_OK;
};


//update Lcd fan rpm value
int ts2ipEvsys::updateLCD_FAN()
{
  static uint pre_A = 0;
  static uint pre_B = 0;
  static uint pre_C = 0;
  static uint pre_D = 0;
  
  if(pre_A  != fanA ){sprintf(lcdStr, "%04d.\n" , fanA  );lcdDrawStr(9,LCD_LINE_4,lcdStr);  }
  if(pre_B  != fanB ){sprintf(lcdStr, "%04d.\n" , fanB  );lcdDrawStr(9,LCD_LINE_5,lcdStr);  }
  if(pre_C  != fanC ){sprintf(lcdStr, "%04d.\n" , fanC  );lcdDrawStr(9,LCD_LINE_6,lcdStr);  }
  if(pre_D  != fanD ){sprintf(lcdStr, "%04d.\n" , fanD  );lcdDrawStr(9,LCD_LINE_7,lcdStr);  }

  pre_A = fanA;
  pre_B = fanB;
  pre_C = fanC;
  pre_D = fanD;

  return RET_OK;
}


//update Lcd version info
int ts2ipEvsys::updateLCD_Ver(char* cVer)
{
  sprintf(lcdStr, "%s\n", cVer);
	lcdSetColor(LCD_COLOR_RED);
	lcdDrawStr(1,LCD_LINE_2,lcdStr);
	lcdSetColor(LCD_COLOR_WHITE);
  return RET_OK;
}


// Event TimingSystem
// 0123456789012345678
// 2020.08.22.12.25.60
  // sprintf(s,"%s","TTwww.durutronix.com\n");
  // sprintf(s,"%s","TTEvent TimingSystem\n");
  // sprintf(s,"TT  %02d.%02d.%02d-%02d:%02d\n", year, month, day, hour, min);

//update time : day hour min sec
int ts2ipEvsys::updateLCD_Time(drTime_T* evTime)
{
  static int tmp_year = 99;
  static int pre_year = 99;
  static int pre_mon  = 99;
  static int pre_day  = 99;
  static int pre_hour = 99;
  static int pre_min  = 99;
  static int pre_sec  = 99;

  static uint lineN = LCD_LINE_5;
  
  tmp_year = evTime->tmT.tm_year - 2000;

  if(pre_year != tmp_year ){
    if(tmp_year > 99)
      tmp_year = 99;
    sprintf(lcdStr, "%02d.\n" , tmp_year );
    lcdDrawStr(1,lineN,lcdStr);
  }

  if(pre_mon  != evTime->tmT.tm_mon ){
    sprintf(lcdStr, "%02d.\n" , evTime->tmT.tm_mon+1  );
    lcdDrawStr(4,lineN,lcdStr);
  }

  if(pre_day  != evTime->tmT.tm_mday ){
    sprintf(lcdStr, "%02d-\n" , evTime->tmT.tm_mday  );
    lcdDrawStr(7,lineN,lcdStr);
  }

  if(pre_hour != evTime->tmT.tm_hour){
    sprintf(lcdStr, "%02d.\n", evTime->tmT.tm_hour);
    lcdDrawStr(10,lineN,lcdStr);
  }

  if(pre_min  != evTime->tmT.tm_min ){
    sprintf(lcdStr, "%02d.\n", evTime->tmT.tm_min);
    lcdDrawStr(13,lineN,lcdStr);
  }

  if(pre_sec  != evTime->tmT.tm_sec ){
    sprintf(lcdStr, "%02d\n", evTime->tmT.tm_sec);
    lcdDrawStr(16,lineN,lcdStr);
  }

  pre_year = tmp_year ;
  pre_mon  = evTime->tmT.tm_mon ;
  pre_day  = evTime->tmT.tm_mday ;
  pre_hour = evTime->tmT.tm_hour;
  pre_min  = evTime->tmT.tm_min ;
  pre_sec  = evTime->tmT.tm_sec ;

  return RET_OK;
}

//update temperature SL SH
int ts2ipEvsys::updateLCD_Temperature(void)
{
  static float pre_slTemp = -1.0f;
  static float pre_shTemp = -1.0f;

  if(pre_slTemp != sl.temp){
    sprintf(lcdStr, "L%02.01f\n" , sl.temp  );
    lcdDrawStr(7,LCD_LINE_6,lcdStr);
    pre_slTemp = sl.temp;
  }

  if(pre_shTemp != sh.temp){
    sprintf(lcdStr, " H%02.01f\n" , sh.temp  );
    lcdDrawStr(13,LCD_LINE_6,lcdStr);
    pre_shTemp = sh.temp;
  }

	return RET_OK;
}

//update evCode status rx evcode count
int ts2ipEvsys::updateLCD_rxCount(uint evCodeA_cntr, uint evCodeB_cntr)
{
	static uint pre_rx_evCodeA_cntr = 999;
	static uint pre_rx_evCodeB_cntr = 999;

  // if(pre_rx_evCodeA_cntr != evCodeA_cntr ){
  //   sprintf(lcdStr, "%6dA\n" , evCodeA_cntr  );
  //   lcdDrawStr(6,LCD_LINE_7,lcdStr);
  //   pre_rx_evCodeA_cntr = evCodeA_cntr;
  // }

  if(pre_rx_evCodeB_cntr != evCodeB_cntr ){
    sprintf(lcdStr, "%8d\n" , evCodeB_cntr  );
    lcdDrawStr(6,LCD_LINE_7,lcdStr);
    pre_rx_evCodeB_cntr = evCodeB_cntr; 
  }
  
	return RET_OK;
}

//update message
int ts2ipEvsys::updateLCD_Message( char color, const char* message)
{
  sprintf(lcdStr, "%-20s\n", message);

  lcdSetColor(color);
  lcdDrawStr(1,LCD_LINE_8,lcdStr);
  lcdSetColor(LCD_COLOR_WHITE); 

	return RET_OK;
}

//update front panel LED
int ts2ipEvsys::updateFrontPanel_LED(char pwrOK, char fanERR, char epicsOK, char timingNetOK)
{
  static char fpLed_stat;
	static char fpLed_stat_pre = 0xff; 

  fpLed_stat = ((pwrOK<<3) | (fanERR<<2) | (epicsOK<<1) | timingNetOK);

  if(fpLed_stat != fpLed_stat_pre)
    set_fp_led(pwrOK, fanERR, epicsOK, timingNetOK);

  fpLed_stat_pre = fpLed_stat;

	return RET_OK;
}




//=====================================
//----===@ 
// Parameters  :
// Description :
int ts2ipEvsys::set_cpsByMode()
{
  prnM2("----- %-16s -----\r\n", __FUNCTION__);

  ob.busId = IIC_BUS_ob;
  sl.busId = IIC_BUS_sl;
  sh.busId = IIC_BUS_sh;

  memset(ob.cpsOut  ,0,sizeof(ob.cpsOut));
  memset(ob.cpsOutEn,0,sizeof(ob.cpsOutEn));

  memset(sl.cpsOut  ,0,sizeof(sl.cpsOut));
  memset(sl.cpsOutEn,0,sizeof(sl.cpsOutEn));

  memset(sh.cpsOut  ,0,sizeof(sh.cpsOut));
  memset(sh.cpsOutEn,0,sizeof(sh.cpsOutEn));

  switch(*ptsMode)
  {
    case RAON_EVG :     ob.cpsOut[CPS_OB_GT0 ] = CPS_OB_GT0;
                        ob.cpsOut[CPS_OB_GT1 ] = CPS_OB_GT1;
                        ob.cpsOut[CPS_OB_FMC1] = CPS_OB_GT1;
                        ob.cpsOut[CPS_OB_FMC2] = CPS_OB_GT1;
                        ob.cpsOut[CPS_OB_SL  ] = CPS_OB_GT1;
                        ob.cpsOut[CPS_OB_SH  ] = CPS_OB_GT1;
                        break;
                        
    case RAON_EVS :     ob.cpsOut[CPS_OB_GT0 ] = CPS_OB_GT0;
                        ob.cpsOut[CPS_OB_GT1 ] = CPS_OB_GT1;
                        ob.cpsOut[CPS_OB_FMC1] = CPS_OB_GT1;
                        ob.cpsOut[CPS_OB_FMC2] = CPS_OB_GT1;
                        ob.cpsOut[CPS_OB_SL  ] = CPS_OB_GT1;
                        ob.cpsOut[CPS_OB_SH  ] = CPS_OB_GT1;
                        break;
  
    case RAON_EVF :     ob.cpsOut[CPS_OB_GT0 ] = CPS_OB_FMC1;
                        ob.cpsOut[CPS_OB_GT1 ] = CPS_OB_FMC1;
                        ob.cpsOut[CPS_OB_FMC1] = CPS_OB_FMC1;
                        ob.cpsOut[CPS_OB_FMC2] = CPS_OB_FMC1;
                        ob.cpsOut[CPS_OB_SL  ] = CPS_OB_FMC1;
                        ob.cpsOut[CPS_OB_SH  ] = CPS_OB_FMC1;
                        break;

    case RAON_EVRUP  :  ob.cpsOut[CPS_OB_GT0 ] = CPS_OB_FMC1;
                        ob.cpsOut[CPS_OB_GT1 ] = CPS_OB_FMC1;
                        ob.cpsOut[CPS_OB_FMC1] = CPS_OB_GT1;
                        ob.cpsOut[CPS_OB_FMC2] = CPS_OB_FMC1;
                        ob.cpsOut[CPS_OB_SL  ] = CPS_OB_FMC1;
                        ob.cpsOut[CPS_OB_SH  ] = CPS_OB_FMC1;
                        break;

    case RAON_EVR :     ob.cpsOut[CPS_OB_GT0 ] = CPS_OB_FMC1;
                        ob.cpsOut[CPS_OB_GT1 ] = CPS_OB_FMC1;
                        ob.cpsOut[CPS_OB_FMC1] = CPS_OB_FMC1;
                        ob.cpsOut[CPS_OB_FMC2] = CPS_OB_FMC1;
                        ob.cpsOut[CPS_OB_SL  ] = CPS_OB_FMC1;
                        ob.cpsOut[CPS_OB_SH  ] = CPS_OB_FMC1;
                        break;
    default   : break;
  }

  ob.cpsOutEn[CPS_OB_GT0 ] = 0x30;
  ob.cpsOutEn[CPS_OB_GT1 ] = 0x30;
  ob.cpsOutEn[CPS_OB_FMC1] = 0x30;
  ob.cpsOutEn[CPS_OB_FMC2] = 0x30;
  ob.cpsOutEn[CPS_OB_SL  ] = 0x30;
  ob.cpsOutEn[CPS_OB_SH  ] = 0x30;

  sl.cpsOut[CPS_SW_CTRL] = CPS_SW_00;
  sl.cpsOutEn[CPS_SW_CTRL] = 0x30;

  for(int i = 0; i < 12; i++)
  {
    sl.cpsOut[CPS_SW_00 + i] = CPS_SW_CTRL;
    sl.cpsOutEn[CPS_SW_00 + i] = 0x31;
  };

  sh.cpsOut[CPS_SW_CTRL] = CPS_SW_00;
  sh.cpsOutEn[CPS_SW_CTRL] = 0x30;

  for(int i = 0; i < 12; i++)
  {
    sh.cpsOut[CPS_SW_00 + i] = CPS_SW_CTRL;
    sh.cpsOutEn[CPS_SW_00 + i] = 0x31;
  };

  set_cpsConfig(&ob);

  if(sl.id==0x02)
    set_cpsConfig(&sl);

  if(sh.id==0x02)
    set_cpsConfig(&sh);

  prnM2("%s done...\r\n", __FUNCTION__);

  return RET_OK;
};


//=====================================
//----===@ set_cpsConfig
// Parameters  :
// Description :
int ts2ipEvsys::set_cpsConfig(struct s_ts2slv* pSlv)
{
  ifRet(pSlv == NULL);
  ifRet(pSlv->busId > IIC_BUS_MAX-1);

  unsigned char txBuf;
  uint  txReg;
  int bytesWrite;

  int i;

  prnM0("tx===============\r\n");
  for( i = 0; i < 8; i++)
  {
    txReg = 0x90+i;
    txBuf = (pSlv->cpsOut[i*2] & 0x0f) | ((pSlv->cpsOut[i*2+1] & 0x0f) << 4);
    bytesWrite = iic_wr(pSlv->busId, SLV_ADN4604, txReg, 1, (char*)&txBuf, 1 );
    prnM0("0x%02x : 0x%02x\r\n",txReg,txBuf);
    if(bytesWrite != 1) prnErrRet();
  }

  for( i = 0; i < 16; i++)
  {
    txReg = 0x20+i;
    bytesWrite = iic_wr(pSlv->busId, SLV_ADN4604, txReg, 1, (char*)&pSlv->cpsOutEn[i], 1 );
    prnM0("0x%02x : 0x%02x\r\n",txReg,pSlv->cpsOutEn[i]);
    if(bytesWrite != 1) prnErrRet();
  }

#if 1 //eq enable
    txReg =0x10;  txBuf = 0xf0;
    bytesWrite = iic_wr(pSlv->busId, SLV_ADN4604, txReg, 1, (char*)&txBuf, 1 );
    prnM0("0x%02x : 0x%02x\r\n",txReg,txBuf);
    if(bytesWrite != 1) prnErrRet();

    txReg =0x11;  txBuf = 0xff;
    bytesWrite = iic_wr(pSlv->busId, SLV_ADN4604, txReg, 1, (char*)&txBuf, 1 );
    prnM0("0x%02x : 0x%02x\r\n",txReg,txBuf);
    if(bytesWrite != 1) prnErrRet();
#endif    

  txReg =0x80;  txBuf = 0x01;
  bytesWrite = iic_wr(pSlv->busId, SLV_ADN4604, txReg, 1, (char*)&txBuf, 1 );
  prnM0("0x%02x : 0x%02x\r\n",txReg,txBuf);
  if(bytesWrite != 1) prnErrRet();

#if 1 //eq enable
  unsigned char rxBuf;
  uint  rxReg;
  int bytesRead;

  prnM0("rx===============\r\n");
  for( i = 0; i < 8; i++)
  {
    rxReg = 0x90+i;
    bytesRead = iic_rd(pSlv->busId, SLV_ADN4604, rxReg, 1, (char*)&rxBuf, 1 );
    prnM0("0x%02x : 0x%02x\r\n",rxReg,rxBuf);
    if(!bytesRead) prnErrRet();
  }

  for( i = 0; i < 16; i++)
  {
    rxReg = 0x20+i;
    bytesRead = iic_rd(pSlv->busId, SLV_ADN4604, rxReg, 1, (char*)&rxBuf, 1 );
    prnM0("0x%02x : 0x%02x\r\n",rxReg,rxBuf);
    if(!bytesRead) prnErrRet();
  }

    rxReg =0x10;
    bytesRead = iic_rd(pSlv->busId, SLV_ADN4604, rxReg, 1, (char*)&rxBuf, 1 );
    prnM0("0x%02x : 0x%02x\r\n",rxReg,rxBuf);
    if(!bytesRead) prnErrRet();

    rxReg =0x11;
    bytesRead = iic_rd(pSlv->busId, SLV_ADN4604, rxReg, 1, (char*)&rxBuf, 1 );
    prnM0("0x%02x : 0x%02x\r\n",rxReg,rxBuf);
    if(!bytesRead) prnErrRet();

#endif    

  return RET_OK;
};




//=====================================
//----===@ set_slaveIO_byMode
// Parameters  :
// Description :
int ts2ipEvsys::set_slaveIO_byMode()
{
  prnM2("----- %-16s -----\r\n", __FUNCTION__);
  // default : high[input]

  if(sl.id==0x01){
    if((*ptsMode == RAON_EVG) || (*ptsMode == RAON_EVS) || (*ptsMode == RAON_EVRUP)){
      // slio = 0xffff;
      slio = 0x3; // only PAL option //only ch0,1 input config
    }
    else{
      slio = 0;
    }
  }
  else{
    slio = 0xffff;
  }

  if(sh.id==0x01){
    shio = 0;
  }
  else{
    shio = 0xffff;
  }

  set_sys_IO(slio, shio);

  taskDelay(8000);

  set_slaveIO_sub(&sl, slio);
  set_slaveIO_sub(&sh, shio);

  prnM2("%s done...\r\n", __FUNCTION__);
  return RET_OK;
};

//=====================================
//----===@ 
// Parameters  :
// Description :
int ts2ipEvsys::set_slaveIO_sub(s_ts2slv* pSlv, uint setVal)
{
  unsigned char wrData[2];

  ifRet(pSlv<0);

  //---------------------------------------------------------------------------
  // port Output set to high or low
  wrData[0] =  setVal & 0xff;
  wrData[1] = (setVal >> 8) & 0xff ;
  prnM0("%d_wrData[0] : %02x\r\n",pSlv->busId, wrData[0]);
  prnM0("%d_wrData[1] : %02x\r\n",pSlv->busId, wrData[1]);

  //---------------------------------------------------------------------------
  // port I/O Config
  // max7313 port I/O config, 1:input, 0:output 
  if(max7313_setOutput(pSlv->busId, SLV_MAX7313_B, wrData[1], wrData[0] )!= 1) 
    prnErrRet();

  if(max7313_configIO(pSlv->busId, SLV_MAX7313_B, 0x00, 0x00 )!= 1) 
    prnErrRet();

  return RET_OK;
};


//=====================================
//----===@ 
// Parameters  :
// Description :
int ts2ipEvsys::set_sys_IO(uint lsbV, uint msbV)
{
  ifRet(fd < 0);
  ip_wr(accType_devRaw, A_cfgSys_IO_LSB, lsbV);
  ip_wr(accType_devRaw, A_cfgSys_IO_MSB, msbV);
  return RET_OK;
};

//=====================================
//----===@ set_sys_FMC_LED
// Parameters  : link0 high:Red low:Green /link1 high:Red low:Green
// Description :
int ts2ipEvsys::set_sys_FMC_LED(uint link0, uint link1) 
{
  ifRet(fd < 0);

  static int led_stat;
	static int led_stat_pre = 0; 

  led_stat = ( ((!link1 & 0x1)<<1) | (!link0 & 0x1) );

  if(led_stat != led_stat_pre)
    ip_wr(accType_devRaw, slv_reg33, led_stat);

  led_stat_pre = led_stat;
  return RET_OK;
};


//=====================================
//----===@ set_fanSpeed
// Parameters  :
// Description :
int ts2ipEvsys::set_fanSpeed(char pwmFront, char pwmRear ) // pwmIntensity
{
  unsigned char txBuf;
  uint txReg;
  int bytesWrite;

  // Outputs intensity P9, P8 
  txReg = 0x14; txBuf = (((0x0F - pwmRear) & 0x0F)<<4) | ((0x0F - pwmFront) & 0x0F);
  bytesWrite = iic_wr(IIC_BUS_bp, SLV_MAX7313_A, txReg, 1, (char*)&txBuf, 1 );
  if(bytesWrite != 1)
    prnErrRet();

  return RET_OK;
};

//=====================================
//----===@ set_fp_led
// Parameters  :
// Description : front panel LED control function [if master]
int ts2ipEvsys::set_fp_led(unsigned char pwrOK, unsigned char fanERR, unsigned char epicsOK, unsigned char timingNetworkOK)
{
  if(isMaster == 1){
    ifRet(pwrOK > 1);
    ifRet(fanERR > 1);
    ifRet(epicsOK > 1);
    ifRet(timingNetworkOK > 1);

    unsigned char txBuf;
    uint txReg;
    int bytesWrite;

    txReg = 0x02;	
    txBuf = pwrOK | (fanERR<<1) | (epicsOK<<2) | (timingNetworkOK<<3) ;
    bytesWrite = iic_wr(IIC_BUS_bp, SLV_MAX7313_A, txReg, 1, (char*)&txBuf, 1 );
    if(bytesWrite != 1) 
      prnErrRet();	
  }

  return RET_OK;
}

//=====================================
//----===@ get_slaveBoardID
// Parameters  :
// Description : get slave board ID 1st & 2nd floor
int ts2ipEvsys::get_slaveBoardID(int busId)
{
  unsigned char rxData[2];
  int bytesRead;

  bytesRead = iic_rd(busId, SLV_MAX7313_A, 0, 1, (char*)rxData, sizeof(rxData));
  if(bytesRead == -1)
  {
    if(busId == IIC_BUS_sl){
      sl.id = 0;
      sprintf(sl.name,"%s","NONE");
      prnErr();
      prnM3("[ERR]slave board low side access Error.\r\n");
    }

    if(busId == IIC_BUS_sh){
      sh.id = 0;
      sprintf(sh.name,"%s","NONE");
      prnErr();
      prnM3("[ERR] slave board high side access Error.\r\n");
    }
    return RET_ERR;
  }
  else
  {
    if(busId == IIC_BUS_sl){
      sl.id = rxData[0] & 0x0f;
      get_slaveBoardName(&sl);
      prnM2("slave Board Low  : %s\r\n", sl.name);
    }
    
    if(busId == IIC_BUS_sh){
      sh.id = rxData[0] & 0x0f;
      get_slaveBoardName(&sh);
      prnM2("slave Board High : %s\r\n", sh.name);
    }
  }
  prnM2("%s done...\r\n", __FUNCTION__);
  return RET_OK;
};

//=====================================
//----===@ get_slaveBoardName
// Parameters  :
// Description : get slave board name
int ts2ipEvsys::get_slaveBoardName(struct s_ts2slv* pSlv )
{
  int rtVal = RET_OK;

  switch(pSlv->id)
  {
    case 1 :
              sprintf(pSlv->name,"%s","I/O LEMO-16");
              break;
    case 2 :
              sprintf(pSlv->name,"%s","FANOUT-12");
              break;
    default:
              pSlv->id = 0;
              sprintf(pSlv->name,"%s","NONE");
              rtVal = RET_ERR;
              break;
  }
  return rtVal;
};

//=====================================
//----===@ get_slaveTemp
// Parameters  :
// Description : get slave board ID 1st & 2nd floor
int ts2ipEvsys::get_slaveTemp(void)
{
  unsigned char rxData[2];
  int bytesRead;

  if(sl.id == 0)
  {
      sl.temp = -1.0;
  }
  else
  {
    rxData[0]=0;
    rxData[1]=0;
    bytesRead = iic_rd(IIC_BUS_sl, SLV_TMP112A, 0, 1, (char*)rxData, sizeof(rxData));
    if(bytesRead == RET_ERR)
      sl.temp = 0;
    else
      sl.temp = (((float)rxData[0] *0.0625) *16) + ((float)(rxData[1] >> 4)*0.0625);
  }
  

  if(sh.id == 0)
  {
      sh.temp = -1.0;
  }
  else
  {
    rxData[0]=0;
    rxData[1]=0;
    bytesRead = iic_rd(IIC_BUS_sh, SLV_TMP112A, 0, 1, (char*)rxData, sizeof(rxData));
    if(bytesRead == RET_ERR)
      sh.temp = 0;
    else
      sh.temp = (((float)rxData[0] *0.0625) *16) + ((float)(rxData[1] >> 4)*0.0625);
  }

  return RET_OK;
};

















//==============================================================================================================
//----===@ lcd
//==============================================================================================================

//=====================================
//----===@ lcdWrite
// Parameters  :
// Description :
ssize_t ts2ipEvsys::lcdWrite(char value) //write a byte to serial port (UART/I2C/SPI)
{
  ssize_t ret =0;
  if(isMaster == 1){
    ret = write( lcd_fd, (const void*)&value, 1);
  }
  //ret = (unsigned char)CSerialManager::Instance()->SendCOM((const char*)&value, 1);
  return ret;
}

//=====================================
//----===@ lcdWriteStr
// Parameters  :
// Description :
void ts2ipEvsys::lcdWriteStr(const char *s)
{
  uint writeCnt = 0;
  while (*s != 0)
  {
    lcdWrite(*s);
    s++;
    if(writeCnt++ > 512)break;
  }
}

//=====================================
//----===@ lcdWrite2B
// Parameters  :
// Description :
void ts2ipEvsys::lcdWrite2B(uint v)
{
  if (v < 255)
    lcdWrite(v);
  else {
    lcdWrite(255);
    lcdWrite(v - 255);
  }
}

//=====================================
//----===@ lcdDrawStr
// Parameters  :
// Description :
void ts2ipEvsys::lcdDrawStr(uint x, uint y, const char *s) 
{
  lcdWriteStr("TP");
  lcdWrite2B(x);
  lcdWrite2B(y);
  lcdWriteStr("TT");
  lcdWriteStr(s);
  lcdWrite((unsigned char) 0);
}

//=====================================
//----===@ lcdClear
// Parameters  :
// Description :
void ts2ipEvsys::lcdClear(void) 
{
  lcdWriteStr("CL");
}

//=====================================
//----===@ lcdSetColor
// Parameters  :
// Description :
void ts2ipEvsys::lcdSetColor(unsigned char color) 
{
    lcdWriteStr("SC");
    lcdWrite(color);
}

//=====================================
//----===@ lcdSetColRow
// Parameters  :
// Description :
void ts2ipEvsys::lcdSetColRow(unsigned char col, unsigned char row) 
{
    lcdWriteStr("STCR");
    lcdWrite(col);
    lcdWrite(row);
    lcdWriteStr("\x80\xC0\x94\xD4");
}

//=====================================
//----===@ lcdSetBgColor
// Parameters  :
// Description :
void ts2ipEvsys::lcdSetBgColor(unsigned char color) //set current color as background
{
#if Ver>32
    lcdWriteStr("BGC");
    lcdWrite(color);
#else
    lcdWriteStr("SC");
    lcdWrite(color);
    lcdWriteStr("FTOB");
#endif
}

//=====================================
//----===@ lcdSetFont
// Parameters  :
// Description :
void ts2ipEvsys::lcdSetFont(unsigned char font) 
{
    lcdWriteStr("SF");
    lcdWrite(font);
}



//==============================================================================================================
//----===@ iic
//==============================================================================================================

//=====================================
//----===@ iic_getInfo
// Parameters  :
// Description :
s_ts2iic* ts2ipEvsys::iic_getInfo(int busId, int slvid)
{
  return iic_piic[busId]+slvid;
};


//=====================================
//----===@ iic_setMux
// Parameters  :
// Description :
int ts2ipEvsys::iic_setMux(int lfd, s_ts2iic* piic)
{
  int status;
  unsigned char WriteBuffer[2];
  unsigned char BytesWritten;

  unsigned char ReadBuffer = 0x0; /* Buffer to hold data read.*/
  unsigned short int BytesToRead;

  if(piic == NULL)prnErrRet();

  status = ioctl(lfd, IIC_SLAVE_FORCE, piic->muxAddress);
  if(status < 0 )prnErrRet();

  WriteBuffer[0] = piic->muxNum;
  BytesWritten = write(lfd, WriteBuffer, 1);
  if(BytesWritten != 1)
  {
    // prnErrRet();
    prnM0("[ERR] L%d [%s] [F:%s] W-%s\n", __LINE__ , __FILE__ , __FUNCTION__ , piic->strName);
    ip_setCommand(SET_SLH_iic_rst, 100000);// set_sLH_iicReset(100000);
    return RET_ERR;
  }

  BytesToRead = read(lfd, &ReadBuffer, 1);
  // if(BytesToRead != 1 )prnErrRet();
  if(BytesToRead != 1)
  {
    // prnErrRet();
    prnM0("[ERR] L%d [%s] [F:%s] R-%s\n", __LINE__ , __FILE__ , __FUNCTION__ , piic->strName);
    ip_setCommand(SET_SLH_iic_rst, 100000);// set_sLH_iicReset(100000);
    return RET_ERR;
  }

  return RET_OK;
};


//=====================================
//----===@ iic_wr
// Parameters  :
// Description :
int ts2ipEvsys::iic_wr(int busId, int slvId, uint offset, uint offset_size, char* pData, int size)
{
  int i;
  int lfd;
  int found = 0;
  int Status = 0;
  unsigned char WriteBuffer[MAX_IIC_RW_BUF_SIZE + 2];
  unsigned char BytesWritten;
  s_ts2iic* piic;

  if(busId >= IIC_maxDrv ) prnErrRet();
  if(pData == NULL       ) prnErrRet();
  if(size == 0           ) prnErrRet();

  /* check is valid slave.. */
  for(i = 0; i < iic_slvNum[busId]; i++)
  {
    piic = iic_getInfo(busId, i);
    prnM0("   %d.%d \r\t  0x%02x \r\t\t   %d \r\t\t\t   0x%02x \r\t\t\t\t   %d\n", 
          i, piic->name, piic->addr, piic->isMux, piic->muxAddress, piic->muxNum );

    if(piic->name == (unsigned)slvId)
    {
      lfd = iic_fd[busId];
      found = 1;
      break;
    }   
  }

  /* if found wirte data to iic */
  if(0 == found)
    prnErrRet();

  if(piic->isMux)
  {
    /* do iic mux control */  
    if(!iic_setMux(lfd, piic))
      prnErrRet();
  }

  if(size > MAX_IIC_RW_BUF_SIZE )prnErrRet();

  Status = ioctl(lfd, IIC_SLAVE_FORCE, piic->addr);
  if(Status < 0)prnErrRet();

  if( offset_size == 1)
  {
    WriteBuffer[0] = (unsigned char)(offset);
  }
  else if( offset_size == 2)
  {
    WriteBuffer[0] = (unsigned char)(offset>>8);
    WriteBuffer[1] = (unsigned char)(offset);
  }
  else
    prnErrRet();

  memcpy(&WriteBuffer[offset_size], pData, size); 
  BytesWritten = write(lfd, WriteBuffer, size + offset_size);

  prnM0("iic_wr : %d,%d,%d,%d,\r\n", lfd, busId, slvId, BytesWritten);

  return BytesWritten-offset_size;
};


//=====================================
//----===@ iic_rd
// Parameters  :
// Description :
int ts2ipEvsys::iic_rd(int busId, int slvId, uint offset, uint offset_size, char* pData, int size)
{
  int i;
  int lfd;
  int found = 0;
  int Status = 0;
  unsigned char WriteBuffer[2];
  unsigned char BytesWritten;
  unsigned char BytesRead=0;
  s_ts2iic* piic;

  if(busId >= IIC_maxDrv ) prnErrRet();
  if(pData == NULL       ) prnErrRet();
  if(size == 0           ) prnErrRet();

  /* check is valid slave.. */
  for(i = 0; i < iic_slvNum[busId]; i++)
  {
    piic = iic_getInfo(busId, i);
    if(piic->name == (unsigned)slvId)
    {
      lfd = iic_fd[busId];
      found = 1;
      break;
    }   
  }

  /* if found wirte data to iic */
  if(!found)
    prnErrRet();

  if(piic->isMux)
  {
    /* do iic mux control */  
    if( iic_setMux(lfd, piic ) == RET_ERR )
    {
      return RET_ERR;
    }
  }

  if(size > MAX_IIC_RW_BUF_SIZE )prnErrRet();

  Status = ioctl(lfd, IIC_SLAVE_FORCE, piic->addr);
  if(Status < 0)prnErrRet();

  if( offset_size == 1)
  {
    WriteBuffer[0] = (unsigned char)(offset);
  }
  else if( offset_size == 2)
  {
    WriteBuffer[0] = (unsigned char)(offset>>8);
    WriteBuffer[1] = (unsigned char)(offset);
  }
  else
    prnErrRet();

  BytesWritten = write(lfd, WriteBuffer, offset_size);
  if(BytesWritten != offset_size)
  {
    // prnErrRet();
    prnM3("[ERR] L%d [%s] [F:%s] %s\n", __LINE__ , __FILE__ , __FUNCTION__ , piic->strName);
    return RET_ERR;
  }

  BytesRead = read(lfd, pData, size);
  return BytesRead;
};


//=====================================
//----===@ iic_prnInfo
// Parameters  :
// Description :
int ts2ipEvsys::iic_prnInfo()
{
  int i,j;

  prnM2("-----------------------------------------------\n");
  prnM2("   M.S \r\t iicAddr \r\t\t isMux \r\t\t\t MuxAddr \r\t\t\t\t MuxCh\n");
  prnM2("-----------------------------------------------\n");

  for(i = 0; i < IIC_maxDrv; i++)
  {
    for(j = 0; j < iic_slvNum[i]; j++)
    {
      s_ts2iic* piic = iic_getInfo(i, j);
      prnM2("   %d.%d \r\t  0x%02x \r\t\t   %d \r\t\t\t   0x%02x \r\t\t\t\t   %d\n", 
          i, piic->name, piic->addr, piic->isMux, piic->muxAddress, piic->muxNum );
    };
  }
  prnM2("\n");

  return RET_OK;
};

//==============================================================================
//----===@ max7313 interface
//==============================================================================

//=====================================
//----===@ max7313_configIO
// Parameters  :
// Description : max7313 port I/O config, 1:input, 0:output 
int ts2ipEvsys::max7313_configIO(int busId, int slvId, char PF8, char P70 )
{
  unsigned char txBuf;
  uint txReg;
  int bytesWrite;

  txReg = 0x06; txBuf = P70;
  bytesWrite = iic_wr(busId, slvId, txReg, 1, (char*)&txBuf, 1 );
  if(bytesWrite != 1){
    prnM3("max7313_configIO  busId %d, slvID %d, PF8 %02x, P70 %02x. bytesWrite %d\r\n",busId, slvId, PF8, P70, bytesWrite);
    prnErr();
  }

  txReg = 0x07; txBuf = PF8;
  bytesWrite = iic_wr(busId, slvId, txReg, 1, (char*)&txBuf, 1 );
  if(bytesWrite != 1){
    prnM3("max7313_configIO  busId %d, slvID %d, PF8 %02x, P70 %02x. bytesWrite %d\r\n",busId, slvId, PF8, P70, bytesWrite);
    prnErr();
  }

  return RET_OK;
};

//=====================================
//----===@ max7313_setOutput
// Parameters  :
// Description :
int ts2ipEvsys::max7313_setOutput(int busId, int slvId, char PF8, char P70 )
{
  unsigned char txBuf;
  uint txReg;
  int bytesWrite;

  txReg = 0x02; txBuf = P70;
  bytesWrite = iic_wr(busId, slvId, txReg, 1, (char*)&txBuf, 1 );
  if(bytesWrite != 1)
      prnErr();


  txReg = 0x03; txBuf = PF8;
  bytesWrite = iic_wr(busId, slvId, txReg, 1, (char*)&txBuf, 1 );
  if(bytesWrite != 1)
      prnErr();

  prnM0("max7313_setOutput busId %d, slvID %d, PF8 %02x, P70 %02x.\r\n",busId, slvId, PF8, P70);
  return RET_OK;
}


//=====================================
//----===@ si5338_read
// Parameters  :
// Description :
unsigned char ts2ipEvsys::si5338_read(unsigned char r_addr)
{
  unsigned char rxData = 0;
  iic_rd(IIC_BUS_ob, SLV_SI5338A, r_addr, sizeof(r_addr), (char*)&rxData, 1 );
  return (unsigned char)rxData;
};

//=====================================
//----===@ si5338_write
// Parameters  :
// Description :
uint ts2ipEvsys::si5338_write(unsigned char w_addr, unsigned char w_data)
{
  int ret = iic_wr(IIC_BUS_ob, SLV_SI5338A, w_addr, 1, (char*)&w_data, 1 );
  return ret;
};

//=====================================
//----===@ si5338_write_mask
// Parameters  :
// Description :
uint ts2ipEvsys::si5338_write_mask(unsigned char Addr, unsigned char Data, unsigned char Mask)
{
  uint ret;
  unsigned char reg_val;

  if(Mask == 0xFF)
  { // All bits
    ret = si5338_write(Addr, Data);
    if(ret != 1)
      return ret;
  }
  else
  {           // Write by mask
    reg_val = si5338_read(Addr);
    reg_val &= ~Mask; // Clear bits to write
    reg_val |= Data & Mask; // Set bits by mask

    ret = si5338_write(Addr, reg_val);
    if(ret != 1)
      return ret;
  }
  return 1;
};













} //name space end




