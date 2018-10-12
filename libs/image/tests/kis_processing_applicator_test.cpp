/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_processing_applicator_test.h"

#include <QTest>

#include <KoColor.h>
#include <KoColorSpaceRegistry.h>
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"

#include "kis_undo_stores.h"
#include "kis_processing_applicator.h"
#include "processing/kis_crop_processing_visitor.h"
#include "kis_image.h"

#include "testutil.h"

/*
  +----------+
  |root      |
  | paint 2  |
  | paint 1  |
  +----------+
*/

KisImageSP createImage(KisUndoStore *undoStore,
                       KisPaintLayerSP &paintLayer1,
                       KisPaintLayerSP &paintLayer2)
{
    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(undoStore, 300, 300, cs, "test");

    QRect fillRect1(50,50,100,100);
    QRect fillRect2(75,75,50,50);

    paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
    paintLayer2 = new KisPaintLayer(image, "paint2", OPACITY_OPAQUE_U8);

    paintLayer1->paintDevice()->fill(fillRect1, KoColor(Qt::white, cs));
    paintLayer2->paintDevice()->fill(fillRect2, KoColor(Qt::red, cs));

    image->addNode(paintLayer1, image->rootLayer());
    image->addNode(paintLayer2, image->rootLayer());

    image->initialRefreshGraph();

    return image;
}

bool checkLayers(KisImageWSP image,
                 const QString &prefix)
{
    KisNodeSP layer1 = image->rootLayer()->firstChild();
    KisNodeSP layer2 = layer1->nextSibling();

    QVector<QImage> images(3);
    images[0] = image->projection()->convertToQImage(0, 0, 0, 300, 300);
    images[1] = layer1->paintDevice()->convertToQImage(0, 0, 0, 300, 300);
    images[2] = layer2->paintDevice()->convertToQImage(0, 0, 0, 300, 300);

    QVector<QString> names(3);
    names[0] = QString("applicator_") + prefix + "_projection.png";
    names[1] = QString("applicator_") + prefix + "_layer1.png";
    names[2] = QString("applicator_") + prefix + "_layer2.png";

    bool valid = true;
    for(int i = 0; i < 3; i++) {
        QImage ref(QString(FILES_DATA_DIR) + QDir::separator() +
                   "applicator" + QDir::separator() + names[i]);

        QPoint temp;

        if(!TestUtil::compareQImages(temp, ref, images[i], 1)) {
            dbgKrita << "--- Wrong image:" << names[i];
            valid = false;
            images[i].save(QString(FILES_OUTPUT_DIR) + QDir::separator() + names[i]);
        }
    }

    return valid;
}


void KisProcessingApplicatorTest::testNonRecursiveProcessing()
{
    KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
    KisPaintLayerSP paintLayer1;
    KisPaintLayerSP paintLayer2;
    KisImageSP image = createImage(undoStore, paintLayer1, paintLayer2);

    QRect cropRect1(25,25,75,75);
    QRect cropRect2(100,100,50,50);

    QVERIFY(checkLayers(image, "initial"));

    {
        KisProcessingApplicator applicator(image, paintLayer1,
                                           KisProcessingApplicator::NONE);

        KisProcessingVisitorSP visitor =
            new KisCropProcessingVisitor(cropRect1, true, false);
        applicator.applyVisitor(visitor);
        applicator.end();
        image->waitForDone();
    }

    QVERIFY(checkLayers(image, "crop_l1"));

    {
        KisProcessingApplicator applicator(image, paintLayer2,
                                           KisProcessingApplicator::NONE);

        KisProcessingVisitorSP visitor =
            new KisCropProcessingVisitor(cropRect2, true, false);
        applicator.applyVisitor(visitor);
        applicator.end();
        image->waitForDone();
    }

    QVERIFY(checkLayers(image, "crop_l2"));

    undoStore->undo();
    image->waitForDone();
    QVERIFY(checkLayers(image, "crop_l1"));

    undoStore->undo();
    image->waitForDone();
    QVERIFY(checkLayers(image, "initial"));
}

void KisProcessingApplicatorTest::testRecursiveProcessing()
{
    KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
    KisPaintLayerSP paintLayer1;
    KisPaintLayerSP paintLayer2;
    KisImageSP image = createImage(undoStore, paintLayer1, paintLayer2);

    QRect cropRect1(40,40,86,86);

    QVERIFY(checkLayers(image, "recursive_initial"));

    {
        KisProcessingApplicator applicator(image, image->rootLayer(),
                                           KisProcessingApplicator::RECURSIVE);

        KisProcessingVisitorSP visitor =
            new KisCropProcessingVisitor(cropRect1, true, true);
        applicator.applyVisitor(visitor);
        applicator.end();
        image->waitForDone();
    }

    QVERIFY(checkLayers(image, "recursive_crop"));

    undoStore->undo();
    image->waitForDone();
    QVERIFY(checkLayers(image, "recursive_initial"));
}

void KisProcessingApplicatorTest::testNoUIUpdates()
{
    KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
    KisPaintLayerSP paintLayer1;
    KisPaintLayerSP paintLayer2;
    KisImageSP image = createImage(undoStore, paintLayer1, paintLayer2);
    QSignalSpy uiSignalsCounter(image.data(), SIGNAL(sigImageUpdated(QRect)));

    QRect cropRect1(40,40,86,86);

    {
        KisProcessingApplicator applicator(image, image->rootLayer(),
                                           KisProcessingApplicator::RECURSIVE |
                                           KisProcessingApplicator::NO_UI_UPDATES);

        KisProcessingVisitorSP visitor =
            new KisCropProcessingVisitor(cropRect1, true, true);
        applicator.applyVisitor(visitor);
        applicator.end();
        image->waitForDone();
    }

    QCOMPARE(uiSignalsCounter.size(), 0);

    uiSignalsCounter.clear();

    undoStore->undo();
    image->waitForDone();

    QCOMPARE(uiSignalsCounter.size(), 0);
}

QTEST_MAIN(KisProcessingApplicatorTest)
