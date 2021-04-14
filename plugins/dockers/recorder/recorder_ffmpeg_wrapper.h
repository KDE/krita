/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef RECORDERFFMPEGWRAPPER_H
#define RECORDERFFMPEGWRAPPER_H

#include <QObject>

class QProcess;

class QProcess;

//struct RecorderFFMpegWrapperSettings
//{
//    QString ffmpeg;
//    QString arguments;
//    QString outputFilePath;
//};


//class RecorderFFMpegWrapper : public QObject
//{
//    Q_OBJECT
//public:
//    explicit RecorderFFMpegWrapper(QObject *parent = nullptr);

//    void start(const RecorderFFMpegWrapperSettings &settings);
//    void kill();

//Q_SIGNALS:
//    void started();
//    void finished();
//    void finishedWithError(QString message);
//    void progressUpdated(int frameNo);

//private Q_SLOTS:
//    void onReadyRead();
//    void onStarted();
//    void onFinished(int exitCode);

//private:
//    QProcess *process = nullptr;
//    QString readBuffer;
//    QString errorMessage;
//};

#endif // RECORDERFFMPEGWRAPPER_H
