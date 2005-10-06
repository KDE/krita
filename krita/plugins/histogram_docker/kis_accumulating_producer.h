/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#ifndef _KIS_ACCUMULATING_PRODUCER_H_
#define _KIS_ACCUMULATING_PRODUCER_H_

#include <kis_basic_histogram_producers.h>
#include "kis_cachedhistogram.h"

/**
 * Kept very minimalistic because all options would require much reiterating which we don't want
 **/
class KisAccumulatingHistogramProducer : public KisBasicHistogramProducer {
public:
    KisAccumulatingHistogramProducer(KisCachedHistogramObserver::Producers* source);
    // Iterates over nothing at all, just does its thing with all the source producers
    virtual void addRegionToBin(KisRectIteratorPixel& it, KisColorSpace *cs);
    virtual QString positionToString(double pos) const
        { return m_source -> at(0) -> positionToString(pos); }

    virtual void setView(double, double) {} // No view support
    virtual double maximalZoom() const { return 1.0; }

    virtual Q_INT32 count() { return m_source -> at(0) -> count(); }
    virtual Q_INT32 numberOfBins() { return m_source -> at(0) -> numberOfBins(); }

    virtual QValueVector<KisChannelInfo *> channels() { return m_source -> at(0) -> channels(); }

    /// Call this when the 'source' list has changed colorspace
    virtual void changedSourceProducer() {
        m_count = m_source -> at(0) -> channels().count();
        m_external.clear();
        makeExternalToInternal();
    }

protected:
    /// source already converts external to internal
    virtual int externalToInternal(int ext) { return ext; }
    KisCachedHistogramObserver::Producers* m_source;
};

#endif // _KIS_ACCUMULATING_PRODUCER_H_
