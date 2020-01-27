/*
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
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

#ifndef KIS_UNDO_STORE_H_
#define KIS_UNDO_STORE_H_

#include <QString>
#include <QVector>

#include <kritacommand_export.h>

class KUndo2Command;
class KUndo2MagicString;


/**
 * See also: https://community.kde.org/Krita/Undo_adapter_vs_Undo_store
 *
 * Split the functionality of KisUndoAdapter into two classes:
 * KisUndoStore and KisUndoAdapter. The former one works as an
 * interface to an external storage of the undo information:
 * undo stack, KisDocument, /dev/null. The latter one defines the
 * behavior of the system when someone wants to add a command. There
 * are three variants:
 *    1) KisSurrogateUndoAdapter -- saves commands directly to the
 *       internal stack. Used for wrapping around legacy code into
 *       a single command.
 *    2) KisLegacyUndoAdapter -- blocks the strokes and updates queue,
 *       and then adds the command to a store
 *    3) KisPostExecutionUndoAdapter -- used by the strokes. It doesn't
 *       call redo() when you add a command. It is assumed, that you have
 *       already executed the command yourself and now just notify
 *       the system about it. Warning: it doesn't inherit KisUndoAdapter
 *       because it doesn't fit the contract of this class. And, more
 *       important, KisTransaction should work differently with this class.
 *
 * The ownership on the KisUndoStore (that substituted KisUndoAdapter
 * in the document's code) now belongs to the image. It means that
 * KisDocument::createUndoStore() is just a factory method, the document
 * doesn't store the undo store itself.
 */
class KRITACOMMAND_EXPORT KisUndoStore
{
public:
    KisUndoStore();
    virtual ~KisUndoStore();

public:
    /**
     * WARNING: All these methods are not considered as thread-safe
     */

    virtual const KUndo2Command* presentCommand() = 0;
    virtual void undoLastCommand() = 0;
    virtual void addCommand(KUndo2Command *cmd) = 0;
    virtual void beginMacro(const KUndo2MagicString& macroName) = 0;
    virtual void endMacro() = 0;
    virtual void purgeRedoState() = 0;

private:
    Q_DISABLE_COPY(KisUndoStore)
};


#endif // KIS_UNDO_STORE_H_

