/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
