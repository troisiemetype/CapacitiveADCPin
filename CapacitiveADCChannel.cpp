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

#include "CapacitiveADCChannel.h"

#if defined(CORE_TEENSY)
static const uint8_t PROGMEM adc_mapping[] = {
// 0, 1, 4, 5, 6, 7, 13, 12, 11, 10, 9, 8
   0, 1, 4, 5, 6, 7, 13, 12, 11, 10, 9, 8, 10, 11, 12, 13, 7, 6, 5, 4, 1, 0, 8 
};

#endif

// Public methods

// Constructor
CapADCChannel::CapADCChannel(){
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
	_transfertDelay = 4;

	// The first reading is longer than a normal one, so let's do one.
	while(ADCSRA & _BV(ADSC));

}


// Init method: affect a pin to the object.
void CapADCChannel::init(uint8_t pin, uint8_t friendPin){
	// Get a local copy of the pin numbers
	_channel = pin;
	_friendChannel = friendPin;

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
#elif defined(__AVR_ATTiny24__) || defined(__AVR_ATTiny44__) || defined(__AVR_ATTiny84__) ||\
	defined(__AVR_ATTiny24A__) || defined(__AVR_ATTiny44A__) || defined(__AVR_ATTiny84A__)
	// No channel offset for ATtiny 24/44/84
	// But we have to check that the power reduction is not active
	PPR &= ~(1 << PRADC);
#else
	if(_channel >= 14) _channel -= 14;
	if(_friendChannel >= 14) _friendChannel -= 14;
#endif

	// Control that this ADC channel exists
	if(_channel >= NUM_ANALOG_INPUTS || _friendChannel >= NUM_ANALOG_INPUTS) return;

	// Configure the friend pin, that will be used to charge the s&h internal capacitor.
	/*
	if(_friendChannel == 0 && _channel == 0){
		++_friendChannel;
		++friendPin;
	}
	*/

	// For Atmega 32u4 (leonardo, micro) only: map the analog pin number to ADC channel.
#if defined(__AVR_ATmega32U4__)
	#if defined(CORE_TEENSY)
	_channel = pgm_read_byte(adc_mapping + _channel);
	_friendChannel = pgm_read_byte(adc_mapping + _friendChannel);
	#else
	_channel = analogPinToChannel(_channel);
	_friendChannel = analogPinToChannel(_friendChannel);
	#endif
#endif

	// We don't need to init the ADC mux now,
	// As we will need to charge and discharge it before to get a reading


	// We init the pin tied to the ADC channels.
	// As we will use direct port addressing, we have to set registers used in Arduino's variant files.
	uint8_t reg = digitalPinToPort(pin);
	_maskPin = digitalPinToBitMask(pin);

	_portRPin = (uint8_t*)portOutputRegister(reg);
	_pinRPin = (uint8_t*)portInputRegister(reg);
	_ddrRPin = (uint8_t*)portModeRegister(reg);	

	reg = digitalPinToPort(friendPin);
	_maskFriendPin = digitalPinToBitMask(friendPin);

	_portRFriendPin = (uint8_t*)portOutputRegister(reg);
	_pinRFriendPin = (uint8_t*)portInputRegister(reg);
	_ddrRFriendPin = (uint8_t*)portModeRegister(reg);

	// Turn both pin OUTPUT, LOW.
	*_ddrRPin |= _maskPin;
	*_portRPin &= ~_maskPin;
	*_ddrRFriendPin |= _maskFriendPin;
	*_portRFriendPin &= ~_maskFriendPin;

	// Set the ADC registers here, because analogRead initialise after third party libraries.
#if defined(MUX5)
	ADMUX = 0b01011111;
	ADCSRA = 0b10000011;
	ADCSRB = 0b00000000;

#elif defined(__AVR_ATTiny24__) || defined(__AVR_ATTiny44__) || defined(__AVR_ATTiny84__) ||\
	defined(__AVR_ATTiny24A__) || defined(__AVR_ATTiny44A__) || defined(__AVR_ATTiny84A__)
	// VCC as reference, channel 0 enabled (is changed on each reading)
	ADMUX = 0b00000000;
	// ADC enable, ADC start conversion 0, ADC auto trigger disabled, ADC interrupt disabled,
	// ADC prescaler set to 16 (0b10).
	ADCSRA = 0b10000010;
	// Bipolar input mode disable, multiplexer disabled, result right-adjusted, no auto-trigger.
	ADCSRB = 0b00000000;
	// See if we set the DIDR0 register, to disable digital input on pin used as ADC.
	// Definition for ADC, when 8 ADC Channels or less (ATmega 328p)
#else
	ADMUX = 0b01001111;
	ADCSRA = 0b10000011;
	ADCSRB = 0b00000000;

#endif

	// Left justified result (we only read the 8 upper bits)
//	ADMUX |= _BV(5);

}

// Set the charge delay.
// This is the minimal delay for the charge to transfer from electrode to s&h capacitor,
// back and forth.
void CapADCChannel::setChargeDelay(uint8_t value){
	_transfertDelay = value;
}


// Read function.
int16_t CapADCChannel::read(){
//	ADMUX |= _BV(5);
	ADCSRA &= ~0b111;
	ADCSRA |= 0b011;
	ADCSRB |= _BV(7);

	int16_t value = 0;
//	uint32_t length = micros();
	// Charge the pin
	// Discharge the ADC s&h cap by linking it to ground.
	// Turn friend pin OUTPUT, LOW
	*_portRFriendPin &= ~_maskFriendPin;
	setMux(_friendChannel);
	// Turn friend pin OUTPUT, LOW
//	*_ddrRFriendPin |= _maskFriendPin;

	// Charge the electrode.
	// Turn pin OUTPUT, HIGH.
//	*_ddrRPin |= _maskPin;
	*_portRPin |= _maskPin;
	// Wait for the electrode to be charged.
	delayMicroseconds(_transfertDelay);

	// Set the pin to input, three-stated.
	*_ddrRPin &= ~_maskPin;
	*_portRPin &= ~_maskPin;
	// Set the ADC channel to that pin.
	setMux(_channel);
	// Launch a conversion, and wait for it to be done.
	ADCSRA |= _BV(ADSC);

	// Wait at least one ADC clock cycle before to change ADMUX.
/*	asm volatile(
		"NOP"	"\n\t"
		"NOP"	"\n\t"
		"NOP"	"\n\t"
		"NOP"	"\n\t"
		);
*/

	// Set the next (we can change it before conversion it's done as ADMUX is buffered)

	while(ADCSRA & _BV(ADSC));

	// Get value from reading.
//	value = ADCH;
	value = ADCL;
	value += ((uint16_t)ADCH << 8);

//	Serial.println(value);

	setMux(_friendChannel);

	// Turn friend pin OUTPUT, HIGH
//	*_ddrRFriendPin |= _maskFriendPin;
	*_portRFriendPin |= _maskFriendPin;
	// Discharge the electrode.
	// Turn pin OUTPUT, LOW.
	*_ddrRPin |= _maskPin;
	*_portRPin &= ~_maskPin;
	// Wait for the electrode to be discharged.
	delayMicroseconds(_transfertDelay);

//	value -= share();
	// Turn pin INPUT, three-stated
	*_ddrRPin &= ~_maskPin;
	*_portRPin &= ~_maskPin;
	// Set the ADC channel to that pin.
	setMux(_channel);
	// Launch a conversion, and wait for it to be done.
	ADCSRA |= _BV(ADSC);

	while(ADCSRA & _BV(ADSC));

//	value -= ADCH;
	uint16_t newValue = ADCL;
	newValue += ((uint16_t)ADCH << 8);
//	Serial.println(newValue);
//	Serial.println();

	value -= newValue;
//	Serial.println(value);


	*_ddrRPin |= _maskPin;
	*_portRPin &= ~_maskPin;
	*_ddrRFriendPin |= _maskFriendPin;
	*_portRFriendPin &= ~_maskFriendPin;

	return value;
}

// Private methods
/*
// Share connect the electrode to its ADC channel.
// This is about the same as an analogRead():
// Set the pin to input (three-stated), then connect ADC mux,
// and start a conversion.
uint8_t CapADCChannel::share(){
	uint8_t value = 0;
	// Set the pin to input, three-stated.
	*_ddrRPin &= ~_maskPin;
	*_portRPin &= ~_maskPin;
	// Set the ADC channel to that pin.
	setMux(_channel);
	// Launch a conversion, and wait for it to be done.
	ADCSRA |= _BV(ADSC);
//	uint32_t length = micros();

	while(ADCSRA & _BV(ADSC));

//	Serial.println(micros() - length);

//	value = ADCL;
//	value |= (ADCH << 8);
	value = ADCH;

	return value;
}
*/
// Set the ADC to a channel.
// That can be ground for discharging, electrode pin for reading, or friend pin for charging.
void CapADCChannel::setMux(uint8_t channel){
	// Set channel number to 0
	ADMUX &= ~(0x1F);

	// Set channel to right channel number
	ADMUX |= (channel & 0x07);
#if !defined(__AVR_ATTiny24__) && !defined(__AVR_ATTiny44__) && !defined(__AVR_ATTiny84__) &&\
	!defined(__AVR_ATTiny24A__) && !defined(__AVR_ATTiny44A__) && !defined(__AVR_ATTiny84A__)
#if defined(MUX5)
	ADCSRB |= (ADCSRB & ~_BV(MUX5)) | (((channel >> 3) & 0x01) << MUX5);
#endif // MUX5
#endif // ATtiny
}
