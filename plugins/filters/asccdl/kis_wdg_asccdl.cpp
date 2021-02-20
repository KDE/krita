/*
 * SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_wdg_asccdl.h"
#include <kis_config.h>
#include <kis_color_button.h>
#include <kis_filter_configuration.h>
#include <KisVisualColorSelectorShape.h>
#include <KisVisualRectangleSelectorShape.h>
#include <KisVisualEllipticalSelectorShape.h>
#include <KisGlobalResourcesInterface.h>

KisASCCDLConfigWidget::KisASCCDLConfigWidget(QWidget *parent, const KoColorSpace *cs)
    :KisConfigWidget(parent),
    m_page(new Ui_WdgASCCDL),
    m_cs(cs)

{
    KoColor black(Qt::black, cs);
    m_page->setupUi(this);
    m_page->btnSlope->setColor(black);
    m_page->btnOffset->setColor(black);
    m_page->btnPower->setColor(black);

    m_page->slopeSelector->slotSetColorSpace(m_cs);
    m_page->offsetSelector->slotSetColorSpace(m_cs);
    m_page->powerSelector->slotSetColorSpace(m_cs);

    connect(m_page->btnSlope , SIGNAL(changed(KoColor)), this,  SLOT(slopeColorChanged(KoColor)));
    connect(m_page->btnOffset, SIGNAL(changed(KoColor)), this,  SLOT(offsetColorChanged(KoColor)));
    connect(m_page->btnPower , SIGNAL(changed(KoColor)), this,  SLOT(powerColorChanged(KoColor)));
    connect(m_page->slopeSelector, SIGNAL(sigNewColor(KoColor)), this, SLOT(slopeColorChanged(KoColor)));
    connect(m_page->offsetSelector, SIGNAL(sigNewColor(KoColor)), this, SLOT(offsetColorChanged(KoColor)));
    connect(m_page->powerSelector, SIGNAL(sigNewColor(KoColor)), this, SLOT(powerColorChanged(KoColor)));
}

KisASCCDLConfigWidget::~KisASCCDLConfigWidget()
{
    delete m_page;
}

KisPropertiesConfigurationSP KisASCCDLConfigWidget::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("asc-cdl", 0, KisGlobalResourcesInterface::instance());
    QVariant colorVariant("KoColor");
    colorVariant.setValue(m_page->btnSlope->color());
    config->setProperty("slope", colorVariant);
    colorVariant.setValue(m_page->btnOffset->color());
    config->setProperty("offset", colorVariant);
    colorVariant.setValue(m_page->btnPower->color());
    config->setProperty("power", colorVariant);
    return config;
}

void KisASCCDLConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    KoColor white(m_cs);
    QVector<float> channels(m_cs->channelCount());
    m_cs->normalisedChannelsValue(white.data(), channels);
    channels.fill(1.0);
    m_cs->fromNormalisedChannelsValue(white.data(), channels);
    KoColor black(Qt::black, m_cs);
    KoColor slope = config->getColor("slope", white);
    slope.convertTo(m_cs);
    KoColor offset = config->getColor("offset", black);
    offset.convertTo(m_cs);
    KoColor power = config->getColor("power", white);
    power.convertTo(m_cs);

    m_page->btnSlope->setColor(slope);
    m_page->slopeSelector->slotSetColor(slope);
    m_page->btnOffset->setColor(offset);
    m_page->offsetSelector->slotSetColor(offset);
    m_page->btnPower->setColor (power);
    m_page->powerSelector->slotSetColor(power);
}

void KisASCCDLConfigWidget::slopeColorChanged(const KoColor &c)
{
    if (QObject::sender() == m_page->btnSlope) {
        m_page->slopeSelector->slotSetColor(c);
    } else {
        m_page->btnSlope->setColor(c);
    }
    emit sigConfigurationItemChanged();
}

void KisASCCDLConfigWidget::offsetColorChanged(const KoColor &c)
{
    if (QObject::sender() == m_page->btnOffset) {
        m_page->offsetSelector->slotSetColor(c);
    } else {
        m_page->btnOffset->setColor(c);
    }
    emit sigConfigurationItemChanged();
}

void KisASCCDLConfigWidget::powerColorChanged(const KoColor &c)
{
    if (QObject::sender() == m_page->btnPower) {
        m_page->powerSelector->slotSetColor(c);
    } else {
        m_page->btnPower->setColor(c);
    }
    emit sigConfigurationItemChanged();
}
