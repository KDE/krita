/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisVideoSaver.h"

#include <QDebug>

#include <QFileInfo>

#include <KisDocument.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoResourcePaths.h>


#include <kis_image.h>
#include <kis_image_animation_interface.h>
#include <kis_time_span.h>

#include "kis_config.h"

#include "KisAnimationRenderingOptions.h"
#include <QFileSystemWatcher>
#include <QProcess>
#include <QProgressDialog>
#include <QEventLoop>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QTime>

#include "KisPart.h"

class KisFFMpegProgressWatcher : public QObject {
    Q_OBJECT
public:
    KisFFMpegProgressWatcher(QFile &progressFile, int totalFrames)
        : m_progressFile(progressFile),
          m_totalFrames(totalFrames)
        {
            connect(&m_progressWatcher, SIGNAL(fileChanged(QString)), SLOT(slotFileChanged()));


            m_progressWatcher.addPath(m_progressFile.fileName());
        }

private Q_SLOTS:
    void slotFileChanged() {
        int currentFrame = -1;
        bool isEnded = false;

        while(!m_progressFile.atEnd()) {
            QString line = QString(m_progressFile.readLine()).remove(QChar('\n'));
            QStringList var = line.split("=");

            if (var[0] == "frame") {
                currentFrame = var[1].toInt();
            } else if (var[0] == "progress") {
                isEnded = var[1] == "end";
            }
        }

        if (isEnded) {
            emit sigProgressChanged(100);
            emit sigProcessingFinished();
        } else {

            emit sigProgressChanged(100 * currentFrame / m_totalFrames);
        }
    }

Q_SIGNALS:
    void sigProgressChanged(int percent);
    void sigProcessingFinished();

private:
    QFileSystemWatcher m_progressWatcher;
    QFile &m_progressFile;
    int m_totalFrames;
};


class KisFFMpegRunner
{
public:
    KisFFMpegRunner(const QString &ffmpegPath)
        : m_cancelled(false),
          m_ffmpegPath(ffmpegPath) {}
public:
    KisImportExportErrorCode runFFMpeg(const QStringList &specialArgs,
                                     const QString &actionName,
                                     const QString &logPath,
                                     int totalFrames)
    {
        dbgFile << "runFFMpeg: specialArgs" << specialArgs
                << "actionName" << actionName
                << "logPath" << logPath
                << "totalFrames" << totalFrames;

        QTemporaryFile progressFile(QDir::tempPath() + '/' + "KritaFFmpegProgress.XXXXXX");
        progressFile.open();

        m_process.setStandardOutputFile(logPath);
        m_process.setProcessChannelMode(QProcess::MergedChannels);
        QStringList args;
        args << "-v" << "debug"
             << "-nostdin"
             << "-progress" << progressFile.fileName()
             << specialArgs;

        qDebug() << "\t" << m_ffmpegPath << args.join(" ");

        m_cancelled = false;
        m_process.start(m_ffmpegPath, args);
        return waitForFFMpegProcess(actionName, progressFile, m_process, totalFrames);
    }

    void cancel() {
        m_cancelled = true;
        m_process.kill();
    }

private:
    KisImportExportErrorCode waitForFFMpegProcess(const QString &message,
                                                QFile &progressFile,
                                                QProcess &ffmpegProcess,
                                                int totalFrames)
    {

        KisFFMpegProgressWatcher watcher(progressFile, totalFrames);

        QProgressDialog progress(message, "", 0, 0, KisPart::instance()->currentMainwindow());
        progress.setWindowModality(Qt::ApplicationModal);
        progress.setCancelButton(0);
        progress.setMinimumDuration(0);
        progress.setValue(0);
        progress.setRange(0, 100);

        QEventLoop loop;
        loop.connect(&watcher, SIGNAL(sigProcessingFinished()), SLOT(quit()));
        loop.connect(&ffmpegProcess, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(quit()));
        loop.connect(&ffmpegProcess, SIGNAL(error(QProcess::ProcessError)), SLOT(quit()));
        loop.connect(&watcher, SIGNAL(sigProgressChanged(int)), &progress, SLOT(setValue(int)));

        if (ffmpegProcess.state() != QProcess::NotRunning) {
            loop.exec();

            // wait for some erroneous case
            ffmpegProcess.waitForFinished(5000);
        }

        KisImportExportErrorCode retval = ImportExportCodes::OK;

        if (ffmpegProcess.state() != QProcess::NotRunning) {
            // sorry...
            ffmpegProcess.kill();
            retval = ImportExportCodes::Failure;
        } else if (m_cancelled) {
            retval = ImportExportCodes::Cancelled;
        } else if (ffmpegProcess.exitCode()) {
            retval = ImportExportCodes::Failure;
        }

        return retval;
    }

private:
    QProcess m_process;
    bool m_cancelled;
    QString m_ffmpegPath;
};


KisVideoSaver::KisVideoSaver(KisDocument *doc, bool batchMode)
    : m_image(doc->image())
    , m_doc(doc)
    , m_batchMode(batchMode)
{
}

KisVideoSaver::~KisVideoSaver()
{
}

KisImageSP KisVideoSaver::image()
{
    return m_image;
}

KisImportExportErrorCode KisVideoSaver::encode(const QString &savedFilesMask, const KisAnimationRenderingOptions &options)
{
    if (!QFileInfo(options.ffmpegPath).exists()) {
        m_doc->setErrorMessage(i18n("ffmpeg could not be found at %1", options.ffmpegPath));
        return ImportExportCodes::Failure;
    }

    KisImportExportErrorCode resultOuter = ImportExportCodes::OK;

    KisImageAnimationInterface *animation = m_image->animationInterface();

    const int sequenceNumberingOffset = options.sequenceStart;
    const KisTimeSpan clipRange = KisTimeSpan::fromTimeToTime(sequenceNumberingOffset + options.firstFrame,
                                                        sequenceNumberingOffset + options.lastFrame);

     // export dimensions could be off a little bit, so the last force option tweaks the pixels for the export to work
    const QString exportDimensions =
        QString("scale=w=")
            .append(QString::number(options.width))
            .append(":h=")
            .append(QString::number(options.height));
            //.append(":force_original_aspect_ratio=decrease"); HOTFIX for even:odd dimension images.

    const QString resultFile = options.resolveAbsoluteVideoFilePath();
    const QDir videoDir(QFileInfo(resultFile).absolutePath());

    const QFileInfo info(resultFile);
    const QString suffix = info.suffix().toLower();
    const QString palettePath = videoDir.filePath("palette.png");
    const QStringList additionalOptionsList = options.customFFMpegOptions.split(' ', QString::SkipEmptyParts);
    QScopedPointer<KisFFMpegRunner> runner(new KisFFMpegRunner(options.ffmpegPath));

    if (suffix == "gif") {
        {
            QStringList args;
            args << "-r" << QString::number(options.frameRate)
                 << "-start_number" << QString::number(clipRange.start())
                 << "-i" << savedFilesMask
                 << "-vf" << "palettegen"
                 << "-y" << palettePath;

            KisImportExportErrorCode result =
                runner->runFFMpeg(args, i18n("Fetching palette..."),
                                    videoDir.filePath("log_generate_palette_gif.log"),
                                    clipRange.duration());

            if (!result.isOk()) {
                return result;
            }
        }

        {
            QStringList args;
            args << "-r" << QString::number(options.frameRate)
                 << "-start_number" << QString::number(clipRange.start())
                 << "-i" << savedFilesMask
                 << "-i" << palettePath
                 << "-lavfi";

            QString filterArgs;

            // if we are exporting out at a different image size, we apply scaling filter
            if (m_image->width() != options.width || m_image->height() != options.height) {
                filterArgs.append(exportDimensions + "[0:v];");
            }

            args << filterArgs.append("[0:v][1:v] paletteuse")
                 << "-y" << resultFile;


            dbgFile << "savedFilesMask" << savedFilesMask << "start" << QString::number(clipRange.start()) << "duration" << clipRange.duration();

            KisImportExportErrorCode result =
                runner->runFFMpeg(args, i18n("Encoding frames..."),
                                    videoDir.filePath("log_encode_gif.log"),
                                    clipRange.duration());

            if (!result.isOk()) {
                return result;
            }
        }
    } else {
        QStringList args;
        args << "-r" << QString::number(options.frameRate)
             << "-start_number" << QString::number(clipRange.start())
             << "-i" << savedFilesMask;

        QFileInfo audioFileInfo = animation->audioChannelFileName();
        if (options.includeAudio && audioFileInfo.exists()) {
            const int msecStart = clipRange.start() * 1000 / animation->framerate();
            const int msecDuration = clipRange.duration() * 1000 / animation->framerate();

            const QTime startTime = QTime::fromMSecsSinceStartOfDay(msecStart);
            const QTime durationTime = QTime::fromMSecsSinceStartOfDay(msecDuration);
            const QString ffmpegTimeFormat("H:m:s.zzz");

            args << "-ss" << startTime.toString(ffmpegTimeFormat);
            args << "-t" << durationTime.toString(ffmpegTimeFormat);

            args << "-i" << audioFileInfo.absoluteFilePath();
        }

        // if we are exporting out at a different image size, we apply scaling filter
        // export options HAVE to go after input options, so make sure this is after the audio import
        if (m_image->width() != options.width || m_image->height() != options.height) {
            args << "-vf" << exportDimensions;
        }

        args << additionalOptionsList;

        args << "-y" << resultFile;

        resultOuter = runner->runFFMpeg(args, i18n("Encoding frames..."),
                                     videoDir.filePath("log_encode.log"),
                                     clipRange.duration());
    }

    return resultOuter;
}

KisImportExportErrorCode KisVideoSaver::convert(KisDocument *document, const QString &savedFilesMask, const KisAnimationRenderingOptions &options, bool batchMode)
{
    KisVideoSaver videoSaver(document, batchMode);
    KisImportExportErrorCode res = videoSaver.encode(savedFilesMask, options);
    return res;
}

#include "KisVideoSaver.moc"
