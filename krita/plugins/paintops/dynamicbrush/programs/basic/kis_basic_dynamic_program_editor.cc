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
    connect(m_basicDynamicProgramEditor->sizeEnabled, SIGNAL(clicked(bool)), SLOT(setEnableSize(bool)));
    connect(m_basicDynamicProgramEditor->angleEnabled, SIGNAL(clicked(bool)), SLOT(setEnableAngle(bool)));
    connect(m_basicDynamicProgramEditor->scatterEnabled, SIGNAL(clicked(bool)), SLOT(setEnableScatter(bool)));
    connect(m_basicDynamicProgramEditor->countEnabled, SIGNAL(clicked(bool)), SLOT(setEnableCount(bool)));
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
