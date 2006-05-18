/*
 *  Copyright (c) 2004 Boudewijn Rempt
 *            (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include <kdebug.h>
#include <QVector>
#include <QDateTime> // ### Debug

#include "kis_types.h"
#include "kis_histogram.h"
#include "kis_paint_layer.h"
#include "kis_iterators_pixel.h"
#include "kis_colorspace.h"
#include "kis_debug_areas.h"

KisHistogram::KisHistogram(KisPaintLayerSP layer,
                           KisHistogramProducerSP producer,
                           const enumHistogramType type)
{
    m_dev = layer->paintDevice();
    m_type = type;
    m_producer = producer;
    m_selection = false;
    m_channel = 0;

    updateHistogram();
}

KisHistogram::KisHistogram(KisPaintDeviceSP paintdev,
                           KisHistogramProducerSP producer,
                           const enumHistogramType type)
{
    m_dev = paintdev;
    m_type = type;
    m_producer = producer;
    m_selection = false;
    m_channel = 0;

    updateHistogram();
}

KisHistogram::~KisHistogram()
{
}

void KisHistogram::updateHistogram()
{
    qint32 x,y,w,h;
    m_dev->exactBounds(x,y,w,h);
    KisRectIteratorPixel srcIt = m_dev->createRectIterator(x,y,w,h, false);
    KisColorSpace* cs = m_dev->colorSpace();

    QTime t;
    t.start();

    // Let the producer do it's work
    m_producer->clear();
    int i;
    // Handle degenerate case (this happens with the accumulating histogram,
    // which has an empty device)
    if (srcIt.isDone()) {
        m_producer->addRegionToBin(0, 0, 0, cs);
    } else {
        while ( !srcIt.isDone() ) {
            i = srcIt.nConseqPixels();
            m_producer->addRegionToBin(srcIt.rawData(), srcIt.selectionMask(), i, cs);
            srcIt += i;
        }
    }

    computeHistogram();
}

void KisHistogram::computeHistogram()
{
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

KisHistogram::Calculations KisHistogram::calculations() {
    return m_completeCalculations.at(m_channel);
}

KisHistogram::Calculations KisHistogram::selectionCalculations() {
    return m_selectionCalculations.at(m_channel);
}

QVector<KisHistogram::Calculations> KisHistogram::calculateForRange(double from, double to) {
    QVector<Calculations> calculations;
    uint count = m_producer->channels().count();

    for (uint i = 0; i < count; i++) {
        calculations.append(calculateSingleRange(i, from, to));
    }

    return calculations;
}

KisHistogram::Calculations KisHistogram::calculateSingleRange(int channel, double from, double to) {
    Calculations c;

    // XXX If from == to, we only want a specific bin, handle that properly!

    double max = from, min = to, total = 0.0, mean = 0.0; //, median = 0.0, stddev = 0.0;
    quint32 high = 0, low = (quint32) -1, count = 0;

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


void KisHistogram::dump() {
    kDebug(DBG_AREA_MATH) << "Histogram\n";

    switch (m_type) {
    case LINEAR:
        kDebug(DBG_AREA_MATH) << "Linear histogram\n";
        break;
    case LOGARITHMIC:
        kDebug(DBG_AREA_MATH) << "Logarithmic histogram\n";
    }

    kDebug(DBG_AREA_MATH) << "Dumping channel " << m_channel << endl;
    Calculations c = calculations();

/*        for( int i = 0; i <256; ++i ) {
        kDebug(DBG_AREA_MATH) << "Value "
              << QString().setNum(i)
              << ": "
              <<  QString().setNum(m_values[i])
              << "\n";
        }*/
    kDebug(DBG_AREA_MATH) << "\n";

    kDebug(DBG_AREA_MATH) << "Max: " << QString().setNum(c.getMax()) << "\n";
    kDebug(DBG_AREA_MATH) << "Min: " << QString().setNum(c.getMin()) << "\n";
    kDebug(DBG_AREA_MATH) << "High: " << QString().setNum(c.getHighest()) << "\n";
    kDebug(DBG_AREA_MATH) << "Low: " << QString().setNum(c.getLowest()) << "\n";
    kDebug(DBG_AREA_MATH) << "Mean: " << m_producer->positionToString(c.getMean()) << "\n";
    kDebug(DBG_AREA_MATH) << "Total: " << QString().setNum(c.getTotal()) << "\n";
//    kDebug(DBG_AREA_MATH) << "Median: " << QString().setNum(m_median) << "\n";
//    kDebug(DBG_AREA_MATH) << "Stddev: " << QString().setNum(m_stddev) << "\n";
//    kDebug(DBG_AREA_MATH) << "percentile: " << QString().setNum(m_percentile) << "\n";

    kDebug(DBG_AREA_MATH) << "\n";
}
