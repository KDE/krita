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
#include <math.h>
#include <QPoint>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <klocalizedstring.h>

#include <KoUpdater.h>

#include <kis_image.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_layer.h>
#include <kis_random_sub_accessor.h>
#include <kis_selection.h>
#include <kis_types.h>
#include <kis_paint_device.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_configuration.h>
#include <kis_processing_information.h>
#include "kis_wdg_wave.h"
#include "ui_wdgwaveoptions.h"
#include <kis_iterator_ng.h>
#include <KisSequentialIteratorProgress.h>

K_PLUGIN_FACTORY_WITH_JSON(KritaWaveFilterFactory, "kritawavefilter.json", registerPlugin<KritaWaveFilter>();)

class KisWaveCurve
{
public:
    virtual ~KisWaveCurve() {}
    virtual double valueAt(int x, int y) = 0;
};

class KisSinusoidalWaveCurve : public KisWaveCurve
{
public:

    KisSinusoidalWaveCurve(int amplitude, int wavelength, int shift) : m_amplitude(amplitude), m_wavelength(wavelength), m_shift(shift) {
    }

    ~KisSinusoidalWaveCurve() override {}

    double valueAt(int x, int y) override {
        return y + m_amplitude * cos((double)(m_shift + x) / m_wavelength);
    }
private:
    int m_amplitude, m_wavelength, m_shift;
};

class KisTriangleWaveCurve : public KisWaveCurve
{
public:

    KisTriangleWaveCurve(int amplitude, int wavelength, int shift) :  m_amplitude(amplitude), m_wavelength(wavelength), m_shift(shift) {
    }

    ~KisTriangleWaveCurve() override {}

    double valueAt(int x, int y) override {
        return y +  m_amplitude * pow(-1.0, (m_shift + x) / m_wavelength)  *(0.5 - (double)((m_shift + x) % m_wavelength) / m_wavelength);
    }
private:
    int m_amplitude, m_wavelength, m_shift;
}; KritaWaveFilter::KritaWaveFilter(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisFilterWave());
}

KritaWaveFilter::~KritaWaveFilter()
{
}

KisFilterWave::KisFilterWave() : KisFilter(id(), FiltersCategoryOtherId, i18n("&Wave..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(false);
    setSupportsAdjustmentLayers(false);

}

KisFilterConfigurationSP KisFilterWave::defaultConfiguration() const
{
    KisFilterConfigurationSP config = factoryConfiguration();
    config->setProperty("horizontalwavelength", 50);
    config->setProperty("horizontalshift", 50);
    config->setProperty("horizontalamplitude", 4);
    config->setProperty("horizontalshape", 0);
    config->setProperty("verticalwavelength", 50);
    config->setProperty("verticalshift", 50);
    config->setProperty("verticalamplitude", 4);
    config->setProperty("verticalshape", 0);
    return config;
}

KisConfigWidget * KisFilterWave::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP, bool) const
{
    return new KisWdgWave((KisFilter*)this, (QWidget*)parent);
}

void KisFilterWave::processImpl(KisPaintDeviceSP device,
                                const QRect& applyRect,
                                const KisFilterConfigurationSP config,
                                KoUpdater* progressUpdater
                                ) const
{
    Q_ASSERT(device.data() != 0);

    QVariant value;
    int horizontalwavelength = (config && config->getProperty("horizontalwavelength", value)) ? value.toInt() : 50;
    int horizontalshift = (config && config->getProperty("horizontalshift", value)) ? value.toInt() : 50;
    int horizontalamplitude = (config && config->getProperty("horizontalamplitude", value)) ? value.toInt() : 4;
    int horizontalshape = (config && config->getProperty("horizontalshape", value)) ? value.toInt() : 0;
    int verticalwavelength = (config && config->getProperty("verticalwavelength", value)) ? value.toInt() : 50;
    int verticalshift = (config && config->getProperty("verticalshift", value)) ? value.toInt() : 50;
    int verticalamplitude = (config && config->getProperty("verticalamplitude", value)) ? value.toInt() : 4;
    int verticalshape = (config && config->getProperty("verticalshape", value)) ? value.toInt() : 0;

    KisWaveCurve* verticalcurve;
    if (verticalshape == 1)
        verticalcurve = new KisTriangleWaveCurve(verticalamplitude, verticalwavelength, verticalshift);
    else
        verticalcurve = new KisSinusoidalWaveCurve(verticalamplitude, verticalwavelength, verticalshift);
    KisWaveCurve* horizontalcurve;
    if (horizontalshape == 1)
        horizontalcurve = new KisTriangleWaveCurve(horizontalamplitude, horizontalwavelength, horizontalshift);
    else
        horizontalcurve = new KisSinusoidalWaveCurve(horizontalamplitude, horizontalwavelength, horizontalshift);
    
    KisSequentialIteratorProgress dstIt(device, applyRect, progressUpdater);
    KisRandomSubAccessorSP srcRSA = device->createRandomSubAccessor();
    while (dstIt.nextPixel()) {
        double xv = horizontalcurve->valueAt(dstIt.y(), dstIt.x());
        double yv = verticalcurve->valueAt(dstIt.x(), dstIt.y());
        srcRSA->moveTo(QPointF(xv, yv));
        srcRSA->sampledOldRawData(dstIt.rawData());
    }
    delete horizontalcurve;
    delete verticalcurve;
}

QRect KisFilterWave::neededRect(const QRect& rect, const KisFilterConfigurationSP config, int lod) const
{
    Q_UNUSED(lod);

    QVariant value;
    int horizontalamplitude = (config && config->getProperty("horizontalamplitude", value)) ? value.toInt() : 4;
    int verticalamplitude = (config && config->getProperty("verticalamplitude", value)) ? value.toInt() : 4;
    return rect.adjusted(-horizontalamplitude, -verticalamplitude, horizontalamplitude, verticalamplitude);
}

#include "wavefilter.moc"
