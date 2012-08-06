/***************************************************************************
 * KoScriptingOdf.h
 * This file is part of the KDE project
 * copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef KOSCRIPTINGODF_H
#define KOSCRIPTINGODF_H

#include <QPair>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoDocument.h>

class KoScriptingOdfStore;
class KoPartAdaptor;

/**
* The KoScriptingOdfReader provides functionality to read content
* from a KoStore.
*
* The following python samples does open the content.xml file and
* reads all text:p elements from it;
* \code
* import Kross, Words
* Words.document().openUrl("/home/kde4/testDoc.odt")
* reader = Words.store().open("META-INF/manifest.xml")
* reader = store.open("content.xml")
* def onElement():
*     if reader.name() != "text:p":
*         raise "This should never happen cause of the filter"
*     print "%s %s" % (reader.level(),reader.attributeNames())
* reader.connect("onElement()", onElement)
* reader.setNameFilter("text:p")
* reader.start()
* \endcode
*/
class KoScriptingOdfReader : public QObject
{
    Q_OBJECT
public:
    /** Constructor. */
    explicit KoScriptingOdfReader(KoScriptingOdfStore *store, const KoXmlDocument &doc);
    /** Destructor. */
    virtual ~KoScriptingOdfReader();
    /** Return the KoScriptingOdfStore instance this reader belongs to. */
    KoScriptingOdfStore *store() const;
    /** Return the KoXmlDocument instance this reader operates on. */
    KoXmlDocument doc() const;
    /** Return the current element. */
    KoXmlElement currentElement() const;

public slots:

    /**
    * Return the element tag-name filter that will be applied on reading.
    */
    QString nameFilter() const;

    /**
    * Set the element tag-name filter that will be applied on reading.
    *
    * This python sample demonstrates usage of the setNameFilter()-method;
    * \code
    * # Only handle text:p elements.
    * reader.setNameFilter("text:p")
    *
    * # Use a regular expression for the tag-name.
    * reader.setNameFilter(".*:p", True)
    * \endcode
    */
    void setNameFilter(const QString &name = QString(), bool regularExpression = false);

    //QString levelFilter() const;
    //void setLevelFilter(const QString &name = QString()) const;

    /**
    * Start the reading.
    *
    * This will fire up the whole reading process. We walk through
    * all elements and emit the onElement() signal or other signals
    * for each element we are interested in.
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

    //QString prefix() const { return m_currentElement.prefix(); }
    //QString localName() const { return m_currentElement.localName(); }

    /**
    * The level the element is on. Elements may nested and the level is
    * a number that defines how much elements are around this element.
    * The root-element has a level of 0, direct children of the
    * root-element have a level of 1, there children of 2, etc.
    */
    int level() const;
#ifndef KOXML_USE_QDOM
    /**
    * Return a list of attribute-names the element has. This could be
    * for example "text:style-name".
    */
    QStringList attributeNames();
#endif

    /**
    * Return the value of the attribute with the name \p name .
    */
    QString attribute(const QString &name, const QString &defaultValue = QString()) const;

    /**
    * Return the value of the attribute with the name \p name that is within
    * the namespace \p namespaceURI .
    */
    QString attributeNS(const QString &namespaceURI, const QString &localName, const QString &defaultValue = QString()) const;

    /**
    * Returns true if the current element has an attribute with name \p name.
    */
    bool hasAttribute(const QString &name) const;

    /**
    * Returns true if the current element has an attribute with name \p name that
    * is within the namespace \p namespaceURI .
    */
    bool hasAttributeNS(const QString &namespaceURI, const QString &localName) const;

    /**
    * Returns true if the current element is invalid.
    */
    bool isNull() const;

    /**
    * Return true if the current element is an element.
    */
    bool isElement() const;

    //bool isText() const;
    //bool isCDATASection() const { return m_currentElement.isCDATASection(); }
    //bool isDocument() const { return m_currentElement.isDocument(); }
    //QString toText() const { return m_currentElement.toText().data(); }
    //KoXmlCDATASection toCDATASection() const;

    /**
    * Return true if the current element has child nodes. Please note, that
    * text-nodes are not counted as children by this method. Fetch any text
    * by using the text() method which will return an empty string if there
    * is actualy no text.
    */
    bool hasChildren() const;

    /**
    * Return the content of the element as string.
    */
    QString text() const;

signals:

    /**
    * This signal got emitted after start() was called for each
    * element we read.
    */
    void onElement();

protected:
    /** Emit the onElement signal above. */
    void emitOnElement();
    /** Set the current element. */
    void setCurrentElement(const KoXmlElement &elem);
    /** Set the level. */
    void setLevel(int level);
    /** Element-handler. */
    virtual void handleElement(KoXmlElement &elem, int level = 0);

private:
    KoScriptingOdfStore *m_store;
    KoXmlDocument m_doc;
    KoXmlElement m_currentElement;
    int m_level;
    QString m_filter;
    QRegExp m_filterRegExp;
};

/**
* The KoScriptingOdfManifestReader class handles reading ODF META-INF/manifest.xml files.
*
* The following python sample script does use the Words scripting module to load a
* ISO OpenDocument Text file and print the content of the manifest-file to stdout;
* \code
* import Kross, Words
* Words.document().openUrl("/home/kde4/testDoc.odt")
* reader = Words.store().open("META-INF/manifest.xml")
* if not reader:
*     raise "Failed to read the mainfest"
* for i in range( reader.count() ):
*     print "%s %s" % (reader.type(i),reader.path(i))
* \endcode
*/
class KoScriptingOdfManifestReader : public KoScriptingOdfReader
{
    Q_OBJECT
public:
    /** Constructor. */
    KoScriptingOdfManifestReader(KoScriptingOdfStore *store, const KoXmlDocument &doc);
    /** Destructor. */
    virtual ~KoScriptingOdfManifestReader() {}

public slots:
    /** Returns the number of file-entries the manifest has. */
    int count() const {
        return m_entries.count();
    }
    /** Returns the type of the file-entry. This could be for example
    something like "text/xml" or "application/vnd.oasis.opendocument.text" */
    QString type(int index) {
        return m_entries.value(index).first;
    }
    /** Return the path of the file-entry. This could be for example
    something like "/", "content.xml" or "styles.xml". */
    QString path(int index) {
        return m_entries.value(index).second;
    }
    /**
    * Return a list of paths for the defined type. If not type is defined
    * just all paths are returned.
    *
    * Python sample that does use the paths-method;
    * \code
    * # Following may print ['/','content.xml','styles.xml']
    * print reader.paths()
    *
    * # Following may print ['content.xml','styles.xml']
    * print reader.paths("text/xml")
    * \endcode
    */
    QStringList paths(const QString &type = QString());
private:
    QList<QPair<QString,QString> > m_entries;
};

/**
* The KoScriptingOdfManifestReader class handles reading ODF styles.xml files.
*/
class KoScriptingOdfStylesReader : public KoScriptingOdfReader
{
    Q_OBJECT
public:
    /** Constructor. */
    KoScriptingOdfStylesReader(KoScriptingOdfStore *store, const KoXmlDocument &doc);
    /** Destructor. */
    virtual ~KoScriptingOdfStylesReader() {}
public slots:
    //QString style(const QString &styleName);
};

/**
* The KoScriptingOdfManifestReader class handles reading ODF content.xml files.
*/
class KoScriptingOdfContentReader : public KoScriptingOdfReader
{
    Q_OBJECT
public:
    /** Constructor. */
    KoScriptingOdfContentReader(KoScriptingOdfStore *store, const KoXmlDocument &doc);
    /** Destructor. */
    virtual ~KoScriptingOdfContentReader() {}
public slots:
    //QStringList headers(const QString &filter);
    //QStringList lists(const QString &filter);
    //QStringList images(const QString &filter);
    //QStringList tables(const QString &filter);
};

/**
* The KoScriptingOdfStore class provides access to the KoStore functionality.
*
* The following python sample does use Words do read a ODT document and then
* flushes the DOM tree to stdout.
* \code
* import Kross
* Words = Kross.module("words")
*
* # Get the Words KoApplicationAdaptor instance.
* docAdaptor = Words.document()
* # Open an ISO OpenDocument Text File.
* docAdaptor.openUrl("/home/kde4/testDoc.odt")
*
* # Get a KoStore instance.
* store = Words.store()
* # Open the content.xml file within the KoStore.
* reader = store.open("content.xml")
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
    /** Constructor. */
    explicit KoScriptingOdfStore(QObject *parent, KoDocument *doc);
    /** Destructor. */
    virtual ~KoScriptingOdfStore();

    //KoStore* readStore() const;
    //QIODevice* readDevice() const;

public slots:

    /**
    * Returns true if there exists a file with the defined name
    * \p fileName in the store else false is returned.
    *
    * The following python sample does use the hasFile-method;
    * \code
    * if not store.hasFile("content.xml"):
    *     raise "No content.xml file within the store."
    * if not store.hasFile("tar:/META-INF/manifest.xml"):
    *     raise "No manifest.xml file within the store."
    * \endcode
    */
    bool hasFile(const QString &fileName);

    /**
    * Returns true if the store is already opened else false
    * got returned. The store does allow to open and deal
    * with maximal one file the same time.
    */
    bool isOpen() const;

    /**
    * Open the file with the defined name \p fileName and
    * return a \a KoScriptingOdfReader object if the file
    * was open successful else NULL (aka no object) got
    * returned.
    *
    * If the store is already opened, it got closed before
    * we try to open the new file. This means, that all
    * previous \a KoScriptingOdfReader instances are
    * invalid and that only the last one returned by using
    * the open-method is valid till closed.
    *
    * The following python sample does use the open-method;
    * \code
    * # Fetch a reader for the styles.xml file.
    * stylesReader = store.open("styles.xml")
    * if not stylesReader:
    *     raise "Failed to read styles"
    *
    * # Now fetch a reader for the manifest. Please note,
    * # that stylesReader.close() is implicit done here.
    * manifestReader = store.open("META-INF/manifest.xml")
    * if not manifestReader:
    *     raise "Failed to read mainfest"
    * \endcode
    */
    QObject* open(const QString &fileName);

    /**
    * Closes the store if it's opened. If the store was not opened
    * or got closed successful true is returned.
    */
    bool close();

    /**
    * Extract the content of the file named \p fileName and return
    * it as an array of bytes.
    */
    QByteArray extract(const QString &fileName);

    /**
    * Extract the content of the file named \p fileName to the defined
    * target-file \p toFileName and return true on success.
    */
    bool extractToFile(const QString &fileName, const QString &toFileName);

    //QStringList directories();
    //bool hasDirectory(const QString &directoryName);
    //QString currentDirectory();
    //bool changeDirectory(const QString &directoryName);

    /**
    * Returns the document the store is the backend for.
    */
    QObject *document() const;

    /**
    * Set the document the store is the backend for. This could
    * be a \a KoPartAdaptor or a \a KoDocument object.
    */
    bool setDocument(QObject *document);

    //void setFilter(const QString &filter) {}
    //void start() {}

signals:

    //void onElement();
    //void onHeader();
    //void onParagraph();

private:
    KoStore *getReadStore();
    QByteArray getByteArray();
    QByteArray getMimeType() const;

    QPointer<KoDocument> m_document;
    QPointer<KoPartAdaptor> m_documentAdaptor;

    KoStore *m_readStore;
    QIODevice *m_readDevice;
    KoScriptingOdfReader *m_reader;
    QByteArray m_byteArray;
};



#endif
