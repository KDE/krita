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

#include "InsertDeleteChangesCommand.h"

#include "KoChangeTrackerElement.h"
#include <KoTextDocument.h>
#include <KoChangeTracker.h>

#include <QVector>
#include <QTextDocument>


InsertDeleteChangesCommand::InsertDeleteChangesCommand(QTextDocument *document, KUndo2Command *parent)
    : KUndo2Command("Insert Delete Changes", parent)
    , m_document(document)
{
}

void InsertDeleteChangesCommand::redo()
{
    insertDeleteChanges();
}


void InsertDeleteChangesCommand::insertDeleteChanges()
{
    int numAddedChars = 0;
    QVector<KoChangeTrackerElement *> elementVector;
    KoTextDocument(m_document).changeTracker()->getDeletedChanges(elementVector);
    qSort(elementVector.begin(), elementVector.end());


}
