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
*/

//  xmlreadertest.cpp - test KoXml classes
//  Ariya Hidayat, November 2005

#include <QString>
#include <q3cstring.h>
#include <qbuffer.h>
#include <qtextstream.h>
#include <qdatetime.h>
#include <QFile>

#include "KoXmlReader.h"
#include <qxml.h>

#define CHECK(x,y)  check(__FILE__,__LINE__,#x,x,y)

static int testCount = 0;
static int testFailed = 0;

template<typename T>
void check( const char *file, int line, const char* msg, 
const T& result, const T& expected )
{
  testCount++;
  if( result != expected )
  {
    testFailed++;
    QString message;
    QTextStream ts( &message, QIODevice::WriteOnly );
    ts << msg;
    ts << "  Result:";
    ts << result;
    ts << ", ";
    ts << "Expected:";
    ts << expected;
    printf( "%s [%d]: %s\n", file, line, message.latin1() );
  }
}

void check( const char *file, int line, const char* msg, bool result, 
bool expected )
{
  testCount++;
  if( result != expected )
  {
    testFailed++;
    QString message;
    QTextStream ts( &message, QIODevice::WriteOnly );
    ts << msg;
    ts << "  Result: ";
    if( result ) ts << "True"; else ts << "False";
    ts << ", ";
    ts << "Expected: ";
    if( expected ) ts << "True"; else ts << "False";
    printf( "%s [%d]: %s\n", file, line, message.latin1() );
  }
}

void testNode()
{
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;

  QByteArray xmlbuf;
  QBuffer xmldevice( xmlbuf );
  QTextStream xmlstream( xmlbuf, QIODevice::WriteOnly );
  xmlstream << "<earth>";
  xmlstream << "<continents>";
  xmlstream << "<asia/>";
  xmlstream << "<africa/>";
  xmlstream << "<europe/>";
  xmlstream << "<america/>";
  xmlstream << "<australia/>";
  xmlstream << "<antartic/>";
  xmlstream << "</continents>";
  xmlstream << "<oceans>";
  xmlstream << "<pacific/>";
  xmlstream << "<atlantic/>";
  xmlstream << "</oceans>";
  xmlstream << "</earth>";

  KoXmlDocument doc;
  CHECK( doc.setContent( &xmldevice, &errorMsg, &errorLine, &errorColumn ), true );
  CHECK( errorMsg.isEmpty(), true );
  CHECK( errorLine, 0 );
  CHECK( errorColumn, 0 );

  // null node
  KoXmlNode node1;
  CHECK( node1.nodeName(), QString::null );
  CHECK( node1.isNull(), true );
  CHECK( node1.isElement(), false );
  CHECK( node1.isElement(), false );
  CHECK( node1.isDocument(), false );
  CHECK( node1.ownerDocument().isNull(), false );
  CHECK( node1.parentNode().isNull(), true );
  CHECK( node1.hasChildNodes(), false );
  CHECK( node1.firstChild().isNull(), true );
  CHECK( node1.lastChild().isNull(), true );
  CHECK( node1.previousSibling().isNull(), true );
  CHECK( node1.nextSibling().isNull(), true );

  // compare with another null node
  KoXmlNode node2;
  CHECK( node2.isNull(), true );
  CHECK( node1==node2, true );
  CHECK( node1!=node2, false );

  // a node which is a document
  KoXmlNode node3 = doc;
  CHECK( node3.nodeName(), QString("#document") );
  CHECK( node3.isNull(), false );
  CHECK( node3.isElement(), false );
  CHECK( node3.isText(), false );
  CHECK( node3.isDocument(), true );
  CHECK( node3.ownerDocument().isNull(), false );
  CHECK( node3.ownerDocument()==doc, true );
  CHECK( node3.toDocument()==doc, true );

  // convert to document and the compare
  KoXmlDocument doc2 = node3.toDocument();
  CHECK( doc2.nodeName(), QString("#document") );
  CHECK( doc2.isNull(), false );
  CHECK( doc2.isDocument(), true );
  CHECK( node3==doc2, true );

  // a document is of course can't be converted to element
  KoXmlElement invalidElement = node3.toElement();
  CHECK( invalidElement.nodeName(), QString::null );
  CHECK( invalidElement.isNull(), true );
  CHECK( invalidElement.isElement(), true );
  CHECK( invalidElement.isText(), false );
  CHECK( invalidElement.isDocument(), false );

  // clear() makes it a null node again
  node3.clear();
  CHECK( node3.isNull(), true );
  CHECK( node3.nodeName(), QString::null );
  CHECK( node3.isElement(), false );
  CHECK( node3.isText(), false );
  CHECK( node3.isDocument(), false );
  CHECK( node3.ownerDocument().isNull(), false );
  CHECK( node1==node3, true );
  CHECK( node1!=node3, false );

  // a node which is an element for <earth>
  KoXmlNode node4 = doc.firstChild();
  CHECK( node4.isNull(), false );
  CHECK( node4.isElement(), true );
  CHECK( node4.isText(), false );
  CHECK( node4.isDocument(), false );
  CHECK( node4.hasChildNodes(), true );
  CHECK( node4.ownerDocument()==doc, true );
  CHECK( node4.toElement()==doc.firstChild().toElement(), true );

  // clear() makes it a null node again
  node4.clear();
  CHECK( node4.isNull(), true );
  CHECK( node4.isElement(), false );
  CHECK( node4.isText(), false );
  CHECK( node4.isDocument(), false );
  CHECK( node4==node1, true );
  CHECK( node4!=node1, false );

  // a node which is an element for <continents>
  KoXmlNode node5 = doc.firstChild().firstChild();
  CHECK( node5.nodeName(), QString("continents") );
  CHECK( node5.isNull(), false );
  CHECK( node5.isElement(), true );
  CHECK( node5.isText(), false );
  CHECK( node5.isDocument(), false );
  CHECK( node5.hasChildNodes(), true );
  CHECK( node5.ownerDocument()==doc, true );

  // convert to element and the compare
  KoXmlElement continentsElement = node5.toElement();
  CHECK( node5==continentsElement, true );
  CHECK( continentsElement.isNull(), false );
  CHECK( continentsElement.isElement(), true );
  CHECK( continentsElement.isText(), false );
  CHECK( continentsElement.hasChildNodes(), true );
  CHECK( continentsElement.ownerDocument()==doc, true );

  // and it doesn't make sense to convert that node to document
  // (instead a brand new document is created, i.e. not a null node)
  KoXmlDocument invalidDoc = node5.toDocument();
  CHECK( invalidDoc.isNull(), false );
  CHECK( invalidDoc.isElement(), false );
  CHECK( invalidDoc.isText(), false );
  CHECK( invalidDoc.isDocument(), true );

  // node for <europe> using namedItem() function
  KoXmlNode europeNode = continentsElement.namedItem( QString("europe") );
  CHECK( europeNode.nodeName(), QString("europe") );
  CHECK( europeNode.isNull(), false );
  CHECK( europeNode.isElement(), true );
  CHECK( europeNode.isText(), false );
  CHECK( europeNode.hasChildNodes(), false );
  CHECK( europeNode.ownerDocument()==doc, true );

  // search non-existing node
  KoXmlNode fooNode = continentsElement.namedItem( QString("foobar") );
  CHECK( fooNode.isNull(), true );
  CHECK( fooNode.isElement(), false );
  CHECK( fooNode.isText(), false );
  CHECK( fooNode.isCDATASection(), false );
}

void testElement()
{
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;

  QByteArray xmlbuf;
  QBuffer xmldevice( xmlbuf );

  QTextStream xmlstream( xmlbuf, QIODevice::WriteOnly );
  xmlstream << "<html>";
  xmlstream << "<body bgcolor=\"#000\">";
  xmlstream << "<p>";
  xmlstream << "Hello, world!";
  xmlstream << "</p>";
  xmlstream << "</body>";
  xmlstream << "</html>";

  KoXmlDocument doc;
  CHECK( doc.setContent( &xmldevice, &errorMsg, &errorLine, &errorColumn ), true );
  CHECK( errorMsg.isEmpty(), true );
  CHECK( errorLine, 0 );
  CHECK( errorColumn, 0 );

  // element for <html> 
  KoXmlElement rootElement;
  rootElement = doc.documentElement();
  CHECK( rootElement.nodeName(), QString("html") );
  CHECK( rootElement.isNull(), false );
  CHECK( rootElement.isElement(), true );
  CHECK( rootElement.isDocument(), false );
  CHECK( rootElement.ownerDocument().isNull(), false );
  CHECK( rootElement.ownerDocument()==doc, true );
  CHECK( rootElement.parentNode().isNull(), false );
  CHECK( rootElement.parentNode().toDocument()==doc, true );
  CHECK( rootElement.hasChildNodes(), true );
  CHECK( rootElement.tagName(), QString("html") );
  CHECK( rootElement.prefix().isNull(), true );
 
  // element for <body> 
  KoXmlElement bodyElement;
  bodyElement = rootElement.firstChild().toElement();
  CHECK( bodyElement.nodeName(), QString("body") );
  CHECK( bodyElement.isNull(), false );
  CHECK( bodyElement.isElement(), true );
  CHECK( bodyElement.isDocument(), false );
  CHECK( bodyElement.ownerDocument().isNull(), false );
  CHECK( bodyElement.ownerDocument()==doc, true );
  CHECK( bodyElement.parentNode().isNull(), false );
  CHECK( bodyElement.parentNode()==rootElement, true );
  CHECK( bodyElement.hasChildNodes(), true );
  CHECK( bodyElement.tagName(), QString("body") );
  CHECK( bodyElement.prefix().isNull(), true );
  CHECK( bodyElement.hasAttribute("bgcolor"), true );
  CHECK( bodyElement.attribute("bgcolor"), QString("#000") );

  // a shared copy of <body>, will still have access to attribute bgcolor
  KoXmlElement body2Element;
  body2Element = bodyElement;
  CHECK( body2Element.nodeName(), QString("body") );
  CHECK( body2Element.isNull(), false );
  CHECK( body2Element.isElement(), true );
  CHECK( body2Element.isDocument(), false );
  CHECK( body2Element.ownerDocument().isNull(), false );
  CHECK( body2Element.ownerDocument()==doc, true );
  CHECK( body2Element==bodyElement, true );
  CHECK( body2Element!=bodyElement, false );
  CHECK( body2Element.hasChildNodes(), true );
  CHECK( body2Element.tagName(), QString("body") );
  CHECK( body2Element.prefix().isNull(), true );
  CHECK( body2Element.hasAttribute("bgcolor"), true );
  CHECK( body2Element.attribute("bgcolor"), QString("#000") );

  // empty element, by default constructor
  KoXmlElement testElement; 
  CHECK( testElement.nodeName(), QString::null );
  CHECK( testElement.isNull(), true );
  CHECK( testElement.isElement(), true );
  CHECK( testElement.isDocument(), false );
  CHECK( testElement.ownerDocument().isNull(), false );
  CHECK( testElement.ownerDocument()!=doc, true );
  CHECK( testElement==rootElement, false );
  CHECK( testElement!=rootElement, true );
  CHECK( testElement.parentNode().isNull(), true );
  CHECK( testElement.hasChildNodes(), false );

  // check assignment operator
  testElement = rootElement;
  CHECK( testElement.nodeName(), QString("html") );
  CHECK( testElement.isNull(), false );
  CHECK( testElement.isElement(), true );
  CHECK( testElement.isDocument(), false );
  CHECK( testElement==rootElement, true );
  CHECK( testElement!=rootElement, false );
  CHECK( testElement.parentNode().isNull(), false );
  CHECK( testElement.parentNode().toDocument()==doc, true );
  CHECK( testElement.tagName(), QString("html") );
  CHECK( testElement.prefix().isNull(), true );

  // assigned from another empty element
  testElement = KoXmlElement();
  CHECK( testElement.isNull(), true );
  CHECK( testElement!=rootElement, true );

  // assigned from <body>
  testElement = bodyElement;
  CHECK( testElement.isNull(), false );
  CHECK( testElement.isElement(), true );
  CHECK( testElement.isDocument(), false );
  CHECK( testElement.ownerDocument().isNull(), false );
  CHECK( testElement.ownerDocument()==doc, true );
  CHECK( testElement==bodyElement, true );
  CHECK( testElement.parentNode().isNull(), false );
  CHECK( testElement.tagName(), QString("body") );
  CHECK( testElement.prefix().isNull(), true );
  CHECK( testElement.hasChildNodes(), true );

  // copy constructor  
  KoXmlElement dummyElement( rootElement ); 
  CHECK( dummyElement.isNull(), false );
  CHECK( dummyElement.isElement(), true );
  CHECK( dummyElement.isDocument(), false );
  CHECK( dummyElement.ownerDocument().isNull(), false );
  CHECK( dummyElement.ownerDocument()==doc, true );
  CHECK( dummyElement==rootElement, true );
  CHECK( dummyElement.parentNode().isNull(), false );
  CHECK( dummyElement.hasChildNodes(), true );
  CHECK( dummyElement.tagName(), QString("html") );
  CHECK( dummyElement.prefix().isNull(), true );

  // clear() turns element to null node
  dummyElement.clear();
  CHECK( dummyElement.isNull(), true );
  CHECK( dummyElement.isElement(), true );
  CHECK( dummyElement.isDocument(), false );
  CHECK( dummyElement.ownerDocument().isNull(), false );
  CHECK( dummyElement.ownerDocument()==doc, false );
  CHECK( dummyElement.hasChildNodes(), false );
  CHECK( dummyElement==rootElement, false );
  CHECK( dummyElement!=rootElement, true );
  
  // check for plain null node converted to element
  KoXmlNode dummyNode;
  dummyElement = dummyNode.toElement();
  CHECK( dummyElement.isNull(), true );
  CHECK( dummyElement.isElement(), true );
  CHECK( dummyElement.isDocument(), false );
  CHECK( dummyElement.ownerDocument().isNull(), false );
  CHECK( dummyElement.hasChildNodes(), false );
  CHECK( dummyElement.ownerDocument()==doc, false );
}

void testAttributes()
{
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;

  QByteArray xmlbuf;
  QBuffer xmldevice( xmlbuf );

  QTextStream xmlstream( xmlbuf, QIODevice::WriteOnly );
  xmlstream << "<p>";
  xmlstream << "<img src=\"foo.png\" width=\"300\" height=\"150\"/>";
  xmlstream << "</p>";

  KoXmlDocument doc;
  CHECK( doc.setContent( &xmldevice, &errorMsg, &errorLine, &errorColumn ), true );
  CHECK( errorMsg.isEmpty(), true );
  CHECK( errorLine, 0 );
  CHECK( errorColumn, 0 );
  
  KoXmlElement rootElement;
  rootElement = doc.documentElement();
  CHECK( rootElement.isNull(), false );
  CHECK( rootElement.isElement(), true );
  CHECK( rootElement.parentNode().isNull(), false );
  CHECK( rootElement.parentNode().toDocument()==doc, true );
  CHECK( rootElement.tagName(), QString("p") );
  CHECK( rootElement.prefix().isNull(), true );

  KoXmlElement imgElement;
  imgElement = rootElement.firstChild().toElement();
  CHECK( imgElement.isNull(), false );
  CHECK( imgElement.isElement(), true );
  CHECK( imgElement.tagName(), QString("img") );
  CHECK( imgElement.prefix().isNull(), true );
  CHECK( imgElement.hasAttribute("src"), true );
  CHECK( imgElement.hasAttribute("width"), true );
  CHECK( imgElement.hasAttribute("height"), true );
  CHECK( imgElement.hasAttribute("non-exist"), false );
  CHECK( imgElement.hasAttribute("SRC"), false );
  CHECK( imgElement.attribute("src"), QString("foo.png") );
  CHECK( imgElement.attribute("width"), QString("300") );
  CHECK( imgElement.attribute("width").toInt(), 300 );
  CHECK( imgElement.attribute("height"), QString("150") );
  CHECK( imgElement.attribute("height").toInt(), 150 );
  CHECK( imgElement.attribute("border").isEmpty(), true );
  CHECK( imgElement.attribute("border","0").toInt(), 0 );
  CHECK( imgElement.attribute("border","-1").toInt(), -1 );
}

void testText()
{
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;

  QByteArray xmlbuf;
  QBuffer xmldevice( xmlbuf );

  QTextStream xmlstream( xmlbuf, QIODevice::WriteOnly );
  xmlstream << "<p>";
  xmlstream << "Hello ";
  xmlstream << "<b>world</b>";
  xmlstream << "</p>";

  KoXmlDocument doc;
  CHECK( doc.setContent( &xmldevice, &errorMsg, &errorLine, &errorColumn ), true );
  CHECK( errorMsg.isEmpty(), true );
  CHECK( errorLine, 0 );
  CHECK( errorColumn, 0 );

  // element for <p> 
  KoXmlElement parElement;
  parElement = doc.documentElement();
  CHECK( parElement.isNull(), false );
  CHECK( parElement.isElement(), true );
  CHECK( parElement.isText(), false );
  CHECK( parElement.isDocument(), false );
  CHECK( parElement.ownerDocument().isNull(), false );
  CHECK( parElement.ownerDocument()==doc, true );
  CHECK( parElement.parentNode().isNull(), false );
  CHECK( parElement.parentNode().toDocument()==doc, true );
  CHECK( parElement.hasChildNodes(), true );
  CHECK( parElement.tagName(), QString("p") );
  CHECK( parElement.prefix().isNull(), true );
  CHECK( parElement.text(), QString("Hello world") );

  // node for "Hello"
  KoXmlNode helloNode;
  helloNode = parElement.firstChild();
  CHECK( helloNode.nodeName(), QString("#text") );
  CHECK( helloNode.isNull(), false );
  CHECK( helloNode.isElement(), false );
  CHECK( helloNode.isText(), true );
  CHECK( helloNode.isDocument(), false );

  // "Hello" text
  KoXmlText helloText;
  helloText = helloNode.toText();
  CHECK( helloText.nodeName(), QString("#text") );
  CHECK( helloText.isNull(), false );
  CHECK( helloText.isElement(), false );
  CHECK( helloText.isText(), true );
  CHECK( helloText.isDocument(), false );
  CHECK( helloText.data(), QString("Hello ") );

  // shared copy of the text
  KoXmlText hello2Text;
  hello2Text = helloText;
  CHECK( hello2Text.isNull(), false );
  CHECK( hello2Text.isElement(), false );
  CHECK( hello2Text.isText(), true );
  CHECK( hello2Text.isDocument(), false );
  CHECK( hello2Text.data(), QString("Hello ") );

  // element for <b>
  KoXmlElement boldElement;
  boldElement = helloNode.nextSibling().toElement();
  CHECK( boldElement.isNull(), false );
  CHECK( boldElement.isElement(), true );
  CHECK( boldElement.isText(), false );
  CHECK( boldElement.isDocument(), false );
  CHECK( boldElement.ownerDocument().isNull(), false );
  CHECK( boldElement.ownerDocument()==doc, true );
  CHECK( boldElement.parentNode().isNull(), false );
  CHECK( boldElement.hasChildNodes(), true );
  CHECK( boldElement.tagName(), QString("b") );
  CHECK( boldElement.prefix().isNull(), true );

  // "world" text
  KoXmlText worldText;
  worldText = boldElement.firstChild().toText();
  CHECK( worldText.isNull(), false );
  CHECK( worldText.isElement(), false );
  CHECK( worldText.isText(), true );
  CHECK( worldText.isDocument(), false );
  CHECK( worldText.data(), QString("world") );
}

void testCDATA()
{
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;

  QByteArray xmlbuf;
  QBuffer xmldevice( xmlbuf );

  QTextStream xmlstream( xmlbuf, QIODevice::WriteOnly );
  xmlstream << "<p>";
  xmlstream << "Hello ";
  xmlstream << "<![CDATA[world]]>";
  xmlstream << "</p>";

  KoXmlDocument doc;
  CHECK( doc.setContent( &xmldevice, &errorMsg, &errorLine, &errorColumn ), true );
  CHECK( errorMsg.isEmpty(), true );
  CHECK( errorLine, 0 );
  CHECK( errorColumn, 0 );

  // element for <p> 
  KoXmlElement parElement;
  parElement = doc.documentElement();
  CHECK( parElement.isNull(), false );
  CHECK( parElement.isElement(), true );
  CHECK( parElement.isText(), false );
  CHECK( parElement.isDocument(), false );
  CHECK( parElement.ownerDocument().isNull(), false );
  CHECK( parElement.ownerDocument()==doc, true );
  CHECK( parElement.parentNode().isNull(), false );
  CHECK( parElement.parentNode().toDocument()==doc, true );
  CHECK( parElement.hasChildNodes(), true );
  CHECK( parElement.tagName(), QString("p") );
  CHECK( parElement.prefix().isNull(), true );
  CHECK( parElement.text(), QString("Hello world") );

  // node for "Hello"
  KoXmlNode helloNode;
  helloNode = parElement.firstChild();
  CHECK( helloNode.isNull(), false );
  CHECK( helloNode.isElement(), false );
  CHECK( helloNode.isText(), true );
  CHECK( helloNode.isDocument(), false );

  // "Hello" text
  KoXmlText helloText;
  helloText = helloNode.toText();
  CHECK( helloText.isNull(), false );
  CHECK( helloText.isElement(), false );
  CHECK( helloText.isText(), true );
  CHECK( helloText.isDocument(), false );
  CHECK( helloText.data(), QString("Hello ") );

  // node for CDATA "world!"
  // Note: isText() is also true for CDATA
  KoXmlNode worldNode;
  worldNode = helloNode.nextSibling();
  CHECK( worldNode.nodeName(), QString("#cdata-section") );
  CHECK( worldNode.isNull(), false );
  CHECK( worldNode.isElement(), false );
  CHECK( worldNode.isText(), true );
  CHECK( worldNode.isCDATASection(), true );
  CHECK( worldNode.isDocument(), false );

  // CDATA section for "world!"
  // Note: isText() is also true for CDATA
  KoXmlCDATASection worldCDATA;
  worldCDATA = worldNode.toCDATASection();
  CHECK( worldCDATA.nodeName(), QString("#cdata-section") );
  CHECK( worldCDATA.isNull(), false );
  CHECK( worldCDATA.isElement(), false );
  CHECK( worldCDATA.isText(), true );
  CHECK( worldCDATA.isCDATASection(), true );
  CHECK( worldCDATA.isDocument(), false );
  CHECK( worldCDATA.data(), QString("world") );
}

void testDocument()
{
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;

  QByteArray xmlbuf;
  QBuffer xmldevice( xmlbuf );

  QTextStream xmlstream( xmlbuf, QIODevice::WriteOnly );
  xmlstream << "<koffice>";
  xmlstream << "  <kword/>\n";
  xmlstream << "  <kpresenter/>\n";
  xmlstream << "  <krita/>\n";
  xmlstream << "</koffice>";

  KoXmlDocument doc;

  // empty document
  CHECK( doc.nodeName(), QString("#document") );
  CHECK( doc.isNull(), false );
  CHECK( doc.isElement(), false );
  CHECK( doc.isDocument(), true );
  CHECK( doc.parentNode().isNull(), true );
  CHECK( doc.firstChild().isNull(), true );
  CHECK( doc.lastChild().isNull(), true );
  CHECK( doc.previousSibling().isNull(), true );
  CHECK( doc.nextSibling().isNull(), true );

  // now give something as the content
  CHECK( doc.setContent(&xmldevice,&errorMsg,&errorLine,&errorColumn ), true );
  CHECK( errorMsg.isEmpty(), true );
  CHECK( errorLine, 0 );
  CHECK( errorColumn, 0 );

  // this document has something already
  CHECK( doc.nodeName(), QString("#document") );
  CHECK( doc.isNull(), false );
  CHECK( doc.isElement(), false );
  CHECK( doc.isDocument(), true );
  CHECK( doc.parentNode().isNull(), true );
  CHECK( doc.firstChild().isNull(), false );
  CHECK( doc.lastChild().isNull(), false );
  CHECK( doc.previousSibling().isNull(), true );
  CHECK( doc.nextSibling().isNull(), true );

  // make sure its children are fine
  KoXmlElement rootElement;
  rootElement = doc.firstChild().toElement();
  CHECK( rootElement.isNull(), false );
  CHECK( rootElement.isElement(), true );
  CHECK( rootElement.isDocument(), false );
  CHECK( rootElement.parentNode().isNull(), false );
  CHECK( rootElement.parentNode().toDocument()==doc, true );
  rootElement = doc.lastChild().toElement();
  CHECK( rootElement.isNull(), false );
  CHECK( rootElement.isElement(), true );
  CHECK( rootElement.isDocument(), false );
  CHECK( rootElement.parentNode().isNull(), false );
  CHECK( rootElement.parentNode().toDocument()==doc, true );

  // clear() converts it into null node
  doc.clear();
  CHECK( doc.nodeName(), QString::null );
  CHECK( doc.isNull(), true );
  CHECK( doc.isElement(), false );
  CHECK( doc.isDocument(), true );
  CHECK( doc.parentNode().isNull(), true );
  CHECK( doc.firstChild().isNull(), true );
  CHECK( doc.lastChild().isNull(), true );
  CHECK( doc.previousSibling().isNull(), true );
  CHECK( doc.nextSibling().isNull(), true );

  // assigned from another empty document
  doc = KoXmlDocument();
  CHECK( doc.nodeName(), QString("#document") );
  CHECK( doc.isNull(), false );
  CHECK( doc.isElement(), false );
  CHECK( doc.isDocument(), true );
  CHECK( doc.parentNode().isNull(), true );
}

void testNamespace()
{
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;

  QByteArray xmlbuf;
  QBuffer xmldevice( xmlbuf );
  QTextStream xmlstream( xmlbuf, QIODevice::WriteOnly );
  
  // taken from example in Qt documentation (xml.html)
  xmlstream << "<document xmlns:book = \"http://trolltech.com/fnord/book/\"";
  xmlstream << "          xmlns      = \"http://trolltech.com/fnord/\" >";
  xmlstream << "<book>";
  xmlstream << "  <book:title>Practical XML</book:title>";
  xmlstream << "  <book:author xmlns:fnord = \"http://trolltech.com/fnord/\"";
  xmlstream << "               title=\"Ms\"";
  xmlstream << "               fnord:title=\"Goddess\"";
  xmlstream << "               name=\"Eris Kallisti\"/>";
  xmlstream << "  <chapter>";
  xmlstream << "    <title>A Namespace Called fnord</title>";
  xmlstream << "  </chapter>";
  xmlstream << "</book>";
  xmlstream << "</document>";

  KoXmlDocument doc;
  KoXmlElement rootElement;
  KoXmlElement bookElement;
  KoXmlElement bookTitleElement;
  KoXmlElement bookAuthorElement;

  // ------------- first without any namespace processing ----------- 
  CHECK( doc.setContent( &xmldevice, &errorMsg, &errorLine, &errorColumn ), true );
  CHECK( errorMsg.isEmpty(), true );
  CHECK( errorLine, 0 );
  CHECK( errorColumn, 0 );

  rootElement = doc.documentElement();
  CHECK( rootElement.isNull(), false );
  CHECK( rootElement.isElement(), true );
  CHECK( rootElement.tagName(), QString("document") );
  CHECK( rootElement.prefix().isNull(), true );

  bookElement = rootElement.firstChild().toElement();
  CHECK( bookElement.isNull(), false );
  CHECK( bookElement.isElement(), true );
  CHECK( bookElement.tagName(), QString("book") );
  CHECK( bookElement.prefix().isNull(), true );
  CHECK( bookElement.localName(), QString::null );

  bookTitleElement = bookElement.firstChild().toElement();
  CHECK( bookTitleElement.isNull(), false );
  CHECK( bookTitleElement.isElement(), true );
  CHECK( bookTitleElement.tagName(), QString("book:title") );
  CHECK( bookTitleElement.prefix().isNull(), true );
  CHECK( bookTitleElement.localName(), QString::null );

  bookAuthorElement = bookTitleElement.nextSibling().toElement();
  CHECK( bookAuthorElement.isNull(), false );
  CHECK( bookAuthorElement.isElement(), true );
  CHECK( bookAuthorElement.tagName(), QString("book:author") );
  CHECK( bookAuthorElement.prefix().isNull(), true );
  CHECK( bookAuthorElement.attribute("title"), QString("Ms") );
  CHECK( bookAuthorElement.attribute("fnord:title"), QString("Goddess") );
  CHECK( bookAuthorElement.attribute("name"), QString("Eris Kallisti") );

  // ------------- now with namespace processing ----------- 
  xmldevice.at(0); // just to rewind

  CHECK( doc.setContent( &xmldevice, true, &errorMsg, &errorLine, &errorColumn ), true );
  CHECK( errorMsg.isEmpty(), true );
  CHECK( errorLine, 0 );
  CHECK( errorColumn, 0 );

  char* defaultNS = "http://trolltech.com/fnord/";
  char* bookNS = "http://trolltech.com/fnord/book/";
  char* fnordNS = "http://trolltech.com/fnord/";

  // <document>
  rootElement = doc.documentElement();
  CHECK( rootElement.isNull(), false );
  CHECK( rootElement.isElement(), true );
  CHECK( rootElement.tagName(), QString("document") );
  CHECK( rootElement.prefix().isEmpty(), true );
  CHECK( rootElement.namespaceURI(), QString( defaultNS ) );
  CHECK( rootElement.localName(), QString("document") );

  // <book>
  bookElement = rootElement.firstChild().toElement();
  CHECK( bookElement.isNull(), false );
  CHECK( bookElement.isElement(), true );
  CHECK( bookElement.tagName(), QString("book") );
  CHECK( bookElement.prefix().isEmpty(), true );
  CHECK( bookElement.namespaceURI(), QString( defaultNS ) );
  CHECK( bookElement.localName(), QString("book") );

  // <book:title>
  bookTitleElement = bookElement.firstChild().toElement();
  CHECK( bookTitleElement.isNull(), false );
  CHECK( bookTitleElement.isElement(), true );
  CHECK( bookTitleElement.tagName(), QString("title") );
  CHECK( bookTitleElement.prefix(), QString("book") );
  CHECK( bookTitleElement.namespaceURI(), QString(bookNS) );
  CHECK( bookTitleElement.localName(), QString("title") );

  // another way, find it using namedItemNS()
  KoXmlElement book2TitleElement;
  book2TitleElement = KoXml::namedItemNS( rootElement.firstChild(), bookNS, "title" );
  //book2TitleElement = bookElement.namedItemNS( bookNS, "title" ).toElement();
  CHECK( book2TitleElement==bookTitleElement, true );
  CHECK( book2TitleElement.isNull(), false );
  CHECK( book2TitleElement.isElement(), true );
  CHECK( book2TitleElement.tagName(), QString("title") );

  // <book:author>
  bookAuthorElement = bookTitleElement.nextSibling().toElement();
  CHECK( bookAuthorElement.isNull(), false );
  CHECK( bookAuthorElement.isElement(), true );
  CHECK( bookAuthorElement.tagName(), QString("author") );
  CHECK( bookAuthorElement.prefix(), QString("book") );
  CHECK( bookAuthorElement.namespaceURI(), QString(bookNS) );
  CHECK( bookAuthorElement.localName(), QString("author") );

  // another way, find it using namedItemNS()
  KoXmlElement book2AuthorElement;
  book2AuthorElement = KoXml::namedItemNS( bookElement, bookNS, "author" );
  //book2AuthorElement = bookElement.namedItemNS( bookNS, "author" ).toElement();
  CHECK( book2AuthorElement==bookAuthorElement, true );
  CHECK( book2AuthorElement.isNull(), false );
  CHECK( book2AuthorElement.isElement(), true );
  CHECK( book2AuthorElement.tagName(), QString("author") );

  // attributes in <book:author>
  // Note: with namespace processing, attribute's prefix is taken out and
  // hence "fnord:title" will simply override "title"
  // and searching attribute with prefix will give no result
  CHECK( bookAuthorElement.hasAttribute("title"), true );
  CHECK( bookAuthorElement.hasAttribute("fnord:title"), false );
  CHECK( bookAuthorElement.hasAttribute("name"), true );
  CHECK( bookAuthorElement.attribute("title"), QString("Goddess") );
  CHECK( bookAuthorElement.attribute("fnord:title").isEmpty(), true );
  CHECK( bookAuthorElement.attribute("name"), QString("Eris Kallisti") );

  // attributes in <book:author>, with NS family of functions
  // those without prefix are not accessible at all, because they do not belong
  // to any namespace at all.
  // Note: default namespace does not apply to attribute names!
  CHECK( bookAuthorElement.hasAttributeNS(defaultNS,"title"), true );
  CHECK( bookAuthorElement.hasAttributeNS(bookNS,"title"), false );
  CHECK( bookAuthorElement.hasAttributeNS(fnordNS,"title"), true );

  CHECK( bookAuthorElement.attributeNS(defaultNS,"title",""), QString("Goddess") );
  CHECK( bookAuthorElement.attributeNS(bookNS,"title",""), QString("") );
  CHECK( bookAuthorElement.attributeNS(fnordNS,"title",""), QString("Goddess") );

  CHECK( bookAuthorElement.hasAttributeNS(defaultNS,"fnord:title"), false );
  CHECK( bookAuthorElement.hasAttributeNS(bookNS,"fnord:title"), false );
  CHECK( bookAuthorElement.hasAttributeNS(fnordNS,"fnord:title"), false );
  
  CHECK( bookAuthorElement.hasAttributeNS(defaultNS,"name"), false );
  CHECK( bookAuthorElement.hasAttributeNS(bookNS,"name"), false );
  CHECK( bookAuthorElement.hasAttributeNS(fnordNS,"name"), false );
  
  CHECK( bookAuthorElement.attributeNS(defaultNS,"name",QString::null).isEmpty(), true );
  CHECK( bookAuthorElement.attributeNS(bookNS,"name",QString::null).isEmpty(), true );
  CHECK( bookAuthorElement.attributeNS(fnordNS,"name",QString::null).isEmpty(), true );
}

void testUnload()
{
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;

  QByteArray xmlbuf;
  QBuffer xmldevice( xmlbuf );
  QTextStream xmlstream( xmlbuf, QIODevice::WriteOnly );
  xmlstream << "<earth>";
  xmlstream << "<continents>";
  xmlstream << "<asia/>";
  xmlstream << "<africa/>";
  xmlstream << "<europe/>";
  xmlstream << "<america/>";
  xmlstream << "<australia/>";
  xmlstream << "<antartic/>";
  xmlstream << "</continents>";
  xmlstream << "<oceans>";
  xmlstream << "<pacific/>";
  xmlstream << "<atlantic/>";
  xmlstream << "</oceans>";
  xmlstream << "</earth>";

  KoXmlDocument doc;
  CHECK( doc.setContent( &xmldevice, &errorMsg, &errorLine, &errorColumn ), true );
  CHECK( errorMsg.isEmpty(), true );
  CHECK( errorLine, 0 );
  CHECK( errorColumn, 0 );

  KoXmlElement earthElement;
  earthElement = doc.documentElement().toElement();
  CHECK( earthElement.isNull(), false );
  CHECK( earthElement.isElement(), true );
  CHECK( earthElement.parentNode().isNull(), false );
  CHECK( earthElement.hasChildNodes(), true );
  CHECK( earthElement.tagName(), QString("earth") );
  CHECK( earthElement.prefix().isNull(), true );

  // this ensures that all child nodes of <earth> are loaded
  earthElement.firstChild();

  // explicitly unload all child nodes of <earth>
  KoXml::unload( earthElement );

  // we should get the correct first child
  KoXmlElement continentsElement = earthElement.firstChild().toElement();
  CHECK( continentsElement.nodeName(), QString("continents") );
  CHECK( continentsElement.isNull(), false );
  CHECK( continentsElement.isElement(), true );
  CHECK( continentsElement.isText(), false );
  CHECK( continentsElement.isDocument(), false );
  CHECK( continentsElement.hasChildNodes(), true );

  // let us unload everything again
  KoXml::unload( earthElement );

  // we should get the correct last child
  KoXmlElement oceansElement = earthElement.lastChild().toElement();
  CHECK( oceansElement.nodeName(), QString("oceans") );
  CHECK( oceansElement.isNull(), false );
  CHECK( oceansElement.isElement(), true );
  CHECK( oceansElement.isText(), false );
  CHECK( oceansElement.isDocument(), false );
  CHECK( oceansElement.hasChildNodes(), true );
}

void testSimpleXML()
{
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;

  QByteArray xmlbuf;
  QBuffer xmldevice( xmlbuf );

  QTextStream xmlstream( xmlbuf, QIODevice::WriteOnly );
  xmlstream << "<solarsystem>";
  xmlstream << "  <mercurius/>\n";
  xmlstream << "  <venus/>\n";
  xmlstream << "  <earth>\n";
  xmlstream << "  <moon/>\n";
  xmlstream << "  </earth>\n";
  xmlstream << "  <mars/>\n";
  xmlstream << "  <jupiter/>\n";
  xmlstream << "</solarsystem>";

  KoXmlDocument doc;
  CHECK( doc.setContent(&xmldevice,&errorMsg,&errorLine,&errorColumn ), true );
  CHECK( errorMsg.isEmpty(), true );
  CHECK( errorLine, 0 );
  CHECK( errorColumn, 0 );

  // <solarsystem>
  KoXmlElement rootElement;
  rootElement = doc.documentElement();
  CHECK( rootElement.isNull(), false );
  CHECK( rootElement.isElement(), true );
  CHECK( rootElement.parentNode().isNull(), false );
  CHECK( rootElement.hasChildNodes(), true );
  CHECK( rootElement.tagName(), QString("solarsystem") );
  CHECK( rootElement.prefix().isNull(), true );

  // node <mercurius>
  KoXmlNode firstPlanetNode;
  firstPlanetNode = rootElement.firstChild();
  CHECK( firstPlanetNode.isNull(), false );
  CHECK( firstPlanetNode.isElement(), true );
  CHECK( firstPlanetNode.nextSibling().isNull(), false );
  CHECK( firstPlanetNode.previousSibling().isNull(), true );
  CHECK( firstPlanetNode.parentNode().isNull(), false );
  CHECK( firstPlanetNode.parentNode()==rootElement, true );
  CHECK( firstPlanetNode.parentNode()!=rootElement, false );
  CHECK( firstPlanetNode.hasChildNodes(), false );
  CHECK( firstPlanetNode.firstChild().isNull(), true );
  CHECK( firstPlanetNode.lastChild().isNull(), true );

  // element <mercurius>
  KoXmlElement firstPlanetElement;
  firstPlanetElement = firstPlanetNode.toElement();
  CHECK( firstPlanetElement.isNull(), false );
  CHECK( firstPlanetElement.isElement(), true );
  CHECK( firstPlanetElement.parentNode().isNull(), false );
  CHECK( firstPlanetElement.parentNode()==rootElement, true );
  CHECK( firstPlanetElement.hasChildNodes(), false );
  CHECK( firstPlanetElement.firstChild().isNull(), true );
  CHECK( firstPlanetElement.lastChild().isNull(), true );
  CHECK( firstPlanetElement.tagName(), QString("mercurius") );
  CHECK( firstPlanetElement.prefix().isNull(), true );

  // node <venus>
  KoXmlNode secondPlanetNode;
  secondPlanetNode = firstPlanetNode.nextSibling();
  CHECK( secondPlanetNode.isNull(), false );
  CHECK( secondPlanetNode.isElement(), true );
  CHECK( secondPlanetNode.nextSibling().isNull(), false );
  CHECK( secondPlanetNode.previousSibling().isNull(), false );
  CHECK( secondPlanetNode.previousSibling()==firstPlanetNode, true );
  CHECK( secondPlanetNode.previousSibling()==firstPlanetElement, true );
  CHECK( secondPlanetNode.parentNode().isNull(), false );
  CHECK( secondPlanetNode.parentNode()==rootElement, true );
  CHECK( secondPlanetNode.parentNode()!=rootElement, false );
  CHECK( secondPlanetNode.hasChildNodes(), false );
  CHECK( secondPlanetNode.firstChild().isNull(), true );
  CHECK( secondPlanetNode.lastChild().isNull(), true );

  // element <venus>
  KoXmlElement secondPlanetElement;
  secondPlanetElement = secondPlanetNode.toElement();
  CHECK( secondPlanetElement.isNull(), false );
  CHECK( secondPlanetElement.isElement(), true );
  CHECK( secondPlanetElement.nextSibling().isNull(), false );
  CHECK( secondPlanetElement.previousSibling().isNull(), false );
  CHECK( secondPlanetElement.previousSibling()==firstPlanetNode, true );
  CHECK( secondPlanetElement.previousSibling()==firstPlanetElement, true );
  CHECK( secondPlanetElement.parentNode().isNull(), false );
  CHECK( secondPlanetElement.parentNode()==rootElement, true );
  CHECK( secondPlanetElement.parentNode()!=rootElement, false );
  CHECK( secondPlanetElement.hasChildNodes(), false );
  CHECK( secondPlanetElement.firstChild().isNull(), true );
  CHECK( secondPlanetElement.lastChild().isNull(), true );
  CHECK( secondPlanetElement.tagName(), QString("venus") );
  CHECK( secondPlanetElement.prefix().isNull(), true );
}

void testRootError()
{
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;

  // multiple root nodes are not valid !
  QByteArray xmlbuf;
  QBuffer xmldevice( xmlbuf );
  QTextStream xmlstream( xmlbuf, QIODevice::WriteOnly );
  xmlstream << "<earth></earth><moon></moon>";

  KoXmlDocument doc;
  CHECK( doc.setContent( &xmldevice, &errorMsg, &errorLine, &errorColumn ), false );
  CHECK( errorMsg.isEmpty(), false );
  CHECK( errorMsg, QString("unexpected character") );
  CHECK( errorLine, 1 );
  CHECK( errorColumn, 17 );
}

void testMismatchedTag()
{
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;

  QByteArray xmlbuf;
  QBuffer xmldevice( xmlbuf );
  QTextStream xmlstream( xmlbuf, QIODevice::WriteOnly );
  xmlstream << "<earth></e>";

  KoXmlDocument doc;
  CHECK( doc.setContent( &xmldevice, &errorMsg, &errorLine, &errorColumn ), false );
  CHECK( errorMsg.isEmpty(), false );
  CHECK( errorMsg, QString("tag mismatch") );
  CHECK( errorLine, 1 );
  CHECK( errorColumn, 11 );
}

void testSimpleOpenDocumentText()
{
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;

  QByteArray xmlbuf;
  QBuffer xmldevice( xmlbuf );
  QTextStream xmlstream( xmlbuf, QIODevice::WriteOnly );
  
  // content.xml from a simple OpenDocument text
  // it has only paragraph "Hello, world!"
  // automatic styles, declarations and unnecessary namespaces are omitted.
  xmlstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
  xmlstream << "<office:document-content ";
  xmlstream << " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"";
  xmlstream << " xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""; 
  xmlstream << " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" ";
  xmlstream << "   office:version=\"1.0\">";
  xmlstream << " <office:automatic-styles/>";
  xmlstream << " <office:body>";
  xmlstream << "  <office:text>";
  xmlstream << "   <text:p text:style-name=\"Standard\">Hello, world!</text:p>";
  xmlstream << "  </office:text>";
  xmlstream << " </office:body>";
  xmlstream << "</office:document-content>";

  KoXmlDocument doc;
  CHECK( doc.setContent( &xmldevice, true, &errorMsg, &errorLine, &errorColumn ), true );
  CHECK( errorMsg.isEmpty(), true );
  CHECK( errorLine, 0 );
  CHECK( errorColumn, 0 );
  
  char* officeNS = "urn:oasis:names:tc:opendocument:xmlns:office:1.0"; 
  char* textNS = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";

  // <office:document-content>
  KoXmlElement contentElement;
  contentElement = doc.documentElement();
  CHECK( contentElement.isNull(), false );
  CHECK( contentElement.isElement(), true );
  CHECK( contentElement.parentNode().isNull(), false );
  CHECK( contentElement.parentNode().toDocument()==doc, true );
  CHECK( contentElement.firstChild().isNull(), false );
  CHECK( contentElement.lastChild().isNull(), false );
  CHECK( contentElement.previousSibling().isNull(), false );
  CHECK( contentElement.nextSibling().isNull(), true );
  CHECK( contentElement.localName(), QString("document-content") );
  CHECK( contentElement.hasAttributeNS(officeNS,"version"), true );
  CHECK( contentElement.attributeNS(officeNS,"version",""), QString("1.0") );
  
  // <office:automatic-styles>
  KoXmlElement stylesElement;
  stylesElement = KoXml::namedItemNS( contentElement, officeNS, "automatic-styles" );
  CHECK( stylesElement.isNull(), false );
  CHECK( stylesElement.isElement(), true );
  CHECK( stylesElement.parentNode().isNull(), false );
  CHECK( stylesElement.parentNode()==contentElement, true );
  CHECK( stylesElement.firstChild().isNull(), true );
  CHECK( stylesElement.lastChild().isNull(), true );
  CHECK( stylesElement.previousSibling().isNull(), true );
  CHECK( stylesElement.nextSibling().isNull(), false );
  CHECK( stylesElement.localName(), QString("automatic-styles") );
  
  // also same <office:automatic-styles>, but without namedItemNS
  KoXmlNode styles2Element;
  styles2Element = contentElement.firstChild().toElement();
  CHECK( styles2Element.isNull(), false );
  CHECK( styles2Element.isElement(), true );
  CHECK( styles2Element.parentNode().isNull(), false );
  CHECK( styles2Element.parentNode()==contentElement, true );
  CHECK( styles2Element.firstChild().isNull(), true );
  CHECK( styles2Element.lastChild().isNull(), true );
  CHECK( styles2Element.previousSibling().isNull(), true );
  CHECK( styles2Element.nextSibling().isNull(), false );
  CHECK( styles2Element.localName(), QString("automatic-styles") );

  // <office:body>
  KoXmlElement bodyElement;
  bodyElement = KoXml::namedItemNS( contentElement, officeNS, "body" );
  CHECK( bodyElement.isNull(), false );
  CHECK( bodyElement.isElement(), true );
  CHECK( bodyElement.parentNode().isNull(), false );
  CHECK( bodyElement.parentNode()==contentElement, true );
  CHECK( bodyElement.firstChild().isNull(), false );
  CHECK( bodyElement.lastChild().isNull(), false );
  CHECK( bodyElement.previousSibling().isNull(), false );
  CHECK( bodyElement.nextSibling().isNull(), true );
  CHECK( bodyElement.localName(), QString("body") );

  // also same <office:body>, but without namedItemNS
  KoXmlElement body2Element;
  body2Element = stylesElement.nextSibling().toElement();
  CHECK( body2Element.isNull(), false );
  CHECK( body2Element.isElement(), true );
  CHECK( body2Element.parentNode().isNull(), false );
  CHECK( body2Element.parentNode()==contentElement, true );
  CHECK( body2Element.firstChild().isNull(), false );
  CHECK( body2Element.lastChild().isNull(), false );
  CHECK( body2Element.previousSibling().isNull(), false );
  CHECK( body2Element.nextSibling().isNull(), true );
  CHECK( body2Element.localName(), QString("body") );

  // <office:text>
  KoXmlElement textElement;
  textElement = KoXml::namedItemNS( bodyElement, officeNS, "text" );
  CHECK( textElement.isNull(), false );
  CHECK( textElement.isElement(), true );
  CHECK( textElement.parentNode().isNull(), false );
  CHECK( textElement.parentNode()==bodyElement, true );
  CHECK( textElement.firstChild().isNull(), false );
  CHECK( textElement.lastChild().isNull(), false );
  CHECK( textElement.previousSibling().isNull(), true );
  CHECK( textElement.nextSibling().isNull(), true );
  CHECK( textElement.localName(), QString("text") );

  // the same <office:text>, but without namedItemNS
  KoXmlElement text2Element;
  text2Element = bodyElement.firstChild().toElement();
  CHECK( text2Element.isNull(), false );
  CHECK( text2Element.isElement(), true );
  CHECK( text2Element.parentNode().isNull(), false );
  CHECK( text2Element.parentNode()==bodyElement, true );
  CHECK( text2Element.firstChild().isNull(), false );
  CHECK( text2Element.lastChild().isNull(), false );
  CHECK( text2Element.previousSibling().isNull(), true );
  CHECK( text2Element.nextSibling().isNull(), true );
  CHECK( text2Element.localName(), QString("text") );

  // <text:p>
  KoXmlElement parElement;
  parElement = textElement.firstChild().toElement();
  CHECK( parElement.isNull(), false );
  CHECK( parElement.isElement(), true );
  CHECK( parElement.parentNode().isNull(), false );
  CHECK( parElement.parentNode()==textElement, true );
  CHECK( parElement.firstChild().isNull(), false );
  CHECK( parElement.lastChild().isNull(), false );
  CHECK( parElement.previousSibling().isNull(), true );
  CHECK( parElement.nextSibling().isNull(), true );
  CHECK( parElement.tagName(), QString("p") );
  CHECK( parElement.text(), QString("Hello, world!") );
  CHECK( parElement.attributeNS( QString(textNS),"style-name",""), QString("Standard") );
}

void testSimpleOpenDocumentSpreadsheet()
{
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;

  QByteArray xmlbuf;
  QBuffer xmldevice( xmlbuf );
  QTextStream xmlstream( xmlbuf, QIODevice::WriteOnly );
  
  // content.xml from a simple OpenDocument spreadsheet
  // the document has three worksheets, the last two are empty.
  // on the first sheet, cell A1 contains the text "Hello, world".
  // automatic styles, font declarations and unnecessary namespaces are omitted.

  xmlstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
  xmlstream << "<office:document-content ";
  xmlstream << "xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\""; 
  xmlstream << "xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" ";
  xmlstream << "xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\">";
  xmlstream << "<office:body>";
  xmlstream << "<office:spreadsheet>";
  xmlstream << "<table:table table:name=\"Sheet1\" table:style-name=\"ta1\" table:print=\"false\">";
  xmlstream << "<table:table-column table:style-name=\"co1\" table:default-cell-style-name=\"Default\"/>";
  xmlstream << "<table:table-row table:style-name=\"ro1\">";
  xmlstream << "<table:table-cell office:value-type=\"string\">";
  xmlstream << "<text:p>Hello, world</text:p>";
  xmlstream << "</table:table-cell>";
  xmlstream << "</table:table-row>";
  xmlstream << "</table:table>";
  xmlstream << "<table:table table:name=\"Sheet2\" table:style-name=\"ta1\" table:print=\"false\">";
  xmlstream << "<table:table-column table:style-name=\"co1\" table:default-cell-style-name=\"Default\"/>";
  xmlstream << "<table:table-row table:style-name=\"ro1\">";
  xmlstream << "<table:table-cell/>";
  xmlstream << "</table:table-row>";
  xmlstream << "</table:table>";
  xmlstream << "<table:table table:name=\"Sheet3\" table:style-name=\"ta1\" table:print=\"false\">";
  xmlstream << "<table:table-column table:style-name=\"co1\" table:default-cell-style-name=\"Default\"/>";
  xmlstream << "<table:table-row table:style-name=\"ro1\">";
  xmlstream << "<table:table-cell/>";
  xmlstream << "</table:table-row>";
  xmlstream << "</table:table>";
  xmlstream << "</office:spreadsheet>";
  xmlstream << "</office:body>";
  xmlstream << "</office:document-content>";

  KoXmlDocument doc;
  CHECK( doc.setContent( &xmldevice, true, &errorMsg, &errorLine, &errorColumn ), true );
  CHECK( errorMsg.isEmpty(), true );
  CHECK( errorLine, 0 );
  CHECK( errorColumn, 0 );
  
  QString officeNS = "urn:oasis:names:tc:opendocument:xmlns:office:1.0"; 
  QString tableNS = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";
  QString textNS = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";

  // <office:document-content>
  KoXmlElement contentElement;
  contentElement = doc.documentElement();
  CHECK( contentElement.isNull(), false );
  CHECK( contentElement.isElement(), true );
  CHECK( contentElement.parentNode().isNull(), false );
  CHECK( contentElement.parentNode().toDocument()==doc, true );
  CHECK( contentElement.firstChild().isNull(), false );
  CHECK( contentElement.lastChild().isNull(), false );
  CHECK( contentElement.previousSibling().isNull(), false );
  CHECK( contentElement.nextSibling().isNull(), true );
  CHECK( contentElement.localName(), QString("document-content") );
  
  // <office:body>
  KoXmlElement bodyElement;
  bodyElement = contentElement.firstChild().toElement();
  CHECK( bodyElement.isNull(), false );
  CHECK( bodyElement.isElement(), true );
  CHECK( bodyElement.parentNode().isNull(), false );
  CHECK( bodyElement.parentNode()==contentElement, true );
  CHECK( bodyElement.firstChild().isNull(), false );
  CHECK( bodyElement.lastChild().isNull(), false );
  CHECK( bodyElement.previousSibling().isNull(), true );
  CHECK( bodyElement.nextSibling().isNull(), true );
  CHECK( bodyElement.localName(), QString("body") );

  // <office:spreadsheet>
  KoXmlElement spreadsheetElement;
  spreadsheetElement = bodyElement.firstChild().toElement();
  CHECK( spreadsheetElement.isNull(), false );
  CHECK( spreadsheetElement.isElement(), true );
  CHECK( spreadsheetElement.parentNode().isNull(), false );
  CHECK( spreadsheetElement.parentNode()==bodyElement, true );
  CHECK( spreadsheetElement.firstChild().isNull(), false );
  CHECK( spreadsheetElement.lastChild().isNull(), false );
  CHECK( spreadsheetElement.previousSibling().isNull(), true );
  CHECK( spreadsheetElement.nextSibling().isNull(), true );
  CHECK( spreadsheetElement.localName(), QString("spreadsheet") );
  
  // <table:table> for Sheet1
  KoXmlElement sheet1Element;
  sheet1Element = spreadsheetElement.firstChild().toElement();
  CHECK( sheet1Element.isNull(), false );
  CHECK( sheet1Element.isElement(), true );
  CHECK( sheet1Element.parentNode().isNull(), false );
  CHECK( sheet1Element.parentNode()==spreadsheetElement, true );
  CHECK( sheet1Element.firstChild().isNull(), false );
  CHECK( sheet1Element.lastChild().isNull(), false );
  CHECK( sheet1Element.previousSibling().isNull(), true );
  CHECK( sheet1Element.nextSibling().isNull(), false );
  CHECK( sheet1Element.tagName(), QString("table") );
  CHECK( sheet1Element.hasAttributeNS(tableNS,"name"), true );
  CHECK( sheet1Element.attributeNS(tableNS,"name",""), QString("Sheet1") );
  CHECK( sheet1Element.attributeNS(tableNS,"style-name",""), QString("ta1") );
  CHECK( sheet1Element.attributeNS(tableNS,"print",""), QString("false") );

    KoXml::load( sheet1Element, 100 );

  // <table:table-column>
  KoXmlElement columnElement;
  columnElement = sheet1Element.firstChild().toElement();
  CHECK( columnElement.isNull(), false );
  CHECK( columnElement.isElement(), true );
  CHECK( columnElement.parentNode().isNull(), false );
  CHECK( columnElement.parentNode()==sheet1Element, true );
  CHECK( columnElement.firstChild().isNull(), true );
  CHECK( columnElement.lastChild().isNull(), true );
  CHECK( columnElement.previousSibling().isNull(), true );
  CHECK( columnElement.nextSibling().isNull(), false );
  CHECK( columnElement.tagName(), QString("table-column") );
  CHECK( columnElement.attributeNS(tableNS,"style-name",""), QString("co1") );
  CHECK( columnElement.attributeNS(tableNS,"default-cell-style-name",""), QString("Default") );

  // <table:table-row>
  KoXmlElement rowElement;
  rowElement = columnElement.nextSibling().toElement();
  CHECK( rowElement.isNull(), false );
  CHECK( rowElement.isElement(), true );
  CHECK( rowElement.parentNode().isNull(), false );
  CHECK( rowElement.parentNode()==sheet1Element, true );
  CHECK( rowElement.firstChild().isNull(), false );
  CHECK( rowElement.lastChild().isNull(), false );
  CHECK( rowElement.previousSibling().isNull(), false );
  CHECK( rowElement.nextSibling().isNull(), true );
  CHECK( rowElement.tagName(), QString("table-row") );
  CHECK( rowElement.attributeNS(tableNS,"style-name",""), QString("ro1") );

  // <table:table-cell>
  KoXmlElement cellElement;
  cellElement = rowElement.firstChild().toElement();
  CHECK( cellElement.isNull(), false );
  CHECK( cellElement.isElement(), true );
  CHECK( cellElement.parentNode().isNull(), false );
  CHECK( cellElement.parentNode()==rowElement, true );
  CHECK( cellElement.firstChild().isNull(), false );
  CHECK( cellElement.lastChild().isNull(), false );
  CHECK( cellElement.previousSibling().isNull(), true );
  CHECK( cellElement.nextSibling().isNull(), true );
  CHECK( cellElement.tagName(), QString("table-cell") );
  CHECK( cellElement.attributeNS(officeNS,"value-type",""), QString("string") );
  
  // <text:p>
  KoXmlElement parElement;
  parElement = cellElement.firstChild().toElement();
  CHECK( parElement.isNull(), false );
  CHECK( parElement.isElement(), true );
  CHECK( parElement.parentNode().isNull(), false );
  CHECK( parElement.parentNode()==cellElement, true );
  CHECK( parElement.firstChild().isNull(), false );
  CHECK( parElement.lastChild().isNull(), false );
  CHECK( parElement.previousSibling().isNull(), true );
  CHECK( parElement.nextSibling().isNull(), true );
  CHECK( parElement.tagName(), QString("p") );
  CHECK( parElement.text(), QString("Hello, world") );

  // <table:table> for Sheet2
  KoXmlElement sheet2Element;
  sheet2Element = sheet1Element.nextSibling().toElement();
  CHECK( sheet2Element.isNull(), false );
  CHECK( sheet2Element.isElement(), true );
  CHECK( sheet2Element.parentNode().isNull(), false );
  CHECK( sheet2Element.parentNode()==spreadsheetElement, true );
  CHECK( sheet2Element.firstChild().isNull(), false );
  CHECK( sheet2Element.lastChild().isNull(), false );
  CHECK( sheet2Element.previousSibling().isNull(), false );
  CHECK( sheet2Element.nextSibling().isNull(), false );
  CHECK( sheet2Element.tagName(), QString("table") );

  // </table:table> for Sheet3
  KoXmlElement sheet3Element;
  sheet3Element = sheet2Element.nextSibling().toElement();
  CHECK( sheet3Element.isNull(), false );
  CHECK( sheet3Element.isElement(), true );
  CHECK( sheet3Element.parentNode().isNull(), false );
  CHECK( sheet3Element.parentNode()==spreadsheetElement, true );
  CHECK( sheet3Element.firstChild().isNull(), false );
  CHECK( sheet3Element.lastChild().isNull(), false );
  CHECK( sheet3Element.previousSibling().isNull(), false );
  CHECK( sheet3Element.nextSibling().isNull(), true );
  CHECK( sheet3Element.tagName(), QString("table") );
}

void testSimpleOpenDocumentPresentation()
{
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;

  QByteArray xmlbuf;
  QBuffer xmldevice( xmlbuf );
  QTextStream xmlstream( xmlbuf, QIODevice::WriteOnly );
  
  // content.xml from a simple OpenDocument presentation
  // styles, declarations and unnecessary namespaces are omitted
  // the first page is "Title" and has two text boxes
  // the second page is 

  xmlstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
  xmlstream << "<office:document-content ";
  xmlstream << "  xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" ";
  xmlstream << "  xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" ";
  xmlstream << "  xmlns:draw=\"urn:oasis:names:tc:opendocument:xmlns:drawing:1.0\" ";
  xmlstream << "  xmlns:presentation=\"urn:oasis:names:tc:opendocument:xmlns:presentation:1.0\" ";
  xmlstream << "  xmlns:svg=\"urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0\" ";
  xmlstream << "  office:version=\"1.0\">";
  xmlstream << " <office:scripts/>";
  xmlstream << " <office:automatic-styles/>";
  xmlstream << " <office:body>";
  xmlstream << "  <office:presentation>";
  xmlstream << "   <draw:page draw:name=\"Title\" draw:style-name=\"dp1\" ";
  xmlstream << "      draw:master-page-name=\"lyt-cool\" ";
  xmlstream << "      presentation:presentation-page-layout-name=\"AL1T0\">";
  xmlstream << "    <draw:frame presentation:style-name=\"pr1\" ";
  xmlstream << "      draw:text-style-name=\"P2\" draw:layer=\"layout\" ";
  xmlstream << "      svg:width=\"23.912cm\" svg:height=\"3.508cm\" ";
  xmlstream << "      svg:x=\"2.058cm\" svg:y=\"1.543cm\" ";
  xmlstream << "      presentation:class=\"title\" ";
  xmlstream << "      presentation:user-transformed=\"true\">";
  xmlstream << "     <draw:text-box>";
  xmlstream << "      <text:p text:style-name=\"P1\">Foobar</text:p>";
  xmlstream << "     </draw:text-box>";
  xmlstream << "    </draw:frame>";
  xmlstream << "    <draw:frame presentation:style-name=\"pr2\" ";
  xmlstream << "      draw:text-style-name=\"P3\" draw:layer=\"layout\"";
  xmlstream << "      svg:width=\"23.912cm\" svg:height=\"13.231cm\"";
  xmlstream << "      svg:x=\"2.058cm\" svg:y=\"5.838cm\" ";
  xmlstream << "      presentation:class=\"subtitle\">";
  xmlstream << "     <draw:text-box>";
  xmlstream << "      <text:p text:style-name=\"P3\">Foo</text:p>";
  xmlstream << "     </draw:text-box>";
  xmlstream << "    </draw:frame>";
  xmlstream << "    <presentation:notes draw:style-name=\"dp2\">";
  xmlstream << "     <draw:page-thumbnail draw:style-name=\"gr1\" draw:layer=\"layout\" svg:width=\"13.706cm\" svg:height=\"10.28cm\" svg:x=\"3.647cm\" svg:y=\"2.853cm\" draw:page-number=\"1\" presentation:class=\"page\"/>";
  xmlstream << "     <draw:frame presentation:style-name=\"pr3\" draw:text-style-name=\"P1\" draw:layer=\"layout\" svg:width=\"14.518cm\" svg:height=\"11.411cm\" svg:x=\"3.249cm\" svg:y=\"14.13cm\" presentation:class=\"notes\" presentation:placeholder=\"true\">";
  xmlstream << "      <draw:text-box/>";
  xmlstream << "     </draw:frame>";
  xmlstream << "    </presentation:notes>";
  xmlstream << "   </draw:page>";
  xmlstream << "   <presentation:settings presentation:stay-on-top=\"true\"/>";
  xmlstream << "  </office:presentation>";
  xmlstream << " </office:body>";
  xmlstream << "</office:document-content>";

  KoXmlDocument doc;
  CHECK( doc.setContent( &xmldevice, true, &errorMsg, &errorLine, &errorColumn ), true );
  CHECK( errorMsg.isEmpty(), true );
  CHECK( errorLine, 0 );
  CHECK( errorColumn, 0 );
  
  char* officeNS = "urn:oasis:names:tc:opendocument:xmlns:office:1.0"; 
  char* drawNS = "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0";
  char* textNS = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
  char* presentationNS = "urn:oasis:names:tc:opendocument:xmlns:presentation:1.0";
  char* svgNS = "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0";

  // <office:document-content>
  KoXmlElement contentElement;
  contentElement = doc.documentElement();
  CHECK( contentElement.isNull(), false );
  
  CHECK( contentElement.isElement(), true );
  CHECK( contentElement.parentNode().isNull(), false );
  CHECK( contentElement.parentNode().toDocument()==doc, true );
  CHECK( contentElement.firstChild().isNull(), false );
  CHECK( contentElement.lastChild().isNull(), false );
  CHECK( contentElement.previousSibling().isNull(), false );
  
  CHECK( contentElement.nextSibling().isNull(), true );
  CHECK( contentElement.localName(), QString("document-content") );
  CHECK( contentElement.hasAttributeNS(officeNS,"version"), true );
  CHECK( contentElement.attributeNS(officeNS,"version",""), QString("1.0") );

  // <office:scripts>
  KoXmlElement scriptsElement;
  scriptsElement = KoXml::namedItemNS( contentElement, officeNS, "scripts" );
  CHECK( scriptsElement.isNull(), false );
  CHECK( scriptsElement.isElement(), true );
  CHECK( scriptsElement.parentNode().isNull(), false );
  CHECK( scriptsElement.parentNode()==contentElement, true );
  CHECK( scriptsElement.firstChild().isNull(), true );
  CHECK( scriptsElement.lastChild().isNull(), true );
  CHECK( scriptsElement.previousSibling().isNull(), true );
  CHECK( scriptsElement.nextSibling().isNull(), false );
  CHECK( scriptsElement.localName(), QString("scripts") );
  
  // <office:automatic-styles>
  KoXmlElement stylesElement;
  stylesElement = KoXml::namedItemNS( contentElement, officeNS, "automatic-styles" );
  CHECK( stylesElement.isNull(), false );
  CHECK( stylesElement.isElement(), true );
  CHECK( stylesElement.parentNode().isNull(), false );
  CHECK( stylesElement.parentNode()==contentElement, true );
  CHECK( stylesElement.firstChild().isNull(), true );
  CHECK( stylesElement.lastChild().isNull(), true );
  CHECK( stylesElement.previousSibling().isNull(), false );
  CHECK( stylesElement.nextSibling().isNull(), false );
  CHECK( stylesElement.localName(), QString("automatic-styles") );
  
  // also same <office:automatic-styles>, but without namedItemNS
  KoXmlNode styles2Element;
  styles2Element = scriptsElement.nextSibling().toElement();
  CHECK( styles2Element.isNull(), false );
  CHECK( styles2Element.isElement(), true );
  CHECK( styles2Element.parentNode().isNull(), false );
  CHECK( styles2Element.parentNode()==contentElement, true );
  CHECK( styles2Element.firstChild().isNull(), true );
  CHECK( styles2Element.lastChild().isNull(), true );
  CHECK( styles2Element.previousSibling().isNull(), false );
  CHECK( styles2Element.nextSibling().isNull(), false );
  CHECK( styles2Element.localName(), QString("automatic-styles") );

  // <office:body>
  KoXmlElement bodyElement;
  bodyElement = KoXml::namedItemNS( contentElement, officeNS, "body" );
  CHECK( bodyElement.isNull(), false );
  CHECK( bodyElement.isElement(), true );
  CHECK( bodyElement.parentNode().isNull(), false );
  CHECK( bodyElement.parentNode()==contentElement, true );
  CHECK( bodyElement.firstChild().isNull(), false );
  CHECK( bodyElement.lastChild().isNull(), false );
  CHECK( bodyElement.previousSibling().isNull(), false );
  CHECK( bodyElement.nextSibling().isNull(), true );
  CHECK( bodyElement.localName(), QString("body") );

  // also same <office:body>, but without namedItemNS
  KoXmlElement body2Element;
  body2Element = stylesElement.nextSibling().toElement();
  CHECK( body2Element.isNull(), false );
  CHECK( body2Element.isElement(), true );
  CHECK( body2Element.parentNode().isNull(), false );
  CHECK( body2Element.parentNode()==contentElement, true );
  CHECK( body2Element.firstChild().isNull(), false );
  CHECK( body2Element.lastChild().isNull(), false );
  CHECK( body2Element.previousSibling().isNull(), false );
  CHECK( body2Element.nextSibling().isNull(), true );
  CHECK( body2Element.localName(), QString("body") );

  // <office:presentation>
  KoXmlElement presentationElement;
  presentationElement = KoXml::namedItemNS( bodyElement, officeNS, "presentation" );
  CHECK( presentationElement.isNull(), false );
  CHECK( presentationElement.isElement(), true );
  CHECK( presentationElement.parentNode().isNull(), false );
  CHECK( presentationElement.parentNode()==bodyElement, true );
  CHECK( presentationElement.firstChild().isNull(), false );
  CHECK( presentationElement.lastChild().isNull(), false );
  CHECK( presentationElement.previousSibling().isNull(), true );
  CHECK( presentationElement.nextSibling().isNull(), true );
  CHECK( presentationElement.localName(), QString("presentation") );

  // the same <office:presentation>, but without namedItemNS
  KoXmlElement presentation2Element;
  presentation2Element = bodyElement.firstChild().toElement();
  CHECK( presentation2Element.isNull(), false );
  CHECK( presentation2Element.isElement(), true );
  CHECK( presentation2Element.parentNode().isNull(), false );
  CHECK( presentation2Element.parentNode()==bodyElement, true );
  CHECK( presentation2Element.firstChild().isNull(), false );
  CHECK( presentation2Element.lastChild().isNull(), false );
  CHECK( presentation2Element.previousSibling().isNull(), true );
  CHECK( presentation2Element.nextSibling().isNull(), true );
  CHECK( presentation2Element.localName(), QString("presentation") );

  // <draw:page> for "Title"
  KoXmlElement titlePageElement;
  titlePageElement = presentationElement.firstChild().toElement();
  CHECK( titlePageElement.isNull(), false );
  CHECK( titlePageElement.isElement(), true );
  CHECK( titlePageElement.parentNode().isNull(), false );
  CHECK( titlePageElement.parentNode()==presentationElement, true );
  CHECK( titlePageElement.firstChild().isNull(), false );
  CHECK( titlePageElement.lastChild().isNull(), false );
  CHECK( titlePageElement.previousSibling().isNull(), true );
  CHECK( titlePageElement.nextSibling().isNull(), false );
  CHECK( titlePageElement.localName(), QString("page") );
  CHECK( titlePageElement.attributeNS(drawNS,"name",""), QString("Title") );
  CHECK( titlePageElement.attributeNS(drawNS,"style-name",""), QString("dp1") );
  CHECK( titlePageElement.attributeNS(drawNS,"master-page-name",""), QString("lyt-cool") );
  CHECK( titlePageElement.attributeNS(presentationNS,
    "presentation-page-layout-name",""), QString("AL1T0") );

  // <draw:frame> for the title frame
  KoXmlElement titleFrameElement;
  titleFrameElement = titlePageElement.firstChild().toElement();
  CHECK( titleFrameElement.isNull(), false );
  CHECK( titleFrameElement.isElement(), true );
  CHECK( titleFrameElement.parentNode().isNull(), false );
  CHECK( titleFrameElement.parentNode()==titlePageElement, true );
  CHECK( titleFrameElement.firstChild().isNull(), false );
  CHECK( titleFrameElement.lastChild().isNull(), false );
  CHECK( titleFrameElement.previousSibling().isNull(), true );
  CHECK( titleFrameElement.nextSibling().isNull(), false );
  CHECK( titleFrameElement.localName(), QString("frame") );
  CHECK( titleFrameElement.attributeNS(presentationNS,"style-name",""), QString("pr1") );
  CHECK( titleFrameElement.attributeNS(presentationNS,"class",""), QString("title") );
  CHECK( titleFrameElement.attributeNS(presentationNS,"user-transformed",""), QString("true") );
  CHECK( titleFrameElement.attributeNS(drawNS,"text-style-name",""), QString("P2") );
  CHECK( titleFrameElement.attributeNS(drawNS,"layer",""), QString("layout") );
  CHECK( titleFrameElement.attributeNS(svgNS,"width",""), QString("23.912cm") );
  CHECK( titleFrameElement.attributeNS(svgNS,"height",""), QString("3.508cm") );
  CHECK( titleFrameElement.attributeNS(svgNS,"x",""), QString("2.058cm") );
  CHECK( titleFrameElement.attributeNS(svgNS,"y",""), QString("1.543cm") );
  
  // <draw:text-box> of the title frame
  KoXmlElement titleBoxElement;
  titleBoxElement = titleFrameElement.firstChild().toElement();
  CHECK( titleBoxElement.isNull(), false );
  CHECK( titleBoxElement.isElement(), true );
  CHECK( titleBoxElement.parentNode().isNull(), false );
  CHECK( titleBoxElement.parentNode()==titleFrameElement, true );
  CHECK( titleBoxElement.firstChild().isNull(), false );
  CHECK( titleBoxElement.lastChild().isNull(), false );
  CHECK( titleBoxElement.previousSibling().isNull(), true );
  CHECK( titleBoxElement.nextSibling().isNull(), true );
  CHECK( titleBoxElement.localName(), QString("text-box") );
  
  // <text:p> for the title text-box
  KoXmlElement titleParElement;
  titleParElement = titleBoxElement.firstChild().toElement();
  CHECK( titleParElement.isNull(), false );
  CHECK( titleParElement.isElement(), true );
  CHECK( titleParElement.parentNode().isNull(), false );
  CHECK( titleParElement.parentNode()==titleBoxElement, true );
  CHECK( titleParElement.firstChild().isNull(), false );
  CHECK( titleParElement.lastChild().isNull(), false );
  CHECK( titleParElement.previousSibling().isNull(), true );
  CHECK( titleParElement.nextSibling().isNull(), true );
  CHECK( titleParElement.localName(), QString("p") );
  CHECK( titleParElement.attributeNS(textNS,"style-name",""), QString("P1") );
  CHECK( titleParElement.text(), QString("Foobar") );

  // <draw:frame> for the subtitle frame
  KoXmlElement subtitleFrameElement;
  subtitleFrameElement = titleFrameElement.nextSibling().toElement();
  CHECK( subtitleFrameElement.isNull(), false );
  CHECK( subtitleFrameElement.isElement(), true );
  CHECK( subtitleFrameElement.parentNode().isNull(), false );
  CHECK( subtitleFrameElement.parentNode()==titlePageElement, true );
  CHECK( subtitleFrameElement.firstChild().isNull(), false );
  CHECK( subtitleFrameElement.lastChild().isNull(), false );
  CHECK( subtitleFrameElement.previousSibling().isNull(), false );
  CHECK( subtitleFrameElement.nextSibling().isNull(), false );
  CHECK( subtitleFrameElement.localName(), QString("frame") );
  CHECK( subtitleFrameElement.attributeNS(presentationNS,"style-name",""), QString("pr2") );
  CHECK( subtitleFrameElement.attributeNS(presentationNS,"class",""), QString("subtitle") );
  CHECK( subtitleFrameElement.hasAttributeNS(presentationNS,"user-transformed"), false );
  CHECK( subtitleFrameElement.attributeNS(drawNS,"text-style-name",""), QString("P3") );
  CHECK( subtitleFrameElement.attributeNS(drawNS,"layer",""), QString("layout") );
  CHECK( subtitleFrameElement.attributeNS(svgNS,"width",""), QString("23.912cm") );
  CHECK( subtitleFrameElement.attributeNS(svgNS,"height",""), QString("13.231cm") );
  CHECK( subtitleFrameElement.attributeNS(svgNS,"x",""), QString("2.058cm") );
  CHECK( subtitleFrameElement.attributeNS(svgNS,"y",""), QString("5.838cm") );

  // <draw:text-box> of the subtitle frame
  KoXmlElement subtitleBoxElement;
  subtitleBoxElement = subtitleFrameElement.firstChild().toElement();
  CHECK( subtitleBoxElement.isNull(), false );
  CHECK( subtitleBoxElement.isElement(), true );
  CHECK( subtitleBoxElement.parentNode().isNull(), false );
  CHECK( subtitleBoxElement.parentNode()==subtitleFrameElement, true );
  CHECK( subtitleBoxElement.firstChild().isNull(), false );
  CHECK( subtitleBoxElement.lastChild().isNull(), false );
  CHECK( subtitleBoxElement.previousSibling().isNull(), true );
  CHECK( subtitleBoxElement.nextSibling().isNull(), true );
  CHECK( subtitleBoxElement.localName(), QString("text-box") );
 
  // <text:p> for the subtitle text-box
  KoXmlElement subtitleParElement;
  subtitleParElement = subtitleBoxElement.firstChild().toElement();
  CHECK( subtitleParElement.isNull(), false );
  CHECK( subtitleParElement.isElement(), true );
  CHECK( subtitleParElement.parentNode().isNull(), false );
  CHECK( subtitleParElement.parentNode()==subtitleBoxElement, true );
  CHECK( subtitleParElement.firstChild().isNull(), false );
  CHECK( subtitleParElement.lastChild().isNull(), false );
  CHECK( subtitleParElement.previousSibling().isNull(), true );
  CHECK( subtitleParElement.nextSibling().isNull(), true );
  CHECK( subtitleParElement.localName(), QString("p") );
  CHECK( subtitleParElement.attributeNS(textNS,"style-name",""), QString("P3") );
  CHECK( subtitleParElement.text(), QString("Foo") );
}

void testSimpleOpenDocumentFormula()
{
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;

  QByteArray xmlbuf;
  QBuffer xmldevice( xmlbuf );
  QTextStream xmlstream( xmlbuf, QIODevice::WriteOnly );
  
  // content.xml from a simple OpenDocument formula
  // this is essentially MathML
  xmlstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
  xmlstream << "<!DOCTYPE math:math PUBLIC \"-//OpenOffice.org//DTD Modified W3C MathML 1.01//EN\" \"math.dtd\">";
  xmlstream << "<math:math xmlns:math=\"http://www.w3.org/1998/Math/MathML\">";
  xmlstream << " <math:semantics>";
  xmlstream << "  <math:mrow>";
  xmlstream << "   <math:mi>E</math:mi>";
  xmlstream << "   <math:mo math:stretchy=\"false\">=</math:mo>";
  xmlstream << "   <math:msup>";
  xmlstream << "    <math:mi math:fontstyle=\"italic\">mc</math:mi>";
  xmlstream << "    <math:mn>2</math:mn>";
  xmlstream << "   </math:msup>";
  xmlstream << "  </math:mrow>";
  xmlstream << "  <math:annotation math:encoding=\"StarMath 5.0\">E  =  mc^2 </math:annotation>";
  xmlstream << " </math:semantics>";
  xmlstream << "</math:math>";

  KoXmlDocument doc;
  CHECK( doc.setContent( &xmldevice, true, &errorMsg, &errorLine, &errorColumn ), true );
  CHECK( errorMsg.isEmpty(), true );
  CHECK( errorLine, 0 );
  CHECK( errorColumn, 0 );

  char* mathNS = "http://www.w3.org/1998/Math/MathML";

  // <math:math>
  KoXmlElement mathElement;
  mathElement = doc.documentElement();
  CHECK( mathElement.isNull(), false );
  CHECK( mathElement.isElement(), true );
  CHECK( mathElement.parentNode().isNull(), false );
  CHECK( mathElement.parentNode().toDocument()==doc, true );
  CHECK( mathElement.firstChild().isNull(), false );
  CHECK( mathElement.lastChild().isNull(), false );
  CHECK( mathElement.previousSibling().isNull(), false );
  CHECK( mathElement.nextSibling().isNull(), true );
  CHECK( mathElement.localName(), QString("math") );
  
  // <math:semantics>
  KoXmlElement semanticsElement;
  semanticsElement = KoXml::namedItemNS( mathElement, mathNS, "semantics" );
  CHECK( semanticsElement.isNull(), false );
  CHECK( semanticsElement.isElement(), true );
  CHECK( semanticsElement.parentNode().isNull(), false );
  CHECK( semanticsElement.parentNode().toElement()==mathElement, true );
  CHECK( semanticsElement.firstChild().isNull(), false );
  CHECK( semanticsElement.lastChild().isNull(), false );
  CHECK( semanticsElement.previousSibling().isNull(), true );
  CHECK( semanticsElement.nextSibling().isNull(), true );
  CHECK( semanticsElement.localName(), QString("semantics") );
    
  // the same <math:semantics> but without namedItemNS
  KoXmlElement semantics2Element;
  semantics2Element = mathElement.firstChild().toElement();
  CHECK( semantics2Element.isNull(), false );
  CHECK( semantics2Element.isElement(), true );
  CHECK( semantics2Element.parentNode().isNull(), false );
  CHECK( semantics2Element.parentNode().toElement()==mathElement, true );
  CHECK( semantics2Element.firstChild().isNull(), false );
  CHECK( semantics2Element.lastChild().isNull(), false );
  CHECK( semantics2Element.previousSibling().isNull(), true );
  CHECK( semantics2Element.nextSibling().isNull(), true );
  CHECK( semantics2Element.localName(), QString("semantics") );

  // <math:mrow>
  KoXmlElement mrowElement;
  mrowElement = semanticsElement.firstChild().toElement();
  CHECK( mrowElement.isNull(), false );
  CHECK( mrowElement.isElement(), true );
  CHECK( mrowElement.parentNode().isNull(), false );
  CHECK( mrowElement.parentNode().toElement()==semanticsElement, true );
  CHECK( mrowElement.firstChild().isNull(), false );
  CHECK( mrowElement.lastChild().isNull(), false );
  CHECK( mrowElement.previousSibling().isNull(), true );
  CHECK( mrowElement.nextSibling().isNull(), false );
  CHECK( mrowElement.localName(), QString("mrow") );

  // <math:mi> for "E"
  KoXmlElement miElement;
  miElement = mrowElement.firstChild().toElement();
  CHECK( miElement.isNull(), false );
  CHECK( miElement.isElement(), true );
  CHECK( miElement.parentNode().isNull(), false );
  CHECK( miElement.parentNode().toElement()==mrowElement, true );
  CHECK( miElement.firstChild().isNull(), false );
  CHECK( miElement.lastChild().isNull(), false );
  CHECK( miElement.previousSibling().isNull(), true );
  CHECK( miElement.nextSibling().isNull(), false );
  CHECK( miElement.localName(), QString("mi") );

  // <math:mo> for "="
  KoXmlElement moElement;
  moElement = miElement.nextSibling().toElement();
  CHECK( moElement.isNull(), false );
  CHECK( moElement.isElement(), true );
  CHECK( moElement.parentNode().isNull(), false );
  CHECK( moElement.parentNode().toElement()==mrowElement, true );
  CHECK( moElement.firstChild().isNull(), false );
  CHECK( moElement.lastChild().isNull(), false );
  CHECK( moElement.previousSibling().isNull(), false );
  CHECK( moElement.nextSibling().isNull(), false );
  CHECK( moElement.localName(), QString("mo") );
  CHECK( moElement.attributeNS(mathNS,"stretchy",""), QString("false") );

  // <math:msup> for "mc" and superscripted "2"
  KoXmlElement msupElement;
  msupElement = moElement.nextSibling().toElement();
  CHECK( msupElement.isNull(), false );
  CHECK( msupElement.isElement(), true );
  CHECK( msupElement.parentNode().isNull(), false );
  CHECK( msupElement.parentNode().toElement()==mrowElement, true );
  CHECK( msupElement.firstChild().isNull(), false );
  CHECK( msupElement.lastChild().isNull(), false );
  CHECK( msupElement.previousSibling().isNull(), false );
  CHECK( msupElement.nextSibling().isNull(), true );
  CHECK( msupElement.localName(), QString("msup") );
  
  // <math:mi> inside the <math:msup> for "mc"
  KoXmlElement mcElement;
  mcElement = msupElement.firstChild().toElement();
  CHECK( mcElement.isNull(), false );
  CHECK( mcElement.isElement(), true );
  CHECK( mcElement.parentNode().isNull(), false );
  CHECK( mcElement.parentNode().toElement()==msupElement, true );
  CHECK( mcElement.firstChild().isNull(), false );
  CHECK( mcElement.lastChild().isNull(), false );
  CHECK( mcElement.previousSibling().isNull(), true );
  CHECK( mcElement.nextSibling().isNull(), false );
  CHECK( mcElement.localName(), QString("mi") );
  CHECK( mcElement.text(), QString("mc") );
  CHECK( mcElement.attributeNS(mathNS,"fontstyle",""), QString("italic") );
  
  // <math:mn> inside the <math:msup> for "2" (superscript)
  KoXmlElement mnElement;
  mnElement = mcElement.nextSibling().toElement();
  CHECK( mnElement.isNull(), false );
  CHECK( mnElement.isElement(), true );
  CHECK( mnElement.parentNode().isNull(), false );
  CHECK( mnElement.parentNode().toElement()==msupElement, true );
  CHECK( mnElement.firstChild().isNull(), false );
  CHECK( mnElement.lastChild().isNull(), false );
  CHECK( mnElement.previousSibling().isNull(), false );
  CHECK( mnElement.nextSibling().isNull(), true );
  CHECK( mnElement.localName(), QString("mn") );
  CHECK( mnElement.text(), QString("2") );
  
  // <math:annotation>
  KoXmlElement annotationElement;
  annotationElement = semanticsElement.lastChild().toElement();
  CHECK( annotationElement.isNull(), false );
  CHECK( annotationElement.isElement(), true );
  CHECK( annotationElement.parentNode().isNull(), false );
  CHECK( annotationElement.parentNode().toElement()==semanticsElement, true );
  CHECK( annotationElement.firstChild().isNull(), false );
  CHECK( annotationElement.lastChild().isNull(), false );
  CHECK( annotationElement.previousSibling().isNull(), false );
  CHECK( annotationElement.nextSibling().isNull(), true );
  CHECK( annotationElement.localName(), QString("annotation") );
  CHECK( annotationElement.text(), QString("E  =  mc^2 ") );
  CHECK( annotationElement.attributeNS(mathNS,"encoding",""), QString("StarMath 5.0") );
}

void testLargeOpenDocumentSpreadsheet()
{
  QString errorMsg;
  int errorLine = 0;
  int errorColumn = 0;

  int sheetCount = 4;
  int rowCount = 200;
  int colCount = 200*2;

  QByteArray xmlbuf;
  QBuffer xmldevice( xmlbuf );
  QTextStream xmlstream( xmlbuf, QIODevice::WriteOnly );
  
  // content.xml
  xmlstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
  xmlstream << "<office:document-content ";
  xmlstream << "xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" "; 
  xmlstream << "xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" ";
  xmlstream << "xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" >";
  xmlstream << "<office:body>";
  xmlstream << "<office:spreadsheet>";
  for( int i = 0; i < sheetCount; i++ )
  {
    QString sheetName = QString("Sheet%1").arg(i+1);
    xmlstream << "<table:table table:name=\"" << sheetName;
    xmlstream << "\" table:print=\"false\">";
    for( int j = 0; j < rowCount; j++ )
    {
      xmlstream << "<table:table-row>";
      for( int k = 0; k < colCount; k++ )
      {
        xmlstream << "<table:table-cell office:value-type=\"string\">";
        xmlstream << "<text:p>Hello, world</text:p>";
        xmlstream << "</table:table-cell>";
      }
      xmlstream << "</table:table-row>";
    }
    xmlstream << "</table:table>";
  }
  xmlstream << "</office:spreadsheet>";
  xmlstream << "</office:body>";
  xmlstream << "</office:document-content>";

  printf("Raw XML size: %d KB\n", xmlbuf.size()/1024 );

  QTime timer;
  timer.start();

#if 1
  // just to test parsing speed with plain dumb handler
  QXmlSimpleReader* reader = new QXmlSimpleReader;
  reader->setFeature( "http://xml.org/sax/features/namespaces", true );
  QXmlDefaultHandler handler;
  reader->setContentHandler( &handler );
  reader->setErrorHandler( &handler );
  reader->setLexicalHandler( &handler );
  reader->setDeclHandler( &handler );
  reader->setDTDHandler( &handler );
  QXmlInputSource xmlSource;
  xmlSource.setData( QString::fromUtf8( xmlbuf.data(), xmlbuf.size() ) );
  timer.start();
  reader->parse( &xmlSource ); 
  printf("Large spreadsheet: QXmlDefaultHandler parsing time is %d ms\n", timer.elapsed() );
  delete reader;
#endif

  timer.start();
  KoXmlDocument doc;

  // uncomment to see the performance if on-demand/lazy loading is not used
  // will definitely eat more memory
#ifndef KOXML_USE_QDOM
  doc.setFastLoading( true );
#endif

  CHECK( doc.setContent( &xmldevice, true, &errorMsg, &errorLine, &errorColumn ), true );
  CHECK( errorMsg.isEmpty(), true );
  CHECK( errorLine, 0 );
  CHECK( errorColumn, 0 );

  printf("Large spreadsheet: parsing time is %d ms\n", timer.elapsed() );

  // release memory taken by the XML document content
  xmlbuf.resize( 0 );

  // namespaces that will be used  
  QString officeNS = "urn:oasis:names:tc:opendocument:xmlns:office:1.0"; 
  QString tableNS = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";
  QString textNS = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";

  // <office:document-content>
  KoXmlElement contentElement;
  contentElement = doc.documentElement();
  CHECK( contentElement.isNull(), false );
  CHECK( contentElement.isElement(), true );
  CHECK( contentElement.localName(), QString("document-content") );
  
  // <office:body>
  KoXmlElement bodyElement;
  bodyElement = contentElement.firstChild().toElement();
  CHECK( bodyElement.isNull(), false );
  CHECK( bodyElement.isElement(), true );
  CHECK( bodyElement.localName(), QString("body") );

  // <office:spreadsheet>
  KoXmlElement spreadsheetElement;
  spreadsheetElement = bodyElement.firstChild().toElement();
  CHECK( spreadsheetElement.isNull(), false );
  CHECK( spreadsheetElement.isElement(), true );
  CHECK( spreadsheetElement.localName(), QString("spreadsheet") );

  // now we visit every sheet, every row, every cell
  timer.start();
  KoXmlElement tableElement;
  tableElement = spreadsheetElement.firstChild().toElement();
  for( int table=0; table < sheetCount; table++ )
  {
    QString tableName = QString("Sheet%1").arg(table+1);
    CHECK( tableElement.isNull(), false );
    CHECK( tableElement.isElement(), true );
    CHECK( tableElement.localName(), QString("table") );
    CHECK( tableElement.hasAttributeNS(tableNS,"name"), true );
    CHECK( tableElement.attributeNS(tableNS,"name",""), tableName );
    CHECK( tableElement.attributeNS(tableNS,"print",""), QString("false") );

    // load everything for this table
    KoXml::load( tableElement, 99 );

    CHECK( tableElement.parentNode().isNull(), false );
    CHECK( tableElement.parentNode()==spreadsheetElement, true );
    CHECK( tableElement.firstChild().isNull(), false );
    CHECK( tableElement.lastChild().isNull(), false );

    KoXmlElement rowElement;
    rowElement = tableElement.firstChild().toElement();
    for( int row=0; row < rowCount; row++ )
    {
      CHECK( rowElement.isNull(), false );
      CHECK( rowElement.isElement(), true );
      CHECK( rowElement.localName(), QString("table-row") );
      CHECK( rowElement.parentNode().isNull(), false );
      CHECK( rowElement.parentNode()==tableElement, true );
      CHECK( rowElement.firstChild().isNull(), false );
      CHECK( rowElement.lastChild().isNull(), false );

      KoXmlElement cellElement;
      cellElement = rowElement.firstChild().toElement();
      for( int col=0; col < colCount; col++ )
      {
        CHECK( cellElement.isNull(), false );
        CHECK( cellElement.isElement(), true );
        CHECK( cellElement.localName(), QString("table-cell") );
        CHECK( cellElement.text(), QString("Hello, world") );
        CHECK( cellElement.hasAttributeNS(officeNS,"value-type"), true );
        CHECK( cellElement.attributeNS(officeNS,"value-type",""), QString("string" ) );
        CHECK( cellElement.parentNode().isNull(), false );
        CHECK( cellElement.parentNode()==rowElement, true );
        CHECK( cellElement.firstChild().isNull(), false );
        CHECK( cellElement.lastChild().isNull(), false );
        cellElement = cellElement.nextSibling().toElement();
      }

      //KoXml::unload( rowElement );
      rowElement = rowElement.nextSibling().toElement();
    }

    KoXml::unload( tableElement );
    tableElement = tableElement.nextSibling().toElement();
  }

  printf("Large spreadsheet: iterating time is %d ms\n", timer.elapsed() );
}

int main( int argc, char** argv )
{
  Q_UNUSED( argc );
  Q_UNUSED( argv );

  testNode();
  testElement();
  testAttributes();
  testText();
  testCDATA();
  testDocument();
  testNamespace();

  testUnload();

  testSimpleXML();
  testRootError();
  testMismatchedTag();

  testSimpleOpenDocumentText();
  testSimpleOpenDocumentSpreadsheet();
  testSimpleOpenDocumentPresentation();
  testSimpleOpenDocumentFormula();

//  testLargeOpenDocumentSpreadsheet();

#ifdef KOXML_USE_QDOM
  printf("Using QDom: ");
#else
  printf("Using KoXml: ");
#endif
  printf("%d tests, %d failed\n", testCount, testFailed );

  return testFailed;
}
