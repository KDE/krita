/* This file is part of the KDE project
 * Copyright ( C ) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOPAPAGEDELETECOMMAND_H
#define KOPAPAGEDELETECOMMAND_H

#include <QUndoCommand>
#include <QMap>

#include "kopageapp_export.h"

class KoPADocument;
class KoPAPageBase;

/**
 * Command for deleting a page from a document
 */
class KOPAGEAPP_TEST_EXPORT KoPAPageDeleteCommand : public QUndoCommand
{
public:
    /**
     * Command to delete a single page.
     * @param document the document to delete the page from
     * @param page the page to delete
     * @param parent the parent command used for macro commands
     */
    KoPAPageDeleteCommand( KoPADocument *document, KoPAPageBase *page, QUndoCommand *parent = 0 );

    /**
     * Command to delete multiple pages.
     * @param document the document to delete the pages from
     * @param pages a list of pages to delete
     * @param parent the parent command used for macro commands
     */
    KoPAPageDeleteCommand(KoPADocument *document, const QList<KoPAPageBase*> &pages, QUndoCommand *parent = 0);

    virtual ~KoPAPageDeleteCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    KoPADocument * m_document; //< the document to delete the pages from
    QMap<int, KoPAPageBase*> m_pages; //< a map of pages to delete, with document index as key
    bool m_deletePages; //< shows if pages should be deleted when deleting the command
};

#endif // KOPAPAGEDELETECOMMAND_H
