/**
 * \file main.c
 *
 * \author
 *      Microchip Technology: http://www.microchip.com \n
 *      Support at http://www.microchip.com/support/ \n
 *
 * Copyright (C) 2018 Microchip Technology. All rights reserved.
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Microchip may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 * Microchip AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY Microchip "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL Microchip BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *****************************************************
 * Introduction
 * ============
 *
 * This example code demonstrates how to do compensation for static drift. The
 * drift is not measured, as it is static. Temperature is not considered.
 *
 * As this examples uses a known value for the drift, the direction of the drift
 * is also known.
 *
 * The RTC is configured to generate an interrupt every second. In the ISR, a
 * variable keeps track of the cumulative error. If the error is larger than one
 * 32.768 kHz clock cycle, the RTC period is reduced to compensate for this.
 *
 * Related documents / Application notes
 * -------------------------------------
 *
 * This application is described in the following application note: To be published
 *
 * Supported evaluation kit
 * ------------------------
 *
 *  - ATTiny817-XPRO
 *
 *
 * Running the demo
 * ----------------
 *
 * 1. Press Download Pack and save the .atzip file
 * 2. Import .atzip file into Atmel Studio 7, File->Import->Atmel Start Project.
 * 3. Follow the instructions in the ATtiny817 Xplained Pro User Guide to connect the crystal to the device.
 * 4. Build the project and program the supported evaluation board
 *******************************************************
 */

#include <avr/io.h>
#include <avr/interrupt.h>

// This defines the number of 32.768 kHz crystal cycles + static error (2)
// Static error must be measured in production, 2 is just an example
// The RTC period is defined as 0 to TOP, therefore 32768-1 is used
#define ONE_SECOND_TICK_COUNT 32767 + 2
volatile uint16_t accumulated_error = 0;
// Maximum error 30.5us, corresponds to 1 32.768 kHz clock cycle
#define MAX_ERROR 305
// Accumulated error 9.6u
// This is measured in production and is individual to all boards/devices
#define ERROR_PER_SECOND 96

ISR(RTC_CNT_vect)
{
	// Clear interrupt flag
	RTC_INTFLAGS = RTC_OVF_bm;
	// Increase the accumulated error
	accumulated_error += ERROR_PER_SECOND;
	// If the accumulated error is larger than one period of the 32.768 kHz crystal clock
	// remove one cycle from the RTC period. Set it back if the accumulated
	// error is less than one period of the 32.768 kHz crystal clock
	if (accumulated_error >= MAX_ERROR) {
		RTC_PER = ONE_SECOND_TICK_COUNT - 1;
		accumulated_error -= MAX_ERROR;
	} else {
		RTC_PER = ONE_SECOND_TICK_COUNT;
	}
}

void drift_compensation()
{
	/*
	 * The crystal is started at the beginning of main in order to give the crystal time to start.
	 * The startup time of the crystal is set to the max value to give the crystal time to settle.
	 * The startup time is based on counting 64K cycles of the crystal. With this startup time it
	 * will take approximately 2 seconds before the crystal is ready to be used by peripherals.
	 *
	 * The run in standby bit is set for the crystal. This will allow the crystal to run in standby
	 * sleep mode. In addition it will allow the crystal to start up even if no peripherals are
	 * requesting the clock.
	 */
	_PROTECTED_WRITE(CLKCTRL_XOSC32KCTRLA, CLKCTRL_ENABLE_bm | CLKCTRL_RUNSTDBY_bm | CLKCTRL_CSUT_64K_gc);

	// Enable global interrupts
	sei();

	// The RTC_STATUS needs to be 0 before writing to the RTC (could be used for start-up time).
	while (RTC.STATUS != 0) {
	}

	// Configure RTC with external 32K osc, one second period, and overflow interrupt enabled
	RTC_CLKSEL  = RTC_CLKSEL_TOSC32K_gc;
	RTC_PER     = ONE_SECOND_TICK_COUNT;
	RTC_INTCTRL = RTC_OVF_bm;
	RTC_CTRLA   = RTC_RTCEN_bm;

	while (1)
		;
}

int main(void)
{
	drift_compensation();

	while (1) {
	}
}
