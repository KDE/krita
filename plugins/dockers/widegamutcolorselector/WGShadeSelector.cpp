/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGShadeSelector.h"

#include "WGConfig.h"
#include "WGShadeSlider.h"

#include <QVBoxLayout>
#include <QMouseEvent>

WGShadeSelector::WGShadeSelector(KisVisualColorModelSP colorModel, QWidget *parent)
    : WGSelectorWidgetBase(parent)
    , m_model(colorModel)
{
    QVBoxLayout* l = new QVBoxLayout(this);
    l->setSpacing(1);
    l->setMargin(0);

    connectToModel();
    updateSettings();
}

void WGShadeSelector::setModel(KisVisualColorModelSP colorModel)
{
    if (m_model) {
        m_model->disconnect(this);
        disconnect(m_model.data());
    }
    m_model = colorModel;
    for (WGShadeSlider *slider: m_sliders) {
        slider->setModel(m_model);
    }
    connectToModel();
    if (m_model->colorModel() != KisVisualColorModel::None) {
        slotChannelValuesChanged(m_model->channelValues());
    }
}

void WGShadeSelector::updateSettings()
{
    WGConfig cfg;
    m_lineHeight = cfg.shadeSelectorLineHeight();
    QVector<WGConfig::ShadeLine> config = cfg.shadeSelectorLines();

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
        m_sliders[i]->setGradient(config[i].gradient, config[i].offset);
        m_sliders[i]->setDisplayMode(config[i].patchCount < 0, config[i].patchCount);
        m_sliders[i]->setFixedHeight(m_lineHeight);
    }
    m_resetOnExternalUpdate = cfg.shadeSelectorUpdateOnExternalChanges();
    m_resetOnInteractions = cfg.shadeSelectorUpdateOnInteractionEnd();
    m_resetOnRightClick = cfg.shadeSelectorUpdateOnRightClick();

    if (m_model->colorModel() != KisVisualColorModel::None) {
        slotReset();
        slotChannelValuesChanged(m_model->channelValues());
    }
}

void WGShadeSelector::mousePressEvent(QMouseEvent *event)
{
    if (m_resetOnRightClick && event->button() == Qt::RightButton) {
        for (int i = 0; i < m_sliders.size(); i++) {
            m_sliders[i]->slotSetChannelValues(m_model->channelValues());
        }
    }
}

void WGShadeSelector::connectToModel()
{
    connect(m_model.data(), SIGNAL(sigColorModelChanged()), SLOT(slotReset()));
    connect(m_model.data(), SIGNAL(sigColorSpaceChanged()), SLOT(slotReset()));
    connect(m_model.data(), SIGNAL(sigChannelValuesChanged(QVector4D)),
            SLOT(slotChannelValuesChanged(QVector4D)));
    connect(this, SIGNAL(sigChannelValuesChanged(QVector4D)),
            m_model.data(), SLOT(slotSetChannelValues(QVector4D)));
}

void WGShadeSelector::slotChannelValuesChanged(const QVector4D &values)
{
    if (m_allowUpdates && (m_resetOnExternalUpdate || !m_initialized)) {
        for (int i = 0; i < m_sliders.size(); i++) {
            m_sliders[i]->slotSetChannelValues(values);
        }
        m_initialized = true;
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
                line->resetHandle();
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
        // reset slider base values if configured for automatic reset
        if (m_resetOnInteractions) {
            for (int i = 0; i < m_sliders.size(); i++) {
                m_sliders[i]->slotSetChannelValues(m_model->channelValues());
            }
        }
        emit sigColorInteraction(active);
    }
}

void WGShadeSelector::slotReset()
{
    m_initialized = false;
}
