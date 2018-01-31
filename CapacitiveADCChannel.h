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

#ifndef CAP_ADC_CHANNEL_H
#define CAP_ADC_CHANNEL_H

#include <Arduino.h>


class CapADCChannel{
public:
	CapADCChannel();

	void init(uint8_t pin, uint8_t friendPin);

	void setChargeDelay(uint8_t value);

	int16_t read();

protected:
//	uint8_t share();
	void setMux(uint8_t channel);

	uint8_t _transfertDelay;

private:
	uint8_t *_portRPin;
	uint8_t *_pinRPin;
	uint8_t *_ddrRPin;
	uint8_t _maskPin;
	
	uint8_t *_portRFriendPin;
	uint8_t *_pinRFriendPin;
	uint8_t *_ddrRFriendPin;
	uint8_t _maskFriendPin;


	uint8_t _channel;
	uint8_t _friendChannel;
};

#endif