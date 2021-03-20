/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "recorder_snapshots_scanner.h"
#include "recorder_const.h"

#include <QDebug>
#include <QThread>
#include <QDir>
#include <QDirIterator>
#include <QRegularExpression>

namespace
{
const int typeidSnapshotDirInfoList = qRegisterMetaType<SnapshotDirInfoList>("SnapshotDirInfoList");
}

RecorderSnapshotsScanner::RecorderSnapshotsScanner()
{
    moveToThread(this);
}

RecorderSnapshotsScanner::~RecorderSnapshotsScanner()
{
    stop();
}

void RecorderSnapshotsScanner::setup(const QString &snapshotDirectory)
{
    this->snapshotDirectory = snapshotDirectory;
}

void RecorderSnapshotsScanner::stop()
{
    if (!isRunning())
        return;

    requestInterruption();
    if (!wait(RecorderConst::waitThreadTimeoutMs)) {
        terminate();
        if (!wait(RecorderConst::waitThreadTimeoutMs)) {
            qCritical() << "Unable to stop RecorderSnapshotsScanner";
        }
    }
}

void RecorderSnapshotsScanner::run()
{
    QList<SnapshotDirInfo> result;

    QDirIterator dirIterator(snapshotDirectory, QDir::Dirs | QDir::NoDotAndDotDot);

    while (dirIterator.hasNext()) {
        dirIterator.next();

        const SnapshotDirInfo &info = readSnapshotDirInfo(dirIterator.filePath());
        if (isInterruptionRequested()) {
            return;
        }
        if (info.size > 0) {
            result.append(info);
        }
    }

    emit scanningFinished(result);
}


SnapshotDirInfo RecorderSnapshotsScanner::readSnapshotDirInfo(const QString &path) const
{
    SnapshotDirInfo result;
    QFileInfo fileInfo(path);
    result.path = path;
    result.name = fileInfo.fileName();

    int recordIndex = -1;
    QDirIterator dirIterator(path, QDir::Files | QDir::NoDotAndDotDot);

    while (dirIterator.hasNext()) {
        dirIterator.next();

        if (isInterruptionRequested()) {
            return {};
        }

        const QRegularExpressionMatch &match = RecorderConst::snapshotFilePattern.match(dirIterator.fileName());
        if (!match.hasMatch())
            continue;

        const QString &filePath = dirIterator.filePath();

        fileInfo.setFile(filePath);
        result.size += fileInfo.size();

        int index = match.captured(1).toInt();
        if (recordIndex < index) {
            recordIndex = index;
            result.thumbnail = filePath;
            result.dateTime = fileInfo.lastModified();
        }
    }

    return result;
}
