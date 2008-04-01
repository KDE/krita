/*
 * Copyright (C)  Hans Bakker <hansmbakker@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Ko3DMouseEvent.h"

Ko3DMouseEvent::Ko3DMouseEvent()
{
}

Ko3DMouseEvent::~Ko3DMouseEvent()
{
}
void Ko3DMouseEvent::setButton(int bnum, int press)
{
    m_bnum = bnum;
    m_press = press;
}

void Ko3DMouseEvent::setMovement(int x, int y, int z, int rx, int ry, int rz)
{
    m_x=x;
    m_y=y;
    m_z=z;
    m_rx=rx;
    m_ry=ry;
    m_rz=rz;
}

void Ko3DMouseEvent::setType(int type)
{
    m_type=type;
}

int Ko3DMouseEvent::getPress()
{
    return m_press;
}

int Ko3DMouseEvent::getBNum()
{
    return m_bnum;
}

int Ko3DMouseEvent::getRZ()
{
    return m_rz;
}

int Ko3DMouseEvent::getRY()
{
    return m_ry;
}

int Ko3DMouseEvent::getRX()
{
    return m_rx;
}

int Ko3DMouseEvent::getZ()
{
    return m_z;
}

int Ko3DMouseEvent::getY()
{
    return m_y;
}

int Ko3DMouseEvent::getX()
{
    return m_z;
}

int Ko3DMouseEvent::getType()
{
    return m_type;
}
