/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2019 Eoin O'Neill <eoinoneill1991@gmail.com>
 * Copyright (c) 2019 Emmet O'Neill <emmetoneill.pdx@gmail.com>
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

#include "kis_wdg_simplex_noise.h"
#include "ui_wdgsimplexnoiseoptions.h"

#include <QLayout>
#include <filter/kis_filter_configuration.h>

KisWdgSimplexNoise::KisWdgSimplexNoise(KisFilter* /*nfilter*/, QWidget* parent)
        : KisConfigWidget(parent),
          updateCompressor(250, KisSignalCompressor::Mode::POSTPONE)
{
    m_widget = new Ui_WdgSimplexNoiseOptions();
    m_widget->setupUi(this);
    connect(m_widget->slider_frequency, SIGNAL(valueChanged(qreal)), &updateCompressor, SLOT(start()));
    connect(m_widget->cb_looping, SIGNAL(stateChanged(int)), &updateCompressor, SLOT(start()));
    connect(m_widget->seed_text, SIGNAL(textChanged(QString)), &updateCompressor, SLOT(start()));
    connect(m_widget->ratiox_slider, SIGNAL(valueChanged(qreal)), &updateCompressor, SLOT(start()));
    connect(m_widget->ratioy_slider, SIGNAL(valueChanged(qreal)), &updateCompressor, SLOT(start()));
    connect(&updateCompressor, SIGNAL(timeout()), this, SIGNAL(sigConfigurationItemChanged()));
    m_widget->slider_frequency->setRange(1.0f, 500.0f, 2);
    m_widget->slider_frequency->setValue(25.0f);
    m_widget->slider_frequency->setExponentRatio(3.0);
    m_widget->ratiox_slider->setRange(0.0f, 2.0f, 2);
    m_widget->ratiox_slider->setValue(1.0f);
    m_widget->ratioy_slider->setRange(0.0f, 2.0f, 2);
    m_widget->ratioy_slider->setValue(1.0f);
}

KisWdgSimplexNoise::~KisWdgSimplexNoise()
{
    delete m_widget;
}

void KisWdgSimplexNoise::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    if( config->getProperty("looping", value)) {
        Qt::CheckState state = value.toBool() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
        widget()->cb_looping->setCheckState(state);
    }
    if( config->getProperty("frequency", value)) {
        widget()->slider_frequency->setValue(value.toDouble());
    }
    if( config->getProperty("custom_seed_string", value)) {
        m_widget->seed_text->setText(value.toString());
    }
    if( config->getProperty("ratio_x", value)) {
        m_widget->ratiox_slider->setValue(value.toDouble());
    }
    if( config->getProperty("ratio_y", value)) {
        m_widget->ratioy_slider->setValue(value.toDouble());
    }
    if( config->getProperty("seed", value)) {
        this->seed = value.toUInt();
    }
}

KisPropertiesConfigurationSP KisWdgSimplexNoise::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("simplex_noise", 1);
    config->setProperty("looping", m_widget->cb_looping->isChecked());
    config->setProperty("frequency", m_widget->slider_frequency->value());
    config->setProperty("ratio_x", m_widget->ratiox_slider->value());
    config->setProperty("ratio_y", m_widget->ratioy_slider->value());
    config->setProperty("custom_seed_string", m_widget->seed_text->text());
    config->setProperty("seed", this->seed);
    return config;
}


