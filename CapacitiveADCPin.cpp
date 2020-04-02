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

#include "CapacitiveADCPin.h"

// Public methods

// Constructor
CapADCPin::CapADCPin():_baseline(200){
	_adcChannel = new CapADCChannel();
	_now = _prev = _state = _previousState = Idle;
	_lSettings.resetCounter = 10;
	_lastTime = millis();
}

// Destructor
CapADCPin::~CapADCPin(){
	delete _adcChannel;
}

// Init the object. Tie it to used pins.
void CapADCPin::init(uint8_t pin, uint8_t friendPin){
	_adcChannel->init(pin, friendPin);
}

// change the charge delay for this channel
void CapADCPin::setChargeDelay(uint8_t value){
	_adcChannel->setChargeDelay(value);
}


// Tune baseline.
// Take an amount of readings and average them to get a new baseline value.
void CapADCPin::tuneBaseline(uint32_t length){
	uint32_t value = 0;
	uint16_t count = 0;
	length += millis();
	while(length > millis()){
		value += (uint32_t)updateRead();
		++count;
//		Serial.print("tuning...\t");
//		Serial.println(i);
//		Serial.println();
	}

	value /= count;
	_baseline = value;
	_read = _lastRead = _baseline;
}

// Tune threshold.
// Tune baseline, then read value from electrode for a given time, compute the max delta
// and set threshold values for touch and prox.
void CapADCPin::tuneThreshold(uint32_t length){
	tuneBaseline();
	length += millis();
	uint16_t _minBaseline = _baseline;
	uint16_t _maxBaseline = _baseline;
	while(length > millis()){
		uint16_t current = updateRead();
		if(_minBaseline > current) _minBaseline = current;
		if(_maxBaseline < current) _maxBaseline = current;

	}

	_maxDelta = _maxBaseline - _minBaseline;
	uint16_t touch = (float)(_maxDelta * 0.4);
	uint16_t release = (float)(touch * 0.6);

	setTouchThreshold(touch);
	setReleaseThreshold(release);
}

// launch a new read sequence.
int16_t CapADCPin::update(){
	// Update reading, save previous one.
	_lastRead = _read;
//	uint32_t length = micros();
//	Serial.println("u1");
//	Serial.print('\t');
	_read = updateRead();
//	Serial.print(_read);
//	Serial.print('\t');
//	length = micros();
	// Compute the exponential filter of reads.
	// Less memory than a running average, and a bit faster to detect changes.
//	float filter =  (float)_read * ((float)_gSettings.expWeight / 100) +
//					(float)_lastRead * ((100 - (float)_gSettings.expWeight) / 100);
	// Fix point math is faster than float numbers.
	uint32_t filter = (uint32_t)_read * _gSettings.expWeight + 
						(uint32_t)_lastRead * (255 - _gSettings.expWeight);
	filter /= 0xff;
	_read = filter;

//	Serial.println(_read);

	// Compute the delta between read and baseline
	_delta = (int16_t)_read - (int16_t)_baseline;

	// Save previous state.
	_prev = _now;

	// Prepare to touch
	if(_delta > _lSettings.touchThreshold){
		_now = Touch;
//		Serial.println("touch");
	// Or it's rising
	} else if(_delta > 0){
		_now = Rising;
//		Serial.println("rising");
	// Or it's falling
	} else if(_delta < 0){
		_now = Falling;
//		Serial.println("falling");
	// Or nothing.
	} else {
		_now = Idle;
//		Serial.println("idle");
	}

	if(_now != _prev) _lastTime = millis();

	if(_now == Rising || _now == Falling) updateCal();

	// Debounce the current instant state to see if we can use it to detect touch
	if((_now == _prev) && ((millis() - _lastTime) > _gSettings.debounce)){
		_previousState = _state;
		_state = _now;
	}

	return _delta;
}

// Getter for touch state
bool CapADCPin::isTouched() const{
	if(_state == Touch) return true;
	return false;
}

// Getter for touch state
bool CapADCPin::isJustTouched() const{
	if((_state == Touch) && (_previousState != Touch)) return true;
	return false;
}

// Getter for touch state
bool CapADCPin::isJustReleased() const{
	if((_state != Touch) && (_previousState == Touch)) return true;
	return false;
}

/*
void CapADCPin::applyLocalSettings(const CapADCSetLocal_t& settings){
	_lSettings = settings;
}

CapADCSetLocal_t CapADCPin::getLocalSettings() const{
	return _lSettings;
}
*/

// Protected methods

// Update the baseline value
void CapADCPin::updateCal(){
	uint16_t timeDelta = millis() - _lastTime;
	if(_now == Rising){
		if(timeDelta >= _gSettings.noiseCountRising){
			_baseline += _gSettings.noiseIncrement;
			_lastTime = millis();
		}
	} else if(_now == Falling){
		if(timeDelta >= _gSettings.noiseCountFalling){
			_baseline -= _gSettings.noiseIncrement;
			_lastTime = millis();
		}
	}

}

// Get a serie of readings.
uint16_t CapADCPin::updateRead(){
	int32_t value = 0;
	uint16_t samples = 1 << _gSettings.samples;
	uint16_t divider = 1 << _gSettings.divider;

	// One discarded read to account for errors on first read an a new ADC
	_adcChannel->read();

	for(uint16_t i = 0; i < samples; ++i){
		value += _adcChannel->read();
	}

	value /= divider;


	return (uint16_t)value;
}
