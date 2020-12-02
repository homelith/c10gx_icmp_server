//------------------------------------------------------------------------------
// rx_split.cpp
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

int test_pkt_len = 21;
L8_AVALON_ST_PKT test_pkt[21] = {
	// orphan fragment to be discarded
	{0xffffffffffff1122ULL, 1, 1, 0},

	// ARP Packet
	{0xffffffffffff1122ULL, 1, 0, 0},
	{0x3344556608060001ULL, 0, 0, 0},
	{0x0800060400011122ULL, 0, 0, 0},
	{0x33445566c0a88901ULL, 0, 0, 0},
	{0xffffffffffffc0a8ULL, 0, 0, 0},
	{0x89c8000000000000ULL, 0, 0, 0},
	{0x0000000000000000ULL, 0, 0, 0},
	{0x0000000000000000ULL, 0, 1, 4},

	// orphan fragment to be discarded
	{0xffffffffffff1122ULL, 1, 1, 0},
	// orphan fragment to be discarded
	{0xffffffffffff1122ULL, 1, 1, 0},

	// ICMP Packet
	{0x00005e00016654bfULL, 1, 0, 0},
	{0x648042d808004500ULL, 0, 0, 0},
	{0x003c02de00008001ULL, 0, 0, 0},
	{0x0000ac1f18873dd3ULL, 0, 0, 0},
	{0xe204080030f50001ULL, 0, 0, 0},
	{0x1c66616263646566ULL, 0, 0, 0},
	{0x6768696a6b6c6d6eULL, 0, 0, 0},
	{0x6f70717273747576ULL, 0, 0, 0},
	{0x7761626364656667ULL, 0, 0, 0},
	{0x6869000000000000ULL, 0, 1, 6}
};

#define S_ETYPE_HOLD      0
#define S_ETYPE_ARP       1
#define S_ETYPE_IPV4      2
#define S_ETYPE_OTHER     3

#define S_IPV4PROTO_HOLD  0
#define S_IPV4PROTO_ICMP  1
#define S_IPV4PROTO_OTHER 2

#define ETYPE_MASK       0x00000000ffff0000ULL
#define ETYPE_IPV4       0x0000000008000000ULL
#define ETYPE_ARP        0x0000000008060000ULL

#define IPV4PROTO_MASK   0x00000000000000ffULL
#define IPV4PROTO_ICMP   0x0000000000000001ULL

#include "hls_component_param"
// terminator block to discard all packet
component void rx_split_discard(
	L8_AVALON_ST_PKT_RX_BUF8 &ch_dat_in
){
	bool en;
	ch_dat_in.tryRead(en);
	return;
}

// fork arp packet, ipv4 packet and the other
#include "hls_component_param"
component void rx_split_by_etype(
	L8_AVALON_ST_PKT_RX_BUF8 &ch_dat_in,
	L8_AVALON_ST_PKT_TX      &ch_dat_out_arp,
	L8_AVALON_ST_PKT_TX      &ch_dat_out_ipv4,
	L8_AVALON_ST_PKT_TX      &ch_dat_out_other
){
	static L8_AVALON_ST_PKT stage3_arp;
	static L8_AVALON_ST_PKT stage3_ipv4;
	static L8_AVALON_ST_PKT stage3_other;
	static L8_AVALON_ST_PKT stage2;
	static L8_AVALON_ST_PKT stage1;
	static L8_AVALON_ST_PKT stage0;
	static bool             stage3_arp_en   = false;
	static bool             stage3_ipv4_en  = false;
	static bool             stage3_other_en = false;
	static bool             stage2_en       = false;
	static bool             stage1_en       = false;
	static bool             stage0_en       = false;
	static UINT8            output_state    = S_ETYPE_HOLD;

	// output stage
	if (stage3_arp_en) {
		stage3_arp_en = !( ch_dat_out_arp.tryWrite(stage3_arp) );
	}
	if (stage3_ipv4_en) {
		stage3_ipv4_en = !( ch_dat_out_ipv4.tryWrite(stage3_ipv4) );
	}
	if (stage3_other_en) {
		stage3_other_en = !( ch_dat_out_other.tryWrite(stage3_other) );
	}

	// stage 3 : routing
	// routing eop on stage3 drives output_state into sop hold mode.
	// but allows immediate override in stage2.
	bool sop_next_expected = false;
	switch (output_state) {
		case S_ETYPE_ARP : {
			// routing to arp processing port
			if ((! stage3_arp_en) & stage2_en) {
				sop_next_expected = stage2.eop ? true : false;
				stage3_arp     = stage2;
				stage3_arp_en  = true;
				stage2_en      = false;
			}
			break;
		}
		case S_ETYPE_IPV4 : {
			// routing to ipv4 port
			if ((! stage3_ipv4_en) & stage2_en) {
				sop_next_expected = stage2.eop ? true : false;
				stage3_ipv4     = stage2;
				stage3_ipv4_en  = true;
				stage2_en       = false;
			}
			break;
		}
		case S_ETYPE_OTHER : {
			// routing to other port
			if ((! stage3_other_en) & stage2_en) {
				sop_next_expected = stage2.eop ? true : false;
				stage3_other      = stage2;
				stage3_other_en   = true;
				stage2_en         = false;
			}
			break;
		}
		default : {
			// hold until routing determined
			sop_next_expected = true;
			break;
		}
	}

	// stage 2 : queueing stage for sop
	if (! stage2_en & stage1_en) {
		stage2    = stage1;
		stage2_en = true;
		stage1_en = false;
	}

	// stage 1 : queueing stage for next data of sop
	if (! stage1_en & stage0_en) {
		stage1    = stage0;
		stage1_en = true;
		stage0_en = false;
	}

	// stage 0 : read stage
	if (! stage0_en) {
		stage0 = ch_dat_in.tryRead(stage0_en);
	}

	// update state
	if (sop_next_expected) {
		if (stage2_en) {
			if (stage2.eop) {
				// exeptionally output_state is drived to S_ETYPE_OTHER
				// when stage2 chunk has both sop and eop (under 8bytes small fragment)
				output_state = S_ETYPE_OTHER;
			} else if (stage1_en) {
				// read ethertype field to select routing destination.
				// successful dicision requires sop chunk exists in stage2
				//  and next chunk in stage1 simultaneously
				switch ((unsigned long long)(stage1.data & ETYPE_MASK)) {
					case ETYPE_ARP  : { output_state = S_ETYPE_ARP;   break;}
					case ETYPE_IPV4 : { output_state = S_ETYPE_IPV4;  break;}
					default         : { output_state = S_ETYPE_OTHER; break;}
				}
			} else {
				// when stage1 is not filled with data on the end of previous packet,
				// stage3 stall on next clock to wait stage1/2 ready
				output_state = S_ETYPE_HOLD;
			}
		} else {
			// when stage2 is not filled with data on the end of previous packet,
			// stage3 stall on next clock to wait stage1/2 ready
			output_state = S_ETYPE_HOLD;
		}
	}

	return;
}

// fork icmp packet and the other from ipv4 packets
#include "hls_component_param"
component void rx_split_by_ipv4proto(
	L8_AVALON_ST_PKT_RX_BUF8 &ch_dat_in,
	L8_AVALON_ST_PKT_TX      &ch_dat_out_icmp,
	L8_AVALON_ST_PKT_TX      &ch_dat_out_other
){
	static L8_AVALON_ST_PKT stage4_icmp;
	static L8_AVALON_ST_PKT stage4_other;
	static L8_AVALON_ST_PKT stage3;
	static L8_AVALON_ST_PKT stage2;
	static L8_AVALON_ST_PKT stage1;
	static L8_AVALON_ST_PKT stage0;
	static bool             stage4_icmp_en   = false;
	static bool             stage4_other_en = false;
	static bool             stage3_en       = false;
	static bool             stage2_en       = false;
	static bool             stage1_en       = false;
	static bool             stage0_en       = false;
	static UINT8            output_state    = S_IPV4PROTO_HOLD;

	// output stage
	if (stage4_icmp_en) {
		stage4_icmp_en = !( ch_dat_out_icmp.tryWrite(stage4_icmp) );
	}
	if (stage4_other_en) {
		stage4_other_en = !( ch_dat_out_other.tryWrite(stage4_other) );
	}

	// stage 4 : routing
	// routing eop on stage4 drives output_state into sop hold mode.
	// but allows immediate override in stage3.
	bool sop_next_expected = false;
	switch (output_state) {
		case S_IPV4PROTO_ICMP : {
			// routing to icmp processing port
			if ((! stage4_icmp_en) & stage3_en) {
				sop_next_expected = stage3.eop ? true : false;
				stage4_icmp    = stage3;
				stage4_icmp_en = true;
				stage3_en      = false;
			}
			break;
		}
		case S_IPV4PROTO_OTHER : {
			// routing to other port
			if ((! stage4_other_en) & stage3_en) {
				sop_next_expected = stage3.eop ? true : false;
				stage4_other      = stage3;
				stage4_other_en   = true;
				stage3_en         = false;
			}
			break;
		}
		default : {
			// hold until routing determined
			sop_next_expected = true;
			break;
		}
	}

	// stage 3 : queueing stage for sop
	if (! stage3_en & stage2_en) {
		stage3    = stage2;
		stage3_en = true;
		stage2_en = false;
	}

	// stage 2
	if (! stage2_en & stage1_en) {
		stage2    = stage1;
		stage2_en = true;
		stage1_en = false;
	}

	// stage 1
	if (! stage1_en & stage0_en) {
		stage1    = stage0;
		stage1_en = true;
		stage0_en = false;
	}

	// stage 0 : read stage
	if (! stage0_en) {
		stage0 = ch_dat_in.tryRead(stage0_en);
	}

	// update state
	if (sop_next_expected) {
		if (stage3_en) {
			if (stage3.eop) {
				// exeptionally output_state is drived to S_IPV4PROTO_OTHER
				// when stage3 chunk has both sop and eop (under 8bytes small fragment)
				output_state = S_IPV4PROTO_OTHER;
			} else if (stage2_en) {
				if (stage2.eop) {
					// exeptionally output_state is drived to S_IPV4PROTO_OTHER
					// when stage2 chunk has both sop and eop (under 16bytes small fragment)
					output_state = S_IPV4PROTO_OTHER;
				} else if (stage1_en) {
					// read ipv4 protocol field to select routing destination.
					// successful dicision requires sop chunk exists in stage3
					//  and next chunks in stage2 and 1 respectively
					switch ((unsigned long long)(stage1.data & IPV4PROTO_MASK)) {
						case IPV4PROTO_ICMP : { output_state = S_IPV4PROTO_ICMP;  break;}
						default             : { output_state = S_IPV4PROTO_OTHER; break;}
					}
				} else {
					// when stage1 is not filled with data on the end of previous packet,
					// stage4 stall on next clock to wait stage1/2/3 ready
					output_state = S_IPV4PROTO_HOLD;
				}
			} else {
				// when stage2 is not filled with data on the end of previous packet,
				// stage4 stall on next clock to wait stage1/2/3 ready
				output_state = S_IPV4PROTO_HOLD;
			}
		} else {
			// when stage3 is not filled with data on the end of previous packet,
			// stage4 stall on next clock to wait stage1/2/3 ready
			output_state = S_IPV4PROTO_HOLD;
		}
	}

	return;
}

int main() {
	L8_AVALON_ST_PKT_RX_BUF8 etype_in;
	L8_AVALON_ST_PKT_TX      etype_arp_out;
	L8_AVALON_ST_PKT_TX      etype_ipv4_out;
	L8_AVALON_ST_PKT_TX      etype_other_out;
	L8_AVALON_ST_PKT_RX_BUF8 ipv4_in;
	L8_AVALON_ST_PKT_TX      ipv4_icmp_out;
	L8_AVALON_ST_PKT_TX      ipv4_other_out;

	// prepare input data for etype filter
	for (unsigned int i = 0; i < test_pkt_len; i ++) {
		etype_in.write(test_pkt[i]);
	}

	printf("-- first stage : etype filter --\n");

	// run etype filter
	for (unsigned int i = 0; i < test_pkt_len * 2; i ++) {
		ihc_hls_enqueue_noret(rx_split_by_etype, etype_in, etype_arp_out, etype_ipv4_out, etype_other_out);
	}
	ihc_hls_component_run_all(rx_split_by_etype);

	// extract etype filter output and transfer to ipv4 filter input
	for (unsigned int i = 0; i < test_pkt_len * 2; i ++) {
		bool en;
		L8_AVALON_ST_PKT t = etype_arp_out.tryRead(en);
		if (en) {
			printf("etype_arp_out   : %016llx, %u, %u, %u\n", (unsigned long long)t.data, (unsigned int)t.sop, (unsigned int)t.eop, (unsigned int)t.empty);
		}
	}
	for (unsigned int i = 0; i < test_pkt_len * 2; i ++) {
		bool en;
		L8_AVALON_ST_PKT t = etype_ipv4_out.tryRead(en);
		if (en) {
			printf("etype_ipv4_out  : %016llx, %u, %u, %u\n", (unsigned long long)t.data, (unsigned int)t.sop, (unsigned int)t.eop, (unsigned int)t.empty);
			ipv4_in.write(t);
		}
	}
	for (unsigned int i = 0; i < test_pkt_len * 2; i ++) {
		bool en;
		L8_AVALON_ST_PKT t = etype_other_out.tryRead(en);
		if (en) {
			printf("etype_other_out : %016llx, %u, %u, %u\n", (unsigned long long)t.data, (unsigned int)t.sop, (unsigned int)t.eop, (unsigned int)t.empty);
		}
	}

	printf("-- second stage : ipv4 protocol filter --\n");

	// run ipv4 protocol filter
	for (unsigned int i = 0; i < test_pkt_len * 2; i ++) {
		ihc_hls_enqueue_noret(rx_split_by_ipv4proto, ipv4_in, ipv4_icmp_out, ipv4_other_out);
	}
	ihc_hls_component_run_all(rx_split_by_ipv4proto);

	// extract ipv4 filter output and transfer to icmp filter input
	for (unsigned int i = 0; i < test_pkt_len * 2; i ++) {
		bool en;
		L8_AVALON_ST_PKT t = ipv4_icmp_out.tryRead(en);
		if (en) {
			printf("ipv4_icmp_out   : %016llx, %u, %u, %u\n", (unsigned long long)t.data, (unsigned int)t.sop, (unsigned int)t.eop, (unsigned int)t.empty);
		}
	}
	for (unsigned int i = 0; i < test_pkt_len * 2; i ++) {
		bool en;
		L8_AVALON_ST_PKT t = ipv4_other_out.tryRead(en);
		if (en) {
			printf("ipv4_other_out  : %016llx, %u, %u, %u\n", (unsigned long long)t.data, (unsigned int)t.sop, (unsigned int)t.eop, (unsigned int)t.empty);
		}
	}

	return 0;
}

