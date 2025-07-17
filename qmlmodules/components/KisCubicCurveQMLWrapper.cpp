/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisCubicCurveQMLWrapper.h"

KisCubicCurveQml::KisCubicCurveQml(QObject *parent)
    : QObject(parent)
{}

const QList<QPointF>& KisCubicCurveQml::points() const
{
    return m_curve.points();
}

const QString& KisCubicCurveQml::name() const
{
    return m_curve.name();
}

qreal KisCubicCurveQml::value(qreal x) const
{
    return m_curve.value(x); 
}

bool KisCubicCurveQml::isIdentity() const
{
    return m_curve.isIdentity();
}

bool KisCubicCurveQml::isConstant(qreal c) const
{
    return m_curve.isConstant(c);
}

qreal KisCubicCurveQml::interpolateLinear(qreal normalizedValue, const QList<qreal> &transfer) const
{
    return m_curve.interpolateLinear(normalizedValue, transfer.toVector());
}

QList<qreal> KisCubicCurveQml::floatTransfer(int size) const
{
    return m_curve.floatTransfer(size).toList();
}

QString KisCubicCurveQml::toString() const
{
    return m_curve.toString();
}

void KisCubicCurveQml::fromString(const QString &str)
{
    m_curve.fromString(str);
}


void KisCubicCurveQml::setPoints(const QList<QPointF> &points)
{
    m_curve.setPoints(points);
    emit pointsChanged(m_curve.points());
}

void KisCubicCurveQml::setPoint(int idx, const QPointF &point)
{
    m_curve.setPoint(idx, point);
    emit pointsChanged(m_curve.points());
}

int KisCubicCurveQml::addPoint(const QPointF &point)
{
    const int idx = m_curve.addPoint(point);
    emit pointsChanged(m_curve.points());
    return idx;
}

void KisCubicCurveQml::removePoint(int idx)
{
    m_curve.removePoint(idx);
    emit pointsChanged(m_curve.points());
}

void KisCubicCurveQml::setName(const QString& name)
{
    m_curve.setName(name);
    emit nameChanged(name);
}
