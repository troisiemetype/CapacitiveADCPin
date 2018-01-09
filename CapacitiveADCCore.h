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

#ifndef CAPACITIVE_ADC_CORE_H
#define CAPACITIVE_ADC_CORE_H

#include <Arduino.h>
#include "DigitalPin.h"

#if defined(MUX5)
#define ADC_MUX_GND		0x1F
#else
#define ADC_MUX_GND		0x0F
#endif

#define ADC_GND 		0xFF

#define READ_OFFSET 	0xFF

class CapacitiveADCCore{
public:
	// Getter for singleton instance
	static CapacitiveADCCore* getInstance();
	void killInstance();

	uint8_t addChannel();

	void setChargeDelay(uint8_t value);
	void setSamples(uint8_t value);
	void setDivider(uint8_t value);
	void setResetDelay(uint8_t value);
	void setDebounce(uint8_t value);
	void setNoiseDelta(uint8_t value);
	void setNoiseIncrement(uint8_t value);
	void setNoiseCountRising(uint8_t value);
	void setNoiseCountFalling(uint8_t value);

	int16_t read();

protected:
	void charge();
	void discharge();
	uint16_t share();
	void setMux(uint8_t channel);

	enum state_t{
		IDLE = 0,
		CHARGING,
		DISCHARGING,
		CONVERTING,
	} _state;

private:
	CapacitiveADCCore():_state(IDLE){};
	~CapacitiveADCCore(){};

	static CapacitiveADCCore *_singleton;
	uint8_t _numChannels;
	uint8_t _currentChannel;



};

class CapacitiveADCPin{
public:
	CapacitiveADCPin();

	void init(uint8_t pin, uint8_t friendPin = 0);

	void setChargeDelay(uint8_t value);

	int16_t read();
	operator int16_t();

protected:
	void charge();
	void discharge();
	uint16_t share();
	void setMux(uint8_t channel);

	uint8_t _transfertDelay;

private:
	DigitalPin _dPin;
	DigitalPin _dFriendPin;
	uint8_t _pin;
	uint8_t _friendPin;

	uint8_t _channel;
	uint8_t _friendChannel;
};

#endif