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
#include <kis_signals_blocker.h>
#include <KisSpinBoxI18nHelper.h>

#include "ui_wdgpatternoptions.h"

KisWdgPattern::KisWdgPattern(QWidget* parent)
        : KisConfigWidget(parent)
{
    m_widget = new Ui_WdgPatternOptions();
    m_widget->setupUi(this);
    m_widget->lblPattern->setVisible(false);

    m_widget->sldShearX->setRange(-500, 500, 2);
    m_widget->sldShearX->setSoftRange(-100, 100);
    m_widget->sldShearX->setSingleStep(1);
    m_widget->sldShearX->setValue(0.0);
    m_widget->sldShearY->setRange(-500, 500, 2);
    m_widget->sldShearY->setSoftRange(-100, 100);
    m_widget->sldShearY->setSingleStep(1);
    m_widget->sldShearY->setValue(0.0);

    m_widget->spbOffsetX->setRange(-10000, 10000);
    m_widget->spbOffsetX->setSoftRange(-500, 500);
    m_widget->spbOffsetX->setValue(0);
    m_widget->spbOffsetY->setRange(-10000, 10000);
    m_widget->spbOffsetY->setSoftRange(-500, 500);
    m_widget->spbOffsetY->setValue(0);

    m_widget->spbScaleWidth->setRange(0, 10000, 2);
    m_widget->spbScaleWidth->setSoftRange(0, 500);
    m_widget->spbScaleWidth->setValue(100.0);
    m_widget->spbScaleHeight->setRange(0, 10000, 2);
    m_widget->spbScaleHeight->setSoftRange(0, 500);
    m_widget->spbScaleHeight->setValue(100.0);

    m_widget->angleSelectorRotationX->setPrefix(i18n("X: "));
    m_widget->angleSelectorRotationY->setPrefix(i18n("Y: "));
    m_widget->angleSelectorRotationZ->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);

    m_widget->container3DRotation->setVisible(false);

    m_widget->sliderAlignToPixelGridX->setRange(1, 20);
    m_widget->sliderAlignToPixelGridY->setRange(1, 20);
    KisSpinBoxI18nHelper::install(m_widget->sliderAlignToPixelGridX, [](int value) {
        // i18n: This is meant to be used in a spinbox so keep the {n} in the text
        //       and it will be substituted by the number. The text before will be
        //       used as the prefix and the text after as the suffix
        return i18ncp("Horizontal pixel grid alignment prefix/suffix for spinboxes in pattern generator", "Every {n} repetition horizontally", "Every {n} repetitions horizontally", value);
    });
    KisSpinBoxI18nHelper::install(m_widget->sliderAlignToPixelGridY, [](int value) {
        // i18n: This is meant to be used in a spinbox so keep the {n} in the text
        //       and it will be substituted by the number. The text before will be
        //       used as the prefix and the text after as the suffix
        return i18ncp("Vertical pixel grid alignment prefix/suffix for spinboxes in pattern generator", "Every {n} repetition vertically", "Every {n} repetitions vertically", value);
    });

    connect(m_widget->patternChooser, SIGNAL(resourceSelected(KoResourceSP)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_widget->sldShearX, SIGNAL(valueChanged(double)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->sldShearY, SIGNAL(valueChanged(double)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_widget->spbOffsetX, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->spbOffsetY, SIGNAL(valueChanged(int)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_widget->spbScaleWidth, SIGNAL(valueChanged(double)), this, SLOT(slotWidthChanged(double)));
    connect(m_widget->spbScaleHeight, SIGNAL(valueChanged(double)), this, SLOT(slotHeightChanged(double)));
    connect(m_widget->btnLockAspectRatio, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slotScaleAspectRatioChanged(bool)));

    connect(m_widget->angleSelectorRotationX, SIGNAL(angleChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->angleSelectorRotationY, SIGNAL(angleChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->angleSelectorRotationZ, SIGNAL(angleChanged(qreal)), this, SIGNAL(sigConfigurationUpdated()));

    connect(m_widget->checkBoxAlignToPixelGrid, SIGNAL(toggled(bool)), this, SIGNAL(sigConfigurationUpdated()));
    connect(m_widget->sliderAlignToPixelGridX, SIGNAL(valueChanged(int)), this, SLOT(slot_sliderAlignToPixelGridX_valueChanged(int)));
    connect(m_widget->sliderAlignToPixelGridY, SIGNAL(valueChanged(int)), this, SLOT(slot_sliderAlignToPixelGridY_valueChanged(int)));
}

KisWdgPattern::~KisWdgPattern()
{
    delete m_widget;
}


void KisWdgPattern::setConfiguration(const KisPropertiesConfigurationSP config)
{
    auto source = KisGlobalResourcesInterface::instance()->source<KoPattern>(ResourceType::Patterns);

    {
        KisSignalsBlocker blocker1(m_widget->patternChooser, m_widget->spbOffsetX, m_widget->spbOffsetY,
                                   m_widget->spbScaleWidth, m_widget->spbScaleHeight, m_widget->btnLockAspectRatio);
        KisSignalsBlocker blocker2(m_widget->sldShearX, m_widget->sldShearY, m_widget->angleSelectorRotationX,
                                   m_widget->angleSelectorRotationY, m_widget->angleSelectorRotationZ);
        KisSignalsBlocker blocker3(m_widget->checkBoxAlignToPixelGrid, m_widget->sliderAlignToPixelGridX,
                                   m_widget->sliderAlignToPixelGridY);

        QString md5sum = config->getString("md5sum");
        QString patternName = config->getString("pattern", "Grid01.pat");
        KoPatternSP pattern = source.bestMatch(md5sum, "", patternName);
        widget()->patternChooser->setCurrentPattern(pattern ? pattern : source.fallbackResource());
        m_widget->spbOffsetX->setValue(config->getInt("transform_offset_x", 0));
        m_widget->spbOffsetY->setValue(config->getInt("transform_offset_y", 0));

        m_widget->spbScaleWidth->setValue(config->getDouble("transform_scale_x", 1.0) * 100);
        m_widget->spbScaleHeight->setValue(config->getDouble("transform_scale_y", 1.0) * 100);
        m_widget->btnLockAspectRatio->setKeepAspectRatio(config->getBool("transform_keep_scale_aspect", true));

        m_widget->sldShearX->setValue(config->getDouble("transform_shear_x", 0.0) * 100);
        m_widget->sldShearY->setValue(config->getDouble("transform_shear_y", 0.0) * 100);

        widget()->angleSelectorRotationX->setAngle(config->getDouble("transform_rotation_x", 0.0));
        widget()->angleSelectorRotationY->setAngle(config->getDouble("transform_rotation_y", 0.0));
        widget()->angleSelectorRotationZ->setAngle(config->getDouble("transform_rotation_z", 0.0));

        m_widget->checkBoxAlignToPixelGrid->setChecked(config->getBool("transform_align_to_pixel_grid", false));
        m_widget->sliderAlignToPixelGridX->setValue(config->getInt("transform_align_to_pixel_grid_x", 1));
        m_widget->sliderAlignToPixelGridY->setValue(config->getInt("transform_align_to_pixel_grid_y", 1));
    }
    Q_EMIT sigConfigurationItemChanged();
}

KisPropertiesConfigurationSP KisWdgPattern::configuration() const
{
    KisGeneratorSP generator = KisGeneratorRegistry::instance()->get("pattern");
    KisFilterConfigurationSP config = generator->factoryConfiguration(KisGlobalResourcesInterface::instance());

    QVariant v;
    KoResourceSP pattern = widget()->patternChooser->currentResource(true);
    if (pattern) {
        config->setProperty("pattern", pattern->name());
        config->setProperty("md5sum", pattern->md5Sum());
        config->setProperty("fileName", pattern->filename());
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

    config->setProperty("transform_align_to_pixel_grid", m_widget->checkBoxAlignToPixelGrid->isChecked());
    config->setProperty("transform_align_to_pixel_grid_x", m_widget->sliderAlignToPixelGridX->value());
    config->setProperty("transform_align_to_pixel_grid_y", m_widget->sliderAlignToPixelGridY->value());

    return config;
}

void KisWdgPattern::slotWidthChanged(double w)
{
    if (m_widget->btnLockAspectRatio->keepAspectRatio()) {
        KisSignalsBlocker blocker(m_widget->spbScaleHeight);
        m_widget->spbScaleHeight->setValue(w);
    }
    Q_EMIT sigConfigurationItemChanged();
}

void KisWdgPattern::slotHeightChanged(double h)
{
    if (m_widget->btnLockAspectRatio->keepAspectRatio()) {
        KisSignalsBlocker blocker(m_widget->spbScaleWidth);
        m_widget->spbScaleWidth->setValue(h);
    }
    Q_EMIT sigConfigurationItemChanged();
}

void KisWdgPattern::slotScaleAspectRatioChanged(bool checked)
{
    if (checked && m_widget->spbScaleHeight->value() != m_widget->spbScaleWidth->value()) {
        KisSignalsBlocker blocker(m_widget->spbScaleHeight);
        m_widget->spbScaleHeight->setValue(m_widget->spbScaleWidth->value());
        Q_EMIT sigConfigurationItemChanged();
    }
}

void KisWdgPattern::slot_sliderAlignToPixelGridX_valueChanged(int value)
{
    Q_UNUSED(value);
    if (m_widget->checkBoxAlignToPixelGrid->isChecked()) {
        Q_EMIT sigConfigurationItemChanged();
    }
}

void KisWdgPattern::slot_sliderAlignToPixelGridY_valueChanged(int value)
{
    Q_UNUSED(value);
    if (m_widget->checkBoxAlignToPixelGrid->isChecked()) {
        Q_EMIT sigConfigurationItemChanged();
    }
}

