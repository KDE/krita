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

/*
  This is a memory-efficient DOM implementation for KOffice. See the API 
  documentation for details.

  IMPORTANT !

  * When you change this stuff, make sure it DOES NOT BREAK the test suite.
    Build tests/koxmlreadertest.cpp and verify it. Many sleepless nights 
    have been sacrificed for this piece of code, do not let those precious 
    hours wasted!

  * Run testdom.cpp WITH Valgrind's memcheck tool and make sure NO illegal 
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

#ifndef KOXML_USE_QDOM

#include <QtXml>
#include <qdom.h>

#include <QMap>
#include <q3cstring.h>

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

KoXmlStream& KoXmlStream::appendEscape( const QString& str )
{
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
  void unref() { --count; if( !count ) delete this; }

  // type information
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

  // for document node
  QXmlSimpleReader* xmlReader;
  QString buffer;
  bool setContent( QXmlInputSource* source, QXmlReader* reader, 
    QString* errorMsg = 0, int* errorLine = 0, int* errorColumn = 0 );

  // used when doing on-demand (re)parse
  bool loaded;
  unsigned startPos, endPos;
  void loadChildren( int depth=1 );
  void unloadChildren();
  bool fastLoading;
  
private:
  QMap<QString,QString> attr;  
  QMap<DQString,QString> attrNS;  
  QString textData;
  friend class KoXmlHandler;
};

class KoXmlHandler : public QXmlDefaultHandler
{
public:
  KoXmlHandler( KoXmlNodeData*, bool processNamespace );
  ~KoXmlHandler();

  void setMaxDepth( int d ){ maxDepth = d; }
  void setInitialOffset( int ofs ){ parseOffset = ofs; }

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
  KoXmlNodeData* rootNode;
  KoXmlNodeData* currentNode;
  QString entityName;
  bool cdata;
  int parseOffset;
  KoXmlStream bufferStream;
  int elementDepth;
  int maxDepth;
};

// ==================================================================
//
//         KoXmlNodeData 
//
// ==================================================================

KoXmlNodeData::KoXmlNodeData()
{
  nodeType = KoXmlNode::NullNode;

  tagName = QString::null;
  prefix = QString::null;
  localName = QString::null;
  namespaceURI = QString::null;
  textData = QString::null;

  count = 1;
  parent = 0;
  prev = next = 0;
  first = last = 0;

  xmlReader = 0;
  startPos = endPos = 0;

  fastLoading = false;

  // assume true, it will be set to false by XML parser when this node
  // apparently has children AND the children are not loaded
  loaded = true;
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

  nodeType = KoXmlNode::NullNode;
  tagName = QString::null;
  prefix = QString::null;
  namespaceURI = QString::null;
  textData = QString::null;

  attr.clear();
  attrNS.clear();

  parent = 0;
  prev = next = 0;
  first = last = 0;

  delete xmlReader;
  xmlReader = 0;
  buffer = QString::null;
}

QString KoXmlNodeData::text()
{
  QString t( "" );

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
  int i = name.find( ':' );
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

  // copy the reader for later on-demand loading
  // FIXME this is a workaround because no copy is possible with QXmlReader
  char* features[] =
  { 
    "http://xml.org/sax/features/namespaces",
    "http://xml.org/sax/features/namespace-prefixes",
    "http://trolltech.com/xml/features/report-whitespace-only-CharData",
    "http://trolltech.com/xml/features/report-start-end-entity"
  };
  xmlReader = new QXmlSimpleReader;
  for( int fi=0; fi<4; fi++ )
    xmlReader->setFeature( features[fi], reader->feature( features[fi] ) );
  
  bool processNamespace = 
    reader->feature( "http://xml.org/sax/features/namespaces" ) && 
    !reader->feature( "http://xml.org/sax/features/namespace-prefixes" );

  KoXmlHandler handler( this, processNamespace );
  reader->setContentHandler( &handler );
  reader->setErrorHandler( &handler );
  reader->setLexicalHandler( &handler );
  reader->setDeclHandler( &handler );
  reader->setDTDHandler( &handler );

  if( !fastLoading )
    handler.setMaxDepth( 4 );

  if( !reader->parse( source ) ) 
  {
    // parsing error has occurred
    if( errorMsg ) *errorMsg = handler.errorMsg;
    if( errorLine ) *errorLine = handler.errorLine;
    if( errorColumn )  *errorColumn = handler.errorColumn;
    return false;
  }

  return true;
}

void KoXmlNodeData::loadChildren( int depth )
{
  // for more than 1 level, force reloading anyway
  if( ( depth== 1 ) && loaded ) return;

  KoXmlNodeData* doc = ownerDocument();
  if( !doc ) return;

  // fast loading? then this on-demand loading makes no sense
  if( doc->fastLoading ) return;

  unloadChildren();

  bool nsProcess = 
    doc->xmlReader->feature( "http://xml.org/sax/features/namespaces" ) && 
    !doc->xmlReader->feature( "http://xml.org/sax/features/namespace-prefixes" );


  // XML snippet for the children, including this element
  QString snippet = doc->buffer.mid( startPos, endPos-startPos+1 );

  // now parse all subnodes
  KoXmlHandler handler( this, nsProcess );
  handler.setMaxDepth( depth );
  handler.setInitialOffset( startPos );
  doc->xmlReader->setContentHandler( &handler );
  doc->xmlReader->setErrorHandler( &handler );
  doc->xmlReader->setLexicalHandler( &handler );
  doc->xmlReader->setDeclHandler( &handler );
  doc->xmlReader->setDTDHandler( &handler );

  QXmlInputSource source;
  source.setData( snippet );
  if( !doc->xmlReader->parse( source ) ) 
  {
    // parsing error has occurred, which should not happen
    // nothing we can do except...
    loaded = false;
    qWarning( "On-demand loading triggers parse error!" );
  }
  else
    loaded = true;
}

void KoXmlNodeData::unloadChildren()
{
  if( !loaded ) return;

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


// ==================================================================
//
//         KoXmlHandler 
//
// ==================================================================

KoXmlHandler::KoXmlHandler( KoXmlNodeData* n, bool ns ): 
QXmlDefaultHandler()
{
  processNamespace = ns;

  rootNode = n;
  currentNode = n;
  cdata = false;
  entityName = QString::null;

  errorMsg = QString::null;
  errorLine = 0;
  errorColumn = 0;

  parseOffset = 0;
  elementDepth = -1;
  maxDepth = 999;

  bufferStream.setSaveData( rootNode->nodeType == KoXmlNode::DocumentNode );
}

KoXmlHandler::~KoXmlHandler()
{
}

bool KoXmlHandler::startDocument()
{
  // just for sanity
  currentNode = rootNode;  
  cdata = false;
  entityName = QString::null;
  elementDepth = -1;

  return true;
}

bool KoXmlHandler::endDocument()
{
  // just for sanity
  if( rootNode->nodeType == KoXmlNode::DocumentNode )
    if( currentNode!=rootNode )
      return false; 

  if( rootNode->nodeType == KoXmlNode::DocumentNode )
    rootNode->buffer = bufferStream.stringData();

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

  // sanity check
  if( !currentNode ) 
    return false;

  // we are going one level deeper
  elementDepth++;

  QString nodePrefix, nodeLocalName, nodeTagName;
  KoXmlNodeData* element = 0;

  if( processNamespace )
  {
    // parse, using namespace
    nodeTagName = name;
    nodeLocalName = name;
    nodePrefix = nsURI.isNull() ? QString::null : QString("");
    int i = name.find( ':' );
    if( i != -1 )
    {
      nodeTagName = name.mid( i + 1 );
      nodeLocalName = nodeTagName;
      nodePrefix = name.left( i );
    }

    if( elementDepth <= maxDepth )
    {
      // construct a new element
      element = new KoXmlNodeData;
      element->nodeType = KoXmlNode::ElementNode;
      element->parent = currentNode;
      element->namespaceURI = nsURI;
      element->prefix = nodePrefix;
      element->localName = nodeLocalName;
      element->tagName = nodeTagName;

      // Note: endPos will be later fixed in endElement
      element->endPos = element->startPos = parseOffset + bufferStream.at();

      // handle the attributes
      for( int c=0; c<atts.length(); c++ )
      {
        QString prefix;
        QString qName; // with prefix
        QString name;  // without prefix, i.e. local name

        name = qName = atts.qName(c);
        int i = qName.find( ':' );
        if( i != -1 ) prefix = qName.left( i );
        if( i != -1 ) name = qName.mid( i + 1 );
        element->setAttributeNS( atts.uri(c), qName, atts.value(c) );
        element->setAttribute( name, atts.value(c) );
      }
    }

    // save in buffer for later on-demand loading
    if( ( rootNode->nodeType != KoXmlNode::DocumentNode ) || !rootNode->fastLoading )
    {
      bufferStream << "<";
      if( !nodePrefix.isEmpty() )
        bufferStream << nodePrefix << ":";
      bufferStream << localName;
      bufferStream << " xmlns";
      if( !nodePrefix.isEmpty() ) 
        bufferStream << ":" << nodePrefix;
      bufferStream << "=\"";
      bufferStream.appendEscape( nsURI );
      bufferStream << "\"";
      for( int c=0; c<atts.length(); c++ )
      {
        QString prefix;
        QString name = atts.qName(c);  // qName contains the prefix
        int i = name.find( ':' );
        if( i != -1 ) prefix = name.left( i );
        if( i != -1 ) name = atts.qName(c).mid( i + 1 );
        if( !atts.uri(c).isEmpty() )
          bufferStream << " xmlns:" << prefix << "=\"" << atts.uri(c) << "\"";
        bufferStream << " ";
        if( !prefix.isEmpty() ) bufferStream << prefix << ":";
        bufferStream << name << "=\"";
        bufferStream.appendEscape( atts.value(c) );
        bufferStream << "\"";
      }
      bufferStream << ">";
    }
  }
  else
  {
    // parse, without using namespace
    nodeTagName = name;

    if( elementDepth <= maxDepth )
    {
      // construct a new element
      element = new KoXmlNodeData;
      element->nodeType = KoXmlNode::ElementNode;
      element->parent = currentNode;
      element->namespaceURI = QString::null;
      element->prefix = QString::null;
      element->localName = QString::null;
      element->tagName = nodeTagName;

      if( rootNode->nodeType == KoXmlNode::DocumentNode ) 
        element->fastLoading = rootNode->fastLoading;
  
      // Note: endPos will be later fixed in endElement
      element->endPos = element->startPos = parseOffset + bufferStream.at();

      // handle the attributes
      for( int c=0; c<atts.length(); c++ )
        element->setAttribute( atts.qName(c), atts.value(c) );
    }

    // save in buffer for later on-demand loading
    if( ( rootNode->nodeType != KoXmlNode::DocumentNode ) || !rootNode->fastLoading )
    {
      bufferStream << "<";
      bufferStream << nodeTagName;
      for( int c=0; c<atts.length(); c++ )
      {
        bufferStream << " " << atts.qName(c) << "=\"";
        bufferStream.appendEscape( atts.value(c) );
        bufferStream << "\""; 
      }
      bufferStream << ">";
    }

  }

  // if we do not parse a complete document, the first element is ignored
  // this feature is used in on-demand loading
  // e.g. "<ul><li><b>bold items</b></li></ul>", if root node points to 
  // <ul> tag, then the first <ul> is skipped so that next <li> element
  // becomes the child of it.
  if( elementDepth == 0 )
    if( rootNode->nodeType != KoXmlNode::DocumentNode )
    {
      delete element;
      return true;
    }

  if( element )
  {
    // add as the child and traverse to it
    currentNode->loaded = true;
    currentNode->appendChild( element );
    currentNode = element;
  }
  else
    currentNode->loaded = false;

  return true;
}

bool KoXmlHandler::endElement( const QString& nsURI, const QString& localName, 
const QString& qName )
{
  Q_UNUSED( nsURI );
  Q_UNUSED( localName );
  
  // sanity check
  if( !currentNode ) return false;
  if( !currentNode->parent ) return false;

  // see comments in startElement about first element and on-demand loading
  if( rootNode->nodeType == KoXmlNode::DocumentNode )
    if( currentNode == rootNode ) 
      return false;

  // buffer for on-demand loading
  if( ( rootNode->nodeType != KoXmlNode::DocumentNode ) || !rootNode->fastLoading )
    bufferStream << "</" << qName << ">";

  // fix up the pointer
  currentNode->endPos = parseOffset + bufferStream.at() - 1;

  // go up one level
  if( elementDepth <= maxDepth )
    currentNode = currentNode->parent;

  // we are going up one level
  elementDepth--;

  return true;
}

bool KoXmlHandler::characters( const QString& str )
{
  // sanity check
  if( rootNode->nodeType == KoXmlNode::DocumentNode )
    if( currentNode == rootNode )
      return false;

  // are we inside entity ?
  if( !entityName.isEmpty() )
  {
    // we do not handle entity but need to keep track of it
    // because we want to skip it alltogether
    return true;
  }

  if( cdata )
  {
    // are we inside CDATA section ?
    if( elementDepth <= maxDepth )
    {
      KoXmlNodeData* cdata = new KoXmlNodeData;
      cdata->nodeType = KoXmlNode::CDATASectionNode;
      cdata->parent = currentNode;
      cdata->setData( str );
      currentNode->appendChild( cdata );
    }

    if( ( rootNode->nodeType != KoXmlNode::DocumentNode ) || !rootNode->fastLoading )
      bufferStream << "<![CDATA[" << str << "]]>"; // no escape for CDATA
  }
  else
  {
    // this must be normal text node
    if( elementDepth <= maxDepth )
    {
      KoXmlNodeData* text = new KoXmlNodeData;
      text->nodeType = KoXmlNode::TextNode;
      text->parent = currentNode;
      text->setData( str );
      currentNode->appendChild( text );
    }
  
    // save it for later use  
    if( ( rootNode->nodeType != KoXmlNode::DocumentNode ) || !rootNode->fastLoading )
      bufferStream.appendEscape( str );
  }

  return true;
}

bool KoXmlHandler::processingInstruction( const QString& target, 
const QString& data )
{
  // sanity check
  if( !currentNode ) 
    return false;
  
  KoXmlNodeData* instruction = new KoXmlNodeData;
  instruction->nodeType = KoXmlNode::ProcessingInstructionNode;
  instruction->parent = currentNode;
  instruction->tagName = target;
  instruction->setData( data );

  currentNode->appendChild( instruction );

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
  entityName = QString::null;
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
  d = new KoXmlNodeData;
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
  return isElement() ? d->prefix : QString::null;
}

QString KoXmlNode::namespaceURI() const
{
  return isElement() ? d->namespaceURI : QString::null;
}

QString KoXmlNode::localName() const
{
  return isElement() ? d->localName : QString::null;
}

KoXmlDocument KoXmlNode::ownerDocument() const
{
  KoXmlNodeData* node = d; 
  while( node->parent ) node = node->parent;

  if( node->nodeType != DocumentNode ) return KoXmlDocument();  
  return KoXmlDocument( node );
}

KoXmlNode KoXmlNode::parentNode() const
{
  return d->parent ? KoXmlNode( d->parent ) : KoXmlNode();
}

bool KoXmlNode::hasChildNodes() const
{
  d->loadChildren();
  return d->first!=0 ;
}

KoXmlNode KoXmlNode::firstChild() const
{
  if( !d->fastLoading ) 
    d->loadChildren();
  return d->first ? KoXmlNode( d->first ) : KoXmlNode();
}

KoXmlNode KoXmlNode::lastChild() const
{
  if( !d->fastLoading ) 
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
  if( !d->fastLoading ) 
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
  if( !d->fastLoading ) 
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

KoXmlElement KoXmlNode::toElement()
{
  return isElement() ? KoXmlElement( d ) : KoXmlElement();
}

KoXmlText KoXmlNode::toText()
{
  return isText() ? KoXmlText( d ) : KoXmlText();
}

KoXmlCDATASection KoXmlNode::toCDATASection()
{
  return isCDATASection() ? KoXmlCDATASection( (KoXmlNodeData*)d ) :
    KoXmlCDATASection();
}

KoXmlDocument KoXmlNode::toDocument()
{
  return isDocument() ? KoXmlDocument( d ) : KoXmlDocument();
}

void KoXmlNode::load( int depth )
{
  d->loadChildren( depth );
}

void KoXmlNode::unload()
{
  d->unloadChildren();
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
  return isElement() ? ((KoXmlNodeData*)d)->tagName: QString::null;
}

QString KoXmlElement::text() const
{
  return d->text();
}

bool KoXmlElement::isElement() const
{
  return true;
}

QString KoXmlElement::attribute( const QString& name ) const
{
  return attribute( name, QString::null );
}

QString KoXmlElement::attribute( const QString& name, 
const QString& defaultValue ) const
{
  if( !isElement() )
    return defaultValue;
    
  if( !hasAttribute( name ) )  
    return defaultValue;

  return ((KoXmlNodeData*)d)->attribute( name );
}

QString KoXmlElement::attributeNS( const QString& namespaceURI, 
const QString& localName, const QString& defaultValue ) const
{
  if( !isElement() )
    return defaultValue;
    
  if( !hasAttributeNS( namespaceURI,localName ) )  
    return defaultValue;

  return ((KoXmlNodeData*)d)->attributeNS( namespaceURI,localName );
}

bool KoXmlElement::hasAttribute( const QString& name ) const
{
  return isElement() ? ((KoXmlNodeData*)d)->hasAttribute( name ) : false;
}

bool KoXmlElement::hasAttributeNS( const QString& namespaceURI, 
const QString& localName ) const
{
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
  d->unref();
  d = new KoXmlNodeData;
  d->nodeType = KoXmlNode::DocumentNode;
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

bool KoXmlDocument::isDocument() const
{
  return true;
}

KoXmlElement KoXmlDocument::documentElement() const
{
  for( KoXmlNodeData* node=d->first; node; )
    if( node->nodeType==KoXmlNode::ElementNode )
      return KoXmlElement( node );
    else node = node->next;  

  return KoXmlElement();
}

void KoXmlDocument::setFastLoading( bool f )
{
  d->fastLoading = f;
}

bool KoXmlDocument::fastLoading() const
{
  return d->fastLoading;
}

bool KoXmlDocument::setContent( QXmlInputSource *source, QXmlReader *reader, 
    QString* errorMsg, int* errorLine, int* errorColumn )
{
  if( d->nodeType != KoXmlNode::DocumentNode ) 
    return false;

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
    return false;

  QXmlSimpleReader reader;
  reader.setFeature( "http://xml.org/sax/features/namespaces", namespaceProcessing );
  reader.setFeature( "http://xml.org/sax/features/namespace-prefixes", !namespaceProcessing );
  reader.setFeature( "http://trolltech.com/xml/features/report-whitespace-only-CharData", false );

  // FIXME this hack is apparently private
  //reader.setUndefEntityInAttrHack(true);

  QXmlInputSource source( device );
  return d->setContent( &source, &reader, errorMsg, errorLine, errorColumn );
}

#endif

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
