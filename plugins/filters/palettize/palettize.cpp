/*
 * This file is part of Krita
 *
 * Copyright (c) 2019 Carl Olsson <carl.olsson@gmail.com>
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

#include "palettize.h"

#include <kis_types.h>
#include <kpluginfactory.h>
#include <kis_config_widget.h>
#include <kis_filter_registry.h>
#include <kis_filter_configuration.h>
#include <kis_filter_category_ids.h>
#include <kis_filter_configuration.h>
#include <KoUpdater.h>
#include <KisSequentialIteratorProgress.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServer.h>
#include <KoResourceServerAdapter.h>
#include <KoResourceItemChooser.h>
#include <KoColorSet.h>
#include <KoPattern.h>
#include <kis_random_generator.h>

K_PLUGIN_FACTORY_WITH_JSON(PalettizeFactory, "kritapalettize.json", registerPlugin<Palettize>();)

Palettize::Palettize(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisFilterPalettize());
}

#include "palettize.moc"

KisFilterPalettize::KisFilterPalettize() : KisFilter(id(), FiltersCategoryMapId, i18n("&Palettize..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
    setShowConfigurationWidget(true);
}

KisPalettizeWidget::KisPalettizeWidget(QWidget* parent)
    : KisConfigWidget(parent)
{
    setupUi(this);

    paletteIconWidget->setFixedSize(32, 32);
    KoResourceServer<KoColorSet>* paletteServer = KoResourceServerProvider::instance()->paletteServer();
    QSharedPointer<KoAbstractResourceServerAdapter> paletteAdapter(new KoResourceServerAdapter<KoColorSet>(paletteServer));
    m_paletteWidget = new KoResourceItemChooser(paletteAdapter, this, false);
    paletteIconWidget->setPopupWidget(m_paletteWidget);
    QObject::connect(m_paletteWidget, &KoResourceItemChooser::resourceSelected, paletteIconWidget, &KisIconWidget::setResource);
    QObject::connect(m_paletteWidget, &KoResourceItemChooser::resourceSelected, this, &KisConfigWidget::sigConfigurationItemChanged);

    QObject::connect(colorspaceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KisConfigWidget::sigConfigurationItemChanged);

    QObject::connect(ditherGroupBox, &QGroupBox::toggled, this, &KisConfigWidget::sigConfigurationItemChanged);

    patternIconWidget->setFixedSize(32, 32);
    KoResourceServer<KoPattern>* patternServer = KoResourceServerProvider::instance()->patternServer();
    QSharedPointer<KoAbstractResourceServerAdapter> patternAdapter(new KoResourceServerAdapter<KoPattern>(patternServer));
    m_ditherPatternWidget = new KoResourceItemChooser(patternAdapter, this, false);
    patternIconWidget->setPopupWidget(m_ditherPatternWidget);
    QObject::connect(m_ditherPatternWidget, &KoResourceItemChooser::resourceSelected, patternIconWidget, &KisIconWidget::setResource);
    QObject::connect(m_ditherPatternWidget, &KoResourceItemChooser::resourceSelected, this, &KisConfigWidget::sigConfigurationItemChanged);

    QObject::connect(patternValueModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KisConfigWidget::sigConfigurationItemChanged);

    noiseSeedLineEdit->setValidator(new QIntValidator(this));
    QObject::connect(noiseSeedLineEdit, &QLineEdit::textChanged, this, &KisConfigWidget::sigConfigurationItemChanged);

    QObject::connect(noiseSeedRandomizeButton, &QToolButton::clicked, [this](){
        noiseSeedLineEdit->setText(QString::number(rand()));
    });

    QObject::connect(thresholdModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KisConfigWidget::sigConfigurationItemChanged);

    QObject::connect(colorModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KisConfigWidget::sigConfigurationItemChanged);

    offsetScaleSpinBox->setPrefix(QString("%1  ").arg(i18n("Offset Scale:")));
    offsetScaleSpinBox->setRange(0.0, 1.0, 3);
    offsetScaleSpinBox->setSingleStep(0.125);
    QObject::connect(offsetScaleSpinBox, &KisDoubleSliderSpinBox::valueChanged, this, &KisConfigWidget::sigConfigurationItemChanged);

    spreadSpinBox->setPrefix(QString("%1  ").arg(i18n("Spread:")));
    spreadSpinBox->setRange(0.0, 1.0, 3);
    spreadSpinBox->setSingleStep(0.125);
    QObject::connect(spreadSpinBox, &KisDoubleSliderSpinBox::valueChanged, this, &KisConfigWidget::sigConfigurationItemChanged);

    colorCountSpinBox->setPrefix(QString("%1  ").arg(i18n("Colors:")));
    colorCountSpinBox->setRange(2, 4); // Limit to 4 colours is arbitrary, but larger values give universally poor results
    QObject::connect(colorCountSpinBox, &KisSliderSpinBox::valueChanged, this, &KisConfigWidget::sigConfigurationItemChanged);

    QObject::connect(alphaGroupBox, &QGroupBox::toggled, this, &KisConfigWidget::sigConfigurationItemChanged);

    QObject::connect(alphaModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &KisConfigWidget::sigConfigurationItemChanged);

    alphaClipSpinBox->setPrefix(QString("%1  ").arg(i18n("Clip:")));
    alphaClipSpinBox->setRange(0.0, 1.0, 3);
    alphaClipSpinBox->setSingleStep(0.125);
    QObject::connect(alphaClipSpinBox, &KisDoubleSliderSpinBox::valueChanged, this, &KisConfigWidget::sigConfigurationItemChanged);

    alphaIndexSpinBox->setPrefix(QString("%1  ").arg(i18n("Index:")));
    alphaIndexSpinBox->setRange(0, 255); // Limit to 4 colours is arbitrary, but larger values give universally poor results
    QObject::connect(alphaIndexSpinBox, &KisSliderSpinBox::valueChanged, this, &KisConfigWidget::sigConfigurationItemChanged);
}

void KisPalettizeWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    KoColorSet* palette = KoResourceServerProvider::instance()->paletteServer()->resourceByName(config->getString("palette"));
    if (palette) m_paletteWidget->setCurrentResource(palette);
    colorspaceComboBox->setCurrentIndex(config->getInt("colorspace"));
    ditherGroupBox->setChecked(config->getBool("ditherEnabled"));
    thresholdModeComboBox->setCurrentIndex(config->getInt("thresholdMode"));
    KoPattern* pattern = KoResourceServerProvider::instance()->patternServer()->resourceByName(config->getString("pattern"));
    if (pattern) m_ditherPatternWidget->setCurrentResource(pattern);
    patternValueModeComboBox->setCurrentIndex(config->getInt("patternValueMode"));
    noiseSeedLineEdit->setText(QString::number(config->getInt("noiseSeed")));
    colorModeComboBox->setCurrentIndex(config->getInt("colorMode"));
    offsetScaleSpinBox->setValue(config->getDouble("offsetScale"));
    spreadSpinBox->setValue(config->getDouble("spread"));
    colorCountSpinBox->setValue(config->getInt("colorCount"));
    alphaGroupBox->setChecked(config->getBool("alphaEnabled"));
    alphaModeComboBox->setCurrentIndex(config->getInt("alphaMode"));
    alphaClipSpinBox->setValue(config->getDouble("alphaClip"));
    alphaIndexSpinBox->setValue(config->getInt("alphaIndex"));
}

KisPropertiesConfigurationSP KisPalettizeWidget::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("palettize", 1);

    if (m_paletteWidget->currentResource()) config->setProperty("palette", QVariant(m_paletteWidget->currentResource()->name()));
    config->setProperty("colorspace", colorspaceComboBox->currentIndex());
    config->setProperty("ditherEnabled", ditherGroupBox->isChecked());
    config->setProperty("thresholdMode",thresholdModeComboBox->currentIndex());
    if (m_ditherPatternWidget->currentResource()) config->setProperty("pattern", QVariant(m_ditherPatternWidget->currentResource()->name()));
    config->setProperty("patternValueMode", patternValueModeComboBox->currentIndex());
    config->setProperty("noiseSeed", noiseSeedLineEdit->text().toInt());
    config->setProperty("colorMode", colorModeComboBox->currentIndex());
    config->setProperty("offsetScale", offsetScaleSpinBox->value());
    config->setProperty("spread", spreadSpinBox->value());
    config->setProperty("colorCount", colorCountSpinBox->value());
    config->setProperty("alphaEnabled", alphaGroupBox->isChecked());
    config->setProperty("alphaMode", alphaModeComboBox->currentIndex());
    config->setProperty("alphaClip", alphaClipSpinBox->value());
    config->setProperty("alphaIndex", alphaIndexSpinBox->value());

    return config;
}

KisConfigWidget* KisFilterPalettize::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool useForMasks) const
{
    Q_UNUSED(dev)
    Q_UNUSED(useForMasks)

    return new KisPalettizeWidget(parent);
}

KisFilterConfigurationSP KisFilterPalettize::factoryConfiguration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("palettize", 1);

    config->setProperty("palette", "Default");
    config->setProperty("colorspace", Colorspace::Lab);
    config->setProperty("ditherEnabled", false);
    config->setProperty("thresholdMode", ThresholdMode::Pattern);
    config->setProperty("pattern", "Grid01.pat");
    config->setProperty("patternValueMode", PatternValueMode::Auto);
    config->setProperty("noiseSeed", rand());
    config->setProperty("colorMode", ColorMode::PerChannelOffset);
    config->setProperty("offsetScale", 0.125);
    config->setProperty("spread", 1.0);
    config->setProperty("colorCount", 2);
    config->setProperty("alphaEnabled", true);
    config->setProperty("alphaMode", AlphaMode::Clip);
    config->setProperty("alphaClip", 0.5);
    config->setProperty("alphaIndex", 0);

    return config;
}

void KisFilterPalettize::processImpl(KisPaintDeviceSP device, const QRect& applyRect, const KisFilterConfigurationSP config, KoUpdater* progressUpdater) const
{
    const KoColorSet* palette = KoResourceServerProvider::instance()->paletteServer()->resourceByName(config->getString("palette"));
    const int searchColorspace = config->getInt("colorspace");
    const bool ditherEnabled = config->getBool("ditherEnabled");
    const int thresholdMode = config->getInt("thresholdMode");
    const KoPattern* pattern = KoResourceServerProvider::instance()->patternServer()->resourceByName(config->getString("pattern"));
    const int patternValueMode = config->getInt("patternValueMode");
    const quint64 noiseSeed = quint64(config->getInt("noiseSeed"));
    const int colorMode = config->getInt("colorMode");
    const double offsetScale = config->getDouble("offsetScale");
    const double spread = config->getDouble("spread");
    const int colorCount = ditherEnabled && colorMode == ColorMode::NearestColors ? config->getInt("colorCount") : 1;
    const bool alphaEnabled = config->getBool("alphaEnabled");
    const int alphaMode = config->getInt("alphaMode");
    const double alphaClip = config->getDouble("alphaClip");
    const int alphaIndex = config->getInt("alphaIndex");

    const KoColorSpace* colorspace = device->colorSpace();
    const KoColorSpace* workColorspace = (searchColorspace == Colorspace::Lab
                                              ? KoColorSpaceRegistry::instance()->lab16()
                                              : KoColorSpaceRegistry::instance()->rgb16());

    KisRandomGenerator random(noiseSeed);

    using TreeColor = boost::geometry::model::point<quint16, 3, boost::geometry::cs::cartesian>;
    using TreeValue = std::tuple<TreeColor, KoColor, quint16>;
    using Rtree = boost::geometry::index::rtree<TreeValue, boost::geometry::index::quadratic<16>>;
    Rtree rtree;

    if (palette) {
        // Add palette colors to search tree
        quint16 index = 0;
        for (int row = 0; row < palette->rowCount(); ++row) {
            for (int column = 0; column < palette->columnCount(); ++column) {
                KisSwatch swatch = palette->getColorGlobal(column, row);
                if (swatch.isValid()) {
                    KoColor color = swatch.color().convertedTo(colorspace);
                    KoColor workColor = swatch.color().convertedTo(workColorspace);
                    TreeColor searchColor;
                    memcpy(&searchColor, workColor.data(), sizeof(TreeColor));
                    // Don't add duplicates so won't dither between identical colors
                    std::vector<TreeValue> result;
                    rtree.query(boost::geometry::index::contains(searchColor), std::back_inserter(result));
                    if (result.empty()) rtree.insert(std::make_tuple(searchColor, color, index++));
                }
            }
        }

        bool patternUseAlpha;
        if (pattern && ditherEnabled && thresholdMode == ThresholdMode::Pattern) {
            // Automatically pick between lightness-based and alpha-based patterns by whichever has maximum range
            if (patternValueMode == PatternValueMode::Auto) {
                qreal lightnessMin = 1.0, lightnessMax = 0.0;
                qreal alphaMin = 1.0, alphaMax = 0.0;
                const QImage &image = pattern->pattern();
                for (int y = 0; y < image.height(); ++y) {
                    for (int x = 0; x < image.width(); ++x) {
                        const QColor pixel = image.pixelColor(x, y);
                        lightnessMin = std::min(lightnessMin, pixel.lightnessF());
                        lightnessMax = std::max(lightnessMax, pixel.lightnessF());
                        alphaMin = std::min(alphaMin, pixel.alphaF());
                        alphaMax = std::max(alphaMax, pixel.alphaF());
                    }
                }
                patternUseAlpha = (alphaMax - alphaMin > lightnessMax - lightnessMin);
            }
            else {
                patternUseAlpha = (patternValueMode == PatternValueMode::Alpha);
            }
        }

        KisSequentialIteratorProgress pixel(device, applyRect, progressUpdater);
        while (pixel.nextPixel()) {
            // Find dither threshold
            double ditherPos = 0.5;
            if (ditherEnabled) {
                if (thresholdMode == ThresholdMode::Pattern) {
                    const QImage &image = pattern->pattern();
                    const QColor ditherPixel = image.pixelColor(pixel.x() % image.width(), pixel.y() % image.height());
                    ditherPos = (patternUseAlpha ? ditherPixel.alphaF() : ditherPixel.lightnessF());
                }
                else if (thresholdMode == ThresholdMode::Noise) {
                    ditherPos = random.doubleRandomAt(pixel.x(), pixel.y());
                }
            }
            const double ditherThreshold = (0.5 - (spread / 2.0) + ditherPos * spread);

            KoColor workColor(pixel.oldRawData(), colorspace);
            workColor.convertTo(workColorspace);
            if (ditherEnabled && colorMode == ColorMode::PerChannelOffset) {
                // Traditional per-channel ordered dithering
                QVector<float> normalized(workColorspace->channelCount());
                workColorspace->normalisedChannelsValue(workColor.data(), normalized);
                for (quint32 channel = 0; channel < workColorspace->channelCount(); ++channel) {
                    normalized[channel] += (ditherPos - 0.5) * offsetScale;
                }
                workColorspace->fromNormalisedChannelsValue(workColor.data(), normalized);
            }

            // Get candidate colors and their distances
            TreeColor searchColor;
            memcpy(reinterpret_cast<quint8 *>(&searchColor), workColor.data(), sizeof(TreeColor));
            std::vector<std::tuple<TreeColor, KoColor, quint16, double>> candidateColors;
            candidateColors.reserve(size_t(colorCount));
            double invDistanceSum = 0.0;
            for (auto it = rtree.qbegin(boost::geometry::index::nearest(searchColor, colorCount)); it != rtree.qend() && candidateColors.size() < colorCount; ++it) {
                const double distance = boost::geometry::distance(searchColor, std::get<0>(*it));
                const double invDistance = 1.0 / (distance * distance);
                candidateColors.push_back(std::tuple_cat(*it, std::make_tuple(invDistance)));
                invDistanceSum += invDistance;
            }

            quint16 colorIndex = 0;
            if (!ditherEnabled || colorMode == ColorMode::PerChannelOffset) {
                memcpy(pixel.rawData(), std::get<1>(candidateColors[0]).data(), colorspace->pixelSize());
                colorIndex = std::get<2>(candidateColors[0]);
            }
            else if (ditherEnabled && colorMode == ColorMode::NearestColors) {
                // Find candidate at threshold
                double probabilitySum = 0.0;
                for (auto it = candidateColors.rbegin(); it != candidateColors.rend(); ++it) {
                    const auto& candidate = *it;
                    const double probability = std::get<3>(candidate) / invDistanceSum;
                    const double prevProbabilitySum = probabilitySum;
                    probabilitySum += probability;
                    if (ditherThreshold >= prevProbabilitySum && ditherThreshold <= probabilitySum) {
                        memcpy(pixel.rawData(), std::get<1>(candidate).data(), colorspace->pixelSize());
                        colorIndex = std::get<2>(candidate);
                        break;
                    }
                }
            }

            // Set alpha
            if (alphaEnabled && !(!ditherEnabled && alphaMode == AlphaMode::UseDither)) {
                double colorAlpha = colorspace->opacityF(pixel.oldRawData());
                double alphaThreshold = 1.0;
                if (alphaMode == AlphaMode::Clip) {
                    alphaThreshold = alphaClip;
                }
                else if (alphaMode == AlphaMode::Index) {
                    alphaThreshold = (colorIndex == alphaIndex ? 2.0 : 0.0);
                }
                else if (alphaMode == AlphaMode::UseDither) {
                    if (colorMode == ColorMode::PerChannelOffset) {
                        alphaThreshold = 0.5;
                        colorAlpha = colorspace->opacityF(workColor.convertedTo(colorspace).data());
                    }
                    else if (colorMode == ColorMode::NearestColors) {
                        alphaThreshold = ditherThreshold;
                    }
                }
                colorspace->setOpacity(pixel.rawData(), (colorAlpha < alphaThreshold ? 0.0 : 1.0), 1);
            }
        }
    }
}
