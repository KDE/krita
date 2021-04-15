/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    m_widget->angleSelectorRotationZ->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);

    m_widget->gb3dRotation->setVisible(false);

    connect(m_widget->patternChooser, SIGNAL(resourceSelected(KoResourceSP)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_widget->sldShearX, SIGNAL(valueChanged(double)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->sldShearY, SIGNAL(valueChanged(double)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_widget->spbOffsetX, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->spbOffsetY, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_widget->spbScaleWidth, SIGNAL(valueChanged(double)), this, SLOT(slotWidthChanged(double)));
    connect(m_widget->spbScaleHeight, SIGNAL(valueChanged(double)), this, SLOT(slotHeightChanged(double)));

    connect(m_widget->angleSelectorRotationX, SIGNAL(angleChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->angleSelectorRotationY, SIGNAL(angleChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->angleSelectorRotationZ, SIGNAL(angleChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
}

KisWdgPattern::~KisWdgPattern()
{
    delete m_widget;
}


void KisWdgPattern::setConfiguration(const KisPropertiesConfigurationSP config)
{
    auto source = KisGlobalResourcesInterface::instance()->source<KoPattern>(ResourceType::Patterns);

    qDebug() << "KisWdgPattern::setConfiguration();" << config->getString("pattern", "Grid01.pat");
    KoPatternSP pattern = source.resourceForName(config->getString("pattern", "Grid01.pat"));
    widget()->patternChooser->setCurrentPattern(pattern ? pattern : source.fallbackResource());
    m_widget->spbOffsetX->setValue(config->getInt("transform_offset_x", 0));
    m_widget->spbOffsetY->setValue(config->getInt("transform_offset_y", 0));

    m_widget->spbScaleWidth->setValue(config->getInt("transform_scale_x", 1.0) * 100);
    m_widget->spbScaleHeight->setValue(config->getInt("transform_scale_y", 1.0) * 100);
    m_widget->btnLockAspectRatio->setKeepAspectRatio(config->getBool("transform_keep_scale_aspect", true));

    m_widget->sldShearX->setValue(config->getDouble("transform_shear_x", 0.0) * 100);
    m_widget->sldShearY->setValue(config->getDouble("transform_shear_y", 0.0) * 100);

    widget()->angleSelectorRotationX->setAngle(config->getDouble("transform_rotation_x", 0.0));
    widget()->angleSelectorRotationY->setAngle(config->getDouble("transform_rotation_y", 0.0));
    widget()->angleSelectorRotationZ->setAngle(config->getDouble("transform_rotation_z", 0.0));
}

KisPropertiesConfigurationSP KisWdgPattern::configuration() const
{
    KisGeneratorSP generator = KisGeneratorRegistry::instance()->get("pattern");
    KisFilterConfigurationSP config = generator->factoryConfiguration(KisGlobalResourcesInterface::instance());

    QVariant v;
    if (widget()->patternChooser->currentResource()) {
        v.setValue(widget()->patternChooser->currentResource()->name());
        qDebug() << "KisWdgPattern::configuration()" << widget()->patternChooser->currentResource();
        config->setProperty("pattern", v);
    }

    config->setProperty("transform_offset_x", m_widget->spbOffsetX->value());
    config->setProperty("transform_offset_y", m_widget->spbOffsetY->value());

    config->setProperty("transform_scale_x", m_widget->spbScaleWidth->value()  / 100);
    config->setProperty("transform_scale_y", m_widget->spbScaleHeight->value() / 100);

    config->setProperty("transform_keep_scale_aspect", m_widget->btnLockAspectRatio->keepAspectRatio());

    config->setProperty("transform_shear_x", widget()->sldShearX->value() / 100);
    config->setProperty("transform_shear_y", widget()->sldShearY->value() / 100);

    config->setProperty("transform_rotation_x", widget()->angleSelectorRotationX->angle());
    config->setProperty("transform_rotation_y", widget()->angleSelectorRotationY->angle());
    config->setProperty("transform_rotation_z", widget()->angleSelectorRotationZ->angle());

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

