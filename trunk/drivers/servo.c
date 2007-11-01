/*
 * The MIT License
 *
 * Copyright (c) 2007 MIT 6.270 Robotics Competition
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "config.h"
#include "fpga.h"
#include <lock.h>

/**
 * The FPGA has 10bit Servo registers for driving from 0ms - 3.96ms range
 *
 * 1lsb = 3.875us
 *
 * servo min = 0.25ms (65  lsb)
 * servo max = 2.31ms (595 lsb)
 *
 * Actual range in bits is more like 9bits
 *
 */

#define SERVO_RAW_MIN		65		
#define SERVO_RAW_MAX		595
#define SERVO_RAW_RANGE 530 // (SERVO_RAW_MAX-SERVO_RAW_MIN)
#define SERVO_RAW_SCALE (((float)SERVO_RAW_RANGE)/511.0)

struct lock servo_lock;

void
init_servo (void) {
	init_lock (&servo_lock, "servo lock");
}

void servoSetPosRaw(uint8_t servo, uint16_t pos) {
	acquire (&servo_lock);
	uint8_t sbase = FPGA_SERVO_BASE + servo*FPGA_SERVO_SIZE;
	fpgaWriteByte(sbase+FPGA_SERVO_LO,pos&0xFF);
	fpgaWriteByte(sbase+FPGA_SERVO_HI,pos>>8);
	release (&servo_lock);
}

/* pos = 0-511 */
void servoSetPos(uint8_t servo, uint16_t pos) {
	uint16_t posRaw = SERVO_RAW_MIN + (uint16_t)((float)(pos)*SERVO_RAW_SCALE);
	servoSetPosRaw(servo,posRaw);
}


void servoSetRange(uint8_t servo, uint16_t lower, uint16_t upper) {
}
