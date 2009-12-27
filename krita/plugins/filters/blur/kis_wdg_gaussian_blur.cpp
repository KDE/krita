/*
 * This file is part of Krita
 *
 * Copyright (c) 2009 Edward Apap <schumifer@hotmail.com>
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

#include "kis_wdg_gaussian_blur.h"
#include <QLayout>
#include <QToolButton>
#include <QIcon>

#include <kcombobox.h>
#include <knuminput.h>

#include <KoImageResource.h>

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>

#include "ui_wdg_gaussian_blur.h"

KisWdgGaussianBlur::KisWdgGaussianBlur(QWidget * parent) : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgGaussianBlur();
    m_widget->setupUi(this);

    m_widget->aspectButton->setKeepAspectRatio(false);

    connect(m_widget->horizontalRadius, SIGNAL(valueChanged(int)), this, SLOT(horizontalRadiusChanged(int)));
    connect(m_widget->verticalRadius, SIGNAL(valueChanged(int)), this, SLOT(verticalRadiusChanged(int)));
    connect(m_widget->aspectButton, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(aspectLockChanged(bool)));

    connect(m_widget->horizontalRadius, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_widget->verticalRadius, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
}

KisPropertiesConfiguration* KisWdgGaussianBlur::configuration() const
{
    KisFilterConfiguration* config = new KisFilterConfiguration("gaussian blur", 1);
    config->setProperty("horizRadius", m_widget->horizontalRadius->value());
    config->setProperty("vertRadius", m_widget->verticalRadius->value());
    config->setProperty("lockAspect", m_widget->aspectButton->keepAspectRatio());
    return config;
}

void KisWdgGaussianBlur::setConfiguration(const KisPropertiesConfiguration* config)
{
    QVariant value;
    if (config->getProperty("horizRadius", value)) {
        m_widget->horizontalRadius->setValue(value.toInt());
    }
    if (config->getProperty("vertRadius", value)) {
        m_widget->verticalRadius->setValue(value.toInt());
    }
    if (config->getProperty("lockAspect", value)) {
        m_widget->aspectButton->setKeepAspectRatio(value.toBool());
    }
}

void KisWdgGaussianBlur::horizontalRadiusChanged(int v)
{
    if (m_widget->aspectButton->keepAspectRatio())
        m_widget->verticalRadius->setValue(v);
}

void KisWdgGaussianBlur::verticalRadiusChanged(int v)
{
    if (m_widget->aspectButton->keepAspectRatio())
        m_widget->horizontalRadius->setValue(v);
}

void KisWdgGaussianBlur::aspectLockChanged(bool v)
{
    if (v) {
        m_widget->verticalRadius->setValue( m_widget->horizontalRadius->value() );
    }
}

#include "kis_wdg_gaussian_blur.moc" 
