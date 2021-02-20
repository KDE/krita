/*
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_spray_shape_dynamics.h"
#include <klocalizedstring.h>

#include "ui_wdgshapedynamicsoptions.h"

class KisShapeDynamicsOptionsWidget: public QWidget, public Ui::WdgShapeDynamicsOptions
{
public:
    KisShapeDynamicsOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
};

KisSprayShapeDynamicsOption::KisSprayShapeDynamicsOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, true)
{
    setObjectName("KisSprayShapeDynamicsOption");

    m_checkable = true;
    m_options = new KisShapeDynamicsOptionsWidget();


    // UI signals
    connect(m_options->fixedRotation, SIGNAL(toggled(bool)), m_options->fixedAngleBox, SLOT(setEnabled(bool)));
    connect(m_options->randomRotation, SIGNAL(toggled(bool)), m_options->randomAngleWeight, SLOT(setEnabled(bool)));
    connect(m_options->followCursor, SIGNAL(toggled(bool)), m_options->followCursorWeight, SLOT(setEnabled(bool)));
    connect(m_options->drawingAngle, SIGNAL(toggled(bool)), m_options->drawingAngleWeight, SLOT(setEnabled(bool)));

    setupBrushPreviewSignals();
    setConfigurationPage(m_options);
}


void KisSprayShapeDynamicsOption::setupBrushPreviewSignals()
{
     // initialize sliders


    m_options->drawingAngleWeight->setRange(0.0, 1.0, 2);
    m_options->drawingAngleWeight->setDisabled(true);

    m_options->followCursorWeight->setRange(0.0, 1.0, 2);
    m_options->followCursorWeight->setDisabled(true);

    m_options->randomAngleWeight->setRange(0.0, 1.0, 2);
    m_options->randomAngleWeight->setDisabled(true);

    m_options->fixedAngleBox->setDecimals(0);
    m_options->fixedAngleBox->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);
    m_options->fixedAngleBox->setDisabled(true);

    connect(m_options->randomSizeCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->fixedRotation, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->fixedAngleBox, SIGNAL(angleChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->randomRotation, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->randomAngleWeight, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->followCursor, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->followCursorWeight, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->drawingAngle, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->drawingAngleWeight, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
}


KisSprayShapeDynamicsOption::~KisSprayShapeDynamicsOption()
{
    delete m_options;
}

void KisSprayShapeDynamicsOption::writeOptionSetting(KisPropertiesConfigurationSP settings) const
{
    settings->setProperty(SHAPE_DYNAMICS_VERSION, "2.3");
    settings->setProperty(SHAPE_DYNAMICS_ENABLED, isChecked());
    settings->setProperty(SHAPE_DYNAMICS_RANDOM_SIZE, m_options->randomSizeCHBox->isChecked());
    settings->setProperty(SHAPE_DYNAMICS_FIXED_ROTATION, m_options->fixedRotation->isChecked());
    settings->setProperty(SHAPE_DYNAMICS_FIXED_ANGEL, m_options->fixedAngleBox->angle());
    settings->setProperty(SHAPE_DYNAMICS_RANDOM_ROTATION, m_options->randomRotation->isChecked());
    settings->setProperty(SHAPE_DYNAMICS_RANDOM_ROTATION_WEIGHT, m_options->randomAngleWeight->value());
    settings->setProperty(SHAPE_DYNAMICS_FOLLOW_CURSOR, m_options->followCursor->isChecked());
    settings->setProperty(SHAPE_DYNAMICS_FOLLOW_CURSOR_WEIGHT, m_options->followCursorWeight->value());
    settings->setProperty(SHAPE_DYNAMICS_DRAWING_ANGLE, m_options->drawingAngle->isChecked());
    settings->setProperty(SHAPE_DYNAMICS_DRAWING_ANGLE_WEIGHT, m_options->drawingAngleWeight->value());
}


void KisSprayShapeDynamicsOption::readOptionSetting(const KisPropertiesConfigurationSP settings)
{
    // backward compatibility with 2.2
    if (settings->getString(SHAPE_DYNAMICS_VERSION, "2.2") == "2.2") {
        setChecked(true);
        m_options->randomSizeCHBox->setChecked(settings->getBool(SPRAYSHAPE_RANDOM_SIZE));
        m_options->fixedRotation->setChecked(settings->getBool(SPRAYSHAPE_FIXED_ROTATION));
        m_options->fixedAngleBox->setAngle(settings->getDouble(SPRAYSHAPE_FIXED_ANGEL));
        m_options->followCursor->setChecked(settings->getBool(SPRAYSHAPE_FOLLOW_CURSOR));
        m_options->followCursorWeight->setValue(settings->getDouble(SPRAYSHAPE_FOLLOW_CURSOR_WEIGHT));
        m_options->drawingAngle->setChecked(settings->getBool(SPRAYSHAPE_DRAWING_ANGLE));
        m_options->drawingAngleWeight->setValue(settings->getDouble(SPRAYSHAPE_DRAWING_ANGLE_WEIGHT));
        m_options->randomRotation->setChecked(settings->getBool(SPRAYSHAPE_RANDOM_ROTATION));
        m_options->randomAngleWeight->setValue(settings->getDouble(SPRAYSHAPE_RANDOM_ROTATION_WEIGHT));
    }
    else {
        setChecked(settings->getBool(SHAPE_DYNAMICS_ENABLED));
        m_options->randomSizeCHBox->setChecked(settings->getBool(SHAPE_DYNAMICS_RANDOM_SIZE));
        m_options->fixedRotation->setChecked(settings->getBool(SHAPE_DYNAMICS_FIXED_ROTATION));
        m_options->fixedAngleBox->setAngle(settings->getDouble(SHAPE_DYNAMICS_FIXED_ANGEL));
        m_options->followCursor->setChecked(settings->getBool(SHAPE_DYNAMICS_FOLLOW_CURSOR));
        m_options->followCursorWeight->setValue(settings->getDouble(SHAPE_DYNAMICS_FOLLOW_CURSOR_WEIGHT));
        m_options->drawingAngle->setChecked(settings->getBool(SHAPE_DYNAMICS_DRAWING_ANGLE));
        m_options->drawingAngleWeight->setValue(settings->getDouble(SHAPE_DYNAMICS_DRAWING_ANGLE_WEIGHT));
        m_options->randomRotation->setChecked(settings->getBool(SHAPE_DYNAMICS_RANDOM_ROTATION));
        m_options->randomAngleWeight->setValue(settings->getDouble(SHAPE_DYNAMICS_RANDOM_ROTATION_WEIGHT));
    }
}
