/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    virtual void addPattern(const QString &path, const KoPatternSP pattern, const QString &patternUuid);
    virtual void addPatternRef(const QString &path, const QString &patternUuid, const QString &patternName);
    virtual void addGradient(const QString &path, KoAbstractGradientSP gradient);

    virtual void newStyleStarted();

    void setArrayMode(bool value);
protected:
    bool m_arrayMode;
};

#endif /* __KIS_ASL_OBJECT_CATCHER_H */
