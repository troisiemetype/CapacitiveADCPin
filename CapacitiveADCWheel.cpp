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

#include "CapacitiveADCWheel.h"

// Public methods

// Constructor
CapacitiveADCWheel::CapacitiveADCWheel(){
	for(uint8_t i = 0; i <= MAX_SLIDER_CHANNEL; ++i){
		_nowState[i] = _prevState[i] = _state[i] = _previousState[i] = Idle;
		_lastTime[i] = millis();
		_numChannels = 0;
	}

	_lSettings.resetCounter = 60;
	_position = _prevPosition = _nowPosition = _step = 0;
	_coeff = 83;
}

// Destructor
CapacitiveADCWheel::~CapacitiveADCWheel(){
	for(uint8_t i = 0; i < _numChannels; ++i){
		delete _adcChannel[i];
	}
}

// Init the object. Tie it to used pins.
// Init with two pins has no meaning for a wheel, but as wheel inherits from slider, we have to keep it.
// Don't use it, it returns without doing anything.
void CapacitiveADCWheel::init(uint8_t pin0, uint8_t pin1){
	return;
}

void CapacitiveADCWheel::init(uint8_t pin0, uint8_t pin1, uint8_t pin2){
	if(_numChannels != 0) return;

	_adcChannel[0] = new CapacitiveADCChannel();
	_adcChannel[0]->init(pin0, pin2);
	_adcChannel[1] = new CapacitiveADCChannel();
	_adcChannel[1]->init(pin1, pin2);
	_adcChannel[2] = new CapacitiveADCChannel();
	_adcChannel[2]->init(pin2, pin1);

	_numChannels = 3;

	_weighting[0] = 0;
	_weighting[1] = 86;
	_weighting[2] = 171;	
}
/*
void CapacitiveADCWheel::init(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3){
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
*/
/*
// If ever you need more channels for one long slider, you can uncomment this init function.
// You will have to uncomment also the matching one in CapacitiveADCWheel.h,
// and also change the #define MAX_SLIDER_CHANNEL accordingly.
// If you ever need even more than 6 channels (wich is unlikely to happen if you use a Uno),
// just copy the body of the function, and add lines for other channels. ;)

void CapacitiveADCWheel::init(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4){
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

void CapacitiveADCWheel::init(uint8_t pin0, uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t pin4, uint8_t pin5){
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

// Protected methods

// Compute current position
bool CapacitiveADCWheel::updatePosition(void){

	bool touch = false;
	// If we have a touch, it's time to see where on the slider we are!
	if(_state[_numChannels] == Touch){
		// Keep a track for the last position
		_prevPosition = _nowPosition;

		int32_t bary0 = 0;
		int32_t bary1 = 0;
		int32_t bary2 = 0;
		int32_t bary = 0;

		bary0 = _coeff * (int32_t)_delta[0];
		bary1 = _coeff * (int32_t)_delta[1];
		bary2 = _coeff * (int32_t)_delta[2];

		// Ponderate / total delta
		bary0 /= _delta[_numChannels];
		bary1 /= _delta[_numChannels];
		bary2 /= _delta[_numChannels];

		// We now want to know which of the three is has the strongest touch,
		// and we adjust calculus regarding its position.
		if(bary0 > bary1 && bary0 > bary2){
			// 12 hours
			bary = _weighting[0] + (bary1 - bary2) / 3;
		} else if( bary1 >= bary0 && bary1 >= bary2){
			// 4 hours
			bary = _weighting[1] + (bary2 - bary0) / 3;
		} else {
			// 8 hours
			bary = _weighting[2] + (bary0 - bary1) / 3;
		}

		_nowPosition = bary % 256;

		// If we have two or more consecutive Touch states, we can update position and step
		// (we don't update on the first touch state to avoid absurd step values:
		// If it had been touched on one side of the slider, and the new touch is on the other,
		// step would take a great value, altough it's only mean to give a variance
		// from one read to another.
		if(_previousState[_numChannels == Touch]){
			touch = true;
			_position = _nowPosition;
			_step += _position - _prevPosition;		
		}
	}

	return touch;
}
