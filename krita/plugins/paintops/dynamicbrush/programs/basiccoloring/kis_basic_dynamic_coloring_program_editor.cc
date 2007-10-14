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
