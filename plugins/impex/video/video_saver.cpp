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

#include <kis_image.h>
#include <kis_image_animation_interface.h>
#include <kis_time_range.h>

#include "kis_animation_exporter.h"

#include <QFileSystemWatcher>
#include <QProcess>
#include <QProgressDialog>
#include <QEventLoop>
#include <QTemporaryFile>
#include <QTemporaryDir>

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
        //qDebug() << "=== progress changed ===";

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
            //qDebug() << var;
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
    KisFFMpegRunner()
        : m_cancelled(false) {}
public:
    KisImageBuilder_Result runFFMpeg(const QStringList &specialArgs,
                                     const QString &actionName,
                                     const QString &logPath,
                                     int totalFrames) {

        QTemporaryFile progressFile("KritaFFmpegProgress.XXXXXX");
        progressFile.open();

        m_process.setStandardOutputFile(logPath);
        m_process.setProcessChannelMode(QProcess::MergedChannels);
        QStringList args;
        args << "-v" << "debug"
             << "-nostdin"
             << specialArgs;

        m_cancelled = false;
        m_process.start("ffmpeg", args);
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
                                                int totalFrames) {

        KisFFMpegProgressWatcher watcher(progressFile, totalFrames);

        QProgressDialog progress(message, "", 0, 0, KisPart::instance()->currentMainwindow());
        progress.setWindowModality(Qt::ApplicationModal);
        progress.setCancelButton(0);
        progress.setMinimumDuration(0);
        progress.setValue(0);
        progress.setRange(0,100);

        QEventLoop loop;
        loop.connect(&watcher, SIGNAL(sigProcessingFinished()), SLOT(quit()));
        loop.connect(&ffmpegProcess, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(quit()));
        loop.connect(&watcher, SIGNAL(sigProgressChanged(int)), &progress, SLOT(setValue(int)));
        loop.exec();

        // wait for some errorneous case
        ffmpegProcess.waitForFinished(5000);

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
};


VideoSaver::VideoSaver(KisDocument *doc, bool batchMode)
    : m_image(doc->image())
    , m_doc(doc)
    , m_batchMode(batchMode)
    , m_runner(new KisFFMpegRunner)
{
}

VideoSaver::~VideoSaver()
{
}

KisImageSP VideoSaver::image()
{
    return m_image;
}

KisImageBuilder_Result VideoSaver::encode(const QString &filename, const QStringList &additionalOptionsList)
{
    KisImageBuilder_Result retval= KisImageBuilder_RESULT_OK;

    KisImageAnimationInterface *animation = m_image->animationInterface();
    const KisTimeRange clipRange = animation->fullClipRange();
    const int frameRate = animation->framerate();

    const QString resultFile = filename;
    const bool removeGeneratedFiles =
        !qEnvironmentVariableIsSet("KRITA_KEEP_FRAMES");

    const QFileInfo info(resultFile);
    const QString suffix = info.suffix().toLower();
    const QString baseDirectory = info.absolutePath();
    const QString frameDirectoryTemplate = baseDirectory + QDir::separator() + "frames.XXXXXX";

    QTemporaryDir framesDirImpl(frameDirectoryTemplate);
    framesDirImpl.setAutoRemove(removeGeneratedFiles);

    const QDir framesDir(framesDirImpl.path());

    const QString framesPath = framesDir.path();
    const QString framesBasePath = framesDir.filePath("frame.png");
    const QString palettePath = framesDir.filePath("palette.png");

    KisAnimationExportSaver saver(m_doc, framesBasePath, clipRange.start(), clipRange.end());

    KisImportExportFilter::ConversionStatus status =
        saver.exportAnimation();

    if (status == KisImportExportFilter::UserCancelled) {
        return  KisImageBuilder_RESULT_CANCEL;
    } else if (status != KisImportExportFilter::OK) {
        return  KisImageBuilder_RESULT_FAILURE;
    }

    if (suffix == "gif") {
        {
            QStringList args;
            args << "-r" << QString::number(frameRate)
                 << "-i" << saver.savedFilesMask()
                 << "-vf" << "palettegen"
                 << "-y" << palettePath;

            KisImageBuilder_Result result =
                m_runner->runFFMpeg(args, i18n("Fetching palette..."),
                                    framesDir.filePath("log_palettegen.log"),
                                    clipRange.duration());

            if (result) {
                return result;
            }
        }

        {
            QStringList args;
            args << "-r" << QString::number(frameRate)
                 << "-i" << saver.savedFilesMask()
                 << "-i" << palettePath
                 << "-lavfi" << "[0:v][1:v] paletteuse"
                 << additionalOptionsList
                 << "-y" << resultFile;

            KisImageBuilder_Result result =
                m_runner->runFFMpeg(args, i18n("Encoding frames..."),
                                    framesDir.filePath("log_paletteuse.log"),
                                    clipRange.duration());

            if (result) {
                return result;
            }
        }
    } else {
        QStringList args;
        args << "-r" << QString::number(frameRate)
             << "-i" << saver.savedFilesMask()
             << additionalOptionsList
             << "-y" << resultFile;

        KisImageBuilder_Result result =
            m_runner->runFFMpeg(args, i18n("Encoding frames..."),
                                framesDir.filePath("log_encode.log"),
                                clipRange.duration());

        if (result) {
            return result;
        }
    }

    return retval;
}

void VideoSaver::cancel()
{
    m_runner->cancel();
}

#include "video_saver.moc"
