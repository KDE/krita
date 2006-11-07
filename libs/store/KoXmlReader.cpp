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

/*
  This is a memory-efficient DOM implementation for KOffice. See the API 
  documentation for details.

  IMPORTANT !

  * When you change this stuff, make sure it DOES NOT BREAK the test suite.
    Build tests/koxmlreadertest.cpp and verify it. Many sleepless nights 
    have been sacrificed for this piece of code, do not let those precious 
    hours wasted!

  * Run koxmlreadertest.cpp WITH Valgrind and make sure NO illegal 
    memory read/write and any type of leak occurs. If you are not familiar 
    with Valgrind then RTFM first and come back again later on.

  * The public API shall remain as compatible as QDom. 

  * All QDom-compatible methods should behave the same. All QDom-compatible
    functions should return the same result. In case of doubt, run 
    koxmlreadertest.cpp but uncomment KOXML_USE_QDOM in koxmlreader.h 
    so that the tests are performed with standard QDom.

  Some differences compared to QDom:

  - DOM tree in KoXmlDocument is read-only, you can not modify it. This is
    sufficient for KOffice since the tree is only accessed when loading 
    a document to the application. For saving the document to XML file,
    use KoXmlWriter.

  - Comment node (like QDomComment) is not implemented as comments are 
    simply ignored.

  - DTD, entity and entity reference are not handled. Thus, the associated
    nodes (like QDomDocumentType, QDomEntity, QDomEntityReference) are also 
    not implemented.

  - Attribute mapping node is not implemented. But of course, functions to 
    query attributes of an element are available.
    
 
 */

#include "KoXmlReader.h"

#include <QTextCodec>
#include <QTextDecoder>

#ifndef KOXML_USE_QDOM

#include <qxml.h>
#include <qdom.h>

#include <QBuffer>
#include <QHash>
#include <QPair>
#include <QStringList>
#include <QVector>

/*
 Use more compact representation of in-memory nodes.
 
 Advantages: faster iteration, can facilitate real-time compression.
 Disadvantages: still buggy, eat much more memory
*/
#define KOXML_COMPACT


typedef QPair<QString,QString> KoXmlStringPair;

// this simplistic hash is rather fast-and-furious. it works because
// likely there is very few namespaced attributes per element
inline uint qHash( const KoXmlStringPair &p )
{
  return qHash(p.first[0].unicode()) ^ 0x1477;
  
  // in case of doubt, use this:
  // return qHash(p.first)^qHash(p.second);
}

// ==================================================================
//
//         KoXmlPackedItem
//
// ==================================================================

// 24 bytes on most system
class KoXmlPackedItem
{
public:
  bool attr:1;
  KoXmlNode::NodeType type:3;
  
#ifdef KOXML_COMPACT
  quint32 childStart:28;
#else
  unsigned depth:28; 
#endif
  
  unsigned nameIndex;
  unsigned nsURIIndex;
  QString value;
  
  // it is important NOT to have a copy constructor, so that growth is optimal
  // see http://doc.trolltech.com/4.2/containers.html#growth-strategies
#if 0
  KoXmlPackedItem(): attr(false), type(KoXmlNode::NullNode), depth(0) {}
#endif
};

// ==================================================================
//
//         KoXmlPackedDocument 
//
// ==================================================================

typedef QVector<KoXmlPackedItem> KoXmlPackedGroup;

// growth strategy: increase every (1 << GROUP_GROW_SHIFT) items
#define GROUP_GROW_SHIFT 3
#define GROUP_GROW_SIZE (1 << GROUP_GROW_SHIFT)

class KoXmlPackedDocument
{
public:
  bool processNamespace;
#ifdef KOXML_COMPACT
  // map given depth to the list of items
  QHash<int, KoXmlPackedGroup> groups; 
#else
  QVector<KoXmlPackedItem> items;
#endif
  
  QStringList stringList;

private:
  QHash<QString, unsigned> stringHash;
  
  unsigned cacheString( const QString& str )
  {
    if( str.isEmpty() )
      return 0;
      
    const unsigned& ii = stringHash[str];
    if(ii > 0)
      return ii;
    
    // not yet declared, so we add it
    unsigned i = stringList.count();
    stringList.append( str );
    stringHash.insert( str, i );
  
    return i;
  }
  
  QHash<QString, unsigned> valueHash;
  QStringList valueList;
  
  QString cacheValue( const QString& value )
  {
    if( value.isEmpty() )
      return 0;
      
    const unsigned& ii = valueHash[value];
    if(ii > 0)
      return valueList[ii];
    
    // not yet declared, so we add it
    unsigned i = valueList.count();
    valueList.append( value );
    valueHash.insert( value, i );
  
    return valueList[i];
  }
  
#ifdef KOXML_COMPACT
public:
  KoXmlPackedItem& itemAt( unsigned depth, unsigned index )
  {
    KoXmlPackedGroup& group = groups[depth];
    return group[index];
  }
    
  unsigned itemCount( unsigned depth )
  {
    const KoXmlPackedGroup& group = groups[depth];
    return group.count();
  }
    
    /*
       NOTE:
          Function clear, newItem, addElement, addAttribute, addText, 
          addCData, addProcessing are all related. These are all necessary
          for stateful manipulation of the document. See also the calls
          to these function from KoXmlHandler.
          
          The state itself is defined by the member variables 
          currentDepth and the groups (see above).
     */  
     
  unsigned currentDepth;
    
  KoXmlPackedItem& newItem( unsigned depth )
  {
    KoXmlPackedGroup& group = groups[depth];
    
    // reserve up front
    if( (groups.size() % GROUP_GROW_SIZE) == 0 )
      group.reserve( GROUP_GROW_SIZE * (1+(groups.size() >> GROUP_GROW_SHIFT)) );
    group.resize( group.count()+1 );
    
    // this is necessary, because intentionally we don't want to have 
    // a constructor for KoXmlPackedItem
    KoXmlPackedItem& item = group[group.count()-1];
    item.attr = false;
    item.type = KoXmlNode::NullNode;
    item.nameIndex = 0;
    item.nsURIIndex = 0;
    item.childStart = itemCount( depth + 1);
    
    return item;
  }
    
  void clear()
  {
    currentDepth = 0;
    stringHash.clear();
    stringList.clear();
    valueHash.clear();
    valueList.clear();
    groups.clear();
    
    // reserve index #0
    cacheString(QString());
    
    // first node is root
    KoXmlPackedItem& rootItem = newItem(0);
    rootItem.type = KoXmlNode::DocumentNode;
  }
    
  void finish()
  {
    // won't be needed anymore
    stringHash.clear();
    valueHash.clear();
    valueList.clear();
    
    // optimize, see documentation on QVector::squeeze
    for(int d = 0; d < groups.count(); d++)
    {
      KoXmlPackedGroup& group = groups[d];
      group.squeeze();
    }  
  }
    
  // in case namespace processing, 'name' contains the prefix already
  void addElement( const QString& name, const QString& nsURI )
  {
    KoXmlPackedItem& item = newItem(currentDepth+1); 
    item.type = KoXmlNode::ElementNode;
    item.nameIndex = cacheString(name);
    item.nsURIIndex = cacheString(nsURI);

    currentDepth++;
  }
    
  void closeElement()
  {
    currentDepth--;
  }
    
  void addAttribute( const QString& name, const QString& nsURI, const QString& value )
  {
    KoXmlPackedItem& item = newItem(currentDepth+1); 
    item.attr = true;
    item.nameIndex = cacheString(name);
    item.nsURIIndex = cacheString(nsURI);
    //item.value = cacheValue( value );
    item.value = value;
  }

  void addText( const QString& text )
  {
    KoXmlPackedItem& item = newItem(currentDepth+1); 
    item.type = KoXmlNode::TextNode;
    item.value = text;
  }
    
  void addCData( const QString& text )
  {
    KoXmlPackedItem& item = newItem(currentDepth+1); 
    item.type = KoXmlNode::CDATASectionNode;
    item.value = text;
  }
    
  void addProcessingInstruction()
  {
    KoXmlPackedItem& item = newItem(currentDepth+1); 
    item.type = KoXmlNode::ProcessingInstructionNode;
  }  
      
public:
  KoXmlPackedDocument(): processNamespace(false), currentDepth(0)
  {
    clear();
  }
  
#else  
  
private:  
  unsigned elementDepth;

public:
  
  KoXmlPackedItem& newItem()
  {
    unsigned count = items.count() + 512;
    count = 1024 * (count >> 10);
    items.reserve(count);
    
    items.resize( items.count() + 1 );
    
    // this is necessary, because intentionally we don't want to have 
    // a constructor for KoXmlPackedItem
    KoXmlPackedItem& item = items[items.count()-1];
    item.attr = false;
    item.type = KoXmlNode::NullNode;
    item.nameIndex = 0;
    item.nsURIIndex = 0;
    item.depth = 0;
    
    return item;
  }
	
  void addElement( const QString& name, const QString& nsURI )
  {
    // we are going one level deeper
    elementDepth++;
      
    KoXmlPackedItem& item = newItem();
    
    item.attr = false;
    item.type = KoXmlNode::ElementNode;
    item.depth = elementDepth;
    item.nameIndex = cacheString( name );
    item.nsURIIndex = cacheString( nsURI );
  }
	
  void closeElement()
  {
    // we are going up one level
    elementDepth--;
  }
  	
  void addAttribute( const QString& name, const QString& nsURI, const QString& value )
  {
    KoXmlPackedItem& item = newItem();
  
    item.attr = true;
    item.type = KoXmlNode::NullNode;
    item.depth = elementDepth;
    item.nameIndex = cacheString( name );
    item.nsURIIndex = cacheString( nsURI );
    //item.value = cacheValue( value );
    item.value = value;
  }

  void addText( const QString& str )
  {
    KoXmlPackedItem& item = newItem();

    item.attr = false;
    item.type = KoXmlNode::TextNode;
    item.depth = elementDepth + 1;
    item.nameIndex = 0;
    item.nsURIIndex = 0;
    item.value = str;
  }
	
  void addCData( const QString& str )
  {
    KoXmlPackedItem& item = newItem();

    item.attr = false;
    item.type = KoXmlNode::CDATASectionNode;
    item.depth = elementDepth + 1;
    item.nameIndex = 0;
    item.nsURIIndex = 0;
    item.value = str;
  }
	
  void addProcessingInstruction()
  {
    KoXmlPackedItem& item = newItem();
  
    item.attr = false;
    item.type = KoXmlNode::ProcessingInstructionNode;
    item.depth = elementDepth + 1;
    item.nameIndex = 0;
    item.nsURIIndex = 0;
    item.value.clear();
  }  
	  
  void clear()
  {
    stringList.clear();
    stringHash.clear();
    valueHash.clear();
    valueList.clear();
    items.clear();
    elementDepth = 0;
    
    // reserve index #0
    cacheString( "." );
    
    KoXmlPackedItem& rootItem = newItem();
    rootItem.attr = false;
    rootItem.type = KoXmlNode::DocumentNode;
    rootItem.depth = 0;
    rootItem.nsURIIndex = 0;
    rootItem.nameIndex = 0;
  }  
	
  void finish()
  {
    stringHash.clear();
    valueList.clear();
    valueHash.clear();
    items.squeeze();
  }  
	
  KoXmlPackedDocument(): processNamespace(false), elementDepth(0)
  {
  }
  
#endif

};

// ==================================================================
//
//         KoXmlHandler 
//
// ==================================================================

class KoXmlHandler : public QXmlDefaultHandler
{
public:
  KoXmlHandler( KoXmlPackedDocument* doc );
  ~KoXmlHandler();

  // content handler
  bool startDocument();
  bool endDocument();

  bool startElement( const QString& nsURI, const QString& localName, 
    const QString& qName, const QXmlAttributes& atts );
  bool endElement( const QString& nsURI, const QString& localName, 
    const QString& qName );

  bool characters( const QString& ch );
  bool processingInstruction( const QString& target, const QString& data );
  bool skippedEntity( const QString& name );

  // lexical handler
  bool startCDATA();
  bool endCDATA();
  bool startEntity( const QString & );
  bool endEntity( const QString & );
  bool startDTD( const QString& name, const QString& publicId, 
    const QString& systemId );
  bool comment( const QString& ch );

  // decl handler
  bool externalEntityDecl( const QString &name, const QString &publicId, 
    const QString &systemId ) ;

  // DTD handler
  bool notationDecl( const QString & name, const QString & publicId, 
    const QString & systemId );
  bool unparsedEntityDecl( const QString &name, const QString &publicId, 
    const QString &systemId, const QString &notationName ) ;

  // error handler
  bool fatalError( const QXmlParseException& exception );

  QString errorMsg;
  int errorLine;
  int errorColumn;

private:
  bool processNamespace;
  QString entityName;
  bool cdata;

  KoXmlPackedDocument* document;
};


KoXmlHandler::KoXmlHandler( KoXmlPackedDocument* doc ): 
QXmlDefaultHandler()
{
  document = doc;
  processNamespace = doc->processNamespace;

  cdata = false;
  entityName = QString();

  errorMsg = QString();
  errorLine = 0;
  errorColumn = 0;
}

KoXmlHandler::~KoXmlHandler()
{
}


bool KoXmlHandler::startDocument()
{
  // just for sanity
  cdata = false;
  entityName = QString();

  errorMsg = QString();
  errorLine = 0;
  errorColumn = 0;

  document->clear();
  return true;
}

bool KoXmlHandler::endDocument()
{
  document->finish();
  return true;
}

bool KoXmlHandler::startDTD( const QString& name, const QString& publicId, 
const QString& systemId )
{
  Q_UNUSED( name );
  Q_UNUSED( publicId );
  Q_UNUSED( systemId );

  // we skip DTD
  return true;
}

bool KoXmlHandler::startElement( const QString& nsURI, const QString& localName, 
const QString& name, const QXmlAttributes& atts )
{
  Q_UNUSED( localName );
  
  document->addElement( name, nsURI );

  // add all attributes
  for( int c=0; c<atts.length(); c++ )
    document->addAttribute( atts.qName(c), atts.uri(c), atts.value(c) );
  
  return true;
}

bool KoXmlHandler::endElement( const QString& nsURI, const QString& localName, 
const QString& qName )
{
  Q_UNUSED( nsURI );
  Q_UNUSED( localName );
  Q_UNUSED( qName );
  
  document->closeElement();
  
  return true;
}

bool KoXmlHandler::characters( const QString& str )
{
  // are we inside entity ?
  if( !entityName.isEmpty() )
  {
    // we do not handle entity but need to keep track of it
    // because we want to skip it alltogether
    return true;
  }

  // add a new text or CDATA 
  if( cdata )
    document->addCData( str );
  else
    document->addText( str );  

  return true;
}

bool KoXmlHandler::processingInstruction( const QString& target, 
const QString& data )
{
  Q_UNUSED( target );
  Q_UNUSED( data );

  document->addProcessingInstruction();

  return true;
}

bool KoXmlHandler::skippedEntity( const QString& name )
{
  Q_UNUSED( name );

  // we skip entity
  return true;
}

bool KoXmlHandler::startCDATA()
{
  cdata = true;
  return true;
}

bool KoXmlHandler::endCDATA()
{
  cdata = false;
  return true;
}

bool KoXmlHandler::startEntity( const QString& name )
{
  entityName = name;
  return true;
}

bool KoXmlHandler::endEntity( const QString& name )
{
  Q_UNUSED( name );
  entityName.clear();
  return true;
}

bool KoXmlHandler::comment( const QString& comment )
{
  Q_UNUSED( comment );

  // we skip comment
  return true;
}

bool KoXmlHandler::unparsedEntityDecl( const QString &name, 
const QString &publicId, const QString &systemId, const QString &notationName )
{
  Q_UNUSED( name );
  Q_UNUSED( publicId );
  Q_UNUSED( systemId );
  Q_UNUSED( notationName );

  // we skip entity
  return true;
}

bool KoXmlHandler::externalEntityDecl( const QString &name, 
const QString &publicId, const QString &systemId )
{
  Q_UNUSED( name );
  Q_UNUSED( publicId );
  Q_UNUSED( systemId );

  // we skip entity
  return true;
}

bool KoXmlHandler::notationDecl( const QString & name, 
const QString & publicId, const QString & systemId )
{
  Q_UNUSED( name );
  Q_UNUSED( publicId );
  Q_UNUSED( systemId );

  // we skip notation node
  return true;
}

bool KoXmlHandler::fatalError( const QXmlParseException& exception )
{
  errorMsg = exception.message();
  errorLine =  exception.lineNumber();
  errorColumn =  exception.columnNumber();
  return QXmlDefaultHandler::fatalError( exception );
}


// ==================================================================
//
//         KoXmlNodeData 
//
// ==================================================================

class KoXmlNodeData
{
public:

  KoXmlNodeData();
  ~KoXmlNodeData();

#ifdef KOXML_COMPACT
  unsigned nodeDepth;
#endif

  // generic properties
  KoXmlNode::NodeType nodeType;
  QString tagName;
  QString namespaceURI;
  QString prefix;
  QString localName;

  // reference counting
  unsigned long count;
  void ref() { count++; }
  void unref() { if( this == &null) return; if( !--count ) delete this; }

  // type information
  bool emptyDocument;
  QString nodeName() const;

  // for tree and linked-list
  KoXmlNodeData* parent;
  KoXmlNodeData* prev;
  KoXmlNodeData* next;
  KoXmlNodeData* first;
  KoXmlNodeData* last;

  QString text();

  // node manipulation
  void appendChild( KoXmlNodeData* child );
  void clear();

  // attributes
  inline void setAttribute( const QString& name, const QString& value );
  inline QString attribute( const QString& name, const QString& def ) const;
  inline bool hasAttribute( const QString& name ) const;
  inline void setAttributeNS( const QString& nsURI, const QString& name, const QString& value );
  inline QString attributeNS( const QString& nsURI, const QString& name, const QString& def ) const;
  inline bool hasAttributeNS( const QString& nsURI, const QString& name ) const;
  inline void clearAttributes();

  // for text and CDATA
  QString data() const;
  void setData( const QString& data );

  // reference from within the packed doc
  KoXmlPackedDocument* packedDoc;
  unsigned long nodeIndex;

  // for document node
  bool setContent( QXmlInputSource* source, QXmlReader* reader, 
    QString* errorMsg = 0, int* errorLine = 0, int* errorColumn = 0 );
  bool setContent( QXmlInputSource* source, bool namespaceProcessing, 
    QString* errorMsg = 0, int* errorLine = 0, int* errorColumn = 0 );

  // used when doing on-demand (re)parse
  bool loaded;
  void loadChildren( int depth=1 );
  void unloadChildren();
  
  void dump();
  
  static KoXmlNodeData null;
  
  // compatibility
  QDomNode asQDomNode( QDomDocument ownerDoc ) const;

private:
  QHash<QString,QString> attr;  
  QHash<KoXmlStringPair,QString> attrNS;
  QString textData;
  friend class KoXmlHandler;
  friend class KoXmlElement;
};

KoXmlNodeData KoXmlNodeData::null;

KoXmlNodeData::KoXmlNodeData()
{
#ifdef KOXML_COMPACT
  nodeDepth = 0;
#endif
  nodeType = KoXmlNode::NullNode;

  tagName = QString();
  prefix = QString();
  localName = QString();
  namespaceURI = QString();
  textData = QString();

  count = 1;
  parent = 0;
  prev = next = 0;
  first = last = 0;

  packedDoc = 0;
  nodeIndex = 0;
  
  emptyDocument = true;
  
  loaded = false;
}

KoXmlNodeData::~KoXmlNodeData()
{ 
  clear();
}

void KoXmlNodeData::clear()
{
  if( first )
  for( KoXmlNodeData* node = first; node ; )
  {
    KoXmlNodeData* next = node->next;
    node->unref();
    node = next;
  }

  // only document can delete these
  // normal nodes don't "own" them
  if(nodeType == KoXmlNode::DocumentNode)
    delete packedDoc;

  nodeType = KoXmlNode::NullNode;
  tagName.clear();
  prefix.clear();
  namespaceURI.clear();
  textData.clear();
  packedDoc = 0;

  attr.clear();
  attrNS.clear();

  parent = 0;
  prev = next = 0;
  first = last = 0;
  
  loaded = false;
}

QString KoXmlNodeData::text()
{
  QString t;

  loadChildren();

  KoXmlNodeData* node = first;
  while ( node ) 
  {
    switch( node->nodeType )
    {
      case KoXmlNode::ElementNode: 
        t += node->text(); break;
      case KoXmlNode::TextNode:
        t += node->data(); break;
      case KoXmlNode::CDATASectionNode:
        t += node->data(); break;
      default: break;
    }
    node = node->next;
  }

  return t;
}

QString KoXmlNodeData::nodeName() const
{
  QString n;

  switch( nodeType )
  {
    case KoXmlNode::ElementNode: 
      n = tagName;
      if (!prefix.isEmpty()) n.prepend(':').prepend(prefix); break;
    case KoXmlNode::TextNode: return QLatin1String("#text");
    case KoXmlNode::CDATASectionNode: return QLatin1String("#cdata-section");
    case KoXmlNode::DocumentNode: return QLatin1String("#document");
    default: break;
  }

  return n;
}

void KoXmlNodeData::appendChild( KoXmlNodeData* node )
{
  node->parent = this;
  if( !last )
    first = last = node;
  else
  {
    last->next = node;
    node->prev = last;
    node->next = 0;
    last = node;
  }
}

void KoXmlNodeData::setAttribute( const QString& name, const QString& value )
{
  attr[ name ] = value;
}

QString KoXmlNodeData::attribute( const QString& name, const QString& def ) const
{
  if( attr.contains(name) )
    return attr[ name ];
  else
    return def;  
}

bool KoXmlNodeData::hasAttribute( const QString& name ) const
{
  return attr.contains( name );
}

void KoXmlNodeData::setAttributeNS( const QString& nsURI, 
const QString& name, const QString& value )
{
  QString prefix;
  QString localName = name;
  int i = name.indexOf( ':' );
  if( i != -1 )
  {
    localName = name.mid( i + 1 );
    prefix = name.left( i );
  }
  
  if( prefix.isNull() ) return;
  
  KoXmlStringPair key( nsURI, localName );
  attrNS[ key ] = value;
}

QString KoXmlNodeData::attributeNS( const QString& nsURI, const QString& name, 
const QString& def ) const
{
  KoXmlStringPair key( nsURI, name );
  if( attrNS.contains( key ) )
    return attrNS[ key ];
  else
    return def;  
}

bool KoXmlNodeData::hasAttributeNS( const QString& nsURI, const QString& name ) const
{
  KoXmlStringPair key( nsURI, name );
  return attrNS.contains( key );
}

void KoXmlNodeData::clearAttributes()
{
  attr.clear();
  attrNS.clear();
}

QString KoXmlNodeData::data() const
{
  return textData;
}

void KoXmlNodeData::setData( const QString& d )
{
  textData = d;
}

bool KoXmlNodeData::setContent( QXmlInputSource* source, bool namespaceProcessing, 
QString* errorMsg, int* errorLine, int* errorColumn )
{
  QXmlSimpleReader reader;
  reader.setFeature(QLatin1String("http://xml.org/sax/features/namespaces"), namespaceProcessing);
  reader.setFeature(QLatin1String("http://xml.org/sax/features/namespace-prefixes"), !namespaceProcessing);
  reader.setFeature(QLatin1String("http://trolltech.com/xml/features/report-whitespace-only-CharData"), false);

  return setContent(source, &reader, errorMsg, errorLine, errorColumn);
}

bool KoXmlNodeData::setContent( QXmlInputSource* source, 
QXmlReader* reader, QString* errorMsg, int* errorLine, int* errorColumn )
{
  if( nodeType != KoXmlNode::DocumentNode )
    return false;
    
  clear();
  nodeType = KoXmlNode::DocumentNode;

  // sanity checks
  if( !source ) return false;
  if( !reader ) return false;

  delete packedDoc;
  packedDoc = new KoXmlPackedDocument;
  packedDoc->processNamespace = false;

  packedDoc->processNamespace = 
    reader->feature( "http://xml.org/sax/features/namespaces" ) && 
    !reader->feature( "http://xml.org/sax/features/namespace-prefixes" );

  KoXmlHandler handler( packedDoc );
  reader->setContentHandler( &handler );
  reader->setErrorHandler( &handler );
  reader->setLexicalHandler( &handler );
  reader->setDeclHandler( &handler );
  reader->setDTDHandler( &handler );
  
  bool result = reader->parse(source);
  if( !result ) 
  {
    // parsing error has occurred
    if( errorMsg ) *errorMsg = handler.errorMsg;
    if( errorLine ) *errorLine = handler.errorLine;
    if( errorColumn )  *errorColumn = handler.errorColumn;
    return false;
  }
  
  
  // initially load 
  loadChildren();

  return true;
}

#ifdef KOXML_COMPACT

void KoXmlNodeData::loadChildren( int depth )
{
  // sanity check
  if( !packedDoc ) return;

  // already loaded ?
  if( loaded && (depth<=1) ) return;
  
  // in case depth is different
  unloadChildren();
  
  
  KoXmlNodeData* lastDat = 0;
  const KoXmlPackedItem& self = packedDoc->itemAt( nodeDepth, nodeIndex );
  
  unsigned childStop = 0;
  if( nodeIndex == packedDoc->itemCount(nodeDepth)-1 )
    childStop = packedDoc->itemCount(nodeDepth+1);
  else  
  {
    const KoXmlPackedItem& next = packedDoc->itemAt( nodeDepth, nodeIndex+1 );
    childStop = next.childStart;
  }  

  for(unsigned i = self.childStart; i < childStop; i++ )
  {
    const KoXmlPackedItem& item = packedDoc->itemAt( nodeDepth + 1, i );
    bool textItem = (item.type==KoXmlNode::TextNode);
    textItem |= (item.type==KoXmlNode::CDATASectionNode);
    
    // attribute belongs to this node
    if(item.attr)
    {
      QString name = packedDoc->stringList[item.nameIndex];
      QString nsURI = packedDoc->stringList[item.nsURIIndex];
      QString value = item.value;
      
      QString prefix;
      
      QString qName; // with prefix
      QString localName;  // without prefix, i.e. local name

      localName = qName = name;
      int i = qName.indexOf( ':' );
      if( i != -1 ) prefix = qName.left( i );
      if( i != -1 ) localName = qName.mid( i + 1 );

      if(packedDoc->processNamespace)
      {
        setAttributeNS(nsURI, qName, value);
        setAttribute(localName, value);  
      }
      else
        setAttribute( qName, value );
    }
    else
    {
      QString name = packedDoc->stringList[item.nameIndex];
      QString nsURI = packedDoc->stringList[item.nsURIIndex];
      QString value = item.value;
    
      QString nodeName = name;
      QString localName;
      QString prefix;
      
      if(packedDoc->processNamespace)
      {
        localName = name;
        int di = name.indexOf( ':' );
        if( di != -1 )
        {
          localName = name.mid( di + 1 );
          prefix = name.left( di );
        }
        nodeName = localName;
      }

      // make a node out of this item
      KoXmlNodeData* dat = new KoXmlNodeData;
      dat->nodeIndex = i;
      dat->packedDoc = packedDoc;
      dat->nodeDepth = nodeDepth + 1;
      dat->nodeType = item.type;
      dat->tagName = nodeName;
      dat->localName = localName;
      dat->prefix = prefix;
      dat->namespaceURI = nsURI;
      dat->count = 1;
      dat->parent = this;
      dat->prev = lastDat;
      dat->next = 0;
      dat->first = 0;
      dat->last = 0;
      dat->loaded = false;        
      dat->textData = (textItem) ? value : QString();
        
      // adjust our linked-list
      first = (first) ? first : dat;
      last = dat;
      if(lastDat)
        lastDat->next = dat;
      lastDat = dat;
      
      // recursive
      if(depth > 1)
        dat->loadChildren(depth - 1);
    }
  }
  
  loaded = true;
}

#else

void KoXmlNodeData::loadChildren( int depth )
{
  // sanity check
  if( !packedDoc ) return;

  // already loaded ?
  if( loaded && (depth<=1) ) return;
  
  // cause we don't know how deep this node's children already loaded are
  unloadChildren();
  
  KoXmlNodeData* lastDat = 0;
  int nodeDepth = packedDoc->items[nodeIndex].depth;

  for(int i = nodeIndex + 1; i < packedDoc->items.count(); i++ )
  {
    KoXmlPackedItem& item = packedDoc->items[i];
    bool textItem = (item.type==KoXmlNode::TextNode);
    textItem |= (item.type==KoXmlNode::CDATASectionNode);
    
    // element already outside our depth
    if(!item.attr && (item.type == KoXmlNode::ElementNode))
    if(item.depth <= (unsigned)nodeDepth)
      break;
    
    // attribute belongs to this node
    if(item.attr && (item.depth == (unsigned)nodeDepth))
    {
      QString name = packedDoc->stringList[item.nameIndex];
      QString nsURI = packedDoc->stringList[item.nsURIIndex];
      QString value = item.value;
      
      QString prefix;
      
      QString qName; // with prefix
      QString localName;  // without prefix, i.e. local name

      localName = qName = name;
      int i = qName.indexOf( ':' );
      if( i != -1 ) prefix = qName.left( i );
      if( i != -1 ) localName = qName.mid( i + 1 );

      if(packedDoc->processNamespace)
      {
        setAttributeNS(nsURI, qName, value);
        setAttribute(localName, value);  
      }
      else
        setAttribute( name, value );
    }
    
      // the child node
    if(!item.attr)
    {
      bool instruction = (item.type == KoXmlNode::ProcessingInstructionNode);
      bool ok = (textItem||instruction)  ? (item.depth==(unsigned)nodeDepth) : (item.depth==(unsigned)nodeDepth+1);
      
      ok = (item.depth==(unsigned)nodeDepth+1);
        
      if(ok)
      {
        QString name = packedDoc->stringList[item.nameIndex];
        QString nsURI = packedDoc->stringList[item.nsURIIndex];
        QString value = item.value;
      
        QString nodeName = name;
        QString localName;
        QString prefix;
        
        if(packedDoc->processNamespace)
        {
          localName = name;
          int di = name.indexOf( ':' );
          if( di != -1 )
          {
            localName = name.mid( di + 1 );
            prefix = name.left( di );
          }
          nodeName = localName;
        }

        // make a node out of this item
        KoXmlNodeData* dat = new KoXmlNodeData;
        dat->nodeIndex = i;
        dat->packedDoc = packedDoc;
        dat->nodeType = item.type;
        dat->tagName = nodeName;
        dat->localName = localName;
        dat->prefix = prefix;
        dat->namespaceURI = nsURI;
        dat->count = 1;
        dat->parent = this;
        dat->prev = lastDat;
        dat->next = 0;
        dat->first = 0;
        dat->last = 0;
        dat->loaded = false;        
        dat->textData = (textItem) ? value : QString();
        
        // adjust our linked-list
        first = (first) ? first : dat;
        last = dat;
        if(lastDat)
          lastDat->next = dat;
        lastDat = dat;
        
        // recursive
        if(depth > 1)
          dat->loadChildren(depth - 1);
      }
  }
  }
  
  loaded = true;
}
#endif

void KoXmlNodeData::unloadChildren()
{
  // sanity check
  if( !packedDoc ) return;

  if( !loaded ) return;
  
  if( first )
  for( KoXmlNodeData* node = first; node ; )
  {
    KoXmlNodeData* next = node->next;
    node->unloadChildren();
    node->unref();
    node = next;
  }

  clearAttributes();
  loaded = false;
  first = last = 0;
}

#ifdef KOXML_COMPACT


static QDomNode itemAsQDomNode( QDomDocument ownerDoc, KoXmlPackedDocument* packedDoc,
unsigned nodeDepth, unsigned nodeIndex )
{
  // sanity check
  if( !packedDoc )
    return QDomNode();
    
  const KoXmlPackedItem& self = packedDoc->itemAt( nodeDepth, nodeIndex );  
  
  unsigned childStop = 0;
  if( nodeIndex == packedDoc->itemCount(nodeDepth)-1 )
    childStop = packedDoc->itemCount(nodeDepth+1);
  else  
  {
    const KoXmlPackedItem& next = packedDoc->itemAt( nodeDepth, nodeIndex+1 );
    childStop = next.childStart;
  }  
  
  // nothing to do here
  if( self.type == KoXmlNode::NullNode )
    return QDomNode();

  // create the element properly
  if( self.type == KoXmlNode::ElementNode )
  {
    QDomElement element;
    
    QString name = packedDoc->stringList[self.nameIndex];
    QString nsURI = packedDoc->stringList[self.nsURIIndex];
    
    if( packedDoc->processNamespace )
      element = ownerDoc.createElementNS( nsURI, name );
    else
      element = ownerDoc.createElement( name );

    // check all subnodes for attributes
    for(unsigned i = self.childStart; i < childStop; i++ )
    {
      const KoXmlPackedItem& item = packedDoc->itemAt( nodeDepth+1, i );
      bool textItem = (item.type==KoXmlNode::TextNode);
      textItem |= (item.type==KoXmlNode::CDATASectionNode);
    
      // attribute belongs to this node
      if(item.attr)
      {
        QString name = packedDoc->stringList[item.nameIndex];
        QString nsURI = packedDoc->stringList[item.nsURIIndex];
        QString value = item.value;
        
        QString prefix;
        
        QString qName; // with prefix
        QString localName;  // without prefix, i.e. local name
  
        localName = qName = name;
        int i = qName.indexOf( ':' );
        if( i != -1 ) prefix = qName.left( i );
        if( i != -1 ) localName = qName.mid( i + 1 );
  
        if(packedDoc->processNamespace)
        {
          element.setAttributeNS(nsURI, qName, value);
          element.setAttribute(localName, value);  
        }
        else
          element.setAttribute( name, value );
      }
      else
      { 
        // add it recursively
        QDomNode childNode = itemAsQDomNode( ownerDoc, packedDoc, nodeDepth+1, i );
        element.appendChild( childNode );
      }  
    }
      
    return element;
  }
  
  // create the text node
  if( self.type == KoXmlNode::TextNode )
  {
    QString text = self.value;
    
    // FIXME: choose CDATA when the value contains special characters
    QDomText textNode = ownerDoc.createTextNode( text );
    return textNode;
  }
          
  // nothing matches? strange...    
  return QDomNode();
}

QDomNode KoXmlNodeData::asQDomNode( QDomDocument ownerDoc ) const
{
  return itemAsQDomNode( ownerDoc, packedDoc, nodeDepth, nodeIndex );
}

#else

static QDomNode itemAsQDomNode( QDomDocument ownerDoc, KoXmlPackedDocument* packedDoc,
unsigned nodeIndex )
{
  // sanity check
  if( !packedDoc )
    return QDomNode();
    
  KoXmlPackedItem& item = packedDoc->items[nodeIndex];  
    
  // nothing to do here
  if( item.type == KoXmlNode::NullNode )
    return QDomNode();

  // create the element properly
  if( item.type == KoXmlNode::ElementNode )
  {
    QDomElement element;
    
    QString name = packedDoc->stringList[item.nameIndex];
    QString nsURI = packedDoc->stringList[item.nsURIIndex];
    
    if( packedDoc->processNamespace )
      element = ownerDoc.createElementNS( nsURI, name );
    else
      element = ownerDoc.createElement( name );

    // check all subnodes for attributes
    int nodeDepth = item.depth;
    for(int i = nodeIndex + 1; i < packedDoc->items.count(); i++ )
    {
      KoXmlPackedItem& item = packedDoc->items[i];
      bool textItem = (item.type==KoXmlNode::TextNode);
      textItem |= (item.type==KoXmlNode::CDATASectionNode);
    
      // element already outside our depth
      if(!item.attr && (item.type == KoXmlNode::ElementNode))
      if(item.depth <= (unsigned)nodeDepth)
        break;
    
      // attribute belongs to this node
      if(item.attr && (item.depth == (unsigned)nodeDepth))
      {
        QString name = packedDoc->stringList[item.nameIndex];
        QString nsURI = packedDoc->stringList[item.nsURIIndex];
        QString value = item.value;
        QString prefix;
      
        QString qName; // with prefix
        QString localName;  // without prefix, i.e. local name

        localName = qName = name;
        int i = qName.indexOf( ':' );
        if( i != -1 ) prefix = qName.left( i );
        if( i != -1 ) localName = qName.mid( i + 1 );

        if(packedDoc->processNamespace)
        {
          element.setAttributeNS(nsURI, qName, value);
          element.setAttribute(localName, value);  
        }
        else
          element.setAttribute( name, value );
      }
      
      // direct child of this node
      if( !item.attr && (item.depth == (unsigned)nodeDepth+1) )
      { 
        // add it recursively
        QDomNode childNode = itemAsQDomNode( ownerDoc, packedDoc, i );
        element.appendChild( childNode );
      }  
    }
      
    return element;
  }
  
  // create the text node
  if( item.type == KoXmlNode::TextNode )
  {
    QString text = item.value;
    // FIXME: choose CDATA when the value contains special characters
    QDomText textNode = ownerDoc.createTextNode( text );
    return textNode;
  }
          
  // nothing matches? strange...    
  return QDomNode();
}

QDomNode KoXmlNodeData::asQDomNode( QDomDocument ownerDoc ) const
{
  return itemAsQDomNode( ownerDoc, packedDoc, nodeIndex );
}

#endif

void KoXmlNodeData::dump()
{
  printf("NodeData %p\n", this);

  printf("  nodeIndex: %d\n", (int)nodeIndex);  
  printf("  packedDoc: %p\n", packedDoc);
  
  printf("  nodeType : %d\n", (int)nodeType);
  printf("  tagName: %s\n", qPrintable( tagName ) );
  printf("  namespaceURI: %s\n", qPrintable( namespaceURI ) );
  printf("  prefix: %s\n", qPrintable( prefix ) );
  printf("  localName: %s\n", qPrintable( localName ) );
  
  printf("  parent : %p\n", parent);
  printf("  prev : %p\n", prev);
  printf("  next : %p\n", next);
  printf("  first : %p\n", first);
  printf("  last : %p\n", last);
  
  printf("  count: %ld\n", count);
  
  if(loaded)
  printf("  loaded: TRUE\n");
  else
  printf("  loaded: FALSE\n");
}

// ==================================================================
//
//         KoXmlNode 
//
// ==================================================================

// Creates a null node
KoXmlNode::KoXmlNode()
{
  d = &KoXmlNodeData::null;
}

// Destroys this node
KoXmlNode::~KoXmlNode()
{
  if( d )
    if( d != &KoXmlNodeData::null )
      d->unref();

  d = 0;
}

// Creates a copy of another node
KoXmlNode::KoXmlNode( const KoXmlNode& node )
{
  d = node.d;
  d->ref();
}

// Creates a node for specific implementation
KoXmlNode::KoXmlNode( KoXmlNodeData* data )
{
  d = data;
  data->ref();
}

// Creates a shallow copy of another node
KoXmlNode& KoXmlNode::operator=( const KoXmlNode& node )
{
  d->unref();
  d = node.d;
  d->ref();
  return *this;
}

// Note: two null nodes are always equal
bool KoXmlNode::operator==( const KoXmlNode& node ) const
{
  if( isNull() && node.isNull() ) return true;
  return( d==node.d );
}

// Note: two null nodes are always equal
bool KoXmlNode::operator!=( const KoXmlNode& node ) const
{
  if( isNull() && !node.isNull() ) return true;
  if( !isNull() && node.isNull() ) return true;
  if( isNull() && node.isNull() ) return false;
  return( d!=node.d );
}

KoXmlNode::NodeType KoXmlNode::nodeType() const
{
  return d->nodeType;
}

bool KoXmlNode::isNull() const
{
  return d->nodeType == NullNode;
}

bool KoXmlNode::isElement() const
{
  return d->nodeType == ElementNode;
}

bool KoXmlNode::isText() const
{
  return (d->nodeType == TextNode) || isCDATASection();
}

bool KoXmlNode::isCDATASection() const
{
  return d->nodeType == CDATASectionNode;
}

bool KoXmlNode::isDocument() const
{
  return d->nodeType == DocumentNode;
}

void KoXmlNode::clear()
{
  d->unref();
  d = new KoXmlNodeData;
}

QString KoXmlNode::nodeName() const
{
  return d->nodeName();
}

QString KoXmlNode::prefix() const
{
  return isElement() ? d->prefix : QString();
}

QString KoXmlNode::namespaceURI() const
{
  return isElement() ? d->namespaceURI : QString();
}

QString KoXmlNode::localName() const
{
  return isElement() ? d->localName : QString();
}

KoXmlDocument KoXmlNode::ownerDocument() const
{
  KoXmlNodeData* node = d; 
  while( node->parent ) node = node->parent;

  return KoXmlDocument( node );
}

KoXmlNode KoXmlNode::parentNode() const
{
  return d->parent ? KoXmlNode( d->parent ) : KoXmlNode();
}

bool KoXmlNode::hasChildNodes() const
{
  if( isText() )
    return false;
  
  if( !d->loaded)  
    d->loadChildren();

  return d->first!=0 ;
}

int KoXmlNode::childNodesCount() const
{
  if( isText() )
    return 0;
    
  if( !d->loaded)  
    d->loadChildren();

  KoXmlNodeData* node = d->first;
  int count = 0;
  while ( node ) 
  {
    count++;
    node = node->next;
  }
  
  return count;
}

KoXmlNode KoXmlNode::firstChild() const
{
  if( !d->loaded)  
    d->loadChildren();
  return d->first ? KoXmlNode( d->first ) : KoXmlNode();
}

KoXmlNode KoXmlNode::lastChild() const
{
  if( !d->loaded)  
    d->loadChildren();
  return d->last ? KoXmlNode( d->last ) : KoXmlNode();
}

KoXmlNode KoXmlNode::nextSibling() const
{
  return d->next ? KoXmlNode( d->next ) : KoXmlNode();
}

KoXmlNode KoXmlNode::previousSibling() const
{
  return d->prev ? KoXmlNode( d->prev ) : KoXmlNode();
}

KoXmlNode KoXmlNode::namedItem( const QString& name ) const
{
  if( !d->loaded)  
    d->loadChildren();

  KoXmlNodeData* node = d->first;
  while ( node ) 
  {
    if( node->nodeName() == name )
      return KoXmlNode( node );
    node = node->next;
  }

  // not found
  return KoXmlNode();
}

KoXmlNode KoXmlNode::namedItemNS( const QString& nsURI, const QString& name ) const
{
  if( !d->loaded)  
    d->loadChildren();


  KoXmlNodeData* node = d->first;
  while ( node ) 
  {
    if( !node->prefix.isNull() )
    if( node->namespaceURI == nsURI )
    if( node->localName == name )
      return KoXmlNode( node );
    node = node->next;
  }

  // not found
  return KoXmlNode();
}

KoXmlElement KoXmlNode::toElement() const
{
  return isElement() ? KoXmlElement( d ) : KoXmlElement();
}

KoXmlText KoXmlNode::toText() const
{
  return isText() ? KoXmlText( d ) : KoXmlText();
}

KoXmlCDATASection KoXmlNode::toCDATASection() const
{
  return isCDATASection() ? KoXmlCDATASection( d ) : KoXmlCDATASection();
}

KoXmlDocument KoXmlNode::toDocument() const
{
  if( isDocument() )
    return KoXmlDocument( d );
    
  KoXmlDocument newDocument;
  newDocument.d->emptyDocument = false;
  return newDocument;
}

void KoXmlNode::load( int depth )
{
  d->loadChildren( depth );
}

void KoXmlNode::unload()
{
  d->unloadChildren();
}

QDomNode KoXmlNode::asQDomNode( QDomDocument ownerDoc ) const
{
  return d->asQDomNode( ownerDoc );
}

// ==================================================================
//
//         KoXmlElement 
//
// ==================================================================

// Creates an empty element
KoXmlElement::KoXmlElement(): KoXmlNode( new KoXmlNodeData )
{
  // because referenced also once in KoXmlNode constructor
  d->unref(); 
}

KoXmlElement::~KoXmlElement()
{
  if( d )
    if( d != &KoXmlNodeData::null )
      d->unref();

  d = 0;
}

// Creates a shallow copy of another element
KoXmlElement::KoXmlElement( const KoXmlElement& element ): KoXmlNode( element.d )
{
}

KoXmlElement::KoXmlElement( KoXmlNodeData* data ): KoXmlNode( data )
{
}

// Copies another element
KoXmlElement& KoXmlElement::operator=( const KoXmlElement& element )
{
  KoXmlNode::operator=( element );
  return *this;
}

bool KoXmlElement::operator== ( const KoXmlElement& element ) const
{
  if( isNull() || element.isNull() ) return false;
  return (d==element.d);
}

bool KoXmlElement::operator!= ( const KoXmlElement& element ) const
{
  if( isNull() && element.isNull() ) return false;
  if( isNull() || element.isNull() ) return true;
  return (d!=element.d);
}

QString KoXmlElement::tagName() const
{
  return isElement() ? ((KoXmlNodeData*)d)->tagName: QString();
}

QString KoXmlElement::text() const
{
  return d->text();
}

QString KoXmlElement::attribute( const QString& name ) const
{
  if( !isElement() )
    return QString();
    
  if( !d->loaded)  
    d->loadChildren();

  return d->attribute( name, QString() );
}

QString KoXmlElement::attribute( const QString& name, 
const QString& defaultValue ) const
{
  if( !isElement() )
    return defaultValue;
    
  if( !d->loaded)  
    d->loadChildren();

  return d->attribute( name, defaultValue );
}

QString KoXmlElement::attributeNS( const QString& namespaceURI, 
const QString& localName, const QString& defaultValue ) const
{
  if( !isElement() )
    return defaultValue;
    
  if( !d->loaded)  
    d->loadChildren();

  KoXmlStringPair key( namespaceURI, localName );
  if( d->attrNS.contains( key ) )
    return d->attrNS[ key ];
  else
    return defaultValue;  

//  return d->attributeNS( namespaceURI, localName, defaultValue );
}

bool KoXmlElement::hasAttribute( const QString& name ) const
{
  if( !d->loaded)  
    d->loadChildren();

  return isElement() ? d->hasAttribute( name ) : false;
}

bool KoXmlElement::hasAttributeNS( const QString& namespaceURI, 
const QString& localName ) const
{
  if( !d->loaded)  
    d->loadChildren();

  return isElement() ? d->hasAttributeNS( namespaceURI, localName ) : false;
}

// ==================================================================
//
//         KoXmlText
//
// ==================================================================

KoXmlText::KoXmlText(): KoXmlNode( new KoXmlNodeData )
{
  // because referenced also once in KoXmlNode constructor
  d->unref();
}

KoXmlText::~KoXmlText()
{
  if( d )
    if( d != &KoXmlNodeData::null )
      d->unref();

  d = 0;
}

KoXmlText::KoXmlText( const KoXmlText& text ): KoXmlNode( text.d )
{
}

KoXmlText::KoXmlText( KoXmlNodeData* data ): KoXmlNode( data )
{
}

bool KoXmlText::isText() const
{
  return true;
}

QString KoXmlText::data() const
{
  return d->data();
}

KoXmlText& KoXmlText::operator=( const KoXmlText& element )
{
  KoXmlNode::operator=( element );
  return *this;
}

// ==================================================================
//
//         KoXmlCDATASection
//
// ==================================================================

KoXmlCDATASection::KoXmlCDATASection(): KoXmlText()
{
  d->nodeType = KoXmlNode::CDATASectionNode;
}

KoXmlCDATASection::~KoXmlCDATASection()
{
  d->unref();
  d = 0;
}

KoXmlCDATASection::KoXmlCDATASection( KoXmlNodeData* cdata ):
KoXmlText( cdata )
{
}

bool KoXmlCDATASection::isCDATASection() const
{
  return true;
}

KoXmlCDATASection& KoXmlCDATASection::operator=( const KoXmlCDATASection& cdata )
{
  KoXmlNode::operator=( cdata );
  return *this;
}

// ==================================================================
//
//         KoXmlDocument 
//
// ==================================================================

KoXmlDocument::KoXmlDocument(): KoXmlNode()
{
  d->emptyDocument = false;
}

KoXmlDocument::~KoXmlDocument()
{
  if( d )
    if( d != &KoXmlNodeData::null )
      d->unref();

  d = 0;
}

KoXmlDocument::KoXmlDocument( KoXmlNodeData* data ): KoXmlNode( data )
{
  d->emptyDocument = true;
}

// Creates a copy of another document
KoXmlDocument::KoXmlDocument( const KoXmlDocument& doc ): KoXmlNode( doc.d )
{
}

// Creates a shallow copy of another document
KoXmlDocument& KoXmlDocument::operator=( const KoXmlDocument& doc )
{
  KoXmlNode::operator=( doc );
  return *this;
}

// Checks if this document and doc are equals
bool KoXmlDocument::operator==( const KoXmlDocument& doc ) const
{
  return( d==doc.d );
}

// Checks if this document and doc are not equals
bool KoXmlDocument::operator!=( const KoXmlDocument& doc ) const
{
  return( d!=doc.d );
}

KoXmlElement KoXmlDocument::documentElement() const
{
  d->loadChildren();
  
  for( KoXmlNodeData* node=d->first; node; )
  {
    if( node->nodeType==KoXmlNode::ElementNode )
      return KoXmlElement( node );
    else node = node->next;  
  }
  
  return KoXmlElement();
}

QString KoXmlDocument::nodeName() const
{
  if( d->emptyDocument )
    return QLatin1String("#document");
  else   
    return QString();
}

void KoXmlDocument::clear()
{
  KoXmlNode::clear();
  d->emptyDocument = false;
}

bool KoXmlDocument::setContent( QXmlInputSource *source, QXmlReader *reader, 
    QString* errorMsg, int* errorLine, int* errorColumn )
{
  if( d->nodeType != KoXmlNode::DocumentNode ) 
  {
    d->unref();
    d = new KoXmlNodeData;
    d->nodeType = KoXmlNode::DocumentNode;
  }

  return d->setContent( source, reader, errorMsg, errorLine, errorColumn );
}

// no namespace processing
bool KoXmlDocument::setContent( QIODevice* device, QString* errorMsg,
int* errorLine, int* errorColumn )
{
  return setContent( device, false, errorMsg, errorLine, errorColumn );
}

bool KoXmlDocument::setContent( QIODevice* device, bool namespaceProcessing, 
QString* errorMsg, int* errorLine, int* errorColumn )
{
  if( d->nodeType != KoXmlNode::DocumentNode ) 
  {
    d->unref();
    d = new KoXmlNodeData;
    d->nodeType = KoXmlNode::DocumentNode;
  }

  QXmlSimpleReader reader;
  reader.setFeature( "http://xml.org/sax/features/namespaces", namespaceProcessing );
  reader.setFeature( "http://xml.org/sax/features/namespace-prefixes", !namespaceProcessing );
  reader.setFeature( "http://trolltech.com/xml/features/report-whitespace-only-CharData", false );

  // FIXME this hack is apparently private
  //reader.setUndefEntityInAttrHack(true);

  QXmlInputSource source( device );
  return d->setContent( &source, &reader, errorMsg, errorLine, errorColumn );
}

bool KoXmlDocument::setContent( const QByteArray& text, bool namespaceProcessing,
QString *errorMsg, int *errorLine, int *errorColumn )
{
  QBuffer buffer;
  buffer.setData( text );
  return setContent( &buffer, namespaceProcessing, errorMsg, errorLine, errorColumn );
}

bool KoXmlDocument::setContent(const QString& text, bool namespaceProcessing, 
QString *errorMsg, int *errorLine, int *errorColumn)
{
  if( d->nodeType != KoXmlNode::DocumentNode ) 
  {
    d->unref();
    d = new KoXmlNodeData;
    d->nodeType = KoXmlNode::DocumentNode;
  }

  QXmlInputSource source;
  source.setData(text);
  return d->setContent( &source, namespaceProcessing, errorMsg, errorLine, errorColumn );
}

bool KoXmlDocument::setContent(const QString& text,  
QString *errorMsg, int *errorLine, int *errorColumn)
{
  return setContent( text, false, errorMsg, errorLine, errorColumn );
}

#endif

// ==================================================================
//
//         KoXmlInputSource 
//
// ==================================================================

/*
  This is the size of buffer every time we read a chunk of data from the device.

  Note 1: maximum allocated space is thus 2*KOXML_BUFSIZE due to the
  stringData (a QString instance).
  TODO: use mmap to avoid double-buffering like this.
  
  Note 2: a much larger buffer won't speed up significantly. This is because
  the bottleneck is elsewhere, not here.
  
*/

#define KOXML_BUFSIZE 16*1024  // should be adequate

KoXmlInputSource::KoXmlInputSource(QIODevice *dev): QXmlInputSource(), 
device( dev )
{
  int mib = 106; // UTF-8
  decoder = QTextCodec::codecForMib( mib )->makeDecoder();     

  stringLength = 0;
  stringIndex = 0;
  buffer = new char[KOXML_BUFSIZE];

  reset();
}

KoXmlInputSource::~KoXmlInputSource()
{
  delete decoder;
  delete [] buffer;
}

void KoXmlInputSource::setData(const QString& dat)
{
  Q_UNUSED(dat);
}

void KoXmlInputSource::setData(const QByteArray& dat)
{
  Q_UNUSED(dat);
}

void KoXmlInputSource::fetchData()
{
}

QString KoXmlInputSource::data() const
{
  return QString();
}

QChar KoXmlInputSource::next()
{
  if(stringIndex >= stringLength)
  {
    // read more data first
    qint64 bytes = device->read( buffer, KOXML_BUFSIZE );
    if( bytes == 0 )
      return EndOfDocument;

    stringData = decoder->toUnicode( buffer, bytes );
    stringLength = stringData.length();
    stringIndex = 0;
  }

  return stringData[stringIndex++];
}

void KoXmlInputSource::reset()
{
  device->seek(0);
}

QString KoXmlInputSource::fromRawData(const QByteArray &data, bool beginning)
{
  Q_UNUSED( data );
  Q_UNUSED( beginning );
  return QString();
}

// ==================================================================
//
//         functions in KoXml namespace
//
// ==================================================================

KoXmlElement KoXml::namedItemNS( const KoXmlNode& node, const char* nsURI, 
const char* localName )
{
#ifdef KOXML_USE_QDOM
    // David's solution for namedItemNS, only for QDom stuff
    KoXmlNode n = node.firstChild();
    for ( ; !n.isNull(); n = n.nextSibling() ) {
        if ( n.isElement() && n.localName() == localName && 
            n.namespaceURI() == nsURI )
               return n.toElement();
    }
    return KoXmlElement();
#else
  return node.namedItemNS( nsURI, localName).toElement();
#endif
}

void KoXml::load( KoXmlNode& node, int depth )
{
#ifdef KOXML_USE_QDOM
  // do nothing, QDom has no on-demand loading
  Q_UNUSED( node );
  Q_UNUSED( depth );
#else
  node.load( depth );
#endif
}


void KoXml::unload( KoXmlNode& node )
{
#ifdef KOXML_USE_QDOM
  // do nothing, QDom has no on-demand unloading
  Q_UNUSED( node );
#else
  node.unload();
#endif
}

int KoXml::childNodesCount( const KoXmlNode& node )
{
#ifdef KOXML_USE_QDOM
  return node.childNodes().count();
#else
  // compatibility function, because no need to implement
  // a class like QDomNodeList
  return node.childNodesCount();
#endif  
}

QDomNode KoXml::asQDomNode( QDomDocument ownerDoc, const KoXmlNode& node )
{
#ifdef KOXML_USE_QDOM
  Q_UNUSED( ownerDoc );
  return node;
#else
  return node.asQDomNode( ownerDoc );
#endif
}

QDomElement KoXml::asQDomElement( QDomDocument ownerDoc, const KoXmlElement& element )
{
  return KoXml::asQDomNode( ownerDoc, element ).toElement();
}

QDomDocument KoXml::asQDomDocument( QDomDocument ownerDoc, const KoXmlDocument& document )
{
  return KoXml::asQDomNode( ownerDoc, document ).toDocument();
}

bool KoXml::setDocument( KoXmlDocument& doc, QIODevice* device,
bool namespaceProcessing, QString* errorMsg, int* errorLine, 
int* errorColumn )
{
  QXmlSimpleReader reader;
  reader.setFeature(QLatin1String("http://xml.org/sax/features/namespaces"), namespaceProcessing);
  reader.setFeature(QLatin1String("http://xml.org/sax/features/namespace-prefixes"), !namespaceProcessing);
  reader.setFeature(QLatin1String("http://trolltech.com/xml/features/report-whitespace-only-CharData"), false);
  
  KoXmlInputSource* source = new KoXmlInputSource( device );
  bool result = doc.setContent( source, &reader, errorMsg, errorLine, errorColumn);
  delete source;
  return result;  
}

bool KoXml::setDocument( KoXmlDocument& doc, QIODevice* device,
QXmlSimpleReader* reader, QString* errorMsg, int* errorLine, int* errorColumn )
{
  KoXmlInputSource* source = new KoXmlInputSource( device );
  bool result = doc.setContent( source, reader, errorMsg, errorLine, errorColumn);
  delete source;
  return result;  
}

