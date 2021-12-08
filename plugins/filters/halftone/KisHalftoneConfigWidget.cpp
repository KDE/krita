/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <filter/kis_filter_configuration.h>
#include <kis_filter_registry.h>
#include <KisGlobalResourcesInterface.h>
#include <KoChannelInfo.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceRegistry.h>
#include <kis_signals_blocker.h>
#include <KisViewManager.h>
#include <kis_canvas_resource_provider.h>


#include "KisHalftoneConfigWidget.h"
#include "KisHalftoneConfigPageWidget.h"
#include "KisHalftoneFilterConfiguration.h"

KisHalftoneConfigWidget::KisHalftoneConfigWidget(QWidget *parent,  const KisPaintDeviceSP dev)
    : KisConfigWidget(parent)
    , m_paintDevice(dev)
    , m_intensityWidget(nullptr)
{
    Q_ASSERT(m_paintDevice);

    m_channelsInfo = m_paintDevice->colorSpace()->channels();
    m_colorModelId = m_paintDevice->colorSpace()->colorModelId().id();

    m_ui.setupUi(this);

    const QString intensityString = i18nc("Brightness in Halftone Widget", "Intensity");
    const QString independentChannelsString = i18n("Independent Channels");
    const QString alphaString = KoColorSpaceRegistry::instance()->graya8()->channels().at(1)->name();
    const QString grayString = KoColorSpaceRegistry::instance()->graya8()->channels().at(0)->name();

    if (m_colorModelId == AlphaColorModelID.id()) {
        m_ui.comboBoxMode->addItem(alphaString);
    } else if (m_colorModelId == GrayColorModelID.id()) {
        m_ui.comboBoxMode->addItem(grayString);
    } else if (m_colorModelId == GrayAColorModelID.id()) {
        m_ui.comboBoxMode->addItem(grayString);
        m_ui.comboBoxMode->addItem(alphaString);
    } else {
        m_ui.comboBoxMode->addItem(intensityString);
        m_ui.comboBoxMode->addItem(independentChannelsString);
        m_ui.comboBoxMode->addItem(alphaString);
        m_intensityWidget = new KisHalftoneConfigPageWidget(this, m_paintDevice);
        m_intensityWidget->hide();
        connect(m_intensityWidget, SIGNAL(signal_configurationUpdated()), this, SIGNAL(sigConfigurationItemChanged()));
    }

    int alphaPos = static_cast<int>(m_paintDevice->colorSpace()->alphaPos());
    for (int i = 0; i < m_channelsInfo.size(); ++i)
    {
        KisHalftoneConfigPageWidget * w = new KisHalftoneConfigPageWidget(this, m_paintDevice);
        if (m_colorModelId != GrayColorModelID.id() && m_colorModelId != GrayAColorModelID.id()) {
            w->hideColors();
            if (i != alphaPos) {
                w->setContentsMargins(10, 10, 10, 10);
            }
        } else {
            if (i == alphaPos) {
                w->hideColors();
            }
        }
        w->hide();
        m_channelWidgets.append(w);
        connect(w, SIGNAL(signal_configurationUpdated()), this, SIGNAL(sigConfigurationItemChanged()));
    }

    connect(m_ui.comboBoxMode, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_comboBoxMode_currentIndexChanged(int)));
}

KisHalftoneConfigWidget::~KisHalftoneConfigWidget()
{}

void KisHalftoneConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    const KisHalftoneFilterConfigurationSP filterConfig =
        const_cast<KisHalftoneFilterConfiguration*>(
            dynamic_cast<const KisHalftoneFilterConfiguration*>(config.data())
        );

    {
        KisSignalsBlocker signalsBlocker(this);

        QString mode = filterConfig->mode();

        if (m_colorModelId == AlphaColorModelID.id()) {
            m_channelWidgets[0]->setConfiguration(filterConfig, "alpha_");
            m_ui.comboBoxMode->setCurrentIndex(0);
        } else if (m_colorModelId == GrayColorModelID.id()) {
            m_channelWidgets[0]->setConfiguration(filterConfig, "intensity_");
            m_ui.comboBoxMode->setCurrentIndex(0);
        } else if (m_colorModelId == GrayAColorModelID.id()) {
            m_channelWidgets[0]->setConfiguration(filterConfig, "intensity_");
            m_channelWidgets[1]->setConfiguration(filterConfig, "alpha_");

            if (mode == KisHalftoneFilterConfiguration::HalftoneMode_Alpha) {
                m_ui.comboBoxMode->setCurrentIndex(1);
            } else {
                m_ui.comboBoxMode->setCurrentIndex(0);
            }
        } else {
            int alphaPos = m_paintDevice->colorSpace()->alphaPos();
            m_intensityWidget->setConfiguration(filterConfig, "intensity_");
            for (int i = 0; i < m_channelWidgets.size(); ++i) {
                if (i != alphaPos) {
                    m_channelWidgets[i]->setConfiguration(filterConfig, m_colorModelId + "_channel" + QString::number(i) + "_");
                }
            }
            m_channelWidgets[alphaPos]->setConfiguration(filterConfig, "alpha_");

            if (mode == KisHalftoneFilterConfiguration::HalftoneMode_Alpha) {
                m_ui.comboBoxMode->setCurrentIndex(2);
            } else if (mode == KisHalftoneFilterConfiguration::HalftoneMode_IndependentChannels) {
                m_ui.comboBoxMode->setCurrentIndex(1);
            } else {
                m_ui.comboBoxMode->setCurrentIndex(0);
            }
        }

        slot_comboBoxMode_currentIndexChanged(m_ui.comboBoxMode->currentIndex());
    }
    emit sigConfigurationItemChanged();
}

KisPropertiesConfigurationSP KisHalftoneConfigWidget::configuration() const
{
    KisFilterSP filter = KisFilterRegistry::instance()->get("halftone");
    KisHalftoneFilterConfigurationSP filterConfig =
        dynamic_cast<KisHalftoneFilterConfiguration*>(
            filter->factoryConfiguration(KisGlobalResourcesInterface::instance()).data()
        );

    filterConfig->setColorModelId(m_colorModelId);
    
    if (m_colorModelId == AlphaColorModelID.id()) {
        filterConfig->setMode(KisHalftoneFilterConfiguration::HalftoneMode_Alpha);
        m_channelWidgets[0]->configuration(filterConfig, "alpha_");
    } else if (m_colorModelId == GrayColorModelID.id()) {
        filterConfig->setMode(KisHalftoneFilterConfiguration::HalftoneMode_Intensity);
        m_channelWidgets[0]->configuration(filterConfig, "intensity_");
    } else if (m_colorModelId == GrayAColorModelID.id()) {
        if (m_ui.comboBoxMode->currentIndex() == 0) {
            filterConfig->setMode(KisHalftoneFilterConfiguration::HalftoneMode_Intensity);
        } else {
            filterConfig->setMode(KisHalftoneFilterConfiguration::HalftoneMode_Alpha);
        }
        m_channelWidgets[0]->configuration(filterConfig, "intensity_");
        m_channelWidgets[1]->configuration(filterConfig, "alpha_");
    } else {
        int alphaPos = m_paintDevice->colorSpace()->alphaPos();
        if (m_ui.comboBoxMode->currentIndex() == 0) {
            filterConfig->setMode(KisHalftoneFilterConfiguration::HalftoneMode_Intensity);
        } else if (m_ui.comboBoxMode->currentIndex() == 1) {
            filterConfig->setMode(KisHalftoneFilterConfiguration::HalftoneMode_IndependentChannels);
        } else {
            filterConfig->setMode(KisHalftoneFilterConfiguration::HalftoneMode_Alpha);
        }
        m_intensityWidget->configuration(filterConfig, "intensity_");
        for (int i = 0; i < m_channelWidgets.size(); ++i) {
            if (i != alphaPos) {
                m_channelWidgets[i]->configuration(filterConfig, m_colorModelId + "_channel" + QString::number(i) + "_");
            }
        }
        m_channelWidgets[alphaPos]->configuration(filterConfig, "alpha_");
    }

    return filterConfig;
}

void KisHalftoneConfigWidget::setView(KisViewManager *view)
{
    if (m_intensityWidget) {
        m_intensityWidget->setView(view);
    }
    for (KisHalftoneConfigPageWidget *configPageWidget : m_channelWidgets) {
        if (configPageWidget) {
            configPageWidget->setView(view);
        }
    }

    KoCanvasResourcesInterfaceSP canvasResourcesInterface = view ? view->canvasResourceProvider()->resourceManager()->canvasResourcesInterface() : nullptr;
    setCanvasResourcesInterface(canvasResourcesInterface);
}

void KisHalftoneConfigWidget::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    if (m_intensityWidget) {
        m_intensityWidget->setCanvasResourcesInterface(canvasResourcesInterface);
    }
    for (KisHalftoneConfigPageWidget *configPageWidget : m_channelWidgets) {
        if (configPageWidget) {
            configPageWidget->setCanvasResourcesInterface(canvasResourcesInterface);
        }
    }
}

void KisHalftoneConfigWidget::slot_comboBoxMode_currentIndexChanged(int index)
{
    while (m_ui.tabWidgetChannels->count()) {
        m_ui.tabWidgetChannels->removeTab(0);
    }
    
    if (m_colorModelId == AlphaColorModelID.id() || m_colorModelId == GrayColorModelID.id()) {
        m_ui.tabWidgetChannels->addTab(m_channelWidgets[0], m_channelsInfo.at(0)->name());
    } else if (m_colorModelId == GrayAColorModelID.id()) {
        if (index == 0) {
            m_ui.tabWidgetChannels->addTab(m_channelWidgets[0], m_channelsInfo.at(0)->name());
        } else {
            m_ui.tabWidgetChannels->addTab(m_channelWidgets[1], m_channelsInfo.at(1)->name());
        }
    } else {
        int alphaPos = m_paintDevice->colorSpace()->alphaPos();
        if (index == 0) {
            m_ui.tabWidgetChannels->addTab(m_intensityWidget, i18nc("Brightness in Halftone Widget", "Intensity"));
        } else if (index == 1) {
            for (int i = 0; i < m_channelsInfo.size(); ++i) {
                if (i != alphaPos) {
                    m_ui.tabWidgetChannels->addTab(m_channelWidgets[i], m_channelsInfo.at(i)->name());
                }
            }
        } else {
            m_ui.tabWidgetChannels->addTab(m_channelWidgets[alphaPos], m_channelsInfo.at(alphaPos)->name());
        }
    }

    emit sigConfigurationItemChanged();
}
