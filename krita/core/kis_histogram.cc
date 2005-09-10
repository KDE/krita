/*
 *  Copyright (c) 2004 Boudewijn Rempt
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
#include <qdatetime.h> // ### Debug

#include "kis_histogram.h"
#include "kis_layer.h"
#include "kis_types.h"
#include "kis_iterators_pixel.h"
#include "kis_abstract_colorspace.h"

KisHistogram::KisHistogram(KisLayerSP layer,
               KisHistogramProducerSP producer,
               const enumHistogramType type)
{
    m_layer = layer;
    m_type = type;
    m_producer = producer;

    m_max = 0;
    m_min = QUANTUM_MAX;
    m_mean = QUANTUM_MAX / 2;
    m_median = QUANTUM_MAX / 2;
    m_stddev = QUANTUM_MAX / 2;
    m_pixels = 0;
    m_count = 0;
    m_high = 0;
    m_low = QUANTUM_MAX;
    m_percentile = 100;
    m_pixels = 1; // AUTOLAYER: should use layer extends.
    m_channel = 0;

    updateHistogram();
}

KisHistogram::~KisHistogram()
{
}

void KisHistogram::updateHistogram()
{
    Q_INT32 x,y,w,h;
    m_layer->exactBounds(x,y,w,h);
    KisRectIteratorPixel srcIt = m_layer -> createRectIterator(x,y,w,h, false);
    KisAbstractColorSpace* cs = m_layer -> colorSpace();

    QTime t;
    t.start();

    // Let the producer do it's work
    m_producer -> clear();
    m_producer -> addRegionToBin(srcIt, cs);

    kdDebug() << t.elapsed() << "ms for histogram (new version)" << endl;

    computeHistogram();
}

void KisHistogram::computeHistogram()
{
    m_high = 0;
    m_low = (Q_UINT32) -1;

    m_count = m_producer -> count();
    if (m_count > 0) {
        Q_INT32 bins = m_producer -> numberOfBins();
        Q_UINT32 current;
        for (Q_INT32 i = 0; i < bins; i++) {
            current = m_producer -> getBinAt(m_channel, i);
            if (current > m_high)
                m_high = current;
            if (current < m_low)
                m_low = current;
        }
    }

#if 0
    dump();
#endif
}


void KisHistogram::dump() {
    kdDebug(DBG_AREA_MATH) << "Histogram\n";

    switch (m_type) {
    case LINEAR:
        kdDebug(DBG_AREA_MATH) << "Linear histogram\n";
        break;
    case LOGARITHMIC:
        kdDebug(DBG_AREA_MATH) << "Logarithmic histogram\n";
    }

        for( int i = 0; i <256; ++i ) {
        kdDebug(DBG_AREA_MATH) << "Value "
              << QString().setNum(i)
              << ": "
              <<  QString().setNum(m_values[i])
              << "\n";
    }
    kdDebug(DBG_AREA_MATH) << "\n";

    kdDebug(DBG_AREA_MATH) << "Max: " << QString().setNum(m_max) << "\n";
    kdDebug(DBG_AREA_MATH) << "Min: " << QString().setNum(m_min) << "\n";
    kdDebug(DBG_AREA_MATH) << "High: " << QString().setNum(m_high) << "\n";
    kdDebug(DBG_AREA_MATH) << "Low: " << QString().setNum(m_low) << "\n";
    kdDebug(DBG_AREA_MATH) << "Mean: " << QString().setNum(m_mean) << "\n";
    kdDebug(DBG_AREA_MATH) << "Median: " << QString().setNum(m_median) << "\n";
    kdDebug(DBG_AREA_MATH) << "Stddev: " << QString().setNum(m_stddev) << "\n";
    kdDebug(DBG_AREA_MATH) << "pixels: " << QString().setNum(m_pixels) << "\n";
    kdDebug(DBG_AREA_MATH) << "count: " << QString().setNum(m_count) << "\n";
    kdDebug(DBG_AREA_MATH) << "percentile: " << QString().setNum(m_percentile) << "\n";

    kdDebug(DBG_AREA_MATH) << "\n";

}
