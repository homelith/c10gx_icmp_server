//------------------------------------------------------------------------------
// l8_to_ulong2.v
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

module l8_to_ulong2(
	input           clk,
	input           arst_n,

	// big endian & firstSymbolInLowOrderBits
	input   [63:0]  in_data,
	input           in_startofpacket,
	input           in_endofpacket,
	input   [2:0]   in_empty,
	input           in_valid,
	output          in_ready,

	// big endian & firstSymbolInHighOrderBits
	// BE <-> LE conversion will executed in any HLS module
	output  [127:0] out_data,
	output          out_valid,
	input           out_ready
);
	//----------------------------------------------------------------------
	// reg & wire
	//----------------------------------------------------------------------
	// FIFO input port
	reg             in_ready_r;
	wire   [3:0]    queue_usedw;

	// FIFO output port
	wire            queue_aempty;
	wire            queue_empty;
	wire   [63:0]   queue_l8_data;
	wire            queue_l8_startofpacket;
	wire            queue_l8_endofpacket;
	wire   [2:0]    queue_l8_empty;
	reg             queue_data_exist_r;
	wire            queue_pop_w;
	wire            next_queue_data_exist;

	// exec stage 0
	wire            next_stage0_acceptable;
	wire            next_stage0_enable;
	wire            next_stage0_update;
	wire   [63:0]   next_stage0_l8_data;
	wire            next_stage0_l8_startofpacket;
	wire            next_stage0_l8_endofpacket;
	wire   [2:0]    next_stage0_l8_empty;
	reg             stage0_enable_r;
	reg    [63:0]   stage0_l8_data_r;
	reg             stage0_l8_startofpacket_r;
	reg             stage0_l8_endofpacket_r;
	reg    [2:0]    stage0_l8_empty_r;

	// output stage
	reg             out_valid_r;


	// data shifter for convert firstSymbolInLowOrderBits attribute
	function [63:0] data_remainder_f (
		input        endofpacket_arg,
		input  [2:0] empty_arg,
		input [63:0] data_arg
	);
		if (endofpacket_arg) begin
			case (empty_arg)
				3'd0 : data_remainder_f = data_arg;
				3'd1 : data_remainder_f = {8'd0, data_arg[63:8]};
				3'd2 : data_remainder_f = {16'd0, data_arg[63:16]};
				3'd3 : data_remainder_f = {24'd0, data_arg[63:24]};
				3'd4 : data_remainder_f = {32'd0, data_arg[63:32]};
				3'd5 : data_remainder_f = {40'd0, data_arg[63:40]};
				3'd6 : data_remainder_f = {48'd0, data_arg[63:48]};
				3'd7 : data_remainder_f = {56'd0, data_arg[63:56]};
			endcase
		end else begin
			data_remainder_f = data_arg;
		end
	endfunction

	//----------------------------------------------------------------------
	// FIFO input port
	//----------------------------------------------------------------------
	assign in_ready = in_ready_r;
	always @ (posedge clk or negedge arst_n) begin
		if (arst_n == 1'b0) begin
			in_ready_r <= 1'b0;
		end else if (queue_usedw >= 4'd5) begin
			in_ready_r <= 1'b0;
		end else begin
			in_ready_r <= 1'b1;
		end
	end

	//----------------------------------------------------------------------
	// FIFO output port
	//----------------------------------------------------------------------
	assign next_queue_data_exist = queue_empty == 1'b0 && (queue_aempty == 1'b0 || queue_pop_w == 1'b0);
	always @ (posedge clk or negedge arst_n) begin
		if (arst_n == 1'b0) begin
			queue_data_exist_r <= 1'b0;
		end else begin
			queue_data_exist_r <= next_queue_data_exist;
		end
	end
	assign queue_pop_w = next_stage0_update;

	//----------------------------------------------------------------------
	// stage 0
	//----------------------------------------------------------------------
	assign next_stage0_acceptable = (out_valid_r & out_ready)
		|| (~stage0_enable_r)
		;
	assign next_stage0_enable           = next_stage0_acceptable ? next_stage0_update : stage0_enable_r;
	assign next_stage0_update           = queue_data_exist_r & next_stage0_acceptable;
	assign next_stage0_l8_data          = data_remainder_f(queue_l8_endofpacket, queue_l8_empty, queue_l8_data);
	assign next_stage0_l8_startofpacket = queue_l8_startofpacket;
	assign next_stage0_l8_endofpacket   = queue_l8_endofpacket;
	assign next_stage0_l8_empty         = queue_l8_empty;

	always @ (posedge clk or negedge arst_n) begin
		if (arst_n == 1'b0) begin
			stage0_enable_r <= 1'b0;
		end else begin
			stage0_enable_r <= next_stage0_enable;
		end
	end
	always @ (posedge clk or negedge arst_n) begin
		if (arst_n == 1'b0) begin
			stage0_l8_data_r          <= 64'd0;
			stage0_l8_startofpacket_r <= 1'b0;
			stage0_l8_endofpacket_r   <= 1'b0;
			stage0_l8_empty_r         <= 3'd0;
		end else if (next_stage0_update == 1'b1) begin
			stage0_l8_data_r          <= next_stage0_l8_data;
			stage0_l8_startofpacket_r <= next_stage0_l8_startofpacket;
			stage0_l8_endofpacket_r   <= next_stage0_l8_endofpacket;
			stage0_l8_empty_r         <= next_stage0_l8_empty;
		end else begin
			stage0_l8_data_r          <= stage0_l8_data_r;
			stage0_l8_startofpacket_r <= stage0_l8_startofpacket_r;
			stage0_l8_endofpacket_r   <= stage0_l8_endofpacket_r;
			stage0_l8_empty_r         <= stage0_l8_empty_r;
		end
	end

	//----------------------------------------------------------------------
	// output stage
	//----------------------------------------------------------------------
	always @ (posedge clk or negedge arst_n) begin
		if (arst_n == 1'b0) begin
			out_valid_r <= 1'b0;
		end else begin
			out_valid_r <= next_stage0_enable;
		end
	end

	assign out_data          = {40'b0, 5'd0, stage0_l8_empty_r, 7'd0, stage0_l8_endofpacket_r, 7'd0, stage0_l8_startofpacket_r, stage0_l8_data_r};
	assign out_valid         = out_valid_r;

	//----------------------------------------------------------------------
	// FIFO instance
	//----------------------------------------------------------------------
	wire    [10:0]   queue_11bit_dummy;
	fifo_80x8 data_fifo(
		.clk         (clk),
		.arst_n      (arst_n),
		.wdata       ({11'd0, in_empty, in_endofpacket, in_startofpacket, in_data}),
		.afull       (),
		.full        (),
		.wval        (in_valid & in_ready_r),
		.rdata       ({queue_11bit_dummy, queue_l8_empty, queue_l8_endofpacket, queue_l8_startofpacket, queue_l8_data}),
		.aempty      (queue_aempty),
		.empty       (queue_empty),
		.rval        (queue_pop_w),
		.usedw       (queue_usedw)
	);
endmodule
