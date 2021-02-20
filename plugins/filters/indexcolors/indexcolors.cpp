/*
 * SPDX-FileCopyrightText: 2014 Manuel Riecke <spell1337@gmail.com>
 *
 * SPDX-License-Identifier: ICS
 */

#include "indexcolors.h"

#include <kpluginfactory.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <KoColorSpaceMaths.h>
#include <KoColorSpaceRegistry.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_color_transformation_configuration.h>
#include <widgets/kis_multi_integer_filter_widget.h>

#include "kiswdgindexcolors.h"
#include "palettegeneratorconfig.h"

K_PLUGIN_FACTORY_WITH_JSON(IndexColorsFactory, "kritaindexcolors.json", registerPlugin<IndexColors>();)

IndexColors::IndexColors(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisFilterRegistry::instance()->add(KisFilterSP(new KisFilterIndexColors()));
}

IndexColors::~IndexColors()
{
}

KisFilterIndexColors::KisFilterIndexColors() : KisColorTransformationFilter(id(), FiltersCategoryArtisticId, i18n("&Index Colors..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT); // Technically it is TO_LAB16 but that would only display a warning we don't want
    // This filter will always degrade the color space, that is it's purpose
    setSupportsPainting(true);
    setShowConfigurationWidget(true);
}

KoColorTransformation* KisFilterIndexColors::createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const
{
    IndexColorPalette pal;

    PaletteGeneratorConfig palCfg;
    palCfg.fromByteArray(config->getProperty("paletteGen").toByteArray());
    pal = palCfg.generate();
    if(config->getBool("reduceColorsEnabled"))
    {
        int maxClrs = config->getInt("colorLimit");
        while(pal.numColors() > maxClrs)
            pal.mergeMostReduantColors();
    }

    pal.similarityFactors.L = config->getFloat("LFactor");
    pal.similarityFactors.a = config->getFloat("aFactor");
    pal.similarityFactors.b = config->getFloat("bFactor");
    return new KisIndexColorTransformation(pal, cs, config->getInt("alphaSteps"));
}

KisConfigWidget* KisFilterIndexColors::createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev, bool) const
{
    Q_UNUSED(dev);
    KisWdgIndexColors* w = new KisWdgIndexColors(parent);
    w->setup(
        QStringList() << i18nc("Color palette shade", "Bright") << i18nc("Color palette shade", "Light") << i18nc("Color palette shade", "Base") << i18nc("Color palette shade", "Shadow")
        , 4
    );
    return w;
}

KisFilterConfigurationSP KisFilterIndexColors::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);

    PaletteGeneratorConfig palCfg; // Default constructor is factory config
    config->setProperty("paletteGen",     palCfg.toByteArray());

    config->setProperty("LFactor",    1.f);
    config->setProperty("aFactor",    1.f);
    config->setProperty("bFactor",    1.f);
    config->setProperty("reduceColorsEnabled", false);
    config->setProperty("colorLimit", 32);
    config->setProperty("alphaSteps", 1);
    return config;
}

KisIndexColorTransformation::KisIndexColorTransformation(IndexColorPalette palette, const KoColorSpace* cs, int alphaSteps)
    : m_colorSpace(cs),
      m_psize(cs->pixelSize())
{
    m_palette = palette;

    static const qreal max = KoColorSpaceMathsTraits<quint16>::max;
    if(alphaSteps > 0)
    {
        m_alphaStep = max / alphaSteps;
        m_alphaHalfStep = m_alphaStep / 2;
    }
    else
    {
        m_alphaStep = 0;
        m_alphaHalfStep = 0;
    }
}

void KisIndexColorTransformation::transform(const quint8* src, quint8* dst, qint32 nPixels) const
{
    union
    {
        quint16 laba[4];
        LabColor lab;
    } clr;
    while (nPixels--)
    {
        m_colorSpace->toLabA16(src, reinterpret_cast<quint8 *>(clr.laba), 1);
        clr.lab = m_palette.getNearestIndex(clr.lab);
        if(m_alphaStep)
        {
            quint16 amod = clr.laba[3] % m_alphaStep;
            clr.laba[3] = clr.laba[3] + (amod > m_alphaHalfStep ? m_alphaStep - amod : -amod);
        }
        m_colorSpace->fromLabA16(reinterpret_cast<quint8 *>(clr.laba), dst, 1);
        src += m_psize;
        dst += m_psize;
    }
}

#include "indexcolors.moc"
