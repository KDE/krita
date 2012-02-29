/*
 *  Copyright (c) 2012 C. Boemann <cbo@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ChangeAnchorPropertiesCommand.h"
#include "KoTextAnchor.h"

#include <KoShapeContainer.h>
#include <KoTextShapeDataBase.h>
#include <KoTextDocument.h>
#include <KoInlineTextObjectManager.h>

#include <QTextDocument>

ChangeAnchorPropertiesCommand::ChangeAnchorPropertiesCommand(KoTextAnchor *anchor, const KoTextAnchor &newAnchorData, KoShapeContainer *newParent, KUndo2Command *parent)
    : KUndo2Command("Change Anchor Properties", parent) //Don't translate
    , m_anchor(anchor)
    , m_oldAnchor(0)
    , m_newAnchor(0)
    , m_oldParent(anchor->shape()->parent())
    , m_newParent(newParent)
    , m_first(true)
{
    copyLayoutProperties(anchor, &m_oldAnchor);
    copyLayoutProperties(&newAnchorData, &m_newAnchor);
}

ChangeAnchorPropertiesCommand::~ChangeAnchorPropertiesCommand()
{
}

void ChangeAnchorPropertiesCommand::copyLayoutProperties(const KoTextAnchor *from, KoTextAnchor *to)
{
    to->setOffset(from->offset());
    to->setVerticalPos(from->verticalPos());
    to->setVerticalRel(from->verticalRel());
    to->setHorizontalPos(from->horizontalPos());
    to->setHorizontalRel(from->horizontalRel());
    to->setAnchorType(from->anchorType());
}

void ChangeAnchorPropertiesCommand::redo()
{
    KoTextShapeDataBase *textData = 0;
    if (m_oldParent) {
        textData = qobject_cast<KoTextShapeDataBase*>(m_oldParent->userData());
    } else  if (m_newParent) {
        textData = qobject_cast<KoTextShapeDataBase*>(m_newParent->userData());
    }

    KUndo2Command::redo();

    copyLayoutProperties(&m_newAnchor, m_anchor);

    if (m_first) {
        m_oldAbsPos =  m_anchor->shape()->absolutePosition();
    }
    m_anchor->shape()->setParent(m_newParent);
    m_anchor->shape()->setAbsolutePosition(m_oldAbsPos);

    if (m_newAnchor.anchorType() != m_oldAnchor.anchorType()) {
        if (m_newAnchor.anchorType() == KoTextAnchor::AnchorPage) {
            Q_ASSERT(textData);
            KoTextDocument doc(textData->document());
            KoInlineTextObjectManager *manager = doc.inlineTextObjectManager();
            Q_ASSERT(manager);
            if (m_first) {
                //first time we need to remove the character manually
                QTextCursor cursor(textData->document());
                cursor.setPosition(m_anchor->positionInDocument());
                cursor.deleteChar();
            }
            manager->removeInlineObject(m_anchor);
        }
        else if (m_oldAnchor.anchorType() == KoTextAnchor::AnchorPage) {
            Q_ASSERT(textData);
            KoTextDocument doc(textData->document());
            KoInlineTextObjectManager *manager = doc.inlineTextObjectManager();
            Q_ASSERT(manager);
            if (m_first) {
                KoTextEditor *editor = doc.textEditor();
                QTextCursor cursor(textData->document());
                cursor.setPosition(editor->position());
                manager->insertInlineObject(cursor, m_anchor);
            } else {
                // only insert in manager as qt re-inserts the character
                manager->addInlineObject(m_anchor);
            }
        }
    }

    m_first = false;

    m_anchor->shape()->notifyChanged();
    if (textData) {
        textData->document()->markContentsDirty(m_anchor->positionInDocument(), 0);
    }
}

void ChangeAnchorPropertiesCommand::undo()
{
    KoTextShapeDataBase *textData = 0;
    if (m_oldParent) {
        textData = qobject_cast<KoTextShapeDataBase*>(m_oldParent->userData());
    } else  if (m_newParent) {
        textData = qobject_cast<KoTextShapeDataBase*>(m_newParent->userData());
    }
    copyLayoutProperties(&m_oldAnchor, m_anchor);

    m_anchor->shape()->setParent(m_oldParent);
    m_anchor->shape()->setAbsolutePosition(m_oldAbsPos);

    if (m_newAnchor.anchorType() != m_oldAnchor.anchorType()) {
        if (m_newAnchor.anchorType() == KoTextAnchor::AnchorPage) {
            Q_ASSERT(textData);
            KoInlineTextObjectManager *manager = KoTextDocument(textData->document()).inlineTextObjectManager();
            Q_ASSERT(manager);
            if (manager) {
                manager->addInlineObject(m_anchor);
            }
        }
        else if (m_oldAnchor.anchorType() == KoTextAnchor::AnchorPage) {
            Q_ASSERT(textData);
            KoInlineTextObjectManager *manager = KoTextDocument(textData->document()).inlineTextObjectManager();
            Q_ASSERT(manager);
            if (manager) {
                manager->removeInlineObject(m_anchor);
            }
        }
    }

    KUndo2Command::undo();
    m_anchor->shape()->notifyChanged();
    if (textData) {
        textData->document()->markContentsDirty(m_anchor->positionInDocument(), 0);
    }
}
