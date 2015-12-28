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

#include "kis_node_juggler_compressed_test.h"

#include <QTest>
#include "../kis_node_juggler_compressed.h"


#include <KoColor.h>
#include <KoColorSpace.h>

void KisNodeJugglerCompressedTest::init()
{
    p.reset(new TestUtil::MaskParent);

    QRect rect1(100, 100, 100, 100);
    QRect rect2(150, 150, 150, 150);
    QRect rect3(50, 50, 100, 100);

    layer1 = p->layer;
    layer1->paintDevice()->fill(rect1, KoColor(Qt::red, layer1->colorSpace()));

    layer2 = new KisPaintLayer(p->image, "paint2", OPACITY_OPAQUE_U8);
    layer2->paintDevice()->fill(rect2, KoColor(Qt::blue, layer2->colorSpace()));

    p->image->addNode(layer2);
    p->image->initialRefreshGraph();
}

void KisNodeJugglerCompressedTest::cleanup()
{
    p.reset();
    layer1.clear();
    layer2.clear();
}

void KisNodeJugglerCompressedTest::testMove(int delayBeforeEnd)
{
    TestUtil::ExternalImageChecker chk("node_juggler", "move_test");
    chk.setMaxFailingPixels(0);

    KisNodeJugglerCompressed juggler(kundo2_i18n("Move Layer"), p->image, 0, 600);
    QVERIFY(chk.checkImage(p->image, "initial"));

    juggler.moveNode(layer1, p->image->root(), layer2);
    QTest::qWait(100);
    QVERIFY(chk.checkImage(p->image, "initial"));

    if (delayBeforeEnd) {
        QTest::qWait(delayBeforeEnd);
        QVERIFY(chk.checkImage(p->image, "moved"));
    }

    juggler.end();
    p->image->waitForDone();
    QVERIFY(chk.checkImage(p->image, "moved"));

    p->undoStore->undo();
    p->image->waitForDone();

    QVERIFY(chk.checkImage(p->image, "initial"));
}

void KisNodeJugglerCompressedTest::testApplyUndo()
{
    testMove(1000);
}

void KisNodeJugglerCompressedTest::testEndBeforeUpdate()
{
    testMove(0);
}

QTEST_MAIN(KisNodeJugglerCompressedTest)
