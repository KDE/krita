/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt boud @valdyas.org
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_filter_test.h"

#include <simpletest.h>
#include "filter/kis_filter_configuration.h"
#include "filter/kis_filter_registry.h"
#include "kis_selection.h"
#include "kis_processing_information.h"
#include "filter/kis_filter.h"
#include <testutil.h>
#include "kis_pixel_selection.h"
#include <KisGlobalResourcesInterface.h>

#include <KoProgressUpdater.h>
#include <KoUpdater.h>
#include "testing_timed_default_bounds.h"

class TestFilter : public KisFilter
{
public:

    TestFilter()
            : KisFilter(KoID("test", "test"), KoID("test", "test"), "TestFilter") {
    }

    void processImpl(KisPaintDeviceSP src,
                     const QRect& size,
                     const KisFilterConfigurationSP config,
                     KoUpdater* progressUpdater) const override {
        Q_UNUSED(src);
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

    QImage qimage(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");
    QImage inverted(QString(FILES_DATA_DIR) + '/' + "inverted_hakonepa.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(qimage, 0, 0, 0);
    dev->setDefaultBounds(new TestUtil::TestingTimedDefaultBounds(dev->exactBounds()));

    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);

    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(kfc);

    f->process(dev, QRect(QPoint(0,0), qimage.size()), kfc->cloneWithResourcesSnapshot(), updater);

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, inverted, dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("filtertest.png");
        QFAIL(QString("Failed to create inverted image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
    delete pu;
    delete bar;
}

void KisFilterTest::testSingleThreaded()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QImage qimage(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");
    QImage inverted(QString(FILES_DATA_DIR) + '/' + "inverted_hakonepa.png");
    KisPaintDeviceSP dev = new KisPaintDevice(cs);
    dev->convertFromQImage(qimage, 0, 0, 0);
    dev->setDefaultBounds(new TestUtil::TestingTimedDefaultBounds(dev->exactBounds()));

    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);

    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(kfc);

    f->process(dev, QRect(QPoint(0,0), qimage.size()), kfc->cloneWithResourcesSnapshot());

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, inverted, dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        dev->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("filtertest.png");
        QFAIL(QString("Failed to create inverted image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
    }
}

void KisFilterTest::testDifferentSrcAndDst()
{
    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();

    QImage qimage(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");
    QImage inverted(QString(FILES_DATA_DIR) + '/' + "inverted_hakonepa.png");
    KisPaintDeviceSP src = new KisPaintDevice(cs);
    KisPaintDeviceSP dst = new KisPaintDevice(cs);
    KisSelectionSP sel = new KisSelection(new KisSelectionDefaultBounds(src));
    sel->pixelSelection()->invert(); // select everything
    sel->updateProjection();

    src->convertFromQImage(qimage, 0, 0, 0);
    src->setDefaultBounds(new TestUtil::TestingTimedDefaultBounds(src->exactBounds()));


    KisFilterSP f = KisFilterRegistry::instance()->value("invert");
    Q_ASSERT(f);

    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(kfc);

    f->process(src, dst, sel, QRect(QPoint(0,0), qimage.size()), kfc->cloneWithResourcesSnapshot());

    QPoint errpoint;
    if (!TestUtil::compareQImages(errpoint, inverted, dst->convertToQImage(0, 0, 0, qimage.width(), qimage.height()))) {
        dst->convertToQImage(0, 0, 0, qimage.width(), qimage.height()).save("filtertest.png");
        QFAIL(QString("Failed to create inverted image, first different pixel: %1,%2 ").arg(errpoint.x()).arg(errpoint.y()).toLatin1());
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
    src->setDefaultBounds(new TestUtil::TestingTimedDefaultBounds(src->exactBounds()));


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
    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(kfc);

    /**
     * This filter reads from oldRawData, so if we have some
     * weirdness with transactions it will read from old and non-cleared
     * version of the device and we will see a black square instead
     * of empty device in tmp
     */
    f->process(dst, tmp, 0, updateRect, kfc->cloneWithResourcesSnapshot());

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

void KisFilterTest::testBlurFilterApplicationRect()
{
    QRect filterRect(10,10,40,40);
    QRect src1Rect(5,5,50,50);
    QRect src2Rect(0,0,60,60);

    const KoColorSpace * cs = KoColorSpaceRegistry::instance()->rgb8();
    quint8 *whitePixel = new quint8[cs->pixelSize()];
    cs->fromQColor(Qt::white, whitePixel);
    cs->setOpacity(whitePixel, OPACITY_OPAQUE_U8, 1);

    KisPaintDeviceSP src1 = new KisPaintDevice(cs);
    src1->fill(src1Rect.left(),src1Rect.top(),src1Rect.width(),src1Rect.height(), whitePixel);
    src1->setDefaultBounds(new TestUtil::TestingTimedDefaultBounds(src1->exactBounds()));

    KisPaintDeviceSP src2 = new KisPaintDevice(cs);
    src2->fill(src2Rect.left(),src2Rect.top(),src2Rect.width(),src2Rect.height(), whitePixel);
    src2->setDefaultBounds(new TestUtil::TestingTimedDefaultBounds(src2->exactBounds()));


    KisPaintDeviceSP dst1 = new KisPaintDevice(cs);
    KisPaintDeviceSP dst2 = new KisPaintDevice(cs);

    KisFilterSP f = KisFilterRegistry::instance()->value("blur");
    Q_ASSERT(f);
    KisFilterConfigurationSP  kfc = f->defaultConfiguration(KisGlobalResourcesInterface::instance());
    Q_ASSERT(kfc);

    f->process(src1, dst1, 0, filterRect, kfc->cloneWithResourcesSnapshot());
    f->process(src2, dst2, 0, filterRect, kfc->cloneWithResourcesSnapshot());

    KisPaintDeviceSP reference = new KisPaintDevice(cs);
    reference->fill(filterRect.left(),filterRect.top(),filterRect.width(),filterRect.height(), whitePixel);

    QImage refImage = reference->convertToQImage(0,10,10,40,40);
    QImage dst1Image = dst1->convertToQImage(0,10,10,40,40);
    QImage dst2Image = dst2->convertToQImage(0,10,10,40,40);

    //dst1Image.save("DST1.png");
    //dst2Image.save("DST2.png");

    QPoint pt;
    QVERIFY(TestUtil::compareQImages(pt, refImage, dst1Image));
    QVERIFY(TestUtil::compareQImages(pt, refImage, dst2Image));
}


SIMPLE_TEST_MAIN(KisFilterTest)
