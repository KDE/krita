/* This file is part of the KDE project
 * Copyright (C) 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#include "PropertyContainer.h"

PropertyContainer::PropertyContainer(QString name, QObject* parent)
    : QObject(parent)
    , m_name(name)
{
}

PropertyContainer::~PropertyContainer()
{
}

void PropertyContainer::writeProperty(QString name, QVariant value)
{
    setProperty(name.toLatin1(), value);
}

QVariant PropertyContainer::readProperty(QString name)
{
    return property(name.toLatin1());
}

const KisCubicCurve& PropertyContainer::curve() const
{
    return m_curve;
}

void PropertyContainer::setCurve(const KisCubicCurve& curve)
{
    m_curve = curve;
}

QList< KisCubicCurve >& PropertyContainer::curves() const
{
    return m_curves;
}

void PropertyContainer::setCurves(const QList< KisCubicCurve >& curves)
{
    m_curves.clear();
    m_curves = curves;
}

int PropertyContainer::curveCount() const
{
    return m_curves.count();
}

KisCubicCurve PropertyContainer::specificCurve(int index) const
{
    if(index > -1 && index < m_curves.count())
    {
        return KisCubicCurve(m_curves[index]);
    }
    return KisCubicCurve();
}

QString PropertyContainer::specificCurveName(int index) const
{
    if(index > -1 && index < m_curves.count())
    {
        return m_curves[index].name();
    }
    return QString();
}

void PropertyContainer::setSpecificCurve(int index, const KisCubicCurve& curve) const
{

    m_curves[index] = curve;
}

QString PropertyContainer::name()
{
    return m_name;
}
