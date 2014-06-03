/*
 This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
 * Copyright (C) 2012 C. Boemann <cbo@boemann.dk>
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

#ifndef DELETECOMMAND_H
#define DELETECOMMAND_H

#include <kundo2qstack.h>

#include "KoTextCommandBase.h"

#include <QTextCharFormat>
#include <QHash>
#include <QSet>
#include <QWeakPointer>

class QTextDocument;
class KoShapeController;
class KoInlineObject;
class KoShape;

class QTextCursor;

class DeleteVisitor;
class KoTextRange;
class KoTextEditor;

class DeleteCommand : public KoTextCommandBase
{
public:
    enum DeleteMode {
        PreviousChar,
        NextChar
    };

    DeleteCommand(DeleteMode mode, QTextDocument *document, KoShapeController *shapeController, KUndo2Command* parent = 0);
    virtual ~DeleteCommand();

    virtual void undo();
    virtual void redo();

    virtual int id() const;
    virtual bool mergeWith(const KUndo2Command *command);

private:
    friend class DeleteVisitor;

    QWeakPointer<QTextDocument> m_document;
    KoShapeController *m_shapeController;

    QSet<KoInlineObject *> m_invalidInlineObjects;
    QHash<int, KoTextRange *> m_rangesToRemove;
    bool m_first;
    bool m_undone;
    DeleteMode m_mode;
    int m_position;
    int m_length;
    QTextCharFormat m_format;
    bool m_mergePossible;

    void doDelete();
    void deleteAnchorInlineObject(KoInlineObject *object);
    bool checkMerge(const KUndo2Command *command);
    void updateListChanges();
};

#endif // DELETECOMMAND_H
