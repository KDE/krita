/*
 This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
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
 * Boston, MA 02110-1301, USA.*/

#ifndef CHANGETRACKEDDELETECOMMAND_H
#define CHANGETRACKEDDELETECOMMAND_H

#include "KoTextCommandBase.h"
#include <KoListStyle.h>
#include <QList>
#include <QWeakPointer>

class QTextDocument;
class QTextCursor;
class KoShapeController;
class KoDocumentRdfBase;
class KoTextEditor;

class ChangeTrackedDeleteCommand : public KoTextCommandBase
{
public:
    enum DeleteMode {
        PreviousChar,
        NextChar
    };

    ChangeTrackedDeleteCommand(DeleteMode mode,
                               QTextDocument *document,
                               KoShapeController *shapeController,
                               KUndo2Command* parent = 0);
    virtual ~ChangeTrackedDeleteCommand();

    virtual void undo();
    virtual void redo();

    virtual int id() const;
    virtual bool mergeWith ( const KUndo2Command *command);

private:
    QWeakPointer<QTextDocument> m_document;
    KoDocumentRdfBase *m_rdf;
    KoShapeController *m_shapeController;
    bool m_first;
    bool m_undone;
    bool m_canMerge;
    DeleteMode m_mode;
    QList<int> m_removedElements;
    QList<KoListStyle::ListIdType> m_newListIds;
    int m_position, m_length;
    int m_addedChangeElement;

    virtual void deleteChar();
    virtual void deletePreviousChar();
    virtual void deleteSelection(KoTextEditor *editor);
    virtual void removeChangeElement(int changeId);
    virtual void updateListIds(QTextCursor &cursor);
    virtual void updateListChanges();
    virtual void handleListItemDelete(KoTextEditor *editor);
};

#endif // CHANGETRACKEDDELETECOMMAND_H
