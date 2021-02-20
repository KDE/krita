/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
