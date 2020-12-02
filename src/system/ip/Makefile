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

TARGET          := system_emif \
                   system_enet_10gmac_p0 \
                   system_enet_10gphy_p0 \
                   system_enet_arp_proc_p0 \
                   system_enet_atxpll \
                   system_enet_icmp_proc_ipv4chksum_p0 \
                   system_enet_icmp_proc_param_p0 \
                   system_enet_icmp_proc_swapip_p0 \
                   system_enet_icmp_proc_swapmac_p0 \
                   system_enet_iopll \
                   system_enet_l8_pkt_buf_p0 \
                   system_enet_l8_to_ulong2_p0 \
                   system_enet_nios_i2c \
                   system_enet_nios_jtaguart \
                   system_enet_nios_mem \
                   system_enet_nios_proc \
                   system_enet_refclk \
                   system_enet_reset_p0 \
                   system_enet_rx_split_by_etype_p0 \
                   system_enet_rx_split_by_ipv4proto_p0 \
                   system_enet_rx_split_etype_dsc_p0 \
                   system_enet_rx_split_ipv4proto_dsc_p0 \
                   system_enet_tx_merge_p0 \
                   system_enet_ulong2_to_l8_p0 \
                   system_enet_xgmii_clk_snoop \
                   system_global_clk \
                   system_global_rst \
                   system_onchip_memory \
                   system_pcie_dclk_snoop \
                   system_pcie_hip

SUBDIRS         := 
.PHONY:         default single modgen sysgen softgen clean $(SUBDIRS)

single:

modgen:         COMMAND := modgen
modgen:         $(SUBDIRS)

sysgen:         COMMAND := sysgen
sysgen:         $(SUBDIRS)

softgen:        COMMAND := softgen
softgen:        $(SUBDIRS)

clean:          COMMAND := clean
clean:          $(SUBDIRS)
	rm -rf .qsys_edit $(TARGET) system

$(SUBDIRS):
	make -C $@ $(COMMAND)
