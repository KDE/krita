/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#include "wavefilter.h"

#include <stdlib.h>
#include <vector>

#include <qpoint.h>

#include <kcombobox.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kstandarddirs.h>
#include <ktempfile.h>

#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_layer.h>
#include <kis_random_sub_accessor.h>
#include <kis_types.h>

#include "kis_wdg_wave.h"
#include "wdgwaveoptions.h"

typedef KGenericFactory<KritaWaveFilter> KritaWaveFilterFactory;
K_EXPORT_COMPONENT_FACTORY( kritawavefilter, KritaWaveFilterFactory( "krita" ) )

class KisWaveCurve {
    public:
        virtual double valueAt(int x, int y) =0;
};
        
class KisSinusoidalWaveCurve : public KisWaveCurve {
    public:
        KisSinusoidalWaveCurve(int amplitude, int wavelenght, int shift) : m_amplitude(amplitude), m_wavelength(wavelenght), m_shift(shift)
        {
        }
        virtual double valueAt(int x, int y)
        {
            return y + m_amplitude * cos( (double) ( m_shift + x) / m_wavelength );
        }
    private:
        int m_amplitude, m_wavelength, m_shift;
};

class KisTriangleWaveCurve : public KisWaveCurve {
    public:
        KisTriangleWaveCurve(int amplitude, int wavelenght, int shift) :  m_amplitude(amplitude), m_wavelength(wavelenght), m_shift(shift)
        {
        }
        virtual double valueAt(int x, int y)
        {
            return y +  m_amplitude * pow( -1, (m_shift + x) / m_wavelength )  * (0.5 - (double)( (m_shift + x) % m_wavelength ) / m_wavelength );
        }
    private:
        int m_amplitude, m_wavelength, m_shift;
};



KritaWaveFilter::KritaWaveFilter(QObject *parent, const char *name, const QStringList &)
        : KParts::Plugin(parent, name)
{
    setInstance(KritaWaveFilterFactory::instance());


    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(new KisFilterWave());
    }
}

KritaWaveFilter::~KritaWaveFilter()
{
}

KisFilterWave::KisFilterWave() : KisFilter(id(), "other", i18n("&Wave"))
{
}

KisFilterConfiguration* KisFilterWave::configuration(QWidget* w)
{
    KisWdgWave* wN = dynamic_cast<KisWdgWave*>(w);
    KisFilterConfiguration* config = new KisFilterConfiguration(id().id(), 1);
    if(wN)
    {
        config->setProperty("horizontalwavelength", wN->widget()->intHWavelength->value() );
        config->setProperty("horizontalshift", wN->widget()->intHShift->value() );
        config->setProperty("horizontalamplitude", wN->widget()->intHAmplitude->value() );
        config->setProperty("horizontalshape", wN->widget()->cbHShape->currentItem() );
        config->setProperty("verticalwavelength", wN->widget()->intVWavelength->value() );
        config->setProperty("verticalshift", wN->widget()->intVShift->value() );
        config->setProperty("verticalamplitude", wN->widget()->intVAmplitude->value() );
        config->setProperty("verticalshape", wN->widget()->cbVShape->currentItem() );
    }
    return config;
}

KisFilterConfigWidget * KisFilterWave::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP /*dev*/)
{
    return new KisWdgWave((KisFilter*)this, (QWidget*)parent, i18n("Configuration of wave filter").ascii());
}

void KisFilterWave::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* config, const QRect& rect)
{
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);
    
    setProgressTotalSteps(rect.width() * rect.height());

    KisColorSpace* cs = dst->colorSpace();
    
    QVariant value;
    int horizontalwavelength = (config->getProperty("horizontalwavelength", value)) ? value.toInt() : 50;
    int horizontalshift = (config->getProperty("horizontalshift", value)) ? value.toInt() : 50;
    int horizontalamplitude = (config->getProperty("horizontalamplitude", value)) ? value.toInt() : 4;
    int horizontalshape = (config->getProperty("horizontalshape", value)) ? value.toInt() : 0;
    int verticalwavelength = (config->getProperty("verticalwavelength", value)) ? value.toInt() : 50;
    int verticalshift = (config->getProperty("verticalshift", value)) ? value.toInt() : 50;
    int verticalamplitude = (config->getProperty("verticalamplitude", value)) ? value.toInt() : 4;
    int verticalshape = (config->getProperty("verticalshape", value)) ? value.toInt() : 0;
    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisWaveCurve* verticalcurve;
    if(verticalshape == 1)
        verticalcurve = new KisTriangleWaveCurve(verticalamplitude, verticalwavelength, verticalshift);
    else
        verticalcurve = new KisSinusoidalWaveCurve(verticalamplitude, verticalwavelength, verticalshift);
    KisWaveCurve* horizontalcurve;
    if(horizontalshape == 1)
        horizontalcurve = new KisTriangleWaveCurve(horizontalamplitude, horizontalwavelength, horizontalshift);
    else
        horizontalcurve = new KisSinusoidalWaveCurve(horizontalamplitude, horizontalwavelength, horizontalshift);
    KisRandomSubAccessorPixel srcRSA = src->createRandomSubAccessor();
    while(!dstIt.isDone())
    {
        double xv = horizontalcurve->valueAt( dstIt.y(), dstIt.x() );
        double yv = verticalcurve->valueAt( dstIt.x(), dstIt.y() );
        if( xv >= rect.left() && xv <= rect.right() && yv >= rect.top() && yv <= rect.bottom() )
        {
            srcRSA.moveTo( KisPoint( xv, yv ) );
            srcRSA.sampledOldRawData(dstIt.rawData());
        } else {
            cs->setAlpha( dstIt.rawData(), 0, 1);
        }
        ++dstIt;
        incProgress();
    }
    delete horizontalcurve;
    delete verticalcurve;
    setProgressDone(); // Must be called even if you don't really support progression
}
