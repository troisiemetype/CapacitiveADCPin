/*
 * This is a demo sketch for capacitives wheel, using ADC.
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

#include "CapacitiveADCWheel.h"
#include "DigitalPin.h"

const uint8_t pin0 = A0;
const uint8_t pin1 = A1;
const uint8_t pin2 = A2;

const uint8_t ledPin = 13;

CapacitiveADCWheel wheel;
DigitalPin lampe;

void setup(){
	Serial.begin(115200);

	wheel.init(pin0, pin1, pin2);
	wheel.setChargeDelay(3);
	wheel.setSamples(3);
	wheel.setDivider(0);
	wheel.setTouchThreshold(1200);
	wheel.setTouchReleaseThreshold(800);
	wheel.setNoiseIncrement(2);
	wheel.setDebounce(2);
	wheel.tuneBaseline(250);

	lampe.init(ledPin, OUTPUT);
	lampe = 0;
}

void loop(){
	if(wheel.update()){
		Serial.println(wheel.getPosition());
//		Serial.println(wheel.getStep());
	}

}