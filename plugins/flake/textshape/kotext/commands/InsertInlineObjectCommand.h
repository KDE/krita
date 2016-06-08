/*
 *  Copyright (c) 2011 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#ifndef INSERTINLINEOBJECTCOMMAND_H
#define INSERTINLINEOBJECTCOMMAND_H

#include <kundo2command.h>

class KoInlineObject;
class QTextDocument;

class InsertInlineObjectCommand : public KUndo2Command
{
public:
    InsertInlineObjectCommand(KoInlineObject *inlineObject, QTextDocument *document, KUndo2Command *parent);
    virtual ~InsertInlineObjectCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();
private:

    KoInlineObject *m_inlineObject;
    QTextDocument *m_document;
    bool m_deleteInlineObject;
    bool m_first;
    int m_position;
};

#endif // INSERTINLINEOBJECTCOMMAND_H
