//------------------------------------------------------------------------------
// top.v
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

`timescale 1 ps / 1 ps

module top (
		// common signals
		input  wire        fpga_resetn,
		input  wire        c10_clk50m,
		//input  wire        c10_clkusr,

		// PCIe gen2 x4 Hard IP
		//input  wire        hip_serial_rx_in0,
		//input  wire        hip_serial_rx_in1,
		//input  wire        hip_serial_rx_in2,
		//input  wire        hip_serial_rx_in3,
		//output wire        hip_serial_tx_out0,
		//output wire        hip_serial_tx_out1,
		//output wire        hip_serial_tx_out2,
		//output wire        hip_serial_tx_out3,
		//input  wire        perstn,
		//input  wire        refclk_clk,

		// GPIO
		input  wire [3:0]  user_dip,
		input  wire [2:0]  user_pb,
		output wire [3:0]  user_led,

		// SFP plus port x2
		input  wire        sfp_refclk,                  // Reference clock for SFP+ from U64
		output wire        sfp_tx_0,                    // SFP+ XCVR TX
		input  wire        sfp_rx_0,                    // SFP+ XCVR RX
		//output wire        sfp_tx_1,                    // SFP+ XCVR TX
		//input  wire        sfp_rx_1,                    // SFP+ XCVR RX
		inout  wire        sfp_scl_0,
		inout  wire        sfp_sda_0
		//input  wire        sfp_int_0,
		//inout  wire        sfp_scl_1,
		//inout  wire        sfp_sda_1,
		//input  wire        sfp_int_1,

		// DDR4 SDRAM
		//output wire [0:0]  emif_0_mem_ck,
		//output wire [0:0]  emif_0_mem_ck_n,
		//output wire [14:0] emif_0_mem_a,
		////output wire [0:0]  emif_0_mem_act_n,
		//output wire [2:0]  emif_0_mem_ba,
		////output wire [0:0]  emif_0_mem_bg,
		//output wire [0:0]  emif_0_mem_cke,
		//output wire [0:0]  emif_0_mem_cs_n,
		//output wire [0:0]  emif_0_mem_odt,
		//output wire [0:0]  emif_0_mem_reset_n,
		//output wire [0:0]  emif_0_mem_we_n,
		//output wire [0:0]  emif_0_mem_ras_n,
		//output wire [0:0]  emif_0_mem_cas_n,
		//inout  wire [4:0]  emif_0_mem_dqs,
		//inout  wire [4:0]  emif_0_mem_dqs_n,
		//inout  wire [39:0] emif_0_mem_dq,
		////inout  wire [7:0]  emif_0_mem_dbi_n,
		//output wire [4:0]  emif_0_mem_dm,
		//input  wire        emif_0_oct_rzqin,
		//input  wire        emif_0_pll_ref_clk
	);
	
	wire pcie_dclk_snoop;
	wire xgmii_clk_snoop;
	wire emif_reset;
	wire xgmii_clk_pps;
	

	wire [63:0] phy_xgmii_rx_data;
	wire [7:0]  phy_xgmii_rx_ctrl;
	wire [63:0] phy_xgmii_tx_data;
	wire [7:0]  phy_xgmii_tx_ctrl;

	wire [71:0] mac_xgmii_rx;
	wire [71:0] mac_xgmii_tx;

	assign mac_xgmii_rx = {phy_xgmii_rx_ctrl[7], phy_xgmii_rx_data[63:56], phy_xgmii_rx_ctrl[6], phy_xgmii_rx_data[55:48],
	                       phy_xgmii_rx_ctrl[5], phy_xgmii_rx_data[47:40], phy_xgmii_rx_ctrl[4], phy_xgmii_rx_data[39:32],
	                       phy_xgmii_rx_ctrl[3], phy_xgmii_rx_data[31:24], phy_xgmii_rx_ctrl[2], phy_xgmii_rx_data[23:16],
	                       phy_xgmii_rx_ctrl[1], phy_xgmii_rx_data[15: 8], phy_xgmii_rx_ctrl[0], phy_xgmii_rx_data[ 7: 0]};
	assign phy_xgmii_tx_data = {mac_xgmii_tx[70:63], mac_xgmii_tx[61:54], mac_xgmii_tx[52:45], mac_xgmii_tx[43:36],
	                            mac_xgmii_tx[34:27], mac_xgmii_tx[25:18], mac_xgmii_tx[16: 9], mac_xgmii_tx[ 7: 0]};
	assign phy_xgmii_tx_ctrl = {mac_xgmii_tx[71], mac_xgmii_tx[62], mac_xgmii_tx[53], mac_xgmii_tx[44],
	                            mac_xgmii_tx[35], mac_xgmii_tx[26], mac_xgmii_tx[17], mac_xgmii_tx[ 8]};

	// i2c output enable
	// workaround for suppressing error on a10 open drain buffer
	wire const_zero_sig /* synthesis keep */;
	assign const_zero_sig = 1'b0;
	wire sfp_scl_oe_0;
	wire sfp_sda_oe_0;
	assign sfp_scl_0 = sfp_scl_oe_0 ? const_zero_sig : 1'bz;
	assign sfp_sda_0 = sfp_sda_oe_0 ? const_zero_sig : 1'bz;

	system system_inst (
		//.emif_mem_mem_ck                    (emif_0_mem_ck),                 //  output,   width = 1,              emif_mem.mem_ck
		//.emif_mem_mem_ck_n                  (emif_0_mem_ck_n),               //  output,   width = 1,                      .mem_ck_n
		//.emif_mem_mem_a                     (emif_0_mem_a),                  //  output,  width = 15,                      .mem_a
		//.emif_mem_mem_ba                    (emif_0_mem_ba),                 //  output,   width = 3,                      .mem_ba
		//.emif_mem_mem_cke                   (emif_0_mem_cke),                //  output,   width = 1,                      .mem_cke
		//.emif_mem_mem_cs_n                  (emif_0_mem_cs_n),               //  output,   width = 1,                      .mem_cs_n
		//.emif_mem_mem_odt                   (emif_0_mem_odt),                //  output,   width = 1,                      .mem_odt
		//.emif_mem_mem_reset_n               (emif_0_mem_reset_n),            //  output,   width = 1,                      .mem_reset_n
		//.emif_mem_mem_we_n                  (emif_0_mem_we_n),               //  output,   width = 1,                      .mem_we_n
		//.emif_mem_mem_ras_n                 (emif_0_mem_ras_n),              //  output,   width = 1,                      .mem_ras_n
		//.emif_mem_mem_cas_n                 (emif_0_mem_cas_n),              //  output,   width = 1,                      .mem_cas_n
		//.emif_mem_mem_dqs                   (emif_0_mem_dqs),                //   inout,   width = 5,                      .mem_dqs
		//.emif_mem_mem_dqs_n                 (emif_0_mem_dqs_n),              //   inout,   width = 5,                      .mem_dqs_n
		//.emif_mem_mem_dq                    (emif_0_mem_dq),                 //   inout,  width = 40,                      .mem_dq
		//.emif_mem_mem_dm                    (emif_0_mem_dm),                 //  output,   width = 5,                      .mem_dm
		//.emif_oct_oct_rzqin                 (emif_0_oct_rzqin),              //   input,   width = 1,              emif_oct.oct_rzqin
		//.emif_refclk_clk                    (emif_0_pll_ref_clk),            //   input,   width = 1,           emif_refclk.clk
		//.emif_status_local_cal_success      (),                              //  output,   width = 1,           emif_status.local_cal_success
		//.emif_status_local_cal_fail         (),                              //  output,   width = 1,                      .local_cal_fail

		.global_clk_clk                     (c10_clk50m),                    //   input,   width = 1,            global_clk.clk
		.global_rst_reset_n                 (fpga_resetn),                   //   input,   width = 1,            global_rst.reset_n

		//.pcie_dclk_snoop_clk                (pcie_dclk_snoop),               //  output,   width = 1,       pcie_dclk_snoop.clk
		//.pcie_refclk_clk                    (refclk_clk),                    //   input,   width = 1,           pcie_refclk.clk
		//.pcie_rst_npor                      (perstn),                        //   input,   width = 1,              pcie_rst.npor
		//.pcie_rst_pin_perst                 (perstn),                        //   input,   width = 1,                      .pin_perst
		//.hip_ctrl_test_in                   (),                              //   input,  width = 32,              hip_ctrl.test_in
		//.hip_ctrl_simu_mode_pipe            (),                              //   input,   width = 1,                      .simu_mode_pipe
		//.hip_serial_rx_in0                  (hip_serial_rx_in0),             //   input,   width = 1,            hip_serial.rx_in0
		//.hip_serial_rx_in1                  (hip_serial_rx_in1),             //   input,   width = 1,                      .rx_in1
		//.hip_serial_rx_in2                  (hip_serial_rx_in2),             //   input,   width = 1,                      .rx_in2
		//.hip_serial_rx_in3                  (hip_serial_rx_in3),             //   input,   width = 1,                      .rx_in3
		//.hip_serial_tx_out0                 (hip_serial_tx_out0),            //  output,   width = 1,                      .tx_out0
		//.hip_serial_tx_out1                 (hip_serial_tx_out1),            //  output,   width = 1,                      .tx_out1
		//.hip_serial_tx_out2                 (hip_serial_tx_out2),            //  output,   width = 1,                      .tx_out2
		//.hip_serial_tx_out3                 (hip_serial_tx_out3),            //  output,   width = 1,                      .tx_out3

		.enet_nios_i2c_scl_in               (sfp_scl_0),                     //   input,   width = 1,                      .scl_in
		.enet_nios_i2c_sda_in               (sfp_sda_0),                     //   input,   width = 1,         enet_nios_i2c.sda_in
		.enet_nios_i2c_scl_oe               (sfp_scl_oe_0),                  //  output,   width = 1,                      .scl_oe
		.enet_nios_i2c_sda_oe               (sfp_sda_oe_0),                  //  output,   width = 1,                      .sda_oe

		.pps_in_data                        (8'b0),                          //   input,   width = 8,                pps_in.data
		.pps_in_ready                       (),                              //  output,   width = 1,                      .ready
		.pps_in_valid                       (xgmii_clk_pps),                 //   input,   width = 1,                      .valid

		.enet_p0_rx_serial_rx_serial_data       (sfp_rx_0),                  //   input,   width = 1,     enet_p0_rx_serial.rx_serial_data
		.enet_p0_tx_serial_tx_serial_data       (sfp_tx_0),                  //  output,   width = 1,     enet_p0_tx_serial.tx_serial_data
		.enet_p0_xgmii_phy_rxd_rx_parallel_data (phy_xgmii_rx_data),         //  output,  width = 64,     enet_p0_xgmii_rxd.rx_parallel_data
		.enet_p0_xgmii_phy_rxc_rx_control       (phy_xgmii_rx_ctrl),         //  output,   width = 8,     enet_p0_xgmii_rxc.rx_control
		.enet_p0_xgmii_phy_txd_tx_parallel_data (phy_xgmii_tx_data),         //   input,  width = 64,     enet_p0_xgmii_txd.tx_parallel_data
		.enet_p0_xgmii_phy_txc_tx_control       (phy_xgmii_tx_ctrl),         //   input,   width = 8,     enet_p0_xgmii_txc.tx_control
		.enet_p0_xgmii_mac_rx_data              (mac_xgmii_rx),              //   input,  width = 72,  enet_p0_xgmii_mac_rx.data
		.enet_p0_xgmii_mac_tx_data              (mac_xgmii_tx),              //  output,  width = 72,  enet_p0_xgmii_mac_tx.data

		.enet_xgmii_clk_snoop_clk           (xgmii_clk_snoop),               //  output,   width = 1,  enet_xgmii_clk_snoop.clk
		.enet_refclk_clk                    (sfp_refclk),                    //   input,   width = 1,           enet_refclk.clk

		.enet_p0_tx_ready_tx_ready          (),                              //  output,   width = 1,      enet_p0_tx_ready.tx_ready
		.enet_p0_pll_select_pll_select      (1'b0),                          //   input,   width = 1,    enet_p0_pll_select.pll_select
		.enet_p0_rx_ready_rx_ready          ()                               //  output,   width = 1,      enet_p0_rx_ready.rx_ready
	);

	reg [24:0] c10_clk50m_cnt_r;
	reg        c10_clk50m_hb_r;
	reg        c10_clk50m_hb_1d_r;
	always @ (posedge c10_clk50m) begin
		if (c10_clk50m_cnt_r < 25'd25000000) begin
			c10_clk50m_cnt_r   <= c10_clk50m_cnt_r + 25'd1;
			c10_clk50m_hb_r    <= c10_clk50m_hb_r;
		end else begin
			c10_clk50m_cnt_r   <= 25'd0;
			c10_clk50m_hb_r    <= ~c10_clk50m_hb_r;
		end
	end
	assign user_led[0] = c10_clk50m_hb_r;

	//reg [25:0] pcie_dclk_cnt_r;
	//reg        pcie_dclk_hb_r;
	//always @ (posedge pcie_dclk_snoop or negedge perstn) begin
	//	if (! perstn) begin
	//		pcie_dclk_cnt_r <= 26'd0;
	//		pcie_dclk_hb_r <= 1'b0;
	//	end else if (pcie_dclk_cnt_r < 26'd62500000) begin
	//		pcie_dclk_cnt_r <= pcie_dclk_cnt_r + 26'd1;
	//		pcie_dclk_hb_r <= pcie_dclk_hb_r;
	//	end else begin
	//		pcie_dclk_cnt_r <= 26'd0;
	//		pcie_dclk_hb_r <= ~pcie_dclk_hb_r;
	//	end
	//end
	//assign user_led[1] = pcie_dclk_hb_r;

	reg [26:0] xgmii_clk_cnt_r;
	reg        xgmii_clk_hb_r;
	reg        xgmii_clk_hb_1d_r;
	always @ (posedge xgmii_clk_snoop) begin
		if (xgmii_clk_cnt_r < 27'd78125000) begin
			xgmii_clk_cnt_r   <= xgmii_clk_cnt_r + 27'd1;
			xgmii_clk_hb_r    <= xgmii_clk_hb_r;
			xgmii_clk_hb_1d_r <= xgmii_clk_hb_r;
		end else begin
			xgmii_clk_cnt_r   <= 27'd0;
			xgmii_clk_hb_r    <= ~xgmii_clk_hb_r;
			xgmii_clk_hb_1d_r <= xgmii_clk_hb_r;
		end
	end
	assign xgmii_clk_pps = xgmii_clk_hb_r & (~xgmii_clk_hb_1d_r);
	assign user_led[2] = xgmii_clk_hb_r;
endmodule
