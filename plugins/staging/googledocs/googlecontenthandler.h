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

#ifndef GOOGLECONTENTHANDLER_H
#define GOOGLECONTENTHANDLER_H

#include <QXmlContentHandler>
#include <QStack>

class GoogleDocumentList;
class GoogleDocument;

class GoogleContentHandler : public QXmlContentHandler
{
public:
    GoogleContentHandler();
    virtual ~GoogleContentHandler();

    virtual bool characters ( const QString & ch );
    virtual bool endDocument ();
    virtual bool endElement ( const QString & namespaceURI, const QString & localName, const QString & qName );
    virtual bool endPrefixMapping ( const QString & prefix );
    virtual QString errorString () const;
    virtual bool ignorableWhitespace ( const QString & ch );
    virtual bool processingInstruction ( const QString & target, const QString & data );
    virtual void setDocumentLocator ( QXmlLocator * locator );
    virtual bool skippedEntity ( const QString & name );
    virtual bool startDocument ();
    virtual bool startElement ( const QString & namespaceURI, const QString & localName, const QString & qName, const QXmlAttributes & atts );
    virtual bool startPrefixMapping ( const QString & prefix, const QString & uri );

    GoogleDocumentList *documentList() {    return m_docList; }

private:
    QStack<QString> m_nodeStack;
    bool insideEntry;
    GoogleDocumentList *m_docList;
    GoogleDocument *m_docEntry;

    void printName(const QString & name);
};

#endif // GOOGLECONTENTHANDLER_H
