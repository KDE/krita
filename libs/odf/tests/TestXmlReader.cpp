#include <qtest_kde.h>

#include <QBuffer>
#include <QFile>
#include <QDateTime>
#include <QProcess>
#include <QString>
#include <QTextStream>

#include <KoXmlReader.h>

#include <QtXml>


class TestXmlReader : public QObject
{
    Q_OBJECT
private slots:
    void testNode();
    void testElement();
    void testAttributes();
    void testText();
    void testCDATA();
    void testDocument();
    void testDocumentType();
    void testNamespace();
    void testParseQString();
    void testUnload();
    void testSimpleXML();
    void testRootError();
    void testMismatchedTag();
    void testConvertQDomElement();
    void testSimpleOpenDocumentText();
    void testWhitespace();
    void testSimpleOpenDocumentSpreadsheet();
    void testSimpleOpenDocumentPresentation();
    void testSimpleOpenDocumentFormula();
    void testLargeOpenDocumentSpreadsheet();
    void testExternalOpenDocumentSpreadsheet(const QString& filename);
};

void TestXmlReader::testNode()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);
    xmlstream << "<earth>";
    xmlstream << " <continents>";
    xmlstream << "  <asia/>";
    xmlstream << "  <africa/>";
    xmlstream << "  <europe/>";
    xmlstream << "  <america/>";
    xmlstream << "  <australia/>";
    xmlstream << "  <antartic/>";
    xmlstream << " </continents>";
    xmlstream << " <oceans>";
    xmlstream << "  <pacific/>";
    xmlstream << "  <atlantic/>";
    xmlstream << " </oceans>";
    xmlstream << "</earth>";
    xmldevice.close();

    KoXmlDocument doc;
    QCOMPARE(doc.setContent(&xmldevice, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    // null node
    KoXmlNode node1;
    QCOMPARE(node1.nodeName(), QString());
    QCOMPARE(node1.isNull(), true);
    QCOMPARE(node1.isElement(), false);
    QCOMPARE(node1.isDocument(), false);
    QCOMPARE(node1.ownerDocument().isNull(), true);
    QCOMPARE(node1.parentNode().isNull(), true);
    QCOMPARE(node1.hasChildNodes(), false);
    QCOMPARE(KoXml::childNodesCount(node1), 0);
    QCOMPARE(node1.firstChild().isNull(), true);
    QCOMPARE(node1.lastChild().isNull(), true);
    QCOMPARE(node1.previousSibling().isNull(), true);
    QCOMPARE(node1.nextSibling().isNull(), true);

    // compare with another null node
    KoXmlNode node2;
    QCOMPARE(node2.isNull(), true);
    QCOMPARE(node1 == node2, true);
    QCOMPARE(node1 != node2, false);

    // a node which is a document
    KoXmlNode node3 = doc;
    QCOMPARE(node3.nodeName(), QString("#document"));
    QCOMPARE(node3.isNull(), false);
    QCOMPARE(node3.isElement(), false);
    QCOMPARE(node3.isText(), false);
    QCOMPARE(node3.isDocument(), true);
    QCOMPARE(node3.ownerDocument().isNull(), false);
    QCOMPARE(node3.ownerDocument() == doc, true);
    QCOMPARE(node3.toDocument() == doc, true);
    QCOMPARE(KoXml::childNodesCount(node3), 1);

    // convert to document and the compare
    KoXmlDocument doc2 = node3.toDocument();
    QCOMPARE(doc2.nodeName(), QString("#document"));
    QCOMPARE(doc2.isNull(), false);
    QCOMPARE(doc2.isDocument(), true);
    QCOMPARE(node3 == doc2, true);
    QCOMPARE(KoXml::childNodesCount(doc2), 1);

    // a document is of course can't be converted to element
    KoXmlElement invalidElement = node3.toElement();
    QCOMPARE(invalidElement.nodeName(), QString());
    QCOMPARE(invalidElement.isNull(), true);
    QCOMPARE(invalidElement.isElement(), false);
    QCOMPARE(invalidElement.isText(), false);
    QCOMPARE(invalidElement.isDocument(), false);

    // clear() makes it a null node again
    node3.clear();
    QCOMPARE(node3.isNull(), true);
    QCOMPARE(node3.nodeName(), QString());
    QCOMPARE(node3.isElement(), false);
    QCOMPARE(node3.isText(), false);
    QCOMPARE(node3.isDocument(), false);
    QCOMPARE(node3.ownerDocument().isNull(), true);
    QCOMPARE(node1 == node3, true);
    QCOMPARE(node1 != node3, false);

    // a node which is an element for <earth>
    KoXmlNode node4 = doc.firstChild();
    QCOMPARE(node4.isNull(), false);
    QCOMPARE(node4.isElement(), true);
    QCOMPARE(node4.isText(), false);
    QCOMPARE(node4.isDocument(), false);
    QCOMPARE(node4.hasChildNodes(), true);
    QCOMPARE(KoXml::childNodesCount(node4), 2);
    QCOMPARE(node4.ownerDocument() == doc, true);
    QCOMPARE(node4.toElement() == doc.firstChild().toElement(), true);

    // clear() makes it a null node again
    node4.clear();
    QCOMPARE(node4.isNull(), true);
    QCOMPARE(node4.isElement(), false);
    QCOMPARE(node4.isText(), false);
    QCOMPARE(node4.isDocument(), false);
    QCOMPARE(node4 == node1, true);
    QCOMPARE(node4 != node1, false);
    QCOMPARE(KoXml::childNodesCount(node4), 0);

    // a node which is an element for <continents>
    KoXmlNode node5 = doc.firstChild().firstChild();
    QCOMPARE(node5.nodeName(), QString("continents"));
    QCOMPARE(node5.isNull(), false);
    QCOMPARE(node5.isElement(), true);
    QCOMPARE(node5.isText(), false);
    QCOMPARE(node5.isDocument(), false);
    QCOMPARE(node5.hasChildNodes(), true);
    QCOMPARE(KoXml::childNodesCount(node5), 6);
    QCOMPARE(node5.ownerDocument() == doc, true);

    // convert to element and the compare
    KoXmlElement continentsElement = node5.toElement();
    QCOMPARE(node5 == continentsElement, true);
    QCOMPARE(continentsElement.isNull(), false);
    QCOMPARE(continentsElement.isElement(), true);
    QCOMPARE(continentsElement.isText(), false);
    QCOMPARE(continentsElement.hasChildNodes(), true);
    QCOMPARE(KoXml::childNodesCount(continentsElement), 6);
    QCOMPARE(continentsElement.ownerDocument() == doc, true);

    // and it doesn't make sense to convert that node to document
    KoXmlDocument invalidDoc = node5.toDocument();
    QCOMPARE(invalidDoc.isNull(), true);
    QCOMPARE(invalidDoc.isElement(), false);
    QCOMPARE(invalidDoc.isText(), false);
    QCOMPARE(invalidDoc.isDocument(), false);

    // node for <europe> using namedItem() function
    KoXmlNode europeNode = continentsElement.namedItem(QString("europe"));
    QCOMPARE(europeNode.nodeName(), QString("europe"));
    QCOMPARE(europeNode.isNull(), false);
    QCOMPARE(europeNode.isElement(), true);
    QCOMPARE(europeNode.isText(), false);
    QCOMPARE(europeNode.hasChildNodes(), false);
    QCOMPARE(KoXml::childNodesCount(europeNode), 0);
    QCOMPARE(europeNode.ownerDocument() == doc, true);

    // search non-existing node
    KoXmlNode fooNode = continentsElement.namedItem(QString("foobar"));
    QCOMPARE(fooNode.isNull(), true);
    QCOMPARE(fooNode.isElement(), false);
    QCOMPARE(fooNode.isText(), false);
    QCOMPARE(fooNode.isCDATASection(), false);
    QCOMPARE(KoXml::childNodesCount(fooNode), 0);
}

void TestXmlReader::testElement()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);
    xmlstream << "<html>";
    xmlstream << "<body bgcolor=\"#000\">";
    xmlstream << "<p>";
    xmlstream << "Hello, world!";
    xmlstream << "</p>";
    xmlstream << "</body>";
    xmlstream << "</html>";
    xmldevice.close();

    KoXmlDocument doc;
    QCOMPARE(doc.setContent(&xmldevice, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    // element for <html>
    KoXmlElement rootElement;
    rootElement = doc.documentElement();
    QCOMPARE(rootElement.nodeName(), QString("html"));
    QCOMPARE(rootElement.isNull(), false);
    QCOMPARE(rootElement.isElement(), true);
    QCOMPARE(rootElement.isDocument(), false);
    QCOMPARE(rootElement.ownerDocument().isNull(), false);
    QCOMPARE(rootElement.ownerDocument() == doc, true);
    QCOMPARE(rootElement.parentNode().isNull(), false);
    QCOMPARE(rootElement.parentNode().toDocument() == doc, true);
    QCOMPARE(rootElement.hasChildNodes(), true);
    QCOMPARE(KoXml::childNodesCount(rootElement), 1);
    QCOMPARE(rootElement.tagName(), QString("html"));
    QCOMPARE(rootElement.prefix().isNull(), true);

    // element for <body>
    KoXmlElement bodyElement;
    bodyElement = rootElement.firstChild().toElement();
    QCOMPARE(bodyElement.nodeName(), QString("body"));
    QCOMPARE(bodyElement.isNull(), false);
    QCOMPARE(bodyElement.isElement(), true);
    QCOMPARE(bodyElement.isDocument(), false);
    QCOMPARE(bodyElement.ownerDocument().isNull(), false);
    QCOMPARE(bodyElement.ownerDocument() == doc, true);
    QCOMPARE(bodyElement.parentNode().isNull(), false);
    QCOMPARE(bodyElement.parentNode() == rootElement, true);
    QCOMPARE(bodyElement.hasChildNodes(), true);
    QCOMPARE(KoXml::childNodesCount(bodyElement), 1);
    QCOMPARE(bodyElement.tagName(), QString("body"));
    QCOMPARE(bodyElement.prefix().isNull(), true);
    QCOMPARE(bodyElement.hasAttribute("bgcolor"), true);
    QCOMPARE(bodyElement.attribute("bgcolor"), QString("#000"));

    // a shared copy of <body>, will still have access to attribute bgcolor
    KoXmlElement body2Element;
    body2Element = bodyElement;
    QCOMPARE(body2Element.nodeName(), QString("body"));
    QCOMPARE(body2Element.isNull(), false);
    QCOMPARE(body2Element.isElement(), true);
    QCOMPARE(body2Element.isDocument(), false);
    QCOMPARE(body2Element.ownerDocument().isNull(), false);
    QCOMPARE(body2Element.ownerDocument() == doc, true);
    QCOMPARE(body2Element == bodyElement, true);
    QCOMPARE(body2Element != bodyElement, false);
    QCOMPARE(body2Element.hasChildNodes(), true);
    QCOMPARE(KoXml::childNodesCount(body2Element), 1);
    QCOMPARE(body2Element.tagName(), QString("body"));
    QCOMPARE(body2Element.prefix().isNull(), true);
    QCOMPARE(body2Element.hasAttribute("bgcolor"), true);
    QCOMPARE(body2Element.attribute("bgcolor"), QString("#000"));

    // empty element, by default constructor
    KoXmlElement testElement;
    QCOMPARE(testElement.nodeName(), QString());
    QCOMPARE(testElement.isNull(), true);
    QCOMPARE(testElement.isElement(), false);
    QCOMPARE(testElement.isDocument(), false);
    QCOMPARE(testElement.ownerDocument().isNull(), true);
    QCOMPARE(testElement.ownerDocument() != doc, true);
    QCOMPARE(testElement == rootElement, false);
    QCOMPARE(testElement != rootElement, true);
    QCOMPARE(testElement.parentNode().isNull(), true);
    QCOMPARE(testElement.hasChildNodes(), false);
    QCOMPARE(KoXml::childNodesCount(testElement), 0);

    // check assignment operator
    testElement = rootElement;
    QCOMPARE(testElement.nodeName(), QString("html"));
    QCOMPARE(testElement.isNull(), false);
    QCOMPARE(testElement.isElement(), true);
    QCOMPARE(testElement.isDocument(), false);
    QCOMPARE(testElement == rootElement, true);
    QCOMPARE(testElement != rootElement, false);
    QCOMPARE(testElement.parentNode().isNull(), false);
    QCOMPARE(testElement.parentNode().toDocument() == doc, true);
    QCOMPARE(testElement.tagName(), QString("html"));
    QCOMPARE(testElement.prefix().isNull(), true);
    QCOMPARE(KoXml::childNodesCount(testElement), 1);

    // assigned from another empty element
    testElement = KoXmlElement();
    QCOMPARE(testElement.isNull(), true);
    QCOMPARE(testElement != rootElement, true);

    // assigned from <body>
    testElement = bodyElement;
    QCOMPARE(testElement.isNull(), false);
    QCOMPARE(testElement.isElement(), true);
    QCOMPARE(testElement.isDocument(), false);
    QCOMPARE(testElement.ownerDocument().isNull(), false);
    QCOMPARE(testElement.ownerDocument() == doc, true);
    QCOMPARE(testElement == bodyElement, true);
    QCOMPARE(testElement.parentNode().isNull(), false);
    QCOMPARE(testElement.tagName(), QString("body"));
    QCOMPARE(testElement.prefix().isNull(), true);
    QCOMPARE(testElement.hasChildNodes(), true);
    QCOMPARE(KoXml::childNodesCount(testElement), 1);

    // copy constructor
    KoXmlElement dummyElement(rootElement);
    QCOMPARE(dummyElement.isNull(), false);
    QCOMPARE(dummyElement.isElement(), true);
    QCOMPARE(dummyElement.isDocument(), false);
    QCOMPARE(dummyElement.ownerDocument().isNull(), false);
    QCOMPARE(dummyElement.ownerDocument() == doc, true);
    QCOMPARE(dummyElement == rootElement, true);
    QCOMPARE(dummyElement.parentNode().isNull(), false);
    QCOMPARE(dummyElement.hasChildNodes(), true);
    QCOMPARE(KoXml::childNodesCount(dummyElement), 1);
    QCOMPARE(dummyElement.tagName(), QString("html"));
    QCOMPARE(dummyElement.prefix().isNull(), true);

    // clear() turns element to null node
    dummyElement.clear();
    QCOMPARE(dummyElement.isNull(), true);
    QCOMPARE(dummyElement.isElement(), false);
    QCOMPARE(dummyElement.isDocument(), false);
    QCOMPARE(dummyElement.ownerDocument().isNull(), true);
    QCOMPARE(dummyElement.ownerDocument() == doc, false);
    QCOMPARE(dummyElement.hasChildNodes(), false);
    QCOMPARE(KoXml::childNodesCount(dummyElement), 0);
    QCOMPARE(dummyElement == rootElement, false);
    QCOMPARE(dummyElement != rootElement, true);

    // check for plain null node converted to element
    KoXmlNode dummyNode;
    dummyElement = dummyNode.toElement();
    QCOMPARE(dummyElement.isNull(), true);
    QCOMPARE(dummyElement.isElement(), false);
    QCOMPARE(dummyElement.isDocument(), false);
    QCOMPARE(dummyElement.ownerDocument().isNull(), true);
    QCOMPARE(dummyElement.hasChildNodes(), false);
    QCOMPARE(KoXml::childNodesCount(dummyElement), 0);
    QCOMPARE(dummyElement.ownerDocument() == doc, false);
}

void TestXmlReader::testAttributes()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);
    xmlstream << "<p>";
    xmlstream << "<img src=\"foo.png\" width=\"300\" height=\"150\"/>";
    xmlstream << "</p>";
    xmldevice.close();

    KoXmlDocument doc;
    QCOMPARE(doc.setContent(&xmldevice, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    KoXmlElement rootElement;
    rootElement = doc.documentElement();
    QCOMPARE(rootElement.isNull(), false);
    QCOMPARE(rootElement.isElement(), true);
    QCOMPARE(rootElement.parentNode().isNull(), false);
    QCOMPARE(rootElement.parentNode().toDocument() == doc, true);
    QCOMPARE(rootElement.tagName(), QString("p"));
    QCOMPARE(rootElement.prefix().isNull(), true);
    QCOMPARE(KoXml::childNodesCount(rootElement), 1);

    KoXmlElement imgElement;
    imgElement = rootElement.firstChild().toElement();
    QCOMPARE(imgElement.isNull(), false);
    QCOMPARE(imgElement.isElement(), true);
    QCOMPARE(imgElement.tagName(), QString("img"));
    QCOMPARE(imgElement.prefix().isNull(), true);
    QCOMPARE(KoXml::childNodesCount(imgElement), 0);
    QCOMPARE(imgElement.hasAttribute("src"), true);
    QCOMPARE(imgElement.hasAttribute("width"), true);
    QCOMPARE(imgElement.hasAttribute("height"), true);
    QCOMPARE(imgElement.hasAttribute("non-exist"), false);
    QCOMPARE(imgElement.hasAttribute("SRC"), false);
    QCOMPARE(imgElement.attribute("src"), QString("foo.png"));
    QCOMPARE(imgElement.attribute("width"), QString("300"));
    QCOMPARE(imgElement.attribute("width").toInt(), 300);
    QCOMPARE(imgElement.attribute("height"), QString("150"));
    QCOMPARE(imgElement.attribute("height").toInt(), 150);
    QCOMPARE(imgElement.attribute("border").isEmpty(), true);
    QCOMPARE(imgElement.attribute("border", "0").toInt(), 0);
    QCOMPARE(imgElement.attribute("border", "-1").toInt(), -1);

    QStringList list = KoXml::attributeNames(imgElement);
    QCOMPARE(list.count(), 3);
    QVERIFY(list.contains("src"));
    QVERIFY(list.contains("width"));
    QVERIFY(list.contains("height"));
    QVERIFY(! list.contains("border"));
    foreach(QString a, list) {
        QCOMPARE(imgElement.hasAttribute(a), true);
        QCOMPARE(imgElement.attribute(a).isEmpty(), false);
    }
}

void TestXmlReader::testText()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);
    xmlstream << "<p>";
    xmlstream << "Hello ";
    xmlstream << "<b>world</b>";
    xmlstream << "</p>";
    xmldevice.close();

    KoXmlDocument doc;
    QCOMPARE(doc.setContent(&xmldevice, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    // element for <p>
    KoXmlElement parElement;
    parElement = doc.documentElement();
    QCOMPARE(parElement.isNull(), false);
    QCOMPARE(parElement.isElement(), true);
    QCOMPARE(parElement.isText(), false);
    QCOMPARE(parElement.isDocument(), false);
    QCOMPARE(parElement.ownerDocument().isNull(), false);
    QCOMPARE(parElement.ownerDocument() == doc, true);
    QCOMPARE(parElement.parentNode().isNull(), false);
    QCOMPARE(parElement.parentNode().toDocument() == doc, true);
    QCOMPARE(parElement.hasChildNodes(), true);
    QCOMPARE(KoXml::childNodesCount(parElement), 2);   // <b> and text node "Hello "
    QCOMPARE(parElement.tagName(), QString("p"));
    QCOMPARE(parElement.prefix().isNull(), true);
    QCOMPARE(parElement.text(), QString("Hello world"));

    // node for "Hello"
    KoXmlNode helloNode;
    helloNode = parElement.firstChild();
    QCOMPARE(helloNode.nodeName(), QString("#text"));
    QCOMPARE(helloNode.isNull(), false);
    QCOMPARE(helloNode.isElement(), false);
    QCOMPARE(helloNode.isText(), true);
    QCOMPARE(helloNode.isDocument(), false);
    QCOMPARE(helloNode.hasChildNodes(), false);
    QCOMPARE(KoXml::childNodesCount(helloNode), 0);

    // "Hello" text
    KoXmlText helloText;
    helloText = helloNode.toText();
    QCOMPARE(helloText.nodeName(), QString("#text"));
    QCOMPARE(helloText.isNull(), false);
    QCOMPARE(helloText.isElement(), false);
    QCOMPARE(helloText.isText(), true);
    QCOMPARE(helloText.isDocument(), false);
    QCOMPARE(helloText.data(), QString("Hello "));
    QCOMPARE(KoXml::childNodesCount(helloText), 0);

    // shared copy of the text
    KoXmlText hello2Text;
    hello2Text = helloText;
    QCOMPARE(hello2Text.isNull(), false);
    QCOMPARE(hello2Text.isElement(), false);
    QCOMPARE(hello2Text.isText(), true);
    QCOMPARE(hello2Text.isDocument(), false);
    QCOMPARE(hello2Text.data(), QString("Hello "));
    QCOMPARE(KoXml::childNodesCount(hello2Text), 0);

    // element for <b>
    KoXmlElement boldElement;
    boldElement = helloNode.nextSibling().toElement();
    QCOMPARE(boldElement.isNull(), false);
    QCOMPARE(boldElement.isElement(), true);
    QCOMPARE(boldElement.isText(), false);
    QCOMPARE(boldElement.isDocument(), false);
    QCOMPARE(boldElement.ownerDocument().isNull(), false);
    QCOMPARE(boldElement.ownerDocument() == doc, true);
    QCOMPARE(boldElement.parentNode().isNull(), false);
    QCOMPARE(boldElement.hasChildNodes(), true);
    QCOMPARE(KoXml::childNodesCount(boldElement), 1);   // text node "world"
    QCOMPARE(boldElement.tagName(), QString("b"));
    QCOMPARE(boldElement.prefix().isNull(), true);

    // "world" text
    KoXmlText worldText;
    worldText = boldElement.firstChild().toText();
    QCOMPARE(worldText.isNull(), false);
    QCOMPARE(worldText.isElement(), false);
    QCOMPARE(worldText.isText(), true);
    QCOMPARE(worldText.isDocument(), false);
    QCOMPARE(worldText.data(), QString("world"));
    QCOMPARE(KoXml::childNodesCount(worldText), 0);
}

void TestXmlReader::testCDATA()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);
    xmlstream << "<p>";
    xmlstream << "Hello ";
    xmlstream << "<![CDATA[world]]>";
    xmlstream << "</p>";
    xmldevice.close();

    KoXmlDocument doc;
    QCOMPARE(doc.setContent(&xmldevice, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    // element for <p>
    KoXmlElement parElement;
    parElement = doc.documentElement();
    QCOMPARE(parElement.isNull(), false);
    QCOMPARE(parElement.isElement(), true);
    QCOMPARE(parElement.isText(), false);
    QCOMPARE(parElement.isDocument(), false);
    QCOMPARE(parElement.ownerDocument().isNull(), false);
    QCOMPARE(parElement.ownerDocument() == doc, true);
    QCOMPARE(parElement.parentNode().isNull(), false);
    QCOMPARE(parElement.parentNode().toDocument() == doc, true);
    QCOMPARE(parElement.hasChildNodes(), true);
    QCOMPARE(KoXml::childNodesCount(parElement), 2);
    QCOMPARE(parElement.tagName(), QString("p"));
    QCOMPARE(parElement.prefix().isNull(), true);
    QCOMPARE(parElement.text(), QString("Hello world"));

    // node for "Hello"
    KoXmlNode helloNode;
    helloNode = parElement.firstChild();
    QCOMPARE(helloNode.isNull(), false);
    QCOMPARE(helloNode.isElement(), false);
    QCOMPARE(helloNode.isText(), true);
    QCOMPARE(helloNode.isDocument(), false);

    // "Hello" text
    KoXmlText helloText;
    helloText = helloNode.toText();
    QCOMPARE(helloText.isNull(), false);
    QCOMPARE(helloText.isElement(), false);
    QCOMPARE(helloText.isText(), true);
    QCOMPARE(helloText.isDocument(), false);
    QCOMPARE(helloText.data(), QString("Hello "));

    // node for CDATA "world!"
    // Note: isText() is also true for CDATA
    KoXmlNode worldNode;
    worldNode = helloNode.nextSibling();
    QCOMPARE(worldNode.nodeName(), QString("#cdata-section"));
    QCOMPARE(worldNode.isNull(), false);
    QCOMPARE(worldNode.isElement(), false);
    QCOMPARE(worldNode.isText(), true);
    QCOMPARE(worldNode.isCDATASection(), true);
    QCOMPARE(worldNode.isDocument(), false);

    // CDATA section for "world!"
    // Note: isText() is also true for CDATA
    KoXmlCDATASection worldCDATA;
    worldCDATA = worldNode.toCDATASection();
    QCOMPARE(worldCDATA.nodeName(), QString("#cdata-section"));
    QCOMPARE(worldCDATA.isNull(), false);
    QCOMPARE(worldCDATA.isElement(), false);
    QCOMPARE(worldCDATA.isText(), true);
    QCOMPARE(worldCDATA.isCDATASection(), true);
    QCOMPARE(worldCDATA.isDocument(), false);
    QCOMPARE(worldCDATA.data(), QString("world"));
}

void TestXmlReader::testDocument()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);
    xmlstream << "<koffice>";
    xmlstream << "  <kword/>\n";
    xmlstream << "  <kpresenter/>\n";
    xmlstream << "  <krita/>\n";
    xmlstream << "</koffice>";
    xmldevice.close();

    KoXmlDocument doc;

    // empty document
    QCOMPARE(doc.nodeName(), QString());
    QCOMPARE(doc.isNull(), true);
    QCOMPARE(doc.isElement(), false);
    QCOMPARE(doc.isDocument(), false);
    QCOMPARE(doc.parentNode().isNull(), true);
    QCOMPARE(doc.firstChild().isNull(), true);
    QCOMPARE(doc.lastChild().isNull(), true);
    QCOMPARE(doc.previousSibling().isNull(), true);
    QCOMPARE(doc.nextSibling().isNull(), true);

    // now give something as the content
    QCOMPARE(doc.setContent(&xmldevice, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    // this document has something already
    QCOMPARE(doc.nodeName(), QString("#document"));
    QCOMPARE(doc.isNull(), false);
    QCOMPARE(doc.isElement(), false);
    QCOMPARE(doc.isDocument(), true);
    QCOMPARE(doc.parentNode().isNull(), true);
    QCOMPARE(doc.firstChild().isNull(), false);
    QCOMPARE(doc.lastChild().isNull(), false);
    QCOMPARE(doc.previousSibling().isNull(), true);
    QCOMPARE(doc.nextSibling().isNull(), true);

    // make sure its children are fine
    KoXmlElement rootElement;
    rootElement = doc.firstChild().toElement();
    QCOMPARE(rootElement.isNull(), false);
    QCOMPARE(rootElement.isElement(), true);
    QCOMPARE(rootElement.isDocument(), false);
    QCOMPARE(rootElement.parentNode().isNull(), false);
    QCOMPARE(rootElement.parentNode().toDocument() == doc, true);
    rootElement = doc.lastChild().toElement();
    QCOMPARE(rootElement.isNull(), false);
    QCOMPARE(rootElement.isElement(), true);
    QCOMPARE(rootElement.isDocument(), false);
    QCOMPARE(rootElement.parentNode().isNull(), false);
    QCOMPARE(rootElement.parentNode().toDocument() == doc, true);

    // clear() converts it into null node
    doc.clear();
    QCOMPARE(doc.nodeName(), QString());
    QCOMPARE(doc.isNull(), true);
    QCOMPARE(doc.isElement(), false);
    QCOMPARE(doc.isDocument(), false);
    QCOMPARE(doc.parentNode().isNull(), true);
    QCOMPARE(doc.firstChild().isNull(), true);
    QCOMPARE(doc.lastChild().isNull(), true);
    QCOMPARE(doc.previousSibling().isNull(), true);
    QCOMPARE(doc.nextSibling().isNull(), true);

    // assigned from another empty document
    doc = KoXmlDocument();
    QCOMPARE(doc.nodeName(), QString());
    QCOMPARE(doc.nodeName().isEmpty(), true);
    QCOMPARE(doc.isNull(), true);
    QCOMPARE(doc.isElement(), false);
    QCOMPARE(doc.isDocument(), false);
    QCOMPARE(doc.parentNode().isNull(), true);
}

void TestXmlReader::testDocumentType()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);
    xmlstream << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\" \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">";
    xmlstream << "<body>";
    xmlstream << "  <img/>\n";
    xmlstream << "  <p/>\n";
    xmlstream << "  <blockquote/>\n";
    xmlstream << "</body>";
    xmldevice.close();

    // empty document
    KoXmlDocument doc;
    QCOMPARE(doc.nodeName(), QString());
    QCOMPARE(doc.isNull(), true);
    QCOMPARE(doc.isElement(), false);
    QCOMPARE(doc.isDocument(), false);
    QCOMPARE(doc.parentNode().isNull(), true);
    QCOMPARE(doc.firstChild().isNull(), true);
    QCOMPARE(doc.lastChild().isNull(), true);
    QCOMPARE(doc.previousSibling().isNull(), true);
    QCOMPARE(doc.nextSibling().isNull(), true);

    // has empty doctype
    KoXmlDocumentType doctype = doc.doctype();
    QCOMPARE(doctype.nodeName(), QString());
    QCOMPARE(doctype.isNull(), true);
    QCOMPARE(doctype.isElement(), false);
    QCOMPARE(doctype.isDocument(), false);
    QCOMPARE(doctype.isDocumentType(), false);
    QCOMPARE(doctype.parentNode().isNull(), true);
    QCOMPARE(doctype.firstChild().isNull(), true);
    QCOMPARE(doctype.lastChild().isNull(), true);
    QCOMPARE(doctype.previousSibling().isNull(), true);
    QCOMPARE(doctype.nextSibling().isNull(), true);

    // now give something as the content
    QCOMPARE(doc.setContent(&xmldevice, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    // this document has something already
    QCOMPARE(doc.nodeName(), QString("#document"));
    QCOMPARE(doc.isNull(), false);
    QCOMPARE(doc.isElement(), false);
    QCOMPARE(doc.isDocument(), true);
    QCOMPARE(doc.parentNode().isNull(), true);
    QCOMPARE(doc.firstChild().isNull(), false);
    QCOMPARE(doc.lastChild().isNull(), false);
    QCOMPARE(doc.previousSibling().isNull(), true);
    QCOMPARE(doc.nextSibling().isNull(), true);

    // the doctype becomes a valid one
    doctype = doc.doctype();
    QCOMPARE(doctype.nodeName(), QString("html"));
    QCOMPARE(doctype.name(), QString("html"));
    QCOMPARE(doctype.isNull(), false);
    QCOMPARE(doctype.isElement(), false);
    QCOMPARE(doctype.isDocument(), false);
    QCOMPARE(doctype.isDocumentType(), true);
    QCOMPARE(doctype.parentNode().isNull(), false);
    QCOMPARE(doctype.parentNode() == doc, true);
    QCOMPARE(doctype.firstChild().isNull(), true);
    QCOMPARE(doctype.lastChild().isNull(), true);
    QCOMPARE(doctype.previousSibling().isNull(), true);
    QCOMPARE(doctype.nextSibling().isNull(), true);
}

void TestXmlReader::testNamespace()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);

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
    xmldevice.close();

    KoXmlDocument doc;
    KoXmlElement rootElement;
    KoXmlElement bookElement;
    KoXmlElement bookTitleElement;
    KoXmlElement bookAuthorElement;

    // ------------- first without any namespace processing -----------
    QCOMPARE(doc.setContent(&xmldevice, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    rootElement = doc.documentElement();
    QCOMPARE(rootElement.isNull(), false);
    QCOMPARE(rootElement.isElement(), true);
    QCOMPARE(rootElement.tagName(), QString("document"));
    QCOMPARE(rootElement.prefix().isNull(), true);

    bookElement = rootElement.firstChild().toElement();
    QCOMPARE(bookElement.isNull(), false);
    QCOMPARE(bookElement.isElement(), true);
    QCOMPARE(bookElement.tagName(), QString("book"));
    QCOMPARE(bookElement.prefix().isNull(), true);
    QCOMPARE(bookElement.localName(), QString());

    bookTitleElement = bookElement.firstChild().toElement();
    QCOMPARE(bookTitleElement.isNull(), false);
    QCOMPARE(bookTitleElement.isElement(), true);
    QCOMPARE(bookTitleElement.tagName(), QString("book:title"));
    QCOMPARE(bookTitleElement.prefix().isNull(), true);
    QCOMPARE(bookTitleElement.localName(), QString());

    bookAuthorElement = bookTitleElement.nextSibling().toElement();
    QCOMPARE(bookAuthorElement.isNull(), false);
    QCOMPARE(bookAuthorElement.isElement(), true);
    QCOMPARE(bookAuthorElement.tagName(), QString("book:author"));
    QCOMPARE(bookAuthorElement.prefix().isNull(), true);
    QCOMPARE(bookAuthorElement.attribute("title"), QString("Ms"));
    QCOMPARE(bookAuthorElement.attribute("fnord:title"), QString("Goddess"));
    QCOMPARE(bookAuthorElement.attribute("name"), QString("Eris Kallisti"));

    // ------------- now with namespace processing -----------
    xmldevice.seek(0); // just to rewind

    QCOMPARE(doc.setContent(&xmldevice, true, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    const char* defaultNS = "http://trolltech.com/fnord/";
    const char* bookNS = "http://trolltech.com/fnord/book/";
    const char* fnordNS = "http://trolltech.com/fnord/";

    // <document>
    rootElement = doc.documentElement();
    QCOMPARE(rootElement.isNull(), false);
    QCOMPARE(rootElement.isElement(), true);
    QCOMPARE(rootElement.tagName(), QString("document"));
    QCOMPARE(rootElement.prefix().isEmpty(), true);
    QCOMPARE(rootElement.namespaceURI(), QString(defaultNS));
    QCOMPARE(rootElement.localName(), QString("document"));

    // <book>
    bookElement = rootElement.firstChild().toElement();
    QCOMPARE(bookElement.isNull(), false);
    QCOMPARE(bookElement.isElement(), true);
    QCOMPARE(bookElement.tagName(), QString("book"));
    QCOMPARE(bookElement.prefix().isEmpty(), true);
    QCOMPARE(bookElement.namespaceURI(), QString(defaultNS));
    QCOMPARE(bookElement.localName(), QString("book"));

    // <book:title>
    bookTitleElement = bookElement.firstChild().toElement();
    QCOMPARE(bookTitleElement.isNull(), false);
    QCOMPARE(bookTitleElement.isElement(), true);
    QCOMPARE(bookTitleElement.tagName(), QString("title"));
    QCOMPARE(bookTitleElement.prefix(), QString("book"));
    QCOMPARE(bookTitleElement.namespaceURI(), QString(bookNS));
    QCOMPARE(bookTitleElement.localName(), QString("title"));

    // another way, find it using namedItemNS()
    KoXmlElement book2TitleElement;
    book2TitleElement = KoXml::namedItemNS(rootElement.firstChild(), bookNS, "title");
    //book2TitleElement = bookElement.namedItemNS( bookNS, "title" ).toElement();
    QCOMPARE(book2TitleElement == bookTitleElement, true);
    QCOMPARE(book2TitleElement.isNull(), false);
    QCOMPARE(book2TitleElement.isElement(), true);
    QCOMPARE(book2TitleElement.tagName(), QString("title"));

    // <book:author>
    bookAuthorElement = bookTitleElement.nextSibling().toElement();
    QCOMPARE(bookAuthorElement.isNull(), false);
    QCOMPARE(bookAuthorElement.isElement(), true);
    QCOMPARE(bookAuthorElement.tagName(), QString("author"));
    QCOMPARE(bookAuthorElement.prefix(), QString("book"));
    QCOMPARE(bookAuthorElement.namespaceURI(), QString(bookNS));
    QCOMPARE(bookAuthorElement.localName(), QString("author"));

    // another way, find it using namedItemNS()
    KoXmlElement book2AuthorElement;
    book2AuthorElement = KoXml::namedItemNS(bookElement, bookNS, "author");
    //book2AuthorElement = bookElement.namedItemNS( bookNS, "author" ).toElement();
    QCOMPARE(book2AuthorElement == bookAuthorElement, true);
    QCOMPARE(book2AuthorElement.isNull(), false);
    QCOMPARE(book2AuthorElement.isElement(), true);
    QCOMPARE(book2AuthorElement.tagName(), QString("author"));

    // attributes in <book:author>
    // Note: with namespace processing, attribute's prefix is taken out and
    // hence "fnord:title" will simply override "title"
    // and searching attribute with prefix will give no result
    QCOMPARE(bookAuthorElement.hasAttribute("title"), true);
    QCOMPARE(bookAuthorElement.hasAttribute("fnord:title"), false);
    QCOMPARE(bookAuthorElement.hasAttribute("name"), true);
    QCOMPARE(bookAuthorElement.attribute("title"), QString("Goddess"));
    QCOMPARE(bookAuthorElement.attribute("fnord:title").isEmpty(), true);
    QCOMPARE(bookAuthorElement.attribute("name"), QString("Eris Kallisti"));

    // attributes in <book:author>, with NS family of functions
    // those without prefix are not accessible at all, because they do not belong
    // to any namespace at all.
    // Note: default namespace does not apply to attribute names!
    QCOMPARE(bookAuthorElement.hasAttributeNS(defaultNS, "title"), true);
    QCOMPARE(bookAuthorElement.hasAttributeNS(bookNS, "title"), false);
    QCOMPARE(bookAuthorElement.hasAttributeNS(fnordNS, "title"), true);

    QCOMPARE(bookAuthorElement.attributeNS(defaultNS, "title", ""), QString("Goddess"));
    QCOMPARE(bookAuthorElement.attributeNS(bookNS, "title", ""), QString(""));
    QCOMPARE(bookAuthorElement.attributeNS(fnordNS, "title", ""), QString("Goddess"));

    QCOMPARE(bookAuthorElement.hasAttributeNS(defaultNS, "fnord:title"), false);
    QCOMPARE(bookAuthorElement.hasAttributeNS(bookNS, "fnord:title"), false);
    QCOMPARE(bookAuthorElement.hasAttributeNS(fnordNS, "fnord:title"), false);

    QCOMPARE(bookAuthorElement.hasAttributeNS(defaultNS, "name"), false);
    QCOMPARE(bookAuthorElement.hasAttributeNS(bookNS, "name"), false);
    QCOMPARE(bookAuthorElement.hasAttributeNS(fnordNS, "name"), false);

    QCOMPARE(bookAuthorElement.attributeNS(defaultNS, "name", QString()).isEmpty(), true);
    QCOMPARE(bookAuthorElement.attributeNS(bookNS, "name", QString()).isEmpty(), true);
    QCOMPARE(bookAuthorElement.attributeNS(fnordNS, "name", QString()).isEmpty(), true);
}

// mostly similar to testNamespace above, but parse from a QString
void TestXmlReader::testParseQString()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QString xmlText;
    xmlText +=  "<document xmlns:book = \"http://trolltech.com/fnord/book/\"";
    xmlText +=  "          xmlns      = \"http://trolltech.com/fnord/\" >";
    xmlText +=  "<book>";
    xmlText +=  "  <book:title>Practical XML</book:title>";
    xmlText +=  "  <book:author xmlns:fnord = \"http://trolltech.com/fnord/\"";
    xmlText +=  "               title=\"Ms\"";
    xmlText +=  "               fnord:title=\"Goddess\"";
    xmlText +=  "               name=\"Eris Kallisti\"/>";
    xmlText +=  "  <chapter>";
    xmlText +=  "    <title>A Namespace Called fnord</title>";
    xmlText +=  "  </chapter>";
    xmlText +=  "</book>";
    xmlText +=  "</document>";

    KoXmlDocument doc;
    KoXmlElement rootElement;
    KoXmlElement bookElement;
    KoXmlElement bookTitleElement;
    KoXmlElement bookAuthorElement;

    QCOMPARE(doc.setContent(xmlText, true, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    const char* defaultNS = "http://trolltech.com/fnord/";
    const char* bookNS = "http://trolltech.com/fnord/book/";
    const char* fnordNS = "http://trolltech.com/fnord/";

    // <document>
    rootElement = doc.documentElement();
    QCOMPARE(rootElement.isNull(), false);
    QCOMPARE(rootElement.isElement(), true);
    QCOMPARE(rootElement.tagName(), QString("document"));
    QCOMPARE(rootElement.prefix().isEmpty(), true);
    QCOMPARE(rootElement.namespaceURI(), QString(defaultNS));
    QCOMPARE(rootElement.localName(), QString("document"));

    // <book>
    bookElement = rootElement.firstChild().toElement();
    QCOMPARE(bookElement.isNull(), false);
    QCOMPARE(bookElement.isElement(), true);
    QCOMPARE(bookElement.tagName(), QString("book"));
    QCOMPARE(bookElement.prefix().isEmpty(), true);
    QCOMPARE(bookElement.namespaceURI(), QString(defaultNS));
    QCOMPARE(bookElement.localName(), QString("book"));

    // <book:title>
    bookTitleElement = bookElement.firstChild().toElement();
    QCOMPARE(bookTitleElement.isNull(), false);
    QCOMPARE(bookTitleElement.isElement(), true);
    QCOMPARE(bookTitleElement.tagName(), QString("title"));
    QCOMPARE(bookTitleElement.prefix(), QString("book"));
    QCOMPARE(bookTitleElement.namespaceURI(), QString(bookNS));
    QCOMPARE(bookTitleElement.localName(), QString("title"));

    // another way, find it using namedItemNS()
    KoXmlElement book2TitleElement;
    book2TitleElement = KoXml::namedItemNS(rootElement.firstChild(), bookNS, "title");
    //book2TitleElement = bookElement.namedItemNS( bookNS, "title" ).toElement();
    QCOMPARE(book2TitleElement == bookTitleElement, true);
    QCOMPARE(book2TitleElement.isNull(), false);
    QCOMPARE(book2TitleElement.isElement(), true);
    QCOMPARE(book2TitleElement.tagName(), QString("title"));

    // <book:author>
    bookAuthorElement = bookTitleElement.nextSibling().toElement();
    QCOMPARE(bookAuthorElement.isNull(), false);
    QCOMPARE(bookAuthorElement.isElement(), true);
    QCOMPARE(bookAuthorElement.tagName(), QString("author"));
    QCOMPARE(bookAuthorElement.prefix(), QString("book"));
    QCOMPARE(bookAuthorElement.namespaceURI(), QString(bookNS));
    QCOMPARE(bookAuthorElement.localName(), QString("author"));

    // another way, find it using namedItemNS()
    KoXmlElement book2AuthorElement;
    book2AuthorElement = KoXml::namedItemNS(bookElement, bookNS, "author");
    //book2AuthorElement = bookElement.namedItemNS( bookNS, "author" ).toElement();
    QCOMPARE(book2AuthorElement == bookAuthorElement, true);
    QCOMPARE(book2AuthorElement.isNull(), false);
    QCOMPARE(book2AuthorElement.isElement(), true);
    QCOMPARE(book2AuthorElement.tagName(), QString("author"));

    // attributes in <book:author>
    // Note: with namespace processing, attribute's prefix is taken out and
    // hence "fnord:title" will simply override "title"
    // and searching attribute with prefix will give no result
    QCOMPARE(bookAuthorElement.hasAttribute("title"), true);
    QCOMPARE(bookAuthorElement.hasAttribute("fnord:title"), false);
    QCOMPARE(bookAuthorElement.hasAttribute("name"), true);
    QCOMPARE(bookAuthorElement.attribute("title"), QString("Goddess"));
    QCOMPARE(bookAuthorElement.attribute("fnord:title").isEmpty(), true);
    QCOMPARE(bookAuthorElement.attribute("name"), QString("Eris Kallisti"));

    // attributes in <book:author>, with NS family of functions
    // those without prefix are not accessible at all, because they do not belong
    // to any namespace at all.
    // Note: default namespace does not apply to attribute names!
    QCOMPARE(bookAuthorElement.hasAttributeNS(defaultNS, "title"), true);
    QCOMPARE(bookAuthorElement.hasAttributeNS(bookNS, "title"), false);
    QCOMPARE(bookAuthorElement.hasAttributeNS(fnordNS, "title"), true);

    QCOMPARE(bookAuthorElement.attributeNS(defaultNS, "title", ""), QString("Goddess"));
    QCOMPARE(bookAuthorElement.attributeNS(bookNS, "title", ""), QString(""));
    QCOMPARE(bookAuthorElement.attributeNS(fnordNS, "title", ""), QString("Goddess"));

    QCOMPARE(bookAuthorElement.hasAttributeNS(defaultNS, "fnord:title"), false);
    QCOMPARE(bookAuthorElement.hasAttributeNS(bookNS, "fnord:title"), false);
    QCOMPARE(bookAuthorElement.hasAttributeNS(fnordNS, "fnord:title"), false);

    QCOMPARE(bookAuthorElement.hasAttributeNS(defaultNS, "name"), false);
    QCOMPARE(bookAuthorElement.hasAttributeNS(bookNS, "name"), false);
    QCOMPARE(bookAuthorElement.hasAttributeNS(fnordNS, "name"), false);

    QCOMPARE(bookAuthorElement.attributeNS(defaultNS, "name", QString()).isEmpty(), true);
    QCOMPARE(bookAuthorElement.attributeNS(bookNS, "name", QString()).isEmpty(), true);
    QCOMPARE(bookAuthorElement.attributeNS(fnordNS, "name", QString()).isEmpty(), true);
}

void TestXmlReader::testUnload()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);

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
    xmldevice.close();

    KoXmlDocument doc;
    QCOMPARE(doc.setContent(&xmldevice, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    KoXmlElement earthElement;
    earthElement = doc.documentElement().toElement();
    QCOMPARE(earthElement.isNull(), false);
    QCOMPARE(earthElement.isElement(), true);
    QCOMPARE(earthElement.parentNode().isNull(), false);
    QCOMPARE(earthElement.hasChildNodes(), true);
    QCOMPARE(KoXml::childNodesCount(earthElement), 2);
    QCOMPARE(earthElement.tagName(), QString("earth"));
    QCOMPARE(earthElement.prefix().isNull(), true);

    // this ensures that all child nodes of <earth> are loaded
    earthElement.firstChild();

    // explicitly unload all child nodes of <earth>
    KoXml::unload(earthElement);

    // we should get the correct first child
    KoXmlElement continentsElement = earthElement.firstChild().toElement();
    QCOMPARE(continentsElement.nodeName(), QString("continents"));
    QCOMPARE(continentsElement.isNull(), false);
    QCOMPARE(continentsElement.isElement(), true);
    QCOMPARE(continentsElement.isText(), false);
    QCOMPARE(continentsElement.isDocument(), false);
    QCOMPARE(continentsElement.hasChildNodes(), true);
    QCOMPARE(KoXml::childNodesCount(continentsElement), 6);

    // let us unload everything again
    KoXml::unload(earthElement);

    // we should get the correct last child
    KoXmlElement oceansElement = earthElement.lastChild().toElement();
    QCOMPARE(oceansElement.nodeName(), QString("oceans"));
    QCOMPARE(oceansElement.isNull(), false);
    QCOMPARE(oceansElement.isElement(), true);
    QCOMPARE(oceansElement.isText(), false);
    QCOMPARE(oceansElement.isDocument(), false);
    QCOMPARE(oceansElement.hasChildNodes(), true);
    QCOMPARE(KoXml::childNodesCount(continentsElement), 6);
}

void TestXmlReader::testSimpleXML()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);
    xmlstream << "<solarsystem>";
    xmlstream << "  <mercurius/>\n";
    xmlstream << "  <venus/>\n";
    xmlstream << "  <earth>\n";
    xmlstream << "    <moon/>\n";
    xmlstream << "  </earth>\n";
    xmlstream << "  <mars/>\n";
    xmlstream << "  <jupiter/>\n";
    xmlstream << "</solarsystem>";
    xmldevice.close();

    KoXmlDocument doc;
    QCOMPARE(doc.setContent(&xmldevice, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    // <solarsystem>
    KoXmlElement rootElement;
    rootElement = doc.documentElement();
    QCOMPARE(rootElement.isNull(), false);
    QCOMPARE(rootElement.isElement(), true);
    QCOMPARE(rootElement.parentNode().isNull(), false);
    QCOMPARE(rootElement.hasChildNodes(), true);
    QCOMPARE(KoXml::childNodesCount(rootElement), 5);
    QCOMPARE(rootElement.tagName(), QString("solarsystem"));
    QCOMPARE(rootElement.prefix().isNull(), true);

    // node <mercurius>
    KoXmlNode firstPlanetNode;
    firstPlanetNode = rootElement.firstChild();
    QCOMPARE(firstPlanetNode.isNull(), false);
    QCOMPARE(firstPlanetNode.isElement(), true);
    QCOMPARE(firstPlanetNode.nextSibling().isNull(), false);
    QCOMPARE(firstPlanetNode.previousSibling().isNull(), true);
    QCOMPARE(firstPlanetNode.parentNode().isNull(), false);
    QCOMPARE(firstPlanetNode.parentNode() == rootElement, true);
    QCOMPARE(firstPlanetNode.parentNode() != rootElement, false);
    QCOMPARE(firstPlanetNode.hasChildNodes(), false);
    QCOMPARE(KoXml::childNodesCount(firstPlanetNode), 0);
    QCOMPARE(firstPlanetNode.firstChild().isNull(), true);
    QCOMPARE(firstPlanetNode.lastChild().isNull(), true);

    // element <mercurius>
    KoXmlElement firstPlanetElement;
    firstPlanetElement = firstPlanetNode.toElement();
    QCOMPARE(firstPlanetElement.isNull(), false);
    QCOMPARE(firstPlanetElement.isElement(), true);
    QCOMPARE(firstPlanetElement.parentNode().isNull(), false);
    QCOMPARE(firstPlanetElement.parentNode() == rootElement, true);
    QCOMPARE(firstPlanetElement.hasChildNodes(), false);
    QCOMPARE(KoXml::childNodesCount(firstPlanetNode), 0);
    QCOMPARE(firstPlanetElement.firstChild().isNull(), true);
    QCOMPARE(firstPlanetElement.lastChild().isNull(), true);
    QCOMPARE(firstPlanetElement.tagName(), QString("mercurius"));
    QCOMPARE(firstPlanetElement.prefix().isNull(), true);

    // node <venus>
    KoXmlNode secondPlanetNode;
    secondPlanetNode = firstPlanetNode.nextSibling();
    QCOMPARE(secondPlanetNode.isNull(), false);
    QCOMPARE(secondPlanetNode.isElement(), true);
    QCOMPARE(secondPlanetNode.nextSibling().isNull(), false);
    QCOMPARE(secondPlanetNode.previousSibling().isNull(), false);
    QCOMPARE(secondPlanetNode.previousSibling() == firstPlanetNode, true);
    QCOMPARE(secondPlanetNode.previousSibling() == firstPlanetElement, true);
    QCOMPARE(secondPlanetNode.parentNode().isNull(), false);
    QCOMPARE(secondPlanetNode.parentNode() == rootElement, true);
    QCOMPARE(secondPlanetNode.parentNode() != rootElement, false);
    QCOMPARE(secondPlanetNode.hasChildNodes(), false);
    QCOMPARE(KoXml::childNodesCount(secondPlanetNode), 0);
    QCOMPARE(secondPlanetNode.firstChild().isNull(), true);
    QCOMPARE(secondPlanetNode.lastChild().isNull(), true);

    // element <venus>
    KoXmlElement secondPlanetElement;
    secondPlanetElement = secondPlanetNode.toElement();
    QCOMPARE(secondPlanetElement.isNull(), false);
    QCOMPARE(secondPlanetElement.isElement(), true);
    QCOMPARE(secondPlanetElement.nextSibling().isNull(), false);
    QCOMPARE(secondPlanetElement.previousSibling().isNull(), false);
    QCOMPARE(secondPlanetElement.previousSibling() == firstPlanetNode, true);
    QCOMPARE(secondPlanetElement.previousSibling() == firstPlanetElement, true);
    QCOMPARE(secondPlanetElement.parentNode().isNull(), false);
    QCOMPARE(secondPlanetElement.parentNode() == rootElement, true);
    QCOMPARE(secondPlanetElement.parentNode() != rootElement, false);
    QCOMPARE(secondPlanetElement.hasChildNodes(), false);
    QCOMPARE(KoXml::childNodesCount(secondPlanetNode), 0);
    QCOMPARE(secondPlanetElement.firstChild().isNull(), true);
    QCOMPARE(secondPlanetElement.lastChild().isNull(), true);
    QCOMPARE(secondPlanetElement.tagName(), QString("venus"));
    QCOMPARE(secondPlanetElement.prefix().isNull(), true);
}

void TestXmlReader::testRootError()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    // multiple root nodes are not valid !
    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);
    xmlstream << "<earth></earth><moon></moon>";
    xmldevice.close();

    KoXmlDocument doc;
    QCOMPARE(doc.setContent(&xmldevice, &errorMsg, &errorLine, &errorColumn), false);
    QCOMPARE(errorMsg.isEmpty(), false);
    QCOMPARE(errorMsg, QString("unexpected character"));
    QCOMPARE(errorLine, 1);
    QCOMPARE(errorColumn, 21);
}

void TestXmlReader::testMismatchedTag()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);
    xmlstream << "<earth></e>";
    xmldevice.close();

    KoXmlDocument doc;
    QCOMPARE(doc.setContent(&xmldevice, &errorMsg, &errorLine, &errorColumn), false);
    QCOMPARE(errorMsg.isEmpty(), false);
    QCOMPARE(errorMsg, QString("tag mismatch"));
    QCOMPARE(errorLine, 1);
    QCOMPARE(errorColumn, 11);
}

void TestXmlReader::testConvertQDomElement()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);
    xmlstream << "<solarsystem star=\"sun\">";
    xmlstream << "  <mercurius/>\n";
    xmlstream << "  <venus/>\n";
    xmlstream << "  <earth habitable=\"true\"><p>The best place</p>";
    xmlstream << "    <moon  habitable=\"possible\"/>\n";
    xmlstream << "  </earth>\n";
    xmlstream << "  <mars/>\n";
    xmlstream << "  <jupiter/>\n";
    xmlstream << "</solarsystem>";
    xmldevice.close();

    KoXmlDocument doc;
    QCOMPARE(doc.setContent(&xmldevice, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    // <solarsystem>
    KoXmlElement rootElement;
    rootElement = doc.documentElement();
    QCOMPARE(rootElement.isNull(), false);
    QCOMPARE(rootElement.isElement(), true);
    QCOMPARE(rootElement.parentNode().isNull(), false);
    QCOMPARE(rootElement.hasChildNodes(), true);
    QCOMPARE(KoXml::childNodesCount(rootElement), 5);
    QCOMPARE(rootElement.tagName(), QString("solarsystem"));
    QCOMPARE(rootElement.prefix().isNull(), true);

    // now test converting KoXmlElement to QDomElement
    QDomDocument universeDoc;
    QDomElement universeRoot = universeDoc.createElement("universe");
    universeDoc.appendChild(universeRoot);
    universeRoot.appendChild(KoXml::asQDomNode(universeDoc, rootElement));

    // <solarsystem>
    QDomElement solarSystemElement = universeRoot.firstChild().toElement();
    QCOMPARE(solarSystemElement.isNull(), false);
    QCOMPARE(solarSystemElement.isElement(), true);
    QCOMPARE(solarSystemElement.parentNode().isNull(), false);
    QCOMPARE(solarSystemElement.hasChildNodes(), true);
    QCOMPARE(solarSystemElement.tagName(), QString("solarsystem"));
    QCOMPARE(solarSystemElement.prefix().isNull(), true);

    // <earth>
    QDomElement earthElement = solarSystemElement.namedItem("earth").toElement();
    QCOMPARE(earthElement.isNull(), false);
    QCOMPARE(earthElement.isElement(), true);
    QCOMPARE(earthElement.parentNode().isNull(), false);
    QCOMPARE(earthElement.hasAttribute("habitable"), true);
    QCOMPARE(earthElement.hasChildNodes(), true);
    QCOMPARE(earthElement.tagName(), QString("earth"));
    QCOMPARE(earthElement.prefix().isNull(), true);

    // <p> in <earth>
    QDomNode placeNode = earthElement.firstChild();
    QCOMPARE(placeNode.isNull(), false);
    QCOMPARE(placeNode.isElement(), true);
    QCOMPARE(placeNode.toElement().text(), QString("The best place"));
    QCOMPARE(placeNode.nextSibling().isNull(), false);
    QCOMPARE(placeNode.previousSibling().isNull(), true);
    QCOMPARE(placeNode.parentNode().isNull(), false);
    QCOMPARE(placeNode.parentNode() == earthElement, true);
    QCOMPARE(placeNode.hasChildNodes(), true);
    QCOMPARE(placeNode.childNodes().count(), 1);

    //printf("Result:\n%s\n\n", qPrintable(universeDoc.toString()));
}

void TestXmlReader::testSimpleOpenDocumentText()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);

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
    xmldevice.close();

    KoXmlDocument doc;
    QCOMPARE(doc.setContent(&xmldevice, true, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    const char* officeNS = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
    const char* textNS = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";

    // <office:document-content>
    KoXmlElement contentElement;
    contentElement = doc.documentElement();
    QCOMPARE(contentElement.isNull(), false);
    QCOMPARE(contentElement.isElement(), true);
    QCOMPARE(contentElement.parentNode().isNull(), false);
    QCOMPARE(contentElement.parentNode().toDocument() == doc, true);
    QCOMPARE(KoXml::childNodesCount(contentElement), 2);
    QCOMPARE(contentElement.firstChild().isNull(), false);
    QCOMPARE(contentElement.lastChild().isNull(), false);
    QCOMPARE(contentElement.previousSibling().isNull(), false);
    QCOMPARE(contentElement.nextSibling().isNull(), true);
    QCOMPARE(contentElement.localName(), QString("document-content"));
    QCOMPARE(contentElement.hasAttributeNS(officeNS, "version"), true);
    QCOMPARE(contentElement.attributeNS(officeNS, "version", ""), QString("1.0"));

    // <office:automatic-styles>
    KoXmlElement stylesElement;
    stylesElement = KoXml::namedItemNS(contentElement, officeNS, "automatic-styles");
    QCOMPARE(stylesElement.isNull(), false);
    QCOMPARE(stylesElement.isElement(), true);
    QCOMPARE(stylesElement.parentNode().isNull(), false);
    QCOMPARE(stylesElement.parentNode() == contentElement, true);
    QCOMPARE(KoXml::childNodesCount(stylesElement), 0);
    QCOMPARE(stylesElement.firstChild().isNull(), true);
    QCOMPARE(stylesElement.lastChild().isNull(), true);
    QCOMPARE(stylesElement.previousSibling().isNull(), true);
    QCOMPARE(stylesElement.nextSibling().isNull(), false);
    QCOMPARE(stylesElement.localName(), QString("automatic-styles"));

    // also same <office:automatic-styles>, but without namedItemNS
    KoXmlNode styles2Element;
    styles2Element = contentElement.firstChild().toElement();
    QCOMPARE(styles2Element.isNull(), false);
    QCOMPARE(styles2Element.isElement(), true);
    QCOMPARE(styles2Element.parentNode().isNull(), false);
    QCOMPARE(styles2Element.parentNode() == contentElement, true);
    QCOMPARE(KoXml::childNodesCount(styles2Element), 0);
    QCOMPARE(styles2Element.firstChild().isNull(), true);
    QCOMPARE(styles2Element.lastChild().isNull(), true);
    QCOMPARE(styles2Element.previousSibling().isNull(), true);
    QCOMPARE(styles2Element.nextSibling().isNull(), false);
    QCOMPARE(styles2Element.localName(), QString("automatic-styles"));

    // <office:body>
    KoXmlElement bodyElement;
    bodyElement = KoXml::namedItemNS(contentElement, officeNS, "body");
    QCOMPARE(bodyElement.isNull(), false);
    QCOMPARE(bodyElement.isElement(), true);
    QCOMPARE(bodyElement.parentNode().isNull(), false);
    QCOMPARE(bodyElement.parentNode() == contentElement, true);
    QCOMPARE(KoXml::childNodesCount(bodyElement), 1);
    QCOMPARE(bodyElement.firstChild().isNull(), false);
    QCOMPARE(bodyElement.lastChild().isNull(), false);
    QCOMPARE(bodyElement.previousSibling().isNull(), false);
    QCOMPARE(bodyElement.nextSibling().isNull(), true);
    QCOMPARE(bodyElement.localName(), QString("body"));

    // also same <office:body>, but without namedItemNS
    KoXmlElement body2Element;
    body2Element = stylesElement.nextSibling().toElement();
    QCOMPARE(body2Element.isNull(), false);
    QCOMPARE(body2Element.isElement(), true);
    QCOMPARE(body2Element.parentNode().isNull(), false);
    QCOMPARE(body2Element.parentNode() == contentElement, true);
    QCOMPARE(KoXml::childNodesCount(body2Element), 1);
    QCOMPARE(body2Element.firstChild().isNull(), false);
    QCOMPARE(body2Element.lastChild().isNull(), false);
    QCOMPARE(body2Element.previousSibling().isNull(), false);
    QCOMPARE(body2Element.nextSibling().isNull(), true);
    QCOMPARE(body2Element.localName(), QString("body"));

    // <office:text>
    KoXmlElement textElement;
    textElement = KoXml::namedItemNS(bodyElement, officeNS, "text");
    QCOMPARE(textElement.isNull(), false);
    QCOMPARE(textElement.isElement(), true);
    QCOMPARE(textElement.parentNode().isNull(), false);
    QCOMPARE(textElement.parentNode() == bodyElement, true);
    QCOMPARE(KoXml::childNodesCount(textElement), 1);
    QCOMPARE(textElement.firstChild().isNull(), false);
    QCOMPARE(textElement.lastChild().isNull(), false);
    QCOMPARE(textElement.previousSibling().isNull(), true);
    QCOMPARE(textElement.nextSibling().isNull(), true);
    QCOMPARE(textElement.localName(), QString("text"));

    // the same <office:text>, but without namedItemNS
    KoXmlElement text2Element;
    text2Element = bodyElement.firstChild().toElement();
    QCOMPARE(text2Element.isNull(), false);
    QCOMPARE(text2Element.isElement(), true);
    QCOMPARE(text2Element.parentNode().isNull(), false);
    QCOMPARE(text2Element.parentNode() == bodyElement, true);
    QCOMPARE(KoXml::childNodesCount(text2Element), 1);
    QCOMPARE(text2Element.firstChild().isNull(), false);
    QCOMPARE(text2Element.lastChild().isNull(), false);
    QCOMPARE(text2Element.previousSibling().isNull(), true);
    QCOMPARE(text2Element.nextSibling().isNull(), true);
    QCOMPARE(text2Element.localName(), QString("text"));

    // <text:p>
    KoXmlElement parElement;
    parElement = textElement.firstChild().toElement();
    QCOMPARE(parElement.isNull(), false);
    QCOMPARE(parElement.isElement(), true);
    QCOMPARE(parElement.parentNode().isNull(), false);
    QCOMPARE(parElement.parentNode() == textElement, true);
    QCOMPARE(KoXml::childNodesCount(parElement), 1);
    QCOMPARE(parElement.firstChild().isNull(), false);
    QCOMPARE(parElement.lastChild().isNull(), false);
    QCOMPARE(parElement.previousSibling().isNull(), true);
    QCOMPARE(parElement.nextSibling().isNull(), true);
    QCOMPARE(parElement.tagName(), QString("p"));
    QCOMPARE(parElement.text(), QString("Hello, world!"));
    QCOMPARE(parElement.attributeNS(QString(textNS), "style-name", ""), QString("Standard"));
}

void TestXmlReader::testWhitespace()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);

    // content.xml for testing paragraphs with whitespace
    xmlstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    xmlstream << "<office:document-content ";
    xmlstream << " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\"";
    xmlstream << " xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\">";
    xmlstream << "   <text:p> </text:p>";
    xmlstream << "   <text:p> <text:span/> </text:p>";
    xmlstream << "</office:document-content>";
    xmldevice.close();

    KoXmlDocument doc;
    QCOMPARE(doc.setContent(&xmldevice, true, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    KoXmlElement p1;
    p1 = doc.documentElement().firstChild().toElement();
    QCOMPARE(p1.isNull(), false);
    QCOMPARE(p1.isElement(), true);
    QEXPECT_FAIL("", "Whitespace handling should be fixed.", Continue);
    QCOMPARE(KoXml::childNodesCount(p1), 1);

    KoXmlElement p2;
    p2 = p1.nextSibling().toElement();
    QCOMPARE(p2.isNull(), false);
    QCOMPARE(p2.isElement(), true);
    QEXPECT_FAIL("", "Whitespace handling should be fixed.", Continue);
    QCOMPARE(KoXml::childNodesCount(p2), 3);
}

void TestXmlReader::testSimpleOpenDocumentSpreadsheet()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);

    // content.xml from a simple OpenDocument spreadsheet
    // the document has three worksheets, the last two are empty.
    // on the first sheet, cell A1 contains the text "Hello, world".
    // automatic styles, font declarations and unnecessary namespaces are omitted.

    xmlstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    xmlstream << "<office:document-content ";
    xmlstream << "xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" ";
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
    xmldevice.close();

    KoXmlDocument doc;
    QCOMPARE(doc.setContent(&xmldevice, true, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    QString officeNS = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
    QString tableNS = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";
    QString textNS = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";

    // <office:document-content>
    KoXmlElement contentElement;
    contentElement = doc.documentElement();
    QCOMPARE(contentElement.isNull(), false);
    QCOMPARE(contentElement.isElement(), true);
    QCOMPARE(contentElement.parentNode().isNull(), false);
    QCOMPARE(contentElement.parentNode().toDocument() == doc, true);
    QCOMPARE(KoXml::childNodesCount(contentElement), 1);
    QCOMPARE(contentElement.firstChild().isNull(), false);
    QCOMPARE(contentElement.lastChild().isNull(), false);
    QCOMPARE(contentElement.previousSibling().isNull(), false);
    QCOMPARE(contentElement.nextSibling().isNull(), true);
    QCOMPARE(contentElement.localName(), QString("document-content"));

    // <office:body>
    KoXmlElement bodyElement;
    bodyElement = contentElement.firstChild().toElement();
    QCOMPARE(bodyElement.isNull(), false);
    QCOMPARE(bodyElement.isElement(), true);
    QCOMPARE(bodyElement.parentNode().isNull(), false);
    QCOMPARE(bodyElement.parentNode() == contentElement, true);
    QCOMPARE(KoXml::childNodesCount(bodyElement), 1);
    QCOMPARE(bodyElement.firstChild().isNull(), false);
    QCOMPARE(bodyElement.lastChild().isNull(), false);
    QCOMPARE(bodyElement.previousSibling().isNull(), true);
    QCOMPARE(bodyElement.nextSibling().isNull(), true);
    QCOMPARE(bodyElement.localName(), QString("body"));

    // <office:spreadsheet>
    KoXmlElement spreadsheetElement;
    spreadsheetElement = bodyElement.firstChild().toElement();
    QCOMPARE(spreadsheetElement.isNull(), false);
    QCOMPARE(spreadsheetElement.isElement(), true);
    QCOMPARE(spreadsheetElement.parentNode().isNull(), false);
    QCOMPARE(spreadsheetElement.parentNode() == bodyElement, true);
    QCOMPARE(KoXml::childNodesCount(spreadsheetElement), 3);
    QCOMPARE(spreadsheetElement.firstChild().isNull(), false);
    QCOMPARE(spreadsheetElement.lastChild().isNull(), false);
    QCOMPARE(spreadsheetElement.previousSibling().isNull(), true);
    QCOMPARE(spreadsheetElement.nextSibling().isNull(), true);
    QCOMPARE(spreadsheetElement.localName(), QString("spreadsheet"));

    // <table:table> for Sheet1
    KoXmlElement sheet1Element;
    sheet1Element = spreadsheetElement.firstChild().toElement();
    QCOMPARE(sheet1Element.isNull(), false);
    QCOMPARE(sheet1Element.isElement(), true);
    QCOMPARE(sheet1Element.parentNode().isNull(), false);
    QCOMPARE(sheet1Element.parentNode() == spreadsheetElement, true);
    QCOMPARE(KoXml::childNodesCount(sheet1Element), 2);
    QCOMPARE(sheet1Element.firstChild().isNull(), false);
    QCOMPARE(sheet1Element.lastChild().isNull(), false);
    QCOMPARE(sheet1Element.previousSibling().isNull(), true);
    QCOMPARE(sheet1Element.nextSibling().isNull(), false);
    QCOMPARE(sheet1Element.tagName(), QString("table"));
    QCOMPARE(sheet1Element.hasAttributeNS(tableNS, "name"), true);
    QCOMPARE(sheet1Element.attributeNS(tableNS, "name", ""), QString("Sheet1"));
    QCOMPARE(sheet1Element.attributeNS(tableNS, "style-name", ""), QString("ta1"));
    QCOMPARE(sheet1Element.attributeNS(tableNS, "print", ""), QString("false"));

    //  KoXml::load( sheet1Element, 100 );

    // <table:table-column>
    KoXmlElement columnElement;
    columnElement = sheet1Element.firstChild().toElement();
    QCOMPARE(columnElement.isNull(), false);
    QCOMPARE(columnElement.isElement(), true);
    QCOMPARE(columnElement.parentNode().isNull(), false);
    QCOMPARE(columnElement.parentNode() == sheet1Element, true);
    QCOMPARE(KoXml::childNodesCount(columnElement), 0);
    QCOMPARE(columnElement.firstChild().isNull(), true);
    QCOMPARE(columnElement.lastChild().isNull(), true);
    QCOMPARE(columnElement.previousSibling().isNull(), true);
    QCOMPARE(columnElement.nextSibling().isNull(), false);
    QCOMPARE(columnElement.tagName(), QString("table-column"));
    QCOMPARE(columnElement.attributeNS(tableNS, "style-name", ""), QString("co1"));
    QCOMPARE(columnElement.attributeNS(tableNS, "default-cell-style-name", ""), QString("Default"));

    // <table:table-row>
    KoXmlElement rowElement;
    rowElement = columnElement.nextSibling().toElement();
    QCOMPARE(rowElement.isNull(), false);
    QCOMPARE(rowElement.isElement(), true);
    QCOMPARE(rowElement.parentNode().isNull(), false);
    QCOMPARE(rowElement.parentNode() == sheet1Element, true);
    QCOMPARE(KoXml::childNodesCount(rowElement), 1);
    QCOMPARE(rowElement.firstChild().isNull(), false);
    QCOMPARE(rowElement.lastChild().isNull(), false);
    QCOMPARE(rowElement.previousSibling().isNull(), false);
    QCOMPARE(rowElement.nextSibling().isNull(), true);
    QCOMPARE(rowElement.tagName(), QString("table-row"));
    QCOMPARE(rowElement.attributeNS(tableNS, "style-name", ""), QString("ro1"));

    // <table:table-cell>
    KoXmlElement cellElement;
    cellElement = rowElement.firstChild().toElement();
    QCOMPARE(cellElement.isNull(), false);
    QCOMPARE(cellElement.isElement(), true);
    QCOMPARE(cellElement.parentNode().isNull(), false);
    QCOMPARE(cellElement.parentNode() == rowElement, true);
    QCOMPARE(KoXml::childNodesCount(cellElement), 1);
    QCOMPARE(cellElement.firstChild().isNull(), false);
    QCOMPARE(cellElement.lastChild().isNull(), false);
    QCOMPARE(cellElement.previousSibling().isNull(), true);
    QCOMPARE(cellElement.nextSibling().isNull(), true);
    QCOMPARE(cellElement.tagName(), QString("table-cell"));
    QCOMPARE(cellElement.attributeNS(officeNS, "value-type", ""), QString("string"));

    // <text:p>
    KoXmlElement parElement;
    parElement = cellElement.firstChild().toElement();
    QCOMPARE(parElement.isNull(), false);
    QCOMPARE(parElement.isElement(), true);
    QCOMPARE(parElement.parentNode().isNull(), false);
    QCOMPARE(parElement.parentNode() == cellElement, true);
    QCOMPARE(KoXml::childNodesCount(parElement), 1);
    QCOMPARE(parElement.firstChild().isNull(), false);
    QCOMPARE(parElement.lastChild().isNull(), false);
    QCOMPARE(parElement.previousSibling().isNull(), true);
    QCOMPARE(parElement.nextSibling().isNull(), true);
    QCOMPARE(parElement.tagName(), QString("p"));
    QCOMPARE(parElement.text(), QString("Hello, world"));

    // <table:table> for Sheet2
    KoXmlElement sheet2Element;
    sheet2Element = sheet1Element.nextSibling().toElement();
    QCOMPARE(sheet2Element.isNull(), false);
    QCOMPARE(sheet2Element.isElement(), true);
    QCOMPARE(sheet2Element.parentNode().isNull(), false);
    QCOMPARE(sheet2Element.parentNode() == spreadsheetElement, true);
    QCOMPARE(KoXml::childNodesCount(sheet2Element), 2);
    QCOMPARE(sheet2Element.firstChild().isNull(), false);
    QCOMPARE(sheet2Element.lastChild().isNull(), false);
    QCOMPARE(sheet2Element.previousSibling().isNull(), false);
    QCOMPARE(sheet2Element.nextSibling().isNull(), false);
    QCOMPARE(sheet2Element.tagName(), QString("table"));

    // </table:table> for Sheet3
    KoXmlElement sheet3Element;
    sheet3Element = sheet2Element.nextSibling().toElement();
    QCOMPARE(sheet3Element.isNull(), false);
    QCOMPARE(sheet3Element.isElement(), true);
    QCOMPARE(sheet3Element.parentNode().isNull(), false);
    QCOMPARE(sheet3Element.parentNode() == spreadsheetElement, true);
    QCOMPARE(KoXml::childNodesCount(sheet3Element), 2);
    QCOMPARE(sheet3Element.firstChild().isNull(), false);
    QCOMPARE(sheet3Element.lastChild().isNull(), false);
    QCOMPARE(sheet3Element.previousSibling().isNull(), false);
    QCOMPARE(sheet3Element.nextSibling().isNull(), true);
    QCOMPARE(sheet3Element.tagName(), QString("table"));
}

void TestXmlReader::testSimpleOpenDocumentPresentation()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);

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
    xmldevice.close();

    KoXmlDocument doc;
    QCOMPARE(doc.setContent(&xmldevice, true, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    const char* officeNS = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
    const char* drawNS = "urn:oasis:names:tc:opendocument:xmlns:drawing:1.0";
    const char* textNS = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";
    const char* presentationNS = "urn:oasis:names:tc:opendocument:xmlns:presentation:1.0";
    const char* svgNS = "urn:oasis:names:tc:opendocument:xmlns:svg-compatible:1.0";

    // <office:document-content>
    KoXmlElement contentElement;
    contentElement = doc.documentElement();
    QCOMPARE(contentElement.isNull(), false);

    QCOMPARE(contentElement.isElement(), true);
    QCOMPARE(contentElement.parentNode().isNull(), false);
    QCOMPARE(contentElement.parentNode().toDocument() == doc, true);
    QCOMPARE(KoXml::childNodesCount(contentElement), 3);
    QCOMPARE(contentElement.firstChild().isNull(), false);
    QCOMPARE(contentElement.lastChild().isNull(), false);
    QCOMPARE(contentElement.previousSibling().isNull(), false);
    QCOMPARE(contentElement.nextSibling().isNull(), true);
    QCOMPARE(contentElement.localName(), QString("document-content"));
    QCOMPARE(contentElement.hasAttributeNS(officeNS, "version"), true);
    QCOMPARE(contentElement.attributeNS(officeNS, "version", ""), QString("1.0"));

    // <office:scripts>
    KoXmlElement scriptsElement;
    scriptsElement = KoXml::namedItemNS(contentElement, officeNS, "scripts");
    QCOMPARE(scriptsElement.isNull(), false);
    QCOMPARE(scriptsElement.isElement(), true);
    QCOMPARE(scriptsElement.parentNode().isNull(), false);
    QCOMPARE(scriptsElement.parentNode() == contentElement, true);
    QCOMPARE(KoXml::childNodesCount(scriptsElement), 0);
    QCOMPARE(scriptsElement.firstChild().isNull(), true);
    QCOMPARE(scriptsElement.lastChild().isNull(), true);
    QCOMPARE(scriptsElement.previousSibling().isNull(), true);
    QCOMPARE(scriptsElement.nextSibling().isNull(), false);
    QCOMPARE(scriptsElement.localName(), QString("scripts"));

    // <office:automatic-styles>
    KoXmlElement stylesElement;
    stylesElement = KoXml::namedItemNS(contentElement, officeNS, "automatic-styles");
    QCOMPARE(stylesElement.isNull(), false);
    QCOMPARE(stylesElement.isElement(), true);
    QCOMPARE(stylesElement.parentNode().isNull(), false);
    QCOMPARE(stylesElement.parentNode() == contentElement, true);
    QCOMPARE(KoXml::childNodesCount(stylesElement), 0);
    QCOMPARE(stylesElement.firstChild().isNull(), true);
    QCOMPARE(stylesElement.lastChild().isNull(), true);
    QCOMPARE(stylesElement.previousSibling().isNull(), false);
    QCOMPARE(stylesElement.nextSibling().isNull(), false);
    QCOMPARE(stylesElement.localName(), QString("automatic-styles"));

    // also same <office:automatic-styles>, but without namedItemNS
    KoXmlNode styles2Element;
    styles2Element = scriptsElement.nextSibling().toElement();
    QCOMPARE(styles2Element.isNull(), false);
    QCOMPARE(styles2Element.isElement(), true);
    QCOMPARE(styles2Element.parentNode().isNull(), false);
    QCOMPARE(styles2Element.parentNode() == contentElement, true);
    QCOMPARE(KoXml::childNodesCount(styles2Element), 0);
    QCOMPARE(styles2Element.firstChild().isNull(), true);
    QCOMPARE(styles2Element.lastChild().isNull(), true);
    QCOMPARE(styles2Element.previousSibling().isNull(), false);
    QCOMPARE(styles2Element.nextSibling().isNull(), false);
    QCOMPARE(styles2Element.localName(), QString("automatic-styles"));

    // <office:body>
    KoXmlElement bodyElement;
    bodyElement = KoXml::namedItemNS(contentElement, officeNS, "body");
    QCOMPARE(bodyElement.isNull(), false);
    QCOMPARE(bodyElement.isElement(), true);
    QCOMPARE(bodyElement.parentNode().isNull(), false);
    QCOMPARE(bodyElement.parentNode() == contentElement, true);
    QCOMPARE(KoXml::childNodesCount(bodyElement), 1);
    QCOMPARE(bodyElement.firstChild().isNull(), false);
    QCOMPARE(bodyElement.lastChild().isNull(), false);
    QCOMPARE(bodyElement.previousSibling().isNull(), false);
    QCOMPARE(bodyElement.nextSibling().isNull(), true);
    QCOMPARE(bodyElement.localName(), QString("body"));

    // also same <office:body>, but without namedItemNS
    KoXmlElement body2Element;
    body2Element = stylesElement.nextSibling().toElement();
    QCOMPARE(body2Element.isNull(), false);
    QCOMPARE(body2Element.isElement(), true);
    QCOMPARE(body2Element.parentNode().isNull(), false);
    QCOMPARE(body2Element.parentNode() == contentElement, true);
    QCOMPARE(KoXml::childNodesCount(body2Element), 1);
    QCOMPARE(body2Element.firstChild().isNull(), false);
    QCOMPARE(body2Element.lastChild().isNull(), false);
    QCOMPARE(body2Element.previousSibling().isNull(), false);
    QCOMPARE(body2Element.nextSibling().isNull(), true);
    QCOMPARE(body2Element.localName(), QString("body"));

    // <office:presentation>
    KoXmlElement presentationElement;
    presentationElement = KoXml::namedItemNS(bodyElement, officeNS, "presentation");
    QCOMPARE(presentationElement.isNull(), false);
    QCOMPARE(presentationElement.isElement(), true);
    QCOMPARE(presentationElement.parentNode().isNull(), false);
    QCOMPARE(presentationElement.parentNode() == bodyElement, true);
    QCOMPARE(KoXml::childNodesCount(presentationElement), 2);
    QCOMPARE(presentationElement.firstChild().isNull(), false);
    QCOMPARE(presentationElement.lastChild().isNull(), false);
    QCOMPARE(presentationElement.previousSibling().isNull(), true);
    QCOMPARE(presentationElement.nextSibling().isNull(), true);
    QCOMPARE(presentationElement.localName(), QString("presentation"));

    // the same <office:presentation>, but without namedItemNS
    KoXmlElement presentation2Element;
    presentation2Element = bodyElement.firstChild().toElement();
    QCOMPARE(presentation2Element.isNull(), false);
    QCOMPARE(presentation2Element.isElement(), true);
    QCOMPARE(presentation2Element.parentNode().isNull(), false);
    QCOMPARE(presentation2Element.parentNode() == bodyElement, true);
    QCOMPARE(KoXml::childNodesCount(presentation2Element), 2);
    QCOMPARE(presentation2Element.firstChild().isNull(), false);
    QCOMPARE(presentation2Element.lastChild().isNull(), false);
    QCOMPARE(presentation2Element.previousSibling().isNull(), true);
    QCOMPARE(presentation2Element.nextSibling().isNull(), true);
    QCOMPARE(presentation2Element.localName(), QString("presentation"));

    // <draw:page> for "Title"
    KoXmlElement titlePageElement;
    titlePageElement = presentationElement.firstChild().toElement();
    QCOMPARE(titlePageElement.isNull(), false);
    QCOMPARE(titlePageElement.isElement(), true);
    QCOMPARE(titlePageElement.parentNode().isNull(), false);
    QCOMPARE(titlePageElement.parentNode() == presentationElement, true);
    QCOMPARE(KoXml::childNodesCount(titlePageElement), 3);
    QCOMPARE(titlePageElement.firstChild().isNull(), false);
    QCOMPARE(titlePageElement.lastChild().isNull(), false);
    QCOMPARE(titlePageElement.previousSibling().isNull(), true);
    QCOMPARE(titlePageElement.nextSibling().isNull(), false);
    QCOMPARE(titlePageElement.localName(), QString("page"));
    QCOMPARE(titlePageElement.attributeNS(drawNS, "name", ""), QString("Title"));
    QCOMPARE(titlePageElement.attributeNS(drawNS, "style-name", ""), QString("dp1"));
    QCOMPARE(titlePageElement.attributeNS(drawNS, "master-page-name", ""), QString("lyt-cool"));
    QCOMPARE(titlePageElement.attributeNS(presentationNS,
                                       "presentation-page-layout-name", ""), QString("AL1T0"));

    // <draw:frame> for the title frame
    KoXmlElement titleFrameElement;
    titleFrameElement = titlePageElement.firstChild().toElement();
    QCOMPARE(titleFrameElement.isNull(), false);
    QCOMPARE(titleFrameElement.isElement(), true);
    QCOMPARE(titleFrameElement.parentNode().isNull(), false);
    QCOMPARE(titleFrameElement.parentNode() == titlePageElement, true);
    QCOMPARE(KoXml::childNodesCount(titleFrameElement), 1);
    QCOMPARE(titleFrameElement.firstChild().isNull(), false);
    QCOMPARE(titleFrameElement.lastChild().isNull(), false);
    QCOMPARE(titleFrameElement.previousSibling().isNull(), true);
    QCOMPARE(titleFrameElement.nextSibling().isNull(), false);
    QCOMPARE(titleFrameElement.localName(), QString("frame"));
    QCOMPARE(titleFrameElement.attributeNS(presentationNS, "style-name", ""), QString("pr1"));
    QCOMPARE(titleFrameElement.attributeNS(presentationNS, "class", ""), QString("title"));
    QCOMPARE(titleFrameElement.attributeNS(presentationNS, "user-transformed", ""), QString("true"));
    QCOMPARE(titleFrameElement.attributeNS(drawNS, "text-style-name", ""), QString("P2"));
    QCOMPARE(titleFrameElement.attributeNS(drawNS, "layer", ""), QString("layout"));
    QCOMPARE(titleFrameElement.attributeNS(svgNS, "width", ""), QString("23.912cm"));
    QCOMPARE(titleFrameElement.attributeNS(svgNS, "height", ""), QString("3.508cm"));
    QCOMPARE(titleFrameElement.attributeNS(svgNS, "x", ""), QString("2.058cm"));
    QCOMPARE(titleFrameElement.attributeNS(svgNS, "y", ""), QString("1.543cm"));

    // <draw:text-box> of the title frame
    KoXmlElement titleBoxElement;
    titleBoxElement = titleFrameElement.firstChild().toElement();
    QCOMPARE(titleBoxElement.isNull(), false);
    QCOMPARE(titleBoxElement.isElement(), true);
    QCOMPARE(titleBoxElement.parentNode().isNull(), false);
    QCOMPARE(titleBoxElement.parentNode() == titleFrameElement, true);
    QCOMPARE(KoXml::childNodesCount(titleBoxElement), 1);
    QCOMPARE(titleBoxElement.firstChild().isNull(), false);
    QCOMPARE(titleBoxElement.lastChild().isNull(), false);
    QCOMPARE(titleBoxElement.previousSibling().isNull(), true);
    QCOMPARE(titleBoxElement.nextSibling().isNull(), true);
    QCOMPARE(titleBoxElement.localName(), QString("text-box"));

    // <text:p> for the title text-box
    KoXmlElement titleParElement;
    titleParElement = titleBoxElement.firstChild().toElement();
    QCOMPARE(titleParElement.isNull(), false);
    QCOMPARE(titleParElement.isElement(), true);
    QCOMPARE(titleParElement.parentNode().isNull(), false);
    QCOMPARE(titleParElement.parentNode() == titleBoxElement, true);
    QCOMPARE(KoXml::childNodesCount(titleParElement), 1);
    QCOMPARE(titleParElement.firstChild().isNull(), false);
    QCOMPARE(titleParElement.lastChild().isNull(), false);
    QCOMPARE(titleParElement.previousSibling().isNull(), true);
    QCOMPARE(titleParElement.nextSibling().isNull(), true);
    QCOMPARE(titleParElement.localName(), QString("p"));
    QCOMPARE(titleParElement.attributeNS(textNS, "style-name", ""), QString("P1"));
    QCOMPARE(titleParElement.text(), QString("Foobar"));

    // <draw:frame> for the subtitle frame
    KoXmlElement subtitleFrameElement;
    subtitleFrameElement = titleFrameElement.nextSibling().toElement();
    QCOMPARE(subtitleFrameElement.isNull(), false);
    QCOMPARE(subtitleFrameElement.isElement(), true);
    QCOMPARE(subtitleFrameElement.parentNode().isNull(), false);
    QCOMPARE(subtitleFrameElement.parentNode() == titlePageElement, true);
    QCOMPARE(KoXml::childNodesCount(subtitleFrameElement), 1);
    QCOMPARE(subtitleFrameElement.firstChild().isNull(), false);
    QCOMPARE(subtitleFrameElement.lastChild().isNull(), false);
    QCOMPARE(subtitleFrameElement.previousSibling().isNull(), false);
    QCOMPARE(subtitleFrameElement.nextSibling().isNull(), false);
    QCOMPARE(subtitleFrameElement.localName(), QString("frame"));
    QCOMPARE(subtitleFrameElement.attributeNS(presentationNS, "style-name", ""), QString("pr2"));
    QCOMPARE(subtitleFrameElement.attributeNS(presentationNS, "class", ""), QString("subtitle"));
    QCOMPARE(subtitleFrameElement.hasAttributeNS(presentationNS, "user-transformed"), false);
    QCOMPARE(subtitleFrameElement.attributeNS(drawNS, "text-style-name", ""), QString("P3"));
    QCOMPARE(subtitleFrameElement.attributeNS(drawNS, "layer", ""), QString("layout"));
    QCOMPARE(subtitleFrameElement.attributeNS(svgNS, "width", ""), QString("23.912cm"));
    QCOMPARE(subtitleFrameElement.attributeNS(svgNS, "height", ""), QString("13.231cm"));
    QCOMPARE(subtitleFrameElement.attributeNS(svgNS, "x", ""), QString("2.058cm"));
    QCOMPARE(subtitleFrameElement.attributeNS(svgNS, "y", ""), QString("5.838cm"));

    // <draw:text-box> of the subtitle frame
    KoXmlElement subtitleBoxElement;
    subtitleBoxElement = subtitleFrameElement.firstChild().toElement();
    QCOMPARE(subtitleBoxElement.isNull(), false);
    QCOMPARE(subtitleBoxElement.isElement(), true);
    QCOMPARE(subtitleBoxElement.parentNode().isNull(), false);
    QCOMPARE(subtitleBoxElement.parentNode() == subtitleFrameElement, true);
    QCOMPARE(KoXml::childNodesCount(subtitleBoxElement), 1);
    QCOMPARE(subtitleBoxElement.firstChild().isNull(), false);
    QCOMPARE(subtitleBoxElement.lastChild().isNull(), false);
    QCOMPARE(subtitleBoxElement.previousSibling().isNull(), true);
    QCOMPARE(subtitleBoxElement.nextSibling().isNull(), true);
    QCOMPARE(subtitleBoxElement.localName(), QString("text-box"));

    // <text:p> for the subtitle text-box
    KoXmlElement subtitleParElement;
    subtitleParElement = subtitleBoxElement.firstChild().toElement();
    QCOMPARE(subtitleParElement.isNull(), false);
    QCOMPARE(subtitleParElement.isElement(), true);
    QCOMPARE(subtitleParElement.parentNode().isNull(), false);
    QCOMPARE(subtitleParElement.parentNode() == subtitleBoxElement, true);
    QCOMPARE(KoXml::childNodesCount(subtitleParElement), 1);
    QCOMPARE(subtitleParElement.firstChild().isNull(), false);
    QCOMPARE(subtitleParElement.lastChild().isNull(), false);
    QCOMPARE(subtitleParElement.previousSibling().isNull(), true);
    QCOMPARE(subtitleParElement.nextSibling().isNull(), true);
    QCOMPARE(subtitleParElement.localName(), QString("p"));
    QCOMPARE(subtitleParElement.attributeNS(textNS, "style-name", ""), QString("P3"));
    QCOMPARE(subtitleParElement.text(), QString("Foo"));
}

void TestXmlReader::testSimpleOpenDocumentFormula()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);

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
    xmldevice.close();

    KoXmlDocument doc;
    QCOMPARE(doc.setContent(&xmldevice, true, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    const char* mathNS = "http://www.w3.org/1998/Math/MathML";

    // <math:math>
    KoXmlElement mathElement;
    mathElement = doc.documentElement();
    QCOMPARE(mathElement.isNull(), false);
    QCOMPARE(mathElement.isElement(), true);
    QCOMPARE(mathElement.parentNode().isNull(), false);
    QCOMPARE(mathElement.parentNode().toDocument() == doc, true);
    QCOMPARE(mathElement.firstChild().isNull(), false);
    QCOMPARE(mathElement.lastChild().isNull(), false);
    QCOMPARE(mathElement.previousSibling().isNull(), false);
    QCOMPARE(mathElement.nextSibling().isNull(), true);
    QCOMPARE(mathElement.localName(), QString("math"));

    // <math:semantics>
    KoXmlElement semanticsElement;
    semanticsElement = KoXml::namedItemNS(mathElement, mathNS, "semantics");
    QCOMPARE(semanticsElement.isNull(), false);
    QCOMPARE(semanticsElement.isElement(), true);
    QCOMPARE(semanticsElement.parentNode().isNull(), false);
    QCOMPARE(semanticsElement.parentNode().toElement() == mathElement, true);
    QCOMPARE(semanticsElement.firstChild().isNull(), false);
    QCOMPARE(semanticsElement.lastChild().isNull(), false);
    QCOMPARE(semanticsElement.previousSibling().isNull(), true);
    QCOMPARE(semanticsElement.nextSibling().isNull(), true);
    QCOMPARE(semanticsElement.localName(), QString("semantics"));

    // the same <math:semantics> but without namedItemNS
    KoXmlElement semantics2Element;
    semantics2Element = mathElement.firstChild().toElement();
    QCOMPARE(semantics2Element.isNull(), false);
    QCOMPARE(semantics2Element.isElement(), true);
    QCOMPARE(semantics2Element.parentNode().isNull(), false);
    QCOMPARE(semantics2Element.parentNode().toElement() == mathElement, true);
    QCOMPARE(semantics2Element.firstChild().isNull(), false);
    QCOMPARE(semantics2Element.lastChild().isNull(), false);
    QCOMPARE(semantics2Element.previousSibling().isNull(), true);
    QCOMPARE(semantics2Element.nextSibling().isNull(), true);
    QCOMPARE(semantics2Element.localName(), QString("semantics"));

    // <math:mrow>
    KoXmlElement mrowElement;
    mrowElement = semanticsElement.firstChild().toElement();
    QCOMPARE(mrowElement.isNull(), false);
    QCOMPARE(mrowElement.isElement(), true);
    QCOMPARE(mrowElement.parentNode().isNull(), false);
    QCOMPARE(mrowElement.parentNode().toElement() == semanticsElement, true);
    QCOMPARE(mrowElement.firstChild().isNull(), false);
    QCOMPARE(mrowElement.lastChild().isNull(), false);
    QCOMPARE(mrowElement.previousSibling().isNull(), true);
    QCOMPARE(mrowElement.nextSibling().isNull(), false);
    QCOMPARE(mrowElement.localName(), QString("mrow"));

    // <math:mi> for "E"
    KoXmlElement miElement;
    miElement = mrowElement.firstChild().toElement();
    QCOMPARE(miElement.isNull(), false);
    QCOMPARE(miElement.isElement(), true);
    QCOMPARE(miElement.parentNode().isNull(), false);
    QCOMPARE(miElement.parentNode().toElement() == mrowElement, true);
    QCOMPARE(miElement.firstChild().isNull(), false);
    QCOMPARE(miElement.lastChild().isNull(), false);
    QCOMPARE(miElement.previousSibling().isNull(), true);
    QCOMPARE(miElement.nextSibling().isNull(), false);
    QCOMPARE(miElement.localName(), QString("mi"));

    // <math:mo> for "="
    KoXmlElement moElement;
    moElement = miElement.nextSibling().toElement();
    QCOMPARE(moElement.isNull(), false);
    QCOMPARE(moElement.isElement(), true);
    QCOMPARE(moElement.parentNode().isNull(), false);
    QCOMPARE(moElement.parentNode().toElement() == mrowElement, true);
    QCOMPARE(moElement.firstChild().isNull(), false);
    QCOMPARE(moElement.lastChild().isNull(), false);
    QCOMPARE(moElement.previousSibling().isNull(), false);
    QCOMPARE(moElement.nextSibling().isNull(), false);
    QCOMPARE(moElement.localName(), QString("mo"));
    QCOMPARE(moElement.attributeNS(mathNS, "stretchy", ""), QString("false"));

    // <math:msup> for "mc" and superscripted "2"
    KoXmlElement msupElement;
    msupElement = moElement.nextSibling().toElement();
    QCOMPARE(msupElement.isNull(), false);
    QCOMPARE(msupElement.isElement(), true);
    QCOMPARE(msupElement.parentNode().isNull(), false);
    QCOMPARE(msupElement.parentNode().toElement() == mrowElement, true);
    QCOMPARE(msupElement.firstChild().isNull(), false);
    QCOMPARE(msupElement.lastChild().isNull(), false);
    QCOMPARE(msupElement.previousSibling().isNull(), false);
    QCOMPARE(msupElement.nextSibling().isNull(), true);
    QCOMPARE(msupElement.localName(), QString("msup"));

    // <math:mi> inside the <math:msup> for "mc"
    KoXmlElement mcElement;
    mcElement = msupElement.firstChild().toElement();
    QCOMPARE(mcElement.isNull(), false);
    QCOMPARE(mcElement.isElement(), true);
    QCOMPARE(mcElement.parentNode().isNull(), false);
    QCOMPARE(mcElement.parentNode().toElement() == msupElement, true);
    QCOMPARE(mcElement.firstChild().isNull(), false);
    QCOMPARE(mcElement.lastChild().isNull(), false);
    QCOMPARE(mcElement.previousSibling().isNull(), true);
    QCOMPARE(mcElement.nextSibling().isNull(), false);
    QCOMPARE(mcElement.localName(), QString("mi"));
    QCOMPARE(mcElement.text(), QString("mc"));
    QCOMPARE(mcElement.attributeNS(mathNS, "fontstyle", ""), QString("italic"));

    // <math:mn> inside the <math:msup> for "2" (superscript)
    KoXmlElement mnElement;
    mnElement = mcElement.nextSibling().toElement();
    QCOMPARE(mnElement.isNull(), false);
    QCOMPARE(mnElement.isElement(), true);
    QCOMPARE(mnElement.parentNode().isNull(), false);
    QCOMPARE(mnElement.parentNode().toElement() == msupElement, true);
    QCOMPARE(mnElement.firstChild().isNull(), false);
    QCOMPARE(mnElement.lastChild().isNull(), false);
    QCOMPARE(mnElement.previousSibling().isNull(), false);
    QCOMPARE(mnElement.nextSibling().isNull(), true);
    QCOMPARE(mnElement.localName(), QString("mn"));
    QCOMPARE(mnElement.text(), QString("2"));

    // <math:annotation>
    KoXmlElement annotationElement;
    annotationElement = semanticsElement.lastChild().toElement();
    QCOMPARE(annotationElement.isNull(), false);
    QCOMPARE(annotationElement.isElement(), true);
    QCOMPARE(annotationElement.parentNode().isNull(), false);
    QCOMPARE(annotationElement.parentNode().toElement() == semanticsElement, true);
    QCOMPARE(annotationElement.firstChild().isNull(), false);
    QCOMPARE(annotationElement.lastChild().isNull(), false);
    QCOMPARE(annotationElement.previousSibling().isNull(), false);
    QCOMPARE(annotationElement.nextSibling().isNull(), true);
    QCOMPARE(annotationElement.localName(), QString("annotation"));
    QCOMPARE(annotationElement.text(), QString("E  =  mc^2 "));
    QCOMPARE(annotationElement.attributeNS(mathNS, "encoding", ""), QString("StarMath 5.0"));
}

void TestXmlReader::testLargeOpenDocumentSpreadsheet()
{
    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    int sheetCount = 4;
    int rowCount = 200;
    int colCount = 200 / 16;

    QBuffer xmldevice;
    xmldevice.open(QIODevice::WriteOnly);
    QTextStream xmlstream(&xmldevice);

    // content.xml
    xmlstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    xmlstream << "<office:document-content ";
    xmlstream << "xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\" ";
    xmlstream << "xmlns:text=\"urn:oasis:names:tc:opendocument:xmlns:text:1.0\" ";
    xmlstream << "xmlns:table=\"urn:oasis:names:tc:opendocument:xmlns:table:1.0\" >\n";
    xmlstream << "<office:body>\n";
    xmlstream << "<office:spreadsheet>\n";
    for (int i = 0; i < sheetCount; i++) {
        QString sheetName = QString("Sheet%1").arg(i + 1);
        xmlstream << "<table:table table:name=\"" << sheetName;
        xmlstream << "\" table:print=\"false\">\n";
        for (int j = 0; j < rowCount; j++) {
            xmlstream << "<table:table-row>\n";
            for (int k = 0; k < colCount; k++) {
                xmlstream << "<table:table-cell office:value-type=\"string\">";
                xmlstream << "<text:p>Hello, world</text:p>";
                xmlstream << "</table:table-cell>\n";
            }
            xmlstream << "</table:table-row>\n";
        }
        xmlstream << "</table:table>\n";
    }
    xmlstream << "</office:spreadsheet>\n";
    xmlstream << "</office:body>\n";
    xmlstream << "</office:document-content>\n";
    xmldevice.close();

    printf("Raw XML size: %lld KB\n", xmldevice.size() / 1024);


    QTime timer;

#if 0
    // just to test parsing speed with plain dumb handler
    QXmlStreamReader *reader = new QXmlStreamReader(xmldevice);
    reader->setNamespaceProcessing(true);
    timer.start();
    ParseError error = parseDocument(*reader, doc);
    printf("Large spreadsheet: QXmlStreamReader parsing time is %d ms\n", timer.elapsed());
    delete reader;
    xmldevice.seek(0);
#endif

    KoXmlDocument doc;

    timer.start();
    QCOMPARE(doc.setContent(&xmldevice, true, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    if (!errorMsg.isEmpty()) {
        qDebug("Error: %s", qPrintable(errorMsg));
        return;
    }

    printf("Large spreadsheet: KoXmlDocument parsing time is %d ms\n", timer.elapsed());

    // release memory taken by the XML document content
    //xmlstream.setDevice( 0 );

    // namespaces that will be used
    QString officeNS = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
    QString tableNS = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";
    QString textNS = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";

    // <office:document-content>
    KoXmlElement contentElement;
    contentElement = doc.documentElement();
    QCOMPARE(contentElement.isNull(), false);
    QCOMPARE(contentElement.isElement(), true);
    QCOMPARE(contentElement.localName(), QString("document-content"));

    // <office:body>
    KoXmlElement bodyElement;
    bodyElement = contentElement.firstChild().toElement();
    QCOMPARE(bodyElement.isNull(), false);
    QCOMPARE(bodyElement.isElement(), true);
    QCOMPARE(bodyElement.localName(), QString("body"));

    // <office:spreadsheet>
    KoXmlElement spreadsheetElement;
    spreadsheetElement = bodyElement.firstChild().toElement();
    QCOMPARE(spreadsheetElement.isNull(), false);
    QCOMPARE(spreadsheetElement.isElement(), true);
    QCOMPARE(spreadsheetElement.localName(), QString("spreadsheet"));

    // now we visit every sheet, every row, every cell
    timer.start();
    KoXmlElement tableElement;
    tableElement = spreadsheetElement.firstChild().toElement();
    for (int table = 0; table < sheetCount; table++) {
        QString tableName = QString("Sheet%1").arg(table + 1);
        QCOMPARE(tableElement.isNull(), false);
        QCOMPARE(tableElement.isElement(), true);
        QCOMPARE(tableElement.localName(), QString("table"));
        QCOMPARE(tableElement.hasAttributeNS(tableNS, "name"), true);
        QCOMPARE(tableElement.attributeNS(tableNS, "name", ""), tableName);
        QCOMPARE(tableElement.attributeNS(tableNS, "print", ""), QString("false"));

        // load everything for this table
        //KoXml::load( tableElement, 99 );

        QCOMPARE(tableElement.parentNode().isNull(), false);
        QCOMPARE(tableElement.parentNode() == spreadsheetElement, true);
        QCOMPARE(tableElement.firstChild().isNull(), false);
        QCOMPARE(tableElement.lastChild().isNull(), false);

        KoXmlElement rowElement;
        rowElement = tableElement.firstChild().toElement();
        for (int row = 0; row < rowCount; row++) {
            QCOMPARE(rowElement.isNull(), false);
            QCOMPARE(rowElement.isElement(), true);
            QCOMPARE(rowElement.localName(), QString("table-row"));
            QCOMPARE(rowElement.parentNode().isNull(), false);
            QCOMPARE(rowElement.parentNode() == tableElement, true);
            QCOMPARE(rowElement.firstChild().isNull(), false);
            QCOMPARE(rowElement.lastChild().isNull(), false);

            KoXmlElement cellElement;
            cellElement = rowElement.firstChild().toElement();
            for (int col = 0; col < colCount; col++) {
                QCOMPARE(cellElement.isNull(), false);
                QCOMPARE(cellElement.isElement(), true);
                QCOMPARE(cellElement.localName(), QString("table-cell"));
                QCOMPARE(cellElement.text(), QString("Hello, world"));
                QCOMPARE(cellElement.hasAttributeNS(officeNS, "value-type"), true);
                QCOMPARE(cellElement.attributeNS(officeNS, "value-type", ""), QString("string"));
                QCOMPARE(cellElement.parentNode().isNull(), false);
                QCOMPARE(cellElement.parentNode() == rowElement, true);
                QCOMPARE(cellElement.firstChild().isNull(), false);
                QCOMPARE(cellElement.lastChild().isNull(), false);
                cellElement = cellElement.nextSibling().toElement();
            }

            //KoXml::unload( rowElement );
            rowElement = rowElement.nextSibling().toElement();
        }

        KoXml::unload(tableElement);
        tableElement = tableElement.nextSibling().toElement();
    }

    printf("Large spreadsheet: iterating time is %d ms\n", timer.elapsed());
}

void TestXmlReader::testExternalOpenDocumentSpreadsheet(const QString& filename)
{
    QProcess unzip;
    QStringList arguments;
    arguments << "-o" << filename << "content.xml";

    printf("Unzipping content.xml from %s...\n", qPrintable(filename));

    unzip.start("unzip", arguments);
    if (!unzip.waitForStarted()) {
        printf("Error: can't invoke unzip. Check your PATH and installation!\n\n");
        return;
    }

    if (!unzip.waitForFinished()) {
        printf("Error: unzip failed, can't continue!\n\n");
        return;
    }

    printf("Procesing content.xml....\n");

    QString errorMsg;
    int errorLine = 0;
    int errorColumn = 0;

    QFile xmlfile("content.xml");
    if (!xmlfile.open(QFile::ReadOnly)) {
        printf("Can not open file '%s'\n", qPrintable(filename));
        return;
    }

    printf("Test external file: %s   %lld KB\n", qPrintable(filename), xmlfile.size() / 1024);

    QTime timer;
    timer.start();

    KoXmlDocument doc;

    QCOMPARE(KoXml::setDocument(doc, &xmlfile, true, &errorMsg, &errorLine, &errorColumn), true);
    QCOMPARE(errorMsg.isEmpty(), true);
    QCOMPARE(errorLine, 0);
    QCOMPARE(errorColumn, 0);

    printf("External spreadsheet: parsing time is %d ms\n", timer.elapsed());

    // namespaces that will be used
    QString officeNS = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
    QString tableNS = "urn:oasis:names:tc:opendocument:xmlns:table:1.0";
    QString textNS = "urn:oasis:names:tc:opendocument:xmlns:text:1.0";

    // <office:document-content>
    KoXmlElement contentElement;
    contentElement = doc.documentElement();
    QCOMPARE(contentElement.isNull(), false);
    QCOMPARE(contentElement.isElement(), true);
    QCOMPARE(contentElement.localName(), QString("document-content"));

    long totalCellCount = 0;

    KoXmlElement bodyElement;
    forEachElement(bodyElement, contentElement) {
        // <office:body>
        if (bodyElement.localName() != QString("body"))
            continue;

        // now we iterate inside the body
        timer.start();

        // <office:spreadsheet>
        KoXmlElement spreadsheetElement;
        spreadsheetElement = bodyElement.firstChild().toElement();
        QCOMPARE(spreadsheetElement.isNull(), false);
        QCOMPARE(spreadsheetElement.isElement(), true);
        QCOMPARE(spreadsheetElement.localName(), QString("spreadsheet"));

        // now we visit every sheet
        long tableCount = -1;
        KoXmlElement tableElement;
        tableElement = spreadsheetElement.firstChild().toElement();
        for (;;) {
            if (tableElement.isNull())
                break;

            if (tableElement.localName() != QString("table")) {
                tableElement = tableElement.nextSibling().toElement();
                continue;
            }

            QString tableName = tableElement.attributeNS(tableNS, "name", "");
            tableCount++;

            printf(" sheet #%ld (%s): ", tableCount + 1, qPrintable(tableName));

            // use to preload everything in this sheet, will slow it down!
            // KoXml::load( tableElement, 50 );

            long rowCount = -1;
            long cellCount = -1;

            KoXmlElement rowElement;
            rowElement = tableElement.firstChild().toElement();
            for (;;) {
                if (rowElement.isNull())
                    break;

                if (rowElement.localName() != QString("table-row")) {
                    rowElement = rowElement.nextSibling().toElement();
                    continue;
                }

                rowCount++;
                KoXml::load(rowElement, 4);

                QCOMPARE(rowElement.isElement(), true);
                QCOMPARE(rowElement.localName(), QString("table-row"));
                QCOMPARE(rowElement.parentNode().isNull(), false);
                QCOMPARE(rowElement.parentNode() == tableElement, true);

                KoXmlElement cellElement;
                cellElement = rowElement.firstChild().toElement();
                for (; ;) {
                    if (cellElement.isNull())
                        break;

                    if (cellElement.localName() != QString("table-cell")) {
                        cellElement = cellElement.nextSibling().toElement();
                        continue;
                    }

                    cellCount++;

                    QCOMPARE(cellElement.isNull(), false);
                    QCOMPARE(cellElement.isElement(), true);
                    QCOMPARE(cellElement.localName(), QString("table-cell"));
                    QString text1 = cellElement.text();
                    QString text2 = cellElement.text();
                    QCOMPARE(text1, text2);
                    QString type1 = cellElement.attributeNS(officeNS, "value-type", QString());
                    QString type2 = cellElement.attributeNS(officeNS, "value-type", QString());
                    QCOMPARE(type1, type2);
                    QString style1 = cellElement.attributeNS(tableNS, "style-name", QString());
                    QString style2 = cellElement.attributeNS(tableNS, "style-name", QString());
                    QCOMPARE(style1, style2);

                    QCOMPARE(cellElement.parentNode().isNull(), false);
                    QCOMPARE(cellElement.parentNode() == rowElement, true);

                    cellElement = cellElement.nextSibling().toElement();
                }


                // better not to unload, freeing memory takes time
                KoXml::unload(rowElement);

                rowElement = rowElement.nextSibling().toElement();
            }

            printf(" %ld rows, %ld cells\n", rowCount + 1, cellCount + 1);
            totalCellCount += (cellCount + 1);

            // IMPORTANT: helps minimizing memory usage !!
            // we do not need that element anymore, so just throw it away
            KoXml::unload(tableElement);

            tableElement = tableElement.nextSibling().toElement();
        }

        KoXml::unload(spreadsheetElement);
    }

    printf("Total number of cells: %ld\n", totalCellCount);

    int elapsed = timer.elapsed();
    printf("External spreadsheet: iterating time is %d ms\n", elapsed);
    if (elapsed > 0)
        printf("  approx. %ld cells/second\n", totalCellCount*1000 / elapsed);

    // uncomment to check the XML
    xmlfile.remove();
}

QTEST_MAIN(TestXmlReader)
#include <TestXmlReader.moc>

