/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_dynamic_programs_editor.h"

#include <QVBoxLayout>

#include "ui_DynamicProgramsEditor.h"

#include "kis_dynamic_program.h"
#include "kis_dynamic_program_registry.h"

KisDynamicProgramsEditor::KisDynamicProgramsEditor(QWidget* parent) : KDialog(parent), m_dynamicProgramsEditor(0), m_currentEditor(0), m_frameVBoxLayout(0)
{
    setButtons(KDialog::Close);
    QWidget* widget = new QWidget(this);
    m_dynamicProgramsEditor = new Ui_DynamicProgramsEditor;
    m_dynamicProgramsEditor->setupUi(widget);
    m_frameVBoxLayout = new QVBoxLayout(m_dynamicProgramsEditor->frame);
    m_frameVBoxLayout->setMargin(0);
    connect(m_dynamicProgramsEditor->comboBoxPrograms, SIGNAL(currentIndexChanged( const QString &) ), this, SLOT(setCurrentProgram(const QString&)));
    m_dynamicProgramsEditor->comboBoxPrograms->addItems( KisDynamicProgramRegistry::instance()->keys() );
    setMainWidget( widget );
}

KisDynamicProgramsEditor::~KisDynamicProgramsEditor()
{
    delete m_dynamicProgramsEditor;
    if(m_currentEditor) delete m_currentEditor;
}

void KisDynamicProgramsEditor::setCurrentProgram(const QString& text)
{
    kDebug() << "program changed to " << text << endl;
    if(m_currentEditor) delete m_currentEditor;
    KisDynamicProgram* program = KisDynamicProgramRegistry::instance()->get( text );
    Q_ASSERT(program);
    m_currentEditor = program->createEditor( m_dynamicProgramsEditor->frame);
    m_frameVBoxLayout->addWidget(m_currentEditor);
}

#include "kis_dynamic_programs_editor.moc"
