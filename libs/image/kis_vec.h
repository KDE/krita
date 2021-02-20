/*
 *  kis_vec.h - part of KImageShop
 *
 *  SPDX-FileCopyrightText: 1999 Matthias Elter <me@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
