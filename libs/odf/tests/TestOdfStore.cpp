#include <qtest_kde.h>

//#include <KDebug>

#include <KoXmlReader.h>
#include <KoOdfReadStore.h>

class TestOdfStore : public QObject
{
    Q_OBJECT
public:
    TestOdfStore() {}

private slots:
    void testMimeForPath();

};

void TestOdfStore::testMimeForPath()
{
    const QString xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<manifest:manifest xmlns:manifest=\"urn:oasis:names:tc:opendocument:xmlns:manifest:1.0\">\n"
        "<manifest:file-entry manifest:media-type=\"application/vnd.oasis.opendocument.text\" manifest:full-path=\"/\"/>\n"
        "<manifest:file-entry manifest:media-type=\"text/xml\" manifest:full-path=\"content.xml\"/>\n"
        "<manifest:file-entry manifest:media-type=\"application/vnd.oasis.opendocument.text\" manifest:full-path=\"Object 1\"/>\n"
        "</manifest:manifest>";

    KoXmlDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;
    bool ok = doc.setContent( xml, true /* namespace processing */, &errorMsg, &errorLine, &errorColumn );
    QVERIFY(ok);

    QString mime = KoOdfReadStore::mimeForPath(doc, "Object 1");
    QCOMPARE(mime, QString::fromLatin1("application/vnd.oasis.opendocument.text"));
}

QTEST_KDEMAIN(TestOdfStore, GUI)

#include <TestOdfStore.moc>
