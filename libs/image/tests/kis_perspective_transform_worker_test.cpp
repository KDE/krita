/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_perspective_transform_worker_test.h"

#include <QTest>

#include "testutil.h"

#define USE_DOCUMENT 0
#include "qimage_based_test.h"

#include "kis_perspectivetransform_worker.h"
#include "kis_transaction.h"


class PerspectiveWorkerTester : public TestUtil::QImageBasedTest
{
public:
    PerspectiveWorkerTester()
        : QImageBasedTest("perspective_worker_test")
    {
        KisSurrogateUndoStore *undoStore = new KisSurrogateUndoStore();
        image = createImage(undoStore);
        image->initialRefreshGraph();

        QVERIFY(checkLayersInitial(image));
    }

    KisPaintDeviceSP paintDevice() {
        return findNode(image->root(), "paint1")->paintDevice();
    }

    void checkLayer(const QString &testName) {
        KisNodeSP node = findNode(image->root(), "paint1");
        QVERIFY(checkOneLayer(image, node, testName, 3));
    }


    KisImageSP image;
};


void KisPerspectiveTransformWorkerTest::testSimpleTransform()
{
    PerspectiveWorkerTester t;
    KisPaintDeviceSP dev = t.paintDevice();

    QPointF dx(326, 214);
    qreal aX = 1.32;
    qreal aY = 0.8;
    qreal z = 1024;

    KisPerspectiveTransformWorker worker(dev, dx, aX, aY, z, 0);
    worker.run();

    t.checkLayer("simple_transform");
}

QTEST_MAIN(KisPerspectiveTransformWorkerTest)
