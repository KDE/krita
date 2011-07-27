/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_surrogate_undo_adapter.h"
#include "kis_undo_stores.h"


KisSurrogateUndoAdapter::KisSurrogateUndoAdapter()
    : KisUndoAdapter(new KisSurrogateUndoStore)
{
    m_undoStore = static_cast<KisSurrogateUndoStore*>(undoStore());
}

KisSurrogateUndoAdapter::~KisSurrogateUndoAdapter()
{
    delete m_undoStore;
}

const KUndo2Command* KisSurrogateUndoAdapter::presentCommand()
{
    return m_undoStore->presentCommand();
}

void KisSurrogateUndoAdapter::undoLastCommand()
{
    m_undoStore->undoLastCommand();
}

void KisSurrogateUndoAdapter::addCommand(KUndo2Command *command)
{
    m_undoStore->addCommand(command);
}

void KisSurrogateUndoAdapter::beginMacro(const QString& macroName)
{
    m_undoStore->beginMacro(macroName);
}

void KisSurrogateUndoAdapter::endMacro()
{
    m_undoStore->endMacro();
}

void KisSurrogateUndoAdapter::undo()
{
    m_undoStore->undo();
}

void KisSurrogateUndoAdapter::redo()
{
    m_undoStore->redo();
}

void KisSurrogateUndoAdapter::undoAll()
{
    m_undoStore->undoAll();
}

void KisSurrogateUndoAdapter::redoAll()
{
    m_undoStore->redoAll();
}

