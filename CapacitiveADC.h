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
 * but WITHOUT ANY WARRANTY{

} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CAPACITIVE_ADC_H
#define CAPACITIVE_ADC_H

#include <Arduino.h>
#include "CapacitiveADCPin.h"

struct SettingsGlobal_t{
	uint8_t samples;
	uint8_t debounce;

	uint8_t maxDelta;
	uint8_t noiseIncrement;
	uint8_t noiseCountRising;
	uint8_t noiseCountFalling;

	SettingsGlobal_t():samples(1),
						debounce(4),
						maxDelta(2),
						noiseIncrement(1),
						noiseCountRising(3),
						noiseCountFalling(1){}
};

class CapacitiveADC{
public:

	enum stat_t{
		Idle = 0,
		BaselineChanged,		// 1
		Rising,					// 2
		Falling,				// 3
		PreProx,				// 4
		PreTouch,				// 5
		Prox,					// 6
		Touch,					// 7
	};

	CapacitiveADC();
	~CapacitiveADC();

	void init(uint8_t pin, uint8_t friendPin = 0);

	uint16_t update();

	void setChargeDelay(uint8_t value);

	bool isTouched();
	bool isJustTouched();
	bool isJustTouchedReleased();

	bool isProx();
	bool isJustProx();
	bool isJustProxReleased();

	void setSamples(uint8_t value);

	void setTouchThreshold(uint8_t threshold);
	void setTouchReleaseThreshold(uint8_t threshold);
	void setProxThreshold(uint8_t threshold);
	void setProxReleaseThreshold(uint8_t threshold);

	void setDebounce(uint8_t value);
	void setMaxDelta(uint8_t value);
	void setNoiseIncrement(uint8_t value);
	void setNoiseCountRising(uint8_t value);
	void setNoiseCountFalling(uint8_t value);

	void setSettings(const SettingsGlobal_t& settings);
	SettingsGlobal_t getSettings();

protected:
	uint16_t updateRead();
	void updateCal();

private:
	// The pin linked to this capacitive channel
	CapacitiveADCPin *_adcPin;

	// Global settings
	SettingsGlobal_t *_gSettings;

	// The number of samples taken for one read.
//	uint8_t _samples; 								// Struct

	// Threshold values
	int16_t _touchThreshold;
	int16_t _touchReleaseThreshold;
	int16_t _proxThreshold;
	int16_t _proxReleaseThreshold;

	// Debounce value for prox or touch detection
//	uint8_t _debounce;								// Struct

	// values from readings
	uint16_t _read;

	// Settings for filtering
	uint8_t _baseline;
//	uint8_t _maxDelta;								// Struct
//	uint8_t _noiseIncrement;						// Struct
//	uint8_t _noiseCountRising;						// Struct
//	uint8_t _noiseCountFalling;						// Struct
	uint16_t _counter;

	// States of sensing
	uint8_t _state;
	uint8_t _previousState;

};

#endif