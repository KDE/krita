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

#include <kdebug.h>

#include "kis_dynamic_programs_editor.h"

#include <QVBoxLayout>

#include <kis_bookmarked_configuration_manager.h>
#include <kis_bookmarked_configurations_model.h>

#include "ui_DynamicProgramsEditor.h"

#include "kis_dynamic_program.h"
#include "kis_dynamic_program_factory_registry.h"

KisDynamicProgramsEditor::KisDynamicProgramsEditor(QWidget* parent, KisBookmarkedConfigurationManager* bookmarksManager) : KDialog(parent), m_dynamicProgramsEditor(0), m_currentEditor(0), m_frameVBoxLayout(0),m_bookmarksManager(bookmarksManager),
    m_bookmarksModel(new KisBookmarkedConfigurationsModel(bookmarksManager))
{
    setCaption(i18n("Edit dynamic programs"));
    setButtons(KDialog::Close);
    QWidget* widget = new QWidget(this);
    m_dynamicProgramsEditor = new Ui_DynamicProgramsEditor;
    m_dynamicProgramsEditor->setupUi(widget);
    setMainWidget(widget);
    m_frameVBoxLayout = new QVBoxLayout(m_dynamicProgramsEditor->frame);
    m_frameVBoxLayout->setMargin(0);
    connect(m_dynamicProgramsEditor->comboBoxPrograms, SIGNAL(currentIndexChanged( const QString &) ), this, SLOT(setCurrentProgram(const QString&)));
    connect(m_dynamicProgramsEditor->pushButtonAdd, SIGNAL(pressed()), SLOT(addProgram()));
    m_dynamicProgramsEditor->comboBoxPrograms->setModel(m_bookmarksModel);
    m_dynamicProgramsEditor->comboBoxProgramsType->setIDList( KisDynamicProgramFactoryRegistry::instance()->listKeys() );
}

KisDynamicProgramsEditor::~KisDynamicProgramsEditor()
{
    delete m_dynamicProgramsEditor;
    if(m_currentEditor) delete m_currentEditor;
}

void KisDynamicProgramsEditor::setCurrentProgram(const QString& text)
{
    kDebug(41006) <<"program changed to" << text;
    delete m_currentEditor;
    m_currentProgram = static_cast<KisDynamicProgram*>( m_bookmarksManager->load( text ) );
    Q_ASSERT(m_currentProgram);
    m_currentEditor = m_currentProgram->createEditor( m_dynamicProgramsEditor->frame);
    m_frameVBoxLayout->addWidget(m_currentEditor);
    connect(m_currentProgram, SIGNAL(programChanged()), SLOT(saveCurrentProgram()));
}

void KisDynamicProgramsEditor::addProgram()
{
    int index = m_dynamicProgramsEditor->comboBoxProgramsType->currentIndex();
    QString id = m_dynamicProgramsEditor->comboBoxProgramsType->currentItem().id();
    KisDynamicProgramFactory* factory = KisDynamicProgramFactoryRegistry::instance()->value( id );
    Q_ASSERT(factory);
    KisDynamicProgram* program = factory->program( m_bookmarksManager->uniqueName( ki18n("New program %1") ) );
    Q_ASSERT(program);
    m_bookmarksManager->save(program->name(), program);
}

void KisDynamicProgramsEditor::saveCurrentProgram()
{
    kDebug(41006) <<"saveCurrentProgram " << m_currentProgram->name();
    m_bookmarksManager->save(m_currentProgram->name(), m_currentProgram);
}

#include "kis_dynamic_programs_editor.moc"
