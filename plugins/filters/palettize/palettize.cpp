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

#include <kis_config_widget.h>
#include <KoUpdater.h>
#include <kis_types.h>
#include <kis_filter_category_ids.h>
#include <kis_filter_configuration.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServer.h>
#include <KoColorSet.h>
#include <KoPattern.h>
#include <KisSequentialIteratorProgress.h>
#include <kpluginfactory.h>
#include <kis_filter_registry.h>
#include <QGridLayout>
#include <QGroupBox>
#include <kis_filter_configuration.h>
#include <KisPaletteListWidget.h>
#include <kis_pattern_chooser.h>
#include <KoResourceServerProvider.h>
#include <KoResourceServer.h>
#include <kis_random_generator.h>
#include <kis_iconwidget.h>
#include <kis_double_widget.h>
#include <kis_elided_label.h>
#include <QLineEdit>
#include <QCheckBox>
#include <QIntValidator>
#include <QRadioButton>
#include <QButtonGroup>
#include <KoResourceServerAdapter.h>
#include <KoResourceItemChooser.h>

K_PLUGIN_FACTORY_WITH_JSON(PalettizeFactory, "kritapalettize.json", registerPlugin<Palettize>();)

Palettize::Palettize(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisFilterRegistry::instance()->add(new KisFilterPalettize());
}

KisFilterPalettize::KisFilterPalettize() : KisFilter(id(), FiltersCategoryMapId, i18n("&Palettize..."))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
    setShowConfigurationWidget(true);
}

KisPalettizeWidget::KisPalettizeWidget(QWidget* parent)
    : KisConfigWidget(parent)
{
    QGridLayout* layout = new QGridLayout(this);
    layout->setColumnStretch(0, 0);
    layout->setColumnStretch(1, 1);

    KisElidedLabel* paletteLabel = new KisElidedLabel(i18n("Palette"), Qt::ElideRight, this);
    layout->addWidget(paletteLabel, 0, 0);
    KisIconWidget* paletteIcon = new KisIconWidget(this);
    paletteIcon->setFixedSize(32, 32);
    KoResourceServer<KoColorSet>* paletteServer = KoResourceServerProvider::instance()->paletteServer();
    QSharedPointer<KoAbstractResourceServerAdapter> paletteAdapter(new KoResourceServerAdapter<KoColorSet>(paletteServer));
    m_paletteWidget = new KoResourceItemChooser(paletteAdapter, this, false);
    paletteIcon->setPopupWidget(m_paletteWidget);
    QObject::connect(m_paletteWidget, &KoResourceItemChooser::resourceSelected, paletteIcon, &KisIconWidget::setResource);
    QObject::connect(m_paletteWidget, &KoResourceItemChooser::resourceSelected, this, &KisConfigWidget::sigConfigurationItemChanged);
    paletteLabel->setBuddy(paletteIcon);
    layout->addWidget(paletteIcon, 0, 1, Qt::AlignLeft);

    m_ditherGroupBox = new QGroupBox(i18n("Dither"), this);
    m_ditherGroupBox->setCheckable(true);
    QGridLayout* ditherLayout = new QGridLayout(m_ditherGroupBox);
    QObject::connect(m_ditherGroupBox, &QGroupBox::toggled, this, &KisConfigWidget::sigConfigurationItemChanged);
    ditherLayout->setColumnStretch(0, 0);
    ditherLayout->setColumnStretch(1, 1);
    layout->addWidget(m_ditherGroupBox, 1, 0, 1, 2);

    QRadioButton* ditherPatternRadio = new QRadioButton(i18n("Pattern"), this);
    ditherLayout->addWidget(ditherPatternRadio, 0, 0);
    KisIconWidget* ditherPatternIcon = new KisIconWidget(this);
    ditherPatternIcon->setFixedSize(32, 32);
    KoResourceServer<KoPattern>* patternServer = KoResourceServerProvider::instance()->patternServer();
    QSharedPointer<KoAbstractResourceServerAdapter> patternAdapter(new KoResourceServerAdapter<KoPattern>(patternServer));
    m_ditherPatternWidget = new KoResourceItemChooser(patternAdapter, this, false);
    ditherPatternIcon->setPopupWidget(m_ditherPatternWidget);
    QObject::connect(m_ditherPatternWidget, &KoResourceItemChooser::resourceSelected, ditherPatternIcon, &KisIconWidget::setResource);
    QObject::connect(m_ditherPatternWidget, &KoResourceItemChooser::resourceSelected, this, &KisConfigWidget::sigConfigurationItemChanged);
    ditherLayout->addWidget(ditherPatternIcon, 0, 1, Qt::AlignLeft);
    m_ditherPatternUseAlphaCheckBox = new QCheckBox(i18n("Use alpha"), this);
    QObject::connect(m_ditherPatternUseAlphaCheckBox, &QCheckBox::toggled, this, &KisConfigWidget::sigConfigurationItemChanged);
    ditherLayout->addWidget(m_ditherPatternUseAlphaCheckBox, 0, 2, Qt::AlignLeft);

    QRadioButton* ditherNoiseRadio = new QRadioButton(i18n("Noise"), this);
    ditherLayout->addWidget(ditherNoiseRadio, 1, 0);
    m_ditherNoiseSeedWidget = new QLineEdit(this);
    m_ditherNoiseSeedWidget->setValidator(new QIntValidator(this));
    QObject::connect(m_ditherNoiseSeedWidget, &QLineEdit::textChanged, this, &KisConfigWidget::sigConfigurationItemChanged);
    ditherLayout->addWidget(m_ditherNoiseSeedWidget, 1, 1, 1, 2);

    KisElidedLabel* ditherWeightLabel = new KisElidedLabel(i18n("Weight"), Qt::ElideRight, this);
    ditherLayout->addWidget(ditherWeightLabel, 2, 0);
    m_ditherWeightWidget = new KisDoubleWidget(this);
    m_ditherWeightWidget->setRange(0.0, 1.0);
    m_ditherWeightWidget->setSingleStep(0.0625);
    m_ditherWeightWidget->setPageStep(0.25);
    QObject::connect(m_ditherWeightWidget, &KisDoubleWidget::valueChanged, this, &KisConfigWidget::sigConfigurationItemChanged);
    ditherWeightLabel->setBuddy(m_ditherWeightWidget);
    ditherLayout->addWidget(m_ditherWeightWidget, 2, 1, 1, 2);

    m_ditherModeGroup = new QButtonGroup(this);
    m_ditherModeGroup->addButton(ditherPatternRadio, 0);
    m_ditherModeGroup->addButton(ditherNoiseRadio, 1);
    QObject::connect(m_ditherModeGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &KisConfigWidget::sigConfigurationItemChanged);

    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding), 2, 0, 1, 2);
}

void KisPalettizeWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    KoColorSet* palette = KoResourceServerProvider::instance()->paletteServer()->resourceByName(config->getString("palette"));
    if (palette) m_paletteWidget->setCurrentResource(palette);

    m_ditherGroupBox->setChecked(config->getBool("ditherEnabled"));

    QAbstractButton* ditherModeButton = m_ditherModeGroup->button(config->getInt("ditherMode"));
    if (ditherModeButton) ditherModeButton->setChecked(true);

    KoPattern* ditherPattern = KoResourceServerProvider::instance()->patternServer()->resourceByName(config->getString("ditherPattern"));
    if (ditherPattern) m_ditherPatternWidget->setCurrentResource(ditherPattern);

    m_ditherPatternUseAlphaCheckBox->setChecked(config->getBool("ditherPatternUseAlpha"));

    m_ditherNoiseSeedWidget->setText(QString::number(config->getInt("ditherNoiseSeed")));

    m_ditherWeightWidget->setValue(config->getDouble("ditherWeight"));
}

KisPropertiesConfigurationSP KisPalettizeWidget::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("palettize", 1);
    if (m_paletteWidget->currentResource()) config->setProperty("palette", QVariant(m_paletteWidget->currentResource()->name()));
    config->setProperty("ditherEnabled", m_ditherGroupBox->isChecked());
    config->setProperty("ditherMode", m_ditherModeGroup->checkedId());
    if (m_ditherPatternWidget->currentResource()) config->setProperty("ditherPattern", QVariant(m_ditherPatternWidget->currentResource()->name()));
    config->setProperty("ditherPatternUseAlpha", m_ditherPatternUseAlphaCheckBox->isChecked());
    config->setProperty("ditherNoiseSeed", m_ditherNoiseSeedWidget->text().toInt());
    config->setProperty("ditherWeight", m_ditherWeightWidget->value());

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
    config->setProperty("ditherEnabled", false);
    config->setProperty("ditherMode", DitherMode::Pattern);
    config->setProperty("ditherPattern", "Grid01.pat");
    config->setProperty("ditherPatternUseAlpha", true);
    config->setProperty("ditherNoiseSeed", rand());
    config->setProperty("ditherWeight", 1.0);

    return config;
}

void KisFilterPalettize::processImpl(KisPaintDeviceSP device, const QRect& applyRect, const KisFilterConfigurationSP config, KoUpdater* progressUpdater) const
{
    const KoColorSet* palette = KoResourceServerProvider::instance()->paletteServer()->resourceByName(config->getString("palette"));
    const bool ditherEnabled = config->getBool("ditherEnabled");
    const int ditherMode = config->getInt("ditherMode");
    const KoPattern* ditherPattern = KoResourceServerProvider::instance()->patternServer()->resourceByName(config->getString("ditherPattern"));
    const bool ditherPatternUseAlpha = config->getBool("ditherPatternUseAlpha");
    const quint64 ditherNoiseSeed = quint64(config->getInt("ditherNoiseSeed"));
    const double ditherWeight = config->getDouble("ditherWeight");

    const KoColorSpace* cs = device->colorSpace();
    KisRandomGenerator random(ditherNoiseSeed);

    using TreeColor = boost::geometry::model::point<quint16, 3, boost::geometry::cs::cartesian>;
    using TreeValue = std::pair<TreeColor, std::pair<KoColor, quint16>>;
    using Rtree = boost::geometry::index::rtree<TreeValue, boost::geometry::index::quadratic<16>>;
    Rtree m_rtree;

    if (palette) {
        quint16 index = 0;
        for (int row = 0; row < palette->rowCount(); ++row) {
            for (int column = 0; column < palette->columnCount(); ++column) {
                KisSwatch swatch = palette->getColorGlobal(column, row);
                if (swatch.isValid()) {
                    KoColor color = swatch.color().convertedTo(cs);
                    TreeColor searchColor;
                    KoColor tempColor;
                    cs->toLabA16(color.data(), tempColor.data(), 1);
                    memcpy(reinterpret_cast<quint8 *>(&searchColor), tempColor.data(), sizeof(TreeColor));
                    // Don't add duplicates so won't dither between identical colors
                    std::vector<TreeValue> result;
                    m_rtree.query(boost::geometry::index::contains(searchColor), std::back_inserter(result));
                    if (result.empty()) m_rtree.insert(std::make_pair(searchColor, std::make_pair(color, index++)));
                }
            }
        }
    }

    KisSequentialIteratorProgress it(device, applyRect, progressUpdater);
    while (it.nextPixel()) {
        // Find 2 nearest palette colors to pixel color
        TreeColor imageColor;
        KoColor tempColor;
        cs->toLabA16(it.oldRawData(), tempColor.data(), 1);
        memcpy(reinterpret_cast<quint8 *>(&imageColor), tempColor.data(), sizeof(TreeColor));
        std::vector<TreeValue> nearestColors;
        nearestColors.reserve(2);
        for (Rtree::const_query_iterator it = m_rtree.qbegin(boost::geometry::index::nearest(imageColor, 2)); it != m_rtree.qend(); ++it) {
            nearestColors.push_back(*it);
        }

        if (nearestColors.size() > 0) {
            size_t nearestIndex;
            // Dither not enabled or only one color found so don't dither
            if (!ditherEnabled || nearestColors.size() == 1) nearestIndex = 0;
            // Otherwise threshold between colors based on relative distance
            else {
                std::vector<double> distances(nearestColors.size());
                double distanceSum = 0.0;
                for (size_t i = 0; i < nearestColors.size(); ++i) {
                    distances[i] = boost::geometry::distance(imageColor, nearestColors[i].first);
                    distanceSum += distances[i];
                }
                // Use palette ordering for stable dither color threshold ordering
                size_t ditherIndices[2] = {0, 1};
                if (nearestColors[ditherIndices[0]].second.second > nearestColors[ditherIndices[1]].second.second) std::swap(ditherIndices[0], ditherIndices[1]);
                const double pos = distances[ditherIndices[0]] / distanceSum;
                double threshold = 0.5;
                if (ditherMode == DitherMode::Pattern) {
                    const QImage &image = ditherPattern->pattern();
                    const QColor pixel = image.pixelColor(it.x() % image.width(), it.y() % image.height());
                    threshold = ditherPatternUseAlpha ? pixel.alphaF() :  pixel.lightnessF();
                }
                else if (ditherMode == DitherMode::Noise) {
                    threshold = random.doubleRandomAt(it.x(), it.y());
                }
                nearestIndex = pos < (0.5 - (ditherWeight / 2.0) + threshold * ditherWeight) ? ditherIndices[0] : ditherIndices[1];
            }
            memcpy(it.rawData(), nearestColors[nearestIndex].second.first.data(), cs->pixelSize());
        }
    }
}

#include "palettize.moc"
