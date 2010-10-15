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

#include "googledocument.h"

GoogleDocument::GoogleDocument()
{
}

void GoogleDocument::setEtag(const QString & etag)
{
    m_etag = etag;
}

QString GoogleDocument::etag () const
{
    return m_etag;
}

void GoogleDocument::setId (const QString & id)
{
    m_id = id;

    setDocumentType(id.left(id.indexOf(":")));
}

QString GoogleDocument::id () const
{
    return m_id;
}

void GoogleDocument::setTitle (const QString & title)
{
    m_title = title;
}

QString GoogleDocument::title () const
{
    return m_title;
}

void GoogleDocument::setAuthor (const QString & author)
{
    m_author = author;
}

QString GoogleDocument::author () const
{
    return m_author;
}

void GoogleDocument::setDocumentType (const QString & docType)
{
    m_docType = docType;
}

QString GoogleDocument::documentType () const
{
    return m_docType;
}

void GoogleDocument::setDocumentUrl (const QString & url)
{
    m_documetUrl = url;
}

QString GoogleDocument::documentUrl () const
{
    return m_documetUrl;
}

