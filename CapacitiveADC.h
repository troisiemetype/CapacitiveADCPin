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

#ifndef CAP_ADC_H
#define CAP_ADC_H

#include <Arduino.h>
#include "CapacitiveADCChannel.h"

struct CapADCSetGlobal_t{
	uint8_t samples; 					// The number of samples taken for one read
	uint8_t divider;					// The number that computes the average from reads
	uint8_t expWeight;					// Weight for exp filter. ratio, 0 to 255. fixpoint math
	uint8_t debounce;					// Delay to wait before a touch is effectively accounted
	uint8_t noiseIncrement;				// Increment for noise detection
	uint16_t noiseCountRising;			// Number of reads above noiseDelta for baseline adjust
	uint16_t noiseCountFalling;			// Number of reads under noiseDelta for baseline adjust

	CapADCSetGlobal_t():	samples(4),
						divider(1),
						expWeight(40),
						debounce(2),
						noiseIncrement(1),
						noiseCountRising(50),
						noiseCountFalling(5){}
};

struct CapADCSetLocal_t{
	// Threshold values
	int16_t touchThreshold;
	int16_t releaseThreshold;
	uint8_t resetCounter;

	CapADCSetLocal_t():	touchThreshold(50),
						releaseThreshold(40),
						resetCounter(255){}
};

class CapADC{
public:

	enum state_t{
		Idle = 0,
		BaselineChanged,		// 1
		Rising,					// 2
		Falling,				// 3
		Prox,					// 4
		Touch,					// 5
	};

	virtual void setChargeDelay(uint8_t value) = 0;

	virtual void setTouchThreshold(uint16_t threshold);
	virtual void setReleaseThreshold(uint16_t threshold);

	void applyGlobalSettings(const CapADCSetGlobal_t& settings);
	CapADCSetGlobal_t* globalSettings();
	CapADCSetGlobal_t getGlobalSettings()const;


	void applyLocalSettings(const CapADCSetLocal_t& settings);
	CapADCSetLocal_t* localSettings();
	CapADCSetLocal_t getLocalSettings()const;

protected:

	// Global settings
	static CapADCSetGlobal_t _gSettings;

	// Local (pin) settings
	CapADCSetLocal_t _lSettings;

};

#endif