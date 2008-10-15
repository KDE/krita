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
    QImage image( KDESRCDIR "/logo-koffice.png" );
    KoImageData * id1 = collection.getImage(image);
    KoImageData * id2 = collection.getImage(image);
    QCOMPARE(*id1, *id2);
    QVERIFY(id1 != id2);
    KoImageData * id3 = collection.getImage(image);
    QCOMPARE(id1->key(), id3->key());
    QCOMPARE(*id1, *id3);
    QImage image2( KDESRCDIR "/logo-kpresenter.png" );
    KoImageData * id4 = collection.getImage(image2);
    QVERIFY(id1->key() != id4->key());
    QCOMPARE(2, collection.size());
    delete id1;
    delete id2;
    QCOMPARE(2, collection.size());
    delete id3;
    QCOMPARE(1, collection.size());
    delete id4;
    QCOMPARE(0, collection.size());
}

void TestImageCollection::testGetImageKUrl()
{
    KoImageCollection collection;
    KUrl image( KDESRCDIR "/logo-koffice.png" );
    KoImageData * id1 = collection.getImage(image);
    KoImageData * id2 = collection.getImage(image);
    QCOMPARE(*id1, *id2);
    QVERIFY(id1 != id2);
    KoImageData * id3 = collection.getImage(image);
    QCOMPARE(id1->key(), id3->key());
    QCOMPARE(*id1, *id3);
    KUrl image2( KDESRCDIR "/logo-kpresenter.png" );
    KoImageData * id4 = collection.getImage(image2);
    QVERIFY(id1->key() != id4->key());
    QCOMPARE(2, collection.size());
    delete id1;
    delete id2;
    QCOMPARE(2, collection.size());
    delete id3;
    QCOMPARE(1, collection.size());
    delete id4;
    QCOMPARE(0, collection.size());
}

void TestImageCollection::testGetImageStore()
{
    KoImageCollection collection;
    KoStore * store = KoStore::createStore( KDESRCDIR "/store.zip", KoStore::Read );
    QString image( "logo-koffice.png" );
    KoImageData * id1 = collection.getImage(image, store);
    KoImageData * id2 = collection.getImage(image, store);
    QCOMPARE(*id1, *id2);
    QVERIFY(id1 != id2);
    KoImageData * id3 = collection.getImage(image, store);
    QCOMPARE(id1->key(), id3->key());
    QCOMPARE(*id1, *id3);
    QString image2( "logo-kpresenter.png" );
    KoImageData * id4 = collection.getImage(image2, store);
    QVERIFY(id1->key() != id4->key());
    QCOMPARE(2, collection.size());
    delete id1;
    delete id2;
    QCOMPARE(2, collection.size());
    delete id3;
    QCOMPARE(1, collection.size());
    delete id4;
    QCOMPARE(0, collection.size());
    delete store;
}

void TestImageCollection::testGetImageMixed()
{
    KoImageCollection collection;
    KoStore * store = KoStore::createStore( KDESRCDIR "/store.zip", KoStore::Read );
    QString href( "logo-koffice.png" );
    KoImageData * id1 = collection.getImage(href, store);
    KoImageData * id2 = collection.getImage(href, store);
    QCOMPARE(*id1, *id2);
    QVERIFY(id1 != id2);

    KUrl image( KDESRCDIR "/logo-koffice.png" );
    KoImageData * id3 = collection.getImage(image);
    QCOMPARE(*id1, *id3);
    QVERIFY(id1 != id3);

    KUrl image2( KDESRCDIR "/logo-kpresenter.png" );
    KoImageData * id4 = collection.getImage(image2);
    QVERIFY(id3->key() != id4->key());

    QString href2( "logo-kpresenter.png" );
    KoImageData * id5 = collection.getImage(href2, store);
    QCOMPARE(*id5, *id4);

    QCOMPARE(2, collection.size());
    delete id1;
    delete id2;
    QCOMPARE(2, collection.size());
    delete id3;
    QCOMPARE(1, collection.size());
    delete id4;
    QCOMPARE(1, collection.size());
    delete id5;
    QCOMPARE(0, collection.size());
    delete store;
}

QTEST_KDEMAIN(TestImageCollection, GUI)
#include "TestImageCollection.moc"
