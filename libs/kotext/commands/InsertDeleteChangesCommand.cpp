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

#include "InsertDeleteChangesCommand.h"

#include "KoChangeTrackerElement.h"
#include <KoTextDocument.h>
#include <KoCharacterStyle.h>
#include <KoChangeTracker.h>
#include <KoTextDocument.h>

#include <QVector>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextCharFormat>


InsertDeleteChangesCommand::InsertDeleteChangesCommand(QTextDocument *document, KUndo2Command *parent)
    : KUndo2Command("Insert Delete Changes", parent)
    , m_document(document)
{
}

void InsertDeleteChangesCommand::redo()
{
    insertDeleteChanges();
}

static bool isPositionLessThan(KoChangeTrackerElement *element1, KoChangeTrackerElement *element2)
{
    return element1->getDeleteChangeMarker()->position() < element2->getDeleteChangeMarker()->position();
}

void InsertDeleteChangesCommand::insertDeleteChanges()
{
    int numAddedChars = 0;
    QVector<KoChangeTrackerElement *> elementVector;
    KoTextDocument(m_document).changeTracker()->getDeletedChanges(elementVector);
    qSort(elementVector.begin(), elementVector.end(), isPositionLessThan);

    foreach (KoChangeTrackerElement *element, elementVector) {
        if (element->isValid() && element->getDeleteChangeMarker()) {
            QTextCursor caret(element->getDeleteChangeMarker()->document());
            caret.setPosition(element->getDeleteChangeMarker()->position() + numAddedChars +  1);
            QTextCharFormat f = caret.charFormat();
            f.clearProperty(KoCharacterStyle::InlineInstanceId);
            caret.setCharFormat(f);
            KoChangeTracker::insertDeleteFragment(caret, element->getDeleteChangeMarker());
            numAddedChars += KoChangeTracker::fragmentLength(element->getDeleteData());
        }
    }
}
