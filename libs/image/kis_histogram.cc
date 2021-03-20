/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt
 *  SPDX-FileCopyrightText: 2005 Bart Coppens <kde@bartcoppens.be>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_histogram.h"

#include <QVector>

#include "kis_image.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
#include "KoColorSpace.h"
#include "kis_debug.h"
#include "kis_iterator_ng.h"

KisHistogram::KisHistogram(const KisPaintLayerSP layer,
                           KoHistogramProducer *producer,
                           const enumHistogramType type)
    : m_paintDevice(layer->projection())
{
    Q_ASSERT(producer);

    KisImageSP imageSP = layer->image().toStrongRef();
    if (imageSP) {
        m_bounds = imageSP->bounds();
    }
    m_type = type;
    m_producer = producer;
    m_selection = false;
    m_channel = 0;

    updateHistogram();
}

KisHistogram::KisHistogram(const KisPaintDeviceSP paintdev,
                           const QRect &bounds,
                           KoHistogramProducer *producer,
                           const enumHistogramType type)
    : m_paintDevice(paintdev)
{
    Q_ASSERT(producer);

    m_bounds = bounds;
    m_producer = producer;
    m_type = type;

    m_selection = false;
    m_channel = 0;

    // TODO: Why does Krita crash when updateHistogram() is *not* called here?
    updateHistogram();
}

KisHistogram::~KisHistogram()
{
    delete m_producer;
}

void KisHistogram::updateHistogram()
{
    if (m_bounds.isEmpty()) {
        int numChannels = m_producer->channels().count();

        m_completeCalculations.clear();
        m_completeCalculations.resize(numChannels);

        m_completeCalculations.clear();
        m_completeCalculations.resize(numChannels);

        return;
    }

    KisSequentialConstIterator srcIt(m_paintDevice, m_bounds);
    const KoColorSpace* cs = m_paintDevice->colorSpace();

    // Let the producer do it's work
    m_producer->clear();


    // XXX: the original code depended on their being a selection mask in the iterator
    //      if the paint device had a selection. When we changed that to passing an
    //      explicit selection to the createRectIterator call, that broke because
    //      paint devices didn't know about their selections anymore.
    //      updateHistogram should get a selection parameter.
    int numConseqPixels = srcIt.nConseqPixels();
    while (srcIt.nextPixels(numConseqPixels)) {

        numConseqPixels = srcIt.nConseqPixels();
        m_producer->addRegionToBin(srcIt.oldRawData(), 0, numConseqPixels, cs);
    }

    computeHistogram();
}

void KisHistogram::computeHistogram()
{
    if (!m_producer) return;

    m_completeCalculations = calculateForRange(m_producer->viewFrom(),
                             m_producer->viewFrom() + m_producer->viewWidth());

    if (m_selection) {
        m_selectionCalculations = calculateForRange(m_selFrom, m_selTo);
    } else {
        m_selectionCalculations.clear();
    }

#if 1
    dump();
#endif
}

KisHistogram::Calculations KisHistogram::calculations()
{
    return m_completeCalculations.at(m_channel);
}

KisHistogram::Calculations KisHistogram::selectionCalculations()
{
    return m_selectionCalculations.at(m_channel);
}

QVector<KisHistogram::Calculations> KisHistogram::calculateForRange(double from, double to)
{
    QVector<Calculations> calculations;
    if (m_producer) {
        uint count = m_producer->channels().count();

        for (uint i = 0; i < count; i++) {
            calculations.append(calculateSingleRange(i, from, to));
        }
    }
    return calculations;
}

KisHistogram::Calculations KisHistogram::calculateSingleRange(int channel, double from, double to)
{
    Calculations c;

    // XXX If from == to, we only want a specific bin, handle that properly!

    double max = from, min = to, total = 0.0, mean = 0.0; //, median = 0.0, stddev = 0.0;
    quint32 high = 0, low = (quint32) - 1, count = 0;

    if (m_producer->count() == 0) {
        // We won't get anything, even if a range is specified
        // XXX make sure all initial '0' values are correct here!
        return c;
    }

    qint32 totbins = m_producer->numberOfBins();
    quint32 current;

    // convert the double range into actual bins:
    double factor = static_cast<double>(totbins) / m_producer->viewWidth();

    qint32 fromBin = static_cast<qint32>((from - m_producer->viewFrom()) * factor);
    qint32 toBin = fromBin + static_cast<qint32>((to - from) * factor);

    // Min, max, count, low, high
    for (qint32 i = fromBin; i < toBin; i++) {
        current = m_producer->getBinAt(channel, i);
        double pos = static_cast<double>(i) / factor + from;
        if (current > high)
            high = current;
        if (current < low)
            low = current;
        if (current > 0) {
            if (pos < min)
                min = pos;
            if (pos > max)
                max = pos;
        }
        // We do the count here as well.
        // we can't use m_producer->count() for this, because of the range
        count += current;
        total += current * pos;
    }

    if (count > 0)
        mean = total / count;

    c.m_high = high;
    c.m_low = low;
    c.m_count = count;
    c.m_min = min;
    c.m_max = max;
    c.m_mean = mean;
    c.m_total = total;

    return c;
}


void KisHistogram::dump()
{
    dbgMath << "Histogram";

    switch (m_type) {
    case LINEAR:
        dbgMath << "Linear histogram";
        break;
    case LOGARITHMIC:
        dbgMath << "Logarithmic histogram";
    }

    dbgMath << "Dumping channel" << m_channel;
    Calculations c = calculations();

    /*        for( int i = 0; i <256; ++i ) {
            dbgMath <<"Value"
                  << QString().setNum(i)
                  << ": "
                  <<  QString().setNum(m_values[i])
                  << "\n";
            }*/
    dbgMath << "";

    dbgMath << "Max:" << QString().setNum(c.getMax()) << "";
    dbgMath << "Min:" << QString().setNum(c.getMin()) << "";
    dbgMath << "High:" << QString().setNum(c.getHighest()) << "";
    dbgMath << "Low:" << QString().setNum(c.getLowest()) << "";
    dbgMath << "Mean:" << m_producer->positionToString(c.getMean()) << "";
    dbgMath << "Total:" << QString().setNum(c.getTotal()) << "";
    //    dbgMath <<"Median:" << QString().setNum(m_median) <<"";
    //    dbgMath <<"Stddev:" << QString().setNum(m_stddev) <<"";
    //    dbgMath <<"percentile:" << QString().setNum(m_percentile) <<"";

    dbgMath << "";
}
