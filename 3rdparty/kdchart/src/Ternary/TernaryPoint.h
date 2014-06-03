/****************************************************************************
** Copyright (C) 2001-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Chart library.
**
** Licensees holding valid commercial KD Chart licenses may use this file in
** accordance with the KD Chart Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#ifndef TERNARYPOINT_H
#define TERNARYPOINT_H

#include <QtDebug>
#include <QPointF>

/**
  * @brief TernaryPoint defines a point within a ternary coordinate plane
  * \internal
  */
class TernaryPoint
{
public:
    TernaryPoint();
    TernaryPoint( double a, double b );

    double a() const { return m_a; }
    double b() const { return m_b; }
    double c() const { return 1.0 - m_a - m_b; }

    void set( double a, double b );

    bool isValid() const;

private:
    double m_a;
    double m_b;
};

QDebug operator<<( QDebug stream, const TernaryPoint& point );

QPointF translate( const TernaryPoint& );

#endif
