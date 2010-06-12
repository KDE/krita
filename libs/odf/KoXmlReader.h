/* This file is part of the KDE project
   Copyright (C) 2005-2006 Ariya Hidayat <ariya@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KOFFICE_XMLREADER
#define KOFFICE_XMLREADER

// KOXML_USE_QDOM is defined there
#include "KoXmlReaderForward.h"

#include "koodf_export.h"

#include <QtXml/qxml.h>
#include <QtXml/qdom.h>

class QIODevice;
class QTextDecoder;

#ifdef KOXML_USE_QDOM

typedef QDomNode KoXmlNode;
typedef QDomElement KoXmlElement;
typedef QDomText KoXmlText;
typedef QDomCDATASection KoXmlCDATASection;
typedef QDomDocumentType KoXmlDocumentType;
typedef QDomDocument KoXmlDocument;

#else

class QString;
class QXmlReader;
class QXmlInputSource;

class KoXmlNode;
class KoXmlText;
class KoXmlCDATASection;
class KoXmlDocumentType;
class KoXmlDocument;
class KoXmlNodeData;

/**
* KoXmlNode represents a node in a DOM tree.
*
* KoXmlNode is a base class for KoXmlElement, KoXmlText.
* Often, these subclasses are used for getting the data instead of KoXmlNode.
* However, as base class, KoXmlNode is very helpful when for example iterating
* all child nodes within one parent node.
*
* KoXmlNode implements an explicit sharing, a node shares its data with
* other copies (if exist).
*
* XXX: DO NOT ADD CONVENIENCE API HERE BECAUSE THIS CLASS MUST REMAIN COMPATIBLE WITH QDOMNODE!
*
* @author Ariya Hidayat <ariya@kde.org>
*/
class KOODF_EXPORT KoXmlNode
{
public:

    enum NodeType {
        NullNode = 0,
        ElementNode,
        TextNode,
        CDATASectionNode,
        ProcessingInstructionNode,
        DocumentNode,
        DocumentTypeNode
    };

    KoXmlNode();
    KoXmlNode(const KoXmlNode& node);
    KoXmlNode& operator=(const KoXmlNode& node);
    bool operator== (const KoXmlNode&) const;
    bool operator!= (const KoXmlNode&) const;
    virtual ~KoXmlNode();

    virtual KoXmlNode::NodeType nodeType() const;
    virtual bool isNull() const;
    virtual bool isElement() const;
    virtual bool isText() const;
    virtual bool isCDATASection() const;
    virtual bool isDocument() const;
    virtual bool isDocumentType() const;

    virtual void clear();
    KoXmlElement toElement() const;
    KoXmlText toText() const;
    KoXmlCDATASection toCDATASection() const;
    KoXmlDocument toDocument() const;

    virtual QString nodeName() const;
    virtual QString namespaceURI() const;
    virtual QString prefix() const;
    virtual QString localName() const;

    KoXmlDocument ownerDocument() const;
    KoXmlNode parentNode() const;

    bool hasChildNodes() const;
    KoXmlNode firstChild() const;
    KoXmlNode lastChild() const;
    KoXmlNode nextSibling() const;
    KoXmlNode previousSibling() const;

    // equivalent to node.childNodes().count() if node is a QDomNode instance
    int childNodesCount() const;

    // workaround to get and iterate over all attributes
    QStringList attributeNames() const;

    KoXmlNode namedItem(const QString& name) const;
    KoXmlNode namedItemNS(const QString& nsURI, const QString& name) const;

    /**
    * Loads all child nodes (if any) of this node. Normally you do not need
    * to call this function as the child nodes will be automatically
    * loaded when necessary.
    */
    void load(int depth = 1);

    /**
    * Releases all child nodes of this node.
    */
    void unload();

    // compatibility
    QDomNode asQDomNode(QDomDocument ownerDoc) const;

protected:
    KoXmlNodeData* d;
    KoXmlNode(KoXmlNodeData*);
};

/**
* KoXmlElement represents a tag element in a DOM tree.
*
* KoXmlElement holds information about an XML tag, along with its attributes.
*
* @author Ariya Hidayat <ariya@kde.org>
*/

class KOODF_EXPORT KoXmlElement: public KoXmlNode
{
public:
    KoXmlElement();
    KoXmlElement(const KoXmlElement& element);
    KoXmlElement& operator=(const KoXmlElement& element);
    virtual ~KoXmlElement();
    bool operator== (const KoXmlElement&) const;
    bool operator!= (const KoXmlElement&) const;

    QString tagName() const;
    QString text() const;

    QString attribute(const QString& name) const;
    QString attribute(const QString& name, const QString& defaultValue) const;
    QString attributeNS(const QString& namespaceURI, const QString& localName,
                        const QString& defaultValue = QString()) const;
    bool hasAttribute(const QString& name) const;
    bool hasAttributeNS(const QString& namespaceURI, const QString& localName) const;

private:
    friend class KoXmlNode;
    friend class KoXmlDocument;
    KoXmlElement(KoXmlNodeData*);
};

/**
* KoXmlText represents a text in a DOM tree.
* @author Ariya Hidayat <ariya@kde.org>
*/
class KOODF_EXPORT KoXmlText: public KoXmlNode
{
public:
    KoXmlText();
    KoXmlText(const KoXmlText& text);
    KoXmlText& operator=(const KoXmlText& text);
    virtual ~KoXmlText();

    QString data() const;
    void setData(const QString& data);
    virtual bool isText() const;

private:
    friend class KoXmlNode;
    friend class KoXmlCDATASection;
    friend class KoXmlDocument;
    KoXmlText(KoXmlNodeData*);
};

/**
* KoXmlCDATASection represents a CDATA section in a DOM tree.
* @author Ariya Hidayat <ariya@kde.org>
*/
class KOODF_EXPORT KoXmlCDATASection: public KoXmlText
{
public:
    KoXmlCDATASection();
    KoXmlCDATASection(const KoXmlCDATASection& cdata);
    KoXmlCDATASection& operator=(const KoXmlCDATASection& cdata);
    virtual ~KoXmlCDATASection();

    virtual bool isCDATASection() const;

private:
    friend class KoXmlNode;
    friend class KoXmlDocument;
    KoXmlCDATASection(KoXmlNodeData*);
};

/**
* KoXmlDocumentType represents the DTD of the document. At the moment,
* it can used only to get the document type, i.e. no support for
* entities etc.
*
* @author Ariya Hidayat <ariya@kde.org>
*/

class KOODF_EXPORT KoXmlDocumentType: public KoXmlNode
{
public:
    KoXmlDocumentType();
    KoXmlDocumentType(const KoXmlDocumentType&);
    KoXmlDocumentType& operator=(const KoXmlDocumentType&);
    virtual ~KoXmlDocumentType();

    QString name() const;

private:
    friend class KoXmlNode;
    friend class KoXmlDocument;
    KoXmlDocumentType(KoXmlNodeData*);
};


/**
* KoXmlDocument represents an XML document, structured in a DOM tree.
*
* KoXmlDocument is designed to be memory efficient. Unlike QDomDocument from
* Qt's XML module, KoXmlDocument does not store all nodes in the DOM tree.
* Some nodes will be loaded and parsed on-demand only.
*
* KoXmlDocument is read-only, you can not modify its content.
*
* @author Ariya Hidayat <ariya@kde.org>
*/

class KOODF_EXPORT KoXmlDocument: public KoXmlNode
{
public:
    KoXmlDocument();
    KoXmlDocument(const KoXmlDocument& node);
    KoXmlDocument& operator=(const KoXmlDocument& node);
    bool operator==(const KoXmlDocument&) const;
    bool operator!=(const KoXmlDocument&) const;
    virtual ~KoXmlDocument();

    KoXmlElement documentElement() const;

    KoXmlDocumentType doctype() const;

    virtual QString nodeName() const;
    virtual void clear();

    bool setContent(QIODevice* device, bool namespaceProcessing,
                    QString* errorMsg = 0, int* errorLine = 0, int* errorColumn = 0);
    bool setContent(QIODevice* device,
                    QString* errorMsg = 0, int* errorLine = 0, int* errorColumn = 0);
    bool setContent(QXmlInputSource *source, QXmlReader *reader,
                    QString* errorMsg = 0, int* errorLine = 0, int* errorColumn = 0);
    bool setContent(const QByteArray& text, bool namespaceProcessing,
                    QString *errorMsg = 0, int *errorLine = 0, int *errorColumn = 0);
    bool setContent(const QString& text, bool namespaceProcessing,
                    QString *errorMsg = 0, int *errorLine = 0, int *errorColumn = 0);

    // no namespace processing
    bool setContent(const QString& text,
                    QString *errorMsg = 0, int *errorLine = 0, int *errorColumn = 0);

private:
    friend class KoXmlNode;
    KoXmlDocumentType dt;
    KoXmlDocument(KoXmlNodeData*);
};

#endif // KOXML_USE_QDOM

class KOODF_EXPORT KoXmlInputSource: public QXmlInputSource
{
public:
    explicit KoXmlInputSource(QIODevice *dev);
    ~KoXmlInputSource();

    virtual void setData(const QString& dat);
    virtual void setData(const QByteArray& dat);
    virtual void fetchData();
    virtual QString data() const;
    virtual QChar next();
    virtual void reset();

protected:
    virtual QString fromRawData(const QByteArray &data, bool beginning = false);

private:
    QIODevice* device;
    QTextDecoder* decoder;
    QString stringData;
    int stringLength;
    int stringIndex;
    char* buffer;
};

/**
 * This namespace contains a few convenience functions to simplify code using QDom
 * (when loading OASIS documents, in particular).
 *
 * To find the child element with a given name, use KoXml::namedItemNS.
 *
 * To find all child elements with a given name, use
 * QDomElement e;
 * forEachElement( e, parent )
 * {
 *     if ( e.localName() == "..." && e.namespaceURI() == KoXmlNS::... )
 *     {
 *         ...
 *     }
 * }
 * Note that this means you don't ever need to use QDomNode nor toElement anymore!
 * Also note that localName is the part without the prefix, this is the whole point
 * of namespace-aware methods.
 *
 * To find the attribute with a given name, use QDomElement::attributeNS.
 *
 * Do not use getElementsByTagNameNS, it's recursive (which is never needed in KOffice).
 * Do not use tagName() or nodeName() or prefix(), since the prefix isn't fixed.
 *
 * @author David Faure <faure@kde.org>
 */
namespace KoXml
{

/**
 * A namespace-aware version of QDomNode::namedItem(),
 * which also takes care of casting to a QDomElement.
 * Use this when a domelement is known to have only *one* child element
 * with a given tagname.
 *
 * Note: do *NOT* use getElementsByTagNameNS, it's recursive!
 */
KOODF_EXPORT KoXmlElement namedItemNS(const KoXmlNode& node,
                                        const QString& nsURI, const QString& localName);

/**
 * Explicitly load child nodes of specified node, up to given depth.
 * This function has no effect if QDom is used.
 */
KOODF_EXPORT void load(KoXmlNode& node, int depth = 1);

/**
 * Unload child nodes of specified node.
 * This function has no effect if QDom is used.
 */
KOODF_EXPORT void unload(KoXmlNode& node);

/**
 * Get the number of child nodes of specified node.
 */
KOODF_EXPORT int childNodesCount(const KoXmlNode& node);

/**
 * Return the name of all attributes of specified node.
 */
KOODF_EXPORT QStringList attributeNames(const KoXmlNode& node);

/**
 * Convert KoXml classes to the corresponding QDom classes, which has
 * 'ownerDoc' as the owner document (QDomDocument instance).
 */
KOODF_EXPORT QDomNode asQDomNode(QDomDocument ownerDoc, const KoXmlNode& node);
KOODF_EXPORT QDomElement asQDomElement(QDomDocument ownerDoc, const KoXmlElement& element);
KOODF_EXPORT QDomDocument asQDomDocument(QDomDocument ownerDoc, const KoXmlDocument& document);

/*
 * Load an XML document from specified device to a document. You can of
 * course use it with QFile (which inherits QIODevice).
 * This is much more memory efficient than standard QDomDocument::setContent
 * because the data from the device is buffered, unlike
 * QDomDocument::setContent which just loads everything in memory.
 *
 * Note: it is assumed that the XML uses UTF-8 encoding.
 */
KOODF_EXPORT bool setDocument(KoXmlDocument& doc, QIODevice* device,
                                bool namespaceProcessing, QString* errorMsg = 0,
                                int* errorLine = 0, int* errorColumn = 0);

KOODF_EXPORT bool setDocument(KoXmlDocument& doc, QIODevice* device,
                                QXmlSimpleReader* reader, QString* errorMsg = 0,
                                int* errorLine = 0, int* errorColumn = 0);
}

/**
 * \def forEachElement( elem, parent )
 * \brief Loop through all child elements of \parent.
 * This convenience macro is used to implement the forEachElement loop.
 * The \elem parameter is a name of a QDomElement variable and the \parent
 * is the name of the parent element. For example:
 *
 * QDomElement e;
 * forEachElement( e, parent )
 * {
 *     kDebug() << e.localName() << " element found.";
 *     ...
 * }
 */
#define forEachElement( elem, parent ) \
    for ( KoXmlNode _node = parent.firstChild(); !_node.isNull(); _node = _node.nextSibling() ) \
        if ( ( elem = _node.toElement() ).isNull() ) {} else


#endif // KOFFICE_XMLREADER
