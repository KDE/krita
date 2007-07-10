/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef KIS_SELETION_BLTMASK_TEST_H
#define KIS_SELETION_BLTMASK_TEST_H

#include <QtTest/QtTest>

class KoColorSpace;

class KisPainterTest : public QObject
{
    Q_OBJECT

private:
    void allCsApplicator(void (KisPainterTest::* funcPtr)( KoColorSpace*cs ) );
    void testPaintDeviceBltMask( KoColorSpace * cs );
    void testPaintDeviceBltMaskIrregular( KoColorSpace * cs );
private slots:

    void testPaintDeviceBltMask(); // Square selection
    void testPaintDeviceBltMaskIrregular(); // Irregular selection
    void testSelectionBltMask(); // Square selection
    void testSelectionBltMaskIrregular(); // Irregular selection

};

#endif

