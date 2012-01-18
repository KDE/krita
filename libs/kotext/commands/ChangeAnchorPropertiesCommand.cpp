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
    : KUndo2Command(parent)
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
    KUndo2Command::redo();

    copyLayoutProperties(&m_newAnchor, m_anchor);

    QPointF absPos =  m_anchor->shape()->absolutePosition();
    m_anchor->shape()->setParent(m_newParent);
    m_anchor->shape()->setAbsolutePosition(absPos);

    KoTextShapeDataBase *oldTextData = 0;
    KoTextShapeDataBase *newTextData = 0;
    if (m_newAnchor.anchorType() != m_oldAnchor.anchorType()) {
        if (m_newAnchor.anchorType() == KoTextAnchor::AnchorPage) {
            oldTextData = qobject_cast<KoTextShapeDataBase*>(m_oldParent->userData());
            Q_ASSERT(oldTextData);
            KoInlineTextObjectManager *manager = KoTextDocument(oldTextData->document()).inlineTextObjectManager();
            Q_ASSERT(manager);
            if (manager) {
                manager->removeInlineObject(m_anchor);
            }
        }
        else if (m_oldAnchor.anchorType() == KoTextAnchor::AnchorPage) {
            newTextData = qobject_cast<KoTextShapeDataBase*>(m_newParent->userData());
            Q_ASSERT(newTextData);
            KoInlineTextObjectManager *manager = KoTextDocument(newTextData->document()).inlineTextObjectManager();
            Q_ASSERT(manager);
            if (manager) {
                manager->addInlineObject(m_anchor);
            }
        }
    }
    if (m_first) {
        m_first = false;

        if (oldTextData) {
            // remove the old anchor from the document
            KoTextEditor *editor = KoTextDocument(oldTextData->document()).textEditor();
            editor->addCommand(this, false);
            QTextCursor cursor(oldTextData->document());
            cursor.setPosition(m_anchor->positionInDocument());
            cursor.deleteChar();
        }
        else if (newTextData) {
            KoTextEditor *editor = KoTextDocument(newTextData->document()).textEditor();
            editor->addCommand(this, false);

            QTextCursor cursor(newTextData->document());
            cursor.setPosition(editor->position());
            QTextCharFormat oldCf = cursor.charFormat();
            // create a new format out of the old so that the current formatting is
            // also used for the inserted object.  KoVariables render text too ;)
            QTextCharFormat cf(oldCf);
            cf.setObjectType(QTextFormat::UserObject + 1);
            cf.setProperty(KoInlineTextObjectManager::InlineInstanceId, m_anchor->id());
            cursor.insertText(QString(QChar::ObjectReplacementCharacter), cf);
            // reset to use old format so that the InlineInstanceId is no longer set.
            cursor.setCharFormat(oldCf);
        }
    }

    m_anchor->shape()->notifyChanged();
    if (oldTextData) {
        oldTextData->document()->markContentsDirty(m_anchor->positionInDocument(), 0);
    }
    else if (newTextData) {
        newTextData->document()->markContentsDirty(m_anchor->positionInDocument(), 0);
    }
}

void ChangeAnchorPropertiesCommand::undo()
{
    copyLayoutProperties(&m_oldAnchor, m_anchor);

    QPointF absPos =  m_anchor->shape()->absolutePosition();
    m_anchor->shape()->setParent(m_oldParent);
    m_anchor->shape()->setAbsolutePosition(absPos);

    KoTextShapeDataBase *oldTextData = 0;
    KoTextShapeDataBase *newTextData = 0;
    if (m_newAnchor.anchorType() != m_oldAnchor.anchorType()) {
        if (m_newAnchor.anchorType() == KoTextAnchor::AnchorPage) {
            oldTextData = qobject_cast<KoTextShapeDataBase*>(m_oldParent->userData());
            Q_ASSERT(oldTextData);
            KoInlineTextObjectManager *manager = KoTextDocument(oldTextData->document()).inlineTextObjectManager();
            Q_ASSERT(manager);
            if (manager) {
                manager->addInlineObject(m_anchor);
            }
        }
        else if (m_oldAnchor.anchorType() == KoTextAnchor::AnchorPage) {
            newTextData = qobject_cast<KoTextShapeDataBase*>(m_newParent->userData());
            Q_ASSERT(newTextData);
            KoInlineTextObjectManager *manager = KoTextDocument(newTextData->document()).inlineTextObjectManager();
            Q_ASSERT(manager);
            if (manager) {
                manager->removeInlineObject(m_anchor);
            }
        }
    }

    KUndo2Command::undo();
    m_anchor->shape()->notifyChanged();
    if (oldTextData) {
        oldTextData->document()->markContentsDirty(m_anchor->positionInDocument(), 0);
    }
    else if (newTextData) {
        newTextData->document()->markContentsDirty(m_anchor->positionInDocument(), 0);
    }
}
