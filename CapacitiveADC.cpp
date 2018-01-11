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

#include "CapacitiveADC.h"

// Public methods

SettingsGlobal_t CapacitiveADC::_gSettings = SettingsGlobal_t();

// Constructor
CapacitiveADC::CapacitiveADC():_baseline(200){
	_adcPin = new CapacitiveADCPin();
	_now = _prev = _state = _previousState = Idle;
	_counter = 0;
	setResetDelay(10);
	_lastTime = millis();
}

// Destructor
CapacitiveADC::~CapacitiveADC(){
	delete _adcPin;
}

// Init the object. Tie it to used pins.
void CapacitiveADC::init(uint8_t pin, uint8_t friendPin){
	_adcPin->init(pin, friendPin);
}

// Tune baseline.
// Take an amount of readings and average them to get a new baseline value.
void CapacitiveADC::tuneBaseline(uint32_t length){
	uint32_t value = 0;
	uint16_t count = 0;
	length += millis();
	while(length > millis()){
		value += updateRead();
		++count;
//		Serial.print("tuning...\t");
//		Serial.println(i);
//		Serial.println();
	}

	value /= count;
	_baseline = value;
	_minBaseline = _maxBaseline = _baseline;
	_read = _lastRead = _baseline;
//	Serial.print("baseline:\t");
//	Serial.println(value);
}

// Tune threshold.
// Tune baseline, then read value from electrode for a given time, compute the max delta
// and set threshold values for touch and prox.
void CapacitiveADC::tuneThreshold(uint32_t length){
	tuneBaseline();
	length += millis();
	while(length > millis()){
		uint16_t current = updateRead();
		if(_minBaseline > current) _minBaseline = current;
		if(_maxBaseline < current) _maxBaseline = current;
/*
		Serial.print(_baseline);
		Serial.print('\t');
		Serial.print(_minBaseline);
		Serial.print('\t');
		Serial.print(current);
		Serial.print('\t');
		Serial.print(_maxBaseline);
		Serial.println();
*/
	}

	uint16_t delta = _maxBaseline - _minBaseline;
	uint16_t touch = (float)(delta * 0.4);
	uint16_t release = (float)(touch * 0.6);
	uint16_t prox = (float)(delta * 0.04);
	uint16_t proxRelease = (float)(prox * 0.6);

	setTouchThreshold(touch);
	setTouchReleaseThreshold(release);
	setProxThreshold(prox);
	setProxReleaseThreshold(proxRelease);
/*
	Serial.println();

	Serial.print("baseline:");
	Serial.print('\t');
	Serial.println(_baseline);

	Serial.print("delta:");
	Serial.print('\t');
	Serial.println(delta);

	Serial.print("touch:");
	Serial.print('\t');
	Serial.print('\t');
	Serial.println(touch);

	Serial.print("release:");
	Serial.print('\t');
	Serial.println(release);

	Serial.print("prox:");
	Serial.print('\t');
	Serial.print('\t');
	Serial.println(prox);

	Serial.print("release:");
	Serial.print('\t');
	Serial.println(proxRelease);	

	Serial.println("tuning done.");
*/
}

// launch a new read sequence.
int16_t CapacitiveADC::update(){
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
	// Fix point math is often faster than float numbers.
	uint32_t filter = (uint32_t)_read * _gSettings.expWeight + 
						(uint32_t)_lastRead * (255 - _gSettings.expWeight);
	filter /= 0xff;
	_read = filter;

//	Serial.println(_read);

	// Compute the delta between read and baseline
	_delta = _read - _baseline;
	int16_t absDelta = abs(_delta);

	// Save previous state.
	_prev = _now;

	// Update baseline
	if(absDelta <= _gSettings.noiseDelta){
		_baseline += _delta;
		_now = BaselineChanged;
	// Or prepare to touch
	} else if(_delta > _lSettings.touchThreshold){
		_now = Touch;
//		Serial.println("touch");
	// Or prepare to prox
	} else if(_delta > _lSettings.proxThreshold){
		_now = Prox;
//		Serial.println("prox");
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

	if(_now != _prev){
		_counter = 0;
		_lastTime = millis();
	} else {
		++_counter;
	}

	// Manage touch change
	if( _now == Touch){
		if(millis() - _lastTime > _resetCounter) tuneBaseline();
		if(_delta < _lSettings.touchReleaseThreshold){
			if(_delta > _lSettings.proxReleaseThreshold){
				_now = Prox;
			} else {
				_now = Idle;
			}
		}
	} else if(_now == Prox){
		if(millis() - _lastTime > _resetCounter) tuneBaseline();
		if(_delta > _lSettings.touchThreshold){
			_now = Touch;
		} else if(_delta < _lSettings.proxReleaseThreshold){
			_now = Idle;
		}
	} else if(_now == Rising || _now == Falling){
		updateCal();
	}

	// Debounce the current instant state to see if we can use it to detect touch
	if((_now == _prev) && ((millis() - _lastTime) > _gSettings.debounce)){
		_previousState = _state;
		_state = _now;
	}

//	Serial.print("baseline: ");
//	Serial.print(_baseline);
//	Serial.print('\t');
//	Serial.print("raw: ");
//	Serial.print(_read);
//	Serial.print('\t');
//	Serial.println(_state * 64);
//	Serial.print("delta: ");
//	Serial.println(_delta);

//	Serial.print("state: ");
//	Serial.println(_now);
/*
	Serial.print("counter: ");
	Serial.println(_counter);
*/	
//	Serial.println();

//	return _read;
//	length = micros() - length;
//	Serial.print("u3");
//	Serial.print('\t');
//	Serial.println(length);
	return _delta;
}

// Set the charge delay.
// This is the minimal delay for the charge to transfer from electrode to s&h capacitor,
// back and forth.
void CapacitiveADC::setChargeDelay(uint8_t value){
	_adcPin->setChargeDelay(value);
}

// Getter for touch state
bool CapacitiveADC::isTouched() const{
	if(_state == Touch) return true;
	return false;
}

// Getter for touch state
bool CapacitiveADC::isJustTouched() const{
	if((_state == Touch) && (_previousState != Touch)) return true;
	return false;
}

// Getter for touch state
bool CapacitiveADC::isJustTouchedReleased() const{
	if((_state != Touch) && (_previousState == Touch)) return true;
	return false;
}

// Getter for prox state
bool CapacitiveADC::isProx() const{
	if(_state == Prox) return true;
	return false;
}

// Getter for prox state
bool CapacitiveADC::isJustProx() const{
	if((_state == Prox) && (_previousState != Prox)) return true;
	return false;
}

// Getter for prox state
bool CapacitiveADC::isJustProxReleased() const{
	if((_state != Prox) && (_previousState == Prox)) return true;
	return false;
}

// Getter for global release
bool CapacitiveADC::isJustReleased() const{
	if((_state != Touch) && (_state != Prox) && ((_previousState == Prox) || (_previousState == Touch))) return true;
	return false;
}


uint8_t CapacitiveADC::proxRatio() const{
	if(_state == Prox){
		uint16_t deltaThre = _lSettings.touchThreshold - _lSettings.proxThreshold;
		float ratio = 256 / (float)deltaThre;
		uint16_t step = _delta - _lSettings.proxThreshold;
		return (uint8_t)(step * ratio);
	}
	return 0;	
}

// Set the number of samples to sense for one reading.
void CapacitiveADC::setSamples(uint8_t value){
	_gSettings.samples = value;
}

// Set the number of samples to sense for one reading.
void CapacitiveADC::setDivider(uint8_t value){
	_gSettings.divider = value;
}

// Set the delay (seconds) after which a touch or prox is reset to idle state
void CapacitiveADC::setResetDelay(uint8_t value){
	_resetCounter = (uint32_t)(value * 1000);
}

// Set touch threshold
void CapacitiveADC::setTouchThreshold(uint16_t threshold){
	_lSettings.touchThreshold = threshold;
}

// Set untouch threshold
void CapacitiveADC::setTouchReleaseThreshold(uint16_t threshold){
	_lSettings.touchReleaseThreshold = threshold;
}

// Set prox threshold
void CapacitiveADC::setProxThreshold(uint16_t threshold){
	_lSettings.proxThreshold = threshold;
}

// Set unprox threshold
void CapacitiveADC::setProxReleaseThreshold(uint16_t threshold){
	_lSettings.proxReleaseThreshold = threshold;
}

void CapacitiveADC::setDebounce(uint8_t value){
	_gSettings.debounce = value;
}

void CapacitiveADC::setNoiseDelta(uint8_t value){
	_gSettings.noiseDelta = value;
}

void CapacitiveADC::setNoiseIncrement(uint8_t value){
	_gSettings.noiseIncrement = value;
}

void CapacitiveADC::setNoiseCountRising(uint8_t value){
	_gSettings.noiseCountRising = value;
}

void CapacitiveADC::setNoiseCountFalling(uint8_t value){
	_gSettings.noiseCountFalling = value;
}

uint16_t CapacitiveADC::getBaseline(){
	return _baseline;
}

uint16_t CapacitiveADC::getMaxDelta(){
	return (_maxBaseline - _minBaseline);
}

void CapacitiveADC::applyGlobalSettings(const SettingsGlobal_t& settings){
	_gSettings = settings;
}

SettingsGlobal_t CapacitiveADC::getGlobalSettings() const{
	return _gSettings;
}

void CapacitiveADC::applyLocalSettings(const SettingsLocal_t& settings){
	_lSettings = settings;
}

SettingsLocal_t CapacitiveADC::getLocalSettings() const{
	return _lSettings;
}


// Protected methods

// Get a serie of readings.
uint16_t CapacitiveADC::updateRead(){
	int32_t value = 0;
//	int32_t value1 = 0;
//	int32_t value2 = 0;
	uint16_t samples = 1 << _gSettings.samples;
	uint16_t divider = 1 << _gSettings.divider;
//	uint32_t length = micros();

	for(uint16_t i = 0; i < samples; ++i){
//		length = micros();
		value += _adcPin->read();
//		Serial.println(micros() - length);
//		value = _adcPin->read();
//		value1 += value & 0xff;
//		value2 += (value & 0xff00) >> 8;
	}

//	Serial.println(micros() - length);
//	Serial.print(value1);
//	Serial.print('\t');
//	Serial.println(value2);
//	Serial.print("total:");
//	Serial.print('\t');
//	Serial.println(value);

	value /= divider;
//	Serial.print("read:");
//	Serial.print('\t');
//	Serial.println(value);

	return (uint16_t)value;
}

void CapacitiveADC::updateCal(){
	uint16_t timeDelta = millis() - _lastTime;
	if(_now == Rising){
		if(timeDelta >= _gSettings.noiseCountRising){
			_baseline += _gSettings.noiseIncrement;
			_counter = 0;
			_lastTime = millis();
		}
	} else if(_now == Falling){
		if(timeDelta >= _gSettings.noiseCountFalling){
			_baseline -= _gSettings.noiseIncrement;
			_counter = 0;
			_lastTime = millis();
		}
	}

}
