/* This file is part of the KDE project
   Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>

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

#include "TestResourceManager.h"

#include "KoCanvasResourceManager.h"
#include "KoPathShape.h"
#include <QSignalSpy>

void TestResourceManager::koShapeResource()
{
    KoPathShape * shape = new KoPathShape();
    int key = 9001;

    KoCanvasResourceManager rp( 0 );
    rp.setResource( key, shape );
    QVERIFY( shape == rp.koShapeResource( key ) );
}

void TestResourceManager::testUnitChanged()
{
    KoCanvasResourceManager rm(0);
    QSignalSpy spy(&rm, SIGNAL(canvasResourceChanged(int, const QVariant &)));

    KoUnit a;
    rm.setResource(KoCanvasResourceManager::Unit, a);
    QCOMPARE(spy.count(), 1);

    KoUnit b(KoUnit::Millimeter);
    rm.setResource(KoCanvasResourceManager::Unit, b);
    QCOMPARE(spy.count(), 2);
}

QTEST_MAIN(TestResourceManager)
#include <TestResourceManager.moc>
