/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "TestKoUnit.h"

#include <KoUnit.h>

void TestKoUnit::testUnit()
{
    KoUnit unit1;
    KoUnit unit2(KoUnit::Point);
    QCOMPARE(unit1, unit2);

    KoUnit unit3(KoUnit::Millimeter);
    unit1 = unit3;
    QCOMPARE(unit1, unit3);

    KoUnit scaled(KoUnit::Pixel, 0.5);
    QCOMPARE(KoUnit::ptToUnit(100, scaled), (qreal)50);

    QVariant variant;
    variant.setValue(scaled);
    QCOMPARE(scaled, variant.value<KoUnit>());
}

QTEST_MAIN(TestKoUnit)
#include "TestKoUnit.moc"
