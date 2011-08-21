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

#include "kis_processings_test.h"

#include <qtest_kde.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include "kis_image.h"
#include "kis_selection.h"
#include "kis_paint_layer.h"
#include "kis_adjustment_layer.h"
#include "kis_transparency_mask.h"
#include "kis_clone_layer.h"

#include "filter/kis_filter.h"
#include "filter/kis_filter_registry.h"

#include "kis_undo_stores.h"
#include "kis_processing_applicator.h"
#include "processing/kis_crop_processing_visitor.h"

#include <KoProgressUpdater.h>
#include "testutil.h"
#include "kis_filter_strategy.h"
#include "kis_transform_worker.h"
#include "processing/kis_transform_processing_visitor.h"


class BaseProcessingTest
{
public:
    BaseProcessingTest()
    {
    }

    void test(const QString &testname, KisProcessingVisitorSP visitor) {
        KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
        KisImageSP image = createImage(undoStore);
        image->refreshGraph();

        QVERIFY(checkLayers(image, "initial"));

        KisProcessingApplicator applicator(image, image->root(), true);
        applicator.applyVisitor(visitor);
        applicator.end();
        image->waitForDone();

        QVERIFY(checkLayers(image, testname));

        undoStore->undo();
        image->waitForDone();

        QVERIFY(checkLayers(image, "initial"));
    }

private:
    KisImageSP createImage(KisSurrogateUndoStore *undoStore) {
        QImage sourceImage(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");

        QRect imageRect = QRect(QPoint(0,0), sourceImage.size());

        QRect transpRect(50,50,300,300);
        QRect blurRect(66,66,300,300);
        QPoint blurShift(34,34);
        QPoint cloneShift(75,75);

        const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
        KisImageSP image = new KisImage(undoStore, imageRect.width(), imageRect.height(), cs, "merge test");

        KisFilterSP filter = KisFilterRegistry::instance()->value("blur");
        Q_ASSERT(filter);
        KisFilterConfiguration *configuration = filter->defaultConfiguration(0);
        Q_ASSERT(configuration);

        KisAdjustmentLayerSP blur1 = new KisAdjustmentLayer(image, "blur1", configuration, 0);
        blur1->selection()->clear();
        blur1->selection()->getOrCreatePixelSelection()->select(blurRect);
        blur1->setX(blurShift.x());
        blur1->setY(blurShift.y());

        KisPaintLayerSP paintLayer1 = new KisPaintLayer(image, "paint1", OPACITY_OPAQUE_U8);
        paintLayer1->paintDevice()->convertFromQImage(sourceImage, "", 0, 0);

        KisTransparencyMaskSP transparencyMask1 = new KisTransparencyMask();
        transparencyMask1->setName("tmask1");
        transparencyMask1->selection()->clear();
        transparencyMask1->selection()->getOrCreatePixelSelection()->select(transpRect);

        KisCloneLayerSP cloneLayer1 =
            new KisCloneLayer(paintLayer1, image, "clone1", OPACITY_OPAQUE_U8);
        cloneLayer1->setX(cloneShift.x());
        cloneLayer1->setY(cloneShift.y());

        image->addNode(cloneLayer1);
        image->addNode(blur1);
        image->addNode(paintLayer1);
        image->addNode(transparencyMask1, paintLayer1);

        return image;
    }

    void fillNamesImages(KisNodeSP node, const QRect &rc,
                         QVector<QImage> &images,
                         QVector<QString> &names) {

        while (node) {
            if(node->original()) {
                names.append(node->name() + "_original");
                images.append(node->original()->
                              convertToQImage(0, rc.x(), rc.y(),
                                              rc.width(), rc.height()));
            }

            if(node->projection()) {
                names.append(node->name() + "_projection");
                images.append(node->projection()->
                              convertToQImage(0, rc.x(), rc.y(),
                                              rc.width(), rc.height()));
            }

            if(node->paintDevice()) {
                names.append(node->name() + "_paintDevice");
                images.append(node->paintDevice()->
                              convertToQImage(0, rc.x(), rc.y(),
                                              rc.width(), rc.height()));
            }

            fillNamesImages(node->firstChild(), rc, images, names);
            node = node->nextSibling();
        }
    }

    bool checkLayers(KisImageWSP image, const QString &prefix) {
        QVector<QImage> images;
        QVector<QString> names;

        fillNamesImages(image->root(), image->bounds(), images, names);

        bool valid = true;

        const int stackSize = images.size();
        for(int i = 0; i < stackSize; i++) {
            QString name = prefix + "_" + names[i] + ".png";

            QImage ref(QString(FILES_DATA_DIR) + QDir::separator() +
                       "processings" + QDir::separator() +
                       prefix + QDir::separator() + name);

            if(ref != images[i]) {
                qDebug() << "--- Wrong image:" << name;
                valid = false;
                images[i].save(QString(FILES_OUTPUT_DIR) + QDir::separator() + name);
            }
        }

        return valid;
    }
};

void KisProcessingsTest::testCropVisitor()
{
    KisProcessingVisitorSP visitor =
        new KisCropProcessingVisitor(QRect(45,45,410,410), true);

    BaseProcessingTest tester;
    tester.test("crop", visitor);
}

void KisProcessingsTest::testTransformVisitorScale()
{
    BaseProcessingTest tester;

    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();
    KisFilterStrategy * filter = new KisBoxFilterStrategy();

    KisProcessingVisitorSP visitor =
        new KisTransformProcessingVisitor(0.5, 0.5,
                                          0,0,QPointF(),
                                          0,
                                          0,0,
                                          updater,filter);

    tester.test("transform_scale", visitor);
}

QTEST_KDEMAIN(KisProcessingsTest, GUI)
