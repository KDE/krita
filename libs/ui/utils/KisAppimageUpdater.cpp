/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "KisAppimageUpdater.h"

#include <QProcess>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <QDir>

#include <KisUsageLogger.h>
#include <kritagitversion.h>

#include <klocalizedstring.h>

KisAppimageUpdater::KisAppimageUpdater()
    : m_checkProcess(new QProcess(this))
    , m_updateProcess(new QProcess(this))
{
    QString updaterPath;

#if defined(KRITA_GIT_SHA1_STRING)
    if (qEnvironmentVariableIsSet("KRITA_APPIMAGEUPDATER_USE_DUMMY")) {
        updaterPath = QString("%1%2AppImageUpdateDummy")
                .arg(QCoreApplication::applicationDirPath())
                .arg(QDir::separator());
    } else {
        updaterPath = QString("%1%2AppImageUpdate")
                .arg(QCoreApplication::applicationDirPath())
                .arg(QDir::separator());
    }
#else
    updaterPath = QString("%1%2AppImageUpdate")
            .arg(QCoreApplication::applicationDirPath())
            .arg(QDir::separator());
#endif

    initialize(updaterPath);
}

KisAppimageUpdater::KisAppimageUpdater(QString dummyUpdaterPath)
    : m_checkProcess(new QProcess(this))
    , m_updateProcess(new QProcess(this))
{
    initialize(dummyUpdaterPath);
}

void KisAppimageUpdater::checkForUpdate()
{
    if (m_updaterInProgress) {
        return;
    }

    if (!m_updateCapability) {
        return;
    }

    // reset output for subsequent checks, is this needed?
    m_checkOutput = QString();
    m_updateOutput = QString();
    m_updaterStatus.setUpdaterOutput(QString());

    QStringList args = QStringList() << "--check-for-update" << m_appimagePath;

    m_checkProcess->start(m_updaterBinary, args);
    m_updaterInProgress = true;
}

bool KisAppimageUpdater::hasUpdateCapability()
{
    return m_updateCapability;
}

void KisAppimageUpdater::doUpdate()
{
    QStringList args = QStringList() << m_appimagePath;
    m_updateProcess->start(m_updaterBinary, args);
}

void KisAppimageUpdater::slotUpdateCheckFinished(int result, QProcess::ExitStatus exitStatus)
{
    KisUsageLogger::log(
                QString("KisAppimageUpdater: update check finished. Result: %1 Exit status: %2\npath: %3\noutput: %4")
                .arg(result)
                .arg(exitStatus)
                .arg(m_appimagePath)
                .arg(m_updateOutput)
                );

    UpdaterStatus::StatusID updateStatus;

    if (exitStatus == QProcess::CrashExit) {
        updateStatus = UpdaterStatus::StatusID::CHECK_ERROR;

    } else {
        switch (result) {
        case 0:
            updateStatus = UpdaterStatus::StatusID::UPTODATE;
            break;
        case 1:
            updateStatus = UpdaterStatus::StatusID::UPDATE_AVAILABLE;
            break;
        case 2:
            updateStatus = UpdaterStatus::StatusID::CHECK_ERROR;
            break;
        default:
            // some errors have exit code of 255 (modified by system, when AppImageUpdate returns -1)
            // one source of 255 is when the AppImage does not contain update information
            updateStatus = UpdaterStatus::StatusID::CHECK_ERROR;
            break;
        }
    }

    m_updaterInProgress = false;

    m_updaterStatus.setStatus(updateStatus);
    m_updaterStatus.setUpdaterOutput(m_updateOutput);

    emit sigUpdateCheckStateChange(m_updaterStatus);
}

void KisAppimageUpdater::slotUpdateCheckStarted()
{
    m_updaterStatus.setStatus(UpdaterStatus::StatusID::IN_PROGRESS);
    emit sigUpdateCheckStateChange(m_updaterStatus);
}

void KisAppimageUpdater::slotUpdateCheckErrorOccurred(QProcess::ProcessError error)
{
    KisUsageLogger::log(
                QString("KisAppimageUpdater: error occurred during update check: %1\npath: %2\noutput: %3")
                .arg(error)
                .arg(m_appimagePath)
                .arg(m_checkOutput)
                );

    m_updaterInProgress = false;

    m_updaterStatus.setStatus(UpdaterStatus::StatusID::CHECK_ERROR);

    emit sigUpdateCheckStateChange(m_updaterStatus);
}

void KisAppimageUpdater::slotUpdateFinished(int result, QProcess::ExitStatus exitStatus)
{
    KisUsageLogger::log(
                QString("KisAppimageUpdater: update finished. Result: %1\nExit status: %2\npath: %3\noutput: %4")
                .arg(result)
                .arg(exitStatus)
                .arg(m_appimagePath)
                .arg(m_updateOutput)
                );

    UpdaterStatus::StatusID updateStatus;
    QFileInfo finfoAppImagePath(m_appimagePath);
    QString statusDetails;

    if (exitStatus == QProcess::CrashExit) {
        updateStatus = UpdaterStatus::StatusID::UPDATE_ERROR;

    } else {
        switch (result) {
        case 0:
            updateStatus = UpdaterStatus::StatusID::RESTART_REQUIRED;
            statusDetails = i18n("New AppImage was downloaded to %1. To complete the update, close Krita and run the new AppImage.", finfoAppImagePath.path());
            break;
        default:
            // some errors have exit code of 255 (modified by system, when AppImageUpdate returns -1)
            updateStatus = UpdaterStatus::StatusID::UPDATE_ERROR;
            break;
        }
    }

    m_updaterInProgress = false;

    m_updaterStatus.setStatus(updateStatus);
    m_updaterStatus.setUpdaterOutput(m_updateOutput);
    m_updaterStatus.setDetails(statusDetails);

    emit sigUpdateCheckStateChange(m_updaterStatus);
}

void KisAppimageUpdater::slotUpdateErrorOccurred(QProcess::ProcessError error)
{
    KisUsageLogger::log(
                QString("KisAppimageUpdater: error occurred during update: %1\npath: %2\noutput: %3")
                .arg(error)
                .arg(m_appimagePath)
                .arg(m_updateOutput)
                );

    m_updaterInProgress = false;

    m_updaterStatus.setStatus(UpdaterStatus::StatusID::UPDATE_ERROR);
    m_updaterStatus.setUpdaterOutput(m_updateOutput);

    emit sigUpdateCheckStateChange(m_updaterStatus);
}

void KisAppimageUpdater::slotAppendCheckOutput()
{
    m_checkOutput.append(m_checkProcess->readAllStandardOutput());
}

void KisAppimageUpdater::slotAppendUpdateOutput()
{
    m_updateOutput.append(m_updateProcess->readAllStandardOutput());
}

#if (QT_VERSION < QT_VERSION_CHECK(5, 10, 0))
QString qEnvironmentVariable(const char *varName) {
    return qgetenv(varName);
}
#endif

void KisAppimageUpdater::initialize(QString& updaterPath)
{
    m_appimagePath = qEnvironmentVariable("APPIMAGE");
    m_updaterBinary = updaterPath;

    m_updateCapability = findUpdaterBinary();

    m_checkProcess->setProcessChannelMode(QProcess::MergedChannels);
    m_updateProcess->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_checkProcess.data(), SIGNAL(started()), this, SLOT(slotUpdateCheckStarted()));
    connect(m_checkProcess.data(), SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(slotUpdateCheckErrorOccurred(QProcess::ProcessError)));
    connect(m_checkProcess.data(), SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotUpdateCheckFinished(int, QProcess::ExitStatus)));
    connect(m_checkProcess.data(), SIGNAL(readyReadStandardOutput()),
            this, SLOT(slotAppendCheckOutput()));


    connect(m_updateProcess.data(), SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(slotUpdateErrorOccurred(QProcess::ProcessError)));
    connect(m_updateProcess.data(), SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(slotUpdateFinished(int, QProcess::ExitStatus)));
    connect(m_updateProcess.data(), SIGNAL(readyReadStandardOutput()),
            this, SLOT(slotAppendUpdateOutput()));
}

bool KisAppimageUpdater::findUpdaterBinary()
{
    QFileInfo finfo(m_updaterBinary);
    if (finfo.isExecutable()) {
        return true;
    } else {
        KisUsageLogger::log(
                    QString("KisAppimageUpdater: AppImageUpdate (%1) was not found within the Krita appimage, or is not executable")
                    .arg(m_updaterBinary)
                    );
        return false;
    }
}
