/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPAPageDeleteCommand.h"

#include <klocale.h>

#include "KoPADocument.h"
#include "KoPAPageBase.h"

KoPAPageDeleteCommand::KoPAPageDeleteCommand( KoPADocument *document, KoPAPageBase *page, QUndoCommand *parent )
: QUndoCommand( parent )
, m_document( document )
, m_deletePages(false)
{
    Q_ASSERT(m_document);
    Q_ASSERT(page);
    int index = m_document->pageIndex(page);
    Q_ASSERT(index != -1);
    m_pages.insert(index, page);

    if ( page->pageType() == KoPageApp::Slide ) {
        setText( i18n( "Delete slide" ) );
    }
    else {
        setText( i18n( "Delete page" ) );
    }
}

KoPAPageDeleteCommand::KoPAPageDeleteCommand(KoPADocument *document, const QList<KoPAPageBase*> &pages, QUndoCommand *parent)
: QUndoCommand(parent)
, m_document(document)
, m_deletePages(false)
{
    Q_ASSERT(m_document);
    Q_ASSERT(m_document->pages().count() > pages.count());
    int index = -1;

    foreach (KoPAPageBase *page, pages) {
        Q_ASSERT(page);
        index = m_document->pageIndex(page);
        Q_ASSERT(index != -1);
        m_pages.insert(index, page);
    }

    if (pages.first()->pageType() == KoPageApp::Slide) {
        setText(i18np("Delete slide", "Delete slides", m_pages.count()));
    }
    else {
        setText(i18np("Delete page", "Delete pages", m_pages.count()));
    }
}

KoPAPageDeleteCommand::~KoPAPageDeleteCommand()
{
    if (!m_deletePages)
        return;

    qDeleteAll(m_pages);
}

void KoPAPageDeleteCommand::redo()
{
    QUndoCommand::redo();
    int index = -1;

    foreach (KoPAPageBase *page, m_pages) {
        index = m_document->takePage(page);
        Q_ASSERT(index != -1);
    }

    m_deletePages = true;
}

void KoPAPageDeleteCommand::undo()
{
    QUndoCommand::undo();
    QMapIterator<int, KoPAPageBase*> i(m_pages);

    while (i.hasNext()) {
        i.next();
        m_document->insertPage(i.value(), i.key());
    }

    m_deletePages = false;
}
