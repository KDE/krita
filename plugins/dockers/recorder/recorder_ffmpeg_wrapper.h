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

#ifndef RECORDERFFMPEGWRAPPER_H
#define RECORDERFFMPEGWRAPPER_H

#include <QObject>

class QProcess;

class QProcess;

struct RecorderFFMpegWrapperSettings
{
    QString ffmpeg;
    int inputFps;
    QString inputDirectory;
    QString arguments;
    QString outputFilePath;
};


class RecorderFFMpegWrapper : public QObject
{
    Q_OBJECT
public:
    explicit RecorderFFMpegWrapper(QObject *parent = nullptr);

    void start(const RecorderFFMpegWrapperSettings &settings);
    void kill();

Q_SIGNALS:
    void started();
    void finished();
    void finishedWithError(QString message);
    void progressUpdated(int frameNo);

private Q_SLOTS:
    void onReadyRead();
    void onStarted();
    void onFinished(int exitCode);

private:
    QProcess *process = nullptr;
    QString readBuffer;
    QString errorMessage;
};

#endif // RECORDERFFMPEGWRAPPER_H
