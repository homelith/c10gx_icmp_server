# TCL File Generated by Component Editor 17.1
# Wed Jan 09 11:26:59 JST 2019
# DO NOT MODIFY


# 
# l8_pkt_buf "l8_pkt_buf" v1.0
#  2019.01.09.11:26:59
# 
# 

# 
# request TCL package from ACDS 16.1
# 
package require -exact qsys 16.1


# 
# module l8_pkt_buf
# 
set_module_property DESCRIPTION ""
set_module_property NAME l8_pkt_buf
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property AUTHOR ""
set_module_property DISPLAY_NAME l8_pkt_buf
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE true
set_module_property REPORT_TO_TALKBACK false
set_module_property ALLOW_GREYBOX_GENERATION false
set_module_property REPORT_HIERARCHY false


# 
# file sets
# 
add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
set_fileset_property QUARTUS_SYNTH TOP_LEVEL l8_pkt_buf
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property QUARTUS_SYNTH ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file fifo_80x2048.v VERILOG PATH fifo_80x2048.v
add_fileset_file l8_pkt_buf.v VERILOG PATH l8_pkt_buf.v TOP_LEVEL_FILE
add_fileset_file altera_fifo_80x2048.v VERILOG PATH altera_fifo_80x2048/altera_fifo_80x2048.v


# 
# parameters
# 


# 
# display items
# 


# 
# connection point pkt_in
# 
add_interface pkt_in avalon_streaming end
set_interface_property pkt_in associatedClock clk
set_interface_property pkt_in associatedReset arst_n
set_interface_property pkt_in dataBitsPerSymbol 8
set_interface_property pkt_in errorDescriptor ""
set_interface_property pkt_in firstSymbolInHighOrderBits true
set_interface_property pkt_in maxChannel 0
set_interface_property pkt_in readyLatency 0
set_interface_property pkt_in ENABLED true
set_interface_property pkt_in EXPORT_OF ""
set_interface_property pkt_in PORT_NAME_MAP ""
set_interface_property pkt_in CMSIS_SVD_VARIABLES ""
set_interface_property pkt_in SVD_ADDRESS_GROUP ""

add_interface_port pkt_in from_l8_data data Input 64
add_interface_port pkt_in from_l8_empty empty Input 3
add_interface_port pkt_in from_l8_endofpacket endofpacket Input 1
add_interface_port pkt_in from_l8_ready ready Output 1
add_interface_port pkt_in from_l8_startofpacket startofpacket Input 1
add_interface_port pkt_in from_l8_valid valid Input 1


# 
# connection point pkt_out
# 
add_interface pkt_out avalon_streaming start
set_interface_property pkt_out associatedClock clk
set_interface_property pkt_out associatedReset arst_n
set_interface_property pkt_out dataBitsPerSymbol 8
set_interface_property pkt_out errorDescriptor ""
set_interface_property pkt_out firstSymbolInHighOrderBits true
set_interface_property pkt_out maxChannel 0
set_interface_property pkt_out readyLatency 0
set_interface_property pkt_out ENABLED true
set_interface_property pkt_out EXPORT_OF ""
set_interface_property pkt_out PORT_NAME_MAP ""
set_interface_property pkt_out CMSIS_SVD_VARIABLES ""
set_interface_property pkt_out SVD_ADDRESS_GROUP ""

add_interface_port pkt_out to_l8_data data Output 64
add_interface_port pkt_out to_l8_startofpacket startofpacket Output 1
add_interface_port pkt_out to_l8_empty empty Output 3
add_interface_port pkt_out to_l8_ready ready Input 1
add_interface_port pkt_out to_l8_valid valid Output 1
add_interface_port pkt_out to_l8_endofpacket endofpacket Output 1


# 
# connection point clk
# 
add_interface clk clock end
set_interface_property clk clockRate 0
set_interface_property clk ENABLED true
set_interface_property clk EXPORT_OF ""
set_interface_property clk PORT_NAME_MAP ""
set_interface_property clk CMSIS_SVD_VARIABLES ""
set_interface_property clk SVD_ADDRESS_GROUP ""

add_interface_port clk clk clk Input 1


# 
# connection point arst_n
# 
add_interface arst_n reset end
set_interface_property arst_n associatedClock ""
set_interface_property arst_n synchronousEdges NONE
set_interface_property arst_n ENABLED true
set_interface_property arst_n EXPORT_OF ""
set_interface_property arst_n PORT_NAME_MAP ""
set_interface_property arst_n CMSIS_SVD_VARIABLES ""
set_interface_property arst_n SVD_ADDRESS_GROUP ""

add_interface_port arst_n xrst reset_n Input 1
