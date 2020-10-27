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

#include "kis_wdg_pattern.h"

#include <QLayout>
#include <QLabel>
#include <QSlider>

#include <KoColor.h>
#include <resources/KoPattern.h>
#include <KisGlobalResourcesInterface.h>
#include <kis_generator_registry.h>

#include <filter/kis_filter_configuration.h>
#include <kis_pattern_chooser.h>
#include "ui_wdgpatternoptions.h"

KisWdgPattern::KisWdgPattern(QWidget* parent)
        : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgPatternOptions();
    m_widget->setupUi(this);
    m_widget->lblPattern->setVisible(false);

    m_widget->sldShearX->setSuffix(QChar(Qt::Key_Percent));
    m_widget->sldShearY->setSuffix(QChar(Qt::Key_Percent));
    m_widget->sldShearX->setRange(-500, 500, 2);
    m_widget->sldShearY->setRange(-500, 500, 2);
    m_widget->sldShearX->setSingleStep(1);
    m_widget->sldShearY->setSingleStep(1);
    m_widget->sldShearX->setValue(0.0);
    m_widget->sldShearY->setValue(0.0);


    m_widget->spbOffsetX->setSuffix(i18n(" px"));
    m_widget->spbOffsetY->setSuffix(i18n(" px"));
    m_widget->spbOffsetX->setRange(-10000, 10000);
    m_widget->spbOffsetY->setRange(-10000, 10000);

    m_widget->sldRotationX->setSuffix(QChar(Qt::Key_degree));
    m_widget->sldRotationY->setSuffix(QChar(Qt::Key_degree));
    m_widget->sldRotationZ->setSuffix(QChar(Qt::Key_degree));
    m_widget->sldRotationX->setRange(0.0, 360.0, 2);
    m_widget->sldRotationY->setRange(0.0, 360.0, 2);
    m_widget->sldRotationZ->setRange(0.0, 360.0, 2);
    m_widget->sldRotationX->setValue(0.0);
    m_widget->sldRotationY->setValue(0.0);
    m_widget->sldRotationZ->setValue(0.0);
    m_widget->sldRotationX->setSingleStep(1.0);
    m_widget->sldRotationY->setSingleStep(1.0);
    m_widget->sldRotationZ->setSingleStep(1.0);

    m_widget->gb3dRotation->setVisible(false);

    connect(m_widget->patternChooser, SIGNAL(resourceSelected(KoResourceSP)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_widget->sldShearX, SIGNAL(valueChanged(double)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->sldShearY, SIGNAL(valueChanged(double)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_widget->spbOffsetX, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->spbOffsetY, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_widget->spbScaleWidth, SIGNAL(valueChanged(double)), this, SLOT(slotWidthChanged(double)));
    connect(m_widget->spbScaleHeight, SIGNAL(valueChanged(double)), this, SLOT(slotHeightChanged(double)));

    connect(m_widget->sldRotationX, SIGNAL(valueChanged(double)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->sldRotationY, SIGNAL(valueChanged(double)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->sldRotationZ, SIGNAL(valueChanged(double)), this, SIGNAL(sigConfigurationUpdated()));
}

KisWdgPattern::~KisWdgPattern()
{
    delete m_widget;
}


void KisWdgPattern::setConfiguration(const KisPropertiesConfigurationSP config)
{
    auto source = KisGlobalResourcesInterface::instance()->source<KoPattern>(ResourceType::Patterns);
    KoPatternSP pattern = source.resourceForName(config->getString("pattern", "Grid01.pat"));
    widget()->patternChooser->setCurrentPattern(pattern ? pattern : source.fallbackResource());
    m_widget->spbOffsetX->setValue(config->getInt("transform_offset_x", 0));
    m_widget->spbOffsetY->setValue(config->getInt("transform_offset_y", 0));

    m_widget->spbScaleWidth->setValue(config->getInt("transform_scale_x", 1.0) * 100);
    m_widget->spbScaleHeight->setValue(config->getInt("transform_scale_y", 1.0) * 100);
    m_widget->btnLockAspectRatio->setKeepAspectRatio(config->getBool("transform_keep_scale_aspect", true));

    m_widget->sldShearX->setValue(config->getDouble("transform_shear_x", 0.0) * 100);
    m_widget->sldShearY->setValue(config->getDouble("transform_shear_y", 0.0) * 100);

    widget()->sldRotationX->setValue(config->getDouble("transform_rotation_x", 0.0));
    widget()->sldRotationY->setValue(config->getDouble("transform_rotation_y", 0.0));
    widget()->sldRotationZ->setValue(config->getDouble("transform_rotation_z", 0.0));
}

KisPropertiesConfigurationSP KisWdgPattern::configuration() const
{
    KisGeneratorSP generator = KisGeneratorRegistry::instance()->get("pattern");
    KisFilterConfigurationSP config = generator->factoryConfiguration(KisGlobalResourcesInterface::instance());

    QVariant v;
    if (widget()->patternChooser->currentResource()) {
        v.setValue(widget()->patternChooser->currentResource()->name());
        config->setProperty("pattern", v);
    }

    config->setProperty("transform_offset_x", m_widget->spbOffsetX->value());
    config->setProperty("transform_offset_y", m_widget->spbOffsetY->value());

    config->setProperty("transform_scale_x", m_widget->spbScaleWidth->value()  / 100);
    config->setProperty("transform_scale_y", m_widget->spbScaleHeight->value() / 100);

    config->setProperty("transform_keep_scale_aspect", m_widget->btnLockAspectRatio->keepAspectRatio());

    config->setProperty("transform_shear_x", widget()->sldShearX->value() / 100);
    config->setProperty("transform_shear_y", widget()->sldShearY->value() / 100);

    config->setProperty("transform_rotation_x", widget()->sldRotationX->value());
    config->setProperty("transform_rotation_y", widget()->sldRotationY->value());
    config->setProperty("transform_rotation_z", widget()->sldRotationZ->value());

    return config;
}

void KisWdgPattern::slotWidthChanged(double w)
{
    if (m_widget->btnLockAspectRatio->keepAspectRatio()) {
        m_widget->spbScaleHeight->blockSignals(true);
        m_widget->spbScaleHeight->setValue(w);
        m_widget->spbScaleHeight->blockSignals(false);
    }
    emit sigConfigurationUpdated();
}

void KisWdgPattern::slotHeightChanged(double h)
{
    if (m_widget->btnLockAspectRatio->keepAspectRatio()) {
        m_widget->spbScaleWidth->blockSignals(true);
        m_widget->spbScaleWidth->setValue(h);
        m_widget->spbScaleWidth->blockSignals(false);
    }
    emit sigConfigurationUpdated();
}

