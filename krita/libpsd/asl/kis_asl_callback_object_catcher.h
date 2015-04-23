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

#ifndef __KIS_ASL_CALLBACK_OBJECT_CATCHER_H
#define __KIS_ASL_CALLBACK_OBJECT_CATCHER_H

#include "kis_asl_object_catcher.h"

#include <boost/function.hpp>
#include <QScopedPointer>

#include "libkispsd_export.h"

class KoPattern;

typedef boost::function<void (double)> ASLCallbackDouble;
typedef boost::function<void (int)> ASLCallbackInteger;
typedef boost::function<void (const QString &)> ASLCallbackString;
typedef boost::function<void (bool)> ASLCallbackBoolean;
typedef boost::function<void (const QColor &)> ASLCallbackColor;
typedef boost::function<void (const QPointF &)> ASLCallbackPoint;
typedef boost::function<void (const QString &, const QVector<QPointF> &)> ASLCallbackCurve;
typedef boost::function<void (const KoPattern *)> ASLCallbackPattern;
typedef boost::function<void (const QString &, const QString &)> ASLCallbackPatternRef;
typedef boost::function<void (KoAbstractGradientSP)> ASLCallbackGradient;
typedef boost::function<void ()> ASLCallbackNewStyle;


class LIBKISPSD_EXPORT KisAslCallbackObjectCatcher : public KisAslObjectCatcher
{
public:
    KisAslCallbackObjectCatcher();
    ~KisAslCallbackObjectCatcher();

    void addDouble(const QString &path, double value);
    void addInteger(const QString &path, int value);
    void addEnum(const QString &path, const QString &typeId, const QString &value);
    void addUnitFloat(const QString &path, const QString &unit, double value);
    void addText(const QString &path, const QString &value);
    void addBoolean(const QString &path, bool value);
    void addColor(const QString &path, const QColor &value);
    void addPoint(const QString &path, const QPointF &value);
    void addCurve(const QString &path, const QString &name, const QVector<QPointF> &points);
    void addPattern(const QString &path, const KoPattern *pattern);
    void addPatternRef(const QString &path, const QString &patternUuid, const QString &patternName);
    void addGradient(const QString &path, KoAbstractGradientSP gradient);
    void newStyleStarted();

    void subscribeDouble(const QString &path, ASLCallbackDouble callback);
    void subscribeInteger(const QString &path, ASLCallbackInteger callback);
    void subscribeEnum(const QString &path, const QString &typeId, ASLCallbackString callback);
    void subscribeUnitFloat(const QString &path, const QString &unit, ASLCallbackDouble callback);
    void subscribeText(const QString &path, ASLCallbackString callback);
    void subscribeBoolean(const QString &path, ASLCallbackBoolean callback);
    void subscribeColor(const QString &path, ASLCallbackColor callback);
    void subscribePoint(const QString &path, ASLCallbackPoint callback);
    void subscribeCurve(const QString &path, ASLCallbackCurve callback);
    void subscribePattern(const QString &path, ASLCallbackPattern callback);
    void subscribePatternRef(const QString &path, ASLCallbackPatternRef callback);
    void subscribeGradient(const QString &path, ASLCallbackGradient callback);
    void subscribeNewStyleStarted(ASLCallbackNewStyle callback);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_ASL_CALLBACK_OBJECT_CATCHER_H */
