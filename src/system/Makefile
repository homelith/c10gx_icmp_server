#-------------------------------------------------------------------------------
# Makefile
# - CLI automated compilation environment for Quartus Prime project src directory
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

TARGET          := system
SUBDIRS         := ip
.PHONY:         default single modgen sysgen softgen clean $(SUBDIRS)

single:
	qsys-generate --quartus-project=../../syn/top.qpf --synthesis=VERILOG $(TARGET).qsys

modgen:         COMMAND := modgen
modgen:         $(SUBDIRS)

sysgen:         COMMAND := sysgen
sysgen:         $(SUBDIRS)
	make single

softgen:        COMMAND := softgen
softgen:        $(SUBDIRS)

clean:          COMMAND := clean
clean:          $(SUBDIRS)
	rm -rf .qsys_edit $(TARGET)

$(SUBDIRS):
	make -C $@ $(COMMAND)
