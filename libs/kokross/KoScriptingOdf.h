/***************************************************************************
 * KoScriptingOdf.h
 * This file is part of the KDE project
 * copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KOSCRIPTINGODF_H
#define KOSCRIPTINGODF_H

#include <QObject>
#include <QDomDocument>
#include <QtDebug>
#include <KoStore.h>
#include <KoOasisStore.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoDocument.h>
#include <KoDocumentAdaptor.h>

//#include "TextFrame.h"
//#include "TextTable.h"
//#include "TextCursor.h"

/**
* The KoScriptingOdfReader provides functionality to read content from
* a KoStore.
*/
class KoScriptingOdfReader : public QObject
{
        Q_OBJECT
    public:
        explicit KoScriptingOdfReader(QObject* parent, const KoXmlDocument& doc);
        virtual ~KoScriptingOdfReader();

    public Q_SLOTS:

        /**
        * Start the reading.
        */
        void start();

        /**
        * Return the tag-name of the current element. This could be
        * e.g. "office:text", "style:style" or "text:p".
        */
        QString name() const;

        /**
        * The namespace of the element. For example "office:document-content".
        */
        QString namespaceURI() const;

        /**
        * The level the element is on. Elements may nested and the level is
        * a number that defines how much elements are around this element.
        * The root-element has a level of 0, direct children of the
        * root-element have a level of 1, there children of 2, etc.
        */
        int level() const;

        //QString prefix() const { return m_currentElement.prefix(); }
        //QString localName() const { return m_currentElement.localName(); }

        /**
        * Return a list of attribute-names the element has. This could be
        * for example "text:style-name".
        */
        QStringList attributeNames();

        /**
        * Return the value of the attribute with the name \p name .
        */
        QString attribute(const QString& name, const QString& defaultValue = QString()) const;

        /**
        * Return the value of the attribute with the name \p name that is within
        * the namespace \p namespaceURI .
        */
        QString attributeNS(const QString& namespaceURI, const QString& localName, const QString& defaultValue = QString()) const;

        /**
        * Returns true if the current element has an attribute with name \p name.
        */
        bool hasAttribute(const QString& name) const;

        /**
        * Returns true if the current element has an attribute with name \p name that
        * is within the namespace \p namespaceURI .
        */
        bool hasAttributeNS(const QString& namespaceURI, const QString& localName) const;

        /**
        * Returns true if the current element is invalid.
        */
        bool isNull() const;

        /**
        * Return true if the current element is an element.
        *
        * \deprecated probably refactor this later.
        */
        bool isElement() const;

        /**
        * Return true if the current element is a text-element.
        *
        * \deprecated probably refactor this later.
        */
        bool isText() const;

        //bool isCDATASection() const { return m_currentElement.isCDATASection(); }
        //bool isDocument() const;

        //QString toText() const { return m_currentElement.toText().data(); }
        //KoXmlCDATASection toCDATASection() const;

        /**
        * Return the content of the element as string.
        */
        QString text() const;

    Q_SIGNALS:

        /**
        * This signal got emitted after start() was called for each element we
        * read.
        */
        void onElement();

    private:
        /// \internal d-pointer class.
        class Private;
        /// \internal d-pointer instance.
        Private* const d;
        /// \internal recursive element-handler.
        void handleElem(KoXmlElement& elem, int level = 0);
};

/**
* The KoScriptingOdfStore class provides access to the KoStore functionality.
*
* The following python sample does use KWord do read a ODT document and then
* flushes the DOM tree to stdout.
* \code
* import Kross
* KWord = Kross.module("kword")
*
* # Get the KWord KoApplicationAdaptor instance.
* docAdaptor = KWord.document()
* # Open an ISO OpenDocument Text File.
* docAdaptor.openUrl("/home/kde4/testDoc.odt")
*
* # Get a KoStore instance.
* store = KWord.store()
* # Open the content.xml file within the KoStore.
* reader = store.readFile("content.xml")
* if not reader:
*     raise "Failed to read file from the store"
*
* # This function got called for each element
* def onElement(name, level):
*     print "ELEMENT name=%s level=%s" % (name, level)
* # Connect our callback function with the reader.
* reader.connect("onElement(QString,int)", onElement)
*
* # Start the reading.
* reader.start()
* \endcode
*/
class KoScriptingOdfStore : public QObject
{
        Q_OBJECT
    public:
        explicit KoScriptingOdfStore(QObject* parent, KoDocument* doc);
        virtual ~KoScriptingOdfStore();

    public Q_SLOTS:

        //QStringList files();

        /**
        * Returns true if there exists a file with the defined name
        * \p fileName in the store else false is returned.
        *
        * The following python sample does use the hasFile-method;
        * \code
        * if not store.hasFile("content.xml"):
        *     raise "No content.xml file within the store."
        * \endcode
        */
        bool hasFile(const QString& fileName);

        /**
        * Open the file with the defined name \p fileName and
        * return a \a KoScriptingOdfReader object if the file
        * was open successful else NULL (aka no object) got
        * returned.
        *
        * The following python sample does use the readFile-method;
        * \code
        * reader = store.readFile("content.xml")
        * if not reader:
        *     raise "Failed to read the content.xml from the store."
        * \endcode
        */
        QObject* readFile(const QString& fileName);

        //QStringList directories();
        //bool hasDirectory(const QString& directoryName);
        //QString currentDirectory();
        //bool changeDirectory(const QString& directoryName);

        /**
        * Returns the document the store is the backend for.
        */
        QObject* document() const;

        /**
        * Set the document the store is the backend for. This could
        * be a \a KoDocumentAdaptor or a \a KoDocument object.
        */
        bool setDocument(QObject* document);

        //void setFilter(const QString& filter) {}
        //void start() {}

        //QString toString() const;

    Q_SIGNALS:

        //void onElement();
        //void onHeader();
        //void onParagraph();

    private:
        /// \internal d-pointer class.
        class Private;
        /// \internal d-pointer instance.
        Private* const d;
};



#endif
