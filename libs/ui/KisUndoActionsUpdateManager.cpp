/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisUndoActionsUpdateManager.h"

#include <QAction>
#include <kundo2stack.h>

#include <KisDocument.h>


KisUndoActionsUpdateManager::KisUndoActionsUpdateManager(QAction *undoAction, QAction *redoAction, QObject *parent)
    : QObject(parent),
      m_undoAction(undoAction),
      m_redoAction(redoAction)
{

}

void KisUndoActionsUpdateManager::setCurrentDocument(KisDocument *document)
{
    m_documentConnections.clear();

    if (document) {

        KUndo2Stack *stack = document->undoStack();

        m_documentConnections.addConnection(stack, SIGNAL(undoTextChanged(QString)), this, SLOT(slotUndoTextChanged(QString)));
        m_documentConnections.addConnection(stack, SIGNAL(redoTextChanged(QString)), this, SLOT(slotRedoTextChanged(QString)));

        slotUndoTextChanged(stack->undoText());
        slotRedoTextChanged(stack->redoText());

        m_undoAction->setEnabled(stack->canUndo());
        m_redoAction->setEnabled(stack->canRedo());
    }
}

void KisUndoActionsUpdateManager::slotUndoTextChanged(const QString &text)
{
    m_undoAction->setText(i18n("Undo %1", text));
}

void KisUndoActionsUpdateManager::slotRedoTextChanged(const QString &text)
{
    m_redoAction->setText(i18n("Redo %1", text));
}

