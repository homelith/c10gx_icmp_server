//------------------------------------------------------------------------------
// icmp_proc.cpp
//
// i++ HLS module for ICMP (IPv4 Ping) processing
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2020 homelith
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//------------------------------------------------------------------------------

#include "HLS/stdio.h"
#include "HLS/hls.h"
#include "HLS/math.h"
#include "HLS/extendedmath.h"
#include "HLS/ac_int.h"
#include "HLS/ac_fixed.h"

typedef ac_int<64,false> UINT64;
typedef ac_int<48,false> UINT48;
typedef ac_int<32,false> UINT32;
typedef ac_int<24,false> UINT24;
typedef ac_int<16,false> UINT16;
typedef ac_int<8,false>  UINT8;

typedef struct L8_AVALON_ST_PKT_tag{
	UINT64 data;
	UINT8  sop;
	UINT8  eop;
	UINT8  empty;
	UINT8  padding_no_use[5];
} L8_AVALON_ST_PKT;

typedef ihc::stream_in  <L8_AVALON_ST_PKT, ihc::buffer<8>, ihc::bitsPerSymbol<8> > L8_AVALON_ST_PKT_RX_BUF8;
typedef ihc::stream_in  <L8_AVALON_ST_PKT, ihc::bitsPerSymbol<8> > L8_AVALON_ST_PKT_RX;
typedef ihc::stream_out <L8_AVALON_ST_PKT, ihc::bitsPerSymbol<8> > L8_AVALON_ST_PKT_TX;

// ICMP Packet Format
// 12-13 : eth type = 0x0800(IPv4)
// 14    : header len = 0x45
// 15    : TOS = 0x00
// 16-17 : total len = 76 = 0x004c
// 18-19 : ident (sequence number?)
// 20-21 : fragment = 0x4000 (do not make fragment)
// 22    : TTL
// 23    : Proto = ICMP (0x01)
// 24-25 : checksum
// 26-29 : src ip
// 30-33 : dst ip
// 34    : type (08: request, 00: reply)
// 35    : code (00)
// 36-37 : checksum
// 38-39 : identifier
// 40-41 : sequence number
// 42-   : payload

UINT32 test_icmp_req_len = 10;
L8_AVALON_ST_PKT test_icmp_req[10] = {
	{0x00005e00016654bfULL, 1, 0, 0},
	{0x648042d808004500ULL, 0, 0, 0},
	{0x003c02de00008001ULL, 0, 0, 0},
	{0x0000ac1f18873dd3ULL, 0, 0, 0},
	{0xe204080030f50001ULL, 0, 0, 0},
	{0x1c66616263646566ULL, 0, 0, 0},
	{0x6768696a6b6c6d6eULL, 0, 0, 0},
	{0x6f70717273747576ULL, 0, 0, 0},
	{0x7761626364656667ULL, 0, 0, 0},
	{0x6869000000000000ULL, 0, 1, 6},
};

#include "hls_component_param"
component void icmp_proc_swapmac(
	L8_AVALON_ST_PKT_RX_BUF8 &ch_dat_in,
	L8_AVALON_ST_PKT_TX      &ch_dat_out
){
	static L8_AVALON_ST_PKT stage2;
	static L8_AVALON_ST_PKT stage1;
	static L8_AVALON_ST_PKT stage0;
	static UINT8            stage2_idx;
	static UINT8            stage1_idx;
	static UINT8            stage0_idx;
	static bool             stage2_en = false;
	static bool             stage1_en = false;
	static bool             stage0_en = false;

	static UINT8            idx_counter = 0;
	// output stage
	if (stage2_en) {
		stage2_en = !( ch_dat_out.tryWrite(stage2) );
	}

	// stage 2
	bool swap_dst_op_en = false;
	if (! stage2_en & stage1_en) {
		if (stage1_idx != 0) {
			// normal bypass operation
			stage2     = stage1;
			stage2_idx = stage1_idx;
			stage2_en  = true;
			stage1_en  = false;
		} else if (stage0_en & (stage0_idx == 1)) {
			// swapping dst mac and src mac
			stage2.data = ((stage1.data << 48) & 0xFFFF000000000000ULL)
				      | ((stage0.data >> 16) & 0x0000FFFFFFFF0000ULL)
				      | ((stage1.data >> 48) & 0x000000000000FFFFULL);
			stage2.sop     = stage1.sop;
			stage2.eop     = stage1.eop;
			stage2.empty   = stage1.empty;
			stage2_idx     = stage1_idx;
			stage2_en      = true;
			stage1_en      = false;
			swap_dst_op_en = true;
		}
	}

	// stage 1
	if (! stage1_en & stage0_en) {
		if (swap_dst_op_en) {
			stage1.data = ((stage1.data << 16) & 0xFFFFFFFF00000000ULL)
			            | (stage0.data & 0x00000000FFFFFFFFULL);
			stage1.sop = stage0.sop;
			stage1.eop = stage0.eop;
			stage1.empty = stage0.empty;
		} else {
			stage1 = stage0;
		}
		stage1_idx = stage0_idx;
		stage1_en = true;
		stage0_en = false;
	}

	// input stage
	if (! stage0_en) {
		stage0 = ch_dat_in.tryRead(stage0_en);
		if (stage0_en) {
			stage0_idx = idx_counter;
			idx_counter = stage0.eop ? (UINT8)0 : (UINT8)(idx_counter + 1);
		}
	}

	return;
}

#include "hls_component_param"
component void icmp_proc_swapip(
	L8_AVALON_ST_PKT_RX_BUF8 &ch_dat_in,
	L8_AVALON_ST_PKT_TX      &ch_dat_out
){
	static L8_AVALON_ST_PKT stage2;
	static L8_AVALON_ST_PKT stage1;
	static L8_AVALON_ST_PKT stage0;
	static UINT8            stage2_idx;
	static UINT8            stage1_idx;
	static UINT8            stage0_idx;
	static bool             stage2_en = false;
	static bool             stage1_en = false;
	static bool             stage0_en = false;

	static UINT8            idx_counter = 0;

	// output stage
	if (stage2_en) {
		stage2_en = !( ch_dat_out.tryWrite(stage2) );
	}

	// stage 2
	bool swap_dst_op_en = false;
	if (! stage2_en & stage1_en) {
		if (stage1_idx != 3) {
			// normal bypass operation
			stage2     = stage1;
			stage2_idx = stage1_idx;
			stage2_en  = true;
			stage1_en  = false;
		} else if (stage0_en & (stage0_idx == 4)) {
			// swapping src ip and dest ip
			stage2.data = (stage1.data & 0xFFFF000000000000ULL)
				      | ((stage1.data << 32) & 0x0000FFFF00000000ULL)
				      | ((stage0.data >> 32) & 0x00000000FFFF0000ULL)
				      | ((stage1.data >> 32) & 0x000000000000FFFFULL);
			stage2.sop     = stage1.sop;
			stage2.eop     = stage1.eop;
			stage2.empty   = stage1.empty;
			stage2_idx     = stage1_idx;
			stage2_en      = true;
			stage1_en      = false;
			swap_dst_op_en = true;
		}
	}

	// stage 1
	if (! stage1_en & stage0_en) {
		if (swap_dst_op_en) {
			stage1.data = ((stage1.data << 32) & 0xFFFF000000000000ULL)
			            | (stage0.data & 0x0000FFFFFFFFFFFFULL);
			stage1.sop = stage0.sop;
			stage1.eop = stage0.eop;
			stage1.empty = stage0.empty;
		} else {
			stage1 = stage0;
		}
		stage1_idx = stage0_idx;
		stage1_en = true;
		stage0_en = false;
	}


	// input stage
	if (! stage0_en) {
		stage0 = ch_dat_in.tryRead(stage0_en);
		if (stage0_en) {
			stage0_idx = idx_counter;
			idx_counter = stage0.eop ? (UINT8)0 : (UINT8)(idx_counter + 1);
		}
	}

	return;
}

#include "hls_component_param"
component void icmp_proc_param(
	L8_AVALON_ST_PKT_RX_BUF8 &ch_dat_in,
	L8_AVALON_ST_PKT_TX      &ch_dat_out
){
	static L8_AVALON_ST_PKT stage1;
	static L8_AVALON_ST_PKT stage0;
	static UINT8            stage1_idx;
	static UINT8            stage0_idx;
	static bool             stage1_en = false;
	static bool             stage0_en = false;
	static UINT8            idx_counter = 0;

	// output stage
	if (stage1_en) {
		stage1_en = !( ch_dat_out.tryWrite(stage1) );
	}

	// stage1
	if (! stage1_en & stage0_en) {
		switch (stage0_idx) {
			case 4 : {
				UINT16 chksum = ~((UINT16)(stage0.data >> 16));
				if (chksum < 0x0800) {
					chksum --;
				}
				chksum = ~(chksum - 0x0800);
				stage1.data = (stage0.data & 0xFFFF00FF0000FFFFULL)
				            | (((UINT64)chksum) << 16);
				break;
			}
			default : {
				stage1.data = stage0.data;
			}
		}
		stage1.sop = stage0.sop;
		stage1.eop = stage0.eop;
		stage1.empty = stage0.empty;
		stage1_idx = stage0_idx;
		stage1_en = true;
		stage0_en = false;
	}

	// input stage
	if (! stage0_en) {
		stage0 = ch_dat_in.tryRead(stage0_en);
		if (stage0_en) {
			stage0_idx = idx_counter;
			idx_counter = stage0.eop ? (UINT8)0 : (UINT8)(idx_counter + 1);
		}
	}
	return;
}

#include "hls_component_param"
component void icmp_proc_ipv4chksum(
	L8_AVALON_ST_PKT_RX_BUF8 &ch_dat_in,
	L8_AVALON_ST_PKT_TX      &ch_dat_out
){
	static L8_AVALON_ST_PKT stage3;
	static L8_AVALON_ST_PKT stage2;
	static L8_AVALON_ST_PKT stage1;
	static L8_AVALON_ST_PKT stage0;
	static UINT8            stage3_idx;
	static UINT8            stage2_idx;
	static UINT8            stage1_idx;
	static UINT8            stage0_idx;
	static bool             stage3_en = false;
	static bool             stage2_en = false;
	static bool             stage1_en = false;
	static bool             stage0_en = false;
	static UINT8            idx_counter = 0;
	static UINT24           checksum = (UINT24)0x00000000UL;

	// output stage
	if (stage3_en) {
		stage3_en = !( ch_dat_out.tryWrite(stage3) );
	}

	// stage 3
	if (! stage3_en & stage2_en) {
		if (stage2_idx != 3) {
			// normal bypass operation
			stage3     = stage2;
			stage3_idx = stage2_idx;
			stage3_en  = true;
			stage2_en  = false;
		} else if (stage1_en & (stage1_idx == 4)) {
			// swapping dst mac and src mac
			stage3.data = ((((UINT64)~checksum) << 48) & 0xFFFF000000000000ULL)
				    | (stage2.data & 0x0000FFFFFFFFFFFFULL);
			stage3.sop   = stage2.sop;
			stage3.eop   = stage2.eop;
			stage3.empty = stage2.empty;
			stage3_idx   = stage2_idx;
			stage3_en    = true;
			stage2_en    = false;
		}
	}

	// stage 2
	if (! stage2_en & stage1_en) {
		stage2 = stage1;
		stage2_idx = stage1_idx;
		stage2_en = true;
		stage1_en = false;
	}

	// stage 1
	if (! stage1_en & stage0_en) {
		switch (stage0_idx) {
			case 1 : {
				checksum = (UINT24)(stage0.data & 0x000000000000FFFFULL);
				break;
			}
			case 2 : {
				checksum += (UINT24)((stage0.data >> 48) & 0x000000000000FFFFULL)
				          + (UINT24)((stage0.data >> 32) & 0x000000000000FFFFULL)
				          + (UINT24)((stage0.data >> 16) & 0x000000000000FFFFULL)
				          + (UINT24)(stage0.data & 0x000000000000FFFFULL);
				break;
			}
			case 3 : {
				checksum += (UINT24)((stage0.data >> 32) & 0x000000000000FFFFULL)
				          + (UINT24)((stage0.data >> 16) & 0x000000000000FFFFULL)
				          + (UINT24)(stage0.data & 0x000000000000FFFFULL);
				break;
			}
			case 4 : {
				checksum += (UINT24)((stage0.data >> 48) & 0x000000000000FFFFULL);
				checksum = (UINT24)(checksum & 0x0000FFFF)
				         + (UINT24)((checksum >> 16) & 0x0000FFFF);
				break;
			}
			default : { break; }
		}
		stage1 = stage0;
		stage1_idx = stage0_idx;
		stage1_en = true;
		stage0_en = false;
	}

	// input stage
	if (! stage0_en) {
		stage0 = ch_dat_in.tryRead(stage0_en);
		if (stage0_en) {
			stage0_idx = idx_counter;
			idx_counter = stage0.eop ? (UINT8)0 : (UINT8)(idx_counter + 1);
		}
	}

	return;
}


int main() {
	L8_AVALON_ST_PKT_RX_BUF8 swapmac_in;
	L8_AVALON_ST_PKT_TX      swapmac_out;
	L8_AVALON_ST_PKT_RX_BUF8 swapip_in;
	L8_AVALON_ST_PKT_TX      swapip_out;
	L8_AVALON_ST_PKT_RX_BUF8 param_in;
	L8_AVALON_ST_PKT_TX      param_out;
	L8_AVALON_ST_PKT_RX_BUF8 ipv4chksum_in;
	L8_AVALON_ST_PKT_TX      ipv4chksum_out;

	// input
	for (int i = 0; i < test_icmp_req_len; i ++) {
		L8_AVALON_ST_PKT t = test_icmp_req[i];
		printf("input  : %016llx, %u, %u, %u\n",
		       (unsigned long long)t.data,
		       (unsigned int)t.sop,
		       (unsigned int)t.eop,
		       (unsigned int)t.empty);
	}

	// swapmac
	for (int i = 0; i < test_icmp_req_len; i ++) {
		swapmac_in.write(test_icmp_req[i]);
	}

	for (int i = 0; i < 50; i ++) {
		ihc_hls_enqueue_noret(icmp_proc_swapmac, swapmac_in, swapmac_out);
		//icmp_proc_swapmac(swapmac_in, swapmac_out);
	}
	ihc_hls_component_run_all(icmp_proc_swapmac);

	// swapip
	for (int i = 0; i < 50; i ++) {
		bool en;
		L8_AVALON_ST_PKT t = swapmac_out.tryRead(en);
		if (en) {
			swapip_in.write(t);
		}
	}
	for (int i = 0; i < 50; i ++) {
		ihc_hls_enqueue_noret(icmp_proc_swapip, swapip_in, swapip_out);
		//icmp_proc_swapip(swapip_in, swapip_out);
	}
	ihc_hls_component_run_all(icmp_proc_swapip);

	// param
	for (int i = 0; i < 50; i ++) {
		bool en;
		L8_AVALON_ST_PKT t = swapip_out.tryRead(en);
		if (en) {
			param_in.write(t);
		}
	}
	for (int i = 0; i < 50; i ++) {
		ihc_hls_enqueue_noret(icmp_proc_param, param_in, param_out);
		//icmp_proc_param(param_in, param_out);
	}
	ihc_hls_component_run_all(icmp_proc_param);

	// ipv4chksum
	for (int i = 0; i < 50; i ++) {
		bool en;
		L8_AVALON_ST_PKT t = param_out.tryRead(en);
		if (en) {
			ipv4chksum_in.write(t);
		}
	}
	for (int i = 0; i < 50; i ++) {
		ihc_hls_enqueue_noret(icmp_proc_ipv4chksum, ipv4chksum_in, ipv4chksum_out);
		//icmp_proc_chksum(ipv4chksum_in, ipv4chksum_out);
	}
	ihc_hls_component_run_all(icmp_proc_ipv4chksum);

	// output
	for (int i = 0; i < 50; i ++) {
		bool en;
		L8_AVALON_ST_PKT t = ipv4chksum_out.tryRead(en);
		if (en) {
			printf("output : %016llx, %u, %u, %u\n",
			       (unsigned long long)t.data,
			       (unsigned int)t.sop,
			       (unsigned int)t.eop,
			       (unsigned int)t.empty);
		}
	}

	return 0;
}

