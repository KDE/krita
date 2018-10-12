/* This file is part of the KDE project
 * Copyright (C) 2006, 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2012-2014 C. Boemann <cbo@boemann.dk>
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

#include "OdfTextTrackStyles.h"
#include "KoTextDocument.h"
#include "KoParagraphStyle.h"
#include "KoCharacterStyle.h"

#include <QTextDocument>
#include "TextDebug.h"


QMap<QObject *, OdfTextTrackStyles *> OdfTextTrackStyles::instances;

OdfTextTrackStyles *OdfTextTrackStyles::instance(KoStyleManager *manager)
{
    if (! instances.contains(manager)) {
        instances[manager] = new OdfTextTrackStyles(manager);
        connect(manager,SIGNAL(destroyed(QObject*)),instances[manager], SLOT(styleManagerDied(QObject*)));
    }

    return instances[manager];
}

void OdfTextTrackStyles::registerDocument(QTextDocument *qDoc)
{
    if (! m_documents.contains(qDoc)) {
        m_documents.append(qDoc);
        connect(qDoc,SIGNAL(destroyed(QObject*)), this, SLOT(documentDied(QObject*)));
    }
}

void OdfTextTrackStyles::unregisterDocument(QTextDocument *qDoc)
{
    if (m_documents.contains(qDoc)) {
        m_documents.removeOne(qDoc);
    }
}

OdfTextTrackStyles::OdfTextTrackStyles(KoStyleManager *manager)
        : QObject(manager)
        , m_styleManager(manager)
        , m_changeCommand(0)
{
    connect(manager, SIGNAL(editHasBegun()), this, SLOT(beginEdit()));
    connect(manager, SIGNAL(editHasEnded()), this, SLOT(endEdit()));
    connect(manager, SIGNAL(styleHasChanged(int,const KoCharacterStyle*,const KoCharacterStyle*)), this, SLOT(recordStyleChange(int,const KoCharacterStyle*,const KoCharacterStyle*)));
    connect(manager, SIGNAL(styleHasChanged(int,const KoParagraphStyle*,const KoParagraphStyle*)), this, SLOT(recordStyleChange(int,const KoParagraphStyle*,const KoParagraphStyle*)));
}

OdfTextTrackStyles::~OdfTextTrackStyles()
{
    delete m_changeCommand;
}

void OdfTextTrackStyles::beginEdit()
{
    Q_ASSERT(m_changeCommand == 0);
    m_changeCommand = new ChangeStylesMacroCommand(m_documents, m_styleManager.data());
}

void OdfTextTrackStyles::endEdit()
{
    if (m_documents.length() > 0) {
        KUndo2Stack *undoStack= KoTextDocument(m_documents.first()).undoStack();
        if (undoStack) {
            undoStack->push(m_changeCommand);
        }
    } else
        delete m_changeCommand;

    m_changeCommand = 0;
}

void OdfTextTrackStyles::recordStyleChange(int id, const KoParagraphStyle *origStyle, const KoParagraphStyle *newStyle)
{
    m_changeCommand->changedStyle(id);

    if (origStyle != newStyle) {
        m_changeCommand->origStyle(origStyle->clone());
        m_changeCommand->changedStyle(newStyle->clone());
    }
}

void OdfTextTrackStyles::recordStyleChange(int id, const KoCharacterStyle *origStyle, const KoCharacterStyle *newStyle)
{
   m_changeCommand->changedStyle(id);

    if (origStyle != newStyle) {
        m_changeCommand->origStyle(origStyle->clone());
        m_changeCommand->changedStyle(newStyle->clone());
    }
}

void OdfTextTrackStyles::styleManagerDied(QObject *manager)
{
    OdfTextTrackStyles::instances.remove(manager);
}

void OdfTextTrackStyles::documentDied(QObject *document)
{
    unregisterDocument(static_cast<QTextDocument *>(document));
}
