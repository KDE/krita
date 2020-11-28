/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2004 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kodomtest.h"

#include "KoXmlReader.h"

#include <QTest>

static QString const KoXmlNS_office("urn:oasis:names:tc:opendocument:xmlns:office:1.0");
static QString const KoXmlNS_text("urn:oasis:names:tc:opendocument:xmlns:text:1.0");

//static void debugElemNS( const QDomElement& elem )
//{
//    qDebug( "nodeName=%s tagName=%s localName=%s prefix=%s namespaceURI=%s", elem.nodeName().latin1(), elem.tagName().latin1(), elem.localName().latin1(), elem.prefix().latin1(), elem.namespaceURI().latin1() );
//}


void KoDomTest::initTestCase()
{
    const QByteArray xml = QByteArray("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                      "<o:document-content xmlns:o=\"")
                           + KoXmlNS_office.toUtf8()
                           + "\" xmlns=\"" + KoXmlNS_text.toUtf8()
                           + "\" xmlns:text=\"" + KoXmlNS_text.toUtf8()
                           + "\">\n"
                           "<o:body><p text:style-name=\"L1\">foobar</p><p>2nd</p></o:body><o:styles></o:styles>\n"
                           "</o:document-content>\n";
    QVERIFY(m_doc.setContent(xml, true /* namespace processing */));

}


void KoDomTest::testQDom()
{
    KoXmlElement docElem = m_doc.documentElement();
    //debugElemNS( docElem );
    QCOMPARE(docElem.nodeName(), QString("o:document-content"));
    QCOMPARE(docElem.tagName(), QString("document-content"));
    QCOMPARE(docElem.localName(), QString("document-content"));
    QCOMPARE(docElem.prefix(), QString("o"));
    QCOMPARE(docElem.namespaceURI(), KoXmlNS_office);

    KoXmlElement elem = KoXml::namedItemNS(docElem, KoXmlNS_office.toUtf8(), "body");

    //debugElemNS( elem );
    QCOMPARE(elem.tagName(), QString("body"));
    QCOMPARE(elem.localName(), QString("body"));
    QCOMPARE(elem.prefix(), QString("o"));
    QCOMPARE(elem.namespaceURI(), KoXmlNS_office);

    KoXmlNode n = elem.firstChild();
    for (; !n.isNull() ; n = n.nextSibling()) {
        if (!n.isElement())
            continue;
        KoXmlElement e = n.toElement();
        //debugElemNS( e );
        QCOMPARE(e.tagName(), QString("p"));
        QCOMPARE(e.localName(), QString("p"));
        QVERIFY(e.prefix().isEmpty());
        QCOMPARE(e.namespaceURI(), KoXmlNS_text);
    }
}


void KoDomTest::testKoDom()
{
    KoXmlElement docElem = KoXml::namedItemNS(m_doc, KoXmlNS_office.toUtf8(), "document-content");
    QCOMPARE(docElem.isNull(), false);
    QCOMPARE(docElem.localName(), QString("document-content"));
    QCOMPARE(docElem.namespaceURI(), KoXmlNS_office);

    KoXmlElement body = KoXml::namedItemNS(docElem, KoXmlNS_office.toUtf8(), "body");
    QCOMPARE(body.isNull(), false);
    QCOMPARE(body.localName(), QString("body"));
    QCOMPARE(body.namespaceURI(), KoXmlNS_office);

    KoXmlElement p = KoXml::namedItemNS(body, KoXmlNS_text.toUtf8(), "p");
    QCOMPARE(p.isNull(), false);
    QCOMPARE(p.localName(), QString("p"));
    QCOMPARE(p.namespaceURI(), KoXmlNS_text);

    const KoXmlElement officeStyle = KoXml::namedItemNS(docElem, KoXmlNS_office.toUtf8(), "styles");
    QCOMPARE(officeStyle.isNull(), false);

    // Look for a non-existing element
    KoXmlElement notexist = KoXml::namedItemNS(body, KoXmlNS_text.toUtf8(), "notexist");
    QVERIFY(notexist.isNull());

    int count = 0;
    KoXmlElement elem;
    forEachElement(elem, body) {
        QCOMPARE(elem.localName(), QString("p"));
        QCOMPARE(elem.namespaceURI(), KoXmlNS_text);
        ++count;
    }
    QCOMPARE(count, 2);

    // Attributes
    // ### Qt bug: it doesn't work if using style-name instead of text:style-name in the XML
    const QString styleName = p.attributeNS(KoXmlNS_text, "style-name", QString());
    // qDebug( "%s", qPrintable( styleName ) );
    QCOMPARE(styleName, QString("L1"));
}


QTEST_MAIN(KoDomTest)
