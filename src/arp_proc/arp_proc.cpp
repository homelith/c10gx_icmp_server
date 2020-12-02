//------------------------------------------------------------------------------
// arp_proc.cpp
//
// i++ HLS module for ARP processing
//   - generate ARP reply with this device's MAC addr in response of ARP query
//   - receive ARP reply (include GARP) from gateway and update gateway ARP cache
//   - spontaneously generate ARP request to gateway IP when gateway ARP cache expires
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

//------------------------------------------------------------------------------
// CHANGE IP AND MAC ADDRESS BEFORE USE !!
//
// default setting has IP 192.0.2.100 and GTW 192.0.2.1 reserved IPv4 address for documentation
// see IPv4 Address Blocks Reserved for Documentation ( https://tools.ietf.org/html/rfc5737 )
//
// default MAC addr is set to 06:00:00:00:00:00 address with local (U/L=1) administration indicated
// see IEEE guidelines ( https://standards.ieee.org/content/dam/ieee-standards/standards/web/documents/tutorials/eui.pdf )
//
// default MAC is EXAMPLE VALUE to confirm successful build of project, please do not use this in public network space
//
#define OWN_MAC_0   0x06
#define OWN_MAC_1   0x00
#define OWN_MAC_2   0x00
#define OWN_MAC_3   0x00
#define OWN_MAC_4   0x00
#define OWN_MAC_5   0x00
#define OWN_IP_0     192
#define OWN_IP_1       0
#define OWN_IP_2       2
#define OWN_IP_3     100
#define GTW_IP_0     192
#define GTW_IP_1       0
#define GTW_IP_2       2
#define GTW_IP_3       1

//------------------------------------------------------------------------------
// HLS component
//------------------------------------------------------------------------------
#include "HLS/stdio.h"
#include "HLS/hls.h"
#include "HLS/math.h"
#include "HLS/extendedmath.h"
#include "HLS/ac_int.h"
#include "HLS/ac_fixed.h"

// type definition for internal usage
typedef ac_int<64,false> UINT64;
typedef ac_int<48,false> UINT48;
typedef ac_int<32,false> UINT32;
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
typedef ihc::stream_out <L8_AVALON_ST_PKT, ihc::bitsPerSymbol<8> >                 L8_AVALON_ST_PKT_TX;
typedef ihc::stream_in  <UINT8, ihc::buffer<1> >                                   PPS_IN;
typedef ihc::stream_out <UINT48>                                                   GTW_MAC_OUT;

#define ARP_DATA_BASE_0 0x0000000000000000ULL
#define ARP_DATA_BASE_1 0x0000000008060001ULL
#define ARP_DATA_BASE_2 0x0800060400000000ULL
#define ARP_DATA_BASE_3 0x0000000000000000ULL
#define ARP_DATA_BASE_4 0x0000000000000000ULL
#define ARP_DATA_BASE_5 0x0000000000000000ULL

#define GTW_IP      (((UINT32)(GTW_IP_0) << 24) | ((UINT32)(GTW_IP_1) << 16) | ((UINT32)(GTW_IP_2) << 8) | (UINT32)(GTW_IP_3))
#define OWN_IP      (((UINT32)(OWN_IP_0) << 24) | ((UINT32)(OWN_IP_1) << 16) | ((UINT32)(OWN_IP_2) << 8) | (UINT32)(OWN_IP_3))
#define OWN_MAC     (((UINT48)(OWN_MAC_0) << 40)| ((UINT48)(OWN_MAC_1) << 32) | ((UINT48)(OWN_MAC_2) << 24) | ((UINT48)(OWN_MAC_3) << 16) | ((UINT48)(OWN_MAC_4) << 8) | (UINT48)(OWN_MAC_5))
#define BCAST_MAC   0x0000ffffffffffffULL

#define ARP_DATA_0 (ARP_DATA_BASE_0 | (((UINT64)OWN_MAC_0)<<8) | ((UINT64)OWN_MAC_1))
#define ARP_DATA_1 (ARP_DATA_BASE_1 | (((UINT64)OWN_MAC_2)<<56) | (((UINT64)OWN_MAC_3)<<48) | (((UINT64)OWN_MAC_4)<<40) | (((UINT64)OWN_MAC_5)<<32))
#define ARP_DATA_2 (ARP_DATA_BASE_2 | (((UINT64)OWN_MAC_0)<<8) | ((UINT64)OWN_MAC_1))
#define ARP_DATA_3 (ARP_DATA_BASE_3 | (((UINT64)OWN_MAC_2)<<56) | (((UINT64)OWN_MAC_3)<<48) | (((UINT64)OWN_MAC_4)<<40) | (((UINT64)OWN_MAC_5)<<32) | (((UINT64)OWN_IP_0)<<24) | (((UINT64)OWN_IP_1)<<16) | (((UINT64)OWN_IP_2)<<8) | ((UINT64)OWN_IP_3))
#define ARP_DATA_4 ARP_DATA_BASE_4
#define ARP_DATA_5 ARP_DATA_BASE_5

#define OUT_SEND_MODE_IDLE        0
#define OUT_SEND_MODE_ARP_REQUEST 1
#define OUT_SEND_MODE_ARP_REPLY   2

#define ARP_CACHE_LIFETIME 16

// sender   : send ARP request for each pps_in trigger
// receiver : ARP cache invalidation is left unimplemented, 
//            ARP cache is continuously updated on ARP reply or GARP advertisement of gateway
#include "hls_component_param"
component void arp_proc(
	L8_AVALON_ST_PKT_RX_BUF8 &ch_dat_in,
	L8_AVALON_ST_PKT_TX      &ch_dat_out,
	PPS_IN                   &ch_pps_in
){
	const L8_AVALON_ST_PKT arp_data[6] = {
		{ARP_DATA_0, 1, 0, 0},
		{ARP_DATA_1, 0, 0, 0},
		{ARP_DATA_2, 0, 0, 0},
		{ARP_DATA_3, 0, 0, 0},
		{ARP_DATA_4, 0, 0, 0},
		{ARP_DATA_5, 0, 1, 6}
	};

	static L8_AVALON_ST_PKT out_stage;
	static bool             out_stage_en           = false;
	static UINT8            out_send_idx           = 0;

	static bool             out_send_arp_rpy_stdby = false;
	static bool             out_send_arp_req_stdby = false;
	static UINT8            out_send_mode          = OUT_SEND_MODE_IDLE;

	static L8_AVALON_ST_PKT in_stage5 = {0x0000000000000000ULL, 0, 0, 0};
	static L8_AVALON_ST_PKT in_stage4 = {0x0000000000000000ULL, 0, 0, 0};
	static L8_AVALON_ST_PKT in_stage3 = {0x0000000000000000ULL, 0, 0, 0};
	static L8_AVALON_ST_PKT in_stage2 = {0x0000000000000000ULL, 0, 0, 0};
	static L8_AVALON_ST_PKT in_stage1 = {0x0000000000000000ULL, 0, 0, 0};
	static L8_AVALON_ST_PKT in_stage0 = {0x0000000000000000ULL, 0, 0, 0};
	static bool             in_stage0_en;

	static UINT48           gtw_mac = (UINT48)0x0000000000000000ULL;
	static UINT48           rpy_mac;
	static UINT32           rpy_ip;

	static UINT16           arp_cache_invalidate_count = 0;
	static bool             pps_en = false;

	// output stage
	if (out_stage_en) {
		out_stage_en = !( ch_dat_out.tryWrite(out_stage) );
	}

	// if ARP request send sequence enabled, fetch data from memory
	if (! out_stage_en) {
		if (out_send_mode == OUT_SEND_MODE_ARP_REQUEST) {
			switch (out_send_idx) {
				case 0 : {
					out_stage = arp_data[0];
					out_stage.data |= (((UINT64)BCAST_MAC) << 16);
					break;
				}
				case 1 : {
					out_stage = arp_data[1];
					break;
				}
				case 2 : {
					out_stage = arp_data[2];
					out_stage.data |= 0x0000000000010000ULL;
					break;
				}
				case 3 : {
					out_stage = arp_data[3];
					break;
				}
				case 4 : {
					out_stage = arp_data[4];
					out_stage.data |= (((UINT64)BCAST_MAC) << 16) |
					                  ((((UINT64)GTW_IP) >> 16) & 0x000000000000ffffULL);
					break;
				}
				case 5 : {
					out_stage = arp_data[5];
					out_stage.data |= (((UINT64)GTW_IP) << 48) & 0xffff000000000000ULL;
					out_send_mode = OUT_SEND_MODE_IDLE;
					break;
				}
			}
			out_send_idx ++;
			out_stage_en = true;
		} else if (out_send_mode == OUT_SEND_MODE_ARP_REPLY) {
			switch (out_send_idx) {
				case 0 : {
					out_stage = arp_data[0];
					out_stage.data |= (((UINT64)rpy_mac) << 16);
					break;
				}
				case 1 : {
					out_stage = arp_data[1];
					break;
				}
				case 2 : {
					out_stage = arp_data[2];
					out_stage.data |= 0x0000000000020000ULL;
					break;
				}
				case 3 : {
					out_stage = arp_data[3];
					break;
				}
				case 4 : {
					out_stage = arp_data[4];
					out_stage.data |= (((UINT64)rpy_mac) << 16) |
					                  ((((UINT64)GTW_IP) >> 16) & 0x000000000000ffffULL);
					break;
				}
				case 5 : {
					out_stage = arp_data[5];
					out_stage.data |= (((UINT64)rpy_ip) << 48) & 0xffff000000000000ULL;
					out_send_mode = OUT_SEND_MODE_IDLE;
					break;
				}
			}
			out_send_idx ++;
			out_stage_en = true;
		}
	}

	// PPS input drives ARP send sequence
	ch_pps_in.tryRead(pps_en);
	if (pps_en) {
		if (arp_cache_invalidate_count == 0) {
			if (! out_send_arp_req_stdby) {
				out_send_arp_req_stdby = true;
			}
		} else {
			arp_cache_invalidate_count --;
		}
	}

	// input ARP processing
	if (in_stage0_en) {
		// rx dispatch flow : this flow drives once per each stage0 data input
		// is req ? -[yes]-> is my ip req ? -[yes]-> update cache & trigger ARP reply
		//          |                       |
		//          |                       +[ no]-> update cache only
		//          |
		//          +[ no]-> is from gtw ?  -[yes]-> update cache
		if (in_stage5.sop) {
			UINT8  recv_arp_op  = (in_stage3.data >> 16) & 0x00000000000000FFULL;
			UINT48 recv_src_mac = ((in_stage3.data << 32) & 0x0000FFFF00000000ULL) | ((in_stage2.data >> 32) & 0x00000000FFFFFFFFULL);
			UINT32 recv_src_ip  = in_stage2.data & 0x00000000FFFFFFFFULL;
			UINT32 recv_dst_ip  = ((in_stage1.data << 16) & 0x00000000FFFF0000ULL) | ((in_stage0.data >> 48) & 0x000000000000FFFFULL);
			bool is_req_own_ip  = (recv_dst_ip == OWN_IP);
			bool is_req_gtw_ip  = (recv_dst_ip == GTW_IP);
			bool is_from_gtw    = (recv_src_ip == GTW_IP);
			
			if (recv_arp_op == 0x01) {
				if (is_req_own_ip && out_send_arp_rpy_stdby == false) {
					rpy_mac = recv_src_mac;
					rpy_ip = recv_src_ip;
					out_send_arp_rpy_stdby = true;
				} else if (is_req_gtw_ip && is_from_gtw) {
					gtw_mac = recv_src_mac;
					arp_cache_invalidate_count = ARP_CACHE_LIFETIME;
				}
			} else if (is_from_gtw) {
				gtw_mac = recv_src_mac;
				arp_cache_invalidate_count = ARP_CACHE_LIFETIME;
			}
		}
		// pushing delay buffer
		in_stage5 = in_stage4;
		in_stage4 = in_stage3;
		in_stage3 = in_stage2;
		in_stage2 = in_stage1;
		in_stage1 = in_stage0;
		in_stage0_en = false;
	}

	// out_send_mode control
	if (out_send_mode == OUT_SEND_MODE_IDLE) {
		if (out_send_arp_rpy_stdby) {
			out_send_mode = OUT_SEND_MODE_ARP_REPLY;
			out_send_arp_rpy_stdby = false;
			out_send_idx = 0;
		} else if (out_send_arp_req_stdby) {
			out_send_mode = OUT_SEND_MODE_ARP_REQUEST;
			out_send_arp_req_stdby = false;
			out_send_idx = 0;
		}
	}
	
	// read input
	if (! in_stage0_en) {
		in_stage0 = ch_dat_in.tryRead(in_stage0_en);
	}

	return;
}

//------------------------------------------------------------------------------
// testbench
//------------------------------------------------------------------------------
// ARP Packet Format
// 0-5   : dst mac
// 6-11  : src mac
// 12-13 : eth type = 0x0806(ARP)
// 14-19 : hwtype=0x0001, proto=0x0800, hwaddrlen=0x06, protoaddrlen=0x04
// 20-21 : operation (01 for req, 02 for ack)
// 22-27 : src mac
// 28-31 : src ip
// 32-37 : dst mac
// 38-41 : dst ip

// test packet 1 : ARP request to this device
UINT32 test_arp_request_len = 8;
L8_AVALON_ST_PKT test_arp_request[8] = {
	{0xffffffffffff1122ULL, 1, 0, 0},
	{0x3344556608060001ULL, 0, 0, 0},
	{0x0800060400011122ULL, 0, 0, 0},
	{0x33445566c0a88901ULL, 0, 0, 0},
	{0xffffffffffffc0a8ULL, 0, 0, 0},
	{0x89c8000000000000ULL, 0, 0, 0},
	{0x0000000000000000ULL, 0, 0, 0},
	{0x0000000000000000ULL, 0, 1, 4}
};
// test packet 2 : gratuitous ARP advertisement from gateway
UINT32 test_gtw_garp_len = 6;
L8_AVALON_ST_PKT test_gtw_garp[6] = {
	{0xffffffffffff1122ULL, 1, 0, 0},
	{0x3344556608060001ULL, 0, 0, 0},
	{0x0800060400011122ULL, 0, 0, 0},
	{0x33445566c0a88901ULL, 0, 0, 0},
	{0xffffffffffffc0a8ULL, 0, 0, 0},
	{0x8901000000000000ULL, 0, 1, 6}
};
// test packet 3 : ARP reply from gateway to this device
UINT32 test_arp_reply_len = 8;
L8_AVALON_ST_PKT test_arp_reply[8] = {
	{0x9ca3baffbeef1122ULL, 1, 0, 0},
	{0x3344556608060001ULL, 0, 0, 0},
	{0x0800060400021122ULL, 0, 0, 0},
	{0x33445566c0a88901ULL, 0, 0, 0},
	{0x9ca3baffbeefc0a8ULL, 0, 0, 0},
	{0x89c8000000000000ULL, 0, 0, 0},
	{0x0000000000000000ULL, 0, 0, 0},
	{0x0000000000000000ULL, 0, 1, 4}
};
// test packet 4 : packet no relation with ARP to be discarded
UINT32 test_unrelated_fragment_len = 6;
L8_AVALON_ST_PKT test_unrelated_fragment[6] = {
	{0xffffffffffff1122ULL, 1, 0, 0},
	{0x3344556608060001ULL, 0, 0, 0},
	{0x0800060400011122ULL, 0, 0, 0},
	{0x33445566c0a88901ULL, 0, 0, 0},
	{0xffffffffffffc0a8ULL, 0, 0, 0},
	{0x8964000000000000ULL, 0, 1, 6}
};

// simlulation scenario
int main() {
	L8_AVALON_ST_PKT_RX_BUF8 in;
	L8_AVALON_ST_PKT_TX      out;
	PPS_IN                   pps;
	UINT48 g;

	// unrelated packet should be ignored
	printf("unrelated packet should be ignored\n");
	for (int i = 0; i < test_unrelated_fragment_len; i ++) {
		in.write(test_unrelated_fragment[i]);
	}
	for (int i = 0; i < 50; i ++) {
		ihc_hls_enqueue_noret(arp_proc, in, out, pps);
	}
	ihc_hls_component_run_all(arp_proc);
	for (int i = 0; i < 50; i ++) {
		bool en;
		L8_AVALON_ST_PKT t = out.tryRead(en);
		if (en) {
			printf("output : %016llx, %u, %u, %u\n",
			       (unsigned long long)t.data,
			       (unsigned int)t.sop,
			       (unsigned int)t.eop,
			       (unsigned int)t.empty);
		}
	}

	// ARP request to OWN_IP (burst input)
	printf("ARP request to OWN_IP (burst input)\n");
	for (int i = 0; i < test_arp_request_len; i ++) {
		in.write(test_arp_request[i]);
		L8_AVALON_ST_PKT t = test_arp_request[i];
		printf("input : %016llx, %u, %u, %u\n",
		       (unsigned long long)t.data,
		       (unsigned int)t.sop,
		       (unsigned int)t.eop,
		       (unsigned int)t.empty);
	}
	for (int i = 0; i < 50; i ++) {
		ihc_hls_enqueue_noret(arp_proc, in, out, pps);
	}
	ihc_hls_component_run_all(arp_proc);
	for (int i = 0; i < 50; i ++) {
		bool en;
		L8_AVALON_ST_PKT t = out.tryRead(en);
		if (en) {
			printf("output : %016llx, %u, %u, %u\n",
			       (unsigned long long)t.data,
			       (unsigned int)t.sop,
			       (unsigned int)t.eop,
			       (unsigned int)t.empty);
		}
	}

	// ARP request to OWN_IP (intermittently)
	printf("ARP request to OWN_IP (intermittently)\n");
	for (int i = 0; i < test_arp_request_len; i ++) {
		in.write(test_arp_request[i]);
		L8_AVALON_ST_PKT t = test_arp_request[i];
		printf("input : %016llx, %u, %u, %u\n",
		       (unsigned long long)t.data,
		       (unsigned int)t.sop,
		       (unsigned int)t.eop,
		       (unsigned int)t.empty);
	}
	for (int i = 0; i < 20; i ++) {
		arp_proc(in, out, pps);
	}
	for (int i = 0; i < 50; i ++) {
		bool en;
		L8_AVALON_ST_PKT t = out.tryRead(en);
		if (en) {
			printf("output : %016llx, %u, %u, %u\n",
			       (unsigned long long)t.data,
			       (unsigned int)t.sop,
			       (unsigned int)t.eop,
			       (unsigned int)t.empty);
		}
	}

	// spontaneously ARP request to GTW_IP driven by pps input
	printf("spontaneously ARP request to GTW_IP driven by pps input\n");
	for (int i = 0; i < test_arp_reply_len; i ++) {
		in.write(test_arp_reply[i]);
	}
	for (int i = 0; i < 40; i ++) {
		printf("%d th pps\n", i);
		pps.write(0x00);

		for (int j = 0; j < 50; j ++) {
			ihc_hls_enqueue_noret(arp_proc, in, out, pps);
		}

		for (int j = 0; j < 50; j ++) {
			bool en;
			L8_AVALON_ST_PKT t = out.tryRead(en);
			if (en) {
				printf("output : %016llx, %u, %u, %u\n",
				       (unsigned long long)t.data,
				       (unsigned int)t.sop,
				       (unsigned int)t.eop,
				       (unsigned int)t.empty);
			}
		}
	}

	// Spontaneous ARP reply suppressed by ARP reply
	printf("Spontaneous ARP reply suppressed by ARP reply\n");
	for (int i = 0; i < 40; i ++) {
		printf("%d th pps\n", i);
		pps.write(0x00);

		for (int j = 0; j < test_arp_reply_len; j ++) {
			in.write(test_arp_reply[j]);
		}
		for (int j = 0; j < 50; j ++) {
			ihc_hls_enqueue_noret(arp_proc, in, out, pps);
		}
		for (int j = 0; j < 50; j ++) {
			bool en;
			L8_AVALON_ST_PKT t = out.tryRead(en);
			if (en) {
				printf("output : %016llx, %u, %u, %u\n",
				       (unsigned long long)t.data,
				       (unsigned int)t.sop,
				       (unsigned int)t.eop,
				       (unsigned int)t.empty);
			}
		}
	}

	// Spontaneous ARP reply also suppressed by GARP advertisement
	printf("Spontaneous ARP reply also suppressed by GARP advertisement\n");
	for (int i = 0; i < 40; i ++) {
		printf("%d th pps\n", i);
		pps.write(0x00);

		for (int j = 0; j < test_gtw_garp_len; j ++) {
			in.write(test_gtw_garp[j]);
		}
		for (int j = 0; j < 50; j ++) {
			ihc_hls_enqueue_noret(arp_proc, in, out, pps);
		}
		for (int j = 0; j < 50; j ++) {
			bool en;
			L8_AVALON_ST_PKT t = out.tryRead(en);
			if (en) {
				printf("output : %016llx, %u, %u, %u\n",
				       (unsigned long long)t.data,
				       (unsigned int)t.sop,
				       (unsigned int)t.eop,
				       (unsigned int)t.empty);
			}
		}
	}

	return 0;
}

