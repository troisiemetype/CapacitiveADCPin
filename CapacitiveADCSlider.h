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

#ifndef CAP_ADC_SLIDER_H
#define CAP_ADC_SLIDER_H

#define MAX_SLIDER_CHANNEL		4

#include <Arduino.h>
#include "CapacitiveADC.h"

class CapADCSlider: public CapADC{
public:

	CapADCSlider(void);
	~CapADCSlider(void);

	virtual void init(uint8_t pin0, uint8_t pin1);
	void init(uint8_t pin0, uint8_t pin1, uint8_t pin2);
	void init(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3);
//	void init(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4);
//	void init(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5);

	void setChargeDelay(uint8_t value);

	void tuneBaseline(uint32_t length = 200);
	void tuneThreshold(uint32_t length = 2000);

	int16_t update(void);

	bool isTouched(void) const;
	int8_t getPosition(void) const;
	int8_t getStep(void);

	uint16_t getBaseline(void) const;

//	void applyLocalSettings(const CapADCSetLocal_t& settings);
//	CapADCSetLocal_t getLocalSettings() const;

protected:
	virtual bool updatePosition(void);
	uint16_t updateRead(uint8_t index);
	void updateCal(uint8_t index);

	// The pin linked to this capacitive channel
	CapADCChannel* _adcChannel[MAX_SLIDER_CHANNEL];

	uint8_t _numChannels;

	// Local (pin) settings
	CapADCSetLocal_t _lSettings;

	// values from readings
	uint16_t _currentRead[MAX_SLIDER_CHANNEL + 1];
	uint16_t _previousRead[MAX_SLIDER_CHANNEL + 1];
	int16_t _delta[MAX_SLIDER_CHANNEL + 1];

	// Settings for filtering
	uint16_t _baseline[MAX_SLIDER_CHANNEL + 1];
	uint8_t _resetCounter[MAX_SLIDER_CHANNEL + 1];
	uint32_t _lastTime[MAX_SLIDER_CHANNEL + 1];

	// States of sensing, instant and for reading
	uint8_t _nowState[MAX_SLIDER_CHANNEL + 1];
	uint8_t _prevState[MAX_SLIDER_CHANNEL + 1];
	uint8_t _state[MAX_SLIDER_CHANNEL + 1];
	uint8_t _previousState[MAX_SLIDER_CHANNEL + 1];

	int8_t _nowPosition, _prevPosition, _position;
	int8_t _step;

	int8_t _weighting[MAX_SLIDER_CHANNEL];

	uint8_t _coeff;
};

#endif