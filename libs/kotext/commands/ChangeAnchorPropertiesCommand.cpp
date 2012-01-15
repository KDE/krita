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

#include <KoShape.h>

#include <QTextDocument>

ChangeAnchorPropertiesCommand::ChangeAnchorPropertiesCommand(KoTextAnchor *anchor, KoTextAnchor *newAnchor, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_anchor(anchor)
    , m_oldAnchor(0)
    , m_newAnchor(0)
{
    copyLayoutProperties(anchor, &m_oldAnchor);
    copyLayoutProperties(newAnchor, &m_newAnchor);
}

ChangeAnchorPropertiesCommand::~ChangeAnchorPropertiesCommand()
{
}

void ChangeAnchorPropertiesCommand::copyLayoutProperties(KoTextAnchor *from, KoTextAnchor *to)
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

    if (m_anchor->anchorType() == KoTextAnchor::AnchorPage) {
        m_anchor->shape()->setParent(0);
    }
    m_anchor->shape()->notifyChanged();
    const_cast<QTextDocument *>(m_anchor->document())->markContentsDirty(m_anchor->positionInDocument(), 0);
}

void ChangeAnchorPropertiesCommand::undo()
{
    KUndo2Command::undo();
    copyLayoutProperties(&m_oldAnchor, m_anchor);

    if (m_anchor->anchorType() == KoTextAnchor::AnchorPage) {
        m_anchor->shape()->setParent(0);
    }
    m_anchor->shape()->notifyChanged();
    const_cast<QTextDocument *>(m_anchor->document())->markContentsDirty(m_anchor->positionInDocument(), 0);
}
