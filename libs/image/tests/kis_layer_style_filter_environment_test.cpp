/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_layer_style_filter_environment_test.h"

#include "layerstyles/kis_layer_style_filter_environment.h"
#include "kis_pixel_selection.h"
#include <testutil.h>


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
