/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_asl_callback_object_catcher.h"

#include <QHash>

#include <QString>
#include <QPointF>
#include <QColor>

#include "kis_debug.h"

typedef QHash<QString, ASLCallbackDouble> MapHashDouble;
typedef QHash<QString, ASLCallbackInteger> MapHashInt;

struct EnumMapping {
    EnumMapping(const QString &_typeId, ASLCallbackString _map)
        : typeId(_typeId),
          map(_map)
        {
        }

    QString typeId;
    ASLCallbackString map;
};

typedef QHash<QString, EnumMapping> MapHashEnum;

struct UnitFloatMapping {
    UnitFloatMapping(const QString &_unit, ASLCallbackDouble _map)
        : unit(_unit),
          map(_map)
        {
        }

    QString unit;
    ASLCallbackDouble map;
};

typedef QHash<QString, UnitFloatMapping> MapHashUnitFloat;

typedef QHash<QString, ASLCallbackString> MapHashText;
typedef QHash<QString, ASLCallbackBoolean> MapHashBoolean;
typedef QHash<QString, ASLCallbackColor> MapHashColor;
typedef QHash<QString, ASLCallbackPoint> MapHashPoint;
typedef QHash<QString, ASLCallbackCurve> MapHashCurve;
typedef QHash<QString, ASLCallbackPattern> MapHashPattern;
typedef QHash<QString, ASLCallbackPatternRef> MapHashPatternRef;
typedef QHash<QString, ASLCallbackGradient> MapHashGradient;

struct KisAslCallbackObjectCatcher::Private
{
    MapHashDouble mapDouble;
    MapHashInt mapInteger;
    MapHashEnum mapEnum;
    MapHashUnitFloat mapUnitFloat;
    MapHashText mapText;
    MapHashBoolean mapBoolean;
    MapHashColor mapColor;
    MapHashPoint mapPoint;
    MapHashCurve mapCurve;
    MapHashPattern mapPattern;
    MapHashPatternRef mapPatternRef;
    MapHashGradient mapGradient;

    ASLCallbackNewStyle newStyleCallback;
};


KisAslCallbackObjectCatcher::KisAslCallbackObjectCatcher()
    : m_d(new Private)
{
}

KisAslCallbackObjectCatcher::~KisAslCallbackObjectCatcher()
{
}

template <class HashType, typename T>
inline void passToCallback(const QString &path, const HashType &hash, const T &value)
{
    typename HashType::const_iterator it = hash.constFind(path);
    if (it != hash.constEnd()) {
        (*it)(value);
    }
}

template <class HashType, typename T1, typename T2>
inline void passToCallback(const QString &path, const HashType &hash, const T1 &value1, const T2 &value2)
{
    typename HashType::const_iterator it = hash.constFind(path);
    if (it != hash.constEnd()) {
        (*it)(value1, value2);
    }
    else {
        warnKrita << "Couldn't find a callback, even though the non-empty catcher is used";
    }
}

void KisAslCallbackObjectCatcher::addDouble(const QString &path, double value)
{
    passToCallback(path, m_d->mapDouble, value);
}

void KisAslCallbackObjectCatcher::addInteger(const QString &path, int value)
{
    passToCallback(path, m_d->mapInteger, value);
}

void KisAslCallbackObjectCatcher::addEnum(const QString &path, const QString &typeId, const QString &value)
{
    MapHashEnum::const_iterator it = m_d->mapEnum.constFind(path);
    if (it != m_d->mapEnum.constEnd()) {
        if (it->typeId == typeId) {
            it->map(value);
        } else {
            warnKrita << "KisAslCallbackObjectCatcher::addEnum: inconsistent typeId"  << ppVar(typeId) << ppVar(it->typeId);
        }
    }
}

void KisAslCallbackObjectCatcher::addUnitFloat(const QString &path, const QString &unit, double value)
{
    MapHashUnitFloat::const_iterator it = m_d->mapUnitFloat.constFind(path);
    if (it != m_d->mapUnitFloat.constEnd()) {
        if (it->unit == unit) {
            it->map(value);
        } else {
            warnKrita << "KisAslCallbackObjectCatcher::addUnitFloat: inconsistent unit"  << ppVar(unit) << ppVar(it->unit);
        }
    }
}

void KisAslCallbackObjectCatcher::addText(const QString &path, const QString &value)
{
    passToCallback(path, m_d->mapText, value);
}

void KisAslCallbackObjectCatcher::addBoolean(const QString &path, bool value)
{
    passToCallback(path, m_d->mapBoolean, value);
}

void KisAslCallbackObjectCatcher::addColor(const QString &path, const QColor &value)
{
    passToCallback(path, m_d->mapColor, value);
}

void KisAslCallbackObjectCatcher::addPoint(const QString &path, const QPointF &value)
{
    passToCallback(path, m_d->mapPoint, value);
}

void KisAslCallbackObjectCatcher::addCurve(const QString &path, const QString &name, const QVector<QPointF> &points)
{
    MapHashCurve::const_iterator it = m_d->mapCurve.constFind(path);
    if (it != m_d->mapCurve.constEnd()) {
        (*it)(name, points);
    }
}

void KisAslCallbackObjectCatcher::addPattern(const QString &path, const KoPatternSP value, const QString& patternUuid)
{
    passToCallback(path, m_d->mapPattern, value, patternUuid);
}

void KisAslCallbackObjectCatcher::addPatternRef(const QString &path, const QString &patternUuid, const QString &patternName)
{
    MapHashPatternRef::const_iterator it = m_d->mapPatternRef.constFind(path);
    if (it != m_d->mapPatternRef.constEnd()) {
        (*it)(patternUuid, patternName);
    }
}

void KisAslCallbackObjectCatcher::addGradient(const QString &path, KoAbstractGradientSP value)
{
    passToCallback(path, m_d->mapGradient, value);
}

void KisAslCallbackObjectCatcher::newStyleStarted()
{
    if (m_d->newStyleCallback) {
        m_d->newStyleCallback();
    }
}

/*****************************************************************/
/*      Subscription methods                                      */
/*****************************************************************/

void KisAslCallbackObjectCatcher::subscribeDouble(const QString &path, ASLCallbackDouble callback)
{
    m_d->mapDouble.insert(path, callback);
}

void KisAslCallbackObjectCatcher::subscribeInteger(const QString &path, ASLCallbackInteger callback)
{
    m_d->mapInteger.insert(path, callback);
}

void KisAslCallbackObjectCatcher::subscribeEnum(const QString &path, const QString &typeId, ASLCallbackString callback)
{
    m_d->mapEnum.insert(path, EnumMapping(typeId, callback));
}

void KisAslCallbackObjectCatcher::subscribeUnitFloat(const QString &path, const QString &unit, ASLCallbackDouble callback)
{
    m_d->mapUnitFloat.insert(path, UnitFloatMapping(unit, callback));
}

void KisAslCallbackObjectCatcher::subscribeText(const QString &path, ASLCallbackString callback)
{
    m_d->mapText.insert(path, callback);
}

void KisAslCallbackObjectCatcher::subscribeBoolean(const QString &path, ASLCallbackBoolean callback)
{
    m_d->mapBoolean.insert(path, callback);
}

void KisAslCallbackObjectCatcher::subscribeColor(const QString &path, ASLCallbackColor callback)
{
    m_d->mapColor.insert(path, callback);
}

void KisAslCallbackObjectCatcher::subscribePoint(const QString &path, ASLCallbackPoint callback)
{
    m_d->mapPoint.insert(path, callback);
}

void KisAslCallbackObjectCatcher::subscribeCurve(const QString &path, ASLCallbackCurve callback)
{
    m_d->mapCurve.insert(path, callback);
}

void KisAslCallbackObjectCatcher::subscribePattern(const QString &path, ASLCallbackPattern callback)
{
    m_d->mapPattern.insert(path, callback);
}

void KisAslCallbackObjectCatcher::subscribePatternRef(const QString &path, ASLCallbackPatternRef callback)
{
    m_d->mapPatternRef.insert(path, callback);
}

void KisAslCallbackObjectCatcher::subscribeGradient(const QString &path, ASLCallbackGradient callback)
{
    m_d->mapGradient.insert(path, callback);
}

void KisAslCallbackObjectCatcher::subscribeNewStyleStarted(ASLCallbackNewStyle callback)
{
    m_d->newStyleCallback = callback;
}
