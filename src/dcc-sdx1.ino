/*************************************************************************
Title:    DCC interface for the SDX-1 sound generator
Authors:  Michael Petersen <railfan@drgw.net>
File:     $Id: $
License:  GNU General Public License v3

LICENSE:
    Copyright (C) 2015 Nathan D. Holmes & Michael D. Petersen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

*************************************************************************/

/*
  Listens to a DCC address and drives the SDX-1 throttle input accordingly.
  
  This sketch uses the NmraDcc library by Alex Shepherd.
  https://github.com/mrrwa
*/

#include <NmraDcc.h>

#define SPEED_MODE 28
#define SDX1_PIN   11

NmraDcc  Dcc ;
DCC_MSG  Packet ;

uint16_t listenAddr = 600;
uint8_t currentSpeed = 0;

void notifyDccSpeed(uint16_t Addr, uint8_t Speed, uint8_t ForwardDir, uint8_t MaxSpeed)
{
	uint16_t realAddr = Addr & 0x3FFF;
	if(realAddr == listenAddr)
	{
		// Off is 1, so adjust to make it zero
		if(Speed)
			currentSpeed = Speed - 1;
		else
			currentSpeed = 0;
	}
}

void setup()
{
	int i;

	Serial.begin(115200);

	digitalWrite(SDX1_PIN, HIGH);  // Pre-set high (so drive to SDX-1 is low)
	pinMode(SDX1_PIN, OUTPUT);

	Dcc.pin(0, 2, 1);

	// Call the main DCC Init function to enable the DCC Receiver
	Dcc.init( MAN_ID_DIY, 10, FLAGS_OUTPUT_ADDRESS_MODE, 0 );
}

void loop()
{
	static float throttle = 0, throttleSum;
	
	Dcc.process();

	if(currentSpeed > SPEED_MODE)
		currentSpeed = SPEED_MODE;

	throttle += (((float)currentSpeed - throttle) / 10.0);
	if(throttle > SPEED_MODE)
		throttle = SPEED_MODE;
	else if(throttle < 0)
		throttle = 0;

	float throttleWarped = (1.0 - (exp(-5.0 * throttle / SPEED_MODE))) * SPEED_MODE;  // Warp the value with an exponential to make the low end more responsive
	
	float analogOut = (((float)SPEED_MODE - throttleWarped) * 255.0) / SPEED_MODE;
	
	analogWrite(SDX1_PIN, analogOut);
	Serial.print(currentSpeed);
	Serial.print(" : ");
	Serial.print(throttle);
	Serial.print(" : ");
	Serial.print(throttleWarped);
	Serial.print(" : ");
	Serial.println(analogOut);
}

