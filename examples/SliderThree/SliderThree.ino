/*
 * This is a demo sketch for capacitives slider, using ADC.
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

#include "CapacitiveADCSlider.h"
#include "DigitalPin.h"

const uint8_t pin0 = A0;
const uint8_t pin1 = A1;
const uint8_t pin2 = A2;

const uint8_t ledPin = 13;

CapacitiveADCSlider slider;
DigitalPin lampe;

void setup(){
	Serial.begin(115200);

	slider.init(pin0, pin1, pin2);
	slider.setChargeDelay(5);
	slider.tuneBaseline();

	lampe.init(ledPin, OUTPUT);
	lampe = 0;
}

void loop(){

}