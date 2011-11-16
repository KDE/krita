/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef INSERTTEXTANCHORCOMMAND_H
#define INSERTTEXTANCHORCOMMAND_H

#include <kundo2command.h>

class KoTextAnchor;
class QTextDocument;

class InsertInlineObjectCommand : public KUndo2Command
{
public:
    InsertInlineObjectCommand(KoTextAnchor *anchor, QTextDocument *document, KUndo2Command *parent);
    virtual ~InsertInlineObjectCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:

    KoTextAnchor *m_anchor;
    QTextDocument *m_document;
    bool m_deleteAnchor;
};

#endif // INSERTTEXTANCHORCOMMAND_H
