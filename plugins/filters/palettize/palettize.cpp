/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2019 Carl Olsson <carl.olsson@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "palettize.h"

#include <kis_types.h>
#include <kpluginfactory.h>
#include <kis_config_widget.h>
#include <kis_filter_registry.h>
#include <kis_filter_configuration.h>
#include <kis_filter_category_ids.h>
#include <KoUpdater.h>
#include <KisSequentialIteratorProgress.h>
#include <KisResourceItemChooser.h>
#include <KoColorSet.h>
#include <KoPattern.h>
#include <kis_random_generator.h>
#include <KisDitherUtil.h>
#include <KisGlobalResourcesInterface.h>

K_PLUGIN_FACTORY_WITH_JSON(PalettizeFactory, "kritapalettize.json", registerPlugin<Palettize>();)

Palettize::Palettize(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisFilterPalettize());
}

#include "palettize.moc"


/*******************************************************************************/
/*                      KisFilterPalettizeConfiguration                        */
/*******************************************************************************/

class KisFilterPalettizeConfiguration : public KisFilterConfiguration
{
public:
    KisFilterPalettizeConfiguration(const QString & name, qint32 version, KisResourcesInterfaceSP resourcesInterface)
        : KisFilterConfiguration(name, version, resourcesInterface)
    {
    }

    KisFilterPalettizeConfiguration(const KisFilterPalettizeConfiguration &rhs)
        : KisFilterConfiguration(rhs)
    {
    }

    virtual KisFilterConfigurationSP clone() const override {
        return new KisFilterPalettizeConfiguration(*this);
    }

    KoColorSetSP palette(KisResourcesInterfaceSP resourcesInterface) const
    {
        auto source = resourcesInterface->source<KoColorSet>(ResourceType::Palettes);
        return source.resourceForName(this->getString("palette"));
    }

    KoColorSetSP palette() const
    {
        return palette(resourcesInterface());
    }

    QList<KoResourceSP> linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const override
    {
        KoColorSetSP palette = this->palette(globalResourcesInterface);

        QList<KoResourceSP> resources;
        if (palette) {
            resources << palette;
        }

        resources << KisDitherWidget::prepareLinkedResources(*this, "dither/", globalResourcesInterface);
        resources << KisDitherWidget::prepareLinkedResources(*this, "alphaDither/", globalResourcesInterface);

        return resources;
    }
};

/*******************************************************************************/
/*                      KisPalettizeWidget                                     */
/*******************************************************************************/

KisPalettizeWidget::KisPalettizeWidget(QWidget* parent)
    : KisConfigWidget(parent)
{
    Q_UNUSED(m_ditherPatternWidget);
    setupUi(this);

    paletteIconWidget->setFixedSize(32, 32);
    m_paletteWidget = new KisResourceItemChooser(ResourceType::Palettes, false, this);
    paletteIconWidget->setPopupWidget(m_paletteWidget);
    QObject::connect(m_paletteWidget, &KisResourceItemChooser::resourceSelected, paletteIconWidget, &KisIconWidget::setResource);
    QObject::connect(m_paletteWidget, &KisResourceItemChooser::resourceSelected, this, &KisConfigWidget::sigConfigurationItemChanged);

    QObject::connect(colorspaceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KisConfigWidget::sigConfigurationItemChanged);

    QObject::connect(ditherGroupBox, &QGroupBox::toggled, this, &KisConfigWidget::sigConfigurationItemChanged);

    QObject::connect(ditherWidget, &KisDitherWidget::sigConfigurationItemChanged, this, &KisConfigWidget::sigConfigurationItemChanged);

    QObject::connect(colorModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KisConfigWidget::sigConfigurationItemChanged);

    offsetScaleSpinBox->setPrefix(QString("%1  ").arg(i18n("Offset Scale:")));
    offsetScaleSpinBox->setRange(0.0, 1.0, 3);
    offsetScaleSpinBox->setSingleStep(0.125);
    QObject::connect(offsetScaleSpinBox, &KisDoubleSliderSpinBox::valueChanged, this, &KisConfigWidget::sigConfigurationItemChanged);

    QObject::connect(alphaGroupBox, &QGroupBox::toggled, this, &KisConfigWidget::sigConfigurationItemChanged);

    QObject::connect(alphaModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KisConfigWidget::sigConfigurationItemChanged);

    alphaClipSpinBox->setPrefix(QString("%1  ").arg(i18n("Clip:")));
    alphaClipSpinBox->setRange(0.0, 1.0, 3);
    alphaClipSpinBox->setSingleStep(0.125);
    QObject::connect(alphaClipSpinBox, &KisDoubleSliderSpinBox::valueChanged, this, &KisConfigWidget::sigConfigurationItemChanged);

    alphaIndexSpinBox->setPrefix(QString("%1  ").arg(i18nc("Index as in Index Color", "Index:")));
    alphaIndexSpinBox->setRange(0, 255);
    QObject::connect(alphaIndexSpinBox, &KisSliderSpinBox::valueChanged, this, &KisConfigWidget::sigConfigurationItemChanged);
    QObject::connect(m_paletteWidget, &KisResourceItemChooser::resourceSelected, [this](){
        const KoColorSetSP palette = m_paletteWidget->currentResource().staticCast<KoColorSet>();
        alphaIndexSpinBox->setMaximum(palette ? int(palette->colorCount() - 1) : 0);
        alphaIndexSpinBox->setValue(std::min(alphaIndexSpinBox->value(), alphaIndexSpinBox->maximum()));
    });

    QObject::connect(alphaDitherWidget, &KisDitherWidget::sigConfigurationItemChanged, this, &KisConfigWidget::sigConfigurationItemChanged);
}

void KisPalettizeWidget::setConfiguration(const KisPropertiesConfigurationSP _config)
{
    const KisFilterPalettizeConfiguration *config = dynamic_cast<const KisFilterPalettizeConfiguration*>(_config.data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(config);

    KoColorSetSP palette = config->palette();
    if (palette) m_paletteWidget->setCurrentResource(palette);
    colorspaceComboBox->setCurrentIndex(config->getInt("colorspace"));
    ditherGroupBox->setChecked(config->getBool("ditherEnabled"));
    ditherWidget->setConfiguration(*config, "dither/");
    colorModeComboBox->setCurrentIndex(config->getInt("dither/colorMode"));
    offsetScaleSpinBox->setValue(config->getDouble("dither/offsetScale"));
    alphaGroupBox->setChecked(config->getBool("alphaEnabled"));
    alphaModeComboBox->setCurrentIndex(config->getInt("alphaMode"));
    alphaClipSpinBox->setValue(config->getDouble("alphaClip"));
    alphaIndexSpinBox->setValue(config->getInt("alphaIndex"));
    alphaDitherWidget->setConfiguration(*config, "alphaDither/");
}

KisPropertiesConfigurationSP KisPalettizeWidget::configuration() const
{
    KisFilterSP filter = KisFilterRegistry::instance()->get("palettize");
    KisFilterConfigurationSP config = filter->factoryConfiguration(KisGlobalResourcesInterface::instance());

    if (m_paletteWidget->currentResource()) config->setProperty("palette", QVariant(m_paletteWidget->currentResource()->name()));
    config->setProperty("colorspace", colorspaceComboBox->currentIndex());
    config->setProperty("ditherEnabled", ditherGroupBox->isChecked());
    ditherWidget->configuration(*config, "dither/");
    config->setProperty("dither/colorMode", colorModeComboBox->currentIndex());
    config->setProperty("dither/offsetScale", offsetScaleSpinBox->value());
    config->setProperty("alphaEnabled", alphaGroupBox->isChecked());
    config->setProperty("alphaMode", alphaModeComboBox->currentIndex());
    config->setProperty("alphaClip", alphaClipSpinBox->value());
    config->setProperty("alphaIndex", alphaIndexSpinBox->value());
    alphaDitherWidget->configuration(*config, "alphaDither/");

    return config;
}

KisConfigWidget* KisFilterPalettize::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool useForMasks) const
{
    Q_UNUSED(dev);
    Q_UNUSED(useForMasks);

    return new KisPalettizeWidget(parent);
}

/*******************************************************************************/
/*                      KisFilterPalettize                                     */
/*******************************************************************************/

KisFilterPalettize::KisFilterPalettize() : KisFilter(id(), FiltersCategoryMapId, i18n("&Palettize..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
    setShowConfigurationWidget(true);
}

KisFilterConfigurationSP KisFilterPalettize::factoryConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    return new KisFilterPalettizeConfiguration("palettize", 1, resourcesInterface);
}


KisFilterConfigurationSP KisFilterPalettize::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);

    config->setProperty("palette", "Default");
    config->setProperty("colorspace", Colorspace::Lab);
    config->setProperty("ditherEnabled", false);
    KisDitherWidget::factoryConfiguration(*config, "dither/");
    config->setProperty("dither/colorMode", ColorMode::PerChannelOffset);
    config->setProperty("dither/offsetScale", 0.125);
    config->setProperty("alphaEnabled", true);
    config->setProperty("alphaMode", AlphaMode::Clip);
    config->setProperty("alphaClip", 0.5);
    config->setProperty("alphaIndex", 0);
    KisDitherWidget::factoryConfiguration(*config, "alphaDither/");

    return config;
}

void KisFilterPalettize::processImpl(KisPaintDeviceSP device, const QRect& applyRect, const KisFilterConfigurationSP _config, KoUpdater* progressUpdater) const
{
    const KisFilterPalettizeConfiguration *config = dynamic_cast<const KisFilterPalettizeConfiguration*>(_config.data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(config);
    KIS_SAFE_ASSERT_RECOVER_NOOP(config->hasLocalResourcesSnapshot());

    const KoColorSetSP palette = config->palette();

    const int searchColorspace = config->getInt("colorspace");
    const bool ditherEnabled = config->getBool("ditherEnabled");
    const int colorMode = config->getInt("dither/colorMode");
    const double offsetScale = config->getDouble("dither/offsetScale");
    const bool alphaEnabled = config->getBool("alphaEnabled");
    const int alphaMode = config->getInt("alphaMode");
    const double alphaClip = config->getDouble("alphaClip");
    const int alphaIndex = config->getInt("alphaIndex");

    const KoColorSpace* colorspace = device->colorSpace();
    const KoColorSpace* workColorspace = (searchColorspace == Colorspace::Lab
                                              ? KoColorSpaceRegistry::instance()->lab16()
                                              : KoColorSpaceRegistry::instance()->rgb16("sRGB-elle-V2-srgbtrc.icc"));

    const quint8 colorCount = ditherEnabled && colorMode == ColorMode::NearestColors ? 2 : 1;

    using SearchColor = boost::geometry::model::point<quint16, 3, boost::geometry::cs::cartesian>;
    struct ColorCandidate {
        KoColor color;
        quint16 index;
        double distance;
    };
    using SearchEntry = std::pair<SearchColor, ColorCandidate>;
    boost::geometry::index::rtree<SearchEntry, boost::geometry::index::quadratic<16>> rtree;

    if (palette) {
        // Add palette colors to search tree
        quint16 index = 0;
        for (int row = 0; row < palette->rowCount(); ++row) {
            for (int column = 0; column < palette->columnCount(); ++column) {
                KisSwatch swatch = palette->getColorGlobal(column, row);
                if (swatch.isValid()) {
                    KoColor color = swatch.color().convertedTo(colorspace);
                    KoColor workColor = swatch.color().convertedTo(workColorspace);
                    SearchColor searchColor;
                    memcpy(&searchColor, workColor.data(), sizeof(SearchColor));
                    // Don't add duplicates so won't dither between identical colors
                    std::vector<SearchEntry> result;
                    rtree.query(boost::geometry::index::contains(searchColor), std::back_inserter(result));
                    if (result.empty()) rtree.insert(SearchEntry(searchColor, {color, index, 0.0}));
                }
                ++index;
            }
        }

        KisDitherUtil ditherUtil;
        if (ditherEnabled) ditherUtil.setConfiguration(*config, "dither/");

        KisDitherUtil alphaDitherUtil;
        if (alphaMode == AlphaMode::Dither) alphaDitherUtil.setConfiguration(*config, "alphaDither/");

        KisSequentialIteratorProgress pixel(device, applyRect, progressUpdater);
        while (pixel.nextPixel()) {
            KoColor workColor(pixel.oldRawData(), colorspace);
            workColor.convertTo(workColorspace);

            // Find dither threshold
            double threshold = 0.5;
            if (ditherEnabled) {
                threshold = ditherUtil.threshold(QPoint(pixel.x(), pixel.y()));

                // Traditional per-channel ordered dithering
                if (colorMode == ColorMode::PerChannelOffset) {
                    QVector<float> normalized(int(workColorspace->channelCount()));
                    workColorspace->normalisedChannelsValue(workColor.data(), normalized);
                    for (int channel = 0; channel < int(workColorspace->channelCount()); ++channel) {
                        normalized[channel] += (threshold - 0.5) * offsetScale;
                    }
                    workColorspace->fromNormalisedChannelsValue(workColor.data(), normalized);
                }
            }

            // Get candidate colors and their distances
            SearchColor searchColor;
            memcpy(reinterpret_cast<quint8 *>(&searchColor), workColor.data(), sizeof(SearchColor));
            std::vector<ColorCandidate> candidateColors;
            candidateColors.reserve(size_t(colorCount));
            double distanceSum = 0.0;
            for (auto it = rtree.qbegin(boost::geometry::index::nearest(searchColor, colorCount)); it != rtree.qend() && candidateColors.size() < colorCount; ++it) {
                ColorCandidate candidate = it->second;
                candidate.distance = boost::geometry::distance(searchColor, it->first);
                candidateColors.push_back(candidate);
                distanceSum += candidate.distance;
            }

            // Select color candidate
            quint16 selected;
            if (ditherEnabled && colorMode == ColorMode::NearestColors) {
                // Sort candidates by palette order for stable dither color ordering
                const bool swap = candidateColors[0].index > candidateColors[1].index;
                selected = swap ^ (candidateColors[swap].distance / distanceSum > threshold);
            }
            else {
                selected = 0;
            }
            ColorCandidate &candidate = candidateColors[selected];

            // Set alpha
            const double oldAlpha = colorspace->opacityF(pixel.oldRawData());
            double newAlpha = oldAlpha;
            if (alphaEnabled && !(!ditherEnabled && alphaMode == AlphaMode::Dither)) {
                if (alphaMode == AlphaMode::Clip) {
                    newAlpha = oldAlpha < alphaClip? 0.0 : 1.0;
                }
                else if (alphaMode == AlphaMode::Index) {
                    newAlpha = (candidate.index == alphaIndex ? 0.0 : 1.0);
                }
                else if (alphaMode == AlphaMode::Dither) {
                    newAlpha = oldAlpha < alphaDitherUtil.threshold(QPoint(pixel.x(), pixel.y())) ? 0.0 : 1.0;
                }
            }
            colorspace->setOpacity(candidate.color.data(), newAlpha, 1);

            // Copy color to pixel
            memcpy(pixel.rawData(), candidate.color.data(), colorspace->pixelSize());
        }
    }
}
