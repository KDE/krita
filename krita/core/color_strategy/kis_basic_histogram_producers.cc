/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include <qstring.h>

#include "kis_basic_histogram_producers.h"
#include "kis_iterators_pixel.h"
#include "kis_integer_maths.h"

KisBasicHistogramProducer::KisBasicHistogramProducer(const KisID& id, int channels,
        int nrOfBins, KisAbstractColorSpace *cs)
    : m_channels(channels),
      m_nrOfBins(nrOfBins),
      m_colorSpace(cs),
      m_id(id)
{
    m_bins.resize(m_channels);
    for (int i = 0; i < m_channels; i++)
        m_bins.at(i).resize(m_nrOfBins);
    m_outLeft.resize(m_channels);
    m_outRight.resize(m_channels);
    m_count = 0;
    m_from = 0.0;
    m_width = 1.0;
}

void KisBasicHistogramProducer::clear() {
    m_count = 0;
    for (int i = 0; i < m_channels; i++) {
        for (int j = 0; j < m_nrOfBins; j++) {
            m_bins.at(i).at(j) = 0;
        }
    }
}

// ------------ U8 ---------------------

KisBasicU8HistogramProducer::KisBasicU8HistogramProducer(const KisID& id,
        KisAbstractColorSpace *cs)
    : KisBasicHistogramProducer(id, cs -> nChannels(), 256, cs) {
}

QString KisBasicU8HistogramProducer::positionToString(double pos) const {
    return QString("%1").arg(static_cast<Q_UINT8>(pos * UINT8_MAX));
}

void KisBasicU8HistogramProducer::addRegionToBin(KisRectIteratorPixel& it,
         KisAbstractColorSpace *cs) {
    for (int i = 0; i < m_channels; i++) {
        m_outRight.at(i) = 0;
        m_outLeft.at(i) = 0;
    }
    while (!it.isDone()) {
        Q_UINT8* pixel = it.rawData();
        if (   (m_skipUnselected && !it.isSelected())
            || (m_skipTransparent && cs -> getAlpha(pixel) == OPACITY_TRANSPARENT) ) {
            ++it;
            continue;
        }

        for (int i = 0; i < m_channels; i++) {
            m_bins.at(i).at(pixel[i])++;
        }

        m_count++;
        ++it;
    }
}

// ------------ U16 ---------------------

KisBasicU16HistogramProducer::KisBasicU16HistogramProducer(const KisID& id,
        KisAbstractColorSpace *cs)
    : KisBasicHistogramProducer(id, cs -> nChannels(), 256, cs) {
}

QString KisBasicU16HistogramProducer::positionToString(double pos) const {
    return QString("%1").arg(static_cast<Q_UINT8>(pos * UINT8_MAX));
}

double KisBasicU16HistogramProducer::maximalZoom() const {
    return 1.0 / 255.0;
}

void KisBasicU16HistogramProducer::addRegionToBin(KisRectIteratorPixel& it,
         KisAbstractColorSpace *cs) {
    // The view
    Q_UINT16 from = static_cast<Q_UINT16>(m_from * UINT16_MAX);
    Q_UINT16 width = static_cast<Q_UINT16>(m_width * UINT16_MAX + 0.5); // We include the end
    Q_UINT16 to = from + width;
    double factor = 255.0 / width;
    kdDebug() << "from: " << from << "; to: " << to << endl;
    m_outRight.clear();
    m_outRight.resize(m_channels);
    m_outLeft.clear();
    m_outLeft.resize(m_channels);

    while (!it.isDone()) {
        Q_UINT8* pixelRaw = it.rawData();
        Q_UINT16* pixel = reinterpret_cast<Q_UINT16*>(pixelRaw);
        if (   (m_skipUnselected && !it.isSelected())
            || (m_skipTransparent && cs -> getAlpha(pixelRaw) == OPACITY_TRANSPARENT) ) {
            ++it;
            continue;
        }

        for (int i = 0; i < m_channels; i++) {
            Q_UINT16 value = pixel[i];
            if (value > to)
                m_outRight.at(i)++;
            else if (value < from)
                m_outLeft.at(i)++;
            else
                m_bins.at(i).at(static_cast<Q_UINT8>((value - from) * factor))++;
        }

        m_count++;
        ++it;
    }
}

// ------------ Float32 ---------------------
KisBasicF32HistogramProducer::KisBasicF32HistogramProducer(const KisID& id,
        KisAbstractColorSpace *cs)
    : KisBasicHistogramProducer(id, cs -> nChannels(), 256, cs) {
}

QString KisBasicF32HistogramProducer::positionToString(double pos) const {
    return QString("%1").arg(static_cast<float>(pos)); // XXX I doubt this is correct!
}

double KisBasicF32HistogramProducer::maximalZoom() const {
    // XXX What _is_ the maximal zoom here? I don't think there is one with floats, so this seems a fine compromis for the moment
    return 1.0 / 255.0;
}

void KisBasicF32HistogramProducer::addRegionToBin(KisRectIteratorPixel& it,
        KisAbstractColorSpace *cs) {
    // The view
    float from = static_cast<float>(m_from);
    float width = static_cast<float>(m_width);
    float to = from + width;
    float factor = 255.0 / width;
    kdDebug() << "from: " << from << "; to: " << to << endl;

    m_outRight.clear();
    m_outRight.resize(m_channels);
    m_outLeft.clear();
    m_outLeft.resize(m_channels);

    while (!it.isDone()) {
        Q_UINT8* pixelRaw = it.rawData();
        float* pixel = reinterpret_cast<float*>(pixelRaw);
        if (   (m_skipUnselected && !it.isSelected())
                || (m_skipTransparent && cs -> getAlpha(pixelRaw) == OPACITY_TRANSPARENT) ) {
            ++it;
            continue;
        }

        for (int i = 0; i < m_channels; i++) {
            float value = pixel[i];
            if (value > to)
                m_outRight.at(i)++;
            else if (value < from)
                m_outLeft.at(i)++;
            else
                m_bins.at(i).at(static_cast<Q_UINT8>((value - from) * factor))++;
        }

        m_count++;
        ++it;
    }
}

