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
        virtual KisPoint valueAt(int x, int y) =0;
};
        
class KisSinusoidalWaveCurve : public KisWaveCurve {
    public:
        KisSinusoidalWaveCurve(int amplitude, int wavelenght, int shift, int ystart, int height) : m_amplitude(amplitude), m_wavelength(wavelenght), m_shift(shift), m_ystart(ystart), m_yend(m_ystart + height - 1)
        {
        }
        virtual KisPoint valueAt(int x, int y)
        {
            double yv = y + m_amplitude * cos( (double) ( m_shift + x) / m_wavelength );
            if(yv < m_ystart) yv = m_ystart;
            else if(yv > m_yend) yv = m_yend;
            return KisPoint(x, yv );
        }
    private:
        int m_amplitude, m_wavelength, m_shift, m_ystart, m_yend;
};

class KisTriangleWaveCurve : public KisWaveCurve {
    public:
        KisTriangleWaveCurve(int amplitude, int wavelenght, int shift, int ystart, int height) :  m_amplitude(amplitude), m_wavelength(wavelenght), m_shift(shift), m_ystart(ystart), m_yend(m_ystart + height - 1)
        {
        }
        virtual KisPoint valueAt(int x, int y)
        {
            double yv = y +  m_amplitude * pow( -1, (m_shift + x) / m_wavelength )  * (0.5 - (double)( (m_shift + x) % m_wavelength ) / m_wavelength );
            if(yv < m_ystart) yv = m_ystart;
            else if(yv > m_yend) yv = m_yend;
            return KisPoint(x, yv );
        }
    private:
        int m_amplitude, m_wavelength, m_shift, m_ystart, m_yend;
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
        config->setProperty("wavelength", wN->widget()->intWavelength->value() );
        config->setProperty("shift", wN->widget()->intShift->value() );
        config->setProperty("amplitude", wN->widget()->intAmplitude->value() );
        config->setProperty("shape", wN->widget()->cbShape->currentItem() );
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

    QVariant value;
    int wavelength = (config->getProperty("wavelength", value)) ? value.toInt() : 50;
    int shift = (config->getProperty("shift", value)) ? value.toInt() : 50;
    int amplitude = (config->getProperty("amplitude", value)) ? value.toInt() : 4;
    int shape = (config->getProperty("shape", value)) ? value.toInt() : 0;
    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisWaveCurve* curve;
    if(shape == 1)
        curve = new KisTriangleWaveCurve(amplitude, wavelength, shift, rect.y(), rect.height());
    else
        curve = new KisSinusoidalWaveCurve(amplitude, wavelength, shift, rect.y(), rect.height());
    KisRandomSubAccessorPixel srcRSA = src->createRandomSubAccessor();
    while(!dstIt.isDone())
    {
        srcRSA.moveTo( curve->valueAt( dstIt.x(), dstIt.y() ) );
//         kdDebug() << dstIt.x() << " " << dstIt.y() << " " << curve->valueAt( dstIt.x(), dstIt.y() ).x() << " " << curve->valueAt( dstIt.x(), dstIt.y() ).y() << endl;
        srcRSA.sampledOldRawData(dstIt.rawData());
        ++dstIt;
        incProgress();
    }
    delete curve;
    setProgressDone(); // Must be called even if you don't really support progression
}
