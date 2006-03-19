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
 : Kross::Api::Class<Histogram>("KritaHistogram")
{
    m_histogram = new KisHistogram(layer, producer, type);
    addFunction("getMax", &Histogram::getMax);
    addFunction("getMin", &Histogram::getMin);
    addFunction("getHighest", &Histogram::getHighest);
    addFunction("getLowest", &Histogram::getLowest);
    addFunction("getMean", &Histogram::getMean);
    addFunction("getCount", &Histogram::getCount);
    addFunction("getTotal", &Histogram::getTotal);
    addFunction("setChannel", &Histogram::setChannel, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("getChannel", &Histogram::getChannel);
    addFunction("getValue", &Histogram::getValue, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("getNumberOfBins", &Histogram::getNumberOfBins);
}

Histogram::~Histogram()
{
}

const QString Histogram::getClassName() const {
    return "Kross::KritaCore::Histogram";
}

Kross::Api::Object::Ptr Histogram::setChannel(Kross::Api::List::Ptr args)
{
    m_histogram->setChannel(Kross::Api::Variant::toUInt(args->item(0)));
    return 0;
}
Kross::Api::Object::Ptr Histogram::getChannel(Kross::Api::List::Ptr)
{
    return new Kross::Api::Variant( m_histogram->channel());
}
Kross::Api::Object::Ptr Histogram::getMax(Kross::Api::List::Ptr)
{
    return new Kross::Api::Variant( m_histogram->calculations().getMax());
}
Kross::Api::Object::Ptr Histogram::getMin(Kross::Api::List::Ptr)
{
    return new Kross::Api::Variant( m_histogram->calculations().getMin() );
}
Kross::Api::Object::Ptr Histogram::getHighest(Kross::Api::List::Ptr)
{
    return new Kross::Api::Variant( m_histogram->calculations().getHighest() );
}
Kross::Api::Object::Ptr Histogram::getLowest(Kross::Api::List::Ptr)
{
    return new Kross::Api::Variant( m_histogram->calculations().getLowest() );
}
Kross::Api::Object::Ptr Histogram::getMean(Kross::Api::List::Ptr)
{
    return new Kross::Api::Variant( m_histogram->calculations().getMean() );
}
Kross::Api::Object::Ptr Histogram::getCount(Kross::Api::List::Ptr)
{
    return new Kross::Api::Variant( m_histogram->calculations().getCount() );
}
Kross::Api::Object::Ptr Histogram::getTotal(Kross::Api::List::Ptr)
{
    return new Kross::Api::Variant( m_histogram->calculations().getTotal() );
}
Kross::Api::Object::Ptr Histogram::getValue(Kross::Api::List::Ptr args)
{
    return new Kross::Api::Variant( m_histogram->getValue( Kross::Api::Variant::toUInt(args->item(0)) ) );
}

Kross::Api::Object::Ptr Histogram::getNumberOfBins(Kross::Api::List::Ptr)
{
    return new Kross::Api::Variant( m_histogram->producer()->numberOfBins() );
}


}

}
