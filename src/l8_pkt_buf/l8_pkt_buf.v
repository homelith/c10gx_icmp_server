//------------------------------------------------------------------------------
// l8_pkt_buf.v
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

module l8_pkt_buf(
	input clk,
	input xrst,

	input  [63:0]   from_l8_data,
	input           from_l8_startofpacket,
	input           from_l8_endofpacket,
	input  [2:0]    from_l8_empty,
	output          from_l8_ready,
	input           from_l8_valid,

	output [63:0]   to_l8_data,
	output          to_l8_startofpacket,
	output          to_l8_endofpacket,
	output [2:0]    to_l8_empty,
	input           to_l8_ready,
	output          to_l8_valid
);
	//----------------------------------------------------------------------
	// reg & wire
	//----------------------------------------------------------------------
	reg             from_l8_ready_r;
	wire   [11:0]   queue_usedw;

	// regs for num of packet stored in buffer
	wire            pkt_in;
	reg             pkt_in_1d_r;
	reg             pkt_in_2d_r;
	wire            pkt_out;
	wire   [11:0]   next_queue_pkt_count;
	reg    [11:0]   queue_pkt_count_r;

	// FIFO output port regs
	wire            queue_aempty;
	wire            queue_empty;
	wire   [63:0]   queue_l8_data;
	wire            queue_l8_startofpacket;
	wire            queue_l8_endofpacket;
	wire   [2:0]    queue_l8_empty;
	reg             queue_data_exist_r;
	wire            queue_pop_w;
	wire            next_queue_data_exist;

	// processing stage regs
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

	// output stage regs
	reg             to_l8_valid_r;

	//----------------------------------------------------------------------
	// FIFO input port ctrl
	//----------------------------------------------------------------------
	assign from_l8_ready = from_l8_ready_r;
	always @ (posedge clk or negedge xrst) begin
		if (xrst == 1'b0) begin
			from_l8_ready_r <= 1'b0;
		end else if (queue_usedw >= 12'd2045 || next_queue_pkt_count >= 12'd2045) begin
			from_l8_ready_r <= 1'b0;
		end else begin
			from_l8_ready_r <= 1'b1;
		end
	end

	function [11:0] next_queue_pkt_count_f;
		input [11:0] queue_pkt_count_arg;
		input pkt_in_arg;
		input pkt_out_arg;
		if (pkt_in_arg == 1'b1 && pkt_out_arg == 1'b0) begin
			next_queue_pkt_count_f = queue_pkt_count_arg + 12'd1;
		end else if (pkt_in_arg == 1'b0 && pkt_out_arg == 1'b1) begin
			next_queue_pkt_count_f = queue_pkt_count_arg - 12'd1;
		end else begin
			next_queue_pkt_count_f = queue_pkt_count_arg;
		end
	endfunction

	assign pkt_in  = from_l8_endofpacket & from_l8_valid & from_l8_ready_r;
	assign pkt_out = queue_l8_endofpacket & queue_pop_w;
	assign next_queue_pkt_count = next_queue_pkt_count_f(queue_pkt_count_r, pkt_in_2d_r, pkt_out);
	always @ (posedge clk or negedge xrst) begin
		if (xrst == 1'b0) begin
			pkt_in_1d_r <= 1'b0;
			pkt_in_2d_r <= 1'b0;
		end else begin
			pkt_in_1d_r <= pkt_in;
			pkt_in_2d_r <= pkt_in_1d_r;
		end
	end
	always @ (posedge clk or negedge xrst) begin
		if (xrst == 1'b0) begin
			queue_pkt_count_r <= 12'd0;
		end else begin
			queue_pkt_count_r <= next_queue_pkt_count;
		end
	end

	//----------------------------------------------------------------------
	// FIFO output port ctrl
	//----------------------------------------------------------------------
	assign next_queue_data_exist = (queue_empty == 1'b0) && (queue_aempty == 1'b0 || queue_pop_w == 1'b0) && (next_queue_pkt_count != 12'd0);
	always @ (posedge clk or negedge xrst) begin
		if (xrst == 1'b0) begin
			queue_data_exist_r <= 1'b0;
		end else begin
			queue_data_exist_r <= next_queue_data_exist;
		end
	end
	assign queue_pop_w = next_stage0_update;

	//----------------------------------------------------------------------
	// processing stage
	//----------------------------------------------------------------------
	assign next_stage0_acceptable = (to_l8_valid_r && to_l8_ready)
		|| (~stage0_enable_r)
		;
	assign next_stage0_enable           = next_stage0_acceptable ? next_stage0_update : stage0_enable_r;
	assign next_stage0_update           = queue_data_exist_r & next_stage0_acceptable;
	assign next_stage0_l8_data          = queue_l8_data;
	assign next_stage0_l8_startofpacket = queue_l8_startofpacket;
	assign next_stage0_l8_endofpacket   = queue_l8_endofpacket;
	assign next_stage0_l8_empty         = queue_l8_empty;

	always @ (posedge clk or negedge xrst) begin
		if (xrst == 1'b0) begin
			stage0_enable_r <= 1'b0;
		end else begin
			stage0_enable_r <= next_stage0_enable;
		end
	end
	always @ (posedge clk or negedge xrst) begin
		if (xrst == 1'b0) begin
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
	// output stage ctrl
	//----------------------------------------------------------------------
	always @ (posedge clk or negedge xrst) begin
		if (xrst == 1'b0) begin
			to_l8_valid_r <= 1'b0;
		end else begin
			to_l8_valid_r <= next_stage0_enable;
		end
	end

	// for synthesis
	assign to_l8_data          = stage0_l8_data_r;
	assign to_l8_startofpacket = stage0_l8_startofpacket_r;
	assign to_l8_endofpacket   = stage0_l8_endofpacket_r;
	assign to_l8_empty         = stage0_l8_empty_r;
	assign to_l8_valid         = to_l8_valid_r;

	//----------------------------------------------------------------------
	// FIFO entity
	//----------------------------------------------------------------------
	wire    [10:0]   queue_11bit_dummy;
	fifo_80x2048 data_fifo(
		.clk         (clk),
		.xrst        (xrst),
		.wdata       ({11'd0, from_l8_empty, from_l8_endofpacket, from_l8_startofpacket, from_l8_data}),
		.afull       (),
		.full        (),
		.wval        (from_l8_valid & from_l8_ready_r),
		.rdata       ({queue_11bit_dummy, queue_l8_empty, queue_l8_endofpacket, queue_l8_startofpacket, queue_l8_data}),
		.aempty      (queue_aempty),
		.empty       (queue_empty),
		.rval        (queue_pop_w),
		.usedw       (queue_usedw)
	);
endmodule
