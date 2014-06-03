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
#include "googlecontenthandler.h"
#include <QDebug>

GoogleContentHandler::GoogleContentHandler()
        : m_docList(0),
          m_docEntry(0)
{
    m_docList = new GoogleDocumentList();
}

GoogleContentHandler::~GoogleContentHandler()
{
    delete m_docList;
}

bool GoogleContentHandler::characters ( const QString & ch )
{
    if (!insideEntry) {
        if (QString::compare(m_nodeStack.top(), "title", Qt::CaseInsensitive) == 0) {
            if (m_nodeStack.count() == 2)
                m_docList->setTitle(ch);
        }
        else if (QString::compare(m_nodeStack.top(), "name", Qt::CaseInsensitive) == 0) {
            if (m_nodeStack.count() == 3)
                m_docList->setAuthor(ch);
        }
        else if (QString::compare(m_nodeStack.top(), "email", Qt::CaseInsensitive) == 0) {
            if (m_nodeStack.count() == 3)
                m_docList->setEmail(ch);
        }
    }
    else
    {
        if (m_docEntry == 0)
            return true;

        if (QString::compare(m_nodeStack.top(), "title", Qt::CaseInsensitive) == 0) {
                m_docEntry->setTitle(ch);
        }
        else if (QString::compare(m_nodeStack.top(), "name", Qt::CaseInsensitive) == 0) {
                m_docEntry->setAuthor(ch);
        }
        else if (QString::compare(m_nodeStack.top(), "resourceId", Qt::CaseInsensitive) == 0) {
            m_docEntry->setId(ch);
        }
    }
    return true;
}

bool GoogleContentHandler::endDocument ()
{
//    qDebug() << "GoogleContentHandler::endDocument()";
    return true;
}

bool GoogleContentHandler::endElement ( const QString & /*namespaceURI*/, const QString & /*localName*/, const QString & /*qName */)
{
//    printName(localName);
    QString element = m_nodeStack.pop();
    if (QString::compare(element, "entry") == 0) {
        insideEntry = false;
        m_docList->append(m_docEntry);
        m_docEntry = 0;
    }
    return true;
}

bool GoogleContentHandler::endPrefixMapping ( const QString & /*prefix */)
{
    return true;
}

QString GoogleContentHandler::errorString () const
{
    return QString();
}

bool GoogleContentHandler::ignorableWhitespace ( const QString & /*ch */)
{
    return true;
}

bool GoogleContentHandler::processingInstruction ( const QString & /*target*/, const QString & /*data */)
{
    return true;
}

void GoogleContentHandler::setDocumentLocator ( QXmlLocator * /*locator*/ )
{
}

bool GoogleContentHandler::skippedEntity ( const QString & /*name*/ )
{
    return true;
}

bool GoogleContentHandler::startDocument ()
{
    return true;
}

bool GoogleContentHandler::startElement ( const QString & /*namespaceURI*/, const QString & localName,
                                          const QString & /*qName*/, const QXmlAttributes & atts )
{
    m_nodeStack.push(localName);

    if ((m_nodeStack.count() == 1) && (m_docList != 0)) { //Feed element
        m_docList->setEtag(atts.value("gd:etag"));
    }

    if (QString::compare(localName, "entry", Qt::CaseInsensitive) == 0 ) {
        m_docEntry = new GoogleDocument();
        m_docEntry->setEtag(atts.value("gd:etag"));
        insideEntry = true;
    }
    if ( insideEntry && (m_docEntry != 0)) {
        if (QString::compare(localName, "content", Qt::CaseInsensitive) == 0 ) {
            m_docEntry->setDocumentUrl(atts.value("src"));
        } else if ((QString::compare(localName, "category", Qt::CaseInsensitive) == 0 ) &&
                   QString::compare(atts.value("scheme"), "http://schemas.google.com/g/2005#kind", Qt::CaseInsensitive) == 0){
            m_docEntry->setDocumentType(atts.value("label"));
        }
    }

//    printName(localName);
    return true;
}

bool GoogleContentHandler::startPrefixMapping ( const QString & /*prefix*/, const QString & /*uri */)
{
    //qDebug() << "GoogleContentHandler::startPrefixMapping() " << prefix << uri;
    return true;
}

void GoogleContentHandler::printName(const QString & name)
{
    int count = m_nodeStack.count();
    QString indent;
    for (int i=0; i < count; i++)
        indent.append("\t");
    indent.append(name);

    if (insideEntry)
        qDebug() << indent;
}
