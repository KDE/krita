/* This file is part of the KDE project
 * Copyright (C) 2011 Thorsten Zachmann <zachmann@kde.org>
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
 * Boston, MA 02110-1301, USA.
 */

#include "DeleteAnchorsCommand.h"

#include <QTextCursor>
#include "KoTextAnchor.h"
#include "KoTextDocument.h"
#include "KoInlineTextObjectManager.h"

#include <KDebug>

bool sortAnchor(KoTextAnchor *a1, KoTextAnchor *a2)
{
    return a1->positionInDocument() > a2->positionInDocument();
}

DeleteAnchorsCommand::DeleteAnchorsCommand(const QList<KoTextAnchor*> &anchors, QTextDocument *document, KUndo2Command *parent)
: KUndo2Command(parent)
, m_document(document)
, m_first(true)
, m_deleteAnchors(false)
{
    foreach (KoTextAnchor *anchor, anchors) {
        if (anchor->document() == document) {
            m_anchors.append(anchor);
        }
    }
    qSort(m_anchors.begin(), m_anchors.end(), sortAnchor);
}

DeleteAnchorsCommand::~DeleteAnchorsCommand()
{
    if (m_deleteAnchors) {
        qDeleteAll(m_anchors);
    }
}

void DeleteAnchorsCommand::redo()
{
    KUndo2Command::redo();
    m_deleteAnchors = true;
    if (m_first) {
        m_first = false;
        foreach (KoTextAnchor *anchor, m_anchors) {
            QTextCursor cursor(m_document);
            cursor.setPosition(anchor->positionInDocument());
//            cursor.beginEditBlock();
            cursor.deleteChar(); //this works also when the DeleteCommand further deletes its selection, which contained this char. Odd
//            cursor.endEditBlock();
        }
    }
    KoInlineTextObjectManager *manager = KoTextDocument(m_document).inlineTextObjectManager();
    Q_ASSERT(manager);
    if (manager) {
        foreach (KoTextAnchor *anchor, m_anchors) {
            manager->removeInlineObject(anchor);
        }
    }
}

void DeleteAnchorsCommand::undo()
{
    KoInlineTextObjectManager *manager = KoTextDocument(m_document).inlineTextObjectManager();
    Q_ASSERT(manager);
    if (manager) {
        foreach (KoTextAnchor *anchor, m_anchors) {
            manager->addInlineObject(anchor);
        }
    }
    KUndo2Command::undo();
    m_deleteAnchors = false;
}
