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

#include "CapacitiveADCPin.h"

// Public methods

// Constructor
CapacitiveADCPin::CapacitiveADCPin(){
	// Definition for ADC, when more than 8 ADC channels (Atmega 32u4, 2560, etc.)
	#if defined(MUX5)
	ADMUX = 0b01011111;
	ADCSRA = 0b11000110;
	ADCSRB = 0b00000000;

	// Definition for ADC, when 8 ADC Channels or less (ATmega 328p)
	#else
	ADMUX = 0b01001111;
	ADCSRA = 0b11000110;
	ADCSRB = 0b00000000;

	#endif

	// Default transfer delay
	_transfertDelay = 6;

	// The first reading is longer than a normal one, so let's do one.
	while(ADCSRA & _BV(ADSC));

}


// Init method: affect a pin to the object.
void CapacitiveADCPin::init(uint8_t pin, uint8_t friendPin){
	// Get a local copy of the pin numbers
	_pin = pin;
	_friendPin = friendPin;
	_channel = _pin;
	_friendChannel = _friendPin;

	// Here we translate the (digital) pin number into analog.
	// As with Arduino's analogRead(), the class can be passed a pin number in either:
	// channel number (1 - 5 for Uno)
	// pin number (14 - 19 for Uno)
	// analog pin number (A0 - A5 for Uno)
	// NOTA: on nano, analog A6 and A7 would be digital pins 20 and 21, that dont exist.
#if defined(__AVR_ATmega32U4__)
	if(_channel >= 18) _channel -= 18;
	if(_friendChannel >= 18) _friendChannel -= 18;
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
	if(_channel >= 54) _channel -= 54;
	if(_friendChannel >= 54) _friendChannel -= 54;
#elif defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega644__)
	if(_channel >= 24) _channel -= 24;
	if(_friendChannel >= 24) _friendChannel -= 24;
#else
	if(_channel >= 14) _channel -= 14;
	if(_friendChannel >= 14) _friendChannel -= 14;
#endif

	// Control that this ADC channel exists
	if(_channel >= NUM_ANALOG_INPUTS || _friendChannel >= NUM_ANALOG_INPUTS) return;

	// Configure the friend pin, that will be used to charge the s&h internal capacitor.
	if(_friendChannel == 0 && _channel == 0){
		++_friendChannel;
		++_friendPin;
	}

	// For Atmega 32u4 (leonardo, micro) only: map the analog pin number to ADC channel.
#if defined(__AVR_ATmega32U4__)
	_channel = analogPinToChannel(_channel);
	_friendChannel = analogPinToChannel(_friendChannel);
#endif

	// We don't need to init the ADC mux now,
	// As we will need to charge and discharge it before to get a reading

	// We init the pin tied to the ADC channel, so it can be turned HIGH or LOW when needed.
	_dPin.init(_pin);
	_dFriendPin.init(_friendPin);
}

// Set the charge delay.
// This is the minimal delay for the charge to transfer from electrode to s&h capacitor,
// back and forth.
void CapacitiveADCPin::setChargeDelay(uint8_t value){
	_transfertDelay = value;
}


// Read function.
int16_t CapacitiveADCPin::read(){
	int16_t value = 0;
	// Charge the pin
	charge();
	// Wait for the electrode to be charged.
	delayMicroseconds(_transfertDelay);
//	Serial.print("charge: ");
	// Get a reading.
	value = share();
	// Discharge the pin.
	discharge();
	// Wait for the electrode to be discharged.
	delayMicroseconds(_transfertDelay);
//	Serial.print("discharge: ");
	// Get a reading.
	value -= share();

	// Add a fix offset to the return result, so the value is always above 0.
	return value + READ_OFFSET;
}

CapacitiveADCPin::operator int16_t(){
	return read();
}

// Private methods

// Charge the electrode
// Set the ADC sample & hold capacitor to 0 by setting mux to ground,
// Then charge the electrode by setting it to output HIGH.
void CapacitiveADCPin::charge(){
	// Discharge the ADC s&h cap by linking it to ground.
	setMux(ADC_GND);
	// Charge the electrode.
	_dPin.setDirection(OUTPUT);
	_dPin.set();
}

// Discharge the electrode
// Set the ADC sample & hold capacitor to max by setting mux to another channel,
// that is output HIGH.
// Then discharge the electrode by setting it to output LOW.
void CapacitiveADCPin::discharge(){
	// Charge the ADC.
	setMux(_friendChannel);
	_dFriendPin.setDirection(OUTPUT);
	_dFriendPin.set();
	// Discharge the electrode.
	_dPin.setDirection(OUTPUT);
	_dPin.clear();
}

// Share connect the electrode to its ADC channel.
// This is about the same as an analogRead():
// Set the pin to input (three-stated), then connect ADC mux,
// and start a conversion.
uint16_t CapacitiveADCPin::share(){
	uint16_t value = 0;
	// Set the pin to intput, three-stated.
	_dPin.setDirection(INPUT);
	// Set the ADC channel to that pin.
	setMux(_channel);
	// Launch a conversion, and wait for it to be done.
	ADCSRA |= _BV(ADSC);
	while(ADCSRA & _BV(ADSC));

	value = ADCL;
	value |= (ADCH << 8);

//	Serial.println(value);

	return value;
}

// Set the ADC to a channel.
// That can be ground for discharging, electrode pin for reading, or friend pin for charging.
void CapacitiveADCPin::setMux(uint8_t channel){
	// Set channel number to 0
	ADMUX &= ~(0x1F);

	// Tie ADC to ground for discharging
	if(channel == ADC_GND){
		ADMUX |= ADC_MUX_GND;
	#if defined(MUX5)
		ADCSRB &= ~_BV(MUX5);
	#endif

	} else {
		// Set channel to right channel number
		ADMUX |= (channel & 0x07);
	#if defined(MUX5)
		ADCSRB |= (ADCSRB & ~_BV(MUX5)) | (((channel >> 3) & 0x01) << MUX5);
	#endif
	}

}
