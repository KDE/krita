/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_group_layer_test.h"
#include <QTest>

#include "testutil.h"

#include <KoColor.h>

#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_types.h"
#include "KoColorSpaceRegistry.h"
#include "kis_image.h"

#include "kis_undo_stores.h"


void KisGroupLayerTest::testProjection()
{
    const KoColorSpace * colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 512, 512, colorSpace, "merge test");

}

#include "commands/kis_image_layer_remove_command.h"

void KisGroupLayerTest::testRemoveAndUndo()
{
    KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();

//    QRect transpRect(50,50,300,300);
//    QRect blurRect(66,66,300,300);
//    QPoint blurShift(34,34);
//    QPoint cloneShift(75,75);

    QRect imageRect = QRect(0, 0, 64, 64);
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(undoStore, imageRect.width(), imageRect.height(), cs, "merge test");

    QRect rect1(10, 10, 30, 30);
    QRect rect2(30, 30, 30, 30);

    KisPaintLayerSP paintLayer0 = new KisPaintLayer(image, "paint0", OPACITY_OPAQUE_U8);
    paintLayer0->paintDevice()->fill(imageRect, KoColor(Qt::white, cs));

    KisPaintLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    paintLayer1->paintDevice()->fill(rect1, KoColor(Qt::red, cs));

    KisPaintLayerSP paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);
    paintLayer2->paintDevice()->fill(rect2, KoColor(Qt::blue, cs));


    KisGroupLayerSP groupLayer1 = new KisGroupLayer(image, "group1", OPACITY_OPAQUE_U8);

    image->addNode(paintLayer0, image->root());
    image->addNode(groupLayer1, image->root());
    image->addNode(paintLayer1, groupLayer1);
    image->addNode(paintLayer2, groupLayer1);

    image->initialRefreshGraph();
    QVERIFY(TestUtil::checkQImage(image->projection()->convertToQImage(0, imageRect),
                                  "group_layer_test",
                                  "undo_removal",
                                  "0_initial"));

    // touch original to cause clearing of the projection
    groupLayer1->original();

    image->undoAdapter()->addCommand(new KisImageLayerRemoveCommand(image, groupLayer1));
    image->waitForDone();

    // touch original again
    groupLayer1->original();

    QVERIFY(TestUtil::checkQImage(image->projection()->convertToQImage(0, imageRect),
                                  "group_layer_test",
                                  "undo_removal",
                                  "1_deleted"));

    undoStore->undo();
    image->waitForDone();

    // hey, man! don't sleep!
    groupLayer1->original();

    QVERIFY(TestUtil::checkQImage(image->projection()->convertToQImage(0, imageRect),
                                  "group_layer_test",
                                  "undo_removal",
                                  "2_undone"));
}

QTEST_MAIN(KisGroupLayerTest)


