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

#ifndef CAPACITIVE_ADC_PIN_H
#define CAPACITIVE_ADC_PIN_H

#include <Arduino.h>
#include "CapacitiveADC.h"

class CapacitiveADCPin: public CapacitiveADC{
public:

	CapacitiveADCPin();
	~CapacitiveADCPin();

	void init(uint8_t pin, uint8_t friendPin = 0);

	void tuneBaseline(uint32_t length = 1000);
	void tuneThreshold(uint32_t length = 5000);

	int16_t update();

	bool isTouched() const;
	bool isJustTouched() const;
	bool isJustTouchedReleased() const;

	bool isProx() const;
	bool isJustProx() const;
	bool isJustProxReleased() const;

	bool isJustReleased() const;

	uint8_t proxRatio() const;

	void setProxThreshold(uint16_t threshold);
	void setProxReleaseThreshold(uint16_t threshold);

	uint16_t getBaseline();
	uint16_t getMaxDelta();

//	void applyLocalSettings(const SettingsLocal_t& settings);
//	SettingsLocal_t getLocalSettings() const;

protected:
	uint16_t updateRead();
	void updateCal();

	// The pin linked to this capacitive channel
	CapacitiveADCChannel *_adcChannel;

/*
	// Local (pin) settings
	SettingsLocal_t _lSettings;
*/
	// values from readings
	uint16_t _read;
	uint16_t _lastRead;
	int16_t _delta;

	// Settings for filtering
	uint16_t _baseline;
	uint16_t _minBaseline, _maxBaseline;
	uint32_t _counter;
	uint32_t _resetCounter;
	uint32_t _lastTime;

	// States of sensing, instant and for reading
	uint8_t _now;
	uint8_t _prev;
	uint8_t _state;
	uint8_t _previousState;

};

#endif