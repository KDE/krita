/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QVBoxLayout>
#include <QHBoxLayout>

#include <filter/kis_filter_configuration.h>
#include <kis_signals_blocker.h>
#include <KisGlobalResourcesInterface.h>

#include <KoGroupButton.h>
#include <KisOptionButtonStrip.h>
#include <KisOptionCollectionWidget.h>
#include <kis_slider_spin_box.h>
#include <KisSpinBoxI18nHelper.h>

#include "KisPropagateColorsFilterConfiguration.h"
#include "KisPropagateColorsConfigWidget.h"

class KisPropagateColorsConfigWidget::Private
{
public:
    KoGroupButton *buttonDistanceMetricChessboard {nullptr};
    KoGroupButton *buttonDistanceMetricCityBlock {nullptr};
    KoGroupButton *buttonDistanceMetricEuclidean {nullptr};

    KoGroupButton *buttonExpansionModeBounded {nullptr};
    KoGroupButton *buttonExpansionModeUnbounded {nullptr};
    KisDoubleSliderSpinBox *sliderExpansionAmount {nullptr};

    KoGroupButton *buttonAlphaChannelModePreserve {nullptr};
    KoGroupButton *buttonAlphaChannelModeExpand {nullptr};

    KisOptionCollectionWidget *optionWidget {nullptr};
};

KisPropagateColorsConfigWidget::KisPropagateColorsConfigWidget(QWidget *parent)
    : KisConfigWidget(parent)
    , m_d(new Private)
{
    // Create widgets
    KisOptionButtonStrip *optionButtonStripDistanceMetric = new KisOptionButtonStrip;
    m_d->buttonDistanceMetricChessboard = optionButtonStripDistanceMetric->addButton(
        i18nc("Expansion pattern button in propagate colors filter", "Rectangle")
    );
    m_d->buttonDistanceMetricCityBlock = optionButtonStripDistanceMetric->addButton(
        i18nc("Expansion pattern button in propagate colors filter", "Diamond")
    );
    m_d->buttonDistanceMetricEuclidean = optionButtonStripDistanceMetric->addButton(
        i18nc("Expansion pattern button in propagate colors filter", "Octagon")
    );
    m_d->buttonDistanceMetricEuclidean->setChecked(true);

    KisOptionButtonStrip *optionButtonStripExpansionMode = new KisOptionButtonStrip;
    m_d->buttonExpansionModeUnbounded = optionButtonStripExpansionMode->addButton(
        i18nc("Expansion distance button in propagate colors filter", "Unrestricted")
    );
    m_d->buttonExpansionModeBounded = optionButtonStripExpansionMode->addButton(
        i18nc("Expansion distance button in propagate colors filter", "Restricted")
    );
    m_d->buttonExpansionModeUnbounded->setChecked(true);

    m_d->sliderExpansionAmount = new KisDoubleSliderSpinBox;
    m_d->sliderExpansionAmount->setRange(0.0, 500.0, 2);
    m_d->sliderExpansionAmount->setSoftMaximum(50.0);
    m_d->sliderExpansionAmount->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
    KisSpinBoxI18nHelper::setText(
        m_d->sliderExpansionAmount,
        i18nc("The 'maximum distance' slider in propagate colors filter; {n} is the number value, px is the pixels suffix",
              "Maximum Distance: {n}px")
    );

    KisOptionButtonStrip *optionButtonStripAlphaChannelMode = new KisOptionButtonStrip;
    m_d->buttonAlphaChannelModeExpand = optionButtonStripAlphaChannelMode->addButton(
        i18nc("Transparency behavior button in propagate colors filter", "Expand")
    );
    m_d->buttonAlphaChannelModePreserve = optionButtonStripAlphaChannelMode->addButton(
        i18nc("Transparency behavior button in propagate colors filter", "Preserve")
    );
    m_d->buttonAlphaChannelModeExpand->setChecked(true);

    // Set tooltips
    m_d->buttonDistanceMetricChessboard->setToolTip(i18n("Expand the colors in a rectangular-like way"));
    m_d->buttonDistanceMetricCityBlock->setToolTip(i18n("Expand the colors in a diamond-like way"));
    m_d->buttonDistanceMetricEuclidean->setToolTip(i18n("Expand the colors in a octagonal-like way"));

    m_d->buttonExpansionModeUnbounded->setToolTip(i18n("Expand the colors all the way until all transparent regions are filled"));
    m_d->buttonExpansionModeBounded->setToolTip(i18n("Expand the colors only until a specific distance"));
    m_d->sliderExpansionAmount->setToolTip(i18n("Specify the maximum distance to expand when the restricted mode is active"));

    m_d->buttonAlphaChannelModeExpand->setToolTip(i18n("Expand the alpha channel as well as the color"));
    m_d->buttonAlphaChannelModePreserve->setToolTip(i18n("Expand only the color, leaving the alpha channel untouched"));

    // Construct the option widget
    m_d->optionWidget = new KisOptionCollectionWidget;
    m_d->optionWidget->setWidgetsMargin(0);

    KisOptionCollectionWidgetWithHeader *sectionDistanceMetric =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'distance metric' section label in propagate colors filter dialog", "Expansion Pattern")
        );
    sectionDistanceMetric->setWidgetsMargin(20);
    sectionDistanceMetric->appendWidget("optionButtonStripDistanceMetric", optionButtonStripDistanceMetric);
    m_d->optionWidget->appendWidget("sectionDistanceMetric", sectionDistanceMetric);

    QWidget *separator1 = new QWidget;
    separator1->setFixedHeight(5);
    m_d->optionWidget->appendWidget("separator1", separator1);

    KisOptionCollectionWidgetWithHeader *sectionExpansionMode =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'expansion mode' section label in propagate colors filter dialog", "Expansion Distance")
        );
    sectionExpansionMode->setWidgetsMargin(20);
    sectionExpansionMode->appendWidget("optionButtonStripExpansionMode", optionButtonStripExpansionMode);
    sectionExpansionMode->appendWidget("sliderExpansionAmount", m_d->sliderExpansionAmount);
    m_d->optionWidget->appendWidget("sectionExpansionMode", sectionExpansionMode);

    QWidget *separator2 = new QWidget;
    separator2->setFixedHeight(5);
    m_d->optionWidget->appendWidget("separator2", separator2);

    KisOptionCollectionWidgetWithHeader *sectionAlphaChannelMode =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'transparency behavior' section label in propagate colors filter dialog", "Transparency Behavior")
        );
    sectionAlphaChannelMode->setWidgetsMargin(20);
    sectionAlphaChannelMode->appendWidget("optionButtonStripAlphaChannelMode", optionButtonStripAlphaChannelMode);
    m_d->optionWidget->appendWidget("sectionAlphaChannelMode", sectionAlphaChannelMode);

    QVBoxLayout *layoutMain = new QVBoxLayout;
    layoutMain->setContentsMargins(0, 0, 0, 0);
    layoutMain->setSpacing(0);
    layoutMain->addWidget(m_d->optionWidget, 0);
    layoutMain->addStretch(1);

    setLayout(layoutMain);

    connect(optionButtonStripDistanceMetric,
            QOverload<int, bool>::of(&KisOptionButtonStrip::buttonToggled),
            this,
            [this](int, bool c){ if (c) { Q_EMIT sigConfigurationItemChanged(); } });
    connect(optionButtonStripExpansionMode,
            QOverload<int, bool>::of(&KisOptionButtonStrip::buttonToggled),
            this,
            [this](int, bool c){ if (c) { Q_EMIT sigConfigurationItemChanged(); } });
    connect(m_d->sliderExpansionAmount, SIGNAL(valueChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
    connect(optionButtonStripAlphaChannelMode,
            QOverload<int, bool>::of(&KisOptionButtonStrip::buttonToggled),
            this,
            [this](int, bool c){ if (c) { Q_EMIT sigConfigurationItemChanged(); } });
}

KisPropagateColorsConfigWidget::~KisPropagateColorsConfigWidget()
{}

void KisPropagateColorsConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    const KisPropagateColorsFilterConfiguration *filterConfig =
        dynamic_cast<const KisPropagateColorsFilterConfiguration *>(config.data());
    KIS_SAFE_ASSERT_RECOVER_RETURN(filterConfig);

    {
        KisSignalsBlocker blocker1(m_d->buttonDistanceMetricChessboard, m_d->buttonDistanceMetricCityBlock,
                                   m_d->buttonDistanceMetricEuclidean, m_d->buttonExpansionModeBounded,
                                   m_d->buttonExpansionModeUnbounded, m_d->sliderExpansionAmount);
        KisSignalsBlocker blocker2(m_d->buttonAlphaChannelModePreserve, m_d->buttonAlphaChannelModeExpand);

        const KisPropagateColorsFilterConfiguration::DistanceMetric distanceMetric =
            filterConfig->distanceMetric();
        const KisPropagateColorsFilterConfiguration::ExpansionMode expansionMode =
            filterConfig->expansionMode();
        const qreal expansionAmount = filterConfig->expansionAmount();
        const KisPropagateColorsFilterConfiguration::AlphaChannelMode alphaChannelMode =
            filterConfig->alphaChannelMode();

        if (distanceMetric == KisPropagateColorsFilterConfiguration::DistanceMetric_Chessboard) {
            m_d->buttonDistanceMetricChessboard->setChecked(true);
        } else if (distanceMetric == KisPropagateColorsFilterConfiguration::DistanceMetric_CityBlock) {
            m_d->buttonDistanceMetricCityBlock->setChecked(true);
        } else {
            m_d->buttonDistanceMetricEuclidean->setChecked(true);
        }

        if (expansionMode == KisPropagateColorsFilterConfiguration::ExpansionMode_Bounded) {
            m_d->buttonExpansionModeBounded->setChecked(true);
        } else {
            m_d->buttonExpansionModeUnbounded->setChecked(true);
        }
        m_d->sliderExpansionAmount->setValue(expansionAmount);

        if (alphaChannelMode == KisPropagateColorsFilterConfiguration::AlphaChannelMode_Preserve) {
            m_d->buttonAlphaChannelModePreserve->setChecked(true);
        } else {
            m_d->buttonAlphaChannelModeExpand->setChecked(true);
        }
    }

    Q_EMIT sigConfigurationItemChanged();
}

KisPropertiesConfigurationSP KisPropagateColorsConfigWidget::configuration() const
{
    KisPropagateColorsFilterConfiguration *config =
        new KisPropagateColorsFilterConfiguration(KisGlobalResourcesInterface::instance());

    config->setDistanceMetric(
        m_d->buttonDistanceMetricChessboard->isChecked()
        ? KisPropagateColorsFilterConfiguration::DistanceMetric_Chessboard
        : m_d->buttonDistanceMetricCityBlock->isChecked()
          ? KisPropagateColorsFilterConfiguration::DistanceMetric_CityBlock
          : KisPropagateColorsFilterConfiguration::DistanceMetric_Euclidean
    );

    config->setExpansionMode(
        m_d->buttonExpansionModeBounded->isChecked()
        ? KisPropagateColorsFilterConfiguration::ExpansionMode_Bounded
        : KisPropagateColorsFilterConfiguration::ExpansionMode_Unbounded
    );

    config->setExpansionAmount(m_d->sliderExpansionAmount->value());

    config->setAlphaChannelMode(
        m_d->buttonAlphaChannelModePreserve->isChecked()
        ? KisPropagateColorsFilterConfiguration::AlphaChannelMode_Preserve
        : KisPropagateColorsFilterConfiguration::AlphaChannelMode_Expand
    );

    return config;
}
