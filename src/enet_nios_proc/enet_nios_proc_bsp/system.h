/*
 * system.h - SOPC Builder system and BSP software package information
 *
 * Machine generated for CPU 'enet_nios_proc' in SOPC Builder design 'system'
 * SOPC Builder design path: /opt/work/c10gx/c10gx_icmp_server/src/system/system/system.sopcinfo
 *
 * Generated: Thu Dec 03 01:23:41 JST 2020
 */

/*
 * DO NOT MODIFY THIS FILE
 *
 * Changing this file will have subtle consequences
 * which will almost certainly lead to a nonfunctioning
 * system. If you do modify this file, be aware that your
 * changes will be overwritten and lost when this file
 * is generated again.
 *
 * DO NOT MODIFY THIS FILE
 */

/*
 * License Agreement
 *
 * Copyright (c) 2008
 * Altera Corporation, San Jose, California, USA.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This agreement shall be governed in all respects by the laws of the State
 * of California and by the laws of the United States of America.
 */

#ifndef __SYSTEM_H_
#define __SYSTEM_H_

/* Include definitions from linker script generator */
#include "linker.h"


/*
 * CPU configuration
 *
 */

#define ALT_CPU_ARCHITECTURE "altera_nios2_gen2"
#define ALT_CPU_BIG_ENDIAN 0
#define ALT_CPU_BREAK_ADDR 0x00080020
#define ALT_CPU_CPU_ARCH_NIOS2_R1
#define ALT_CPU_CPU_FREQ 50000000u
#define ALT_CPU_CPU_ID_SIZE 1
#define ALT_CPU_CPU_ID_VALUE 0x00000000
#define ALT_CPU_CPU_IMPLEMENTATION "tiny"
#define ALT_CPU_DATA_ADDR_WIDTH 0x14
#define ALT_CPU_DCACHE_LINE_SIZE 0
#define ALT_CPU_DCACHE_LINE_SIZE_LOG2 0
#define ALT_CPU_DCACHE_SIZE 0
#define ALT_CPU_EXCEPTION_ADDR 0x00000020
#define ALT_CPU_FLASH_ACCELERATOR_LINES 0
#define ALT_CPU_FLASH_ACCELERATOR_LINE_SIZE 0
#define ALT_CPU_FLUSHDA_SUPPORTED
#define ALT_CPU_FREQ 50000000
#define ALT_CPU_HARDWARE_DIVIDE_PRESENT 0
#define ALT_CPU_HARDWARE_MULTIPLY_PRESENT 0
#define ALT_CPU_HARDWARE_MULX_PRESENT 0
#define ALT_CPU_HAS_DEBUG_CORE 1
#define ALT_CPU_HAS_DEBUG_STUB
#define ALT_CPU_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define ALT_CPU_HAS_JMPI_INSTRUCTION
#define ALT_CPU_ICACHE_LINE_SIZE 0
#define ALT_CPU_ICACHE_LINE_SIZE_LOG2 0
#define ALT_CPU_ICACHE_SIZE 0
#define ALT_CPU_INST_ADDR_WIDTH 0x14
#define ALT_CPU_NAME "enet_nios_proc"
#define ALT_CPU_OCI_VERSION 1
#define ALT_CPU_RESET_ADDR 0x00000000


/*
 * CPU configuration (with legacy prefix - don't use these anymore)
 *
 */

#define NIOS2_BIG_ENDIAN 0
#define NIOS2_BREAK_ADDR 0x00080020
#define NIOS2_CPU_ARCH_NIOS2_R1
#define NIOS2_CPU_FREQ 50000000u
#define NIOS2_CPU_ID_SIZE 1
#define NIOS2_CPU_ID_VALUE 0x00000000
#define NIOS2_CPU_IMPLEMENTATION "tiny"
#define NIOS2_DATA_ADDR_WIDTH 0x14
#define NIOS2_DCACHE_LINE_SIZE 0
#define NIOS2_DCACHE_LINE_SIZE_LOG2 0
#define NIOS2_DCACHE_SIZE 0
#define NIOS2_EXCEPTION_ADDR 0x00000020
#define NIOS2_FLASH_ACCELERATOR_LINES 0
#define NIOS2_FLASH_ACCELERATOR_LINE_SIZE 0
#define NIOS2_FLUSHDA_SUPPORTED
#define NIOS2_HARDWARE_DIVIDE_PRESENT 0
#define NIOS2_HARDWARE_MULTIPLY_PRESENT 0
#define NIOS2_HARDWARE_MULX_PRESENT 0
#define NIOS2_HAS_DEBUG_CORE 1
#define NIOS2_HAS_DEBUG_STUB
#define NIOS2_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define NIOS2_HAS_JMPI_INSTRUCTION
#define NIOS2_ICACHE_LINE_SIZE 0
#define NIOS2_ICACHE_LINE_SIZE_LOG2 0
#define NIOS2_ICACHE_SIZE 0
#define NIOS2_INST_ADDR_WIDTH 0x14
#define NIOS2_OCI_VERSION 1
#define NIOS2_RESET_ADDR 0x00000000


/*
 * Define for each module class mastered by the CPU
 *
 */

#define __ALTERA_AVALON_I2C
#define __ALTERA_AVALON_JTAG_UART
#define __ALTERA_AVALON_ONCHIP_MEMORY2
#define __ALTERA_NIOS2_GEN2
#define __ALT_EM10G32


/*
 * System configuration
 *
 */

#define ALT_DEVICE_FAMILY "Cyclone 10 GX"
#define ALT_ENHANCED_INTERRUPT_API_PRESENT
#define ALT_IRQ_BASE NULL
#define ALT_LOG_PORT "/dev/null"
#define ALT_LOG_PORT_BASE 0x0
#define ALT_LOG_PORT_DEV null
#define ALT_LOG_PORT_TYPE ""
#define ALT_NUM_EXTERNAL_INTERRUPT_CONTROLLERS 0
#define ALT_NUM_INTERNAL_INTERRUPT_CONTROLLERS 1
#define ALT_NUM_INTERRUPT_CONTROLLERS 1
#define ALT_STDERR "/dev/enet_nios_jtaguart"
#define ALT_STDERR_BASE 0x82000
#define ALT_STDERR_DEV enet_nios_jtaguart
#define ALT_STDERR_IS_JTAG_UART
#define ALT_STDERR_PRESENT
#define ALT_STDERR_TYPE "altera_avalon_jtag_uart"
#define ALT_STDIN "/dev/enet_nios_jtaguart"
#define ALT_STDIN_BASE 0x82000
#define ALT_STDIN_DEV enet_nios_jtaguart
#define ALT_STDIN_IS_JTAG_UART
#define ALT_STDIN_PRESENT
#define ALT_STDIN_TYPE "altera_avalon_jtag_uart"
#define ALT_STDOUT "/dev/enet_nios_jtaguart"
#define ALT_STDOUT_BASE 0x82000
#define ALT_STDOUT_DEV enet_nios_jtaguart
#define ALT_STDOUT_IS_JTAG_UART
#define ALT_STDOUT_PRESENT
#define ALT_STDOUT_TYPE "altera_avalon_jtag_uart"
#define ALT_SYSTEM_NAME "system"


/*
 * enet_10gmac_r0 configuration
 *
 */

#define ALT_MODULE_CLASS_enet_10gmac_r0 alt_em10g32
#define ENET_10GMAC_R0_BASE 0x83000
#define ENET_10GMAC_R0_IRQ -1
#define ENET_10GMAC_R0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define ENET_10GMAC_R0_NAME "/dev/enet_10gmac_r0"
#define ENET_10GMAC_R0_SPAN 4096
#define ENET_10GMAC_R0_TYPE "alt_em10g32"


/*
 * enet_nios_i2c configuration
 *
 */

#define ALT_MODULE_CLASS_enet_nios_i2c altera_avalon_i2c
#define ENET_NIOS_I2C_BASE 0x81000
#define ENET_NIOS_I2C_FIFO_DEPTH 4
#define ENET_NIOS_I2C_FREQ 50000000
#define ENET_NIOS_I2C_IRQ 0
#define ENET_NIOS_I2C_IRQ_INTERRUPT_CONTROLLER_ID 0
#define ENET_NIOS_I2C_NAME "/dev/enet_nios_i2c"
#define ENET_NIOS_I2C_SPAN 64
#define ENET_NIOS_I2C_TYPE "altera_avalon_i2c"
#define ENET_NIOS_I2C_USE_AV_ST 0


/*
 * enet_nios_jtaguart configuration
 *
 */

#define ALT_MODULE_CLASS_enet_nios_jtaguart altera_avalon_jtag_uart
#define ENET_NIOS_JTAGUART_BASE 0x82000
#define ENET_NIOS_JTAGUART_IRQ 1
#define ENET_NIOS_JTAGUART_IRQ_INTERRUPT_CONTROLLER_ID 0
#define ENET_NIOS_JTAGUART_NAME "/dev/enet_nios_jtaguart"
#define ENET_NIOS_JTAGUART_READ_DEPTH 64
#define ENET_NIOS_JTAGUART_READ_THRESHOLD 8
#define ENET_NIOS_JTAGUART_SPAN 8
#define ENET_NIOS_JTAGUART_TYPE "altera_avalon_jtag_uart"
#define ENET_NIOS_JTAGUART_WRITE_DEPTH 64
#define ENET_NIOS_JTAGUART_WRITE_THRESHOLD 8


/*
 * enet_nios_mem configuration
 *
 */

#define ALT_MODULE_CLASS_enet_nios_mem altera_avalon_onchip_memory2
#define ENET_NIOS_MEM_ALLOW_IN_SYSTEM_MEMORY_CONTENT_EDITOR 0
#define ENET_NIOS_MEM_ALLOW_MRAM_SIM_CONTENTS_ONLY_FILE 0
#define ENET_NIOS_MEM_BASE 0x0
#define ENET_NIOS_MEM_CONTENTS_INFO ""
#define ENET_NIOS_MEM_DUAL_PORT 0
#define ENET_NIOS_MEM_GUI_RAM_BLOCK_TYPE "AUTO"
#define ENET_NIOS_MEM_INIT_CONTENTS_FILE "system_init_"
#define ENET_NIOS_MEM_INIT_MEM_CONTENT 1
#define ENET_NIOS_MEM_INSTANCE_ID "NONE"
#define ENET_NIOS_MEM_IRQ -1
#define ENET_NIOS_MEM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define ENET_NIOS_MEM_NAME "/dev/enet_nios_mem"
#define ENET_NIOS_MEM_NON_DEFAULT_INIT_FILE_ENABLED 1
#define ENET_NIOS_MEM_RAM_BLOCK_TYPE "AUTO"
#define ENET_NIOS_MEM_READ_DURING_WRITE_MODE "DONT_CARE"
#define ENET_NIOS_MEM_SINGLE_CLOCK_OP 0
#define ENET_NIOS_MEM_SIZE_MULTIPLE 1
#define ENET_NIOS_MEM_SIZE_VALUE 131072
#define ENET_NIOS_MEM_SPAN 131072
#define ENET_NIOS_MEM_TYPE "altera_avalon_onchip_memory2"
#define ENET_NIOS_MEM_WRITABLE 1


/*
 * hal configuration
 *
 */

#define ALT_INCLUDE_INSTRUCTION_RELATED_EXCEPTION_API
#define ALT_MAX_FD 32
#define ALT_SYS_CLK none
#define ALT_TIMESTAMP_CLK none

#endif /* __SYSTEM_H_ */
