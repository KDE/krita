/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _Ko_BASIC_HISTOGRAM_PRODUCERS_
#define _Ko_BASIC_HISTOGRAM_PRODUCERS_

#include "KoHistogramProducer.h"

#include <q3valuevector.h>
#include <klocale.h>

#include "config.h"
#include "KoColorSpace.h"
#include "KoID.h"
#include <koffice_export.h>

class KoLabColorSpace;

class /*PIGMENT_EXPORT*/ KoBasicHistogramProducer : public KoHistogramProducer {
public:
    KoBasicHistogramProducer(const KoID& id, int channels, int nrOfBins, KoColorSpace *colorSpace);
    virtual ~KoBasicHistogramProducer() {}

    virtual void clear();

    virtual void setView(double from, double size) { m_from = from; m_width = size; }

    virtual const KoID& id() const { return m_id; }
    virtual Q3ValueVector<KoChannelInfo *> channels() { return m_colorSpace->channels(); }
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
    KoColorSpace *m_colorSpace;
    KoID m_id;
    Q3ValueVector<qint32> m_external;
};

class /*PIGMENT_EXPORT*/ KoBasicU8HistogramProducer : public KoBasicHistogramProducer {
public:
    KoBasicU8HistogramProducer(const KoID& id, KoColorSpace *colorSpace);
    virtual void addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KoColorSpace *colorSpace);
    virtual QString positionToString(double pos) const;
    virtual double maximalZoom() const { return 1.0; }
};

class /*PIGMENT_EXPORT*/ KoBasicU16HistogramProducer : public KoBasicHistogramProducer {
public:
    KoBasicU16HistogramProducer(const KoID& id, KoColorSpace *colorSpace);
    virtual void addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KoColorSpace *colorSpace);
    virtual QString positionToString(double pos) const;
    virtual double maximalZoom() const;
};

class /*PIGMENT_EXPORT*/ KoBasicF32HistogramProducer : public KoBasicHistogramProducer {
public:
    KoBasicF32HistogramProducer(const KoID& id, KoColorSpace *colorSpace);
    virtual void addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KoColorSpace *colorSpace);
    virtual QString positionToString(double pos) const;
    virtual double maximalZoom() const;
};

#ifdef HAVE_OPENEXR
class /*PIGMENT_EXPORT*/ KoBasicF16HalfHistogramProducer : public KoBasicHistogramProducer {
public:
    KoBasicF16HalfHistogramProducer(const KoID& id, KoColorSpace *colorSpace);
    virtual void addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KoColorSpace *colorSpace);
    virtual QString positionToString(double pos) const;
    virtual double maximalZoom() const;
};
#endif

/**
 * Parametrized on a specific KoHistogramProducer. Its generated producers
 * will have the same KoID as the factory's. This is acceptable because we can't mix
 * Factories with Producers in the code because they are incompatible types, and
 * in the GUI we actually only need a producer's name, not a factory's.
 */
template<class T> class KoBasicHistogramProducerFactory : public KoHistogramProducerFactory {
public:
    KoBasicHistogramProducerFactory(const KoID& id, KoColorSpace *colorSpace)
        : KoHistogramProducerFactory(id), m_cs(colorSpace) {}
    virtual ~KoBasicHistogramProducerFactory() {}
    virtual KoHistogramProducerSP generate() { return KoHistogramProducerSP(new T(id(), m_cs)); }
    virtual bool isCompatibleWith(KoColorSpace* colorSpace) const { return colorSpace->id() == m_cs->id(); }
    virtual float preferrednessLevelWith(KoColorSpace* /*colorSpace*/) const { return 1.0; }
protected:
    KoColorSpace *m_cs;
};

/**
 * This is a Producer (with associated factory) that converts the pixels of the colorspace
 * to RGB8 with toQColor, and then does its counting on RGB. This is NOT registered with the
 * Registry, because it isCompatibleWith all colorspaces, and should only be used in extreme
 * cases (like no other producer being available
 **/
class /*PIGMENT_EXPORT*/ KoGenericRGBHistogramProducer : public KoBasicHistogramProducer {
public:
    KoGenericRGBHistogramProducer();
    virtual void addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KoColorSpace *colorSpace);
    virtual QString positionToString(double pos) const;
    virtual double maximalZoom() const;
    virtual Q3ValueVector<KoChannelInfo *> channels();
protected:
    Q3ValueVector<KoChannelInfo *> m_channelsList;
};

/** KoGenericRGBHistogramProducer his special Factory that isCompatibleWith everything. */
class  /*PIGMENT_EXPORT*/ KoGenericRGBHistogramProducerFactory : public KoHistogramProducerFactory {
public:
    KoGenericRGBHistogramProducerFactory()
        : KoHistogramProducerFactory(KoID("GENRGBHISTO", i18n("Generic RGB Histogram"))) {}
    virtual ~KoGenericRGBHistogramProducerFactory() {}
    virtual KoHistogramProducerSP generate() { return KoHistogramProducerSP(new KoGenericRGBHistogramProducer()); }
    virtual bool isCompatibleWith(KoColorSpace*) const { return true; }
    virtual float preferrednessLevelWith(KoColorSpace*) const { return 0.0; }
};


/**
 * This is a Producer (with associated factory) that converts the pixels of the colorspace
 * to L*a*b*, and then does its counting.
 * It isCompatibleWith all colorspaces
 **/
class  /*PIGMENT_EXPORT*/ KoGenericLabHistogramProducer : public KoBasicHistogramProducer {
    public:
        KoGenericLabHistogramProducer();
        virtual ~KoGenericLabHistogramProducer();
        virtual void addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KoColorSpace *colorSpace);
        virtual QString positionToString(double pos) const;
        virtual double maximalZoom() const;
        virtual Q3ValueVector<KoChannelInfo *> channels();
    protected:
        Q3ValueVector<KoChannelInfo *> m_channelsList;
    private:
        static KoLabColorSpace* m_labCs;
};

/** KoGenericLabHistogramProducer his special Factory that isCompatibleWith everything. */
class /*PIGMENT_EXPORT*/ KoGenericLabHistogramProducerFactory : public KoHistogramProducerFactory {
    public:
        KoGenericLabHistogramProducerFactory()
    : KoHistogramProducerFactory(KoID("GENLABHISTO", i18n("Generic L*a*b* Histogram"))) {}
        virtual ~KoGenericLabHistogramProducerFactory() {}
        virtual KoHistogramProducerSP generate() { return KoHistogramProducerSP(new KoGenericLabHistogramProducer()); }
        virtual bool isCompatibleWith(KoColorSpace*) const { return true; }
        virtual float preferrednessLevelWith(KoColorSpace*) const { return 0.0; }
};


#endif // _Ko_BASIC_HISTOGRAM_PRODUCERS_
