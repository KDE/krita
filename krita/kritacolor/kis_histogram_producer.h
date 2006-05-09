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

#ifndef _KIS_HISTOGRAM_PRODUCER_
#define _KIS_HISTOGRAM_PRODUCER_

#include <qglobal.h>
#include <ksharedptr.h>

#include <kis_colorspace.h>

#include "kis_generic_registry.h"

class KisRectIteratorPixel;
class QString;
class KisChannelInfo;

/**
 * This class is an interface used in the generation of a histogram. It is a container of
 * data, all mathematically interesting things will calculated by a KisHistogram.
 *
 * The default view will be the entire range each color can be in. And don't let the
 * numberOfBins return anything else then 256 unless you have a very good reason for it.
 *
 * About the views: a view is a zoom combined with a start level: the entire
 * range of a channel is 0.0 - 1.0: this is the position. Combined with a zoom, we can
 * calculate what part of a channel will fall in a bin. This gives us an interface to
 * that the views that is not dependent of the actual colorspace of the histogram.
 * The 'size' value is the size, again from 0.0 to 1.0 of the displayed range.
 *
 * For comfort of the GUI, and because it is logical, channels are accessed in the order
 * in which they are found in the channels() method. This is potentially different from
 * the order in which they are internally ordered!
 **/
class KRITACOLOR_EXPORT KisHistogramProducer : public KShared {
public:
    KisHistogramProducer() : m_skipTransparent(true), m_skipUnselected(true) {}
    virtual ~KisHistogramProducer() {}

    // Methods to change the bins

    /** Clears the data in this producer, but keeps its other settings */
    virtual void clear() = 0;

    /**
     * Adds the values from the specified array of pixels to the bins -- does not
     * reset anything.
     *
     * @param pixels A pointer an array of pixeldata in the given colorspace
     * @param selectionMask a pointer to an array of bytes, where 0 is unselected and 1-255 is degree of selectedness. The array
     *                      must be just as long as the array of pixels.
     * @param nPixels The number of pixels
     * @param colorSpace the colorspace that can decode the pixel data.
     */
    virtual void addRegionToBin(quint8 * pixels, quint8 * selectionMask, quint32 nPixels, KisColorSpace* colorSpace) = 0;

    // Methods to set what exactly is being added to the bins
    virtual void setView(double from, double width) = 0;
    virtual void setSkipTransparent(bool set) { m_skipTransparent = set; }
    virtual void setSkipUnselected(bool set) { m_skipUnselected = set; }

    // Methods with general information about this specific producer
    virtual const KisID& id() const = 0;
    virtual Q3ValueVector<KisChannelInfo *> channels() = 0;
    virtual qint32 numberOfBins() = 0;
    virtual QString positionToString(double pos) const = 0;
    virtual double viewFrom() const = 0;
    virtual double viewWidth() const = 0;
    virtual double maximalZoom() const = 0;

    // Methods to get information on the data we have seen
    virtual qint32 count() = 0;
    virtual qint32 getBinAt(qint32 channel, qint32 position) = 0;
    virtual qint32 outOfViewLeft(qint32 channel) = 0;
    virtual qint32 outOfViewRight(qint32 channel) = 0;
protected:
    bool m_skipTransparent;
    bool m_skipUnselected;
};

typedef KSharedPtr<KisHistogramProducer> KisHistogramProducerSP;

class KRITACOLOR_EXPORT KisHistogramProducerFactory {
public:
    KisHistogramProducerFactory(const KisID& id) : m_id(id) {}
    virtual ~KisHistogramProducerFactory() {}
    /// Factory method, generates a new KisHistogramProducer
    virtual KisHistogramProducerSP generate() = 0;
    /// Returns if a colorspace can be used with this producer
    virtual bool isCompatibleWith(KisColorSpace* colorSpace) const = 0;
    /// Returns a float in the [0.0, 1.0] range, 0.0 means this is a very generic method
    virtual float preferrednessLevelWith(KisColorSpace* colorSpace) const = 0;
    virtual const KisID& id() const { return m_id; }
protected:
    KisID m_id;
};

class KRITACOLOR_EXPORT KisHistogramProducerFactoryRegistry
    : public KisGenericRegistry<KisHistogramProducerFactory*> {
public:
    virtual ~KisHistogramProducerFactoryRegistry();
    static KisHistogramProducerFactoryRegistry* instance();
    /// returns a list, sorted by preferrence: higher preferance comes first
    KisIDList listKeysCompatibleWith(KisColorSpace* colorSpace) const;

private:
   KisHistogramProducerFactoryRegistry();
   KisHistogramProducerFactoryRegistry(const KisHistogramProducerFactoryRegistry&);
   KisHistogramProducerFactoryRegistry operator=(const KisHistogramProducerFactoryRegistry&);

   static KisHistogramProducerFactoryRegistry* m_singleton;
};

#endif // _KIS_HISTOGRAM_PRODUCER
