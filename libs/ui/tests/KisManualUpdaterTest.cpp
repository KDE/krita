/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "KisManualUpdaterTest.h"

#include <QtTest>
#include <QString>
#include <QScopedPointer>
#include <KisManualUpdater.h>
#include <MockMultiFeedRssModel.h>

#include <sdk/tests/testui.h>

KisManualUpdaterTest::KisManualUpdaterTest(QObject *parent) : QObject(parent)
{

}

void KisManualUpdaterTest::testAvailableVersionIsHigher()
{
    QFETCH(QString, currentVersion);
    QFETCH(QString, availableVersion);
    QFETCH(bool, expectedReturnValue);

    QScopedPointer<KisManualUpdater> updater(new KisManualUpdater());
    bool actualReturnValue = updater->availableVersionIsHigher(currentVersion, availableVersion);
    QCOMPARE(actualReturnValue, expectedReturnValue);
}

void KisManualUpdaterTest::testAvailableVersionIsHigher_data()
{
    QTest::addColumn<QString>("currentVersion");
    QTest::addColumn<QString>("availableVersion");
    QTest::addColumn<bool>("expectedReturnValue");

    QTest::addRow("equal") << "4.2.5" << "4.2.5" << false;

    QTest::addRow("higher major") << "3.0.0" << "4.2.5" << true;
    QTest::addRow("higher minor") << "4.2.4" << "4.3.4" << true;
    QTest::addRow("higher rev") << "4.2.4" << "4.2.5" << true;

    QTest::addRow("lower major") << "4.2.5" << "3.2.5" << false;
    QTest::addRow("lower minor") << "4.2.5" << "4.1.5" << false;
    QTest::addRow("lower rev") << "4.2.5" << "4.2.4" << false;

    QTest::addRow("current is git, available lower stable") << "4.3.0-d8ea4b" << "4.2.5" << false;
    QTest::addRow("current is beta, available lower stable") << "4.2.6-beta1" << "4.2.5" << false;

    QTest::addRow("current is git, available higher stable") << "4.3.0-d8ea4b" << "4.4.2" << true;
    QTest::addRow("current is beta, available lower stable") << "4.2.6-beta1" << "4.3.1" << true;

    QTest::addRow("current is git, available equal version release") << "4.3.0-d8ea4b" << "4.3.0" << true;
    QTest::addRow("current is beta, available equal version release") << "4.2.6-beta1" << "4.2.6" << true;

    QTest::addRow("available is git, current equal version release") << "4.3.0" << "4.3.0-d8ea4b" << false;
    QTest::addRow("available is beta, current equal version release") << "4.2.6" << "4.2.6-beta1" << false;

// this is not solved in the function, as it is unlikely to have such versions in the RSS feed
//    QTest::addRow("current is git, available equal version beta") << "4.3.0-d8ea4b" << "4.3.0-beta1" << true;
    //    QTest::addRow("available is beta, current equal version git") << "4.2.6-beta1" << "4.2.6-d8ea4b" << false;

    QTest::addRow("current is empty") << QString() << "4.2.5" << false;
    QTest::addRow("available is empty") << "4.2.6" << QString() << false;
}

void KisManualUpdaterTest::testCheckForUpdate()
{
    QFETCH(RssItemList, feed);
    QFETCH(QString, currentVersion);
    QFETCH(QString, availableVersion);
    QFETCH(QString, downloadLink);
    QFETCH(UpdaterStatus::StatusID, resultStatus);

    MockMultiFeedRssModel* mockRssModel(new MockMultiFeedRssModel());
    mockRssModel->loadFeedData(feed);

    QScopedPointer<KisManualUpdater> updater(new KisManualUpdater(mockRssModel, currentVersion));

    QSignalSpy spy(updater.data(), SIGNAL(sigUpdateCheckStateChange(KisUpdaterStatus)));

    QVERIFY(spy.isValid());

    updater->checkForUpdate();

    QTest::qWait(1000);

    QList<QVariant> arguments = spy.takeFirst();
    KisUpdaterStatus updaterStatus = arguments.at(0).value<KisUpdaterStatus>();

    QCOMPARE(updaterStatus.status(), resultStatus);
    QCOMPARE(updaterStatus.availableVersion(), availableVersion);
    QCOMPARE(updaterStatus.downloadLink(), downloadLink);
}

void KisManualUpdaterTest::testCheckForUpdate_data()
{
    QTest::addColumn<RssItemList>("feed");
    QTest::addColumn<QString>("currentVersion");
    QTest::addColumn<QString>("availableVersion");
    QTest::addColumn<QString>("downloadLink");
    QTest::addColumn<UpdaterStatus::StatusID>("resultStatus");

    RssItem item1 = RssItem();
    item1.title = "Krita 4.3.0 Released";
    item1.link = "https://krita.org/en/item/krita-4-3-0/";
    item1.category = "Official Release";
    item1.pubDate = QDateTime::fromString(QString("Wed, 10 Sep 2019 09:42:15 +0000"), Qt::RFC2822Date);

    RssItem item2 = RssItem();
    item2.title = "Krita 4.2.4 Released";
    item2.link = "https://krita.org/en/item/krita-4-2-4/";
    item2.category = "Official Release";
    item2.pubDate = QDateTime::fromString(QString("Wed, 10 Sep 2018 09:42:15 +0000"), Qt::RFC2822Date);

    RssItem item3 = RssItem();
    item3.title = "Interview with The Artist";
    item3.link = "https://krita.org/en/item/interview-with-the-artist/";
    item3.category = "Artist Interview";
    item3.pubDate = QDateTime::fromString(QString("Wed, 15 Sep 2019 09:42:15 +0000"), Qt::RFC2822Date);


    RssItemList feed = RssItemList() << item1 << item2 << item3;

    QTest::addRow("uptodate")
            << feed
            << QString("4.3.0")
            << QString("4.3.0")
            << QString("https://krita.org/en/item/krita-4-3-0/")
            << UpdaterStatus::StatusID::UPTODATE;

    QTest::addRow("update available")
            << feed
            << QString("4.2.4")
            << QString("4.3.0")
            << QString("https://krita.org/en/item/krita-4-3-0/")
            << UpdaterStatus::StatusID::UPDATE_AVAILABLE;
}

KISTEST_MAIN(KisManualUpdaterTest);
