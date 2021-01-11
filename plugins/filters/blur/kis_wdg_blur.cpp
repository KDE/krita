/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_wdg_blur.h"
#include <QLayout>
#include <QToolButton>
#include <QIcon>

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>

#include "ui_wdgblur.h"

KisWdgBlur::KisWdgBlur(QWidget * parent) : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgBlur();
    m_widget->setupUi(this);
    linkSpacingToggled(true);

    widget()->angleSelector->setDecimals(0);

    connect(widget()->aspectButton, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(linkSpacingToggled(bool)));
    connect(widget()->intHalfWidth, SIGNAL(valueChanged(int)), this, SLOT(spinBoxHalfWidthChanged(int)));
    connect(widget()->intHalfHeight, SIGNAL(valueChanged(int)), this, SLOT(spinBoxHalfHeightChanged(int)));

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
    KisFilterConfigurationSP config = new KisFilterConfiguration("blur", 1);
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

void KisWdgBlur::spinBoxHalfWidthChanged(int v)
{
    if (m_halfSizeLink) {
        widget()->intHalfHeight->setValue(v);
    }
    /*    if( widget()->intHalfHeight->value() == v && widget()->cbShape->currentItem() != 1)
            widget()->intAngle->setEnabled(false);
        else
            widget()->intAngle->setEnabled(true);*/
    emit sigConfigurationItemChanged();
}

void KisWdgBlur::spinBoxHalfHeightChanged(int v)
{
    if (m_halfSizeLink) {
        widget()->intHalfWidth->setValue(v);
    }
    /*    if( widget()->intHalfWidth->value() == v && widget()->cbShape->currentItem() != 1)
            widget()->intAngle->setEnabled(false);
        else
            widget()->intAngle->setEnabled(true);*/
    emit sigConfigurationItemChanged();
}

