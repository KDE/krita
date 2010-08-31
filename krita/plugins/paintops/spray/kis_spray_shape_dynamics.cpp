/*
 *  Copyright (c) 2008,2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_spray_shape_dynamics.h"
#include <klocale.h>

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
        : KisPaintOpOption(i18n("Shape dynamics"), KisPaintOpOption::brushCategory(), true)
{
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
    connect(m_options->randomSizeCHBox, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->fixedRotation, SIGNAL(toggled(bool)),SIGNAL(sigSettingChanged()));
    connect(m_options->fixedAngleBox,SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->randomRotation, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->randomAngleWeight,SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->followCursor, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->followCursorWeight,SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
    connect(m_options->drawingAngle, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->drawingAngleWeight,SIGNAL(valueChanged(double)),SIGNAL(sigSettingChanged()));
}


KisSprayShapeDynamicsOption::~KisSprayShapeDynamicsOption()
{
    delete m_options;
}

void KisSprayShapeDynamicsOption::writeOptionSetting(KisPropertiesConfiguration* settings) const
{
    settings->setProperty(SHAPE_DYNAMICS_VERSION, "2.3");
    settings->setProperty(SHAPE_DYNAMICS_ENABLED, isChecked());
    settings->setProperty(SHAPE_DYNAMICS_RANDOM_SIZE, m_options->randomSizeCHBox->isChecked());
    settings->setProperty(SHAPE_DYNAMICS_FIXED_ROTATION, m_options->fixedRotation->isChecked());
    settings->setProperty(SHAPE_DYNAMICS_FIXED_ANGEL, m_options->fixedAngleBox->value());
    settings->setProperty(SHAPE_DYNAMICS_RANDOM_ROTATION, m_options->randomRotation->isChecked());
    settings->setProperty(SHAPE_DYNAMICS_RANDOM_ROTATION_WEIGHT, m_options->randomAngleWeight->value());
    settings->setProperty(SHAPE_DYNAMICS_FOLLOW_CURSOR, m_options->followCursor->isChecked());
    settings->setProperty(SHAPE_DYNAMICS_FOLLOW_CURSOR_WEIGHT, m_options->followCursorWeight->value());
    settings->setProperty(SHAPE_DYNAMICS_DRAWING_ANGLE, m_options->drawingAngle->isChecked());
    settings->setProperty(SHAPE_DYNAMICS_DRAWING_ANGLE_WEIGHT, m_options->drawingAngleWeight->value());
}


void KisSprayShapeDynamicsOption::readOptionSetting(const KisPropertiesConfiguration* settings)
{
    // backward compatibility with 2.2
    if (settings->getString(SHAPE_DYNAMICS_VERSION, "2.2") == "2.2")
    {
        m_options->randomSizeCHBox->setChecked(settings->getBool(SPRAYSHAPE_RANDOM_SIZE));
        m_options->fixedRotation->setChecked(settings->getBool(SPRAYSHAPE_FIXED_ROTATION));
        m_options->fixedAngleBox->setValue(settings->getDouble(SPRAYSHAPE_FIXED_ANGEL));
        m_options->followCursor->setChecked(settings->getBool(SPRAYSHAPE_FOLLOW_CURSOR));
        m_options->followCursorWeight->setValue(settings->getDouble(SPRAYSHAPE_FOLLOW_CURSOR_WEIGHT) );
        m_options->drawingAngle->setChecked(settings->getBool(SPRAYSHAPE_DRAWING_ANGLE));
        m_options->drawingAngleWeight->setValue(settings->getDouble(SPRAYSHAPE_DRAWING_ANGLE_WEIGHT) );
        m_options->randomRotation->setChecked(settings->getBool(SPRAYSHAPE_RANDOM_ROTATION));
        m_options->randomAngleWeight->setValue(settings->getDouble(SPRAYSHAPE_RANDOM_ROTATION_WEIGHT) );
        setChecked(true);
    }else
    {
        setChecked(settings->getBool(SHAPE_DYNAMICS_ENABLED));
        m_options->randomSizeCHBox->setChecked(settings->getBool(SHAPE_DYNAMICS_RANDOM_SIZE));
        m_options->fixedRotation->setChecked(settings->getBool(SHAPE_DYNAMICS_FIXED_ROTATION));
        m_options->fixedAngleBox->setValue(settings->getDouble(SHAPE_DYNAMICS_FIXED_ANGEL));
        m_options->followCursor->setChecked(settings->getBool(SHAPE_DYNAMICS_FOLLOW_CURSOR));
        m_options->followCursorWeight->setValue(settings->getDouble(SHAPE_DYNAMICS_FOLLOW_CURSOR_WEIGHT) );
        m_options->drawingAngle->setChecked(settings->getBool(SHAPE_DYNAMICS_DRAWING_ANGLE));
        m_options->drawingAngleWeight->setValue(settings->getDouble(SHAPE_DYNAMICS_DRAWING_ANGLE_WEIGHT) );
        m_options->randomRotation->setChecked(settings->getBool(SHAPE_DYNAMICS_RANDOM_ROTATION));
        m_options->randomAngleWeight->setValue(settings->getDouble(SHAPE_DYNAMICS_RANDOM_ROTATION_WEIGHT) );
    }        
}
