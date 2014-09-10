/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_warp_transform_worker_test.h"

#include <qtest_kde.h>
#include "testutil.h"

#include "kis_warptransform_worker.h"


void KisWarpTransformWorkerTest::test()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
//    QImage image(TestUtil::fetchDataFileLazy("test_transform_quality.png"));
    QImage image(TestUtil::fetchDataFileLazy("test_transform_quality_second.png"));

    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(image, 0);

    QVector<QPointF> origPoints;
    QVector<QPointF> transfPoints;
    qreal alpha = 1.0;

    QRectF bounds(dev->exactBounds());

    origPoints << bounds.topLeft();
    origPoints << bounds.topRight();
    origPoints << bounds.bottomRight();
    origPoints << bounds.bottomLeft();

    origPoints << 0.5 * (bounds.bottomLeft() + bounds.bottomRight());
    origPoints << 0.5 * (bounds.bottomLeft() + bounds.bottomRight()) + QPointF(-20, 0);


    transfPoints << bounds.topLeft();
    transfPoints << bounds.bottomLeft() + 0.6 * (bounds.topRight() - bounds.bottomLeft());
    transfPoints << bounds.topLeft() + 0.8 * (bounds.bottomRight() - bounds.topLeft());
    transfPoints << bounds.bottomLeft() + QPointF(200, 0);

    transfPoints << 0.5 * (bounds.bottomLeft() + bounds.bottomRight()) + QPointF(40,20);
    transfPoints << 0.5 * (bounds.bottomLeft() + bounds.bottomRight()) + QPointF(-20, 0) + QPointF(-40,20);


    KisWarpTransformWorker worker(KisWarpTransformWorker::RIGID_TRANSFORM,
                                  dev,
                                  origPoints,
                                  transfPoints,
                                  alpha,
                                  updater);
    worker.run();

    QImage result = dev->convertToQImage(0);

    TestUtil::checkQImage(result, "warp_trasnform_test", "simple", "tr");
}

QTEST_KDEMAIN(KisWarpTransformWorkerTest, GUI)
