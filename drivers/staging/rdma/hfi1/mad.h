/*
 *
 * This file is provided under a dual BSD/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 *
 * GPL LICENSE SUMMARY
 *
 * Copyright(c) 2015 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * BSD LICENSE
 *
 * Copyright(c) 2015 Intel Corporation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Intel Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef _HFI1_MAD_H
#define _HFI1_MAD_H

#include <rdma/ib_pma.h>
#define USE_PI_LED_ENABLE	1 /* use led enabled bit in struct
				   * opa_port_states, if available */
#include <rdma/opa_smi.h>
#include <rdma/opa_port_info.h>
#ifndef PI_LED_ENABLE_SUP
#define PI_LED_ENABLE_SUP 0
#endif
#include "opa_compat.h"

#define IB_VLARB_LOWPRI_0_31    1
#define IB_VLARB_LOWPRI_32_63   2
#define IB_VLARB_HIGHPRI_0_31   3
#define IB_VLARB_HIGHPRI_32_63  4

#define OPA_MAX_PREEMPT_CAP         32
#define OPA_VLARB_LOW_ELEMENTS       0
#define OPA_VLARB_HIGH_ELEMENTS      1
#define OPA_VLARB_PREEMPT_ELEMENTS   2
#define OPA_VLARB_PREEMPT_MATRIX     3

#define IB_PMA_PORT_COUNTERS_CONG       cpu_to_be16(0xFF00)

struct ib_pma_portcounters_cong {
	u8 reserved;
	u8 reserved1;
	__be16 port_check_rate;
	__be16 symbol_error_counter;
	u8 link_error_recovery_counter;
	u8 link_downed_counter;
	__be16 port_rcv_errors;
	__be16 port_rcv_remphys_errors;
	__be16 port_rcv_switch_relay_errors;
	__be16 port_xmit_discards;
	u8 port_xmit_constraint_errors;
	u8 port_rcv_constraint_errors;
	u8 reserved2;
	u8 link_overrun_errors; /* LocalLink: 7:4, BufferOverrun: 3:0 */
	__be16 reserved3;
	__be16 vl15_dropped;
	__be64 port_xmit_data;
	__be64 port_rcv_data;
	__be64 port_xmit_packets;
	__be64 port_rcv_packets;
	__be64 port_xmit_wait;
	__be64 port_adr_events;
} __packed;

#define IB_SMP_UNSUP_VERSION    cpu_to_be16(0x0004)
#define IB_SMP_UNSUP_METHOD     cpu_to_be16(0x0008)
#define IB_SMP_UNSUP_METH_ATTR  cpu_to_be16(0x000C)
#define IB_SMP_INVALID_FIELD    cpu_to_be16(0x001C)

#define OPA_MAX_PREEMPT_CAP         32
#define OPA_VLARB_LOW_ELEMENTS       0
#define OPA_VLARB_HIGH_ELEMENTS      1
#define OPA_VLARB_PREEMPT_ELEMENTS   2
#define OPA_VLARB_PREEMPT_MATRIX     3

#define HFI1_XMIT_RATE_UNSUPPORTED               0x0
#define HFI1_XMIT_RATE_PICO                      0x7
/* number of 4nsec cycles equaling 2secs */
#define HFI1_CONG_TIMER_PSINTERVAL               0x1DCD64EC

#define IB_CC_SVCTYPE_RC 0x0
#define IB_CC_SVCTYPE_UC 0x1
#define IB_CC_SVCTYPE_RD 0x2
#define IB_CC_SVCTYPE_UD 0x3

/*
 * There should be an equivalent IB #define for the following, but
 * I cannot find it.
 */
#define OPA_CC_LOG_TYPE_HFI	2

struct opa_hfi1_cong_log_event_internal {
	u32 lqpn;
	u32 rqpn;
	u8 sl;
	u8 svc_type;
	u32 rlid;
	s64 timestamp; /* wider than 32 bits to detect 32 bit rollover */
};

struct opa_hfi1_cong_log_event {
	u8 local_qp_cn_entry[3];
	u8 remote_qp_number_cn_entry[3];
	u8 sl_svc_type_cn_entry; /* 5 bits SL, 3 bits svc type */
	u8 reserved;
	__be32 remote_lid_cn_entry;
	__be32 timestamp_cn_entry;
} __packed;

#define OPA_CONG_LOG_ELEMS	96

struct opa_hfi1_cong_log {
	u8 log_type;
	u8 congestion_flags;
	__be16 threshold_event_counter;
	__be32 current_time_stamp;
	u8 threshold_cong_event_map[OPA_MAX_SLS/8];
	struct opa_hfi1_cong_log_event events[OPA_CONG_LOG_ELEMS];
} __packed;

#define IB_CC_TABLE_CAP_DEFAULT 31

/* Port control flags */
#define IB_CC_CCS_PC_SL_BASED 0x01

struct opa_congestion_setting_entry {
	u8 ccti_increase;
	u8 reserved;
	__be16 ccti_timer;
	u8 trigger_threshold;
	u8 ccti_min; /* min CCTI for cc table */
} __packed;

struct opa_congestion_setting_entry_shadow {
	u8 ccti_increase;
	u8 reserved;
	u16 ccti_timer;
	u8 trigger_threshold;
	u8 ccti_min; /* min CCTI for cc table */
} __packed;

struct opa_congestion_setting_attr {
	__be32 control_map;
	__be16 port_control;
	struct opa_congestion_setting_entry entries[OPA_MAX_SLS];
} __packed;

struct opa_congestion_setting_attr_shadow {
	u32 control_map;
	u16 port_control;
	struct opa_congestion_setting_entry_shadow entries[OPA_MAX_SLS];
} __packed;

#define IB_CC_TABLE_ENTRY_INCREASE_DEFAULT 1
#define IB_CC_TABLE_ENTRY_TIMER_DEFAULT 1

/* 64 Congestion Control table entries in a single MAD */
#define IB_CCT_ENTRIES 64
#define IB_CCT_MIN_ENTRIES (IB_CCT_ENTRIES * 2)

struct ib_cc_table_entry {
	__be16 entry; /* shift:2, multiplier:14 */
};

struct ib_cc_table_entry_shadow {
	u16 entry; /* shift:2, multiplier:14 */
};

struct ib_cc_table_attr {
	__be16 ccti_limit; /* max CCTI for cc table */
	struct ib_cc_table_entry ccti_entries[IB_CCT_ENTRIES];
} __packed;

struct ib_cc_table_attr_shadow {
	u16 ccti_limit; /* max CCTI for cc table */
	struct ib_cc_table_entry_shadow ccti_entries[IB_CCT_ENTRIES];
} __packed;

#define CC_TABLE_SHADOW_MAX \
	(IB_CC_TABLE_CAP_DEFAULT * IB_CCT_ENTRIES)

struct cc_table_shadow {
	u16 ccti_limit; /* max CCTI for cc table */
	struct ib_cc_table_entry_shadow entries[CC_TABLE_SHADOW_MAX];
} __packed;

/*
 * struct cc_state combines the (active) per-port congestion control
 * table, and the (active) per-SL congestion settings. cc_state data
 * may need to be read in code paths that we want to be fast, so it
 * is an RCU protected structure.
 */
struct cc_state {
	struct rcu_head rcu;
	struct cc_table_shadow cct;
	struct opa_congestion_setting_attr_shadow cong_setting;
};

/*
 * OPA BufferControl MAD
 */

/* attribute modifier macros */
#define OPA_AM_NPORT_SHIFT	24
#define OPA_AM_NPORT_MASK	0xff
#define OPA_AM_NPORT_SMASK	(OPA_AM_NPORT_MASK << OPA_AM_NPORT_SHIFT)
#define OPA_AM_NPORT(am)	(((am) >> OPA_AM_NPORT_SHIFT) & \
					OPA_AM_NPORT_MASK)

#define OPA_AM_NBLK_SHIFT	24
#define OPA_AM_NBLK_MASK	0xff
#define OPA_AM_NBLK_SMASK	(OPA_AM_NBLK_MASK << OPA_AM_NBLK_SHIFT)
#define OPA_AM_NBLK(am)		(((am) >> OPA_AM_NBLK_SHIFT) & \
					OPA_AM_NBLK_MASK)

#define OPA_AM_START_BLK_SHIFT	0
#define OPA_AM_START_BLK_MASK	0xff
#define OPA_AM_START_BLK_SMASK	(OPA_AM_START_BLK_MASK << \
					OPA_AM_START_BLK_SHIFT)
#define OPA_AM_START_BLK(am)	(((am) >> OPA_AM_START_BLK_SHIFT) & \
					OPA_AM_START_BLK_MASK)

#define OPA_AM_PORTNUM_SHIFT	0
#define OPA_AM_PORTNUM_MASK	0xff
#define OPA_AM_PORTNUM_SMASK	(OPA_AM_PORTNUM_MASK << OPA_AM_PORTNUM_SHIFT)
#define OPA_AM_PORTNUM(am)	(((am) >> OPA_AM_PORTNUM_SHIFT) & \
					OPA_AM_PORTNUM_MASK)

#define OPA_AM_ASYNC_SHIFT	12
#define OPA_AM_ASYNC_MASK	0x1
#define OPA_AM_ASYNC_SMASK	(OPA_AM_ASYNC_MASK << OPA_AM_ASYNC_SHIFT)
#define OPA_AM_ASYNC(am)	(((am) >> OPA_AM_ASYNC_SHIFT) & \
					OPA_AM_ASYNC_MASK)

#define OPA_AM_START_SM_CFG_SHIFT	9
#define OPA_AM_START_SM_CFG_MASK	0x1
#define OPA_AM_START_SM_CFG_SMASK	(OPA_AM_START_SM_CFG_MASK << \
						OPA_AM_START_SM_CFG_SHIFT)
#define OPA_AM_START_SM_CFG(am)		(((am) >> OPA_AM_START_SM_CFG_SHIFT) \
						& OPA_AM_START_SM_CFG_MASK)

#define OPA_AM_CI_ADDR_SHIFT	19
#define OPA_AM_CI_ADDR_MASK	0xfff
#define OPA_AM_CI_ADDR_SMASK	(OPA_AM_CI_ADDR_MASK << OPA_CI_ADDR_SHIFT)
#define OPA_AM_CI_ADDR(am)	(((am) >> OPA_AM_CI_ADDR_SHIFT) & \
					OPA_AM_CI_ADDR_MASK)

#define OPA_AM_CI_LEN_SHIFT	13
#define OPA_AM_CI_LEN_MASK	0x3f
#define OPA_AM_CI_LEN_SMASK	(OPA_AM_CI_LEN_MASK << OPA_CI_LEN_SHIFT)
#define OPA_AM_CI_LEN(am)	(((am) >> OPA_AM_CI_LEN_SHIFT) & \
					OPA_AM_CI_LEN_MASK)

/* error info macros */
#define OPA_EI_STATUS_SMASK	0x80
#define OPA_EI_CODE_SMASK	0x0f

struct vl_limit {
	__be16 dedicated;
	__be16 shared;
};

struct buffer_control {
	__be16 reserved;
	__be16 overall_shared_limit;
	struct vl_limit vl[OPA_MAX_VLS];
};

struct sc2vlnt {
	u8 vlnt[32]; /* 5 bit VL, 3 bits reserved */
};

/*
 * The PortSamplesControl.CounterMasks field is an array of 3 bit fields
 * which specify the N'th counter's capabilities. See ch. 16.1.3.2.
 * We support 5 counters which only count the mandatory quantities.
 */
#define COUNTER_MASK(q, n) (q << ((9 - n) * 3))
#define COUNTER_MASK0_9 \
	cpu_to_be32(COUNTER_MASK(1, 0) | \
		    COUNTER_MASK(1, 1) | \
		    COUNTER_MASK(1, 2) | \
		    COUNTER_MASK(1, 3) | \
		    COUNTER_MASK(1, 4))

#endif				/* _HFI1_MAD_H */
