/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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

        m_documentConnections.addConnection(stack, SIGNAL(canUndoChanged(bool)), m_undoAction, SLOT(setEnabled(bool)));
        m_documentConnections.addConnection(stack, SIGNAL(canRedoChanged(bool)), m_redoAction, SLOT(setEnabled(bool)));

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

