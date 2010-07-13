/*
 *  Copyright (c) 2010 Mani Chandrasekar <maninc@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "googledocumentlist.h"
#include "googledocument.h"

#include <stdio.h>
#include <QtCore>

GoogleDocumentList::GoogleDocumentList()
{
    docModel = new QStandardItemModel(0, 2);
}

void GoogleDocumentList::setEtag(const QString  &etag)
{
    m_etag = etag;
}

QString GoogleDocumentList::etag ()
{
    return m_etag;
}

void GoogleDocumentList::setEmail (const QString &email)
{
    m_email = email;
}

QString GoogleDocumentList::email ()
{
    return m_email;
}

void GoogleDocumentList::setTitle (const QString &title)
{
    m_title = title;
}

QString GoogleDocumentList::title ()
{
    return m_title;
}

void GoogleDocumentList::setAuthor (const QString &author)
{
    m_author = author;
}

QString GoogleDocumentList::author ()
{
    return m_author;
}

void GoogleDocumentList::append(GoogleDocument *entry)
{
    if (entry != 0 ) {
        m_entries.append(entry);

        //count = docModel->rowCount();
        docModel->insertRow(0);
        docModel->setData(docModel->index(0, 0), entry->title());
        docModel->setData(docModel->index(0, 1), entry->documentUrl());
       /*qDebug() << "Resource ID : " << entry->id();
        qDebug() << "Title : " << entry->title();
        qDebug() << "Author : " << entry->author();
        qDebug() << "Url : " << entry->documentUrl();*/
    }
}

QList<GoogleDocument *> GoogleDocumentList::entries()
{
    return m_entries;
}

int GoogleDocumentList::documentsCount()
{
    return m_entries.count();
}
