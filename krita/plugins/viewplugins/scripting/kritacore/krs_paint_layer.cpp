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

#include <KoColorSpaceRegistry.h>
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

using namespace Kross::KritaCore;

PaintLayer::PaintLayer(KisPaintLayerSP layer, KisDoc* doc)
    : QObject(), m_layer(layer), m_doc(doc), m_cmd(0)
{
    setObjectName("KritaLayer");
}


PaintLayer::~PaintLayer()
{
}

int PaintLayer::width()
{
    QRect r1 = paintLayer()->extent();
    QRect r2 = paintLayer()->image()->bounds();
    QRect rect = r1.intersect(r2);
    return rect.width();
}

int PaintLayer::height()
{
    QRect r1 = paintLayer()->extent();
    QRect r2 = paintLayer()->image()->bounds();
    QRect rect = r1.intersect(r2);
    return rect.height();
}

QString PaintLayer::colorSpaceId()
{
    return paintLayer()->paintDevice()->colorSpace()->id();
}

QObject* PaintLayer::createRectIterator(uint x, uint y, uint width, uint height)
{
    return new Iterator<KisRectIteratorPixel>(
            paintLayer()->paintDevice()->createRectIterator(x, y, width, height, true),
            paintLayer());
}

#if 0
QObject* PaintLayer::createHLineIterator(uint x, uint y, uint width)
{
    return new Iterator<KisHLineIteratorPixel>(
            paintLayer()->paintDevice()->createHLineIterator(x, y, width, true),
            paintLayer());
}

QObject* PaintLayer::createVLineIterator(uint x, uint y, uint height)
{
    return new Iterator<KisVLineIteratorPixel>(
            paintLayer()->paintDevice()->createVLineIterator(x, y, height, true),
            paintLayer());
}

Kross::Api::Object::Ptr PaintLayer::createHistogram(Kross::Api::List::Ptr args)
{
    QString histoname = Kross::Api::Variant::toString(args->item(0));
    KisHistogramProducerFactory* factory = KisHistogramProducerFactoryRegistry::instance()->get(histoname);

/*    QList<KoID> listID = KisHistogramProducerFactoryRegistry::instance()->listKeys();
    for(QList<KoID>::iterator it = listID.begin(); it != listID.end(); it++)
    {
        kDebug(41011) << (*it).name() << " " << (*it).id() << endl;
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
        return Kross::Api::Object::Ptr(new Histogram( paintLayer(), factory->generate() , type));
    } else {
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("An error has occured in %1", QString("createHistogram") ) + '\n' + i18n("The histogram %1 is not available", histoname) ) );
    }
    return Kross::Api::Object::Ptr(0);
}
Kross::Api::Object::Ptr PaintLayer::createPainter(Kross::Api::List::Ptr )
{
    return Kross::Api::Object::Ptr(new Painter(paintLayer()));
}
#endif

void PaintLayer::beginPainting(const QString& name)
{
    if(m_cmd != 0)
    {
        delete m_cmd;
    }
    m_cmd = new KisTransaction(name, paintLayer()->paintDevice());
    Q_CHECK_PTR(m_cmd);
}

void PaintLayer::endPainting()
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
}

#if 0
Kross::Api::Object::Ptr PaintLayer::convertToColorspace(Kross::Api::List::Ptr args)
{
    KoColorSpace * dstCS = KisMetaRegistry::instance()->csRegistry()->colorSpace(Kross::Api::Variant::toString(args->item(0)), 0);
    if(!dstCS)
    {
        // FIXME: inform user
        throw Kross::Api::Exception::Ptr( new Kross::Api::Exception( i18n("An error has occured in %1", QString("convertToColorspace") ) + '\n' + i18n("Colorspace %1 is not available, please check your installation.", Kross::Api::Variant::toString(args->item(0))) ) );
        return Kross::Api::Object::Ptr(0);
    }
    paintLayer()->paintDevice()->convertTo(dstCS);
    return Kross::Api::Object::Ptr(0);
}
Kross::Api::Object::Ptr PaintLayer::fastWaveletTransformation(Kross::Api::List::Ptr )
{
    KisMathToolbox* mathToolbox = KisMetaRegistry::instance()->mtRegistry()->get( paintLayer()->paintDevice()->colorSpace()->mathToolboxID() );
    QRect rect = paintLayer()->exactBounds();
    KisMathToolbox::KisWavelet* wav = mathToolbox->fastWaveletTransformation(paintLayer()->paintDevice(), rect);
    return Kross::Api::Object::Ptr(new Wavelet(wav));
}
Kross::Api::Object::Ptr PaintLayer::fastWaveletUntransformation(Kross::Api::List::Ptr args)
{
    Wavelet* wav = (Wavelet*)args->item(0);
    KisMathToolbox* mathToolbox = KisMetaRegistry::instance()->mtRegistry()->get( paintLayer()->paintDevice()->colorSpace()->mathToolboxID() );
    QRect rect = paintLayer()->exactBounds();
    mathToolbox->fastWaveletUntransformation( paintLayer()->paintDevice(), rect, wav->wavelet() );
    return Kross::Api::Object::Ptr(0);
}
#endif

#include "krs_paint_layer.moc"
