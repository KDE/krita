/*
 * Copyright (C) 2007 Boudewijn Rempt <boud@kde.org>
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

#include <QTest>
#include <QCoreApplication>

#include <qtest_kde.h>
#include "kis_prescaled_projection_test.h"
#include "kis_prescaled_projection.h"

void KisPrescaledProjectionTest::testCreation()
{
    KisPrescaledProjection * prescaledProjection = 0;
    prescaledProjection = new KisPrescaledProjection();
    QVERIFY( prescaledProjection != 0 );
    QVERIFY( prescaledProjection->drawCheckers() == false );
    QVERIFY( prescaledProjection->prescaledPixmap().isNull() );
    QVERIFY( prescaledProjection->prescaledQImage().isNull() );
    delete prescaledProjection;
}

QTEST_KDEMAIN(KisPrescaledProjectionTest, GUI)

#include "kis_prescaled_projection_test.moc"

