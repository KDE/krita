/*
 *  tool_transform_args.h - part of Krita
 *
 *  Copyright (c) 2010 Marc Pegon
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

#include "tool_transform_args.h"

ToolTransformArgs::ToolTransformArgs()
{
    m_translate = QPointF(0, 0);
    m_rotationCenterOffset = QPointF(0, 0);
    m_aX = 0;
    m_aY = 0;
    m_aZ = 0;
    m_scaleX = 1.0;
    m_scaleY = 1.0;
    m_shearX = 0.0;
    m_shearY = 0.0;
}

ToolTransformArgs::ToolTransformArgs(QPointF translate, QPointF rotationCenterOffset, double aX, double aY, double aZ, double scaleX, double scaleY, double shearX, double shearY)
{
    m_translate = translate;
    m_rotationCenterOffset = rotationCenterOffset;
    m_aX = aX;
    m_aY = aY;
    m_aZ = aZ;
    m_scaleX = scaleX;
    m_scaleY = scaleY;
    m_shearX = shearX;
    m_shearY = shearY;
}


ToolTransformArgs::~ToolTransformArgs()
{
}


QPointF ToolTransformArgs::translate()
{
    return m_translate;
}

QPointF ToolTransformArgs::rotationCenterOffset()
{
    return m_rotationCenterOffset;
}

double ToolTransformArgs::aX()
{
    return m_aX;
}

double ToolTransformArgs::aY()
{
    return m_aY;
}

double ToolTransformArgs::aZ()
{
    return m_aZ;
}

double ToolTransformArgs::scaleX()
{
    return m_scaleX;
}

double ToolTransformArgs::scaleY()
{
    return m_scaleY;
}

double ToolTransformArgs::shearX()
{
    return m_shearX;
}

double ToolTransformArgs::shearY()
{
    return m_shearY;
}

void ToolTransformArgs::setTranslate(QPointF translate)
{
    m_translate = translate;
}

void ToolTransformArgs::setRotationCenterOffset(QPointF rotationCenterOffset)
{
    m_rotationCenterOffset = rotationCenterOffset;
}

void ToolTransformArgs::setAX(double aX)
{
    m_aX = aX;
}

void ToolTransformArgs::setAY(double aY)
{
    m_aY = aY;
}

void ToolTransformArgs::setAZ(double aZ)
{
    m_aZ = aZ;
}

void ToolTransformArgs::setScaleX(double scaleX)
{
    m_scaleX = scaleX;
}

void ToolTransformArgs::setScaleY(double scaleY)
{
    m_scaleY = scaleY;
}

void ToolTransformArgs::setShearX(double shearX)
{
    m_shearX = shearX;
}

void ToolTransformArgs::setShearY(double shearY)
{
    m_shearY = shearY;
}
