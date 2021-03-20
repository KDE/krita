/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_CUBIC_CURVE_H_
#define _KIS_CUBIC_CURVE_H_

#include<QList>
#include<QVector>
#include<QVariant>

#include <kritaimage_export.h>

class QPointF;

const QString DEFAULT_CURVE_STRING = "0,0;1,1;";

/**
 * Hold the data for a cubic curve.
 */
class KRITAIMAGE_EXPORT KisCubicCurve
{
public:
    KisCubicCurve();
    KisCubicCurve(const QList<QPointF>& points);
    KisCubicCurve(const QVector<QPointF>& points);
    KisCubicCurve(const KisCubicCurve& curve);
    ~KisCubicCurve();
    KisCubicCurve& operator=(const KisCubicCurve& curve);
    bool operator==(const KisCubicCurve& curve) const;
public:
    qreal value(qreal x) const;
    QList<QPointF> points() const;
    void setPoints(const QList<QPointF>& points);
    void setPoint(int idx, const QPointF& point);
    /**
     * Add a point to the curve, the list of point is always sorted.
     * @return the index of the inserted point
     */
    int addPoint(const QPointF& point);
    void removePoint(int idx);

    /*
     * Check whether the curve maps all values to themselves.
     */
    bool isIdentity() const;

    /*
     * Check whether the curve maps all values to given constant.
     */
    bool isConstant(qreal c) const;

    /**
     * This allows us to carry around a display name for the curve internally. It is used
     * currently in Sketch for perchannel, but would potentially be useful anywhere
     * curves are used in the UI
     */
    void setName(const QString& name);
    const QString& name() const;

    static qreal interpolateLinear(qreal normalizedValue, const QVector<qreal> &transfer);

public:
    const QVector<quint16> uint16Transfer(int size = 256) const;
    const QVector<qreal> floatTransfer(int size = 256) const;
public:
    QString toString() const;
    void fromString(const QString&);
private:
    struct Data;
    struct Private;
    Private* const d;
};

Q_DECLARE_METATYPE(KisCubicCurve)

#endif
