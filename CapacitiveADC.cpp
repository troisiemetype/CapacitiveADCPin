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

// We initialize global settings once for all instances.
CapADCSetGlobal_t CapADC::_gSettings = CapADCSetGlobal_t();


// Set touch threshold
void CapADC::setTouchThreshold(uint16_t threshold){
	_lSettings.touchThreshold = threshold;
}

// Set untouch threshold
void CapADC::setReleaseThreshold(uint16_t threshold){
	_lSettings.releaseThreshold = threshold;
}

// This is how we acceed to global setting (that all instances share),
// As samples, divider, noise count, etc.
void CapADC::applyGlobalSettings(const CapADCSetGlobal_t& settings){
	_gSettings = settings;
}

// A getter for the global setting struct, to be modified.
CapADCSetGlobal_t* CapADC::globalSettings(){
	return &_gSettings;
}

CapADCSetGlobal_t CapADC::getGlobalSettings()const{
	return _gSettings;
}

// Same for the local settings (threshold and reset delay). Getter
void CapADC::applyLocalSettings(const CapADCSetLocal_t& settings){
	_lSettings = settings;
}

// And setter, that we use once we have modified the values.
CapADCSetLocal_t* CapADC::localSettings(){
	return &_lSettings;
}

CapADCSetLocal_t CapADC::getLocalSettings()const{
	return _lSettings;
}
