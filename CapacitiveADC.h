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

#ifndef CAPACITIVE_ADC_H
#define CAPACITIVE_ADC_H

#include <Arduino.h>
#include "CapacitiveADCPin.h"

struct SettingsGlobal_t{
	uint8_t samples; 					// The number of samples taken for one read
	uint8_t debounce;					// The number of reads for a touch to be detect

	uint8_t maxDelta;					// Max delta for baseline adjust
	uint8_t noiseIncrement;				// Increment for noise detection
	uint16_t noiseCountRising;			// Number of reads above maxDelta for baseline adjust
	uint16_t noiseCountFalling;			// Number of reads under maxDelta for baseline adjust

	SettingsGlobal_t():samples(1),
						debounce(4),
						maxDelta(2),
						noiseIncrement(1),
						noiseCountRising(1600),
						noiseCountFalling(50){}
};

struct SettingsLocal_t{
	// Threshold values
	int16_t touchThreshold;
	int16_t touchReleaseThreshold;
	int16_t proxThreshold;
	int16_t proxReleaseThreshold;

	SettingsLocal_t():touchThreshold(50),
						touchReleaseThreshold(40),
						proxThreshold(5),
						proxReleaseThreshold(3){}
};

class CapacitiveADC{
public:

	enum state_t{
		Idle = 0,
		BaselineChanged,		// 1
		Rising,					// 2
		Falling,				// 3
		Prox,					// 4
		Touch,					// 5
	};

	CapacitiveADC();
	~CapacitiveADC();

	void init(uint8_t pin, uint8_t friendPin = 0);

	void autoTune();

	uint16_t update();

	void setChargeDelay(uint8_t value);

	bool isTouched() const;
	bool isJustTouched() const;
	bool isJustTouchedReleased() const;

	bool isProx() const;
	bool isJustProx() const;
	bool isJustProxReleased() const;

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

	void applyGlobalSettings(const SettingsGlobal_t& settings);
	SettingsGlobal_t getGlobalSettings() const;

	void applyLocalSettings(const SettingsLocal_t& settings);
	SettingsLocal_t getLocalSettings() const;

protected:
	uint16_t updateRead();
	void updateCal();

private:
	// The pin linked to this capacitive channel
	CapacitiveADCPin *_adcPin;

	// Global settings
	SettingsGlobal_t *_gSettings;

	// Local (pin) settings
	SettingsLocal_t _lSettings;

	// values from readings
	uint16_t _read;

	// Settings for filtering
	uint16_t _baseline;
	uint16_t _counter;
	uint16_t _lastTime;

	// States of sensing
	uint8_t _state;
	uint8_t _previousState;

};

#endif