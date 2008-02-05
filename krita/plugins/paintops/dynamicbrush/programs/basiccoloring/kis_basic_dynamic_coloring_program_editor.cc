/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_basic_dynamic_coloring_program_editor.h"

#include "kis_basic_dynamic_coloring_program.h"

#include "ui_BasicDynamicColoringProgramEditor.h"

KisBasicDynamicColoringProgramEditor::KisBasicDynamicColoringProgramEditor(KisBasicDynamicColoringProgram* program) :
        m_program(program)
{
    m_basicDynamicColoringProgramEditor = new Ui_BasicDynamicColoringProgramEditor();
    m_basicDynamicColoringProgramEditor->setupUi(this);
    // Connect to enable/disable widgets
    connect(m_basicDynamicColoringProgramEditor->mixerEnabled, SIGNAL(clicked(bool)), SLOT(setMixerEnable(bool)));
    connect(m_basicDynamicColoringProgramEditor->hueEnabled, SIGNAL(clicked(bool)), SLOT(setHueEnable(bool)));
    connect(m_basicDynamicColoringProgramEditor->saturationEnabled, SIGNAL(clicked(bool)), SLOT(setSaturationEnable(bool)));
    connect(m_basicDynamicColoringProgramEditor->brightnessEnabled, SIGNAL(clicked(bool)), SLOT(setBrightnessEnable(bool)));
    // Connect to edit the program
    // Connect to edit the mixer
    connect(m_basicDynamicColoringProgramEditor->mixerEnabled, SIGNAL(clicked(bool)), program, SLOT(setMixerEnable(bool)));
    connect(m_basicDynamicColoringProgramEditor->mixerJitter, SIGNAL(valueChanged(int)), program, SLOT(setMixerJitter(int)));
    connect(m_basicDynamicColoringProgramEditor->mixerSensor, SIGNAL(sensorChanged(KisDynamicSensor*)), program, SLOT(setMixerSensor(KisDynamicSensor* )));
    // Connect to edit the hue
    connect(m_basicDynamicColoringProgramEditor->hueEnabled, SIGNAL(clicked(bool)), program, SLOT(setHueEnable(bool)));
    connect(m_basicDynamicColoringProgramEditor->hueJitter, SIGNAL(valueChanged(int)), program, SLOT(setHueJitter(int)));
    connect(m_basicDynamicColoringProgramEditor->hueSensor, SIGNAL(sensorChanged(KisDynamicSensor*)), program, SLOT(setHueSensor(KisDynamicSensor* )));
    // Connect to edit the saturation
    connect(m_basicDynamicColoringProgramEditor->saturationEnabled, SIGNAL(clicked(bool)), program, SLOT(setSaturationEnable(bool)));
    connect(m_basicDynamicColoringProgramEditor->saturationJitter, SIGNAL(valueChanged(int)), program, SLOT(setSaturationJitter(int)));
    connect(m_basicDynamicColoringProgramEditor->saturationSensor, SIGNAL(sensorChanged(KisDynamicSensor*)), program, SLOT(setSaturationSensor(KisDynamicSensor* )));
    // Connect to edit the brightness
    connect(m_basicDynamicColoringProgramEditor->brightnessEnabled, SIGNAL(clicked(bool)), program, SLOT(setBrightnessEnable(bool)));
    connect(m_basicDynamicColoringProgramEditor->brightnessJitter, SIGNAL(valueChanged(int)), program, SLOT(setBrightnessJitter(int)));
    connect(m_basicDynamicColoringProgramEditor->brightnessSensor, SIGNAL(sensorChanged(KisDynamicSensor*)), program, SLOT(setBrightnessSensor(KisDynamicSensor* )));
    // Set the value
    setMixerEnable(program->isMixerEnabled());
    m_basicDynamicColoringProgramEditor->mixerEnabled->setChecked( program->isMixerEnabled() );
    m_basicDynamicColoringProgramEditor->mixerJitter->setValue( program->mixerJitter() );
    m_basicDynamicColoringProgramEditor->mixerSensor->setCurrent( program->mixerSensor() );
    setHueEnable(program->isHueEnabled());
    m_basicDynamicColoringProgramEditor->hueEnabled->setChecked( program->isHueEnabled() );
    m_basicDynamicColoringProgramEditor->hueJitter->setValue( program->hueJitter() );
    m_basicDynamicColoringProgramEditor->hueSensor->setCurrent( program->hueSensor() );
    setSaturationEnable(program->isSaturationEnabled());
    m_basicDynamicColoringProgramEditor->saturationEnabled->setChecked( program->isSaturationEnabled() );
    m_basicDynamicColoringProgramEditor->saturationJitter->setValue( program->saturationJitter() );
    m_basicDynamicColoringProgramEditor->saturationSensor->setCurrent( program->saturationSensor() );
    setBrightnessEnable(program->isBrightnessEnabled());
    m_basicDynamicColoringProgramEditor->brightnessEnabled->setChecked( program->isBrightnessEnabled() );
    m_basicDynamicColoringProgramEditor->brightnessJitter->setValue( program->brightnessJitter() );
    m_basicDynamicColoringProgramEditor->brightnessSensor->setCurrent( program->brightnessSensor() );
}

KisBasicDynamicColoringProgramEditor::~KisBasicDynamicColoringProgramEditor()
{
    delete m_basicDynamicColoringProgramEditor;
}

void KisBasicDynamicColoringProgramEditor::setMixerEnable(bool v)
{
    m_basicDynamicColoringProgramEditor->labelMixerJitter->setEnabled(v);
    m_basicDynamicColoringProgramEditor->mixerJitter->setEnabled(v);
    m_basicDynamicColoringProgramEditor->labelMixerSensor->setEnabled(v);
    m_basicDynamicColoringProgramEditor->mixerSensor->setEnabled(v);
}

void KisBasicDynamicColoringProgramEditor::setHueEnable(bool v)
{
    m_basicDynamicColoringProgramEditor->labelHueJitter->setEnabled(v);
    m_basicDynamicColoringProgramEditor->hueJitter->setEnabled(v);
    m_basicDynamicColoringProgramEditor->labelHueSensor->setEnabled(v);
    m_basicDynamicColoringProgramEditor->hueSensor->setEnabled(v);
}

void KisBasicDynamicColoringProgramEditor::setSaturationEnable(bool v)
{
    m_basicDynamicColoringProgramEditor->labelSaturationJitter->setEnabled(v);
    m_basicDynamicColoringProgramEditor->saturationJitter->setEnabled(v);
    m_basicDynamicColoringProgramEditor->labelSaturationSensor->setEnabled(v);
    m_basicDynamicColoringProgramEditor->saturationSensor->setEnabled(v);
}

void KisBasicDynamicColoringProgramEditor::setBrightnessEnable(bool v)
{
    m_basicDynamicColoringProgramEditor->labelBrightnessJitter->setEnabled(v);
    m_basicDynamicColoringProgramEditor->brightnessJitter->setEnabled(v);
    m_basicDynamicColoringProgramEditor->labelBrightnessSensor->setEnabled(v);
    m_basicDynamicColoringProgramEditor->brightnessSensor->setEnabled(v);
}

#include "kis_basic_dynamic_coloring_program_editor.moc"
