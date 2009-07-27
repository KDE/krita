#include "TestImageCollection.h"

#include <KoImageData.h>
#include <KoImageCollection.h>
#include <KoStore.h>

#include <QImage>
#include <kurl.h>
#include <kdebug.h>

#include <qtest_kde.h>

void TestImageCollection::testGetImageImage()
{
    KoImageCollection collection;
    QImage image(KDESRCDIR "/logo-koffice.png");

    KoImageData id1 = collection.getImage(image);
    KoImageData id2 = collection.getImage(image);
    QCOMPARE(id1, id2);
    KoImageData id3 = collection.getImage(image);
    QCOMPARE(id1.key(), id3.key());
    QCOMPARE(id1, id3);
    QImage image2(KDESRCDIR "/logo-kpresenter.png");
    {
        KoImageData id4 = collection.getImage(image2);
        QVERIFY(id1.key() != id4.key());
        QCOMPARE(collection.count(), 2);
qDebug() << "-====";
    } // id4 goes out of scope
    QCOMPARE(collection.count(), 1);
    id1 = KoImageData();
    QCOMPARE(collection.size(), 1);
    id2 = KoImageData();
    QCOMPARE(collection.size(), 1);
    id3 = KoImageData();
    QCOMPARE(collection.size(), 0);
}

void TestImageCollection::testGetImageKUrl()
{
    KoImageCollection collection;
    KUrl url(KDESRCDIR "/logo-koffice.png");
    KoImageData id1 = collection.getImage(url);
    KoImageData id2 = collection.getImage(url);
    QCOMPARE(id1, id2);
    KoImageData id3 = collection.getImage(url);
    QCOMPARE(id1.key(), id3.key());
    QCOMPARE(id1, id3);
    KUrl url2(KDESRCDIR "/logo-kpresenter.png");
    {
        KoImageData id4 = collection.getImage(url2);
        QVERIFY(id1.key() != id4.key());
        QCOMPARE(collection.size(), 2);
    } // id4 goes out of scope
    QCOMPARE(collection.size(), 1);
}

void TestImageCollection::testGetImageStore()
{
    KoImageCollection collection;
    KoStore *store = KoStore::createStore(KDESRCDIR "/store.zip", KoStore::Read);
    QString image("logo-koffice.png");
    KoImageData id1 = collection.getImage(image, store);
    KoImageData id2 = collection.getImage(image, store);
    QCOMPARE(id1, id2);
    KoImageData id3 = collection.getImage(image);
    QCOMPARE(id1.key(), id3.key());
    QCOMPARE(id1, id3);
    QString image2("logo-kpresenter.png");
    {
        KoImageData id4 = collection.getImage(image2);
        QVERIFY(id1.key() != id4.key());
        QCOMPARE(collection.count(), 2);
    } // id4 goes out of scope
    QCOMPARE(collection.count(), 1);
    delete store;
}

void TestImageCollection::testGetImageMixed()
{
    KoImageCollection collection;
    KoStore *store = KoStore::createStore(KDESRCDIR "/store.zip", KoStore::Read);
    QString href("logo-koffice.png");
    KoImageData id1 = collection.getImage(href, store);
    KoImageData id2 = collection.getImage(href, store);
    QCOMPARE(id1, id2);

    KUrl url(KDESRCDIR "/logo-koffice.png");
    KoImageData id3 = collection.getImage(url);
    QCOMPARE(id1, id3);

    KUrl image2(KDESRCDIR "/logo-kpresenter.png");
    KoImageData id4 = collection.getImage(image2);
    QVERIFY(id3.key() != id4.key());

    QString href2("logo-kpresenter.png");
    {
        KoImageData id5 = collection.getImage(href2, store);
        QCOMPARE(id5, id4);
        QCOMPARE(collection.size(), 2);
    }
    QCOMPARE(collection.size(), 2);
    delete store;
}

void TestImageCollection::testImageDataAsSharedData()
{
    KoImageData data;
    QCOMPARE(data.isValid(), false);

    QImage image(100, 101, QImage::Format_RGB32);
    data.setImage(image);
    QCOMPARE(data.isValid(), true);
    QCOMPARE(data.image(), image);
}

QTEST_KDEMAIN(TestImageCollection, GUI)
#include "TestImageCollection.moc"
