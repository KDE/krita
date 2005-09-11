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

#ifndef _KIS_BASIC_HISTOGRAM_PRODUCERS_
#define _KIS_BASIC_HISTOGRAM_PRODUCERS_

#include <qvaluevector.h>

#include "kis_histogram_producer.h"
#include "kis_abstract_colorspace.h"
#include "kis_id.h"

class KisBasicHistogramProducer : public KisHistogramProducer {
public:
    KisBasicHistogramProducer(const KisID& id, int channels, int nrOfBins, KisAbstractColorSpace *cs);
    virtual ~KisBasicHistogramProducer() {}

    virtual void clear();

    virtual void setView(double from, double size) { m_from = from; m_width = size; }

    virtual const KisID& id() const { return m_id; }
    virtual vKisChannelInfoSP channels() { return m_colorSpace -> channels(); }
    virtual Q_INT32 numberOfBins() { return m_nrOfBins; }
    virtual double viewFrom() const { return m_from; }
    virtual double viewWidth() const { return m_width; }

    virtual Q_INT32 count() { return m_count; }
    virtual Q_INT32 getBinAt(int channel, int position)
        { return m_bins.at(channel).at(position); }
    virtual Q_INT32 outOfViewLeft(int channel) { return m_outLeft.at(channel); }
    virtual Q_INT32 outOfViewRight(int channel) { return m_outRight.at(channel); }
protected:
    typedef QValueVector<Q_UINT32> vBins;
    QValueVector<vBins> m_bins;
    vBins m_outLeft, m_outRight;
    double m_from, m_width;
    Q_INT32 m_count;
    int m_channels, m_nrOfBins;
    KisAbstractColorSpace *m_colorSpace;
    KisID m_id;
};

class KisBasicU8HistogramProducer : public KisBasicHistogramProducer {
public:
    KisBasicU8HistogramProducer(const KisID& id, KisAbstractColorSpace *cs);
    virtual void addRegionToBin(KisRectIteratorPixel& it, KisAbstractColorSpace *cs);
    virtual QString positionToString(double pos) const;
    virtual double maximalZoom() const { return 1.0; }
};

class KisBasicU16HistogramProducer : public KisBasicHistogramProducer {
public:
    KisBasicU16HistogramProducer(const KisID& id, KisAbstractColorSpace *cs);
    virtual void addRegionToBin(KisRectIteratorPixel& it, KisAbstractColorSpace *cs);
    virtual QString positionToString(double pos) const;
    virtual double maximalZoom() const;
};

class KisBasicF32HistogramProducer : public KisBasicHistogramProducer {
public:
    KisBasicF32HistogramProducer(const KisID& id, KisAbstractColorSpace *cs);
    virtual void addRegionToBin(KisRectIteratorPixel& it, KisAbstractColorSpace *cs);
    virtual QString positionToString(double pos) const;
    virtual double maximalZoom() const;
};

/**
 * Parametrized on a specific KisHistogramProducer. Its generated producers
 * will have the same KisID as the factory's. This is acceptable because we can't mix
 * Factories with Producers in the code because they are incompatible types, and
 * in the GUI we actually only need a producer's name, not a factory's.
 */
template<class T> class KisBasicHistogramProducerFactory : public KisHistogramProducerFactory {
public:
    KisBasicHistogramProducerFactory(const KisID& id, KisAbstractColorSpace *cs)
        : KisHistogramProducerFactory(id), m_cs(cs) {}
    virtual ~KisBasicHistogramProducerFactory() {}
    virtual KisHistogramProducerSP generate() { return new T(id(), m_cs); }
    virtual bool isCompatibleWith(KisAbstractColorSpace* cs) const { return cs == m_cs; }
protected:
    KisAbstractColorSpace *m_cs;
};

/**
 * This is a Producer (with associated factory) that converts the pixels of the colorspace
 * to RGB8 with toQColor, and then does its counting on RGB. This is NOT registered with the
 * Registry, because it isCompatibleWith all colorspaces, and should only be used in extreme
 * cases (like no other producer being available
 **/
class KisGenericRGBHistogramProducer : public KisBasicHistogramProducer {
public:
    KisGenericRGBHistogramProducer();
    virtual void addRegionToBin(KisRectIteratorPixel& it, KisAbstractColorSpace *cs);
    virtual QString positionToString(double pos) const;
    virtual double maximalZoom() const;
    virtual vKisChannelInfoSP channels();
protected:
    vKisChannelInfoSP m_channelsList;
};

/** KisGenericRGBHistogramProducer his special Factory that isCompatibleWith everything. */
class KisGenericRGBHistogramProducerFactory : public KisHistogramProducerFactory {
public:
    KisGenericRGBHistogramProducerFactory()
        : KisHistogramProducerFactory(KisID("GENRGBHISTO", "")) {}
    virtual ~KisGenericRGBHistogramProducerFactory() {}
    virtual KisHistogramProducerSP generate() { return new KisGenericRGBHistogramProducer(); }
    virtual bool isCompatibleWith(KisAbstractColorSpace*) const { return true; }
};
#endif // _KIS_BASIC_HISTOGRAM_PRODUCERS_
