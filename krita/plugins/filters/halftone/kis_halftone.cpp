/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_halftone.h"

#include <cmath>

#include <kgenericfactory.h>

#include <kis_autobrush_resource.h>
#include <kis_convolution_painter.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_math_toolbox.h>
#include <kis_meta_registry.h>
#include <kis_multi_integer_filter_widget.h>
#include <kis_paint_device.h>
#include <kis_transaction.h>
    
    typedef KGenericFactory<KritaHalftone> KritaHalftoneFactory;
K_EXPORT_COMPONENT_FACTORY( kritahalftone, KritaHalftoneFactory( "krita" ) )

        KritaHalftone::KritaHalftone(QObject *parent, const char *name, const QStringList &)
  : KParts::Plugin(parent, name)
{
    setInstance(KritaHalftoneFactory::instance());

    kdDebug(41006) << "Halftone filter plugin. Class: "
           << className()
           << ", Parent: "
           << parent->className()
           << "\n";


    if ( parent->inherits("KisFilterRegistry") )
    {
        KisFilterRegistry * r = dynamic_cast<KisFilterRegistry*>(parent);
        r->add(new KisHalftoneReduction());
    }
}

KritaHalftone::~KritaHalftone()
{
}

    
    
KisHalftoneReduction::KisHalftoneReduction()
    : KisFilter(id(), "enhance", i18n("Halftone Reduction..."))
{
}


KisHalftoneReduction::~KisHalftoneReduction()
{
}

KisFilterConfigWidget * KisHalftoneReduction::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP )
{
    vKisIntegerWidgetParam param;
    param.push_back( KisIntegerWidgetParam( 0, 100, BEST_WAVELET_FREQUENCY_VALUE, i18n("Frequency"), "frequency" ) );
    param.push_back( KisIntegerWidgetParam( 0, 100, 2, i18n("Half-size"), "halfsize" ) );
    return new KisMultiIntegerFilterWidget(parent, id().id().ascii(), id().id().ascii(), param );
}

KisFilterConfiguration* KisHalftoneReduction::configuration(QWidget* nwidget )
{
    KisMultiIntegerFilterWidget* widget = (KisMultiIntegerFilterWidget*) nwidget;
    if( widget == 0 )
    {
        return new KisHalftoneReductionConfiguration( BEST_WAVELET_FREQUENCY_VALUE, 2 );
    } else {
        return new KisHalftoneReductionConfiguration( widget->valueAt( 0 ), widget->valueAt( 1 ) );
    }
}

void KisHalftoneReduction::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* config, const QRect& rect)
{

    float frequency = 1.0;
    int halfSize = 2;
    
    if(config !=0)
    {
        KisHalftoneReductionConfiguration* configWNRC = (KisHalftoneReductionConfiguration*)config;
        frequency = configWNRC->getDouble("frequency");
        halfSize = configWNRC->getInt("halfsize");
        kdDebug(41005) << "frequency: " << frequency << endl;
    }


    Q_INT32 depth = src->colorSpace()->nColorChannels();

    int size;
    int maxrectsize = (rect.height() < rect.width()) ? rect.width() : rect.height();
    for(size = 2; size < maxrectsize; size *= 2) ;

    KisMathToolbox* mathToolbox = KisMetaRegistry::instance()->mtRegistry()->get( src->colorSpace()->mathToolboxID() );
    setProgressTotalSteps(mathToolbox->fastWaveletTotalSteps(rect) * 2 + size*size*depth );
    connect(mathToolbox, SIGNAL(nextStep()), this, SLOT(incProgress()));


    kdDebug(41005) << size << " " << maxrectsize << " " << rect.x() << " " << rect.y() << endl;

    kdDebug(41005) << halfSize << endl;
    KisAutobrushShape* kas = new KisAutobrushCircleShape(2*halfSize+1, 2*halfSize+1 , halfSize, halfSize);
    
    QImage mask;
    kas->createBrush(&mask);
    
    KisKernelSP kernel = KisKernel::fromQImage(mask); // TODO: for 1.6 reuse the krita's core function for creating kernel : KisKernel::fromQImage
    mask.save("testmask.png", "PNG");
    
    KisPaintDeviceSP interm = new KisPaintDevice(*src);
    KisColorSpace * cs = src->colorSpace();

    KisConvolutionPainter painter( interm );
    painter.beginTransaction("bouuh");
    painter.applyMatrix(kernel, rect.x(), rect.y(), rect.width(), rect.height(), BORDER_REPEAT);
    
//     KisPaintDeviceSP interm2 = new KisPaintDevice(*interm);
    
    
    kdDebug(41005) << "Transforming..." << endl;
    setProgressStage( i18n("Fast wavelet transformation") ,progress());
    KisMathToolbox::KisWavelet* buff = 0;
    KisMathToolbox::KisWavelet* wav = 0;
    KisMathToolbox::KisWavelet* blurwav = 0;
    try {
        buff = mathToolbox->initWavelet(src, rect);
    } catch(std::bad_alloc)
    {
        if(buff) delete buff;
        return;
    }
    try {
        wav = mathToolbox->fastWaveletTransformation(src, rect, buff);
        blurwav = mathToolbox->fastWaveletTransformation(interm, rect, buff);
    } catch(std::bad_alloc)
    {
        if(wav) delete wav;
        return;
    }

    kdDebug(41005) << "frequencying..." << endl;
//     float* fin = wav->coeffs + wav->depth*wav->size*wav->size;
//     setProgressStage( i18n("frequencying") ,progress());
//     float* it2 = wav->coeffs + wav->depth;
    int sizecopy = (int)pow(2, frequency);
    if(sizecopy > wav->size) sizecopy = wav->size;
    for(int i = 0; i < sizecopy; i++)
    {
      float *itNotblured = wav->coeffs + wav->depth * wav->size * i;
      float *itBlured = blurwav->coeffs + blurwav->depth * blurwav->size * i;
      for(int j = 0; j< sizecopy; j++)
      {
        memcpy(itBlured, itNotblured, sizeof(float) * blurwav->depth);
        itBlured += blurwav->depth;
        itNotblured +=  wav->depth;
      }
    }

    kdDebug(41005) << "Untransforming..." << endl;

    setProgressStage( i18n("Fast wavelet untransformation") ,progress());
    mathToolbox->fastWaveletUntransformation( dst, rect, blurwav, buff);

    delete wav;
    delete blurwav;
    delete buff;
    disconnect(mathToolbox, SIGNAL(nextStep()), this, SLOT(incProgress()));

    setProgressDone(); // Must be called even if you don't really support progression
}
