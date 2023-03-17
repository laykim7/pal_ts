//Wrapping class for system calls of timing device driver.
#include <stdlib.h>
#include <termios.h>

#include "ts2ipZqsys.h"

namespace timing{

//==============================================================================
//----===@ define
//==============================================================================
#define DRV_NAME_psiic "/dev/i2c-0"


//==============================================================================
//----===@ global variable
//==============================================================================


// IDT 8V54816A
typedef struct
{
    uint8_t     addr;   // Device address
    char        access; // Access type 1 = read, 2 = write, 3 = read|write

    uint8_t     outMode;   // 1 = Port is output
    uint8_t     termOn;    // 1 = Internal termination is on (100 ohm) , 0 = Internal termination is off (high-impedance)
    uint8_t     nonInvert; // 1 = Non-inverted, 0 = Inverted 
    uint8_t     outPortSel;

    const char* name;
    const char* comment;
} TIdtTbl;

static const TIdtTbl idt_clk_reg[] =
{
  { 0x00, 3, 0, 1, 1,  0, "Port  0", "amc.TCLKA"    }, // IN  | Termination | Non-inversed | 0x60,  0
  { 0x01, 3, 0, 1, 1,  0, "Port  1", "amc.TCLKB"    }, // IN  | Termination | Non-inversed | 0x60,  0
  { 0x02, 3, 0, 1, 1,  0, "Port  2", "amc.TCLKC"    }, // IN  | Termination | Non-inversed | 0x60,  0
  { 0x03, 3, 0, 1, 1,  0, "Port  3", "amc.TCLKD"    }, // IN  | Termination | Non-inversed | 0x60,  0
  { 0x04, 3, 1, 0, 1, 15, "Port  4", "clk.MGS_CLK0" }, // OUT |             | Non-inversed | 0xAF, 15(MG_CLK3)
  { 0x05, 3, 1, 0, 1, 15, "Port  5", "clk.MGS_CLK1" }, // OUT |             | Non-inversed | 0xAF, 15(MG_CLK3)
  { 0x06, 3, 0, 1, 1,  0, "Port  6", "clk.FEXT_CLK" }, // IN  | Termination | Non-inversed | 0x60,  0
  { 0x07, 3, 1, 0, 1, 15, "Port  7", "clk.CLK_F_A"  }, // OUT |             | Non-inversed | 0xAF, 15(MG_CLK3)
  { 0x08, 3, 1, 0, 1, 15, "Port  8", "clk.CLK_F_B"  }, // OUT |             | Non-inversed | 0xAF, 15(MG_CLK3)
  { 0x09, 3, 1, 0, 1, 15, "Port  9", "clk.CLK_F_C"  }, // OUT |             | Non-inversed | 0xAF, 15(MG_CLK3)
  { 0x0A, 3, 1, 0, 1, 15, "Port 10", "clk.CLK_F_D"  }, // OUT |             | Non-inversed | 0xAF, 15(MG_CLK3)
  { 0x0B, 3, 1, 0, 1, 15, "Port 11", "clk.CLK_F_E"  }, // OUT |             | Non-inversed | 0xAF, 15(MG_CLK3)
  { 0x0C, 3, 1, 0, 1, 15, "Port 12", "clk.CLK12(TP)"}, // OUT |             | Non-inversed | 0xAF, 15(MG_CLK3)
  { 0x0D, 3, 1, 0, 1, 15, "Port 13", "clk.CLK13(TP)"}, // OUT |             | Non-inversed | 0xAF, 15(MG_CLK3)
  { 0x0E, 3, 1, 0, 1, 15, "Port 14", "clk.MCLK_A"   }, // OUT |             | Non-inversed | 0xA6,  6(FEXT_CLK)
  { 0x0F, 3, 0, 1, 1,  6, "Port 15", "clk.MG_CLK3"  }, // IN  | Termination | Non-inversed | 0x60,  0
  { 0x00, 0, 0, 0, 1,  0, NULL,      NULL           }
};


#if 1
static const TIdtTbl idt_trig_reg[] =
{
  { 0x00, 3, 0, 1, 1,  0, "Port  0", "amc.MLVDS_P17T" }, // IN  | Termination | Non-inversed | 
  { 0x01, 3, 1, 0, 1, 14, "Port  1", "amc.MLVDS_P17R" }, // OUT |             | Non-inversed | 
  { 0x02, 3, 0, 1, 1,  0, "Port  2", "amc.MLVDS_P18T" }, // IN  | Termination | Non-inversed | 
  { 0x03, 3, 1, 0, 1, 13, "Port  3", "amc.MLVDS_P18R" }, // OUT |             | Non-inversed | 
  { 0x04, 3, 0, 1, 1,  0, "Port  4", "amc.MLVDS_P19T" }, // IN  | Termination | Non-inversed | 
  { 0x05, 3, 0, 1, 1,  0, "Port  5", "amc.MLVDS_P19R" }, // IN  | Termination | Non-inversed | 
  { 0x06, 3, 0, 1, 1,  0, "Port  6", "amc.MLVDS_P20T" }, // IN  | Termination | Non-inversed | 
  { 0x07, 3, 0, 1, 1,  0, "Port  7", "amc.MLVDS_P20R" }, // IN  | Termination | Non-inversed | 
  { 0x08, 3, 1, 0, 1,  2, "Port  8", "clk.TRIG_F_0"   }, // OUT |             | Non-inversed | 
  { 0x09, 3, 1, 0, 1,  5, "Port  9", "clk.TRIG_F_1"   }, // OUT |             | Non-inversed | 
  { 0x0A, 3, 1, 0, 1,  4, "Port 10", "clk.TRIG_F_2"   }, // OUT |             | Non-inversed | 
  { 0x0B, 3, 1, 0, 1,  7, "Port 11", "clk.TRIG_F_3"   }, // OUT |             | Non-inversed | 
  { 0x0C, 3, 1, 0, 1,  6, "Port 12", "clk.TRIG_F_4"   }, // OUT |             | Non-inversed | 
  { 0x0D, 3, 0, 1, 1,  0, "Port 13", "clk.TRIG_F_5"   }, // IN  | Termination | Non-inversed | 
  { 0x0E, 3, 0, 1, 1,  0, "Port 14", "clk.TRIG_F_6"   }, // IN  | Termination | Non-inversed | 
  { 0x0F, 3, 0, 1, 1,  0, "Port 15", "clk.TRIG_F_7"   }, // IN  | Termination | Non-inversed | 
  { 0x00, 0, 0, 1, 1,  0, NULL,      NULL             }
};
#else
static const TIdtTbl idt_trig_reg[] =
{
  { 0x00, 3, 0, 1, 1,  0, "Port  0", "amc.MLVDS_P17T" }, // IN  | Termination | Non-inversed | 
  { 0x01, 3, 1, 0, 1, 14, "Port  1", "amc.MLVDS_P17R" }, // OUT |             | Non-inversed | 
  { 0x02, 3, 0, 1, 1,  0, "Port  2", "amc.MLVDS_P18T" }, // IN  | Termination | Non-inversed | 
  { 0x03, 3, 1, 0, 1, 13, "Port  3", "amc.MLVDS_P18R" }, // OUT |             | Non-inversed | 
  { 0x04, 3, 0, 1, 1,  0, "Port  4", "amc.MLVDS_P19T" }, // IN  | Termination | Non-inversed | 
  { 0x05, 3, 0, 1, 1,  0, "Port  5", "amc.MLVDS_P19R" }, // IN  | Termination | Non-inversed | 
  { 0x06, 3, 1, 0, 1, 14, "Port  6", "amc.MLVDS_P20T" }, // IN  | Termination | Non-inversed | 
  { 0x07, 3, 1, 0, 1, 13, "Port  7", "amc.MLVDS_P20R" }, // IN  | Termination | Non-inversed | 
  { 0x08, 3, 1, 0, 1,  2, "Port  8", "clk.TRIG_F_0"   }, // OUT |             | Non-inversed | 
  { 0x09, 3, 1, 0, 1,  5, "Port  9", "clk.TRIG_F_1"   }, // OUT |             | Non-inversed | 
  { 0x0A, 3, 1, 0, 1,  4, "Port 10", "clk.TRIG_F_2"   }, // OUT |             | Non-inversed | 
  { 0x0B, 3, 1, 0, 1,  7, "Port 11", "clk.TRIG_F_3"   }, // OUT |             | Non-inversed | 
  { 0x0C, 3, 1, 0, 1,  6, "Port 12", "clk.TRIG_F_4"   }, // OUT |             | Non-inversed | 
  { 0x0D, 3, 0, 1, 1,  0, "Port 13", "clk.TRIG_F_5"   }, // IN  | Termination | Non-inversed | 
  { 0x0E, 3, 0, 1, 1,  0, "Port 14", "clk.TRIG_F_6"   }, // IN  | Termination | Non-inversed | 
  { 0x0F, 3, 0, 1, 1,  0, "Port 15", "clk.TRIG_F_7"   }, // IN  | Termination | Non-inversed | 
  { 0x00, 0, 0, 1, 1,  0, NULL,      NULL             }
};


#endif

//==============================================================================
//----===@ class
//==============================================================================

#define Test_drMpsPico 


//=====================================
//----===@ ts2ipZqsys
// Parameters  :
// Description :
ts2ipZqsys::ts2ipZqsys(const char *deviceName)
{
  ip_open(deviceName);


  psiic = new drxiic(DRV_NAME_psiic);

  idt_clk_id = -1;
  idt_data_id = -1;

  idt_clk_id = psiic->iic_addChip("idt_clk" ,0x58,0,0);
  idt_data_id = psiic->iic_addChip("idt_data",0x59,0,0);

  psiic->iic_prnInfo();

  char txBuf[32];
  // char rxBuf[32];
  int i;
  // int bReads = 0;
  int bWrites = 0;

  for(i=0;i<16;i++)
  {
    txBuf[i] =  (idt_trig_reg[i].outMode << 7) | \
                (idt_trig_reg[i].termOn << 6) | \
                (idt_trig_reg[i].nonInvert << 5) | \
                idt_trig_reg[i].outPortSel;
    // printf("txBuf %02d : 0x%02x \r\n",i, txBuf[i]);
  }

  bWrites = psiic->iic_wr(idt_data_id, 0, 0, txBuf, 16);

  // printf("iic_wr bWrites : %d\r\n", bWrites);
  // bReads = psiic->iic_rd(idt_data_id, 0, 0, rxBuf, 16);
  // for(i=0;i<16;i++)
  //   printf("iic_rd rxBuf %02d : 0x%02x / %d\r\n",i, rxBuf[i], bReads);

}

//=====================================
//----===@ ~ts2ipZqsys
// Parameters  :
// Description :
ts2ipZqsys::~ts2ipZqsys()
{
  prnM2("~ts2ipZqsys();\r\n");
  delete psiic;
}




//=====================================
//----===@ getStat
// Parameters  :
// Description :
int ts2ipZqsys::getStat(void)
{
  ifRet(fd < 0);
  uint rdData;

  ip_rd(accType_devRaw, slv_reg12, &rdData); 
  sfp_present  = ((!rdData >>  8) & 0x1);
  sfp_txFault  = ((rdData >>  4) & 0x1);
  sfp_loss     = ((rdData >>  0) & 0x1);

  ip_rd(accType_devRaw, slv_reg32, &rdData); 
  sfp_cs1  = ((rdData >>  1) & 0x1);
  sfp_cs0  = ((rdData >>  0) & 0x1);
  
  return RET_OK;
}

//=====================================
//----===@ prnStat
// Parameters  :
// Description :
int ts2ipZqsys::prnStat(void)
{
  prnM2("======================================================\r\n");
  prnM2("++ %s prnStat ++\r\n", devName);
  prnM2("------------------------------------------------------\r\n");
  
  ifRet(fd < 0);
  getStat();

  prnM2("%-12s: %d\r\n", "sfp_present", sfp_present);
  prnM2("%-12s: %d\r\n", "sfp_txFault", sfp_txFault);
  prnM2("%-12s: %d\r\n", "sfp_loss   ", sfp_loss   );
  prnM2("%-12s: %d\r\n", "sfp_cs0    ", sfp_cs0    );
  prnM2("%-12s: %d\r\n", "sfp_cs1    ", sfp_cs1    );

  prnM2("\r\n");

  return RET_OK;
}


} //name space end





