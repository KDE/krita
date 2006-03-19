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

#ifndef _CACHED_HISTOGRAM_H_
#define _CACHED_HISTOGRAM_H_

#include <qvaluevector.h>
#include <kis_histogram_producer.h>

#include "kis_imagerasteredcache.h"

class KisCachedHistogramObserver : public KisImageRasteredCache::Observer {
public:
    typedef QValueVector<KisHistogramProducer*> Producers;
    KisCachedHistogramObserver(Producers* p, KisHistogramProducerFactory* f,
                               int x, int y, int w, int h, bool add = true)
        : m_producers(p), m_factory(f), m_x(x), m_y(y), m_w(w), m_h(h)
    {
        m_producer = m_factory->generate();
        if (add)
            m_producers->append(m_producer);
    }
    virtual ~KisCachedHistogramObserver() {}

    virtual Observer* createNew(int x, int y, int w, int h)
        { return new KisCachedHistogramObserver(m_producers, m_factory, x, y, w, h); }
        
    virtual void regionUpdated(KisPaintDeviceSP dev);
private:
    Producers* m_producers;
    KisHistogramProducerFactory* m_factory;
    KisHistogramProducerSP m_producer;
    int m_x, m_y, m_w, m_h;
};

#endif // _CACHED_HISTOGRAM_H_
