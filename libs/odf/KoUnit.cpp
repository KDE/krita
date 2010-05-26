/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>
   Copyright (C) 2004, Nicolas GOUTTE <goutte@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoUnit.h"

#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>

#include <QRegExp>

QStringList KoUnit::listOfUnitName(bool hidePixel)
{
    QStringList lst;
    for (int i = KoUnit::Millimeter; i <= KoUnit::Pixel; ++i) {
        Unit unit = static_cast<Unit>(i);
        if ((i != Pixel) || (hidePixel == false))
            lst.append(KoUnit::unitDescription(KoUnit(unit)));
    }
    return lst;
}

int KoUnit::indexInList(PixelVisibility visibility) const
{
    if (visibility == HidePixel && m_unit > Pixel)
        return m_unit -1;
    else
        return m_unit;
}

QString KoUnit::unitDescription(KoUnit _unit)
{
    switch (_unit.m_unit) {
    case KoUnit::Millimeter:
        return i18n("Millimeters (mm)");
    case KoUnit::Centimeter:
        return i18n("Centimeters (cm)");
    case KoUnit::Decimeter:
        return i18n("Decimeters (dm)");
    case KoUnit::Inch:
        return i18n("Inches (in)");
    case KoUnit::Pica:
        return i18n("Pica (pi)");
    case KoUnit::Cicero:
        return i18n("Cicero (cc)");
    case KoUnit::Point:
        return i18n("Points (pt)");
    case KoUnit::Pixel:
        return i18n("Pixels (px)");
    default:
        return i18n("Error!");
    }
}

qreal KoUnit::toUserValue(qreal ptValue) const
{
    switch (m_unit) {
    case Millimeter:
        return toMillimeter(ptValue);
    case Centimeter:
        return toCentimeter(ptValue);
    case Decimeter:
        return toDecimeter(ptValue);
    case Inch:
        return toInch(ptValue);
    case Pica:
        return toPica(ptValue);
    case Cicero:
        return toCicero(ptValue);
    case Pixel:
        return floor(ptValue * m_pixelConversion + 0.5);
    case Point:
    default:
        return toPoint(ptValue);
    }
}

qreal KoUnit::ptToUnit(const qreal ptValue, const KoUnit &unit)
{
    switch (unit.m_unit) {
    case Millimeter:
        return POINT_TO_MM(ptValue);
    case Centimeter:
        return POINT_TO_CM(ptValue);
    case Decimeter:
        return POINT_TO_DM(ptValue);
    case Inch:
        return POINT_TO_INCH(ptValue);
    case Pica:
        return POINT_TO_PI(ptValue);
    case Cicero:
        return POINT_TO_CC(ptValue);
    case Pixel:
        return ptValue * unit.m_pixelConversion;
    case Point:
    default:
        return ptValue;
    }
}

QString KoUnit::toUserStringValue(qreal ptValue) const
{
    return KGlobal::locale()->formatNumber(toUserValue(ptValue));
}

qreal KoUnit::fromUserValue(qreal value) const
{
    switch (m_unit) {
    case Millimeter:
        return MM_TO_POINT(value);
    case Centimeter:
        return CM_TO_POINT(value);
    case Decimeter:
        return DM_TO_POINT(value);
    case Inch:
        return INCH_TO_POINT(value);
    case Pica:
        return PI_TO_POINT(value);
    case Cicero:
        return CC_TO_POINT(value);
    case Pixel:
        return value / m_pixelConversion;
    case Point:
    default:
        return value;
    }
}

qreal KoUnit::fromUserValue(const QString &value, bool *ok) const
{
    return fromUserValue(KGlobal::locale()->readNumber(value, ok));
}

qreal KoUnit::parseValue(const QString& _value, qreal defaultVal)
{
    if (_value.isEmpty())
        return defaultVal;

    QString value(_value.simplified());
    value.remove(' ');

    int firstLetter = -1;
    for (int i = 0; i < value.length(); ++i) {
        if (value.at(i).isLetter()) {
            if (value.at(i) == 'e')
                continue;
            firstLetter = i;
            break;
        }
    }

    if (firstLetter == -1)
        return value.toDouble();

    const QString unit = value.mid(firstLetter);
    value.truncate(firstLetter);
    const qreal val = value.toDouble();

    if (unit == "pt")
        return val;

    bool ok;
    KoUnit u = KoUnit::unit(unit, &ok);
    if (ok)
        return u.fromUserValue(val);

    if (unit == "m")
        return DM_TO_POINT(val * 10.0);
    else if (unit == "km")
        return DM_TO_POINT(val * 10000.0);
    kWarning() << "KoUnit::parseValue: Unit " << unit << " is not supported, please report.";

    // TODO : add support for mi/ft ?
    return defaultVal;
}

KoUnit KoUnit::unit(const QString &_unitName, bool* ok)
{
    if (ok)
        *ok = true;
    if (_unitName == QString::fromLatin1("mm")) return KoUnit(Millimeter);
    if (_unitName == QString::fromLatin1("cm")) return KoUnit(Centimeter);
    if (_unitName == QString::fromLatin1("dm")) return KoUnit(Decimeter);
    if (_unitName == QString::fromLatin1("in")
            || _unitName == QString::fromLatin1("inch") /*compat*/) return KoUnit(Inch);
    if (_unitName == QString::fromLatin1("pi")) return KoUnit(Pica);
    if (_unitName == QString::fromLatin1("cc")) return KoUnit(Cicero);
    if (_unitName == QString::fromLatin1("pt")) return KoUnit(Point);
    if (_unitName == QString::fromLatin1("px")) return KoUnit(Pixel);
    if (ok)
        *ok = false;
    return KoUnit(Point);
}

QString KoUnit::unitName(KoUnit _unit)
{
    if (_unit.m_unit == Millimeter) return QString::fromLatin1("mm");
    if (_unit.m_unit == Centimeter) return QString::fromLatin1("cm");
    if (_unit.m_unit == Decimeter) return QString::fromLatin1("dm");
    if (_unit.m_unit == Inch) return QString::fromLatin1("in");
    if (_unit.m_unit == Pica) return QString::fromLatin1("pi");
    if (_unit.m_unit == Cicero) return QString::fromLatin1("cc");
    if (_unit.m_unit == Pixel) return QString::fromLatin1("px");
    return QString::fromLatin1("pt");
}
