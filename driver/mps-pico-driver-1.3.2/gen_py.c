
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "mps_pico.h"
#include "mps_pico_version.h"

int main(int argc, char *argv[])
{
    struct trg_ctrl trg;
    struct pico_float_coeff float_coeff;
    FILE *out = stdout;

    if(argc>1) {
        out = fopen(argv[1], "w");
        if(!out) {
            perror("Failed to open output file");
            return 1;
        }
    }

    fprintf(out, "# Generated by gen_py from " MPS_PICO_VERSION "\n");

#define EMIT(N) fprintf(out, #N " = 0x%lx\n", (unsigned long)N)
    EMIT(GET_VERSION);
    EMIT(GET_VERSION_CURRENT);
    EMIT(SET_RANGE);
    EMIT(GET_RANGE);
    EMIT(SET_FSAMP);
    EMIT(GET_FSAMP);
    EMIT(GET_B_TRANS);
    EMIT(SET_TRG);
    EMIT(SET_RING_BUF);
    EMIT(SET_GATE_MUX);
    EMIT(SET_CONV_MUX);
    EMIT(GET_THRESHOLD_FLAG);
    EMIT(GET_THRESHOLD);
    EMIT(SET_THRESHOLD);
    EMIT(GET_NSAMP);
    EMIT(SET_NSAMP);
    EMIT(GET_MPS_CONFIG);
    EMIT(SET_MPS_CONFIG);
    EMIT(SET_MPS_CTRL);
    EMIT(GET_TRG_ACC_DATA);
    EMIT(GET_TRG_CNTR_1M);
    EMIT(GET_TRG_CNTR_1K);
    EMIT(SET_TRG_NRSAMP);
    EMIT(GET_TRG_NRSAMP);
    EMIT(SET_TRG_DELAY);
    EMIT(GET_TRG_DELAY);
    EMIT(ABORT_READ);
    EMIT(USER_SITE_NONE);
    EMIT(USER_SITE_FRIB);
    EMIT(GET_SITE_ID);
    EMIT(GET_SITE_VERSION);
    EMIT(SET_SITE_MODE);
#undef EMIT

    fprintf(out,
            "import ctypes\n"
            "class trg_ctrl(ctypes.Structure):\n"
            "    _pack_ = 1\n"
            "    _fields_ = (('limit', ctypes.c_float),\n"
            "               ('nr_samp', ctypes.c_uint32),\n"
            "               ('ch_sel', ctypes.c_uint32),\n"
            "               ('mode', ctypes.c_int),\n"
            "              )\n"
            "class pico_float_coeff(ctypes.Structure):\n"
            "    _pack_ = 1\n"
            "    _fields_ = (('data', ctypes.c_uint32),\n"
            "               ('addr', ctypes.c_uint32),\n"
            "              )\n"
            "    DISABLED = 0\n"
            "    POS_EDGE = 1\n"
            "    NEG_EDGE = 2\n"
            "    BOTH_EDGE = 3\n"
            );

    /* verify that struct packing is consistent */
    fprintf(out, "assert trg_ctrl.limit.offset==%lu\n", offsetof(struct trg_ctrl, limit));
    fprintf(out, "assert trg_ctrl.limit.size==%lu\n", sizeof(trg.limit));

    fprintf(out, "assert trg_ctrl.nr_samp.offset==%lu\n", offsetof(struct trg_ctrl, nr_samp));
    fprintf(out, "assert trg_ctrl.nr_samp.size==%lu\n", sizeof(trg.nr_samp));

    fprintf(out, "assert trg_ctrl.ch_sel.offset==%lu\n", offsetof(struct trg_ctrl, ch_sel));
    fprintf(out, "assert trg_ctrl.ch_sel.size==%lu\n", sizeof(trg.ch_sel));

    fprintf(out, "assert trg_ctrl.mode.offset==%lu\n", offsetof(struct trg_ctrl, mode));
    fprintf(out, "assert trg_ctrl.mode.size==%lu\n", sizeof(trg.mode));

    fprintf(out, "assert pico_float_coeff.data.offset==%lu\n", offsetof(struct pico_float_coeff, data));
    fprintf(out, "assert pico_float_coeff.data.size==%lu\n", sizeof(float_coeff.data));

    fprintf(out, "assert pico_float_coeff.addr.offset==%lu\n", offsetof(struct pico_float_coeff, addr));
    fprintf(out, "assert pico_float_coeff.addr.size==%lu\n", sizeof(float_coeff.addr));

    return 0;
}
