/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef RECORDERSNAPSHOTSSCANNER_H
#define RECORDERSNAPSHOTSSCANNER_H

#include <QThread>
#include <QDateTime>

struct SnapshotDirInfo
{
    QString path;
    QString name;
    quint64 size = 0;
    QDateTime dateTime;
    QString thumbnail;
};
typedef QList<SnapshotDirInfo> SnapshotDirInfoList;


class RecorderSnapshotsScanner : public QThread
{
    Q_OBJECT
public:
    RecorderSnapshotsScanner();
    ~RecorderSnapshotsScanner();

    void setup(const QString &snapshotDirectory);
    void stop();

Q_SIGNALS:
    void scanningFinished(SnapshotDirInfoList snapshots);

protected:
    void run() override;

private:
    SnapshotDirInfo readSnapshotDirInfo(const QString &path) const;

private:
    QString snapshotDirectory;
};

#endif // RECORDERSNAPSHOTSSCANNER_H
