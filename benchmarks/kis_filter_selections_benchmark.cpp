/*
 *  SPDX-FileCopyrightText: 2009 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_filter_selections_benchmark.h"

#include "kis_painter.h"

#include <simpletest.h>
#include <testutil.h>
#include "kis_transaction.h"
#include <KoCompositeOpRegistry.h>
#include "kis_datamanager.h"
#include <KisGlobalResourcesInterface.h>

#define NUM_CYCLES 50
#define WARMUP_CYCLES 2
#define SHOW_WARMUPS  0

/**
 * Our filters don't know anything about applyAlphaU8Mask
 * That's why they treat semy-selected pixels badly
 *
 * If you have a hack in KisFilter - processSpecial(..) method
 * that takes into account this alpha mask then activate
 * the following define
 */
#define USE_GOOD_SELECTIONS 0

#define USE_UTIME 0


#if(USE_UTIME==1)
#include <sys/times.h>
class KisTimeCounter
{
public:
    KisTimeCounter() {
        m_factor = double(sysconf(_SC_CLK_TCK)) / 1000.0;
        restart();
    }

    void restart() {
        times(&m_startTime);
    }

    double elapsed() {
        struct tms endTime;
        times(&endTime);
        return double(endTime.tms_utime - m_startTime.tms_utime) / m_factor;
    }

private:
    struct tms m_startTime;
    double m_factor;
};
#else /* if(USE_UTIME==0) */
typedef QElapsedTimer KisTimeCounter;
#endif

void KisFilterSelectionsBenchmark::initSelection()
{
    m_selection = new KisSelection();
    KisPixelSelectionSP pixelSelection = m_selection->pixelSelection();


//67.2% deselected
    dbgKrita << "Deselected: 67.2%";
    pixelSelection->dataManager()->clear(75, 75, 500, 320, 255);
    pixelSelection->dataManager()->clear(100, 100, 50, 50, quint8(0));
    pixelSelection->dataManager()->clear(150, 150, 50, 50, quint8(0));
    pixelSelection->dataManager()->clear(200, 200, 50, 50, quint8(0));

    pixelSelection->dataManager()->clear(375, 195, 200, 200, quint8(0));
    pixelSelection->dataManager()->clear(75, 195, 200, 200, quint8(0));

    pixelSelection->dataManager()->clear(375, 75, 150, 150, quint8(0));

    pixelSelection->dataManager()->clear(205, 105, 50, 50, quint8(128));

// 94.9% deselected
//    dbgKrita << "Deselected: 94.9%";
//    pixelSelection->dataManager()->clear(75,75,500,320,255);
//    pixelSelection->dataManager()->clear(80,80,490,310,quint8(0));



    pixelSelection->convertToQImage(0).save("TEST_FILTER_SELECTION.png");
}

void KisFilterSelectionsBenchmark::initFilter(const QString &name)
{
    m_filter = KisFilterRegistry::instance()->value(name);
    Q_ASSERT(m_filter);
    m_configuration = m_filter->defaultConfiguration(KisGlobalResourcesInterface::instance());

    dbgKrita << "Filter initialized:" << name;
}

void KisFilterSelectionsBenchmark::testFilter(const QString &name)
{
    blockSignals(true);

    initFilter(name);

    testUsualSelections(WARMUP_CYCLES);
    testUsualSelections(NUM_CYCLES);

    testGoodSelections(WARMUP_CYCLES);
    testGoodSelections(NUM_CYCLES);

    testNoSelections(WARMUP_CYCLES);
    testNoSelections(NUM_CYCLES);

    testBitBltWOSelections(WARMUP_CYCLES);
    testBitBltWOSelections(NUM_CYCLES);

    testBitBltSelections(WARMUP_CYCLES);
    testBitBltSelections(NUM_CYCLES);

    blockSignals(false);
}


void KisFilterSelectionsBenchmark::testAll()
{
    initSelection();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->rgb8();
    QImage image(QString(FILES_DATA_DIR) + '/' + "hakonepa.png");
    m_device = new KisPaintDevice(cs);
    m_device->convertFromQImage(image, 0, 0, 0);

    testFilter("brightnesscontrast");
    testFilter("invert");
//    testFilter("levels");

}

void KisFilterSelectionsBenchmark::testUsualSelections(int num)
{
    KisPaintDeviceSP projection =
        new KisPaintDevice(m_device->colorSpace());

    double avTime;
    KisTimeCounter timer;

    QRect filterRect = m_selection->selectedExactRect();

    timer.restart();
    for (int i = 0; i < num; i++) {
        KisTransaction transac(projection, 0);
        m_filter->process(m_device, projection, m_selection, filterRect, m_configuration, 0);
    }
    avTime = double(timer.elapsed()) / num;

    projection->convertToQImage(0).save("TFS__USUAL_SELECTIONS.png");

    if (num > WARMUP_CYCLES || SHOW_WARMUPS)
        dbgKrita << "Selections inside filter:\t\t" << avTime;
}

void KisFilterSelectionsBenchmark::testNoSelections(int num)
{
    KisPaintDeviceSP projection =
        new KisPaintDevice(m_device->colorSpace());

    double avTime;
    KisTimeCounter timer;

    QRect filterRect = m_selection->selectedExactRect();

    timer.restart();
    for (int i = 0; i < num; i++) {
        KisTransaction transac(projection, 0);
        m_filter->process(m_device, projection, 0, filterRect, m_configuration, 0);
    }
    avTime = double(timer.elapsed()) / num;

    projection->convertToQImage(0).save("TFS__NO_SELECTIONS.png");

    if (num > WARMUP_CYCLES || SHOW_WARMUPS)
        dbgKrita << "No Selections:\t\t\t\t" << avTime;
}

void KisFilterSelectionsBenchmark::testGoodSelections(int num)
{
#if(USE_GOOD_SELECTIONS==1)
    KisPaintDeviceSP projection =
        new KisPaintDevice(m_device->colorSpace());

    double avTime;
    KisTimeCounter timer;

    QRect filterRect = m_selection->selectedExactRect();
    KisConstProcessingInformation src(m_device,  filterRect.topLeft(), m_selection);
    KisProcessingInformation dst(projection, filterRect.topLeft(), 0);

    timer.restart();
    for (int i = 0; i < num; i++) {
        KisTransaction transac(0, projection, 0);
        m_filter->processSpecial(src, dst, filterRect.size(), m_configuration, 0);
    }
    avTime = double(timer.elapsed()) / num;

    projection->convertToQImage(0).save("TFS__GOOD_SELECTIONS.png");

    if (num > WARMUP_CYCLES || SHOW_WARMUPS)
        dbgKrita << "Selections with alpha (filter):\t" << avTime;
#else /* if (USE_GOOD_SELECTIONS!=1) */
    if (num > WARMUP_CYCLES || SHOW_WARMUPS)
        dbgKrita << "Selections with alpha (filter):\t [Disabled]";
#endif
}

void KisFilterSelectionsBenchmark::testBitBltWOSelections(int num)
{
    KisPaintDeviceSP projection =
        new KisPaintDevice(m_device->colorSpace());

    double avTime;
    KisTimeCounter timer;

    QRect filterRect = m_selection->selectedExactRect();

    timer.restart();
    for (int i = 0; i < num; i++) {
        KisPaintDeviceSP cacheDevice = new KisPaintDevice(projection->colorSpace());

        KisTransaction transac(cacheDevice, 0);
        m_filter->process(m_device, projection, 0, filterRect, m_configuration, 0);

        KisPainter painter(projection);
        painter.beginTransaction();
        painter.setCompositeOp(projection->colorSpace()->compositeOp(COMPOSITE_ALPHA_DARKEN));
        painter.bitBlt(filterRect.topLeft(), cacheDevice, filterRect);
        painter.deleteTransaction();
    }
    avTime = double(timer.elapsed()) / num;

    projection->convertToQImage(0).save("TFS__BITBLT_WO_SELECTIONS.png");

    if (num > WARMUP_CYCLES || SHOW_WARMUPS)
        dbgKrita << "bitBlt w/o sel:\t\t\t" << avTime;
}

void KisFilterSelectionsBenchmark::testBitBltSelections(int num)
{
    KisPaintDeviceSP projection =
        new KisPaintDevice(m_device->colorSpace());

    double avTime;
    KisTimeCounter timer;

    QRect filterRect = m_selection->selectedExactRect();

    timer.restart();
    for (int i = 0; i < num; i++) {
        KisPaintDeviceSP cacheDevice = new KisPaintDevice(projection->colorSpace());

        KisTransaction transac(cacheDevice, 0);
        m_filter->process(m_device, cacheDevice, 0, filterRect, m_configuration, 0);

        KisPainter gc(projection);
        gc.beginTransaction();
        gc.setCompositeOp(projection->colorSpace()->compositeOp(COMPOSITE_ALPHA_DARKEN));
        gc.setSelection(m_selection);
        gc.bitBlt(filterRect.topLeft(), cacheDevice, filterRect);
        gc.deleteTransaction();
    }
    avTime = double(timer.elapsed()) / num;

    projection->convertToQImage(0).save("TFS__BITBLT_WITH_SELECTIONS.png");

    if (num > WARMUP_CYCLES || SHOW_WARMUPS)
        dbgKrita << "bitBlt with sel:\t\t\t" << avTime;
}

SIMPLE_TEST_MAIN(KisFilterSelectionsBenchmark)
