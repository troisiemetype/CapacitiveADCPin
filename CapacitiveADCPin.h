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

#ifndef CAP_ADC_PIN_H
#define CAP_ADC_PIN_H

#include <Arduino.h>
#include "CapacitiveADC.h"

class CapADCPin: public CapADC{
public:

	CapADCPin();
	~CapADCPin();

	void init(uint8_t pin, uint8_t friendPin = 0);

	void setChargeDelay(uint8_t value);


	void tuneBaseline(uint32_t length = 1000);
	void tuneThreshold(uint32_t length = 5000);

	int16_t update();

	bool isTouched() const;
	bool isJustTouched() const;
	bool isJustReleased() const;

	uint16_t getBaseline() const{return _baseline;}
	uint16_t getMaxDelta() const {return _maxDelta;}
	int16_t getDelta() const {return _delta;}

//	void applyLocalSettings(const CapADCSetLocal_t& settings);
//	CapADCSetLocal_t getLocalSettings() const;

protected:
	uint16_t updateRead();
	void updateCal();

	// The pin linked to this capacitive channel;
	CapADCChannel *_adcChannel;

/*
	// Local (pin) settings
	CapADCSetLocal_t _lSettings;
*/
	// values from readings
	uint16_t _read;
	uint16_t _lastRead;
	int16_t _delta;

	// Settings for filtering
	uint16_t _baseline;
	uint16_t _maxDelta;
	uint32_t _lastTime;

	// States of sensing, instant and for reading
	uint8_t _now;
	uint8_t _prev;
	uint8_t _state;
	uint8_t _previousState;

};

#endif