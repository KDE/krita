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

#ifndef __KIS_DOCUMENT_UNDO_STORE_H
#define __KIS_DOCUMENT_UNDO_STORE_H

#include "kis_undo_store.h"
#include "kritaui_export.h"

class KisDocument;

class KRITAUI_EXPORT KisDocumentUndoStore : public KisUndoStore
{
public:
    KisDocumentUndoStore(KisDocument *doc);

    const KUndo2Command* presentCommand() override;
    void undoLastCommand() override;
    void addCommand(KUndo2Command *cmd) override;
    void beginMacro(const KUndo2MagicString& macroName) override;
    void endMacro() override;
    void purgeRedoState() override;

private:
    KisDocument* m_doc;
};

#endif /* __KIS_DOCUMENT_UNDO_STORES_H */
