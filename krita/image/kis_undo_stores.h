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

#ifndef __KIS_UNDO_STORES_H
#define __KIS_UNDO_STORES_H

#include "kis_undo_store.h"

class KoDocument;
class KUndo2Stack;

class KRITAIMAGE_EXPORT KisDocumentUndoStore : public KisUndoStore
{
public:
    KisDocumentUndoStore(KoDocument *doc);

    /**
     * NOTE: This is a kind of hack to allow KisDoc2
     * to notify undo store when a command is executed
     * during undo/redo actions
     */
    using KisUndoStore::notifyCommandExecuted;

    const KUndo2Command* presentCommand();
    void undoLastCommand();
    void addCommand(KUndo2Command *cmd);
    void beginMacro(const QString& macroName);
    void endMacro();

private:
    KoDocument* m_doc;
};


/**
 * KisSurrogateUndoAdapter -- saves commands directly to the
 * internal stack. Used for wrapping around legacy code into
 * a single command.
 */
class KRITAIMAGE_EXPORT KisSurrogateUndoStore : public KisUndoStore
{
public:
    KisSurrogateUndoStore();
    ~KisSurrogateUndoStore();

    const KUndo2Command* presentCommand();
    void undoLastCommand();
    void addCommand(KUndo2Command *cmd);
    void beginMacro(const QString& macroName);
    void endMacro();

    void undo();
    void redo();

    void undoAll();
    void redoAll();

private:
    KUndo2Stack *m_undoStack;
};

/**
 * @brief The KisDumbUndoStore class doesn't actually save commands,
 * so you cannot undo or redo!
 */
class KRITAIMAGE_EXPORT KisDumbUndoStore : public KisUndoStore
{
public:
    const KUndo2Command* presentCommand();
    void undoLastCommand();
    void addCommand(KUndo2Command *cmd);
    void beginMacro(const QString& macroName);
    void endMacro();
};

#endif /* __KIS_UNDO_STORES_H */
