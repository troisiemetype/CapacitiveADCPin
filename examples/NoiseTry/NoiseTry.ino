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

#include "CapacitiveADC.h"
#include "DigitalPin.h"


const uint8_t SENSE1 = A4;
const uint8_t SENSE2 = A3;
const uint8_t SENSE3 = A2;
const uint8_t SENSE4 = A1;

const uint8_t numSense = 1;
CapacitiveADC sense[numSense];

void setup(){

	Serial.begin(115200);
	while(!Serial);



	sense[0].init(SENSE1, SENSE2);
//	sense[1].init(SENSE2, SENSE1);
//	sense[2].init(SENSE3, SENSE1);
//	sense[3].init(SENSE4, SENSE1);

	for(uint8_t i = 0; i < numSense; ++i){
		sense[i].setChargeDelay(5);
		sense[i].tuneBaseline();
	}
}

void loop(){
	int16_t value = 0;

	for(uint8_t i = 0; i < numSense; ++i){
		Serial.print(sense[i].update());
		Serial.print('\t');
	}
	Serial.println();

	/*
	if(value > 300){
		lampe = 1;
	} else {
		lampe = 0;
	}
	*/
/*
	Serial.print("raw: ");
	Serial.println(value);
	Serial.println();
*/

//	delay(10);

}