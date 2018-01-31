/*
 * This Arduino library is for using Arduino pins as capacitives pins, using ADC.
 * Copyright (C) 2017  Pierre-Loup Martin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CAP_ADC_WHEEL_H
#define CAP_ADC_WHEEL_H

#define MAX_WHEEL_CHANNEL		3

#include <Arduino.h>
#include "CapacitiveADCSlider.h"

class CapADCWheel: public CapADCSlider{
public:

	CapADCWheel(void);
	~CapADCWheel(void);

	void init(uint8_t pin0, uint8_t pin1);
	void init(uint8_t pin0, uint8_t pin1, uint8_t pin2);
//	void init(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3);
//	void init(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4);
//	void init(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5);

protected:
	bool updatePosition(void);

};

#endif