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
	uint8_t divider;					// The number that computes the average from reads
	uint8_t debounce;					// The number of reads for a touch to be detect

	uint8_t noiseDelta;					// Max delta for baseline adjust
	uint8_t expWeight;					// Weight for exp filter. ratio, 0 to 255. fixpoint math
	uint8_t noiseIncrement;				// Increment for noise detection
	uint16_t noiseCountRising;			// Number of reads above noiseDelta for baseline adjust
	uint16_t noiseCountFalling;			// Number of reads under noiseDelta for baseline adjust

	SettingsGlobal_t():	samples(4),
						divider(3),
						debounce(20),
						noiseDelta(2),
						expWeight(40),
						noiseIncrement(1),
						noiseCountRising(800),
						noiseCountFalling(10){}
};

struct SettingsLocal_t{
	// Threshold values
	int16_t touchThreshold;
	int16_t touchReleaseThreshold;
	int16_t proxThreshold;
	int16_t proxReleaseThreshold;

	SettingsLocal_t():	touchThreshold(50),
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

	void tuneBaseline();
	void tuneThreshold(uint32_t length = 5000);

	int16_t update();

	void setChargeDelay(uint8_t value);

	bool isTouched() const;
	bool isJustTouched() const;
	bool isJustTouchedReleased() const;

	bool isProx() const;
	bool isJustProx() const;
	bool isJustProxReleased() const;

	bool isJustReleased() const;

	uint8_t proxRatio() const;

	void setSamples(uint8_t value);
	void setDivider(uint8_t value);
	void setResetDelay(uint8_t value);

	void setTouchThreshold(uint16_t threshold);
	void setTouchReleaseThreshold(uint16_t threshold);
	void setProxThreshold(uint16_t threshold);
	void setProxReleaseThreshold(uint16_t threshold);

	void setDebounce(uint8_t value);
	void setNoiseDelta(uint8_t value);
	void setNoiseIncrement(uint8_t value);
	void setNoiseCountRising(uint8_t value);
	void setNoiseCountFalling(uint8_t value);

	uint16_t getBaseline();
	uint16_t getMaxDelta();

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
	static SettingsGlobal_t _gSettings;

	// Local (pin) settings
	SettingsLocal_t _lSettings;

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