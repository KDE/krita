/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_FIXED_POINT_MATHS_H
#define __KIS_FIXED_POINT_MATHS_H

#include <boost/operators.hpp>
#include <QDataStream>


class KisFixedPoint : boost::ordered_field_operators<KisFixedPoint>
{
public:
    KisFixedPoint() {
        d = 0;
    }

    KisFixedPoint(int iValue) {
        d = iValue << 8;
    }

    KisFixedPoint(qreal fValue) {
        d = fValue * (1 << 8);
    }

    qint32 toInt() const {
        return d > 0 ? d >> 8 : -((-d) >> 8);
    }

    qint32 toIntRound() const {
        return d > 0 ? (d + (1 << 7)) >> 8 : -((-d + (1 << 7)) >> 8);
    }

    qint32 toIntCeil() const {
        return d > 0 ? (d + ((1 << 8) - 1)) >> 8 : -((-d + ((1 << 8) - 1)) >> 8);
    }

    qreal toFloat() const {
        return qreal(d) / qreal(1 << 8);
    }

    KisFixedPoint& from256Frac(qint32 v) {
        d = v;
        return *this;
    }

    qint32 to256Frac() const {
        return d;
    }

    KisFixedPoint& inc256Frac() {
        d++;
        return *this;
    }

    KisFixedPoint& dec256Frac() {
        d--;
        return *this;
    }

    bool isInteger() const {
        return !(d & ((1 << 8) -1 ));
    }

    bool operator<(const KisFixedPoint& x) const {
        return d < x.d;
    }

    bool operator==(const KisFixedPoint& x) const {
        return d == x.d;
    }

    KisFixedPoint& operator+=(const KisFixedPoint& x) {
        d += x.d;
        return *this;
    }

    KisFixedPoint& operator-=(const KisFixedPoint& x) {
        d -= x.d;
        return *this;
    }

    KisFixedPoint& operator*=(const KisFixedPoint& x) {
        d *= x.d;
        d >>= 8;
        return *this;
    }

    KisFixedPoint& operator/=(const KisFixedPoint& x) {
        d <<= 8;
        d /= x.d;
        return *this;
    }

private:
    friend KisFixedPoint operator-(KisFixedPoint x);
    friend QDebug operator<<(QDebug dbg, const KisFixedPoint &v);

private:
    qint32 d;
};

inline KisFixedPoint operator-(KisFixedPoint x) {
    x.d = -x.d;
    return x;
}

QDebug operator<<(QDebug dbg, const KisFixedPoint &v) {
    dbg.nospace() << v.toFloat() << " (d = " << v.d << ")";
    return dbg.space();
}

#endif /* __KIS_FIXED_POINT_MATHS_H */
