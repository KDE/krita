/*
 *  Copyright (c) 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
