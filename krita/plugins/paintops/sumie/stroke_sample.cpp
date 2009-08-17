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
#include "stroke_sample.h"

StrokeSample::StrokeSample(float x, float y, float pressure, float tiltX, float tiltY, float rotation)
{
    m_x = x;
    m_y = y;
    m_pressure = pressure;
    m_tiltX = tiltX;
    m_tiltY = tiltY;
    m_rotation = rotation;
}


StrokeSample::StrokeSample()
{

}

StrokeSample::~StrokeSample()
{

}


float StrokeSample::x()
{
    return m_x;
}

float StrokeSample::y()
{
    return m_y;
}

float StrokeSample::pressure()
{
    return m_pressure;
}

float StrokeSample::tiltX()
{
    return m_tiltX;
}

float StrokeSample::tiltY()
{
    return m_tiltY;
}

float StrokeSample::rotation()
{
    return m_rotation;
}
