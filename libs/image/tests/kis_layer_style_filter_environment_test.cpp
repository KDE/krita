/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_layer_style_filter_environment_test.h"

#include "layerstyles/kis_layer_style_filter_environment.h"
#include "kis_pixel_selection.h"
#include "testutil.h"


#include <QTest>

void KisLayerStyleFilterEnvironmentTest::testRandomSelectionCaching()
{
    TestUtil::MaskParent p;
    KisLayerStyleFilterEnvironment env(p.layer.data());

    const QRect r1 = QRect(0,0,100,100);
    const QRect r2 = QRect(0,0,101,101);

    KisPixelSelectionSP selection1 = env.cachedRandomSelection(r1);

    QCOMPARE(selection1->selectedExactRect(), r1);

    KisPixelSelectionSP selection2 = env.cachedRandomSelection(r1);

    QVERIFY(selection1 == selection2);

    KisPixelSelectionSP selection3 = env.cachedRandomSelection(r2);

    QVERIFY(selection1 != selection3);
    QCOMPARE(selection3->selectedExactRect(), r2);
}

void KisLayerStyleFilterEnvironmentTest::benchmarkRandomSelectionGeneration()
{
    TestUtil::MaskParent p;
    KisLayerStyleFilterEnvironment env(p.layer.data());

    QBENCHMARK_ONCE {
        const QRect r1 = QRect(0,0,10000,10000);
        KisPixelSelectionSP selection1 = env.cachedRandomSelection(r1);
    }
}

QTEST_MAIN(KisLayerStyleFilterEnvironmentTest)
