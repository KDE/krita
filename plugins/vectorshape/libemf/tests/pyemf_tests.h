/*
  Copyright 2008 Brad Hards <bradh@frogmouth.net>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either 
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public 
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PYEMF_TESTS_H
#define PYEMF_TESTS_H

#include <QtTest/QtTest>

class PyEmfTests: public QObject
{
    Q_OBJECT
private slots:
    void test1();
    void testArcChordPie();
    void testDeleteObject();
    void testDrawing1();
    void testFontBackground();
    void testOptimize16Bit();
    void testPaths1();
    void testPoly1();
    void testPoly2();
    void testSetClipPath();
    void testSetPixel();
    void testViewportWindowOrigin();
    void testWorldTransform1();
};

#endif
