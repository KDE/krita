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

#include "krs_layer.h"

#include <kis_colorspace_factory_registry.h>
#include <kis_doc.h>
#include <kis_layer.h>
#include <kis_meta_registry.h>
#include <kis_iterators_pixel.h>
#include <kis_transaction.h>

#include "krs_iterator.h"
#include "krs_histogram.h"

namespace Kross {

namespace KritaCore {

Layer::Layer(KisLayerSP layer, KisDoc* doc)
    : Kross::Api::Class<Layer>("KritaLayer"), m_layer(layer), m_doc(doc), m_cmd(0)
{
    addFunction("createRectIterator", &Layer::createRectIterator,
                Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt")  << Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt") );
    addFunction("createHLineIterator", &Layer::createHLineIterator,
                Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt")  <<  Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt") );
    addFunction("createVLineIterator", &Layer::createVLineIterator,
                Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt")  <<  Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt") );
    addFunction("getWidth", &Layer::getWidth);
    addFunction("getHeight", &Layer::getHeight);
    addFunction("createHistogram", &Layer::createHistogram, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String") << Kross::Api::Argument("Kross::Api::Variant::UInt") );
    addFunction("beginPainting", &Layer::beginPainting, Kross::Api::ArgumentList() );
    addFunction("endPainting", &Layer::endPainting);
    addFunction("convertToColorspace", &Layer::convertToColorspace, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String") );
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

Kross::Api::Object::Ptr Layer::createHistogram(Kross::Api::List::Ptr args)
{
    KisHistogramProducerFactory* factory = KisHistogramProducerFactoryRegistry::instance()->get(Kross::Api::Variant::toString(args->item(0)));
    
/*    KisIDList listID = KisHistogramProducerFactoryRegistry::instance()->listKeys();
    for(KisIDList::iterator it = listID.begin(); it != listID.end(); it++)
    {
        kdDebug() << (*it).name() << " " << (*it).id() << endl;
    }*/
    
    enumHistogramType type ;
    switch( Kross::Api::Variant::toUInt(args->item(1)) )
    {
        case 1:
            type = LOGARITHMIC;
            break;
        case 0:
        default:
            type = LINEAR;
            break;
    }
    if(factory && factory->isCompatibleWith( m_layer->colorSpace() ))
    {
        return new Histogram( m_layer, factory->generate() , type);
    }
    return 0;
}

Kross::Api::Object::Ptr Layer::beginPainting(Kross::Api::List::Ptr args)
{
    QString name = args->count() > 0 ? Kross::Api::Variant::toString(args->item(0)) : "script";
    if(m_cmd != 0)
    {
        delete m_cmd;
    }
    m_cmd = new KisTransaction(name, m_layer.data());
    Q_CHECK_PTR(m_cmd);
    return 0;
}

Kross::Api::Object::Ptr Layer::endPainting(Kross::Api::List::Ptr)
{
    if(m_doc !=0)
    {
        m_doc->setModified(true);
        m_doc->currentImage()->notify();
    }
    if(m_cmd != 0)
    {
        m_layer->image()->undoAdapter()->addCommand(m_cmd);
    }
    return 0;
}

Kross::Api::Object::Ptr Layer::convertToColorspace(Kross::Api::List::Ptr args)
{
    KisColorSpace * dstCS = KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID(Kross::Api::Variant::toString(args->item(0)), ""), "");
    if(!dstCS)
    {
        // FIXME: inform user
        kdDebug() << Kross::Api::Variant::toString(args->item(0)) << " colorspace is not available, please check your installation." << endl;
        return 0;
    }
    m_layer->convertTo(dstCS);
    return 0;
}

}

}
