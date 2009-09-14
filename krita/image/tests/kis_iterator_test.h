/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@kde.org>
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

#ifndef KIS_ITERATOR_TEST_H
#define KIS_ITERATOR_TEST_H

#include <QtTest/QtTest>

class KoColorSpace;

class KisIteratorTest : public QObject
{
    Q_OBJECT

private:
    void allCsApplicator(void (KisIteratorTest::* funcPtr)(const KoColorSpace*cs));

    void vLineIter(const KoColorSpace * cs);
    void writeBytes(const KoColorSpace * cs);
    void fill(const KoColorSpace * cs);
    void rectIter(const KoColorSpace * cs);
    void hLineIter(const KoColorSpace * cs);
    void randomAccessor(const KoColorSpace * cs);

private slots:

    void vLineIter();
    void writeBytes();
    void fill();
    void rectIter();
    void hLineIter();
    void randomAccessor();
};

#endif

