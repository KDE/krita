/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2009 Edward Apap <schumifer@hotmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_wdg_gaussian_blur.h"
#include <QLayout>
#include <QToolButton>
#include <QIcon>

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <KisGlobalResourcesInterface.h>

#include "ui_wdg_gaussian_blur.h"

KisWdgGaussianBlur::KisWdgGaussianBlur(bool usedForMasks, QWidget * parent) : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgGaussianBlur();
    m_widget->setupUi(this);

    m_widget->aspectButton->setKeepAspectRatio(false);

    const qreal maxRadius = usedForMasks ? 100.0 : 1000.0;

    m_widget->horizontalRadius->setRange(0.0, maxRadius, 2);
    m_widget->horizontalRadius->setSingleStep(0.2);
    m_widget->horizontalRadius->setValue(0.5);
    m_widget->horizontalRadius->setExponentRatio(3.0);
    m_widget->horizontalRadius->setSuffix(i18n(" px"));

    connect(m_widget->horizontalRadius, SIGNAL(valueChanged(qreal)), this, SLOT(horizontalRadiusChanged(qreal)));

    m_widget->verticalRadius->setRange(0.0, maxRadius, 2);
    m_widget->verticalRadius->setSingleStep(0.2);
    m_widget->verticalRadius->setValue(0.5);
    m_widget->verticalRadius->setExponentRatio(3.0);
    m_widget->verticalRadius->setSuffix(i18n(" px"));
    connect(m_widget->verticalRadius, SIGNAL(valueChanged(qreal)), this, SLOT(verticalRadiusChanged(qreal)));

    connect(m_widget->aspectButton, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(aspectLockChanged(bool)));
    connect(m_widget->horizontalRadius, SIGNAL(valueChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_widget->verticalRadius, SIGNAL(valueChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
}

KisWdgGaussianBlur::~KisWdgGaussianBlur()
{
    delete m_widget;
}

KisPropertiesConfigurationSP KisWdgGaussianBlur::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("gaussian blur", 1, KisGlobalResourcesInterface::instance());
    config->setProperty("horizRadius", m_widget->horizontalRadius->value());
    config->setProperty("vertRadius", m_widget->verticalRadius->value());
    config->setProperty("lockAspect", m_widget->aspectButton->keepAspectRatio());
    return config;
}

void KisWdgGaussianBlur::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    if (config->getProperty("horizRadius", value)) {
        m_widget->horizontalRadius->setValue(value.toFloat());
    }
    if (config->getProperty("vertRadius", value)) {
        m_widget->verticalRadius->setValue(value.toFloat());
    }
    if (config->getProperty("lockAspect", value)) {
        m_widget->aspectButton->setKeepAspectRatio(value.toBool());
    }
}

void KisWdgGaussianBlur::horizontalRadiusChanged(qreal v)
{
    m_widget->horizontalRadius->blockSignals(true);
    m_widget->horizontalRadius->setValue(v);
    m_widget->horizontalRadius->blockSignals(false);

    if (m_widget->aspectButton->keepAspectRatio()) {
        m_widget->verticalRadius->blockSignals(true);
        m_widget->verticalRadius->setValue(v);
        m_widget->verticalRadius->blockSignals(false);
    }
}

void KisWdgGaussianBlur::verticalRadiusChanged(qreal v)
{
    m_widget->verticalRadius->blockSignals(true);
    m_widget->verticalRadius->setValue(v);
    m_widget->verticalRadius->blockSignals(false);

    if (m_widget->aspectButton->keepAspectRatio()) {
        m_widget->horizontalRadius->blockSignals(true);
        m_widget->horizontalRadius->setValue(v);
        m_widget->horizontalRadius->blockSignals(false);
    }
}

void KisWdgGaussianBlur::aspectLockChanged(bool v)
{
    if (v) {
        m_widget->verticalRadius->setValue( m_widget->horizontalRadius->value() );
    }
}

