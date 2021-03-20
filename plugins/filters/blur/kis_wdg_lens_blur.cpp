/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2010 Edward Apap <schumifer@hotmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_wdg_lens_blur.h"
#include <QLayout>
#include <QToolButton>
#include <QIcon>

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <KisGlobalResourcesInterface.h>

#include "kis_lens_blur_filter.h"

#include "ui_wdg_lens_blur.h"

KisWdgLensBlur::KisWdgLensBlur(QWidget * parent) : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgLensBlur();
    m_widget->setupUi(this);

    m_widget->irisRotationSelector->setDecimals(0);
    m_widget->irisRotationSelector->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);

    m_shapeTranslations[i18n("Triangle")] = "Triangle";
    m_shapeTranslations[i18n("Quadrilateral (4)")] = "Quadrilateral (4)";
    m_shapeTranslations[i18n("Pentagon (5)")] = "Pentagon (5)";
    m_shapeTranslations[i18n("Hexagon (6)")] = "Hexagon (6)";
    m_shapeTranslations[i18n("Heptagon (7)")] = "Heptagon (7)";
    m_shapeTranslations[i18n("Octagon (8)")] = "Octagon (8)";

    connect(m_widget->irisShapeCombo, SIGNAL(currentIndexChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_widget->irisRadiusSlider, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_widget->irisRotationSelector, SIGNAL(angleChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
}

KisWdgLensBlur::~KisWdgLensBlur()
{
    delete m_widget;
}

KisPropertiesConfigurationSP KisWdgLensBlur::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("lens blur", 1, KisGlobalResourcesInterface::instance());
    config->setProperty("irisShape", m_shapeTranslations[m_widget->irisShapeCombo->currentText()]);
    config->setProperty("irisRadius", m_widget->irisRadiusSlider->value());
    config->setProperty("irisRotation", static_cast<int>(m_widget->irisRotationSelector->angle()));

    QSize halfSize = KisLensBlurFilter::getKernelHalfSize(config, 0);
    config->setProperty("halfWidth", halfSize.width());
    config->setProperty("halfHeight", halfSize.height());

    return config;
}

void KisWdgLensBlur::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    if (config->getProperty("irisShape", value)) {
        for (int i = 0; i < m_widget->irisShapeCombo->count(); ++i) {
            if (m_shapeTranslations[value.toString()] == m_widget->irisShapeCombo->itemText(i)) {
                m_widget->irisShapeCombo->setCurrentIndex(i);
            }
        }
    }
    if (config->getProperty("irisRadius", value)) {
        m_widget->irisRadiusSlider->setValue(value.toInt());
    }
    if (config->getProperty("irisRotation", value)) {
        m_widget->irisRotationSelector->setAngle(static_cast<qreal>(value.toInt()));
    }
}

