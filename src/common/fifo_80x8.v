//------------------------------------------------------------------------------
// fifo_80x8.v
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


module fifo_80x8 (
	input           clk,
	input           arst_n,
	input   [79:0]  wdata,
	output          afull,
	output          full,
	input           wval,
	output  [79:0]  rdata,
	output          aempty,
	output          empty,
	input           rval,
	output  [3:0]   usedw
	);

	reg     [79:0]  buf_r[0:8];
	reg     [3:0]   usedw_r;
	reg     [3:0]   usedw_1d_r;

	always @ (posedge clk or negedge arst_n) begin
		if (arst_n == 1'b0) begin
			buf_r[0] <= 80'd0;
			buf_r[1] <= 80'd0;
			buf_r[2] <= 80'd0;
			buf_r[3] <= 80'd0;
			buf_r[4] <= 80'd0;
			buf_r[5] <= 80'd0;
			buf_r[6] <= 80'd0;
			buf_r[7] <= 80'd0;
			usedw_r <= 4'd0;
		end else begin
			buf_r[0] <= rval ? ((wval & (usedw_r == 4'd1)) ? wdata : buf_r[1]) : ((wval & (usedw_r == 4'd0)) ? wdata : buf_r[0]);
			buf_r[1] <= rval ? ((wval & (usedw_r == 4'd2)) ? wdata : buf_r[2]) : ((wval & (usedw_r == 4'd1)) ? wdata : buf_r[1]);
			buf_r[2] <= rval ? ((wval & (usedw_r == 4'd3)) ? wdata : buf_r[3]) : ((wval & (usedw_r == 4'd2)) ? wdata : buf_r[2]);
			buf_r[3] <= rval ? ((wval & (usedw_r == 4'd4)) ? wdata : buf_r[4]) : ((wval & (usedw_r == 4'd3)) ? wdata : buf_r[3]);
			buf_r[4] <= rval ? ((wval & (usedw_r == 4'd5)) ? wdata : buf_r[5]) : ((wval & (usedw_r == 4'd4)) ? wdata : buf_r[4]);
			buf_r[5] <= rval ? ((wval & (usedw_r == 4'd6)) ? wdata : buf_r[6]) : ((wval & (usedw_r == 4'd5)) ? wdata : buf_r[5]);
			buf_r[6] <= rval ? ((wval & (usedw_r == 4'd7)) ? wdata : buf_r[7]) : ((wval & (usedw_r == 4'd6)) ? wdata : buf_r[6]);
			buf_r[7] <= rval ? ((wval & (usedw_r == 4'd8)) ? wdata : 80'd0) : ((wval & (usedw_r == 4'd7)) ? wdata : buf_r[7]);
			usedw_r <= usedw_r + {3'd0, wval} - {3'd0, rval};
		end
	end
	always @ (posedge clk or negedge arst_n) begin
		if (arst_n == 1'b0) begin
			usedw_1d_r <= 4'd0;
		end else begin
			usedw_1d_r <= usedw_r;
		end
	end

	assign afull  = (usedw_r >= 4'd7);
	assign full   = (usedw_r == 4'd8);
	assign rdata  = buf_r[0];
	assign aempty = (usedw_r <= 4'd1);
	assign empty  = (usedw_r == 4'd0);
	assign usedw  = usedw_1d_r;
endmodule

