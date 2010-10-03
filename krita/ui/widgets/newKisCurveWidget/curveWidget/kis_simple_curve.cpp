/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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

#include "kis_simple_curve.h"

#include <QString>

KisSimpleCurve::KisSimpleCurve() : KisCurve()
{
}

QList<QPointF> KisSimpleCurve::points() const
{
    return m_points;
}

void KisSimpleCurve::setPoints(const QList<QPointF>& points)
{
    m_points = points;
}

void KisSimpleCurve::setPoint(int idx, const QPointF& point)
{
    if(idx<m_points.size() && idx>=0) {
        m_points[idx]=point;
    }
    else {
        Q_ASSERT(false);
    }
}

int KisSimpleCurve::addPoint(const QPointF& point)
{
    m_points.append(point);
    qSort(m_points.begin(), m_points.end(), pointCompare);
    return m_points.indexOf(point);
}

void KisSimpleCurve::removePoint(int idx)
{
    if(idx<m_points.size() && idx>=0) {
        m_points.removeAt(idx);
    }
    else {
        Q_ASSERT(false);
    }
}

QString KisSimpleCurve::toString() const
{
    QString ret = className();
    ret.append(':');

    for(int i=0; i<m_points.size(); i++) {
        ret.append(QString::number(m_points.at(i).x()));
        ret.append(',');
        ret.append(QString::number(m_points.at(i).y()));
        ret.append(';');
    }

    return ret;
}

QVector<quint16> KisSimpleCurve::uint16Transfer(int size) const
{
    return transfer<quint16>(size);
}

QVector<qreal> KisSimpleCurve::floatTransfer(int size) const
{
    return transfer<qreal>(size);
}

qreal KisSimpleCurve::value(qreal x) const
{
    Q_UNUSED(x);
    return 0.5;
}

QPainterPath KisSimpleCurve::painterPath() const
{
    return m_painterPath;
}

template <typename T>
QVector<T> KisSimpleCurve::transfer(int size) const
{
    QList<QPointF> data;

    // data must be more precise, then size points
    qreal delta = 1.0/qreal(2*size);
    for(qreal t=0; t<=1.0; t+=delta) {
        data.append(m_painterPath.pointAtPercent(t));
    }

    QVector<T> transferedData;
    delta = 1.0/qreal(size);
    int j=0;
    for(int i=0; i<data.size(); i++) {
        qreal currentSearchX = j*delta;
        qreal currentIx = data.at(i).x();
        qreal nextIx = (i+1<data.size())?(data.at(i+1).x()):1.0;

        if(currentIx<currentSearchX && currentSearchX<nextIx) {
            // linear interpolation

        }
    }
}
