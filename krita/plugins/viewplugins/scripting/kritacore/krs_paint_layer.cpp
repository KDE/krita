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

#include "krs_paint_layer.h"

#include <klocale.h>

#include <kis_colorspace_factory_registry.h>
#include <kis_doc.h>
#include <kis_layer.h>
#include <kis_meta_registry.h>
#include <kis_iterators_pixel.h>
#include <kis_transaction.h>
#include <kis_math_toolbox.h>

#include "krs_iterator.h"
#include "krs_histogram.h"
#include "krs_painter.h"
#include "krs_wavelet.h"

namespace Kross {

namespace KritaCore {

PaintLayer::PaintLayer(KisPaintLayerSP layer, KisDoc* doc)
    : Kross::Api::Class<PaintLayer>("KritaLayer"), m_layer(layer), m_doc(doc), m_cmd(0)
{
    addFunction("createRectIterator", &PaintLayer::createRectIterator,
                Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt")  << Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt") );
    addFunction("createHLineIterator", &PaintLayer::createHLineIterator,
                Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt")  <<  Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt") );
    addFunction("createVLineIterator", &PaintLayer::createVLineIterator,
                Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::UInt")  <<  Kross::Api::Argument("Kross::Api::Variant::UInt") << Kross::Api::Argument("Kross::Api::Variant::UInt") );
    addFunction("getWidth", &PaintLayer::getWidth);
    addFunction("getHeight", &PaintLayer::getHeight);
    addFunction("createHistogram", &PaintLayer::createHistogram, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String") << Kross::Api::Argument("Kross::Api::Variant::UInt") );
    addFunction("createPainter", &PaintLayer::createPainter);
    addFunction("beginPainting", &PaintLayer::beginPainting, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String" ) );
    addFunction("endPainting", &PaintLayer::endPainting);
    addFunction("convertToColorspace", &PaintLayer::convertToColorspace, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String") );
    addFunction("fastWaveletTransformation", &PaintLayer::fastWaveletTransformation);
    addFunction("fastWaveletUntransformation", &PaintLayer::fastWaveletUntransformation);
    addFunction("colorSpaceId", &PaintLayer::colorSpaceId);
}


PaintLayer::~PaintLayer()
{
}

const QString PaintLayer::getClassName() const {
    return "Kross::KritaCore::PaintLayer";
}

Kross::Api::Object::Ptr PaintLayer::createRectIterator(Kross::Api::List::Ptr args)
{
    return new Iterator<KisRectIteratorPixel>(
            paintLayer()->paintDevice()->createRectIterator(Kross::Api::Variant::toUInt(args->item(0)),
                                        Kross::Api::Variant::toUInt(args->item(1)),
                                        Kross::Api::Variant::toUInt(args->item(2)),
                                        Kross::Api::Variant::toUInt(args->item(3)), true),
            paintLayer());
}
Kross::Api::Object::Ptr PaintLayer::createHLineIterator(Kross::Api::List::Ptr args)
{
    return new Iterator<KisHLineIteratorPixel>(
            paintLayer()->paintDevice()->createHLineIterator(Kross::Api::Variant::toUInt(args->item(0)),
                                        Kross::Api::Variant::toUInt(args->item(1)),
                                        Kross::Api::Variant::toUInt(args->item(2)), true),
            paintLayer());
}
Kross::Api::Object::Ptr PaintLayer::createVLineIterator(Kross::Api::List::Ptr args)
{
    return new Iterator<KisVLineIteratorPixel>(
            paintLayer()->paintDevice()->createVLineIterator(Kross::Api::Variant::toUInt(args->item(0)),
                                        Kross::Api::Variant::toUInt(args->item(1)),
                                        Kross::Api::Variant::toUInt(args->item(2)), true),
            paintLayer());
}
Kross::Api::Object::Ptr PaintLayer::getWidth(Kross::Api::List::Ptr)
{
    QRect r1 = paintLayer()->extent();
    QRect r2 = paintLayer()->image()->bounds();
    QRect rect = r1.intersect(r2);
    return new Kross::Api::Variant(rect.width());
}
Kross::Api::Object::Ptr PaintLayer::getHeight(Kross::Api::List::Ptr)
{
    QRect r1 = paintLayer()->extent();
    QRect r2 = paintLayer()->image()->bounds();
    QRect rect = r1.intersect(r2);
    return new Kross::Api::Variant(rect.height());
}

Kross::Api::Object::Ptr PaintLayer::createHistogram(Kross::Api::List::Ptr args)
{
    QString histoname = Kross::Api::Variant::toString(args->item(0));
    KisHistogramProducerFactory* factory = KisHistogramProducerFactoryRegistry::instance()->get(histoname);

/*    KisIDList listID = KisHistogramProducerFactoryRegistry::instance()->listKeys();
    for(KisIDList::iterator it = listID.begin(); it != listID.end(); it++)
    {
        kdDebug(41011) << (*it).name() << " " << (*it).id() << endl;
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
    if(factory && factory->isCompatibleWith( paintLayer()->paintDevice()->colorSpace() ))
    {
        return new Histogram( paintLayer().data(), factory->generate() , type);
    } else {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("An error has occured in %1").arg("createHistogram") + "\n" + i18n("The histogram %1 is not available").arg(histoname) ) );
    }
    return 0;
}

Kross::Api::Object::Ptr PaintLayer::createPainter(Kross::Api::List::Ptr )
{
    return new Painter(paintLayer());
}

Kross::Api::Object::Ptr PaintLayer::beginPainting(Kross::Api::List::Ptr args)
{
    QString name = Kross::Api::Variant::toString(args->item(0));
    if(m_cmd != 0)
    {
        delete m_cmd;
    }
    m_cmd = new KisTransaction(name, paintLayer()->paintDevice());
    Q_CHECK_PTR(m_cmd);
    return 0;
}

Kross::Api::Object::Ptr PaintLayer::endPainting(Kross::Api::List::Ptr)
{
    if(doc() !=0)
    {
        doc()->setModified(true);
        doc()->currentImage()->activeLayer()->setDirty();
    }
    if(m_cmd != 0)
    {
        paintLayer()->image()->undoAdapter()->addCommand(m_cmd);
    }
    return 0;
}

Kross::Api::Object::Ptr PaintLayer::convertToColorspace(Kross::Api::List::Ptr args)
{
    KisColorSpace * dstCS = KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID(Kross::Api::Variant::toString(args->item(0)), ""), "");
    if(!dstCS)
    {
        // FIXME: inform user
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("An error has occured in %1").arg("convertToColorspace") + "\n" + i18n("Colorspace %1 is not available, please check your installation.").arg(Kross::Api::Variant::toString(args->item(0))) ) );
        return 0;
    }
    paintLayer()->paintDevice()->convertTo(dstCS);
    return 0;
}

Kross::Api::Object::Ptr PaintLayer::colorSpaceId(Kross::Api::List::Ptr )
{
    return new Kross::Api::Variant( paintLayer()->paintDevice()->colorSpace()->id().id() );
}


Kross::Api::Object::Ptr PaintLayer::fastWaveletTransformation(Kross::Api::List::Ptr )
{
    KisMathToolbox* mathToolbox = KisMetaRegistry::instance()->mtRegistry()->get( paintLayer()->paintDevice()->colorSpace()->mathToolboxID() );
    QRect rect = paintLayer()->exactBounds();
    KisMathToolbox::KisWavelet* wav = mathToolbox->fastWaveletTransformation(paintLayer()->paintDevice(), rect);
    return new Wavelet(wav);
}
Kross::Api::Object::Ptr PaintLayer::fastWaveletUntransformation(Kross::Api::List::Ptr args)
{
    Wavelet* wav = (Wavelet*)args->item(0).data();
    KisMathToolbox* mathToolbox = KisMetaRegistry::instance()->mtRegistry()->get( paintLayer()->paintDevice()->colorSpace()->mathToolboxID() );
    QRect rect = paintLayer()->exactBounds();
    mathToolbox->fastWaveletUntransformation( paintLayer()->paintDevice(), rect, wav->wavelet() );
    return 0;
}


}

}
