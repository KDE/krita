/*
 *  kis_vec.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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

#ifndef __kis_vec_h__
#define __kis_vec_h__

#include <QPoint>
#include <Eigen/Core>
#include <QVector2D>


typedef Eigen::Matrix<qreal, 2, 1> KisVector2D;

inline KisVector2D toKisVector2D(const QPointF& p)
{
    return KisVector2D(p.x(), p.y());
}
inline KisVector2D toKisVector2D(const QPoint& p)
{
    return KisVector2D(p.x(), p.y());
}

template<typename ExpressionType>
inline QPointF toQPointF(const ExpressionType& expr)
{
    return QPointF(expr.x(), expr.y());
}

#endif
