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
    KisImageWSP image = new KisImage(0, 10, 10, cs, "bla");
    KisFilterStrategy * filter = new KisBoxFilterStrategy();
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    KisTransformWorker tw(dev, 1.0, 1.0, 1.0, 1.0, 1.5, 0, 0, updater, filter, true);
}

void KisTransformWorkerTest::testMirror()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace("RGBA", 0);
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    quint8* pixel = cs->allocPixelBuffer(1);
    cs->fromQColor(Qt::white, pixel);
    dev->fill(0, 0, 512, 512, pixel);

    cs->fromQColor(Qt::black, pixel);
    dev->fill(512, 0, 512, 512, pixel);

    QColor c1;
    dev->pixel(5, 5, &c1);

    QColor c2;
    dev->pixel(517, 5, &c2);

    QVERIFY(c1 == Qt::white);
    QVERIFY(c2 == Qt::black);
    dev->convertToQImage(0, 0, 0, 1024, 512).save("before.png");

    KisTransformWorker::mirrorX(dev);
    dev->convertToQImage(0, 0, 0, 1024, 512).save("mirror_x.png");

    dev->pixel(5, 5, &c1);
    dev->pixel(517, 5, &c2);

    QVERIFY(c1 == Qt::black);
    QVERIFY(c2 == Qt::white);

    cs->fromQColor(Qt::white, pixel);
    dev->fill(0, 0, 512, 512, pixel);

    cs->fromQColor(Qt::black, pixel);
    dev->fill(0, 512, 512, 512, pixel);

    dev->pixel(5, 5, &c1);
    dev->pixel(5, 517, &c2);

    QVERIFY(c1 == Qt::white);
    QVERIFY(c2 == Qt::black);

    KisTransformWorker::mirrorY(dev);
    dev->convertToQImage(0, 0, 0, 1024, 512).save("mirror_y.png");

    dev->pixel(5, 5, &c1);
    dev->pixel(5, 517, &c2);

    QVERIFY(c1 == Qt::black);
    QVERIFY(c2 == Qt::white);

    {
        QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
        KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
        dev2->convertFromQImage(image, "");
        KisTransformWorker::mirrorX(dev2);
        KisTransformWorker::mirrorX(dev2);
        KisTransformWorker::mirrorX(dev2);
        dev2->convertToQImage(0, 0, 0, image.width(), image.height()).save("mirror_test2.png");
    }
}

void KisTransformWorkerTest::testMirrorTransaction()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->colorSpace("RGBA", 0);
    KisPaintDeviceSP dev = new KisPaintDevice(cs);

    quint8* pixel = cs->allocPixelBuffer(1);
    cs->fromQColor(Qt::white, pixel);
    dev->fill(0, 0, 512, 512, pixel);

    cs->fromQColor(Qt::black, pixel);
    dev->fill(512, 0, 512, 512, pixel);

    QColor c1;
    QColor c2;

    dev->pixel(5, 5, &c1);
    dev->pixel(517, 5, &c2);

    QVERIFY(c1 == Qt::white);
    QVERIFY(c2 == Qt::black);
    dev->convertToQImage(0, 0, 0, 1024, 512).save("transaction_before.png");

    KisTransaction t("mirror", dev, 0);
    KisTransformWorker::mirrorX(dev);

    dev->pixel(5, 5, &c1);
    dev->pixel(517, 5, &c2);

    dev->convertToQImage(0, 0, 0, 1024, 512).save("transaction_mirror_x.png");
    QVERIFY(c1 == Qt::black);
    QVERIFY(c2 == Qt::white);

    cs->fromQColor(Qt::white, pixel);
    dev->fill(0, 0, 512, 512, pixel);

    cs->fromQColor(Qt::black, pixel);
    dev->fill(0, 512, 512, 512, pixel);

    dev->pixel(5, 5, &c1);
    dev->pixel(5, 517, &c2);

    QVERIFY(c1 == Qt::white);
    QVERIFY(c2 == Qt::black);

    KisTransaction t2("mirror", dev, 0);
    KisTransformWorker::mirrorY(dev);
    dev->convertToQImage(0, 0, 0, 1024, 512).save("transaction_mirror_y.png");

    dev->pixel(5, 5, &c1);
    dev->pixel(5, 517, &c2);

    QVERIFY(c1 == Qt::black);
    QVERIFY(c2 == Qt::white);


    {
        QImage image(QString(FILES_DATA_DIR) + QDir::separator() + "mirror_source.png");
        KisPaintDeviceSP dev2 = new KisPaintDevice(cs);
        dev2->convertFromQImage(image, "");
        KisTransaction t("mirror", dev2, 0);
        KisTransformWorker::mirrorX(dev2);
        dev2->convertToQImage(0, 0, 0, image.width(), image.height()).save("mirror_test_t_2.png");
    }
}


QTEST_KDEMAIN(KisTransformWorkerTest, GUI)
#include "kis_transform_worker_test.moc"
