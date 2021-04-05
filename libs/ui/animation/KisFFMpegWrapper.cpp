/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "KisFFMpegWrapper.h"

#include <klocalizedstring.h>

#include <KoResourcePaths.h>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QDebug>
#include <QRegularExpression>
#include <QApplication>

#include "KisPart.h"

namespace
{
QRegularExpression frameRegexp("^frame=[ ]*([0-9]+) .*$");
QRegularExpression lineDelimiter("[\n\r]");
QRegularExpression junkRegex("\\[[a-zA-Z0-9]+ @ 0x[a-fA-F0-9]*\\][ ]*");
QStringList errorWords = { "Unable", "Invalid", "Error", "failed", "NULL", "No such", "divisible", "not" };
QRegularExpression ffmpegVersionRX("(ffmpeg|ffprobe) version (.+?)\\s");

}

KisFFMpegWrapper::KisFFMpegWrapper(QObject *parent)
    : QObject(parent)
{
}




void KisFFMpegWrapper::start(const KisFFMpegWrapperSettings &settings)
{
    Q_ASSERT(process == nullptr); // shall never happen

    stdoutBuffer.clear();
    errorMessage.clear();
    processSTDOUT.clear();
    processSTDERR.clear();

    process = new QProcess(this);
    processSettings = settings;
    
    if ( !settings.logPath.isEmpty() )
        process->setStandardOutputFile(settings.logPath);
    
    if (!processSettings.batchMode) {
        QString progressText = processSettings.progressMessage;
    
        progressText.replace("[progress]", "0");
    
        progress = new QProgressDialog(progressText, "", 0, 0, KisPart::instance()->currentMainwindow());

        progress->setWindowModality(Qt::ApplicationModal);
        progress->setCancelButton(0);
        progress->setMinimumDuration(0);
        progress->setValue(0);
        progress->setRange(0, 100);
        
        progress->show();
        
        dbgFile << "Open progress dialog!";
    }

    connect(process, SIGNAL(readyReadStandardOutput()), SLOT(slotReadyReadSTDOUT()));
    connect(process, SIGNAL(readyReadStandardError()), SLOT(slotReadyReadSTDERR()));
    connect(process, SIGNAL(started()), SLOT(slotStarted()));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(slotFinished(int)));

    QStringList args;
  
    if ( !settings.defaultPrependArgs.isEmpty() ) {
        args << settings.defaultPrependArgs;
    }    
    
    args << settings.args;
    
    if ( !settings.outputFile.isEmpty() ) {
        args << settings.outputFile;
    }

    dbgFile << "starting process: " << qUtf8Printable(settings.processPath) << args;

    process->start(settings.processPath, args);
}

void KisFFMpegWrapper::waitForFinished(int msecs)
{
    if (process == nullptr) return;

    if (process->waitForStarted(msecs)) process->waitForFinished(msecs);

}



QByteArray KisFFMpegWrapper::stdout() {
    return processSTDOUT;
}

QString KisFFMpegWrapper::stderr() {
    return processSTDERR;
}

void KisFFMpegWrapper::updateProgressDialog(int progressValue) {
    
    dbgFile << "Update Progress" << progressValue << "/" << processSettings.totalFrames;
    
    if (progress == nullptr) return;

    QString progressText = processSettings.progressMessage;

    progressText.replace("[progress]", QString::number(progressValue));
    progress->setLabelText(progressText);

    if (processSettings.totalFrames > 0) progress->setValue(progressValue/processSettings.totalFrames);
     QApplication::processEvents();

}

void KisFFMpegWrapper::kill()
{
    if (process == nullptr)
        return;

    process->disconnect(this);
    process->kill();
    process->deleteLater();
    process = nullptr;
}

void KisFFMpegWrapper::slotReadyReadSTDERR()
{
    QByteArray stderrRawBuffer = process->readAllStandardError();
    
    emit sigReadSTDOUT(stderrRawBuffer);
    stderrBuffer += stderrRawBuffer;
    
    int frameNo = -1;
    int startPos = 0;
    int endPos = 0;
    
    while ((endPos = stderrBuffer.indexOf(lineDelimiter, startPos)) != -1) {
        const QString &line = stderrBuffer.mid(startPos, endPos - startPos).trimmed();

        if (processSettings.storeOutput) processSTDERR += line + "\n";
                
        emit sigReadLine(2,line);
        
        for (const QString &word : errorWords) {
            if (line.contains(word)) {
                errorMessage += line % "\n";
                break;
            }
        }
        
        const QRegularExpressionMatch &match = frameRegexp.match(line);
        
        if (match.hasMatch()) {
            frameNo = match.captured(1).toInt();
        }


        dbgFile << "ffmpeg stderr:" << line;
        startPos = endPos + 1;
    }

    stderrBuffer.remove(0, startPos);
    
    if (frameNo != -1) {
        updateProgressDialog(frameNo);
        emit sigProgressUpdated(frameNo);
    }
    
}

void KisFFMpegWrapper::slotReadyReadSTDOUT()
{
    QByteArray stdoutRawBuffer = process->readAllStandardOutput();
    
    emit sigReadSTDOUT(stdoutRawBuffer);
    stdoutBuffer += stdoutRawBuffer;

  
    if (processSettings.binaryOutput) {
        if (processSettings.storeOutput) processSTDOUT += stdoutRawBuffer;
    } else {
        
        int startPos = 0;
        int endPos = 0;
        QString str;
        
        if (processSettings.storeOutput) processSTDOUT += stdoutRawBuffer + "\n";
        
        // ffmpeg splits normal lines by '\n' and progress data lines by '\r'
        while ((endPos = stdoutBuffer.indexOf(lineDelimiter, startPos)) != -1) {
            const QString &line = stdoutBuffer.mid(startPos, endPos - startPos).trimmed();

            dbgFile << "ffmpeg stdout:" << line;
            emit sigReadLine(1,line);
            startPos = endPos + 1;
        }
        
        stdoutBuffer.remove(0, startPos);
    }

    
}

void KisFFMpegWrapper::slotStarted()
{
    dbgFile << "ffmpeg process started!";

    emit sigStarted();
}

void KisFFMpegWrapper::slotFinished(int exitCode)
{
    dbgFile << "FFMpeg finished with code" << exitCode;
    if (!processSettings.batchMode && progress != nullptr) progress->setValue(100);
    if (exitCode != 0) {
        errorMessage.remove(junkRegex);
        if (process->exitStatus() == QProcess::CrashExit) {
            errorMessage = i18n("FFMpeg Crashed") % "\n" % errorMessage;
        }
        emit sigFinishedWithError(errorMessage);
    } else {
        emit sigFinished();
    }

    process->deleteLater();
    process = nullptr;
}

QByteArray KisFFMpegWrapper::runProcessAndReturn(const QString &processPath, const QStringList &args, int msecs)
{
    QProcess runProcess;
    
    dbgFile << "runProcessAndReturn:" << processPath << args;
    
    runProcess.start(processPath, args);
    
    if (runProcess.waitForStarted(msecs)) runProcess.waitForFinished(msecs);

    const bool successfulStart =
        runProcess.state() == QProcess::NotRunning &&
        runProcess.error() == QProcess::UnknownError;

    dbgFile << "runProcessAndReturn Success:" << successfulStart;
    
    if (successfulStart) return runProcess.readAllStandardOutput();

    return "";
}

QJsonObject KisFFMpegWrapper::findProcessPath(const QString &processName, const QString &customLocation, bool includeProcessInfo)
{
    QJsonObject resultJsonObj;
    QStringList proposedPaths;

    if (!customLocation.isEmpty()) {
        proposedPaths << customLocation;
        proposedPaths << customLocation + '/' + processName;
    }

    proposedPaths << KoResourcePaths::getApplicationRoot()
                   + '/' + "bin" + '/' + processName;

#ifndef Q_OS_WIN
    proposedPaths << QDir::homePath() + "/bin/" + processName;
    proposedPaths << "/usr/bin/" + processName;
    proposedPaths << "/usr/local/bin/" + processName;
#endif
    dbgFile << proposedPaths;
    for (auto end = proposedPaths.size(), i = 0; i != end; ++i) {
        if (proposedPaths[i].isEmpty()) continue;

#ifdef Q_OS_WIN
        proposedPaths[i] = QDir::toNativeSeparators(QDir::cleanPath(proposedPaths[i]));
        if (proposedPaths[i].endsWith('/')) {
            continue;
        }
        if (!proposedPaths[i].endsWith(".exe")) {
            if (!QFile::exists(proposedPaths[i])) {
                proposedPaths[i] += ".exe";
                if (!QFile::exists(proposedPaths[i])) {
                    continue;
                }
            }
        }
#endif

        QJsonObject processInfoJsonObj = findProcessInfo(processName, proposedPaths[i], includeProcessInfo);
        dbgFile << "PATH" << proposedPaths[i] << processInfoJsonObj.value("enabled").toBool();
        if (processInfoJsonObj.value("enabled").toBool()) {
            processInfoJsonObj["custom"]=(!customLocation.isEmpty() && i <= 1) ? true:false;
            resultJsonObj = processInfoJsonObj;
            break;
        }        
    }

    return resultJsonObj;
}

QJsonObject KisFFMpegWrapper::findProcessInfo(const QString &processName, const QString &processPath, bool includeProcessInfo)
{

    QJsonObject ffmpegInfo {{"path", processPath},
                            {"enabled",false},
                            {"version", "0"},
                            {"encoder",QJsonValue::Object},
                            {"decoder",QJsonValue::Object}};
                            
    if (!QFile::exists(processPath)) return ffmpegInfo;
    
    dbgFile << "Found process at:" << processPath;
    
    QString processVersion = KisFFMpegWrapper::runProcessAndReturn(processPath, QStringList() << "-version",1000);

    if (!processVersion.isEmpty()) {
        
        QRegularExpressionMatch versionMatch = ffmpegVersionRX.match(processVersion);
        
        if (versionMatch.hasMatch() && versionMatch.captured(1) == processName ) {
            ffmpegInfo["version"] = versionMatch.captured(2);
            dbgFile << "found version" << ffmpegInfo.value("version").toString();
            ffmpegInfo["enabled"] = true;
        }
        
        if (!includeProcessInfo || !ffmpegInfo["enabled"].toBool()) return ffmpegInfo;
        
        QString processCodecs = KisFFMpegWrapper::runProcessAndReturn(processPath, QStringList() << "-codecs",1000);

        QRegularExpression ffmpegCodecsRX("(D|\\.)(E|\\.)....\\s+(.+?)\\s+");
        QRegularExpressionMatchIterator codecsMatchList = ffmpegCodecsRX.globalMatch(processCodecs);
        
        QJsonObject encoderJsonObj;
        QJsonObject decoderJsonObj;
            
        while (codecsMatchList.hasNext()) {
            QRegularExpressionMatch codecsMatch = codecsMatchList.next();

            if (codecsMatch.hasMatch()) {
                QString codecName = codecsMatch.captured(3);
                decoderJsonObj[codecName] = codecsMatch.captured(1) == "D" ? true:false;
                encoderJsonObj[codecName] = codecsMatch.captured(2) == "E" ? true:false;
            }
        }
        
        ffmpegInfo.insert("encoder",encoderJsonObj);
        ffmpegInfo.insert("decoder",decoderJsonObj);
            
        dbgFile << "codec support:" << ffmpegInfo;
        
 
    } else {
        dbgFile << "Not a valid process at:" << processPath;
    }
    
    return ffmpegInfo;
    
}

QJsonObject KisFFMpegWrapper::findFFMpeg(const QString &customLocation)
{
    return findProcessPath("ffmpeg", customLocation, true);
}

QJsonObject KisFFMpegWrapper::findFFProbe(const QString &customLocation)
{
    return findProcessPath("ffprobe", customLocation, false);
}



QJsonObject KisFFMpegWrapper::ffprobe(const QString &inputFile, const QString &ffprobePath) 
{
    struct KisFFMpegWrapperSettings ffprobeSettings;
     
    ffprobeSettings.processPath = ffprobePath;
    ffprobeSettings.storeOutput = true;
    ffprobeSettings.defaultPrependArgs.clear();
    
    ffprobeSettings.args << "-hide_banner" 
                         << "-v" << "warning" 
                         << "-of" << "json=compact=1" 
                         << "-show_format" 
                         << "-show_streams" 
                         << "-i" << inputFile;
    this->start(ffprobeSettings);
    this->waitForFinished();
    
    QString ffprobeSTDOUT = this->stdout();
    QString ffprobeSTDERR = this->stderr();
     
    QJsonDocument ffprobeJsonDoc = QJsonDocument::fromJson(ffprobeSTDOUT.toUtf8());
    QJsonObject ffprobeJsonObj;

    if (!ffprobeJsonDoc.isNull()) {
        if (ffprobeJsonDoc.isObject()) {
            ffprobeJsonObj = ffprobeJsonDoc.object();
           
            ffprobeJsonObj["error"] = (ffprobeSTDERR.indexOf("Unsupported codec with id") == -1) ? 0:1;
            
        } else {
            dbgFile << "Document is not an object";
            ffprobeJsonObj["error"] = 3;
        }
    } else {
        dbgFile << "Invalid JSON...";
        ffprobeJsonObj["error"] = 2;
        
    } 
    
    return ffprobeJsonObj;
}

QJsonObject KisFFMpegWrapper::ffmpegProbe(const QString &inputFile, const QString &ffmpegPath, bool batchMode) 
{
    struct KisFFMpegWrapperSettings ffmpegSettings;
     
    ffmpegSettings.processPath = ffmpegPath;
    ffmpegSettings.storeOutput = true;
    ffmpegSettings.progressMessage = "Loading video data... [progress] frames loaded";
    ffmpegSettings.batchMode = batchMode;

    ffmpegSettings.args << "-stats"
                        << "-v" << "info" 
                        << "-progress" << "pipe:1" 
                        << "-map" << "0:v:0"
                        << "-c" << "copy"
                        << "-f" << "null" << "pipe:1"
                        << "-i" << inputFile;
                        

    this->start(ffmpegSettings);
    this->waitForFinished();

    
    QString ffmpegSTDOUT = this->stderr() + "\n" + this->stdout();
    
    dbgFile << "ffmpegProbe stdout:" << ffmpegSTDOUT;


    QJsonObject ffmpegJsonObj;
    QJsonArray ffmpegStreamsJsonArr;
    QJsonObject ffmpegFormatJsonObj;
    QJsonObject ffmpegProgressJsonObj;
    
    QStringList stdoutLines = ffmpegSTDOUT.split('\n');
    
    ffmpegJsonObj["error"] = 4;
    
    for (const QString &line : stdoutLines) {
        dbgFile << "ffmpeg probe stdout" << line;
       QRegularExpression videoInfoInputRX("Duration: (\\d+):(\\d+):([\\d\\.]+),");
    
        QRegularExpressionMatch durationMatch = videoInfoInputRX.match(line);
        if (ffmpegFormatJsonObj.value("duration").isUndefined() && durationMatch.hasMatch()) {
            ffmpegFormatJsonObj["duration"] = QString::number( (durationMatch.captured(1).toInt() * 60 * 60)
                                                             + (durationMatch.captured(2).toInt() * 60) 
                                                             + durationMatch.captured(3).toFloat()
                                                             );
        } else {
            QRegularExpression videoInfoStreamRX(
                QString("Stream #(\\d+):(\\d+)(?:[ ]*?\\((\\w+)\\)|): Video: (\\w+?)(?:[ ]*?\\((.+?)\\)|),[ ]*?")
                        .append("(\\w+?)(?:[ ]*?\\((.+?)\\)|),[ ]*?")
                        .append("(\\d+)x(\\d+)([ ]*?\\[.+?\\]|.*?),[ ]*?")
                        .append("([\\d\\.]+) fps, (.+?) tbr, (.+?) tbn, (.+?) tbc")
            );
            
            QRegularExpressionMatch streamMatch = videoInfoStreamRX.match(line);
            
            if (streamMatch.hasMatch() && ffmpegStreamsJsonArr[streamMatch.captured(1).toInt()].isUndefined() ) {
                int index = streamMatch.captured(1).toInt();
                QJsonObject ffmpegJsonOnStreamObj;
                
                ffmpegJsonOnStreamObj["index"] = index;
                ffmpegJsonOnStreamObj["codec_name"] = streamMatch.captured(4);
                ffmpegJsonOnStreamObj["profile"] = streamMatch.captured(5);
                ffmpegJsonOnStreamObj["pix_fmt"] = streamMatch.captured(6);
                ffmpegJsonOnStreamObj["width"] = streamMatch.captured(8).toInt();
                ffmpegJsonOnStreamObj["height"] = streamMatch.captured(9).toInt();
                
                ffmpegJsonOnStreamObj["codec_type"] = "video";
                
                if (streamMatch.captured(11).toFloat() > 0)
                    ffmpegJsonOnStreamObj["r_frame_rate"] = QString::number(streamMatch.captured(11).toFloat() * 10000).append(QString("/10000"));

                ffmpegJsonObj["error"] = 0;
                
                dbgFile << "ffmpegProbe stream:" << ffmpegJsonOnStreamObj;
                
                ffmpegStreamsJsonArr.insert(index, ffmpegJsonOnStreamObj);
                
            } else {
                
                QRegularExpression videoInfoFrameRX("^(\\w+?)=([\\w\\./:]+?)$");
                QRegularExpressionMatch frameMatch = videoInfoFrameRX.match(line);

                if (frameMatch.hasMatch()) ffmpegProgressJsonObj[frameMatch.captured(1)] = frameMatch.captured(2);
    
            }
        }
    }
    
    
    ffmpegJsonObj.insert("streams",ffmpegStreamsJsonArr);
    ffmpegJsonObj.insert("format",ffmpegFormatJsonObj);
    ffmpegJsonObj.insert("progress",ffmpegProgressJsonObj);
    
    return ffmpegJsonObj;
}
