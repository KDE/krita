/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2004 David Faure <faure@kde.org>
 * SPDX-FileCopyrightText: 2008 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include <KoXmlWriter.h>

#include <QString>
#include <QBuffer>
#include <QTest>
#include <QElapsedTimer>

class TestXmlWriter : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDoctype();
    void testEmptyElement();
    void testAttributes();
    void testIndent();
    void testTextNode();
    void testAddManifestEntry();
    void testEscapingLongString();
    void testEscalingLongString2();

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

void TestXmlWriter::testDoctype()
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
    writer->addAttribute("f", false);
    writer->addAttribute("g", true);
    writer->endElement();
    QCOMPARE(content(), QString("<test a=\"val\" b=\"&lt;&quot;&gt;\" c=\"-42\" d=\"1234.56789012345\" f=\"false\" g=\"true\"/>"));
}

void TestXmlWriter::testEmptyElement()
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
    QCOMPARE(content(), QString("<a><b><c/>text</b>\n </a>"));
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
    QString x(sz, 'x');
    x += '&';
    setup();

    writer->startElement("test");
    writer->addAttribute("a", x);
    writer->endElement();

    QString expected = "<test a=\"";
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

static const int NumParagraphs = 30000;

void TestXmlWriter::speedTest()
{
    QElapsedTimer time;
    time.start();
    QString paragText = QString::fromUtf8("This is the text of the paragraph. I'm including a euro sign to test encoding issues: €");
    QString styleName = "Heading 1";

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
    out.remove();
    qDebug("writing %i XML elements using KoXmlWriter: %i ms", NumParagraphs, (int)time.elapsed());
    // TODO we might want to convert this into a QBenchmark test
}

QTEST_GUILESS_MAIN(TestXmlWriter)
#include <TestXmlWriter.moc>

