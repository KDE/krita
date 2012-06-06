/*
 *  Copyright (c) 2011 C. Boemann <cbo@boemann.dk>
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

#include "RemoveDeleteChangesCommand.h"

#include <QTextDocument>
#include <QVector>
#include <QTextCursor>
#include <QTextCharFormat>

#include <KoChangeTracker.h>
#include <KoChangeTrackerElement.h>
#include <KoTextDocument.h>

RemoveDeleteChangesCommand::RemoveDeleteChangesCommand(QTextDocument *document,
                                                       KUndo2Command *parent)
    : KUndo2Command("Insert Delete Changes", parent)
    , m_document(document)
{
}

void RemoveDeleteChangesCommand::redo()
{
    removeDeleteChanges();
}

static bool isPositionLessThan(KoChangeTrackerElement *element1, KoChangeTrackerElement *element2)
{
    return element1->getDeleteChangeMarker()->position() < element2->getDeleteChangeMarker()->position();
}


void RemoveDeleteChangesCommand::removeDeleteChanges()
{
    int numDeletedChars = 0;
    QVector<KoChangeTrackerElement *> elementVector;
    KoTextDocument(m_document).changeTracker()->getDeletedChanges(elementVector);
    qSort(elementVector.begin(), elementVector.end(), isPositionLessThan);

    foreach(KoChangeTrackerElement *element, elementVector) {
        if (element->isValid() && element->getDeleteChangeMarker()) {
            QTextCursor caret(element->getDeleteChangeMarker()->document());
            QTextCharFormat f;
            int deletePosition = element->getDeleteChangeMarker()->position() + 1 - numDeletedChars;
            caret.setPosition(deletePosition);
            int deletedLength = KoChangeTracker::fragmentLength(element->getDeleteData());
            caret.setPosition(deletePosition + deletedLength, QTextCursor::KeepAnchor);
            caret.removeSelectedText();
            numDeletedChars += KoChangeTracker::fragmentLength(element->getDeleteData());
        }
    }
}
