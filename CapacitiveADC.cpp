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

// Set the charge delay.
// This is the minimal delay for the charge to transfer from electrode to s&h capacitor,
// back and forth.
void CapacitiveADC::setChargeDelay(uint8_t value){
	_adcChannel->setChargeDelay(value);
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
	_lSettings.resetCounter = (uint32_t)(value * 1000);
}

// Set touch threshold
void CapacitiveADC::setTouchThreshold(uint16_t threshold){
	_lSettings.touchThreshold = threshold;
}

// Set untouch threshold
void CapacitiveADC::setTouchReleaseThreshold(uint16_t threshold){
	_lSettings.touchReleaseThreshold = threshold;
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

void CapacitiveADC::setNoiseCountRising(uint16_t value){
	_gSettings.noiseCountRising = value;
}

void CapacitiveADC::setNoiseCountFalling(uint16_t value){
	_gSettings.noiseCountFalling = value;
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
