/*
 *  This file is part of Calligra tests
 *
 *  SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "TestImageCollection.h"

#include <KoImageData.h>
#include <KoImageCollection.h>
#include <KoStore.h>

#include <QImage>
#include <QPixmap>
#include <QBuffer>
#include <QUrl>
#include <FlakeDebug.h>

#include <QTest>

void TestImageCollection::testGetImageImage()
{
    KoImageCollection collection;
    QImage image(FILES_DATA_DIR "logo-calligra.png");

    KoImageData *id1 = collection.createImageData(image);
    QCOMPARE(id1->hasCachedImage(), true);
    QCOMPARE(id1->suffix(), QString("png"));
    KoImageData *id2 = collection.createImageData(image);
    QCOMPARE(id2->hasCachedImage(), true);
    QCOMPARE(id1->priv(), id2->priv());
    KoImageData *id3 = collection.createImageData(image);
    QCOMPARE(id3->hasCachedImage(), true);
    QCOMPARE(id1->key(), id3->key());
    QCOMPARE(id1->priv(), id3->priv());
    QImage image2(FILES_DATA_DIR "logo-kpresenter.png");
    KoImageData *id4 = collection.createImageData(image2);
    QCOMPARE(id4->hasCachedImage(), true);
    QVERIFY(id1->key() != id4->key());
    QCOMPARE(collection.count(), 2);
    delete id4;
    QCOMPARE(id1->hasCachedImage(), true);
    QCOMPARE(id2->hasCachedImage(), true);
    QCOMPARE(id3->hasCachedImage(), true);
    QCOMPARE(collection.count(), 1);
    delete id1;
    QCOMPARE(id2->hasCachedImage(), true);
    QCOMPARE(id3->hasCachedImage(), true);
    QCOMPARE(collection.size(), 1);
    delete id2;
    QCOMPARE(id3->hasCachedImage(), true);
    QCOMPARE(collection.size(), 1);
    delete id3;
    QCOMPARE(collection.size(), 0);

    // add an image bigger than the allowed size to be held in memory
    QImage hugeImage(500, 500, QImage::Format_RGB32);
    KoImageData *id5 = collection.createImageData(hugeImage);
    delete id5;
}

void TestImageCollection::testGetImageStore()
{
    KoImageCollection collection;
    KoStore *store = KoStore::createStore(FILES_DATA_DIR "store.zip", KoStore::Read);
    QString image("logo-calligra.jpg");
    KoImageData *id1 = collection.createImageData(image, store);
    QCOMPARE(id1->suffix(), QString("jpg"));
    QCOMPARE(id1->hasCachedImage(), false);
    KoImageData *id2 = collection.createImageData(image, store);
    QCOMPARE(id2->hasCachedImage(), false);
    QCOMPARE(id1->priv(), id2->priv());
    QCOMPARE(id1->key(), id2->key());
    QCOMPARE(collection.count(), 1);

    delete store;
}

void TestImageCollection::testInvalidImageData()
{
    KoImageCollection collection;
    QByteArray invalidImageData(100, '^');
    KoImageData *data = collection.createImageData(invalidImageData);
    QVERIFY(data);
    QVERIFY(!data->isValid());
    QVERIFY(data->errorCode() == KoImageData::OpenFailed);
    QCOMPARE(collection.count(), 1);
    QBuffer storedData;
    QVERIFY(!data->saveData(storedData)); // should fail if QIODevice is closed
    storedData.open(QIODevice::WriteOnly);
    QVERIFY(data->saveData(storedData));
    QCOMPARE(invalidImageData, storedData.buffer());
    delete data;
}

void TestImageCollection::testImageDataAsSharedData()
{
    KoImageData data;
    QCOMPARE(data.isValid(), false);

    QImage image(100, 101, QImage::Format_RGB32);
    data.setImage(image);
    QCOMPARE(data.isValid(), true);
    QCOMPARE(data.hasCachedImage(), true);
    QCOMPARE(data.image(), image);

    KoImageData data2(data);
    QCOMPARE(data, data2);
    QCOMPARE(data.isValid(), true);
    QCOMPARE(data.image(), image);
    QCOMPARE(data2.isValid(), true);
    QCOMPARE(data2.image(), image);
    {
        KoImageData data3;
        data3 = data;
        QCOMPARE(data3.isValid(), true);
        QCOMPARE(data3.image(), image);
    }
    QCOMPARE(data, data2);
    QCOMPARE(data.isValid(), true);
    QCOMPARE(data.image(), image);
    QCOMPARE(data2.isValid(), true);
    QCOMPARE(data2.image(), image);

    KoImageData empty;
    KoImageData second(empty);
}

void TestImageCollection::testPreload1()
{
    KoImageData data;
    QImage image(100, 102, QImage::Format_RGB32);
    data.setImage(image);
    QCOMPARE(data.hasCachedImage(), true);

    QCOMPARE(data.hasCachedPixmap(), false);
    QPixmap pixmap = data.pixmap(QSize(40, 41));
    QCOMPARE(pixmap.width(), 40);
    QCOMPARE(pixmap.height(), 41);
    QCOMPARE(data.hasCachedPixmap(), true);
    QPixmap pixmap2 = data.pixmap(QSize(40, 41));
    QCOMPARE(pixmap.cacheKey(), pixmap2.cacheKey());

    QPixmap pixmap3 = data.pixmap();
    QCOMPARE(pixmap.cacheKey(), pixmap3.cacheKey());
    QCOMPARE(data.hasCachedImage(), true);
}

void TestImageCollection::testPreload3()
{
    KoImageData data;
    KoStore *store = KoStore::createStore(FILES_DATA_DIR "store.zip", KoStore::Read);
    QString image("logo-calligra.png");
    data.setImage(image, store);

    QCOMPARE(data.hasCachedImage(), true); // the png is tiny.. Its kept in memory.
    QCOMPARE(data.hasCachedPixmap(), false);
    QPixmap pixmap = data.pixmap(QSize(40, 41));
    QCOMPARE(pixmap.width(), 40);
    QCOMPARE(pixmap.height(), 41);
    QCOMPARE(data.hasCachedPixmap(), true);
    QCOMPARE(data.hasCachedImage(), true);
    QPixmap pixmap2 = data.pixmap(QSize(40, 41));
    QCOMPARE(pixmap.cacheKey(), pixmap2.cacheKey());

    QPixmap pixmap3 = data.pixmap();
    QCOMPARE(pixmap.cacheKey(), pixmap3.cacheKey());

    // now get a different size;
    QPixmap pixmap4 = data.pixmap(QSize(10, 12));
    QCOMPARE(pixmap.width(), 40);
    QCOMPARE(pixmap.height(), 41);
    QCOMPARE(pixmap4.width(), 10);
    QCOMPARE(pixmap4.height(), 12);
    QVERIFY(pixmap.cacheKey() != pixmap4.cacheKey());

    QPixmap pixmap5 = data.pixmap();
    QCOMPARE(pixmap5.cacheKey(), pixmap4.cacheKey());
}

void TestImageCollection::testSameKey()
{
    KoStore *store = KoStore::createStore(FILES_DATA_DIR "store.zip", KoStore::Read);
    QString image("logo-calligra.png");
    KoImageData data;
    data.setImage(image, store);

    KoImageData data2;
    data2.setImage(image, store);

    QCOMPARE(data.key(), data2.key());

    QFile file(FILES_DATA_DIR "logo-calligra.png");
    file.open(QIODevice::ReadOnly);
    QByteArray imageData = file.readAll();
    KoImageData data3;
    data3.setImage(imageData);
    QCOMPARE(data.key(), data3.key());
    QCOMPARE(data2.key(), data3.key());

    QImage qImage1(FILES_DATA_DIR "logo-calligra.png");
    QImage qImage2(FILES_DATA_DIR "logo-calligra.png");
    KoImageData data4;
    data4.setImage(qImage1);
    KoImageData data5;
    data5.setImage(qImage2);
    QCOMPARE(data4.key(), data5.key());

    QImage qImage3(FILES_DATA_DIR "/logo-calligra-big.png");
    QImage qImage4(FILES_DATA_DIR "/logo-calligra-big.png");
    KoImageData data6;
    data6.setImage(qImage3);
    KoImageData data7;
    data7.setImage(qImage4);
    QCOMPARE(data6.key(), data7.key());

    // should reset the key so it's the same as data6
    data2.setImage(qImage3);
    QCOMPARE(data2.key(), data7.key());
}

void TestImageCollection::testIsValid()
{
    KoImageData data;
    QCOMPARE(data.isValid(), false);

    QImage image(100, 102, QImage::Format_RGB32);
    data.setImage(image);
    QCOMPARE(data.isValid(), true);

    QByteArray bytes("foo");
    data.setImage(bytes); // obviously not a correct image.
    QCOMPARE(data.isValid(), false);
}

QTEST_MAIN(TestImageCollection)
