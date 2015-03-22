/*
 *  Copyright (c) 2011 C. Boemann <cbo@boemann.dk>
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

#ifndef INSERTDELETECHANGESCOMMAND_H
#define INSERTDELETECHANGESCOMMAND_H

#include <kundo2command.h>
#include <QWeakPointer>

class QTextDocument;

class InsertDeleteChangesCommand : public KUndo2Command
{
public:
    explicit InsertDeleteChangesCommand(QTextDocument *document, KUndo2Command *parent = 0);
    void redo();

private:
    QWeakPointer<QTextDocument> m_document;
    void insertDeleteChanges();
};


#endif // INSERTDELETECHANGESCOMMAND_H
