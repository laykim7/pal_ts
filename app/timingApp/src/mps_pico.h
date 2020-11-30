/*
 * MPS-Pico8 Linux Driver
 *
 *  Copyright (C) 2018 CAEN ELS s.r.l.
 *
 *  Paolo Scarbolo <p.scarbolo@caenels.com>
 *
 *  Copyright 2016 Board of Trustees of Michigan State University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file
 * \brief User-space interface
 */

#ifndef MPS_PICO_H_
#define MPS_PICO_H_

#include <asm/ioctl.h>
#include <linux/types.h>

#ifndef __KERNEL__
#include <stdint.h>
#endif


/** AMC-Pico magic number */
#define	MPS_PICO_MAGIC	'P'

/** Structure for trigger control */
struct __attribute__((__packed__)) trg_ctrl {
	float limit;
	uint32_t nr_samp;
	uint32_t ch_sel;
	enum mode_t {DISABLED, POS_EDGE, NEG_EDGE, BOTH_EDGE} mode;
};

/** Structure to set float coeff + address*/
struct __attribute__((__packed__)) pico_float_coeff {
	uint32_t data;
	uint32_t addr;
};

/* Driver interface version number.
 * GET_VERSION_CURRENT is current version.
 * Initialize integer w/ zero to identify version zero interface
 * which does not fail unknown requests (errno==EINVAL implies version zero)
 @code
  int ver = 0;
  ioctl(fd, GET_VERSION, &ver);
  if(ver!=GET_VERSION_CURRENT) { oops ... }
 @endcode
 */
#define GET_VERSION	_IOR(MPS_PICO_MAGIC, 10, uint32_t)
#define GET_VERSION_CURRENT 3

/** Sets the picoammeter range, each bit sets the individual channel,
 * RNG0 is the higher current range
 */
#define SET_RANGE	_IOWR(MPS_PICO_MAGIC, 11, uint8_t)

/** Gets picoammeter range */
#define GET_RANGE	_IOR(MPS_PICO_MAGIC, 14, uint8_t)

/** Sets the picoammeter sampling frequency (in Hz) */
#define SET_FSAMP	_IOWR(MPS_PICO_MAGIC, 12, uint32_t)

/** Gets the picoammeter sampling frequency (in Hz) */
#define GET_FSAMP	_IOR(MPS_PICO_MAGIC, 13, uint32_t)

/** Gets Over Threshold Flags */
#define GET_THRESHOLD_FLAG	_IOWR(MPS_PICO_MAGIC, 20, struct pico_float_coeff)
#define GET_THRESHOLD		_IOWR(MPS_PICO_MAGIC, 21, struct pico_float_coeff)
#define SET_THRESHOLD		_IOW(MPS_PICO_MAGIC, 22, struct pico_float_coeff)
#define GET_NSAMP			_IOWR(MPS_PICO_MAGIC, 23, struct pico_float_coeff)
#define SET_NSAMP			_IOW(MPS_PICO_MAGIC, 24, struct pico_float_coeff)

/** Get and Set MPS Configuration register **/
#define GET_MPS_CONFIG	_IOR(MPS_PICO_MAGIC, 25, uint32_t)
#define SET_MPS_CONFIG	_IOWR(MPS_PICO_MAGIC, 26, uint32_t)
#define SET_MPS_CTRL 	_IOWR(MPS_PICO_MAGIC, 27, uint32_t)
#define GET_MPS_MANUAL	_IOR(MPS_PICO_MAGIC, 28, uint32_t)
#define SET_MPS_MANUAL	_IOWR(MPS_PICO_MAGIC, 29, uint32_t)
#define GET_MPS_STATUS	_IOR(MPS_PICO_MAGIC, 30, uint32_t)

/** Gets number of bytes last DMA transfer succesfully transfered */
#define GET_B_TRANS	_IOR(MPS_PICO_MAGIC, 40, uint32_t)

/** Set and Get Trigger parameters */
#define SET_TRG				_IOWR(MPS_PICO_MAGIC, 50, struct trg_ctrl)
#define GET_RT_DATA_1M    	_IOWR(MPS_PICO_MAGIC, 51, struct pico_float_coeff)
#define GET_TRG_CNTR_1M		_IOWR(MPS_PICO_MAGIC, 52, uint32_t)
#define GET_TRG_CNTR_1K		_IOWR(MPS_PICO_MAGIC, 53, uint32_t)
#define SET_TRG_NRSAMP		_IOW(MPS_PICO_MAGIC, 54, uint32_t)
#define GET_TRG_NRSAMP		_IOWR(MPS_PICO_MAGIC, 55, uint32_t)
#define SET_TRG_DELAY		_IOW(MPS_PICO_MAGIC, 56, uint32_t)
#define GET_TRG_DELAY		_IOWR(MPS_PICO_MAGIC, 57, uint32_t)
#define GET_RT_DATA_1K    	_IOWR(MPS_PICO_MAGIC, 58, struct pico_float_coeff)
#define GET_MA_BUFFER_INDEX _IOWR(MPS_PICO_MAGIC, 59, uint32_t)

/** Sets ring buffer (pre-trigger storage) parameters */
#define SET_RING_BUF	_IOW(MPS_PICO_MAGIC, 60,  uint32_t)

/** Sets gate mux */
#define SET_GATE_MUX	_IOWR(MPS_PICO_MAGIC, 70,  uint32_t)

/** Sets convert signal mux */
#define SET_CONV_MUX	_IOWR(MPS_PICO_MAGIC, 80,  uint32_t)

/** Abort in progress read() w/o closing FD */
#define ABORT_READ _IO(MPS_PICO_MAGIC, 81)

#define USER_SITE_NONE 0xcae4
#define USER_SITE_FRIB 0xf41B

/** Identify user/site firmware customization */
#define GET_SITE_ID _IOR(MPS_PICO_MAGIC, 91, uint32_t)
#define GET_SITE_VERSION _IOR(MPS_PICO_MAGIC, 92, uint32_t)
#define SET_SITE_MODE _IOW(MPS_PICO_MAGIC, 92, uint32_t)

#endif /* MPS_PICO_H_ */
