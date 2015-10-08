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

#include "DeleteAnnotationsCommand.h"

#include "KoAnnotation.h"
#include "KoTextDocument.h"
#include "KoTextRangeManager.h"

#include "TextDebug.h"

DeleteAnnotationsCommand::DeleteAnnotationsCommand(const QList<KoAnnotation *> &annotations, QTextDocument *document, KUndo2Command *parent)
    : KUndo2Command(parent)
    , m_annotations(annotations)
    , m_document(document)
    , m_deleteAnnotations(false)
{
}

DeleteAnnotationsCommand::~DeleteAnnotationsCommand()
{
    if (m_deleteAnnotations) {
        qDeleteAll(m_annotations);
    }
}

void DeleteAnnotationsCommand::redo()
{
    KUndo2Command::redo();
    m_deleteAnnotations = true;
    KoTextRangeManager *rangeManager = KoTextDocument(m_document).textRangeManager();
    if (rangeManager) {
        foreach (KoAnnotation *annotation, m_annotations) {
            rangeManager->remove(annotation);
        }
    }
}

void DeleteAnnotationsCommand::undo()
{
    KUndo2Command::undo();
    KoTextRangeManager *rangeManager = KoTextDocument(m_document).textRangeManager();
    if (rangeManager) {
        foreach (KoAnnotation *annotation, m_annotations) {
            rangeManager->insert(annotation);
            //it's a textrange so we need to ask for a layout so we know where it is
            m_document->markContentsDirty(annotation->rangeStart(), 0);
       }
    }

    m_deleteAnnotations = false;
}
