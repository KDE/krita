/* This file is part of the KDE project
   Copyright (C) 2005 Ariya Hidayat <ariya@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOFFICE_XMLREADER
#define KOFFICE_XMLREADER

// use standard QDom, useful to test KoXml classes against Qt's QDom
#define KOXML_USE_QDOM

#include <qdom.h> 
//Added by qt3to4:
#include <QTextStream>
#include <koffice_export.h>

#ifdef KOXML_USE_QDOM

#define KoXmlNode QDomNode
#define KoXmlNodeList QDomNodeList
#define KoXmlElement QDomElement
#define KoXmlText QDomText
#define KoXmlCDATASection QDomCDATASection
#define KoXmlDocument QDomDocument

#else

class QIODevice;
class QString;
class QTextStream;
class QXmlReader;
class QXmlInputSource;

class KoXmlElement;
class KoXmlText;
class KoXmlCDATASection;
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
 * @author Ariya Hidayat <ariya@kde.org>
 */
class KOFFICECORE_EXPORT KoXmlNode
{
public:

  enum NodeType 
  {
    NullNode = 0,
    ElementNode,
    TextNode,
    CDATASectionNode,
    ProcessingInstructionNode,
    DocumentNode 
  };

  KoXmlNode();
  KoXmlNode( const KoXmlNode& node );
  KoXmlNode& operator=( const KoXmlNode& node );
  bool operator== ( const KoXmlNode& ) const;
  bool operator!= ( const KoXmlNode& ) const;
  virtual ~KoXmlNode();

  virtual KoXmlNode::NodeType nodeType() const;
  virtual bool isNull() const;
  virtual bool isElement() const;
  virtual bool isText() const;
  virtual bool isCDATASection() const;
  virtual bool isDocument() const;

  void clear();
  KoXmlElement toElement();
  KoXmlText toText();
  KoXmlCDATASection toCDATASection();
  KoXmlDocument toDocument();

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

  KoXmlNode namedItem( const QString& name ) const;
  KoXmlNode namedItemNS( const QString& nsURI, const QString& name ) const;

  /**
   * Loads all child nodes (if any) of this node. Normally you do not need
   * to call this function as the child nodes will be automatically 
   * loaded when necessary.
   */
  void load( int depth=1 );

  /**
   * Releases all child nodes of this node. 
   */
  void unload();

protected:
  KoXmlNodeData* d;
  KoXmlNode( KoXmlNodeData* );
};

/**
 * KoXmlElement represents a tag element in a DOM tree.
 *
 * KoXmlElement holds information about an XML tag, along with its attributes.
 *
 * @author Ariya Hidayat <ariya@kde.org>
 */

class KOFFICECORE_EXPORT KoXmlElement: public KoXmlNode
{
public:
  KoXmlElement();
  KoXmlElement( const KoXmlElement& element );
  KoXmlElement& operator=( const KoXmlElement& element );
  virtual ~KoXmlElement();
  bool operator== ( const KoXmlElement& ) const;
  bool operator!= ( const KoXmlElement& ) const;

  QString tagName() const;
  QString text() const;
  virtual bool isElement() const;

  QString attribute( const QString& name ) const;
  QString attribute( const QString& name, const QString& defaultValue ) const;
  QString attributeNS( const QString& namespaceURI, const QString& localName, 
    const QString& defaultValue ) const;
  bool hasAttribute( const QString& name ) const;
  bool hasAttributeNS( const QString& namespaceURI, const QString& localName ) const;

private:
  friend class KoXmlNode;
  friend class KoXmlDocument;
  KoXmlElement( KoXmlNodeData* );
};

/**
 * KoXmlText represents a text in a DOM tree.
 * @author Ariya Hidayat <ariya@kde.org>
 */
class KOFFICECORE_EXPORT KoXmlText: public KoXmlNode
{
public:
  KoXmlText();
  KoXmlText( const KoXmlText& text );
  KoXmlText& operator=( const KoXmlText& text );
  virtual ~KoXmlText();

  QString data() const;
  void setData( const QString& data );
  virtual bool isText() const;

private:
  friend class KoXmlNode;
  friend class KoXmlDocument;
  KoXmlText( KoXmlNodeData* );
};

/**
 * KoXmlCDATASection represents a CDATA section in a DOM tree.
 * @author Ariya Hidayat <ariya@kde.org>
 */
class KOFFICECORE_EXPORT KoXmlCDATASection: public KoXmlText
{
public:
  KoXmlCDATASection();
  KoXmlCDATASection( const KoXmlCDATASection& cdata );
  KoXmlCDATASection& operator=( const KoXmlCDATASection& cdata );
  virtual ~KoXmlCDATASection();

  virtual bool isCDATASection() const;

private:
  friend class KoXmlNode;
  friend class KoXmlDocument;
  KoXmlCDATASection( KoXmlNodeData* );
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

class KOFFICECORE_EXPORT KoXmlDocument: public KoXmlNode
{
public:
  KoXmlDocument();
  KoXmlDocument( const KoXmlDocument& node );
  KoXmlDocument& operator=( const KoXmlDocument& node );
  bool operator==( const KoXmlDocument& ) const;
  bool operator!=( const KoXmlDocument& ) const;
  virtual ~KoXmlDocument();

  virtual bool isDocument() const;

  KoXmlElement documentElement() const;

  void setFastLoading( bool f );
  bool fastLoading() const;

  bool setContent( QIODevice* device, bool namespaceProcessing, 
    QString* errorMsg = 0, int* errorLine = 0, int* errorColumn = 0 );
  bool setContent( QIODevice* device, 
    QString* errorMsg = 0, int* errorLine = 0, int* errorColumn = 0 );
  bool setContent( QXmlInputSource *source, QXmlReader *reader, 
    QString* errorMsg = 0, int* errorLine = 0, int* errorColumn = 0 );
  bool setContent( const QByteArray& text, bool namespaceProcessing,
    QString *errorMsg=0, int *errorLine=0, int *errorColumn=0  );

//  bool setContent( const QCString& text, bool namespaceProcessing, QString *errorMsg=0, int *errorLine=0, int *errorColumn=0  );
//  bool setContent( const QString& text, bool namespaceProcessing, QString *errorMsg=0, int *errorLine=0, int *errorColumn=0  );
//  bool setContent( QIODevice* dev, bool namespaceProcessing, QString *errorMsg=0, int *errorLine=0, int *errorColumn=0  );
//  bool setContent( const QCString& text, QString *errorMsg=0, int *errorLine=0, int *errorColumn=0 );
//  bool setContent( const QByteArray& text, QString *errorMsg=0, int *errorLine=0, int *errorColumn=0  );
//  bool setContent( const QString& text, QString *errorMsg=0, int *errorLine=0, 
//    int *errorColumn=0  );

    
private:
  friend class KoXmlNode;
  KoXmlDocument( KoXmlNodeData* );
};

#endif // KOXML_USE_QDOM

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
namespace KoXml {

    /**
     * A namespace-aware version of QDomNode::namedItem(),
     * which also takes care of casting to a QDomElement.
     * Use this when a domelement is known to have only *one* child element
     * with a given tagname.
     *
     * Note: do *NOT* use getElementsByTagNameNS, it's recursive!
     */
    KOFFICECORE_EXPORT KoXmlElement namedItemNS( const KoXmlNode& node, 
        const char* nsURI, const char* localName );

    /**
     * Explicitly load child nodes of specified node, up to given depth.
     * This function has no effect if QDom is used.
     */
    KOFFICECORE_EXPORT void load( KoXmlNode& node, int depth = 1 );

    /**
     * Unload child nodes of specified node.
     * This function has no effect if QDom is used.
     */
    KOFFICECORE_EXPORT void unload( KoXmlNode& node );

}

#define forEachElement( elem, parent ) \
      for ( KoXmlNode _node = parent.firstChild(); !_node.isNull(); _node = _node.nextSibling() ) \
        if ( !( elem = _node.toElement() ).isNull() )


#endif // KOFFICE_XMLREADER
