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

#include "CapacitiveADCSlider.h"

// Public methods

// Constructor
CapacitiveADCSlider::CapacitiveADCSlider(){
	for(uint8_t i = 0; i <= MAX_SLIDER_CHANNEL; ++i){
		_nowState[i] = _prevState[i] = _state[i] = _previousState[i] = Idle;
		_lastTime[i] = millis();
		_numChannels = 0;
	}

	setResetDelay(60);
	_position = _prevPosition = _nowPosition = _step = 0;
	_coeff = 32;
}

// Destructor
CapacitiveADCSlider::~CapacitiveADCSlider(){
	for(uint8_t i = 0; i < _numChannels; ++i){
		delete _adcChannel[i];
	}
}

// Init the object. Tie it to used pins.

void CapacitiveADCSlider::init(uint8_t pin0, uint8_t pin1){
	if(_numChannels != 0) return;
	_adcChannel[0] = new CapacitiveADCChannel();
	_adcChannel[0]->init(pin0, pin1);
	_adcChannel[1] = new CapacitiveADCChannel();
	_adcChannel[1]->init(pin1, pin0);

	_numChannels = 2;

	_weighting[0] = 127;
	_weighting[1] = -127;	
}

void CapacitiveADCSlider::init(uint8_t pin0, uint8_t pin1, uint8_t pin2){
	if(_numChannels != 0) return;

	_adcChannel[0] = new CapacitiveADCChannel();
	_adcChannel[0]->init(pin0, pin1);
	_adcChannel[1] = new CapacitiveADCChannel();
	_adcChannel[1]->init(pin1, pin2);
	_adcChannel[2] = new CapacitiveADCChannel();
	_adcChannel[2]->init(pin2, pin0);

	_numChannels = 3;

	_weighting[0] = 127;
	_weighting[1] = 0;	
	_weighting[2] = -127;	
}

void CapacitiveADCSlider::init(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3){
	if(_numChannels != 0) return;

	_adcChannel[0] = new CapacitiveADCChannel();
	_adcChannel[0]->init(pin0, pin1);
	_adcChannel[1] = new CapacitiveADCChannel();
	_adcChannel[1]->init(pin1, pin2);
	_adcChannel[2] = new CapacitiveADCChannel();
	_adcChannel[2]->init(pin2, pin3);
	_adcChannel[3] = new CapacitiveADCChannel();
	_adcChannel[3]->init(pin3, pin0);

	_numChannels = 4;

	_weighting[0] = 127;
	_weighting[1] = 42;	
	_weighting[2] = -42;	
	_weighting[3] = -127;	

}

/*
// If ever you need more channels for one long slider, you can uncomment this init function.
// You will have to uncomment also the matching one in CapacitiveADCSlider.h,
// and also change the #define MAX_SLIDER_CHANNEL accordingly.
// If you ever need even more than 6 channels (wich is unlikely to happen if you use a Uno),
// just copy the body of the function, and add lines for other channels. ;)

void CapacitiveADCSlider::init(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4){
	if(_numChannels != 0) return;

	_adcChannel[0] = new CapacitiveADCChannel();
	_adcChannel[0]->init(pin0, pin1);
	_adcChannel[1] = new CapacitiveADCChannel();
	_adcChannel[1]->init(pin1, pin2);
	_adcChannel[2] = new CapacitiveADCChannel();
	_adcChannel[2]->init(pin2, pin3);
	_adcChannel[3] = new CapacitiveADCChannel();
	_adcChannel[3]->init(pin3, pin4);
	_adcChannel[4] = new CapacitiveADCChannel();
	_adcChannel[4]->init(pin4, pin0);

	_numChannels = 5;

	_weighting[0] = 127;
	_weighting[1] = 32;	
	_weighting[2] = 0;	
	_weighting[3] = -32;	
	_weighting[4] = -127;	

}

void CapacitiveADCSlider::init(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5){
	if(_numChannels != 0) return;

	_adcChannel[0] = new CapacitiveADCChannel();
	_adcChannel[0]->init(pin0, pin1);
	_adcChannel[1] = new CapacitiveADCChannel();
	_adcChannel[1]->init(pin1, pin2);
	_adcChannel[2] = new CapacitiveADCChannel();
	_adcChannel[2]->init(pin2, pin3);
	_adcChannel[3] = new CapacitiveADCChannel();
	_adcChannel[3]->init(pin3, pin4);
	_adcChannel[4] = new CapacitiveADCChannel();
	_adcChannel[4]->init(pin4, pin5);
	_adcChannel[5] = new CapacitiveADCChannel();
	_adcChannel[5]->init(pin5, pin0);

	_numChannels = 6;

	_weighting[0] = 127;
	_weighting[1] = 77;	
	_weighting[2] = 27;	
	_weighting[3] = -27;
	_weighting[4] = -77;	
	_weighting[5] = -127;	

}
*/

// Tune baseline.
// Take an amount of readings and average them to get a new baseline value.
void CapacitiveADCSlider::tuneBaseline(uint32_t length){
	_baseline[_numChannels] = 0;

	for(uint8_t i = 0; i < _numChannels; ++i){
		uint32_t value = 0;
		uint16_t count = 0;
		uint32_t len = millis() + length;
		while(len > millis()){
			value += (uint32_t)updateRead(i);
			++count;
		}

		value /= count;
		_baseline[i] = value;
		_baseline[_numChannels] += value;
//		_minBaseline = _maxBaseline = _baseline;
		_currentRead[i] = _previousRead[i] = _baseline[i];
	}

	_baseline[_numChannels] /= _numChannels;
	_currentRead[_numChannels] = _previousRead[_numChannels] = _baseline[_numChannels];


}

// Tune threshold.
// Tune baseline, then read value from electrode for a given time, compute the max delta
// and set threshold values for touch.
void CapacitiveADCSlider::tuneThreshold(uint32_t length){
	tuneBaseline();
	length += millis();
	uint16_t minBaseline[_numChannels];
	uint16_t maxBaseline[_numChannels];
	while(length > millis()){
		for(uint8_t i = 0; i < _numChannels; ++i){
			uint16_t current = updateRead(0);
			if(minBaseline[i] > current) minBaseline[i] = current;
			if(maxBaseline[i] < current) maxBaseline[i] = current;
		}

	}

	uint16_t delta = 0;
	for(uint8_t i = 0; i < _numChannels; ++i){
		delta += maxBaseline[i] - minBaseline[i];
	}

	delta /= _numChannels;

	uint16_t touch = (float)(delta * 0.4);
	uint16_t release = (float)(touch * 0.6);

	setTouchThreshold(touch);
	setTouchReleaseThreshold(release);
}

// launch a new read sequence.
int16_t CapacitiveADCSlider::update(void){

	// We first update the last virtual channel, which is the average of all others
	_previousRead[_numChannels] = _currentRead[_numChannels];
	// And set the current read to 0, so we can add to it on each reading.
	_currentRead[_numChannels] = 0;

	for(uint8_t i = 0; i <= _numChannels; ++i){
		if(i < _numChannels){
			// For each "real" channel, we update the reading
			_previousRead[i] = _currentRead[i];
			_currentRead[i] = updateRead(i);
			// And ad it to the virtual global channel
			_currentRead[_numChannels] += _currentRead[i];
		} else {
			// If we are processing the global channel, we finish compute average.
			_currentRead[i] /= _numChannels;
		}

		// We compute the exponential filter for this channel
		// This take the last value of a channel, and ponderate it with the new one.
		// It has about the same effect than a running average, but uses much less memory!
		uint32_t filter = (uint32_t)_currentRead[i] * _gSettings.expWeight + 
						(uint32_t)_previousRead[i] * (255 - _gSettings.expWeight);
		filter /= 0xff;
		// Store the new filter value in place of the reading.
		_currentRead[i] = filter;

		// We compute the delta between current read and baseline
		_delta[i] = _currentRead[i] - _baseline[i];
		// Update current state
		_prevState[i] = _nowState[i];

		// And act accordingly.
		// Update baseline
		if(_delta[i] > _lSettings.touchThreshold){
			_nowState[i] = Touch;
		// Or it's rising
		} else if(_delta[i] > 0){
			_nowState[i] = Rising;
		// Or it's falling
		} else if(_delta[i] < 0){
			_nowState[i] = Falling;
		// Or nothing.
		} else {
			_nowState[i] = Idle;
		}

		// If the state has changed, we reset counter (for baseline updating),
		// and keep track of current time, for debouncing.
		if(_nowState[i] != _prevState[i]){
			_lastTime[i] = millis();
		}

		// If we are on a real channel, we may have to update baseline.
		if((_nowState[i] == Rising || _nowState[i] == Falling) && i != _numChannels){
			updateCal(i);
		}

		// Debounce the current instant state to see if we can use it to detect touch
		if((_nowState[i] == _prevState[i]) && ((millis() - _lastTime[i]) > _gSettings.debounce)){
			_previousState[i] = _state[i];
			_state[i] = _nowState[i];
		}
	}

	return updatePosition();
}

// Getter for touch state
bool CapacitiveADCSlider::isTouched(void) const{
	if(_state[_numChannels] == Touch) return true;
	return false;
}

// Getter for current value
int8_t CapacitiveADCSlider::getPosition(void) const{
	return _position;
}

// Getter for current step value.
// We reset the _step value, so we can keep track of missed movement.
int8_t CapacitiveADCSlider::getStep(void){
	int8_t step = _step;
	_step = 0;
	return step;
}


uint16_t CapacitiveADCSlider::getBaseline(void) const{
	return _baseline[_numChannels];
}

// Protected methods

// Compute current position
bool CapacitiveADCSlider::updatePosition(void){

	bool touch = false;
	// If we have a touch, it's time to see where on the slider we are!
	if(_state[_numChannels] == Touch){
		// Keep a track for the last position
		_prevPosition = _nowPosition;

		int32_t bary = 0;
		// Compute, for each channel, its ponderate delta / average, then weight it.
		for(uint8_t i = 0; i < _numChannels; ++i){

			int32_t bary0 = _coeff * (int32_t)_delta[i];
			bary0 /= _delta[_numChannels];
			bary += _weighting[i] * bary0;
		}

		_nowPosition = bary / 64;

		// If we have two or more consecutive Touch states, we can update position and step
		// (we don't update on the first read state to avoid absurd step values)
		if(_previousState[_numChannels == Touch]){
			touch = true;
			_position = _nowPosition;
			_step += _position - _prevPosition;		
		}
	}

	return touch;
}

// Update the baseline value
void CapacitiveADCSlider::updateCal(uint8_t index){
	// We check the time delta since last update
	uint16_t timeDelta = millis() - _lastTime[index];
	// Then if above noise count threshold, we update baseline, rising or falling.
	if(_nowState[index] == Rising){
		if(timeDelta >= _gSettings.noiseCountRising){
			_baseline[index] += _gSettings.noiseIncrement;
			_lastTime[index] = millis();
		}
	} else if(_nowState[index] == Falling){
		if(timeDelta >= _gSettings.noiseCountFalling){
			_baseline[index] -= _gSettings.noiseIncrement;
			_lastTime[index] = millis();
		}
	}

}

// Get a serie of readings from the bare channel
uint16_t CapacitiveADCSlider::updateRead(uint8_t index){
	int32_t value = 0;
	// samples sets (2^samples) the number of consecutive reads to be made
	// More is better filtering, but means a longer time.
	// We sum all of them, then
	// divider sets (2^divider) the number we divide the above cumulative read with.
	uint16_t samples = 1 << _gSettings.samples;
	uint16_t divider = 1 << _gSettings.divider;

	// Sum up the consecutive reads
	for(uint16_t i = 0; i < samples; ++i){
		value += _adcChannel[index]->read();
	}
	//Then divide.
	value /= divider;


	return (uint16_t)value;
}
