/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#include "krs_layer.h"

#include <kis_layer.h>
#include <kis_iterators_pixel.h>

#include "krs_iterator.h"

namespace Kross {

namespace KritaCore {

Layer::Layer(KisLayerSP layer)
    : Kross::Api::Class<Layer>("KritaLayer"), m_layer(layer)
{
    addFunction("createRectIterator", &Layer::createRectIterator,
                Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt")  << Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt") );
    addFunction("createHLineIterator", &Layer::createHLineIterator,
                Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt")  <<  Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt") );
    addFunction("createVLineIterator", &Layer::createVLineIterator,
                Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt")  <<  Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt") );
    addFunction("getWidth", &Layer::getWidth);
    addFunction("getHeight", &Layer::getHeight);
}


Layer::~Layer()
{
}

const QString Layer::getClassName() const {
    return "Kross::KritaCore::Layer";
}

Kross::Api::Object::Ptr Layer::createRectIterator(Kross::Api::List::Ptr args)
{
    return new Iterator<KisRectIteratorPixel>(
            m_layer->createRectIterator(Kross::Api::Variant::toUInt(args->item(0)),
                                        Kross::Api::Variant::toUInt(args->item(1)),
                                        Kross::Api::Variant::toUInt(args->item(2)),
                                        Kross::Api::Variant::toUInt(args->item(3)), true),
            m_layer);
}
Kross::Api::Object::Ptr Layer::createHLineIterator(Kross::Api::List::Ptr args)
{
    return new Iterator<KisHLineIteratorPixel>(
            m_layer->createHLineIterator(Kross::Api::Variant::toUInt(args->item(0)),
                                        Kross::Api::Variant::toUInt(args->item(1)),
                                        Kross::Api::Variant::toUInt(args->item(2)), true),
            m_layer);
}
Kross::Api::Object::Ptr Layer::createVLineIterator(Kross::Api::List::Ptr args)
{
    return new Iterator<KisVLineIteratorPixel>(
            m_layer->createVLineIterator(Kross::Api::Variant::toUInt(args->item(0)),
                                        Kross::Api::Variant::toUInt(args->item(1)),
                                        Kross::Api::Variant::toUInt(args->item(2)), true),
            m_layer);
}
Kross::Api::Object::Ptr Layer::getWidth(Kross::Api::List::Ptr)
{
    QRect r1 = m_layer->extent();
    QRect r2 = m_layer->image()->bounds();
    QRect rect = r1.intersect(r2);
    return new Kross::Api::Variant(rect.width());
}
Kross::Api::Object::Ptr Layer::getHeight(Kross::Api::List::Ptr)
{
    QRect r1 = m_layer->extent();
    QRect r2 = m_layer->image()->bounds();
    QRect rect = r1.intersect(r2);
    return new Kross::Api::Variant(rect.height());
}



}

}
