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

#include <avr/interrupt.h>
#include <global.h>
#include <board.h>
#include <util.h>
#include <util/delay.h>
#include <lcd.h>
#include <lock.h>
#include <async_printf.h>

void
waitForClick(char *msg) {
	if (msg) {
		// TODO: clean up, make more general (don't assume LCD)
		lcdClear();
		// print to LCD
		lcd_printf("%s", msg);
		// print to UART
		uart_printf("%s\n", msg);
	}

	goClick();
}

// prints n bytes of 'bytes' to UART
void
dumpBytes (uint8_t *bytes, uint8_t n) {
	uart_printf("INDEX   ADDR   VALUE\n");
	uart_printf("====================\n");
	for (int i = 0; i < n; i++) {
		uart_printf("% 5d   %4p   %02x\n", i, (void *)(bytes + i), bytes[i]);
	}
}

int 
printf (const char *fmt, ...) {
	extern struct lock lcd_lock;
	extern FILE lcdout;
	acquire(&lcd_lock);
	va_list ap;
	int count;

	va_start(ap, fmt);
	count = vfprintf (&lcdout, fmt, ap);
	//count = async_vfprintf (&lcdout, fmt, ap);
	va_end(ap);

	release(&lcd_lock);
	return count;
}

