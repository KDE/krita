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

#include <QBuffer>
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

bool PaintLayer::convertToColorspace(const QString& colorspacename)
{
    KoColorSpace * dstCS = KisMetaRegistry::instance()->csRegistry()->colorSpace(colorspacename, 0);
    if(!dstCS)
    {
        kWarning() << QString("Colorspace %1 is not available, please check your installation.").arg(colorspacename) << endl;
        return false;
    }
    paintLayer()->paintDevice()->convertTo(dstCS);
    return true;
}

QObject* PaintLayer::createRectIterator(uint x, uint y, uint width, uint height)
{
    return new Iterator<KisRectIteratorPixel>(
            paintLayer()->paintDevice()->createRectIterator(x, y, width, height, true),
            paintLayer());
}

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

QObject* PaintLayer::createHistogram(const QString& histoname, uint typenr)
{
    KisHistogramProducerFactory* factory = KisHistogramProducerFactoryRegistry::instance()->get(histoname);

    /*
    QList<KoID> listID = KisHistogramProducerFactoryRegistry::instance()->listKeys();
    for(QList<KoID>::iterator it = listID.begin(); it != listID.end(); it++)
        kDebug(41011) << (*it).name() << " " << (*it).id() << endl;
    */

    enumHistogramType type ;
    switch(typenr)
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
        return new Histogram(paintLayer(), factory->generate() , type);

    kWarning() << QString("An error has occured in %1\n%2").arg("createHistogram").arg( QString("The histogram %1 is not available").arg(histoname) );
    return 0;
}

QObject* PaintLayer::createPainter()
{
    return new Painter(paintLayer());
}

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

QObject* PaintLayer::fastWaveletTransformation()
{
    KisMathToolbox* mathToolbox = KisMetaRegistry::instance()->mtRegistry()->get( paintLayer()->paintDevice()->colorSpace()->mathToolboxID() );
    QRect rect = paintLayer()->exactBounds();
    KisMathToolbox::KisWavelet* wav = mathToolbox->fastWaveletTransformation(paintLayer()->paintDevice(), rect);
    return new Wavelet(wav);
}

bool PaintLayer::fastWaveletUntransformation(QObject* wavelet)
{
    Wavelet* wav = dynamic_cast< Wavelet* >(wavelet);
    if(! wav) {
        kWarning() << "The passed argument is not a valid Wavelet-object." << endl;
        return false;
    }
    KisMathToolbox* mathToolbox = KisMetaRegistry::instance()->mtRegistry()->get( paintLayer()->paintDevice()->colorSpace()->mathToolboxID() );
    QRect rect = paintLayer()->exactBounds();
    mathToolbox->fastWaveletUntransformation( paintLayer()->paintDevice(), rect, wav->wavelet() );
    return true;
}

QByteArray PaintLayer::bytes()
{
    qint32 pixelsize = paintLayer()->paintDevice()->colorSpace()->pixelSize();
    const int w = width();
    const int h = height();
    const int size = w * h * pixelsize;
    Q_ASSERT(size >= 0);

    QByteArray bytearray;
    QBuffer buffer(&bytearray);
    buffer.open(QIODevice::WriteOnly);
    QDataStream out(&buffer);

    quint8* data = new quint8[size];
    Q_CHECK_PTR(data);
    paintLayer()->paintDevice()->readBytes(data, 0, 0, w, h);
    for(int i = 0; i < size; ++i)
        out << data[i];
    delete [] data;

    kDebug()<<"PaintLayer::bytes width="<<w<<" height="<<h<<" pixelsize="<<pixelsize<<" size="<<size<<endl;
    return bytearray;
}

bool PaintLayer::setBytes(QByteArray bytearray)
{
    qint32 pixelsize = paintLayer()->paintDevice()->colorSpace()->pixelSize();
    const int w = width();
    const int h = height();
    const int size = w * h * pixelsize;

    if(size < 0 || bytearray.size() < size)
        return false;

    QBuffer buffer(&bytearray);
    buffer.open(QIODevice::ReadOnly);
    QDataStream in(&buffer);

    quint8* data = new quint8[size];
    Q_CHECK_PTR(data);
    for(int i = 0; i < size; ++i)
        in >> data[i];
    paintLayer()->paintDevice()->writeBytes(data, 0, 0, w, h);
    delete [] data;

    return true;
}

#include "krs_paint_layer.moc"
