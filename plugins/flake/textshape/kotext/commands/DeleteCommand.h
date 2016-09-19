/*
 This file is part of the KDE project
 * Copyright (C) 2009 Ganesh Paramasivam <ganesh@crystalfab.com>
 * Copyright (C) 2012 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2014-2015 Denis Kuplyakov <dener.kup@gmail.com>
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

#include "KoTextCommandBase.h"

#include <QTextCharFormat>
#include <QHash>
#include <QSet>
#include <QWeakPointer>

class QTextDocument;

class KoShapeController;
class KoInlineObject;
class KoTextRange;
class KoSection;

class DeleteVisitor;

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

    struct SectionDeleteInfo {
        SectionDeleteInfo(KoSection *_section, int _childIdx)
            : section(_section)
            , childIdx(_childIdx)
        {
        }

        bool operator<(const SectionDeleteInfo &other) const;

        KoSection *section; ///< Section to remove
        int childIdx; ///< Position of section in parent's children() list
    };

    QWeakPointer<QTextDocument> m_document;
    KoShapeController *m_shapeController;

    QSet<KoInlineObject *> m_invalidInlineObjects;
    QList<QTextCursor> m_cursorsToWholeDeleteBlocks;
    QHash<int, KoTextRange *> m_rangesToRemove;
    QList<SectionDeleteInfo> m_sectionsToRemove;

    bool m_first;
    DeleteMode m_mode;
    int m_position;
    int m_length;
    QTextCharFormat m_format;
    bool m_mergePossible;

    void doDelete();
    void deleteInlineObject(KoInlineObject *object);
    bool checkMerge(const KUndo2Command *command);
    void updateListChanges();
    void finalizeSectionHandling(QTextCursor *caret, DeleteVisitor &visitor);
    void deleteSectionsFromModel();
    void insertSectionsToModel();
};

#endif // DELETECOMMAND_H
