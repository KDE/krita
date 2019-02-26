/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "video_saver.h"

#include <QDebug>

#include <QFileInfo>

#include <KisDocument.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoResourcePaths.h>


#include <kis_image.h>
#include <kis_image_animation_interface.h>
#include <kis_time_range.h>

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
    KisImageBuilder_Result runFFMpeg(const QStringList &specialArgs,
                                     const QString &actionName,
                                     const QString &logPath,
                                     int totalFrames)
    {
        dbgFile << "runFFMpeg: specialArgs" << specialArgs
                << "actionName" << actionName
                << "logPath" << logPath
                << "totalFrames" << totalFrames;

        QTemporaryFile progressFile(QDir::tempPath() + QDir::separator() + "KritaFFmpegProgress.XXXXXX");
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
    KisImageBuilder_Result waitForFFMpegProcess(const QString &message,
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

        KisImageBuilder_Result retval = KisImageBuilder_RESULT_OK;

        if (ffmpegProcess.state() != QProcess::NotRunning) {
            // sorry...
            ffmpegProcess.kill();
            retval = KisImageBuilder_RESULT_FAILURE;
        } else if (m_cancelled) {
            retval = KisImageBuilder_RESULT_CANCEL;
        } else if (ffmpegProcess.exitCode()) {
            retval = KisImageBuilder_RESULT_FAILURE;
        }

        return retval;
    }

private:
    QProcess m_process;
    bool m_cancelled;
    QString m_ffmpegPath;
};


VideoSaver::VideoSaver(KisDocument *doc, bool batchMode)
    : m_image(doc->image())
    , m_doc(doc)
    , m_batchMode(batchMode)
{
}

VideoSaver::~VideoSaver()
{
}

KisImageSP VideoSaver::image()
{
    return m_image;
}

KisImageBuilder_Result VideoSaver::encode(const QString &savedFilesMask, const KisAnimationRenderingOptions &options)
{
    if (!QFileInfo(options.ffmpegPath).exists()) {
        m_doc->setErrorMessage(i18n("ffmpeg could not be found at %1", options.ffmpegPath));
        return KisImageBuilder_RESULT_FAILURE;
    }

    KisImageBuilder_Result result = KisImageBuilder_RESULT_OK;

    KisImageAnimationInterface *animation = m_image->animationInterface();

    const int sequenceNumberingOffset = options.sequenceStart;
    const KisTimeRange clipRange(sequenceNumberingOffset + options.firstFrame,
                                 sequenceNumberingOffset + options.lastFrame);

     // export dimensions could be off a little bit, so the last force option tweaks the pixels for the export to work
    const QString exportDimensions =
        QString("scale=w=")
            .append(QString::number(options.width))
            .append(":h=")
            .append(QString::number(options.height))
            .append(":force_original_aspect_ratio=decrease");


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

            KisImageBuilder_Result result =
                runner->runFFMpeg(args, i18n("Fetching palette..."),
                                    videoDir.filePath("log_generate_palette_gif.log"),
                                    clipRange.duration());

            if (result != KisImageBuilder_RESULT_OK) {
                return result;
            }
        }

        {
            QStringList args;
            args << "-r" << QString::number(options.frameRate)
                 << "-start_number" << QString::number(clipRange.start())
                 << "-i" << savedFilesMask
                 << "-i" << palettePath
                 << "-lavfi" << "[0:v][1:v] paletteuse"
                 << additionalOptionsList
                 << "-y" << resultFile;

            // if we are exporting out at a different image size, we apply scaling filter
            if (m_image->width() != options.width || m_image->height() != options.height) {
                args << "-vf" << exportDimensions;
            }


            dbgFile << "savedFilesMask" << savedFilesMask << "start" << QString::number(clipRange.start()) << "duration" << clipRange.duration();

            KisImageBuilder_Result result =
                runner->runFFMpeg(args, i18n("Encoding frames..."),
                                    videoDir.filePath("log_encode_gif.log"),
                                    clipRange.duration());

            if (result != KisImageBuilder_RESULT_OK) {
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

        args << additionalOptionsList
             << "-y" << resultFile;


        result = runner->runFFMpeg(args, i18n("Encoding frames..."),
                                     videoDir.filePath("log_encode.log"),
                                     clipRange.duration());
    }

    return result;
}

KisImportExportFilter::ConversionStatus VideoSaver::convert(KisDocument *document, const QString &savedFilesMask, const KisAnimationRenderingOptions &options, bool batchMode)
{
    VideoSaver videoSaver(document, batchMode);
    KisImageBuilder_Result res = videoSaver.encode(savedFilesMask, options);

    if (res == KisImageBuilder_RESULT_OK) {
        return KisImportExportFilter::OK;

    } else if (res == KisImageBuilder_RESULT_CANCEL) {
        return KisImportExportFilter::ProgressCancelled;

    }else {
        document->setErrorMessage(i18n("FFMpeg failed to convert the image sequence. Check the logfile in your output directory for more information."));
    }

    return KisImportExportFilter::InternalError;
}

#include "video_saver.moc"
