/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KROSS_KRITACOREHISTOGRAM_H
#define KROSS_KRITACOREHISTOGRAM_H

#include <api/class.h>

#include <kis_types.h>
#include <kis_histogram.h>

namespace Kross {

namespace KritaCore {

/**
@author Cyrille Berger
*/
class Histogram : public Kross::Api::Class<Histogram>
{
    public:
        Histogram(KisLayerSP layer, KisHistogramProducerSP producer, const enumHistogramType type);
        ~Histogram();
        virtual const QString getClassName() const;
    public:
        Kross::Api::Object::Ptr getMax(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr getMin(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr getHighest(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr getLowest(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr getMean(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr getCount(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr getTotal(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr setChannel(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr getChannel(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr getValue(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr getNumberOfBins(Kross::Api::List::Ptr);
    private:
        KisHistogram* m_histogram;
};

}

}

#endif
