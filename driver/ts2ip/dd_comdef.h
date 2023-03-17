/*******************************************************************************
 *                                                                             *
 *  Copyright (c) 2014 ~ by DURUTRONIX. All Rights Reserved.                   *
 *                                                                             *
 ******************************************************************************/

/*==============================================================================
                        EDIT HISTORY FOR MODULE

when                who            what, where, why
------------------- -------------  ---------------------------------------------
2019-02-22 14:53:53 laykim         Create
==============================================================================*/
#if !defined( __dd_comdef_h__ )
#define __dd_comdef_h__

#ifdef __cplusplus
extern "C" {
#endif

#define slv_reg00 0x000
#define slv_reg01 0x004
#define slv_reg02 0x008
#define slv_reg03 0x00C
#define slv_reg04 0x010
#define slv_reg05 0x014
#define slv_reg06 0x018
#define slv_reg07 0x01C
#define slv_reg08 0x020
#define slv_reg09 0x024
#define slv_reg0a 0x028
#define slv_reg0b 0x02C
#define slv_reg0c 0x030
#define slv_reg0d 0x034
#define slv_reg0e 0x038
#define slv_reg0f 0x03C

#define slv_reg10 0x040
#define slv_reg11 0x044
#define slv_reg12 0x048
#define slv_reg13 0x04C
#define slv_reg14 0x050
#define slv_reg15 0x054
#define slv_reg16 0x058
#define slv_reg17 0x05C
#define slv_reg18 0x060
#define slv_reg19 0x064
#define slv_reg1a 0x068
#define slv_reg1b 0x06C
#define slv_reg1c 0x070
#define slv_reg1d 0x074
#define slv_reg1e 0x078
#define slv_reg1f 0x07C

#define slv_reg20 0x080
#define slv_reg21 0x084
#define slv_reg22 0x088
#define slv_reg23 0x08C
#define slv_reg24 0x090
#define slv_reg25 0x094
#define slv_reg26 0x098
#define slv_reg27 0x09C
#define slv_reg28 0x0A0
#define slv_reg29 0x0A4
#define slv_reg2a 0x0A8
#define slv_reg2b 0x0AC
#define slv_reg2c 0x0B0
#define slv_reg2d 0x0B4
#define slv_reg2e 0x0B8
#define slv_reg2f 0x0BC

#define slv_reg30 0x0C0
#define slv_reg31 0x0C4
#define slv_reg32 0x0C8
#define slv_reg33 0x0CC
#define slv_reg34 0x0D0
#define slv_reg35 0x0D4
#define slv_reg36 0x0D8
#define slv_reg37 0x0DC
#define slv_reg38 0x0E0
#define slv_reg39 0x0E4
#define slv_reg3a 0x0E8
#define slv_reg3b 0x0EC
#define slv_reg3c 0x0F0
#define slv_reg3d 0x0F4
#define slv_reg3e 0x0F8
#define slv_reg3f 0x0FC

#define slv_reg40 0x100
#define slv_reg41 0x104
#define slv_reg42 0x108
#define slv_reg43 0x10C
#define slv_reg44 0x110
#define slv_reg45 0x114
#define slv_reg46 0x118
#define slv_reg47 0x11C
#define slv_reg48 0x120
#define slv_reg49 0x124
#define slv_reg4a 0x128
#define slv_reg4b 0x12C
#define slv_reg4c 0x130
#define slv_reg4d 0x134
#define slv_reg4e 0x138
#define slv_reg4f 0x13C

#define slv_reg50 0x140
#define slv_reg51 0x144
#define slv_reg52 0x148
#define slv_reg53 0x14C
#define slv_reg54 0x150
#define slv_reg55 0x154
#define slv_reg56 0x158
#define slv_reg57 0x15C
#define slv_reg58 0x160
#define slv_reg59 0x164
#define slv_reg5a 0x168
#define slv_reg5b 0x16C
#define slv_reg5c 0x170
#define slv_reg5d 0x174
#define slv_reg5e 0x178
#define slv_reg5f 0x17C

#define slv_reg60 0x180
#define slv_reg61 0x184
#define slv_reg62 0x188
#define slv_reg63 0x18C
#define slv_reg64 0x190
#define slv_reg65 0x194
#define slv_reg66 0x198
#define slv_reg67 0x19C
#define slv_reg68 0x1A0
#define slv_reg69 0x1A4
#define slv_reg6a 0x1A8
#define slv_reg6b 0x1AC
#define slv_reg6c 0x1B0
#define slv_reg6d 0x1B4
#define slv_reg6e 0x1B8
#define slv_reg6f 0x1BC

#define slv_reg70 0x1C0
#define slv_reg71 0x1C4
#define slv_reg72 0x1C8
#define slv_reg73 0x1CC
#define slv_reg74 0x1D0
#define slv_reg75 0x1D4
#define slv_reg76 0x1D8
#define slv_reg77 0x1DC
#define slv_reg78 0x1E0
#define slv_reg79 0x1E4
#define slv_reg7a 0x1E8
#define slv_reg7b 0x1EC
#define slv_reg7c 0x1F0
#define slv_reg7d 0x1F4
#define slv_reg7e 0x1F8
#define slv_reg7f 0x1FC


#define A_vendor                  slv_reg00
#define A_ipInfo                  slv_reg01
#define A_axi_aclk_freq           slv_reg02
#define A_axi_aclk_cntr           slv_reg03

//==============================================================================
//----===@ define user register map Address
//==============================================================================


//==============================================================================
//----===@ 
//==============================================================================
struct st_regRW
{
    unsigned int offset;     
    unsigned int val;
};

#define IOCTL_R           _IOWR (MAGIC_NUM, 1, struct st_regRW)
#define IOCTL_W           _IOWR (MAGIC_NUM, 2, struct st_regRW)

#ifdef __cplusplus
}
#endif
#endif
/*==============================================================================
                                  END OF FILE
==============================================================================*/
