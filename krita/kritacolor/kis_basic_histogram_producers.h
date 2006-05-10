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

#include <q3valuevector.h>
#include <klocale.h>

#include "config.h"

#include "kis_histogram_producer.h"
#include "kis_colorspace.h"
#include "kis_id.h"
#include <krita_export.h>

class KisLabColorSpace;

class KRITACOLOR_EXPORT KisBasicHistogramProducer : public KisHistogramProducer {
public:
    KisBasicHistogramProducer(const KisID& id, int channels, int nrOfBins, KisColorSpace *colorSpace);
    virtual ~KisBasicHistogramProducer() {}

    virtual void clear();

    virtual void setView(double from, double size) { m_from = from; m_width = size; }

    virtual const KisID& id() const { return m_id; }
    virtual Q3ValueVector<KisChannelInfo *> channels() { return m_colorSpace->channels(); }
    virtual qint32 numberOfBins() { return m_nrOfBins; }
    virtual double viewFrom() const { return m_from; }
    virtual double viewWidth() const { return m_width; }

    virtual qint32 count() { return m_count; }

    virtual qint32 getBinAt(int channel, int position)
        { return m_bins.at(externalToInternal(channel)).at(position); }

    virtual qint32 outOfViewLeft(int channel)
        { return m_outLeft.at(externalToInternal(channel)); }

    virtual qint32 outOfViewRight(int channel)
        { return m_outRight.at(externalToInternal(channel)); }

protected:
    /**
     * The order in which channels() returns is not the same as the internal representation,
     * that of the pixel internally. This method converts external usage to internal usage.
     * This method uses some basic assumtpions about the layout of the pixel, so _extremely_
     * exotic spaces might want to override this (see makeExternalToInternal source for
     * those assumptions)
     **/
    virtual int externalToInternal(int ext) {
        if (channels().count() > 0 && m_external.count() == 0) // Set up the translation table
            makeExternalToInternal();
        return m_external.at(ext);
    }
    // not virtual since that is useless: we call it from constructor
    void makeExternalToInternal();
    typedef Q3ValueVector<quint32> vBins;
    Q3ValueVector<vBins> m_bins;
    vBins m_outLeft, m_outRight;
    double m_from, m_width;
    qint32 m_count;
    int m_channels, m_nrOfBins;
    KisColorSpace *m_colorSpace;
    KisID m_id;
    Q3ValueVector<qint32> m_external;
};

class KRITACOLOR_EXPORT KisBasicU8HistogramProducer : public KisBasicHistogramProducer {
public:
    KisBasicU8HistogramProducer(const KisID& id, KisColorSpace *colorSpace);
    virtual void addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KisColorSpace *colorSpace);
    virtual QString positionToString(double pos) const;
    virtual double maximalZoom() const { return 1.0; }
};

class KRITACOLOR_EXPORT KisBasicU16HistogramProducer : public KisBasicHistogramProducer {
public:
    KisBasicU16HistogramProducer(const KisID& id, KisColorSpace *colorSpace);
    virtual void addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KisColorSpace *colorSpace);
    virtual QString positionToString(double pos) const;
    virtual double maximalZoom() const;
};

class KRITACOLOR_EXPORT KisBasicF32HistogramProducer : public KisBasicHistogramProducer {
public:
    KisBasicF32HistogramProducer(const KisID& id, KisColorSpace *colorSpace);
    virtual void addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KisColorSpace *colorSpace);
    virtual QString positionToString(double pos) const;
    virtual double maximalZoom() const;
};

#ifdef HAVE_OPENEXR
class KisBasicF16HalfHistogramProducer : public KisBasicHistogramProducer {
public:
    KisBasicF16HalfHistogramProducer(const KisID& id, KisColorSpace *colorSpace);
    virtual void addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KisColorSpace *colorSpace);
    virtual QString positionToString(double pos) const;
    virtual double maximalZoom() const;
};
#endif

/**
 * Parametrized on a specific KisHistogramProducer. Its generated producers
 * will have the same KisID as the factory's. This is acceptable because we can't mix
 * Factories with Producers in the code because they are incompatible types, and
 * in the GUI we actually only need a producer's name, not a factory's.
 */
template<class T> class KisBasicHistogramProducerFactory : public KisHistogramProducerFactory {
public:
    KisBasicHistogramProducerFactory(const KisID& id, KisColorSpace *colorSpace)
        : KisHistogramProducerFactory(id), m_cs(colorSpace) {}
    virtual ~KisBasicHistogramProducerFactory() {}
    virtual KisHistogramProducerSP generate() { return KisHistogramProducerSP(new T(id(), m_cs)); }
    virtual bool isCompatibleWith(KisColorSpace* colorSpace) const { return colorSpace->id() == m_cs->id(); }
    virtual float preferrednessLevelWith(KisColorSpace* /*colorSpace*/) const { return 1.0; }
protected:
    KisColorSpace *m_cs;
};

/**
 * This is a Producer (with associated factory) that converts the pixels of the colorspace
 * to RGB8 with toQColor, and then does its counting on RGB. This is NOT registered with the
 * Registry, because it isCompatibleWith all colorspaces, and should only be used in extreme
 * cases (like no other producer being available
 **/
class KRITACOLOR_EXPORT KisGenericRGBHistogramProducer : public KisBasicHistogramProducer {
public:
    KisGenericRGBHistogramProducer();
    virtual void addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KisColorSpace *colorSpace);
    virtual QString positionToString(double pos) const;
    virtual double maximalZoom() const;
    virtual Q3ValueVector<KisChannelInfo *> channels();
protected:
    Q3ValueVector<KisChannelInfo *> m_channelsList;
};

/** KisGenericRGBHistogramProducer his special Factory that isCompatibleWith everything. */
class  KRITACOLOR_EXPORT KisGenericRGBHistogramProducerFactory : public KisHistogramProducerFactory {
public:
    KisGenericRGBHistogramProducerFactory()
        : KisHistogramProducerFactory(KisID("GENRGBHISTO", i18n("Generic RGB Histogram"))) {}
    virtual ~KisGenericRGBHistogramProducerFactory() {}
    virtual KisHistogramProducerSP generate() { return KisHistogramProducerSP(new KisGenericRGBHistogramProducer()); }
    virtual bool isCompatibleWith(KisColorSpace*) const { return true; }
    virtual float preferrednessLevelWith(KisColorSpace*) const { return 0.0; }
};


/**
 * This is a Producer (with associated factory) that converts the pixels of the colorspace
 * to L*a*b*, and then does its counting.
 * It isCompatibleWith all colorspaces
 **/
class  KRITACOLOR_EXPORT KisGenericLabHistogramProducer : public KisBasicHistogramProducer {
    public:
        KisGenericLabHistogramProducer();
        virtual ~KisGenericLabHistogramProducer();
        virtual void addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KisColorSpace *colorSpace);
        virtual QString positionToString(double pos) const;
        virtual double maximalZoom() const;
        virtual Q3ValueVector<KisChannelInfo *> channels();
    protected:
        Q3ValueVector<KisChannelInfo *> m_channelsList;
    private:
        static KisLabColorSpace* m_labCs;
};

/** KisGenericLabHistogramProducer his special Factory that isCompatibleWith everything. */
class KRITACOLOR_EXPORT KisGenericLabHistogramProducerFactory : public KisHistogramProducerFactory {
    public:
        KisGenericLabHistogramProducerFactory()
    : KisHistogramProducerFactory(KisID("GENLABHISTO", i18n("Generic L*a*b* Histogram"))) {}
        virtual ~KisGenericLabHistogramProducerFactory() {}
        virtual KisHistogramProducerSP generate() { return KisHistogramProducerSP(new KisGenericLabHistogramProducer()); }
        virtual bool isCompatibleWith(KisColorSpace*) const { return true; }
        virtual float preferrednessLevelWith(KisColorSpace*) const { return 0.0; }
};


#endif // _KIS_BASIC_HISTOGRAM_PRODUCERS_
