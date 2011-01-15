/*
 *  This file is part of Calligra tests
 *
 *  Copyright (C) 2011 Hanna Skott <hannaetscott@gmail.com>
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

#include "TestInputDevice.h"

#include <qtest_kde.h>
#include "KoInputDevice.h"


void TestInputDevice::testTabletConstructor()
{
  QTabletEvent::TabletDevice parameterTabletDevice = QTabletEvent::Stylus;
  QTabletEvent::PointerType parameterPointerType = QTabletEvent::Eraser; 
  qint64 parameterUniqueTabletId = 0;
  KoInputDevice toTest(parameterTabletDevice, parameterPointerType, parameterUniqueTabletId);
  bool isMouse = toTest.isMouse();
  QVERIFY(!isMouse);
}

void TestInputDevice::testNoParameterConstructor()
{
  KoInputDevice toTest;
  bool isMouse = toTest.isMouse();
  qint64 theUniqueTabletId = toTest.uniqueTabletId();
  qint64 toCompareWith = -1;
  QVERIFY(isMouse);
  QCOMPARE(theUniqueTabletId, toCompareWith);
}

void TestInputDevice::testConstructorWithSingleReference()
{
  QTabletEvent::TabletDevice parameterTabletDevice = QTabletEvent::Stylus;
  QTabletEvent::PointerType parameterPointerType = QTabletEvent::Eraser; 
  qint64 parameterUniqueTabletId = 0;
  KoInputDevice parameterTabletDevice2(parameterTabletDevice, parameterPointerType, parameterUniqueTabletId);
  KoInputDevice toTest(parameterTabletDevice2);
  bool isMouse = toTest.isMouse();
  QVERIFY(!isMouse);
  KoInputDevice parameterMouseDevice;
  toTest = KoInputDevice(parameterMouseDevice);
  isMouse = toTest.isMouse();
  QVERIFY(isMouse);
}

void TestInputDevice::testEqualityCheckOperator()
{
  QTabletEvent::TabletDevice parameterTabletDevice = QTabletEvent::Stylus;
  QTabletEvent::PointerType parameterPointerType = QTabletEvent::Eraser; 
  qint64 parameterUniqueTabletId = 0;
  KoInputDevice parameterTabletDevice2(parameterTabletDevice, parameterPointerType, parameterUniqueTabletId);
  KoInputDevice toTest(parameterTabletDevice2);
  KoInputDevice copy = toTest;
  KoInputDevice notSame;
  QVERIFY(toTest==copy);
  QVERIFY(toTest!=notSame);
}

void TestInputDevice::testMouse()
{
  KoInputDevice toTest = KoInputDevice::mouse();
  bool isMouse = toTest.isMouse();
  QVERIFY(isMouse);
    //QCOMPARE(a, 2);
}
void TestInputDevice::testStylus()
{
  KoInputDevice toTest = KoInputDevice::stylus();
  bool isMouse = toTest.isMouse();
  QVERIFY(!isMouse);
}
void TestInputDevice::testEraser()
{
  KoInputDevice toTest = KoInputDevice::eraser();
  bool isMouse = toTest.isMouse();
  QVERIFY(!isMouse);
}


QTEST_KDEMAIN(TestInputDevice, GUI)
#include <TestInputDevice.moc>
