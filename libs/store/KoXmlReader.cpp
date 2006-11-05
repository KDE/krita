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
#include <QDomElement>
#include <QMap>
#include <QVector>

// double QString, used for hashing against namespace and qualified name pair
class DQString
{
public:
  DQString() { }
  DQString( const QString& s1, const QString& s2 ){ str1 = s1; str2 = s2; }
  DQString( const DQString& b ) { str1 = b.str1; str2 = b.str2; }
  DQString& operator=( const DQString& b ){ str1 = b.str1; str2 = b.str2; return *this; }
  bool operator==( const DQString& b ) const { return (str1==b.str1) && (str2==b.str2); }
  bool operator!=( const DQString& b ) const { return (str1!=b.str1) || (str2!=b.str2); }
  bool operator<( const DQString& b ) const 
  { return ( str1 < b.str1 ) ? true : ( str1==b.str1 ) ? str2<b.str2 : false; }
  QString s1() const { return str1; }
  QString s2() const { return str2; }
private:
  QString str1, str2;  
};

// just for completeness, this is the self-test for DQString above
#if 0
  DQString b1;
  DQString b2;
  CHECK( b1==b2, true );
  CHECK( b1!=b2, false );

  b1 = DQString( "sweet","princess" );
  b2 = DQString( "sweet","princess" );
  CHECK( b1==b2, true );
  CHECK( b1!=b2, false );

  b1 = DQString( "sweet","princess" );
  b2 = DQString( "bad","prince" );
  CHECK( b1==b2, false );
  CHECK( b1!=b2, true );
#endif

class KoXmlStream
{
public:
  KoXmlStream(){ saveData = true; data.reserve(1024); pos = 0; }
  QString stringData() const { return data; }
  void setSaveData( bool s ){ saveData = s; }
  int at() const { return pos; }
  KoXmlStream& operator<<( const QString& str )
    { if(saveData) data.append(str); pos+=str.length(); return *this; }
  KoXmlStream& appendEscape( const QString& str );

private:
  bool saveData;
  QString data;
  int pos;
};

static QString escapeXML( const QString& str )
{
  QString result;
  
  for( int c=0; c<str.length(); c++ )
    switch( str[c].unicode() )
    {
      case '<': result.append( "&lt;"); break;
      case '>': result.append( "&gt;"); break;
      case '"': result.append( "&quot;"); break;
      case '&': result.append( "&amp;"); break;
      default: result.append( str[c] );
    }

  return result;
}


KoXmlStream& KoXmlStream::appendEscape( const QString& str )
{
  return operator<<( escapeXML(str) );

  unsigned len = str.length();

  if( saveData )
  {
    data.reserve( data.length() + len );
    for( unsigned c=0; c<len; c++ )
      if( str[c]=='<' ){ data.append( "&lt;"); pos += 4; } else
      if( str[c]=='>'){ data.append( "&gt;"); pos+= 4; } else
      if( str[c]=='"'){ data.append( "&quot;"); pos += 6; } else
      if( str[c]=='&'){ data.append( "&amp;"); pos += 5; } else
       { data.append( str[c] ); pos++; } 
  }
  else
  {
    pos += len;
    for( unsigned c=0; c<len; c++ )
      if( str[c]=='<' ) pos += 3; else // "&lt;"
      if( str[c]=='>') pos+= 3; else   // "&gt;"
      if( str[c]=='"') pos += 5; else  // "&quot;"
      if( str[c]=='&') pos += 4;  // "&amp;"
  }

  return *this;
}

// 40 bytes
class KoXmlPackedItem
{
public:
  bool attr:1;
  KoXmlNode::NodeType type:3;
  unsigned depth:12; // 4096 nested element should be enough
  QString nsURI;
  QString name;
  QString value;
  
  KoXmlPackedItem(): attr(false), type(KoXmlNode::NullNode),
    depth(0), name(0) {}
};

class KoXmlPackedDocument
{
public:
  bool processNamespace;
  QVector<KoXmlPackedItem> items;
  QVector<QString> names;
  QVector<QString> nsURIs;
  QVector<QString> values;

  void dump();
  KoXmlPackedDocument(): processNamespace(false){};
  
  ~KoXmlPackedDocument()
  {
    names.clear();
    nsURIs.clear();
    values.clear();
  }
};


class KoXmlNodeData
{
public:

  KoXmlNodeData();
  virtual ~KoXmlNodeData();

  // generic properties
  KoXmlNode::NodeType nodeType;
  QString tagName;
  QString namespaceURI;
  QString prefix;
  QString localName;

  // reference counting
  unsigned long count;
  void ref() { count++; }
  void unref() { --count; if( !count ) if( this != &null) delete this; }

  // type information
  bool emptyDocument;
  virtual const char* typeInfo() const { return "Node"; }
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
  virtual void clear();
  KoXmlNodeData* ownerDocument();

  // attributes
  void setAttribute( const QString& name, const QString& value );
  QString attribute( const QString& name );
  bool hasAttribute( const QString& name );
  void setAttributeNS( const QString& nsURI, const QString& name, const QString& value );
  QString attributeNS( const QString& nsURI, const QString& name );
  bool hasAttributeNS( const QString& nsURI, const QString& name );

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
  QMap<QString,QString> attr;  
  QMap<DQString,QString> attrNS;  
  QString textData;
  friend class KoXmlHandler;
};

class KoXmlHandler : public QXmlDefaultHandler
{
public:
  KoXmlHandler( KoXmlPackedDocument* doc, bool processNamespace );
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

protected:
  QString nameIndex( const QString& name );
  QString nsURIIndex( const QString& nsURI );
  QString valueIndex( const QString& value );
  void addItem( const KoXmlPackedItem& item );

private:
  bool processNamespace;
  QString entityName;
  bool cdata;
  int elementDepth;

  KoXmlPackedDocument* document;
  unsigned itemCount;
  QMap<QString,unsigned> nameHash;
  QMap<QString,unsigned> nsURIHash;
  QMap<QString,unsigned> valueHash;
};

// ==================================================================
//
//         KoXmlPackedDocument 
//
// ==================================================================

void KoXmlPackedDocument::dump()
{
  printf("Names:\n");
  for( int i=0; i<names.count(); i++ )
    printf("%5d: %s\n", i, qPrintable( names[i] ) );

  printf("\n");
  printf("Namespace URIs:\n");
  for( int i=0; i<nsURIs.count(); i++ )
    printf("%5d: %s\n", i, qPrintable( nsURIs[i] ) );

  printf("\n");
  printf("Values:\n");
  for( int i=0; i<values.count(); i++ )
    printf("%5d: %s\n", i, qPrintable( values[i] ) );

  printf("\n");
  printf("Nodes:\n");
  for( int i=0; i<items.count(); i++ )
  {
    KoXmlPackedItem& item = items[i];
    QString name = item.name;
    QString nsURI = item.nsURI;
    QString value = item.value;
    for( unsigned j=0; j<item.depth; j++ ) printf("  ");
    if( item.attr )
      printf("  %s = %s\n", qPrintable( name ), qPrintable( value ) );
    else
    {
      if( item.type == KoXmlNode::ElementNode )
        printf( "<%s>\n", qPrintable( name ) );
      else if( item.type == KoXmlNode::TextNode )
        printf( "%s\n", qPrintable( value ) );
      else if( item.type == KoXmlNode::CDATASectionNode )
        printf( "%s\n", qPrintable( value ) );
    }
  }
}


// ==================================================================
//
//         KoXmlNodeData 
//
// ==================================================================

KoXmlNodeData KoXmlNodeData::null;

KoXmlNodeData::KoXmlNodeData()
{
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
  tagName = QString();
  prefix = QString();
  namespaceURI = QString();
  textData = QString();
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
      if (!prefix.isEmpty()) n.prepend(":").prepend(prefix); break;
    case KoXmlNode::TextNode: return QString("#text");
    case KoXmlNode::CDATASectionNode: return QString("#cdata-section");
    case KoXmlNode::DocumentNode: return QString("#document");
    default: break;
  }

  return n;
}

KoXmlNodeData* KoXmlNodeData::ownerDocument()
{
  KoXmlNodeData* owner = this;

  while( owner->parent ) 
    owner = owner->parent;

  return (owner->nodeType==KoXmlNode::DocumentNode) ? owner : 0;
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

QString KoXmlNodeData::attribute( const QString& name )
{
  return attr[ name ];
}

bool KoXmlNodeData::hasAttribute( const QString& name )
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
  
  DQString key( nsURI, localName );
  attrNS[ key ] = value;
}

QString KoXmlNodeData::attributeNS( const QString& nsURI, const QString& name )
{
  DQString key( nsURI, name );
  return attrNS[ key ];
}

bool KoXmlNodeData::hasAttributeNS( const QString& nsURI, const QString& name )
{
  DQString key( nsURI, name );
  return attrNS.contains( key );
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

  KoXmlHandler handler( packedDoc, packedDoc->processNamespace );
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

void KoXmlNodeData::loadChildren( int depth )
{
  // sanity check
  if( !packedDoc ) return;

  // already loaded ?
  if( loaded && (depth<=1) ) return;
  
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
      QString name = item.name;
      QString nsURI = item.nsURI;
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
        QString name = item.name;
        QString nsURI = item.nsURI;
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

void KoXmlNodeData::unloadChildren()
{
  // sanity check
  if( !packedDoc ) return;

  if( !loaded ) return;
  
  attr.clear();
  attrNS.clear();

  if( first )
  for( KoXmlNodeData* node = first; node ; )
  {
    KoXmlNodeData* next = node->next;
    node->unloadChildren();
    node->unref();
    node = next;
  }

  loaded = false;
  first = last = 0;
}

static QDomNode itemAsQDomNode( QDomDocument ownerDoc, KoXmlPackedDocument* packedDoc,
unsigned long nodeIndex )
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
    
    QString name = item.name;
    QString nsURI = item.nsURI;
    
    if( packedDoc->processNamespace )
      element = ownerDoc.createElementNS( nsURI, item.name );
    else
      element = ownerDoc.createElement( item.name );

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
        QString name = item.name;
        QString nsURI = item.nsURI;
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
    // FIXME: choose CDATA when the value contains special characters
    QDomText textNode = ownerDoc.createTextNode( item.value );
    return textNode;
  }
          
  // nothing matches? strange...    
  return QDomNode();
}

QDomNode KoXmlNodeData::asQDomNode( QDomDocument ownerDoc ) const
{
  return itemAsQDomNode( ownerDoc, packedDoc, nodeIndex );
}

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
//         KoXmlHandler 
//
// ==================================================================

KoXmlHandler::KoXmlHandler( KoXmlPackedDocument* doc, bool ns ): 
QXmlDefaultHandler()
{
  processNamespace = ns;

  document = doc;

  cdata = false;
  entityName = QString();

  errorMsg = QString();
  errorLine = 0;
  errorColumn = 0;

  elementDepth = -1;
  
  itemCount = 0;
}

KoXmlHandler::~KoXmlHandler()
{
  nameHash.clear();
  nsURIHash.clear();
  valueHash.clear();
}

QString KoXmlHandler::nameIndex( const QString& name )
{
  const unsigned& ii = nameHash[name];
  if(ii > 0)
    return document->names[ii];

  // not yet declared, so we add it
  unsigned i = document->names.count();
  document->names.append( name );
  nameHash.insert( name, i );
  
  return document->names[i];
}

QString KoXmlHandler::nsURIIndex( const QString& nsURI )
{
  const unsigned& ii = nsURIHash[nsURI];
  if(ii > 0)
    return document->nsURIs[ii];

  // not yet declared, so we add it
  unsigned i = document->nsURIs.count();
  document->nsURIs.append( nsURI );
  nsURIHash.insert( nsURI, i );

  return document->nsURIs[i];
}

void KoXmlHandler::addItem( const KoXmlPackedItem& item )
{
  document->items.append(item);
  itemCount = document->items.count();
}

// Note: we cache only attribute value, not text/CDATA data !
QString KoXmlHandler::valueIndex( const QString& value )
{
  const unsigned& ii = valueHash[value];
  if(ii > 0)
    return document->values[ii];

  // not yet declared, so we add it
  unsigned i = document->values.count();
  document->values.append( value );
  valueHash.insert( value, i );

  return document->values[i];
}

bool KoXmlHandler::startDocument()
{
  // just for sanity
  cdata = false;
  entityName = QString();
  elementDepth = 0;
  nameHash.clear();
  nsURIHash.clear();
  
  // reset document
  document->names.clear();
  document->nsURIs.clear();
  document->values.clear();
  document->items.resize(0);
  itemCount = 0;

  // the first one is always empty
  // this is to reserve index 0
  document->names.append( "-" );
  document->nsURIs.append( "-" );
  document->values.append( "-" );
  
  // first node is root
  KoXmlPackedItem rootItem;
  rootItem.attr = false;
  rootItem.type = KoXmlNode::DocumentNode;
  rootItem.depth = 0;
  rootItem.nsURI = QString();
  rootItem.name = QString();
  rootItem.value = QString();
  addItem(rootItem);
  
  // should speed up initial allocations
  document->names.reserve( 1024 );
  document->nsURIs.reserve( 1024 );
  document->values.reserve( 1024 );

  // put some common attribute values and cache them as well
  valueIndex( QString::fromLatin1("Default" ) );
  valueIndex( QString::fromLatin1("true" ) );
  valueIndex( QString::fromLatin1("false" ) );
  valueIndex( QString::fromLatin1("string" ) );

  return true;
}

bool KoXmlHandler::endDocument()
{
  document->items.resize(itemCount);
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
  
  // we are going one level deeper
  elementDepth++;

  // add a new element
  KoXmlPackedItem item; 
  item.attr = false;
  item.type = KoXmlNode::ElementNode;
  item.depth = elementDepth;
  item.name = nameIndex( name );
  item.nsURI = nsURIIndex( nsURI );
  item.value = QString();
  addItem( item );

  // add all attributes
  for( int c=0; c<atts.length(); c++ )
  {
    // create a new attribute
    item.attr = true;
    item.type = KoXmlNode::NullNode;
    item.depth = elementDepth;
    item.name = nameIndex( atts.qName(c) );
    item.nsURI = nsURIIndex( atts.uri(c) );
    item.value = valueIndex( atts.value(c) );
    addItem( item );
  }

  return true;
}

bool KoXmlHandler::endElement( const QString& nsURI, const QString& localName, 
const QString& qName )
{
  Q_UNUSED( nsURI );
  Q_UNUSED( localName );
  Q_UNUSED( qName );
  
  // we are going up one level
  elementDepth--;

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

  // append characters a new (string) value
  // we do not cache this because unlikely it will be duplicated
  // too often (hence, it will incur cache-miss if we do it)
  document->values.append( str );

  // add a new text or CDATA 
  KoXmlPackedItem item; 
  item.attr = false;
  if( cdata )
    item.type = KoXmlNode::CDATASectionNode;
  else
    item.type = KoXmlNode::TextNode;
  item.depth = elementDepth + 1;
  item.name = QString();
  item.nsURI = QString();
  item.value = str;
  addItem( item );

  return true;
}

bool KoXmlHandler::processingInstruction( const QString& target, 
const QString& data )
{
  Q_UNUSED( target );
  Q_UNUSED( data );

  // add a new processing instruction
  KoXmlPackedItem item; 
  item.attr = false;
  item.type = KoXmlNode::ProcessingInstructionNode;
  item.depth = elementDepth + 1;
  item.name = QString();
  item.nsURI = QString();
  item.value = QString();
  addItem( item );

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
  entityName = QString();
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
  if( d ) d->unref();
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

bool KoXmlNode::isNull() const
{
  return d->nodeType == NullNode;
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
    
  d->loadChildren();

  return d->first!=0 ;
}

int KoXmlNode::childNodesCount() const
{
  if( isText() )
    return 0;
    
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
  d->loadChildren();
  return d->first ? KoXmlNode( d->first ) : KoXmlNode();
}

KoXmlNode KoXmlNode::lastChild() const
{
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
  return isElement() ? KoXmlElement( d ) : KoXmlElement( new KoXmlNodeData );
}

KoXmlText KoXmlNode::toText() const
{
  return isText() ? KoXmlText( d ) : KoXmlText();
}

KoXmlCDATASection KoXmlNode::toCDATASection() const
{
  return isCDATASection() ? KoXmlCDATASection( (KoXmlNodeData*)d ) :
    KoXmlCDATASection();
}

KoXmlDocument KoXmlNode::toDocument() const
{
  KoXmlNodeData* data = d;
  if( !isDocument() )
  {
    data = new KoXmlNodeData;
    data->emptyDocument = false;
  }
  
  return KoXmlDocument( data );
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
KoXmlElement::KoXmlElement(): KoXmlNode()
{
  d->unref();
  d = new KoXmlNodeData;
}

KoXmlElement::~KoXmlElement()
{
  d->unref();
  d = 0;
}

// Creates a shallow copy of another element
KoXmlElement::KoXmlElement( const KoXmlElement& element ): KoXmlNode()
{
  d->unref();
  d = element.d;
  d->ref();
}

KoXmlElement::KoXmlElement( KoXmlNodeData* data ): KoXmlNode()
{
  d->unref();
  d = data;
  d->ref();
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
  return attribute( name, QString() );
}

QString KoXmlElement::attribute( const QString& name, 
const QString& defaultValue ) const
{
  if( !isElement() )
    return defaultValue;
    
  d->loadChildren();

  if( !hasAttribute( name ) )  
    return defaultValue;

  return ((KoXmlNodeData*)d)->attribute( name );
}

QString KoXmlElement::attributeNS( const QString& namespaceURI, 
const QString& localName, const QString& defaultValue ) const
{
  if( !isElement() )
    return defaultValue;
    
  d->loadChildren();

  if( !hasAttributeNS( namespaceURI,localName ) )  
    return defaultValue;

  return ((KoXmlNodeData*)d)->attributeNS( namespaceURI,localName );
}

bool KoXmlElement::hasAttribute( const QString& name ) const
{
  d->loadChildren();
  return isElement() ? ((KoXmlNodeData*)d)->hasAttribute( name ) : false;
}

bool KoXmlElement::hasAttributeNS( const QString& namespaceURI, 
const QString& localName ) const
{
  d->loadChildren();
  return isElement() ? ((KoXmlNodeData*)d)->hasAttributeNS( 
    namespaceURI, localName ) : false;;
}

// ==================================================================
//
//         KoXmlText
//
// ==================================================================

KoXmlText::KoXmlText(): KoXmlNode()
{
  d->unref();
  d = new KoXmlNodeData;
  d->nodeType = TextNode;
}

KoXmlText::~KoXmlText()
{
  if( d ) d->unref();
  d = 0;
}

KoXmlText::KoXmlText( const KoXmlText& text ): KoXmlNode()
{
  d->unref();
  d = (KoXmlNodeData*) text.d;
  d->ref();
}

KoXmlText::KoXmlText( KoXmlNodeData* data ): KoXmlNode()
{
  d->unref();
  d = data;
  d->ref();
}

bool KoXmlText::isText() const
{
  return true;
}

QString KoXmlText::data() const
{
  return ((KoXmlNodeData*)d)->data();
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
  d->unref();
  d = new KoXmlNodeData;
  d->nodeType = KoXmlNode::CDATASectionNode;
}

KoXmlCDATASection::~KoXmlCDATASection()
{
  d->unref();
  d = 0;
}

KoXmlCDATASection::KoXmlCDATASection( const KoXmlCDATASection& cdata ):
KoXmlText()
{
  d->unref();
  d = (KoXmlNodeData*) cdata.d;
  d->ref();
}

KoXmlCDATASection::KoXmlCDATASection( KoXmlNodeData* cdata ):
KoXmlText()
{
  d->unref();
  d = cdata;
  d->ref();
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
  d->unref();
  d = 0;
}

KoXmlDocument::KoXmlDocument( KoXmlNodeData* data ): KoXmlNode()
{
  d->unref();
  d = data;
  d->ref();
  d->emptyDocument = true;
}

// Creates a copy of another document
KoXmlDocument::KoXmlDocument( const KoXmlDocument& doc ): KoXmlNode()
{
  d->unref();
  d = doc.d;
  d->ref();
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
    return QString("#document");
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

