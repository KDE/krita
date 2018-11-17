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

#ifndef __KIS_ASL_OBJECT_CATCHER_H
#define __KIS_ASL_OBJECT_CATCHER_H

#include <QVector>

#include <KoPattern.h>

class QString;
class QColor;
class QPointF;
class KoAbstractGradient;

#include "kritapsd_export.h"

template<class T> class QSharedPointer;
typedef QSharedPointer<KoAbstractGradient> KoAbstractGradientSP;

class KRITAPSD_EXPORT KisAslObjectCatcher
{
public:
    KisAslObjectCatcher();
    virtual ~KisAslObjectCatcher();

    virtual void addDouble(const QString &path, double value);
    virtual void addInteger(const QString &path, int value);
    virtual void addEnum(const QString &path, const QString &typeId, const QString &value);
    virtual void addUnitFloat(const QString &path, const QString &unit, double value);
    virtual void addText(const QString &path, const QString &value);
    virtual void addBoolean(const QString &path, bool value);
    virtual void addColor(const QString &path, const QColor &value);
    virtual void addPoint(const QString &path, const QPointF &value);
    virtual void addCurve(const QString &path, const QString &name, const QVector<QPointF> &points);
    virtual void addPattern(const QString &path, const KoPatternSP pattern);
    virtual void addPatternRef(const QString &path, const QString &patternUuid, const QString &patternName);
    virtual void addGradient(const QString &path, KoAbstractGradientSP gradient);

    virtual void newStyleStarted();

    void setArrayMode(bool value);
protected:
    bool m_arrayMode;
};

#endif /* __KIS_ASL_OBJECT_CATCHER_H */
