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

#include "kis_filter_test.h"

#include <qtest_kde.h>
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_selection.h"
#include "kis_processing_information.h"
#include "filter/kis_filter.h"
#include "testutil.h"
#include "kis_threaded_applicator.h"
#include "kis_selection.h"
#include "kis_pixel_selection.h"

#include <KoUpdater.h>

class TestFilter : public KisFilter
{
public:

    TestFilter()
            : KisFilter(KoID("test", "test"), KoID("test", "test"), "TestFilter") {
    }

    using KisFilter::process;

    void process(KisConstProcessingInformation src,
                 KisProcessingInformation dst,
                 const QSize& size,
                 const KisFilterConfiguration* config,
                 KoUpdater* progressUpdater) const {
        Q_UNUSED(src);
        Q_UNUSED(dst);
        Q_UNUSED(size);
        Q_UNUSED(config);
        Q_UNUSED(progressUpdater);
    }

};

void KisFilterTest::testCreation()
{
    TestFilter test;
}

void KisFilterTest::testWithProgressUpdater()
{
    TestUtil::TestProgressBar * bar = new TestUtil::TestProgressBar();
    KoProgressUpdater* pu = new KoProgressUpdater(bar);
    KoUpdaterPtr updater = pu->startSubtask();

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    QImage inverted(QString(FILES_DATA_DIR) + QDir::separator() + "inverted_hakonepa.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(qimage, "", 0, 0);

    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);

    KisFilterConfiguration * kfc = f->defaultConfiguration(0);
    Q_ASSERT(kfc);

    KisConstProcessingInformation src(dev,  QPoint(0, 0), 0);
    KisProcessingInformation dst(dev, QPoint(0, 0), 0);

    f->process(src, dst, qimage.size(), kfc, updater);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, inverted, dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("filtertest.png");
        QFAIL(QString("Failed to create inverted image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
    delete pu;
    delete bar;
}

void KisFilterTest::testSingleThreaded()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    QImage inverted(QString(FILES_DATA_DIR) + QDir::separator() + "inverted_hakonepa.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(qimage, "", 0, 0);

    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);

    KisFilterConfiguration * kfc = f->defaultConfiguration(0);
    Q_ASSERT(kfc);

    KisConstProcessingInformation src(dev,  QPoint(0, 0), 0);
    KisProcessingInformation dst(dev, QPoint(0, 0), 0);

    f->process(src, dst, qimage.size(), kfc);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, inverted, dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("filtertest.png");
        QFAIL(QString("Failed to create inverted image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisFilterTest::testDifferentSrcAndDst()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QImage qimage(QString(FILES_DATA_DIR) + QDir::separator() + "hakonepa.png");
    QImage inverted(QString(FILES_DATA_DIR) + QDir::separator() + "inverted_hakonepa.png");
    KisPaintDeviceSP src = new KisPaintDevice(cs);
    KisPaintDeviceSP dst = new KisPaintDevice(cs);
    KisSelectionSP sel = new KisSelection(src);
    sel->getOrCreatePixelSelection()->invert(); // select everything
    sel->updateProjection();

    src->convertFromQImage(qimage, "", 0, 0);

    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);

    KisFilterConfiguration * kfc = f->defaultConfiguration(0);
    Q_ASSERT(kfc);

    KisConstProcessingInformation srcCfg(src,  QPoint(0, 0), sel);
    KisProcessingInformation dstCfg(dst, QPoint(0, 0), sel);

    f->process(srcCfg, dstCfg, qimage.size(), kfc);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, inverted, dst->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        dst->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("filtertest.png");
        QFAIL(QString("Failed to create inverted image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toAscii());
    }
}

void KisFilterTest::testOldDataApiAfterCopy()
{
    QRect updateRect(0,0,63,63);

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    quint8 *whitePixel = new quint8[cs->pixelSize()];
    cs->fromQColor(Qt::white, whitePixel);
    cs->setOpacity(whitePixel, OPACITY_OPAQUE_U8, 1);

    KisPaintDeviceSP tmp = new KisPaintDevice(cs);

    KisPaintDeviceSP src = new KisPaintDevice(cs);
    src->fill(0, 0, 50, 50, whitePixel);

    /**
     * Make a full copy here to catch the bug.
     * Buggy memento manager would make a commit
     * that is not good.
     */
    KisPaintDeviceSP dst = new KisPaintDevice(*src);

    /**
     * This would write go to a new revision in a buggy
     * memento manager
     */
    dst->clear(updateRect);

    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);
    KisFilterConfiguration * kfc = f->defaultConfiguration(0);
    Q_ASSERT(kfc);

    KisConstProcessingInformation srcCfg(dst,  updateRect.topLeft(), 0);
    KisProcessingInformation dstCfg(tmp, updateRect.topLeft(), 0);


    /**
     * This filter reads from oldRawData, so if we have some
     * weirdness with transactions it will read from old and non-cleared
     * version of the device and we will see a black square instead
     * of empty device in tmp
     */
    f->process(srcCfg, dstCfg, updateRect.size(), kfc);

    /**
     * In theory, both devices: dst and tmp must be empty by now
     */
    KisPaintDeviceSP reference = new KisPaintDevice(cs);

    QImage refImage = reference->convertToQImage(0,0,0,63,63);
    QImage dstImage = dst->convertToQImage(0,0,0,63,63);
    QImage tmpImage = tmp->convertToQImage(0,0,0,63,63);

    QPoint pt;
    QVERIFY(TestUtil::compareQImages(pt, refImage, dstImage));
    QVERIFY(TestUtil::compareQImages(pt, refImage, tmpImage));

}


QTEST_KDEMAIN(KisFilterTest, GUI)
#include "kis_filter_test.moc"
