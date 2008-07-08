/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "bristle.h"

Bristle::Bristle(){
	m_x = 0;
	m_y = 0;
	m_length = 0;
}

Bristle::Bristle(float x, float y,float length){
	m_x = x;
	m_y = y;
	m_length = length;
}

Bristle::~Bristle(){

}

float Bristle::x(){
	return m_x;
}

float Bristle::y(){
	return m_y;
}

float Bristle::length(){
	return m_length;
}

void Bristle::setLength(float length){
	m_length = length;
}


void Bristle::addInk(float value){
	m_inkAmount = m_inkAmount + value;
}

void Bristle::removeInk(float value){
	m_inkAmount = m_inkAmount - value;
}

void Bristle::setInkAmount(float inkAmount){
	if (inkAmount > 1.0f)
	{
		inkAmount = 1.0f;
	}
	else if (inkAmount < -1.0f)
	{
		inkAmount = -1.0f;
	}

	m_inkAmount = inkAmount;
}

float Bristle::amount(){
	return m_inkAmount;
}

KoColor Bristle::color(){
	return m_inkColor;
}
void Bristle::setColor(KoColor inkColor){
	m_inkColor = inkColor;
}
