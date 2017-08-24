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

} without even the implied warranty o_pinR |= _mask;
}f
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of_pinR |= _mask;
} the GNU General Public License
 * along with this program.  If not, _pinR |= _mask;
}see <http://www.gnu.org/licenses/>.
 */

#include "CapacitiveADC.h"

// Public methods

// Constructor
CapacitiveADC::CapacitiveADC()://_samples(1),
								_touchThreshold(50),
								_touchReleaseThreshold(40),
								_proxThreshold(5),
								_proxReleaseThreshold(3),
								//_debounce(4),
								_baseline(100)
								//_maxDelta(2),
								//_noiseIncrement(1),
								//_noiseCountRising(3),
								//_noiseCountFalling(1)
								{
	_adcPin = new CapacitiveADCPin();
	_gSettings = new SettingsGlobal_t();
	_state = _previousState = Idle;
	_counter = 0;
//	_debounce = 1;
}

// Destructor
CapacitiveADC::~CapacitiveADC(){
	delete _adcPin;
	delete _gSettings;
}

// Init the object. Tie it to used pins.
void CapacitiveADC::init(uint8_t pin, uint8_t friendPin){
	_adcPin->init(pin, friendPin);
}

// launch a new read sequence.
uint16_t CapacitiveADC::update(){
	// Update reading
	_read = updateRead();

	// Compute the delta between read and baseline
	int16_t delta = _read - _baseline;
	uint16_t absDelta = abs(delta);

	// Save previous state.
	_previousState = _state;

	// Update baseline
	if(absDelta <= _gSettings->maxDelta){
		_baseline += delta;
		_state = Idle;
	// Or prepare to touch
	} else if(delta > _touchThreshold){
		_state = PreTouch;
	// Or prepare to prox
	} else if(delta > _proxThreshold){
		_state = PreProx;
	// Or it's rising
	} else if(delta > 0){
		_state = Rising;
	// Or it's falling
	} else if(delta < 0){
		_state = Falling;
	// Or nothing.
	} else {
		_state = Idle;
	}

	// Test if there is a proximty or touch detection.
	if(_state == PreTouch && _counter >= _gSettings->debounce){
		_state = Touch;
	} else if(_state == PreProx && _counter >= _gSettings->debounce){
		_state = Prox;
	}
	if(_state != _previousState){
		_counter = 0;
	} else {
		++_counter;
	}

	// Manage touch change
	// TODO: create a temporary state for managing transitions
	if( _state == Touch){
		if(delta < _touchReleaseThreshold){
			if(delta > _proxReleaseThreshold){
				_state = Prox;
			} else {
				_state = Idle;
			}
		}
	} else if(_state == Prox){
		if(delta > _touchThreshold){
			_state = Touch;
		} else if(delta < _proxReleaseThreshold){
			_state = Idle;
		}
	} else if(_state == Rising || _state == Falling){
		updateCal();
	}

	Serial.print("baseline: ");
	Serial.println(_baseline);
	Serial.print("delta: ");
	Serial.println(delta);
	Serial.print("state: ");
	Serial.println(_state);
	Serial.print("counter: ");
	Serial.println(_counter);

	return _read;
}

// Set the charge delay.
// This is the minimal delay for the charge to transfer from electrode to s&h capacitor,
// back and forth.
void CapacitiveADC::setChargeDelay(uint8_t value){
	_adcPin->setChargeDelay(value);
}

// Getter for touch state
bool CapacitiveADC::isTouched(){
	return false;
}

// Getter for touch state
bool CapacitiveADC::isJustTouched(){
	return false;
}

// Getter for touch state
bool CapacitiveADC::isJustTouchedReleased(){
	return false;
}

// Getter for prox state
bool CapacitiveADC::isProx(){
	return false;
}

// Getter for prox state
bool CapacitiveADC::isJustProx(){
	return false;
}

// Getter for prox state
bool CapacitiveADC::isJustProxReleased(){
	return false;
}

// Set the number of samples to sense for one reading.
void CapacitiveADC::setSamples(uint8_t value){
	_gSettings->samples = value;
}

// Set touch threshold
void CapacitiveADC::setTouchThreshold(uint8_t threshold){
	_touchThreshold = threshold;
}

// Set untouch threshold
void CapacitiveADC::setTouchReleaseThreshold(uint8_t threshold){
	_touchReleaseThreshold = threshold;
}

// Set prox threshold
void CapacitiveADC::setProxThreshold(uint8_t threshold){
	_proxThreshold = threshold;
}

// Set unprox threshold
void CapacitiveADC::setProxReleaseThreshold(uint8_t threshold){
	_proxReleaseThreshold = threshold;
}

void CapacitiveADC::setDebounce(uint8_t value){
	_gSettings->debounce = value;
}

void CapacitiveADC::setMaxDelta(uint8_t value){
	_gSettings->maxDelta = value;
}

void CapacitiveADC::setNoiseIncrement(uint8_t value){
	_gSettings->noiseIncrement = value;
}

void CapacitiveADC::setNoiseCountRising(uint8_t value){
	_gSettings->noiseCountRising = value;
}

void CapacitiveADC::setNoiseCountFalling(uint8_t value){
	_gSettings->noiseCountFalling = value;
}

void CapacitiveADC::setSettings(const SettingsGlobal_t& settings){
	*_gSettings = settings;
}

SettingsGlobal_t CapacitiveADC::getSettings(){
	return *_gSettings;
}


// Protected methods

// Get a serie of readings.
uint16_t CapacitiveADC::updateRead(){
	int32_t value = 0;
	for(uint8_t i = 0; i < _gSettings->samples; i++){
		value += _adcPin->read();
	}
	value /= _gSettings->samples;
	return (uint16_t)value;
}

void CapacitiveADC::updateCal(){
	if(_state == Rising){
		if(_counter >= _gSettings->noiseCountRising){
			_baseline += _gSettings->noiseIncrement;
			_counter = 0;
		}
	} else if(_state == Falling){
		if(_counter >= _gSettings->noiseCountFalling){
			_baseline -= _gSettings->noiseIncrement;
			_counter = 0;
		}
	}

}
