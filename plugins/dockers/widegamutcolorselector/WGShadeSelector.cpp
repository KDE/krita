/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGShadeSelector.h"

#include <WGShadeSlider.h>

#include <QVBoxLayout>

WGShadeSelector::WGShadeSelector(KisVisualColorModel *selector, QWidget *parent)
    : QWidget(parent)
    , m_model(selector)
{
    QVBoxLayout* l = new QVBoxLayout(this);
    l->setSpacing(1);
    l->setMargin(0);

    updateSettings();
}

QVector<WGShadeSelector::LineConfig> WGShadeSelector::loadConfiguration()
{
    QVector<WGShadeSelector::LineConfig> config;
    // TODO: just a placeholder for testing
    WGShadeSelector::LineConfig test;
    test.gradient = QVector4D(0.5, 0, 0, 0);
    config.append(test);
    test.gradient = QVector4D(0, -0.2, 0.2, 0);
    config.append(test);
    return config;
}

void WGShadeSelector::updateSettings()
{
    QVector<LineConfig> config = loadConfiguration();

    while (config.size() > m_sliders.size()) {
        WGShadeSlider *line = new WGShadeSlider(this, m_model);
        m_sliders.append(line);
        layout()->addWidget(m_sliders.last());
        connect(line, SIGNAL(sigChannelValuesChanged(QVector4D)), SLOT(slotSliderValuesChanged(QVector4D)));
        connect(line, SIGNAL(sigInteraction(bool)), SLOT(slotSliderInteraction(bool)));
    }
    while (config.size() < m_sliders.size()) {
        layout()->removeWidget(m_sliders.last());
        delete m_sliders.takeLast();
    }

    for (int i=0; i < config.size(); i++) {
        m_sliders[i]->setGradient(config[i].gradient);
        m_sliders[i]->setFixedHeight(m_lineHeight);
    }
}

void WGShadeSelector::slotChannelValuesChanged(const QVector4D &values)
{
    if (m_allowUpdates) {
        for (int i = 0; i < m_sliders.size(); i++) {
            m_sliders[i]->slotSetChannelValues(values);
        }
    }
}

void WGShadeSelector::slotSliderValuesChanged(const QVector4D &values)
{
    m_allowUpdates = false;
    emit sigChannelValuesChanged(values);
    m_allowUpdates = true;
}

void WGShadeSelector::slotSliderInteraction(bool active)
{
    if (active) {
        const WGShadeSlider* activeLine = qobject_cast<WGShadeSlider*>(sender());
        for (WGShadeSlider* line: m_sliders) {
            if (line != activeLine) {
                line->setSliderValue(0.0);
            }
        }
        emit sigColorInteraction(active);
        if (activeLine) {
            // the line may have different channel values at any position than the last used one
            m_allowUpdates = false;
            emit sigChannelValuesChanged(activeLine->channelValues());
            m_allowUpdates = true;
        }
    }
    else {
        // TODO: else, reset slider base values if configured for automatic reset
        emit sigColorInteraction(active);
    }
}
