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

struct lock digital_lock;

void
init_digital (void) {
	init_lock(&digital_lock, "digital lock");
}

uint8_t digitalGetAll() {
	acquire(&digital_lock);
	// read from the FPGA all 8 digital ports
	uint8_t result = ~(fpgaReadByte(FPGA_DIGITAL_BASE));
	release(&digital_lock);

	return result;
}

uint8_t digitalGet(uint8_t port) {
	acquire(&digital_lock);
	// gnab a byte from the FPGA and mask the port's bit...
	uint8_t result = ((~(fpgaReadByte(FPGA_DIGITAL_BASE)))>>port)&1;
	release(&digital_lock);

	return result;
}

