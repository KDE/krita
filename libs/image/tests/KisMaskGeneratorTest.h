/*
 *  Copyright (c) 2018 Iván Santa María <ghevan@gmail.com>
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

#ifndef KISMASKGENERATORBENCHMARK_H
#define KISMASKGENERATORBENCHMARK_H

#include <QtTest>

class KisMaskGeneratorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDefaultScalarMask();
    void testDefaultVectorMask();

    void testCircularGaussScalarMask();
    void testCircularGaussVectorMask();

    void testCircularSoftScalarMask();
    void testCircularSoftVectorMask();

    void testRectangularScalarMask();
    void testRectangularVectorMask();

    void testRectangularGaussScalarMask();
    void testRectangularGaussVectorMask();

    void testRectangularSoftScalarMask();
    void testRectangularSoftVectorMask();

};

#endif // KISMASKGENERATORBENCHMARK_H
