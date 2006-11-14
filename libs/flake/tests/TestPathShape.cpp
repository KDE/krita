/* This file is part of the KDE project
   Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include "TestPathShape.h"

#include <QPainterPath>
#include "KoPathShape.h"

void TestPathShape::close()
{
    KoPathShape path;
    path.lineTo( QPointF( 10, 0 ) );
    QPainterPath ppath( QPointF( 0, 0 ) );
    ppath.lineTo( QPointF( 10, 0 ) );

    path.lineTo( QPointF( 10, 10 ) );
    ppath.lineTo( 10, 10 );
    path.close();
    ppath.closeSubpath();
    QVERIFY( ppath == path.outline() );
    path.lineTo( QPointF( 0, 10 ) );
    ppath.lineTo( 0, 10 );
    QVERIFY( ppath == path.outline() );
}

void TestPathShape::moveTo()
{
    KoPathShape path;
    path.moveTo( QPointF( 10, 10 ) );
    QPainterPath ppath( QPointF( 10, 10 ) );
    path.lineTo( QPointF( 20, 20 ) );
    ppath.lineTo( 20, 20 );
    QVERIFY( ppath == path.outline() );
    path.moveTo( QPointF( 30, 30 ) );
    ppath.moveTo( 30, 30 );
    path.lineTo( QPointF( 40, 40 ) );
    ppath.lineTo( QPointF( 40, 40 ) );
    QVERIFY( ppath == path.outline() );
}

void TestPathShape::normalize()
{
    KoPathShape path;
    path.moveTo( QPointF( 10, 10 ) );
    path.lineTo( QPointF( 20, 20 ) );
    path.normalize();
    QPainterPath ppath( QPointF( 0, 0 ) );
    ppath.lineTo( 10, 10 );
    QVERIFY( ppath == path.outline() );
}

QTEST_MAIN(TestPathShape)
#include "TestPathShape.moc"
