/*
 *  Copyright (c) 2007 Boudewijn Rempt boud@valdyas.org
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

#include "kis_transform_worker_test.h"

#include <qtest_kde.h>
#include <KoProgressUpdater.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include "kis_types.h"
#include "kis_image.h"
#include "kis_filter_strategy.h"
#include "kis_paint_device.h"
#include "kis_transform_worker.h"
#include "testutil.h"
#include "kis_transaction.h"

void KisTransformWorkerTest::testCreation()
{
    TestUtil::TestProgressBar bar;
    KoProgressUpdater pu(&bar);
    KoUpdaterPtr updater = pu.startSubtask();
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(0, 10, 10, cs, "bla");
    KisFilterStrategy * filter = new KisBoxFilterStrategy();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    KisTransformWorker tw(dev, 1.0, 1.0,
                          1.0, 1.0,
                          0.0, 0.0,
                          1.5,
                          0, 0, updater, filter, true);
}

void KisTransformWorkerTest::testMirrorX()
{

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, "");
    KisTransformWorker::mirrorX(dev2);
    KisTransformWorker::mirrorX(dev2);
    KisTransformWorker::mirrorX(dev2);
    QImage result = dev2->convertToQImage(0, 0, 0, image.width(), image.height());
    image = image.mirrored(true, false);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        // They are the same, but should be mirrored
        image.save("mirror_test_1_source.png");
        result.save("mirror_test_1_result.png");
        QFAIL(QString("Failed to mirror the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

}

void KisTransformWorkerTest::testMirrorY()
{

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, "");
    KisTransformWorker::mirrorY(dev2);
    KisTransformWorker::mirrorY(dev2);
    KisTransformWorker::mirrorY(dev2);
    QImage result = dev2->convertToQImage(0, 0, 0, image.width(), image.height());
    image = image.mirrored(false, true);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        // They are the same, but should be mirrored
        image.save("mirror_test_2_source.png");
        result.save("mirror_test_2_result.png");
        QFAIL(QString("Failed to mirror the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }

}

void KisTransformWorkerTest::testMirrorTransactionX()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, "");

    KisTransaction t("mirror", dev2);
    KisTransformWorker::mirrorX(dev2);
    t.end();

    QImage result = dev2->convertToQImage(0, 0, 0, image.width(), image.height());

    image = image.mirrored(true, false);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        // They are the same, but should be mirrored
        image.save("mirror_test_3_source.png");
        result.save("mirror_test_3_result.png");
        QFAIL(QString("Failed to mirror the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());

    }
}
void KisTransformWorkerTest::testMirrorTransactionY()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
    KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
    dev2->convertFromQImage(image, "");

    KisTransaction t("mirror", dev2);
    KisTransformWorker::mirrorY(dev2);
    t.end();

    QImage result = dev2->convertToQImage(0, 0, 0, image.width(), image.height());

    image = image.mirrored(false, true);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, image, result)) {
        // They are the same, but should be mirrored
        image.save("mirror_test_4_source.png");
        result.save("mirror_test_4_result.png");
        QFAIL(QString("Failed to mirror the image, first different pixel: %1,%2 \n").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }


}


QTEST_KDEMAIN(KisTransformWorkerTest, GUI)
#include "kis_transform_worker_test.moc"
