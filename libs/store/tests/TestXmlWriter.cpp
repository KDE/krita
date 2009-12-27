/* This file is part of the KDE project
 * Copyright (C) 2004 David Faure <faure@kde.org>
 * Copyright 2008 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include <KoXmlWriter.h>

#include <Q3CString>
#include <qtest_kde.h>

class TestXmlWriter : public QObject
{
    Q_OBJECT
private slots:
    void testDocytype();
    void testEmtpyElement();
    void testAttributes();
    void testIndent();
    void testTextNode();
    void testTextSpan();
    void testTextSpanWithTabCache();
    void testProcessingInstruction();
    void testAddManifestEntry();
    void testEscapingLongString();
    void testEscalingLongString2();
    void testConfig();

    void speedTest();

private:
    void setup(const char *publicId = 0, const char *systemId = 0);
    QString content();

    KoXmlWriter *writer;
    QBuffer *buffer;
};

void TestXmlWriter::setup(const char *publicId, const char *systemId)
{
    buffer = new QBuffer();
    buffer->open( QIODevice::WriteOnly );

    writer = new KoXmlWriter( buffer );
    writer->startDocument( "dummy", publicId, systemId );
    writer->startElement( "dummy" );
}

QString TestXmlWriter::content()
{
    writer->endElement();
    writer->endDocument();
    buffer->putChar( '\0' ); /*null-terminate*/
    buffer->close();
    QString stringContent = QString::fromUtf8(buffer->data());
    int index = stringContent.indexOf("<dummy");
    Q_ASSERT(index);
    index = stringContent.indexOf('>', index);
    stringContent = stringContent.mid(index+1, stringContent.length() - index - 11).trimmed();
    return stringContent;
}

void TestXmlWriter::testDocytype()
{
    setup("foo", "bar");
    QCOMPARE(content(), QString());
    QString stringContent = QString::fromUtf8(buffer->data());
    QCOMPARE(stringContent, QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<!DOCTYPE dummy PUBLIC \"foo\" \"bar\">\n<dummy/>\n"));
}

void TestXmlWriter::testAttributes()
{
    setup();

    writer->startElement("test");
    writer->addAttribute("a", "val");
    writer->addAttribute("b", "<\">");
    writer->addAttribute("c", -42);
    writer->addAttribute("d", 1234.56789012345);
    writer->addAttributePt("e", 1234.56789012345);
    writer->endElement();
    QCOMPARE(content(), QString("<test a=\"val\" b=\"&lt;&quot;&gt;\" c=\"-42\" d=\"1234.56789012345\" e=\"1234.56789012345pt\"/>"));
}

void TestXmlWriter::testEmtpyElement()
{
    setup();
    writer->startElement("m");
    writer->endElement();
    QCOMPARE(content(), QString("<m/>"));
}

void TestXmlWriter::testIndent()
{
    setup();
    writer->startElement("a");
    writer->startElement("b");
    writer->startElement("c");
    writer->endElement();
    writer->endElement();
    writer->endElement();
    QCOMPARE(content(), QString("<a>\n  <b>\n   <c/>\n  </b>\n </a>"));
}

void TestXmlWriter::testTextNode()
{
    setup();
    writer->startElement("a");
    writer->startElement("b", false /*no indent*/);
    writer->startElement("c");
    writer->endElement();
    writer->addTextNode("te");
    writer->addTextNode("xt");
    writer->endElement();
    writer->endElement();
    QCOMPARE(content(), QString("<a>\n  <b><c/>text</b>\n </a>"));
}

void TestXmlWriter::testTextSpan()
{
    setup();
    writer->startElement("p", false /*no indent*/);
    writer->addTextSpan(QString::fromLatin1("   \t\n foo  "));
    writer->endElement();
    QCOMPARE(content(), QString("<p><text:s text:c=\"3\"/><text:tab/><text:line-break/> foo<text:s text:c=\"2\"/></p>"));
}

void TestXmlWriter::testTextSpanWithTabCache()
{
    setup();
    writer->startElement("p", false /*no indent*/);
    QMap<int, int> tabCache;
    tabCache.insert(3, 0);
    writer->addTextSpan(QString::fromUtf8("   \t\n foö  "), tabCache);
    writer->endElement();
    QCOMPARE(content(), QString::fromUtf8("<p><text:s text:c=\"3\"/><text:tab text:tab-ref=\"1\"/>"
            "<text:line-break/> foö<text:s text:c=\"2\"/></p>"));
}

void TestXmlWriter::testProcessingInstruction()
{
    setup();
    writer->startElement("p", false /*no indent*/);
    writer->addProcessingInstruction("opendocument foobar");
    writer->addTextSpan(QString::fromLatin1("foo"));
    writer->endElement();
    QCOMPARE(content(), QString("<p><?opendocument foobar?>foo</p>"));
}

void TestXmlWriter::testAddManifestEntry()
{
    setup();
    writer->addManifestEntry(QString::fromLatin1("foo/bar/blah"), QString::fromLatin1("mime/type"));
    QCOMPARE(content(), QString("<manifest:file-entry manifest:media-type=\"mime/type\" "
                "manifest:full-path=\"foo/bar/blah\"/>"));
}


void TestXmlWriter::testEscapingLongString()
{
    int sz = 15000;  // must be more than KoXmlWriter::s_escapeBufferLen
    Q3CString x(sz);
    x.fill('x', sz);
    x += '&';
    setup();

    writer->startElement("test");
    writer->addAttribute("a", x);
    writer->endElement();

    Q3CString expected = "<test a=\"";
    expected += x + "amp;\"/>";
    QCOMPARE(content(), QString(expected));
}

void TestXmlWriter::testEscalingLongString2()
{
    QString longPath;
    for (uint i = 0 ; i < 1000 ; ++i)
        longPath += QString::fromLatin1("M10 10L20 20 ");
    setup();
    writer->startElement("test");
    writer->addAttribute("a", longPath);
    writer->endElement();
    QString expected = "<test a=\"";
    expected += longPath.toUtf8() + "\"/>";
    QCOMPARE(content(), expected);

}

void TestXmlWriter::testConfig()
{
    setup();
    const bool val = true;
    const int num = 1;
    const qreal numdouble = 5.0;
    writer->addConfigItem(QString::fromLatin1("TestConfigBool"), val);
    writer->addConfigItem(QString::fromLatin1("TestConfigInt"), num);
    writer->addConfigItem(QString::fromLatin1("TestConfigDouble"), numdouble);
    QCOMPARE(content(), QString("<config:config-item config:name=\"TestConfigBool\""
            " config:type=\"boolean\">true</config:config-item>\n"
            " <config:config-item config:name=\"TestConfigInt\" config:type=\"int\">1</config:config-item>\n"
            " <config:config-item config:name=\"TestConfigDouble\" config:type=\"double\">5</config:config-item>"));
}

static const int NumParagraphs = 30000;

void TestXmlWriter::speedTest()
{
    QTime time;
    time.start();
    QString paragText = QString::fromUtf8("This is the text of the paragraph. I'm including a euro sign to test encoding issues: €");
    Q3CString styleName = "Heading 1";

    QFile out(QString::fromLatin1("out5.xml"));
    if (out.open(QIODevice::WriteOnly)) {
        KoXmlWriter writer(&out);
        writer.startDocument("rootelem");
        writer.startElement("rootelem");
        for (int i = 0 ; i < NumParagraphs ; ++i) {
            writer.startElement("paragraph");
            writer.addAttribute("text:style-name", styleName);
            writer.addTextNode(paragText);
            writer.endElement();
        }
        writer.endElement();
        writer.endDocument();
    }
    out.close();
    qDebug("writing %i XML elements using KoXmlWriter: %i ms", NumParagraphs, time.elapsed());
    // TODO we might want to convert this into a QBenchmark test
}

QTEST_MAIN(TestXmlWriter)
#include <TestXmlWriter.moc>

