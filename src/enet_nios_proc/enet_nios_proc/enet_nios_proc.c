//------------------------------------------------------------------------------
// enet_nios_proc.c
//
//   Send commands to SFP-plus I/O expander on c10gx dev kit to setup state of side-band signals
//   Typically, this turns TX_DISABLE signal LOW allows SFP to start TX optical output
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

#include <stdio.h>
#include <unistd.h>
#include "altera_avalon_i2c.h"

#define DEBUG_PRINT             0       // set 1 to print debug line to jtag stdout

//------------------------------------------------------------------------------
// peripherals
//------------------------------------------------------------------------------
ALT_AVALON_I2C_DEV_t *         __attribute__ ((aligned(4))) i2c_dev;
ALT_AVALON_I2C_MASTER_CONFIG_t __attribute__ ((aligned(4))) i2c_dev_cfg;

ALT_AVALON_I2C_STATUS_CODE
check_alt_avalon_i2c_cmd_write(ALT_AVALON_I2C_DEV_t *i2c_dev,
                         alt_u8 val,
                         alt_u8 issue_restart,
                         alt_u8 issue_stop) {
	ALT_AVALON_I2C_STATUS_CODE ret;
	ret = alt_avalon_i2c_cmd_write(i2c_dev, val, issue_restart, issue_stop);
	#if DEBUG_PRINT
		//printf("alt_avalon_i2c_cmd_write : %d\n", ret);
	#endif
	return ret;
}

int main()
{
	usleep(30000);

	//--------------------------------------------------------------------------
	// i2c master setup
	//--------------------------------------------------------------------------
	// open i2c master
	i2c_dev = alt_avalon_i2c_open(ENET_NIOS_I2C_NAME);
	if (NULL == i2c_dev) {
		printf("Error:I2C Master Open Failed\n");
		return 0;
	}

	// initialize
	i2c_dev->ip_freq_in_hz = 50000000;
	alt_avalon_i2c_init(i2c_dev);
	alt_avalon_i2c_master_config_speed_set(i2c_dev, &i2c_dev_cfg, 100000);
	alt_avalon_i2c_master_config_set(i2c_dev, &i2c_dev_cfg);

	// start i2c master
	ALT_AVALON_I2C_STATUS_CODE i2c_status;
	i2c_status = alt_avalon_i2c_enable(i2c_dev);
	if (ALT_AVALON_I2C_SUCCESS != i2c_status) {
		printf("Error:I2C Master Enable Failed\n");
		return 0;
	}

	//--------------------------------------------------------------------------
	// setup side-bands
	//--------------------------------------------------------------------------
	// TCA9534PWR
	// addr : 8'b0100000x
	// port : {SFP_PRSNT_IN, SFP_RS0_IN, SFP_RXLOS_IN, SFP_RS1_IN,
	//         SFP_TXFAULT_IN, SFP_TXDISABLE_IN, SFP_GLED_IN, SFP_RLED_IN}
	// LEDs are LOW ACTIVE
	const alt_u8 WRITE_ADDR = 0x40;

	// write 8'b11111000 to i/o config register (0x03)
	check_alt_avalon_i2c_cmd_write(i2c_dev, WRITE_ADDR, 0x01, 0x00);
	check_alt_avalon_i2c_cmd_write(i2c_dev, 0x03, 0x00, 0x00);
	check_alt_avalon_i2c_cmd_write(i2c_dev, 0xf8, 0x00, 0x01);

	// write 8'b00000000 to output register (0x01)
	check_alt_avalon_i2c_cmd_write(i2c_dev, WRITE_ADDR, 0x01, 0x00);
	check_alt_avalon_i2c_cmd_write(i2c_dev, 0x01, 0x00, 0x00);
	check_alt_avalon_i2c_cmd_write(i2c_dev, 0x00, 0x00, 0x01);

	printf("done\n");

	return 0;
}
