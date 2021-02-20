/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_legacy_undo_adapter.h"

#include "kis_image.h"


KisLegacyUndoAdapter::KisLegacyUndoAdapter(KisUndoStore *undoStore,
                                           KisImageWSP image)
    : KisUndoAdapter(undoStore, image.data()),
      m_image(image),
      m_macroCounter(0)
{
}

const KUndo2Command* KisLegacyUndoAdapter::presentCommand()
{
    return undoStore()->presentCommand();
}

void KisLegacyUndoAdapter::undoLastCommand()
{
    undoStore()->undoLastCommand();
}

void KisLegacyUndoAdapter::addCommand(KUndo2Command *command)
{
    if(!command) return;

    if(m_macroCounter) {
        undoStore()->addCommand(command);
    }
    else {
        m_image->barrierLock();
        undoStore()->addCommand(command);
        m_image->unlock();
    }
}

void KisLegacyUndoAdapter::beginMacro(const KUndo2MagicString& macroName)
{
    if(!m_macroCounter) {
        m_image->barrierLock();
    }

    m_macroCounter++;
    undoStore()->beginMacro(macroName);
}

void KisLegacyUndoAdapter::endMacro()
{
    m_macroCounter--;

    if(!m_macroCounter) {
        m_image->unlock();
    }
    undoStore()->endMacro();
}

