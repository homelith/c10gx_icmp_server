//------------------------------------------------------------------------------
// tx_merge.cpp
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
typedef ihc::stream_out <L8_AVALON_ST_PKT, ihc::bitsPerSymbol<8> > L8_AVALON_ST_PKT_TX;

int arp_test_pkt_len = 35;
L8_AVALON_ST_PKT arp_test_pkt[35] = {
	{0xffffffffffff1122ULL, 1, 0, 0},
	{0x3344556608001111ULL, 0, 0, 0},
	{0xbbbbccccddddeeeeULL, 0, 0, 0},
	{0xffffaaaabbbbccccULL, 0, 0, 0},
	{0xddddeeeeffffaaaaULL, 0, 0, 0},
	{0xbbbbccccddddeeeeULL, 0, 0, 0},
	{0xffffaaaabbbbccccULL, 0, 0, 0},
	{0xddddeeeeffffaaaaULL, 0, 0, 0},
	{0xddddee0000000000ULL, 0, 1, 5},

	{0xaabbccddeeff1122ULL, 1, 0, 0},
	{0x3344556608002222ULL, 0, 0, 0},
	{0xbbbbccccddddeeeeULL, 0, 0, 0},
	{0xffffaaaabbbbccccULL, 0, 0, 0},
	{0xddddeeeeffffaaaaULL, 0, 0, 0},
	{0xbbbbccccddddeeeeULL, 0, 0, 0},
	{0xffffaaaabbbbccccULL, 0, 0, 0},
	{0xddddeeeeffffaaaaULL, 0, 0, 0},
	{0xdddd000000000000ULL, 0, 1, 6},

	{0xabcdefabcdef1122ULL, 1, 0, 0},
	{0x3344556608003333ULL, 0, 0, 0},
	{0xbbbbccccddddeeeeULL, 0, 0, 0},
	{0xffffaaaabbbbccccULL, 0, 0, 0},
	{0xddddeeeeffffaaaaULL, 0, 0, 0},
	{0xbbbbccccddddeeeeULL, 0, 0, 0},
	{0xffffaaaabbbbccccULL, 0, 0, 0},
	{0xddddeeeeffffaaaaULL, 0, 0, 0},
	{0xdd00000000000000ULL, 0, 1, 7},

	{0xabcdefabcdef1122ULL, 1, 0, 0},
	{0x3344556608004444ULL, 0, 0, 0},
	{0xbbbbccccddddeeeeULL, 0, 0, 0},
	{0xffffaaaabbbbccccULL, 0, 0, 0},
	{0xddddeeeeffffaaaaULL, 0, 0, 0},
	{0xbbbbccccddddeeeeULL, 0, 0, 0},
	{0xffffaaaabbbbccccULL, 0, 0, 0},
	{0xddddeeeeffffaaaaULL, 0, 1, 0}
};

int ipv4_test_pkt_len = 32;
L8_AVALON_ST_PKT ipv4_test_pkt[32] = {
	{0xaabbccddeeff1122ULL, 1, 0, 0},
	{0x3344556608005555ULL, 0, 0, 0},
	{0xbbbbccccddddeeeeULL, 0, 0, 0},
	{0xffffaaaabbbbccccULL, 0, 0, 0},
	{0xddddeeeeffffaaaaULL, 0, 0, 0},
	{0xbbbbccccddddeeeeULL, 0, 0, 0},
	{0xffffaaaabbbbccccULL, 0, 0, 0},
	{0xddddeeeeffffaa00ULL, 0, 1, 1},

	{0xffffffffffff1122ULL, 1, 0, 0},
	{0x3344556608006666ULL, 0, 0, 0},
	{0xbbbbccccddddeeeeULL, 0, 0, 0},
	{0xffffaaaabbbbccccULL, 0, 0, 0},
	{0xddddeeeeffffaaaaULL, 0, 0, 0},
	{0xbbbbccccddddeeeeULL, 0, 0, 0},
	{0xffffaaaabbbbccccULL, 0, 0, 0},
	{0xddddeeeeffff0000ULL, 0, 1, 2},

	{0xaabbccddeeff1122ULL, 1, 0, 0},
	{0x3344556608007777ULL, 0, 0, 0},
	{0xbbbbccccddddeeeeULL, 0, 0, 0},
	{0xffffaaaabbbbccccULL, 0, 0, 0},
	{0xddddeeeeffffaaaaULL, 0, 0, 0},
	{0xbbbbccccddddeeeeULL, 0, 0, 0},
	{0xffffaaaabbbbccccULL, 0, 0, 0},
	{0xddddeeeeff000000ULL, 0, 1, 3},

	{0xabcdefabcdef1122ULL, 1, 0, 0},
	{0x3344556608008888ULL, 0, 0, 0},
	{0xbbbbccccddddeeeeULL, 0, 0, 0},
	{0xffffaaaabbbbccccULL, 0, 0, 0},
	{0xddddeeeeffffaaaaULL, 0, 0, 0},
	{0xbbbbccccddddeeeeULL, 0, 0, 0},
	{0xffffaaaabbbbccccULL, 0, 0, 0},
	{0xddddeeee00000000ULL, 0, 1, 4}
};

#define PORT_SELECT_IDLE 0
#define PORT_SELECT_ARP  1
#define PORT_SELECT_IPV4 2

#include "hls_component_param"
component void tx_merge(
	L8_AVALON_ST_PKT_RX_BUF8 &ch_dat_in_arp,
	L8_AVALON_ST_PKT_RX_BUF8 &ch_dat_in_ipv4,
	L8_AVALON_ST_PKT_TX      &ch_dat_out
){
	static L8_AVALON_ST_PKT stage1;
	static L8_AVALON_ST_PKT stage0_arp;
	static L8_AVALON_ST_PKT stage0_ipv4;
	static bool             stage1_en     = false;
	static bool             stage0_arp_en = false;
	static bool             stage0_ipv4_en = false;

	static UINT8            port_select   = PORT_SELECT_IDLE;

	// output stage
	if (stage1_en) {
		stage1_en = !( ch_dat_out.tryWrite(stage1) );
	}

	// stage 1 : lane merging
	if (! stage1_en) {
		if (stage0_arp_en && (port_select != PORT_SELECT_IPV4)) {
			stage1 = stage0_arp;
			stage1_en = true;
			stage0_arp_en = false;
			port_select = stage0_arp.eop ? PORT_SELECT_IDLE : PORT_SELECT_ARP;
		} else if (stage0_ipv4_en && (port_select != PORT_SELECT_ARP)) {
			stage1 = stage0_ipv4;
			stage1_en = true;
			stage0_ipv4_en = false;
			port_select = stage0_ipv4.eop ? PORT_SELECT_IDLE : PORT_SELECT_IPV4;
		}
	}

	// stage 0 : read stage
	if (! stage0_arp_en) {
		stage0_arp = ch_dat_in_arp.tryRead(stage0_arp_en);
	}
	if (! stage0_ipv4_en) {
		stage0_ipv4 = ch_dat_in_ipv4.tryRead(stage0_ipv4_en);
	}

	return;
}

int main() {
	L8_AVALON_ST_PKT_RX_BUF8 arp;
	L8_AVALON_ST_PKT_RX_BUF8 ipv4;
	L8_AVALON_ST_PKT_TX      out;

	for (unsigned int i = 0; i < arp_test_pkt_len; i ++) {
		arp.write(arp_test_pkt[i]);
	}
	for (unsigned int i = 0; i < ipv4_test_pkt_len; i ++) {
		ipv4.write(ipv4_test_pkt[i]);
	}

	for (unsigned int i = 0; i < (arp_test_pkt_len + ipv4_test_pkt_len) * 2; i ++) {
		ihc_hls_enqueue_noret(tx_merge, arp, ipv4, out);
	}

	ihc_hls_component_run_all(tx_merge);

	for (unsigned int i = 0; i < (arp_test_pkt_len + ipv4_test_pkt_len) * 2; i ++) {
		bool en;
		L8_AVALON_ST_PKT t = out.tryRead(en);
		if (en) {
			printf("output : %llx, %u, %u, %u\n", (unsigned long long)t.data, (unsigned int)t.sop, (unsigned int)t.eop, (unsigned int)t.empty);
		}
	}

	return 0;
}

