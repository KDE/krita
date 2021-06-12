/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAppimageUpdaterTest.h"

#include <simpletest.h>
#include <QString>
#include <QSignalSpy>
#include <QFile>
#include <QScopedPointer>

#include <KisUpdaterStatus.h>
#include <KisAppimageUpdater.h>
#include <testutil.h>

KisAppimageUpdaterTest::KisAppimageUpdaterTest(QObject *parent) : QObject(parent)
{

}

void KisAppimageUpdaterTest::testCheckForUpdate()
{
    QFETCH(QString, controlValue);
    QFETCH(UpdaterStatus::StatusID, resultStatus);

    QString updaterDummyPath = QString("%1%2/AppImageUpdateDummy")
            .arg(QString(FILES_DATA_DIR))
            .arg(QDir::separator());


    qputenv("APPIMAGEUPDATE_DUMMY_STATE", controlValue.toLocal8Bit());

    QScopedPointer<KisAppimageUpdater> updater(new KisAppimageUpdater(updaterDummyPath));

    QSignalSpy spy(updater.data(), SIGNAL(sigUpdateCheckStateChange(KisUpdaterStatus)));

    QVERIFY(spy.isValid());

    updater->checkForUpdate();

    QTest::qWait(1000);

    QList<QVariant> arguments = spy.takeFirst();
    KisUpdaterStatus firstStatus = arguments.at(0).value<KisUpdaterStatus>();

    QCOMPARE(firstStatus.status(), UpdaterStatus::StatusID::IN_PROGRESS);

    arguments = spy.takeFirst();
    KisUpdaterStatus secondStatus = arguments.at(0).value<KisUpdaterStatus>();

    QCOMPARE(secondStatus.status(), resultStatus);
}

void KisAppimageUpdaterTest::testCheckForUpdate_data()
{
    QTest::addColumn<QString>("controlValue");
    QTest::addColumn<UpdaterStatus::StatusID>("resultStatus");

    QTest::addRow("uptodate")
            << QString("check_uptodate")
            << UpdaterStatus::StatusID::UPTODATE;

    QTest::addRow("update available")
            << QString("check_update_avail")
            << UpdaterStatus::StatusID::UPDATE_AVAILABLE;

    QTest::addRow("error")
            << QString("check_error")
            << UpdaterStatus::StatusID::CHECK_ERROR;

    QTest::addRow("empty update info")
            << QString("check_updinfo_empty")
            << UpdaterStatus::StatusID::CHECK_ERROR;
}

void KisAppimageUpdaterTest::testDoUpdate()
{
    QFETCH(QString, controlValue);
    QFETCH(UpdaterStatus::StatusID, resultStatus);

    QString updaterDummyPath = QString("%1%2/AppImageUpdateDummy")
            .arg(QString(FILES_DATA_DIR))
            .arg(QDir::separator());


    qputenv("APPIMAGEUPDATE_DUMMY_STATE", controlValue.toLocal8Bit());

    QScopedPointer<KisAppimageUpdater> updater(new KisAppimageUpdater(updaterDummyPath));

    QSignalSpy spy(updater.data(), SIGNAL(sigUpdateCheckStateChange(KisUpdaterStatus)));

    QVERIFY(spy.isValid());

    updater->doUpdate();

    QTest::qWait(1000);

    QList<QVariant> arguments = spy.takeFirst();
    KisUpdaterStatus secondStatus = arguments.at(0).value<KisUpdaterStatus>();

    QCOMPARE(secondStatus.status(), resultStatus);

    // for error also check for output
    if (secondStatus.status() == UpdaterStatus::StatusID::UPDATE_ERROR) {
        QCOMPARE(secondStatus.updaterOutput(), QString("DUMMY: an error occurred\n"));
    }
}

void KisAppimageUpdaterTest::testDoUpdate_data()
{
    QTest::addColumn<QString>("controlValue");
    QTest::addColumn<UpdaterStatus::StatusID>("resultStatus");

    QTest::addRow("update ok")
            << QString("update_ok")
            << UpdaterStatus::StatusID::RESTART_REQUIRED;

    QTest::addRow("error")
            << QString("update_error")
            << UpdaterStatus::StatusID::UPDATE_ERROR;

    QTest::addRow("runtime error")
            << QString("runtime_error")
            << UpdaterStatus::StatusID::UPDATE_ERROR;
}

SIMPLE_TEST_MAIN(KisAppimageUpdaterTest);
