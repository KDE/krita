/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisMultiFeedRssModelTest.h"

#include <simpletest.h>
#include <QSignalSpy>

#include <MockNetworkAccessManager.h>
#include <KisMultiFeedRSSModel.h>
#include <testutil.h>


KisMultiFeedRssModelTest::KisMultiFeedRssModelTest(QObject *parent) : QObject(parent)
{

}

void KisMultiFeedRssModelTest::testAddFeed()
{
    QFETCH(QList<FakeReplyData>, replyDataList);
    QFETCH(int, resultArticleCount);


    MockNetworkAccessManager* nam(new MockNetworkAccessManager());

    QStringList feedList;
    for(FakeReplyData& replyData: replyDataList) {
        feedList << replyData.url.toString();
        nam->setReplyData(replyData);
    }

    MultiFeedRssModel* rssModel(new MultiFeedRssModel(nam));
    QSignalSpy spy(rssModel, SIGNAL(feedDataChanged()));
    QVERIFY(spy.isValid());

    for(QString& feedUrl: feedList) {
        rssModel->addFeed(feedUrl);
    }

    // wait for signal
    QVERIFY(spy.wait());

    QCOMPARE(rssModel->articleCount(), resultArticleCount);
}

void KisMultiFeedRssModelTest::testAddFeed_data()
{
    QTest::addColumn<QList<FakeReplyData>>("replyDataList");
    QTest::addColumn<int>("resultArticleCount");

    // setup feed1
    QFile rssFile1(TestUtil::fetchDataFileLazy("rss_feeds/feed.xml"));
    bool fileOpened = rssFile1.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(fileOpened);

    QString urlFeed1("https://krita.org/en/feed/");

    // create reply data
    FakeReplyData replyFeed1;
    replyFeed1.url = QUrl(urlFeed1);
    replyFeed1.statusCode = 200;
    replyFeed1.requestMethod = QNetworkAccessManager::GetOperation;
    replyFeed1.contentType = "application/atom+xml";
    replyFeed1.responseBody = rssFile1.readAll();


    // setup feed2
    QFile rssFile2(TestUtil::fetchDataFileLazy("rss_feeds/feed.xml"));
    fileOpened = rssFile2.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(fileOpened);

    QString urlFeed2("https://krita.org/en/another_feed/");

    FakeReplyData replyFeed2;
    replyFeed2.url = QUrl(urlFeed2);
    replyFeed2.statusCode = 200;
    replyFeed2.requestMethod = QNetworkAccessManager::GetOperation;
    replyFeed2.contentType = "application/atom+xml";
    replyFeed2.responseBody = rssFile2.readAll();


    // test with 1 feed
    QList<FakeReplyData> listTest1 = { replyFeed1 };
    QTest::addRow("1 feed")
            << listTest1
            << 10;

    // test with 2 feeds
    QList<FakeReplyData> listTest2 = { replyFeed1, replyFeed2 };
    QTest::addRow("2 feeds")
            << listTest2
            << 20;
}

void KisMultiFeedRssModelTest::testRemoveFeed()
{
    QFETCH(QList<FakeReplyData>, replyDataList);
    QFETCH(QString, removeFeedUrl);
    QFETCH(int, resultArticleCount);


    MockNetworkAccessManager* nam(new MockNetworkAccessManager());

    QStringList feedList;
    for(FakeReplyData& replyData: replyDataList) {
        feedList << replyData.url.toString();
        nam->setReplyData(replyData);
    }

    MultiFeedRssModel* rssModel(new MultiFeedRssModel(nam));
    QSignalSpy spyAdd(rssModel, SIGNAL(feedDataChanged()));
    QVERIFY(spyAdd.isValid());

    for(QString& feedUrl: feedList) {
        rssModel->addFeed(feedUrl);
    }

    // wait for signal
    QVERIFY(spyAdd.wait());

    // the remove test itself
    rssModel->removeFeed(removeFeedUrl);

    QCOMPARE(rssModel->articleCount(), resultArticleCount);
}

void KisMultiFeedRssModelTest::testRemoveFeed_data()
{
    QTest::addColumn<QList<FakeReplyData>>("replyDataList");
    QTest::addColumn<QString>("removeFeedUrl");
    QTest::addColumn<int>("resultArticleCount");

    // setup feed1
    QFile rssFile1(TestUtil::fetchDataFileLazy("rss_feeds/feed.xml"));
    bool fileOpened = rssFile1.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(fileOpened);

    QString urlFeed1("https://krita.org/en/feed/");

    // create reply data
    FakeReplyData replyFeed1;
    replyFeed1.url = QUrl(urlFeed1);
    replyFeed1.statusCode = 200;
    replyFeed1.requestMethod = QNetworkAccessManager::GetOperation;
    replyFeed1.contentType = "application/atom+xml";
    replyFeed1.responseBody = rssFile1.readAll();

    // setup feed2
    QFile rssFile2(TestUtil::fetchDataFileLazy("rss_feeds/feed.xml"));
    fileOpened = rssFile2.open(QIODevice::ReadOnly | QIODevice::Text);
    QVERIFY(fileOpened);

    QString urlFeed2("https://krita.org/en/another_feed/");

    FakeReplyData replyFeed2;
    replyFeed2.url = QUrl(urlFeed2);
    replyFeed2.statusCode = 200;
    replyFeed2.requestMethod = QNetworkAccessManager::GetOperation;
    replyFeed2.contentType = "application/atom+xml";
    replyFeed2.responseBody = rssFile2.readAll();


    // test with 1 feed
    QList<FakeReplyData> listTest1 = { replyFeed1 };
    QTest::addRow("1 feed")
            << listTest1
            << urlFeed1
            << 0;

    // test with 2 feeds
    QList<FakeReplyData> listTest2 = { replyFeed1, replyFeed2 };
    QTest::addRow("2 feeds")
            << listTest2
            << urlFeed1
            << 10;
}

SIMPLE_TEST_MAIN(KisMultiFeedRssModelTest);

