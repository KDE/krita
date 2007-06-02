/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#include "kis_filters_list_dynamic_program_editor.h"

#include <kdialog.h>
#include <klocale.h>

#include "ui_FiltersListDynamicProgramEditor.h"

#include "kis_filters_list_dynamic_program.h"
#include "kis_filters_list_model.h"
#include "kis_dynamic_transformation.h"

KisFiltersListDynamicProgramEditor::KisFiltersListDynamicProgramEditor(KisFiltersListDynamicProgram* program) :
        m_program(program), m_currentFilterEditor(0)
{
    m_filtersListDynamicProgramEditor = new Ui_FiltersListDynamicProgramEditor();
    m_filtersListDynamicProgramEditor->setupUi(this);
    // Initialize the model
    m_filtersModel = new KisFiltersListModel( program, m_filtersListDynamicProgramEditor->listViewFilters);
    m_filtersListDynamicProgramEditor->listViewFilters->setModel( m_filtersModel );
    // Connect the respective signals to the actions of the model
    m_filtersModel->connect(m_filtersListDynamicProgramEditor->comboBoxFilter,
                            SIGNAL(activated(int)), SLOT(setCurrentFilterType(int)));
    m_filtersModel->connect(m_filtersListDynamicProgramEditor->pushButtonAdd,
                            SIGNAL(pressed()), SLOT(addNewFilter()));
    m_filtersModel->connect(m_filtersListDynamicProgramEditor->listViewFilters->selectionModel(),
                            SIGNAL(currentChanged ( const QModelIndex & , const QModelIndex & )),
                            SLOT(setCurrentFilter(const QModelIndex&)));
    m_filtersModel->connect(m_filtersListDynamicProgramEditor->pushButtonRemove,
                            SIGNAL(pressed()), SLOT(deleteCurrentFilter()));
    // Connect the respective signals to the actions of the editor
    connect(m_filtersListDynamicProgramEditor->listViewFilters->selectionModel(), 
                            SIGNAL(currentChanged ( const QModelIndex & , const QModelIndex & )),
                            SLOT(setCurrentFilter(const QModelIndex&)));

}
KisFiltersListDynamicProgramEditor::~KisFiltersListDynamicProgramEditor()
{
    delete m_filtersListDynamicProgramEditor;
    delete m_filtersModel;
    if(m_currentFilterEditor) delete m_currentFilterEditor;
}

void KisFiltersListDynamicProgramEditor::setCurrentFilter(const QModelIndex& index)
{
    if(m_currentFilterEditor)
    {
        delete m_currentFilterEditor;
        m_currentFilterEditor = 0;
    }
    m_currentFilterEditor = m_program->transfoAt( index.row() )->createConfigWidget(  m_filtersListDynamicProgramEditor->groupBoxProperties );
    kDebug() << m_currentFilterEditor << endl;
    if(m_currentFilterEditor)
    {
        m_filtersListDynamicProgramEditor->widgetNoProperties->setVisible(false);         m_filtersListDynamicProgramEditor->gridLayout->addWidget( m_currentFilterEditor, 0,0,1,1);
    } else {
        m_filtersListDynamicProgramEditor->widgetNoProperties->setVisible(true);
    }
}

#include "kis_filters_list_dynamic_program_editor.moc"
