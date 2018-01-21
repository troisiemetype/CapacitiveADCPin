/*
 * This is a demo sketch for capacitives pins, using ADC.
 * Copyright (C) 2017  Pierre-Loup Martin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{

} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CapacitiveADCChannel.h"
#include "DigitalPin.h"

const uint8_t readPin = A0;
const uint8_t friendPin = A1;

const uint8_t ledPin = 13;

CapacitiveADCChannel touchChannel;
DigitalPin lampe;

void setup(){
	Serial.begin(115200);

	touchChannel.init(readPin, friendPin);
	lampe.init(ledPin, OUTPUT);
	lampe = 0;
}

void loop(){
	int16_t value = 0;
	value = touchChannel.read();
//	value = touchChannel;				// operator overloading
	if(value > 300){
		lampe = 1;
	} else {
		lampe = 0;
	}
	Serial.print("delta: ");
	Serial.println(value);
	Serial.println();
	delay(100);

}