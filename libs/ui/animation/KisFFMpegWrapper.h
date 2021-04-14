/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Eoin O'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef KISFFMPEGWRAPPER_H
#define KISFFMPEGWRAPPER_H

#include <QObject>
#include <QProgressDialog>

#include "KisImportExportErrorCode.h"

#include <kritaimage_export.h>

class QProcess;

struct KRITAIMAGE_EXPORT KisFFMpegWrapperSettings
{
    QString processPath;
    QStringList args;
    QString outputFile;
    bool storeOutput = false;
    QString logPath = "";
    QStringList defaultPrependArgs = {"-hide_banner", "-nostdin", "-y"};
    bool batchMode = false;
    bool binaryOutput = false;
    int totalFrames = 0;

    QString progressMessage = "";
    bool progressIndeterminate = false;

};


class KRITAIMAGE_EXPORT  KisFFMpegWrapper : public QObject
{
    Q_OBJECT
public:
    explicit KisFFMpegWrapper(QObject *parent = nullptr);

    void startNonBlocking(const KisFFMpegWrapperSettings &settings);
    KisImportExportErrorCode start(const KisFFMpegWrapperSettings &settings);
    void waitForFinished(int msecs = 60000);
    void kill();

    static QJsonObject findProcessPath(const QString &processName, const QString &customLocation, bool processInfo);
    static QJsonObject findFFMpeg(const QString &customLocation);
    static QJsonObject findFFProbe(const QString &customLocation);
    static QJsonObject findProcessInfo(const QString &processName, const QString &processPath, bool includeProcessInfo);
    static QByteArray runProcessAndReturn(const QString &processPath, const QStringList &args, int msecs = 5000);
    QJsonObject ffprobe(const QString &inputFile, const QString &ffprobePath);
    QJsonObject ffmpegProbe(const QString &inputFile, const QString &ffmpegPath, bool batchMode);

Q_SIGNALS:
    void sigStarted();
    void sigFinished();
    void sigFinishedWithError(QString message);
    void sigProgressUpdated(int frameNo);
    void sigReadLine(int pipe, QString line);
    void sigReadSTDOUT(QByteArray stdoutBuffer);
    void sigReadSTDERR(QByteArray stderrBuffer);

private Q_SLOTS:
    void slotReadyReadSTDOUT();
    void slotReadyReadSTDERR();
    void slotStarted();
    void slotFinished(int exitCode);

private:
    void updateProgressDialog(int progressValue);

    
private:
    QScopedPointer<QProcess> process;
    QSharedPointer<QProgressDialog> progress = nullptr;
    KisFFMpegWrapperSettings processSettings;
    
    QString stdoutBuffer;
    QString stderrBuffer;
    QString errorMessage;
    
    QByteArray processSTDOUT;
    QString processSTDERR;

};


#endif // KISFFMPEGWRAPPER_H
