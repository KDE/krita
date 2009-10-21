/* This file is part of the KDE project
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_actions_editor.h"

#include "ui_wdgactionseditor.h"

#include "kis_macro_model.h"
#include <recorder/kis_recorded_action.h>
#include <recorder/kis_macro.h>
#include <recorder/kis_recorded_action_editor_factory_registry.h>

KisActionsEditor::KisActionsEditor(QWidget* parent) : QWidget(parent), m_currentEditor(0), m_form(new Ui::ActionsEditor), m_macro(0), m_model(0), m_widgetLayout(0)

{
    m_form->setupUi(this);

    // Setup buttons
    m_form->bnAdd->setIcon(SmallIcon("list-add"));

    m_form->bnDelete->setIcon(SmallIcon("list-remove"));
    connect(m_form->bnDelete, SIGNAL(released()), SLOT(slotBtnDelete()));

    m_form->bnRaise->setIcon(SmallIcon("go-up"));
    connect(m_form->bnRaise, SIGNAL(released()), SLOT(slotBtnRaise()));

    m_form->bnLower->setIcon(SmallIcon("go-down"));
    connect(m_form->bnLower, SIGNAL(released()), SLOT(slotBtnLower()));

    m_form->bnDuplicate->setIcon(SmallIcon("edit-copy"));
    connect(m_form->bnDuplicate, SIGNAL(released()), SLOT(slotBtnDuplicate()));

    // Setup actions list
    connect(m_form->actionsList, SIGNAL(clicked(const QModelIndex&)), SLOT(slotActionActivated(const QModelIndex&)));
    connect(m_form->actionsList, SIGNAL(activated(QModelIndex)), SLOT(slotActionActivated(const QModelIndex&)));

    // Editor
    m_widgetLayout = new QGridLayout(m_form->editorWidget);
    setCurrentAction(0);
}

KisActionsEditor::~KisActionsEditor()
{
    delete m_form;
}

void KisActionsEditor::setMacro(KisMacro* _macro)
{
    m_macro = _macro;
    KisMacroModel* oldModel = m_model;
    m_model = new KisMacroModel(m_macro);
    m_form->actionsList->setModel(m_model);
    delete oldModel;
}

void KisActionsEditor::slotActionActivated(const QModelIndex& item)
{
    if (item.isValid() && m_macro) {
        setCurrentAction(m_macro->actions()[item.row()]);
    } else {
        setCurrentAction(0);
    }
}

void KisActionsEditor::setCurrentAction(KisRecordedAction* _action)
{
    // First change, the editor
    delete m_currentEditor;
    m_currentEditor = 0;
    if (_action) {
        m_currentEditor = KisRecordedActionEditorFactoryRegistry::instance()->createEditor(this, _action);
    } else {
        m_currentEditor = new QLabel(i18n("No action is selected."), this);
    }
    if (!m_currentEditor) {
        m_currentEditor = new QLabel(i18n("No editor for current action."), this);
    }
    m_widgetLayout->addWidget(m_currentEditor, 0 , 0);

    // Then disable/enalbed button
    m_form->bnDuplicate->setEnabled(_action);
    m_form->bnRaise->setEnabled(_action);
    m_form->bnLower->setEnabled(_action);
    m_form->bnDelete->setEnabled(_action);
    if (_action) {
        int pos = m_macro->actions().indexOf(_action);
        if (pos == 0) {
            m_form->bnRaise->setEnabled(false);
        }
        if (pos == m_macro->actions().count() - 1) {
            m_form->bnLower->setEnabled(false);
        }
    }
}

void KisActionsEditor::slotBtnDelete()
{
    QModelIndex idx = m_form->actionsList->currentIndex();
    Q_ASSERT(idx.isValid());
    m_model->removeRows(idx.row(), 1);
    setCurrentAction(0);
}

void KisActionsEditor::slotBtnDuplicate()
{
    QModelIndex idx = m_form->actionsList->currentIndex();
    Q_ASSERT(idx.isValid());
    m_model->duplicateAction(idx);
}

void KisActionsEditor::slotBtnRaise()
{
    QModelIndex idx = m_form->actionsList->currentIndex();
    Q_ASSERT(idx.isValid());
    m_model->raise(idx);
    m_form->actionsList->setCurrentIndex(m_model->index(idx.row() - 1));
    slotActionActivated(m_form->actionsList->currentIndex());
}

void KisActionsEditor::slotBtnLower()
{
    QModelIndex idx = m_form->actionsList->currentIndex();
    Q_ASSERT(idx.isValid());
    m_model->lower(idx);
    m_form->actionsList->setCurrentIndex(m_model->index(idx.row() + 1));
    slotActionActivated(m_form->actionsList->currentIndex());
}

#include "kis_actions_editor.moc"
