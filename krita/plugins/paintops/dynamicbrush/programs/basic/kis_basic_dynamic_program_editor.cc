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

#include "kis_basic_dynamic_program_editor.h"

#include <kdialog.h>
#include <klocale.h>

#include "ui_BasicDynamicProgramEditor.h"

#include "kis_basic_dynamic_program.h"
#include "kis_dynamic_transformation.h"
#include "kis_dynamic_transformations_factory.h"

KisBasicDynamicProgramEditor::KisBasicDynamicProgramEditor(KisBasicDynamicProgram* program) :
        m_program(program)
{
    m_basicDynamicProgramEditor = new Ui_BasicDynamicProgramEditor();
    m_basicDynamicProgramEditor->setupUi(this);
    // Connect to enable/disable widgets
    connect(m_basicDynamicProgramEditor->sizeEnabled, SIGNAL(clicked(bool)), SLOT(setEnableSize(bool)));
    connect(m_basicDynamicProgramEditor->angleEnabled, SIGNAL(clicked(bool)), SLOT(setEnableAngle(bool)));
    connect(m_basicDynamicProgramEditor->scatterEnabled, SIGNAL(clicked(bool)), SLOT(setEnableScatter(bool)));
    connect(m_basicDynamicProgramEditor->countEnabled, SIGNAL(clicked(bool)), SLOT(setEnableCount(bool)));
    // Connect to edit the program
    // Connect to edit the size
    connect(m_basicDynamicProgramEditor->sizeEnabled, SIGNAL(clicked(bool)), program, SLOT(setEnableSize(bool)));
    connect(m_basicDynamicProgramEditor->sizeJitter, SIGNAL(valueChanged(int)), program, SLOT(setSizeJitter(int)));
    connect(m_basicDynamicProgramEditor->sizeMinimum, SIGNAL(valueChanged(int)), program, SLOT(setSizeMinimum(int)));
    connect(m_basicDynamicProgramEditor->sizeMaximum, SIGNAL(valueChanged(int)), program, SLOT(setSizeMaximum(int)));
    connect(m_basicDynamicProgramEditor->sizeSensor, SIGNAL(sensorChanged(KisDynamicSensor*)), program, SLOT(setSizeSensor(KisDynamicSensor* )));
    // Connect to edit the angle
    connect(m_basicDynamicProgramEditor->angleEnabled, SIGNAL(clicked(bool)), program, SLOT(setEnableAngle(bool)));
    connect(m_basicDynamicProgramEditor->angleJitter, SIGNAL(valueChanged(int)), program, SLOT(setAngleJitter(int)));
    connect(m_basicDynamicProgramEditor->angleSensor, SIGNAL(sensorChanged(KisDynamicSensor*)), program, SLOT(setAngleSensor(KisDynamicSensor* )));
    // Connect to edit the scattering
    connect(m_basicDynamicProgramEditor->scatterEnabled, SIGNAL(clicked(bool)), program, SLOT(setEnableScatter(bool)));
    connect(m_basicDynamicProgramEditor->scatterJitter, SIGNAL(valueChanged(int)), program, SLOT(setScatterJitter(int)));
    connect(m_basicDynamicProgramEditor->scatterAmount, SIGNAL(valueChanged(int)), program, SLOT(setScatterAmount(int)));
    connect(m_basicDynamicProgramEditor->scatterSensor, SIGNAL(sensorChanged(KisDynamicSensor*)), program, SLOT(setScatterSensor(KisDynamicSensor* )));
    // Connect to edit the count
    connect(m_basicDynamicProgramEditor->countEnabled, SIGNAL(clicked(bool)), program, SLOT(setEnableCount(bool)));
    connect(m_basicDynamicProgramEditor->countJitter, SIGNAL(valueChanged(int)), program, SLOT(setCountJitter(int)));
    connect(m_basicDynamicProgramEditor->countCount, SIGNAL(valueChanged(int)), program, SLOT(setCountCount(int)));
    connect(m_basicDynamicProgramEditor->countSensor, SIGNAL(sensorChanged(KisDynamicSensor*)), program, SLOT(setCountSensor(KisDynamicSensor* )));
    // Set the value
    setEnableSize(program->isSizeEnabled());
    m_basicDynamicProgramEditor->sizeEnabled->setChecked( program->isSizeEnabled() );
    m_basicDynamicProgramEditor->sizeJitter->setValue( program->sizeJitter() );
    m_basicDynamicProgramEditor->sizeMinimum->setValue( program->sizeMinimum() );
    m_basicDynamicProgramEditor->sizeMaximum->setValue( program->sizeMaximum() );
    m_basicDynamicProgramEditor->sizeSensor->setCurrent( program->sizeSensor() );
    setEnableAngle(program->isAngleEnabled());
    m_basicDynamicProgramEditor->angleEnabled->setChecked( program->isAngleEnabled() );
    m_basicDynamicProgramEditor->angleJitter->setValue( program->angleJitter() );
    m_basicDynamicProgramEditor->angleSensor->setCurrent( program->angleSensor() );
    setEnableScatter(program->isScatterEnabled());
    m_basicDynamicProgramEditor->scatterEnabled->setChecked( program->isScatterEnabled() );
    m_basicDynamicProgramEditor->scatterJitter->setValue( program->scatterJitter() );
    m_basicDynamicProgramEditor->scatterAmount->setValue( program->scatterAmount() );
    m_basicDynamicProgramEditor->scatterSensor->setCurrent( program->scatterSensor() );
    setEnableCount(program->isCountEnabled());
    m_basicDynamicProgramEditor->countEnabled->setChecked( program->isCountEnabled() );
    m_basicDynamicProgramEditor->countJitter->setValue( program->countJitter() );
    m_basicDynamicProgramEditor->countCount->setValue( program->countCount() );
    m_basicDynamicProgramEditor->countSensor->setCurrent( program->countSensor() );
}

KisBasicDynamicProgramEditor::~KisBasicDynamicProgramEditor()
{
    delete m_basicDynamicProgramEditor;
}

void KisBasicDynamicProgramEditor::setEnableSize(bool e)
{
    m_basicDynamicProgramEditor->sizeJitter->setEnabled(e);
    m_basicDynamicProgramEditor->labelSizeJitter->setEnabled(e);
    m_basicDynamicProgramEditor->sizeSensor->setEnabled(e);
    m_basicDynamicProgramEditor->labelSizeSensor->setEnabled(e);
    m_basicDynamicProgramEditor->sizeMinimum->setEnabled(e);
    m_basicDynamicProgramEditor->labelSizeMinimum->setEnabled(e);
    m_basicDynamicProgramEditor->sizeMaximum->setEnabled(e);
    m_basicDynamicProgramEditor->labelSizeMaximum->setEnabled(e);
}
void KisBasicDynamicProgramEditor::setEnableAngle(bool e)
{
    m_basicDynamicProgramEditor->angleJitter->setEnabled(e);
    m_basicDynamicProgramEditor->labelAngleJitter->setEnabled(e);
    m_basicDynamicProgramEditor->angleSensor->setEnabled(e);
    m_basicDynamicProgramEditor->labelAngleSensor->setEnabled(e);
}
void KisBasicDynamicProgramEditor::setEnableScatter(bool e)
{
    m_basicDynamicProgramEditor->scatterAmount->setEnabled(e);
    m_basicDynamicProgramEditor->labelScatterAmount->setEnabled(e);
    m_basicDynamicProgramEditor->scatterJitter->setEnabled(e);
    m_basicDynamicProgramEditor->labelScatterJitter->setEnabled(e);
    m_basicDynamicProgramEditor->scatterSensor->setEnabled(e);
    m_basicDynamicProgramEditor->labelScatterSensor->setEnabled(e);
    m_basicDynamicProgramEditor->countEnabled->setEnabled(e);
    if(not e)
    {
        setEnableCount(false);
    } else {
        setEnableCount( m_basicDynamicProgramEditor->countEnabled->isChecked() );
    }
}
void KisBasicDynamicProgramEditor::setEnableCount(bool e)
{
    m_basicDynamicProgramEditor->countCount->setEnabled(e);
    m_basicDynamicProgramEditor->labelCountCount->setEnabled(e);
    m_basicDynamicProgramEditor->countJitter->setEnabled(e);
    m_basicDynamicProgramEditor->labelCountJitter->setEnabled(e);
    m_basicDynamicProgramEditor->countSensor->setEnabled(e);
    m_basicDynamicProgramEditor->labelCountSensor->setEnabled(e);
}

#include "kis_basic_dynamic_program_editor.moc"
