/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_wdg_blur.h"
#include <QLayout>
#include <QToolButton>
#include <QIcon>

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>
#include <KisGlobalResourcesInterface.h>

#include "ui_wdgblur.h"

KisWdgBlur::KisWdgBlur(QWidget * parent) : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgBlur();
    m_widget->setupUi(this);
    linkSpacingToggled(true);

    widget()->angleSelector->setDecimals(0);

    connect(widget()->aspectButton, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(linkSpacingToggled(bool)));
    connect(widget()->intHalfWidth, SIGNAL(valueChanged(int)), this, SLOT(sldHalfWidthChanged(int)));
    connect(widget()->intHalfHeight, SIGNAL(valueChanged(int)), this, SLOT(sldHalfHeightChanged(int)));

    connect(widget()->intStrength, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->angleSelector, SIGNAL(angleChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
    connect(widget()->cbShape, SIGNAL(activated(int)), SIGNAL(sigConfigurationItemChanged()));
}

KisWdgBlur::~KisWdgBlur()
{
    delete m_widget;
}

KisPropertiesConfigurationSP KisWdgBlur::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("blur", 1, KisGlobalResourcesInterface::instance());
    config->setProperty("lockAspect", widget()->aspectButton->keepAspectRatio());
    config->setProperty("halfWidth", widget()->intHalfWidth->value());
    config->setProperty("halfHeight", widget()->intHalfHeight->value());
    config->setProperty("rotate", static_cast<int>(widget()->angleSelector->angle()));
    config->setProperty("strength", widget()->intStrength->value());
    config->setProperty("shape", widget()->cbShape->currentIndex());
    return config;
}

void KisWdgBlur::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;
    if (config->getProperty("lockAspect", value)) {
        m_widget->aspectButton->setKeepAspectRatio(value.toBool());
    }
    if (config->getProperty("shape", value)) {
        widget()->cbShape->setCurrentIndex(value.toUInt());
    }
    if (config->getProperty("halfWidth", value)) {
        widget()->intHalfWidth->setValue(value.toUInt());
    }
    if (config->getProperty("halfHeight", value)) {
        widget()->intHalfHeight->setValue(value.toUInt());
    }
    if (config->getProperty("rotate", value)) {
        widget()->angleSelector->setAngle(static_cast<qreal>(value.toUInt()));
    }
    if (config->getProperty("strength", value)) {
        widget()->intStrength->setValue(value.toUInt());
    }
}

void KisWdgBlur::linkSpacingToggled(bool b)
{
    m_halfSizeLink = b;
    widget()->intHalfHeight->setValue(widget()->intHalfWidth->value());
}

void KisWdgBlur::sldHalfWidthChanged(int v)
{
    if (m_halfSizeLink) {
        widget()->intHalfHeight->blockSignals(true);
        widget()->intHalfHeight->setValue(v);
        widget()->intHalfHeight->blockSignals(false);
    }
    /*    if( widget()->intHalfHeight->value() == v && widget()->cbShape->currentItem() != 1)
            widget()->intAngle->setEnabled(false);
        else
            widget()->intAngle->setEnabled(true);*/
    emit sigConfigurationItemChanged();
}

void KisWdgBlur::sldHalfHeightChanged(int v)
{
    if (m_halfSizeLink) {
        widget()->intHalfWidth->blockSignals(true);
        widget()->intHalfWidth->setValue(v);
        widget()->intHalfWidth->blockSignals(false);
    }
    /*    if( widget()->intHalfWidth->value() == v && widget()->cbShape->currentItem() != 1)
            widget()->intAngle->setEnabled(false);
        else
            widget()->intAngle->setEnabled(true);*/
    emit sigConfigurationItemChanged();
}

