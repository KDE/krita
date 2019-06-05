/*
 *  Copyright (c) 2014 Manuel Riecke <spell1337@gmail.com>
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

#include "posterize.h"
#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QTime>

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_processing_information.h>
#include <kis_types.h>
#include <kis_selection.h>
#include <kis_layer.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>

#include <KoColorSpaceMaths.h>
#include <filter/kis_color_transformation_configuration.h>
#include <widgets/kis_multi_integer_filter_widget.h>

K_PLUGIN_FACTORY_WITH_JSON(PosterizeFactory, "kritaposterize.json", registerPlugin<Posterize>();)

Posterize::Posterize(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisFilterRegistry::instance()->add(KisFilterSP(new KisFilterPosterize()));
}

Posterize::~Posterize()
{
}

KisFilterPosterize::KisFilterPosterize() : KisColorTransformationFilter(id(), FiltersCategoryArtisticId, i18n("&Posterize..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
    setShowConfigurationWidget(true);
}

KoColorTransformation* KisFilterPosterize::createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const
{
    return new KisPosterizeColorTransformation(config->getInt("steps", 16), cs);
}

KisPosterizeColorTransformation::KisPosterizeColorTransformation(int steps, const KoColorSpace* cs) : m_colorSpace(cs), m_psize(cs->pixelSize())
{
    m_step = KoColorSpaceMathsTraits<quint16>::max / steps;
    m_halfStep = m_step / 2;
    m_fromConversion = KoColorSpaceRegistry::instance()->createColorConverter(
        m_colorSpace,
        KoColorSpaceRegistry::instance()->rgb16("sRGB-elle-V2-srgbtrc.icc"),
        KoColorConversionTransformation::internalRenderingIntent(),
        KoColorConversionTransformation::internalConversionFlags());
    m_toConversion = KoColorSpaceRegistry::instance()->createColorConverter(
        KoColorSpaceRegistry::instance()->rgb16("sRGB-elle-V2-srgbtrc.icc"),
        m_colorSpace,
        KoColorConversionTransformation::internalRenderingIntent(),
        KoColorConversionTransformation::internalConversionFlags());
}

KisPosterizeColorTransformation::~KisPosterizeColorTransformation()
{
    delete m_fromConversion;
    delete m_toConversion;
}

KisConfigWidget* KisFilterPosterize::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    vKisIntegerWidgetParam param;
    param.push_back(KisIntegerWidgetParam(2, 128, 16, i18n("Steps"), "steps"));
    return new KisMultiIntegerFilterWidget(id().id(), parent, id().id(), param);
}

KisFilterConfigurationSP KisFilterPosterize::factoryConfiguration() const
{
    KisColorTransformationConfigurationSP config = new KisColorTransformationConfiguration(id().id(), 0);
    config->setProperty("steps", 16);
    return config;
}

void KisPosterizeColorTransformation::transform(const quint8* src, quint8* dst, qint32 nPixels) const
{
    quint16 m_rgba[4];
    quint16 m_mod[4];

    while (nPixels--) {
        m_fromConversion->transform(src, reinterpret_cast<quint8 *>(m_rgba), 1);

        m_mod[0] = m_rgba[0] % m_step;
        m_mod[1] = m_rgba[1] % m_step;
        m_mod[2] = m_rgba[2] % m_step;
        m_mod[3] = m_rgba[3] % m_step;

        m_rgba[0] = m_rgba[0] + (m_mod[0] > m_halfStep ? m_step - m_mod[0] : -m_mod[0]);
        m_rgba[1] = m_rgba[1] + (m_mod[1] > m_halfStep ? m_step - m_mod[1] : -m_mod[1]);
        m_rgba[2] = m_rgba[2] + (m_mod[2] > m_halfStep ? m_step - m_mod[2] : -m_mod[2]);
        m_rgba[3] = m_rgba[3] + (m_mod[3] > m_halfStep ? m_step - m_mod[3] : -m_mod[3]);

        m_toConversion->transform(reinterpret_cast<quint8 *>(m_rgba), dst, 1);
        src += m_psize;
        dst += m_psize;
    }
}

#include "posterize.moc"
