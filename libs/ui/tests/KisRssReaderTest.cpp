/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisRssReaderTest.h"
#include <KisRssReader.h>
#include <QtTest>
#include <testutil.h>
#include <QFile>

KisRssReaderTest::KisRssReaderTest(QObject *parent) : QObject(parent)
{

}

void KisRssReaderTest::testParseData()
{
    KisRssReader reader;
    QFile rssFile(TestUtil::fetchDataFileLazy("rss_feeds/feed.xml"));

    RssItemList itemList = reader.parse(rssFile);

    QCOMPARE(itemList.count(), 10);

}

QTEST_MAIN(KisRssReaderTest);
