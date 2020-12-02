#-------------------------------------------------------------------------------
# Makefile
# - CLI automated compilation environment fo Intel i++ HLS Compiler
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# MIT License
#
# Copyright (c) 2020 homelith
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#-------------------------------------------------------------------------------

.PHONY: default single modgen sysgen softgen csim vsim

TARGET          := icmp_proc
TARGET_FREQ     := 160MHz
SOURCE_FILES    := icmp_proc.cpp
FPGA_PART       := 10CX220YF780E5G
CXX             := i++
CLEAN_FILES     := $(TARGET) $(TARGET).prj csim vsim vsim.prj hls_component_param transcript

# Generate final target code by default
default:        single

single:         modgen

modgen:         $(TARGET)

sysgen:

softgen:

# Run a C testbench with C components
csim: $(SOURCE_FILES)
	echo "" > hls_component_param
	$(CXX) $(SOURCE_FILES) -march=x86-64 -o csim
	./csim

# Run a C testbench with verilog component
vsim: $(SOURCE_FILES)
	echo "" > hls_component_param
	$(CXX) $(SOURCE_FILES) -v -ghdl -march=$(FPGA_PART) --clock $(TARGET_FREQ) -o vsim
	./vsim
	vsim vsim.prj/verification/vsim.wlf

# Generate final target code with "always enabled" state
$(TARGET): $(SOURCE_FILES)
	echo "hls_stall_free_return hls_always_run_component" > hls_component_param
	$(CXX) $(SOURCE_FILES) -v --simulator none -march=$(FPGA_PART) --clock $(TARGET_FREQ) -o $(TARGET)

clean:
	rm -rf $(CLEAN_FILES)
