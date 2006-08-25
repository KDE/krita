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

#include "krs_histogram.h"

#include <kis_paint_layer.h>

namespace Kross {

namespace KritaCore {

Histogram::Histogram(KisPaintLayerSP layer,
                     KisHistogramProducerSP producer,
                     const enumHistogramType type)
    : QObject()
{
    setObjectName("KritaHistogram");
    m_histogram = new KisHistogram(layer, producer, type);
}

Histogram::~Histogram()
{
}

#if 0
Kross::Api::Object::Ptr Histogram::setChannel(Kross::Api::List::Ptr args)
{
    m_histogram->setChannel(Kross::Api::Variant::toUInt(args->item(0)));
    return Kross::Api::Object::Ptr(0);
}
Kross::Api::Object::Ptr Histogram::getChannel(Kross::Api::List::Ptr)
{
    return Kross::Api::Object::Ptr(new Kross::Api::Variant( m_histogram->channel()));
}
Kross::Api::Object::Ptr Histogram::getMax(Kross::Api::List::Ptr)
{
    return Kross::Api::Object::Ptr(new Kross::Api::Variant( m_histogram->calculations().getMax()));
}
Kross::Api::Object::Ptr Histogram::getMin(Kross::Api::List::Ptr)
{
    return Kross::Api::Object::Ptr(new Kross::Api::Variant( m_histogram->calculations().getMin() ));
}
Kross::Api::Object::Ptr Histogram::getHighest(Kross::Api::List::Ptr)
{
    return Kross::Api::Object::Ptr(new Kross::Api::Variant( m_histogram->calculations().getHighest() ));
}
Kross::Api::Object::Ptr Histogram::getLowest(Kross::Api::List::Ptr)
{
    return Kross::Api::Object::Ptr(new Kross::Api::Variant( m_histogram->calculations().getLowest() ));
}
Kross::Api::Object::Ptr Histogram::getMean(Kross::Api::List::Ptr)
{
    return Kross::Api::Object::Ptr(new Kross::Api::Variant( m_histogram->calculations().getMean() ));
}
Kross::Api::Object::Ptr Histogram::getCount(Kross::Api::List::Ptr)
{
    return Kross::Api::Object::Ptr(new Kross::Api::Variant( m_histogram->calculations().getCount() ));
}
Kross::Api::Object::Ptr Histogram::getTotal(Kross::Api::List::Ptr)
{
    return Kross::Api::Object::Ptr(new Kross::Api::Variant( m_histogram->calculations().getTotal() ));
}
Kross::Api::Object::Ptr Histogram::getValue(Kross::Api::List::Ptr args)
{
    return Kross::Api::Object::Ptr(new Kross::Api::Variant( m_histogram->getValue( Kross::Api::Variant::toUInt(args->item(0)) ) ));
}

Kross::Api::Object::Ptr Histogram::getNumberOfBins(Kross::Api::List::Ptr)
{
    return Kross::Api::Object::Ptr(new Kross::Api::Variant( m_histogram->producer()->numberOfBins() ));
}
#endif

}

}
