/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2010 Edward Apap <schumifer@hotmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_wdg_motion_blur.h"
#include <QLayout>
#include <QToolButton>
#include <QIcon>

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <KisGlobalResourcesInterface.h>

#include "ui_wdg_motion_blur.h"

KisWdgMotionBlur::KisWdgMotionBlur(QWidget * parent) : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgMotionBlur();
    m_widget->setupUi(this);

    m_widget->blurAngleSelector->setDecimals(0);
    m_widget->blurAngleSelector->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);

    connect(m_widget->blurAngleSelector, SIGNAL(angleChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_widget->blurLength, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
}

KisWdgMotionBlur::~KisWdgMotionBlur()
{
    delete m_widget;
}

KisPropertiesConfigurationSP KisWdgMotionBlur::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("motion blur", 1, KisGlobalResourcesInterface::instance());
    config->setProperty("blurAngle", static_cast<int>(m_widget->blurAngleSelector->angle()));
    config->setProperty("blurLength", m_widget->blurLength->value());
    return config;
}

void KisWdgMotionBlur::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    if (config->getProperty("blurAngle", value)) {
        m_widget->blurAngleSelector->setAngle(static_cast<qreal>(value.toInt()));
    }
    if (config->getProperty("blurLength", value)) {
        m_widget->blurLength->setValue(value.toInt());
    }
}
