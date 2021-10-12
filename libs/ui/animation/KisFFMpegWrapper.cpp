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

#include "kis_config.h"

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

KisFFMpegWrapper::~KisFFMpegWrapper()
{
}


void KisFFMpegWrapper::startNonBlocking(const KisFFMpegWrapperSettings &settings)
{
    KIS_ASSERT(m_process == nullptr);

    m_stdoutBuffer.clear();
    m_errorMessage.clear();
    m_processSTDOUT.clear();
    m_processSTDERR.clear();

    m_process.reset(new QProcess(this));
    m_processSettings = settings;
    
    if ( !settings.logPath.isEmpty() ) {
        const QString basePath = QFileInfo(settings.logPath).dir().path();
        QDir().mkpath(basePath);
        
        //First we open the file with truncate,
        //Then, we connect our signals for response
        //After that, every message will append to that file if possible...
        QFile loggingFile(settings.logPath);
        if (loggingFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            //Due to various reasons (including image preview), ffmpeg uses STDERR and not STDOUT
            //for general output logging.
            connect(this, &KisFFMpegWrapper::sigReadSTDERR, [this](QByteArray stderrBuffer){
                QString line = stderrBuffer;
                QFile loggingFile(m_processSettings.logPath);
                if (loggingFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
                    loggingFile.write(stderrBuffer);
                }
            });
            
            if (!settings.outputFile.isEmpty()) {
                connect(this, &KisFFMpegWrapper::sigFinishedWithError, [this](QString){
                    QFile loggingFile(m_processSettings.logPath);
                    QString targetPath = m_processSettings.outputFile + ".log";
                    
                    if (QFile::exists(targetPath)) {
                        QFile existingFile(targetPath);
                        existingFile.remove();
                    }
                    
                    loggingFile.copy(targetPath);
                });
            }
        }
    }
    
    if (!m_processSettings.batchMode) {
        QString progressText = m_processSettings.progressMessage;
    
        progressText.replace("[progress]", "0");
    
        m_progress = toQShared(new QProgressDialog(progressText, "", 0, 0, KisPart::instance()->currentMainwindow()));

        m_progress->setWindowModality(Qt::ApplicationModal);
        m_progress->setCancelButton(0);
        m_progress->setMinimumDuration(0);
        m_progress->setValue(0);

        if (settings.progressIndeterminate) {
            m_progress->setRange(0,0);
        } else {
            m_progress->setRange(0, 100);
        }
        
        m_progress->show();
        
        dbgFile << "Open progress dialog!";
    }

    connect(m_process.data(), SIGNAL(readyReadStandardOutput()), SLOT(slotReadyReadSTDOUT()));
    connect(m_process.data(), SIGNAL(readyReadStandardError()), SLOT(slotReadyReadSTDERR()));
    connect(m_process.data(), SIGNAL(started()), SLOT(slotStarted()));
    connect(m_process.data(), SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(slotFinished(int)));

    QStringList args;
  
    if ( !settings.defaultPrependArgs.isEmpty() ) {
        args << settings.defaultPrependArgs;
    }    
    
    args << settings.args;
    
    if ( !settings.outputFile.isEmpty() ) {
        args << settings.outputFile;
    }

    dbgFile << "starting process: " << qUtf8Printable(settings.processPath) << args;

    m_process->start(settings.processPath, args);
}

KisImportExportErrorCode KisFFMpegWrapper::start(const KisFFMpegWrapperSettings &settings)
{
    struct ProcessResults {
        bool finish = false;
        QString error = QString();
    };

    QSharedPointer<ProcessResults> processResults = toQShared(new ProcessResults);

    connect( this, &KisFFMpegWrapper::sigFinishedWithError, [processResults](const QString& errMsg){
        processResults->finish = true;
        processResults->error = errMsg;
    });

    connect( this, &KisFFMpegWrapper::sigFinished, [processResults](){
       processResults->finish = true;
    });

    startNonBlocking(settings);
    waitForFinished(FFMPEG_TIMEOUT);

    if (processResults->finish == true) {
        if (processResults->error.isEmpty()) {
            return ImportExportCodes::OK;
        } else {
            return ImportExportCodes::Failure;
        }
    } else {
        kill(); // Ensure process isn't running before returning failure here.
        return ImportExportCodes::Failure;
    }
}

void KisFFMpegWrapper::waitForFinished(int msecs)
{
    if (!m_process) return;

    if (m_process->waitForStarted(msecs)) m_process->waitForFinished(msecs);

}

void KisFFMpegWrapper::updateProgressDialog(int progressValue) {
    
    dbgFile << "Update Progress" << progressValue << "/" << m_processSettings.totalFrames;
    
    if (!m_progress) return;

    QString progressText = m_processSettings.progressMessage;
    QStringList outputFileParts = m_processSettings.outputFile.split(".");
    QString suffix = outputFileParts.size() == 2 ? outputFileParts[1] : m_processSettings.outputFile;

    progressText.replace("[progress]", QString::number(progressValue));
    progressText.replace("[framecount]", QString::number(m_processSettings.totalFrames));
    progressText.replace("[suffix]", suffix );
    m_progress->setLabelText(progressText);

    if (m_processSettings.totalFrames > 0) m_progress->setValue(100 * progressValue / m_processSettings.totalFrames);

    QApplication::processEvents();
}

void KisFFMpegWrapper::kill()
{
    if (m_process == nullptr)
        return;

    m_process->disconnect(this);
    m_process->kill();
    m_process.reset();
}

void KisFFMpegWrapper::slotReadyReadSTDERR()
{
    QByteArray stderrRawBuffer = m_process->readAllStandardError();
    
    emit sigReadSTDERR(stderrRawBuffer);
    m_stderrBuffer += stderrRawBuffer;
    
    int frameNo = -1;
    int startPos = 0;
    int endPos = 0;
    
    while ((endPos = m_stderrBuffer.indexOf(lineDelimiter, startPos)) != -1) {
        const QString &line = m_stderrBuffer.mid(startPos, endPos - startPos).trimmed();

        if (m_processSettings.storeOutput) m_processSTDERR += line + "\n";
                
        emit sigReadLine(2,line);
        
        for (const QString &word : errorWords) {
            if (line.contains(word)) {
                m_errorMessage += line % "\n";
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

    m_stderrBuffer.remove(0, startPos);
    
    if (frameNo != -1) {
        updateProgressDialog(frameNo);
        emit sigProgressUpdated(frameNo);
    }
    
}

void KisFFMpegWrapper::slotReadyReadSTDOUT()
{
    QByteArray stdoutRawBuffer = m_process->readAllStandardOutput();
    
    emit sigReadSTDOUT(stdoutRawBuffer);
    m_stdoutBuffer += stdoutRawBuffer;

  
    if (m_processSettings.binaryOutput) {
        if (m_processSettings.storeOutput) m_processSTDOUT += stdoutRawBuffer;
    } else {
        
        int startPos = 0;
        int endPos = 0;
        QString str;
        
        if (m_processSettings.storeOutput) m_processSTDOUT += stdoutRawBuffer + "\n";
        
        // ffmpeg splits normal lines by '\n' and progress data lines by '\r'
        while ((endPos = m_stdoutBuffer.indexOf(lineDelimiter, startPos)) != -1) {
            const QString &line = m_stdoutBuffer.mid(startPos, endPos - startPos).trimmed();

            dbgFile << "ffmpeg stdout:" << line;
            emit sigReadLine(1,line);
            startPos = endPos + 1;
        }
        
        m_stdoutBuffer.remove(0, startPos);
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
    if (!m_processSettings.batchMode && m_progress){
        m_progress->setValue(100);
    }
    
    if (exitCode != 0) {
        m_errorMessage.remove(junkRegex);
        if (m_process->exitStatus() == QProcess::CrashExit) {
            m_errorMessage = i18n("FFMpeg Crashed") % "\n" % m_errorMessage;
        }

        emit sigFinishedWithError(m_errorMessage);
    } else {
        emit sigFinished();
    }
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

QString KisFFMpegWrapper::configuredFFMpegLocation()
{
    KisConfig cfg(true);
    return cfg.ffmpegLocation();
}

void KisFFMpegWrapper::setConfiguredFFMpegLocation(QString &location)
{
    KisConfig cfg(false);
    cfg.setFFMpegLocation(location);
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
    QString processVersion = KisFFMpegWrapper::runProcessAndReturn(processPath, QStringList() << "-version", FFMPEG_TIMEOUT);

    if (!processVersion.isEmpty()) {
        
        QRegularExpressionMatch versionMatch = ffmpegVersionRX.match(processVersion);
        
        if (versionMatch.hasMatch() && versionMatch.captured(1) == processName ) {
            ffmpegInfo["version"] = versionMatch.captured(2);
            dbgFile << "found version" << ffmpegInfo.value("version").toString();
            ffmpegInfo["enabled"] = true;
        }
        
        if (!includeProcessInfo || !ffmpegInfo["enabled"].toBool()) return ffmpegInfo;
        
        QString processCodecs = KisFFMpegWrapper::runProcessAndReturn(processPath, QStringList() << "-codecs", FFMPEG_TIMEOUT);

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
        
        if (processVersion.contains("--disable-libx264") || processVersion.contains("--disable-encoder=h264")) {
            encoderJsonObj["h264"]=false;
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
    startNonBlocking(ffprobeSettings);
    waitForFinished();
    
    QString ffprobeSTDOUT = m_processSTDOUT;
    QString ffprobeSTDERR = m_processSTDERR;
     
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
    ffmpegSettings.progressMessage = i18nc("Video information probing dialog. arg1: frame number.", "Loading video data... %1 frames examined.", "[progress]");
    ffmpegSettings.batchMode = batchMode;

    ffmpegSettings.args << "-stats"
                        << "-v" << "info" 
                        << "-progress" << "pipe:1" 
                        << "-map" << "0:v:0"
                        << "-c" << "copy"
                        << "-f" << "null" << "pipe:1"
                        << "-i" << inputFile;
                        

    this->startNonBlocking(ffmpegSettings);
    this->waitForFinished();

    
    QString ffmpegSTDOUT = m_processSTDERR + "\n" + m_processSTDOUT;
    
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
                        .append("(?:([\\d\\.]+) fps,|) (\\S+?) tbr, (.+?) tbn, (.+?) tbc")
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
                
                if (streamMatch.captured(11).toFloat() > 0) {
                    float fps = streamMatch.captured(11).toFloat();

                    ffmpegProgressJsonObj["ffmpeg_fps"] = QString::number(fps);
                    ffmpegJsonOnStreamObj["r_frame_rate"] = QString::number( fps * 10000 ).append(QString("/10000"));
                } else {
                    ffmpegProgressJsonObj["ffmpeg_fps"] = 0;
                }

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

ColorPrimaries KisFFMpegWrapper::colorPrimariesFromName(QString name)
{
    if (name == "bt709") {
        return PRIMARIES_ITU_R_BT_709_5;
    }
    if (name == "bt470m") {
        return PRIMARIES_ITU_R_BT_470_6_SYSTEM_M;
    }
    if (name == "bt470bg") {
        return PRIMARIES_ITU_R_BT_470_6_SYSTEM_B_G;
    }
    if (name == "smpte170m") {
        return PRIMARIES_ITU_R_BT_601_6;
    }
    if (name == "smpte240m") {
        return PRIMARIES_SMPTE_240M;
    }
    if (name == "film") {
        return PRIMARIES_GENERIC_FILM;
    }
    if (name == "bt2020") {
        return PRIMARIES_ITU_R_BT_2020_2_AND_2100_0;
    }
    if (name.startsWith("smpte428")) {
        return PRIMARIES_SMPTE_ST_428_1;
    }
    if (name == "smpte431") {
        return PRIMARIES_SMPTE_RP_431_2;
    }
    if (name == "smpte432") {
        return PRIMARIES_SMPTE_EG_432_1;
    }
    if (name == "jedec-p22") {
        return PRIMARIES_EBU_Tech_3213_E;
    }

    return PRIMARIES_UNSPECIFIED;
}

TransferCharacteristics KisFFMpegWrapper::transferCharacteristicsFromName(QString name)
{
    if (name == "bt709") {
        return TRC_ITU_R_BT_709_5;
    }
    if (name == "gamma22") {
        return TRC_ITU_R_BT_470_6_SYSTEM_M;
    }
    if (name == "gamma28") {
        return TRC_ITU_R_BT_470_6_SYSTEM_B_G;
    }
    if (name == "smpte170m") {
        return TRC_ITU_R_BT_601_6;
    }
    if (name == "smpte240m") {
        return TRC_SMPTE_240M;
    }
    if (name == "linear") {
        return TRC_LINEAR;
    }
    if (name == "log" || name == "log100") {
        return TRC_LOGARITHMIC_100;
    }
    if (name == "log316" || name == "log_sqrt") {
        return TRC_LOGARITHMIC_100_sqrt10;
    }
    if (name == "iec61966_2_4" || name == "iec61966-2-4") {
        return TRC_IEC_61966_2_4;
    }
    if (name.startsWith("bt1361")) {
        return TRC_ITU_R_BT_1361;
    }
    if (name == "iec61966_2_1" || name == "iec61966-2-1") {
        return TRC_IEC_61966_2_1;
    }
    if (name.startsWith("bt2020_10")) {
        return TRC_ITU_R_BT_2020_2_10bit;
    }
    if (name.startsWith("bt2020_12")) {
        return TRC_ITU_R_BT_2020_2_12bit;
    }
    if (name == "smpte2084") {
        return TRC_ITU_R_BT_2100_0_PQ;
    }
    if (name == "smpte240m") {
        return TRC_SMPTE_240M;
    }
    if (name.startsWith("smpte428")) {
        return TRC_SMPTE_ST_428_1;
    }
    if (name == "arib-std-b67") {
        return TRC_ITU_R_BT_2100_0_HLG;
    }

    return TRC_UNSPECIFIED;
}
