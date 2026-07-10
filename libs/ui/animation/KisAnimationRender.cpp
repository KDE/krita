/*
 *  SPDX-FileCopyrightText: 2020 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAnimationRender.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QApplication>

#include "KisDocument.h"
#include "KisViewManager.h"
#include "KisAnimationRenderingOptions.h"
#include "KisMimeDatabase.h"
#include "dialogs/KisAsyncAnimationFramesSaveDialog.h"
#include "kis_time_span.h"
#include "KisMainWindow.h"

#include "krita_container_utils.h"

#include "KisVideoSaver.h"

#ifdef Q_OS_ANDROID
#include <QTemporaryDir>
#include <memory>
#endif

namespace
{

bool looksLikeMp4(const QString &videoType)
{
#ifdef Q_OS_ANDROID
    return videoType.contains(QStringLiteral("mp4"));
#else
    return videoType == QStringLiteral("video/mp4");
#endif
}

bool looksLikeMatroska(const QString &videoType)
{
#ifdef Q_OS_ANDROID
    return videoType.contains(QStringLiteral("matroska"));
#else
    return videoType == QStringLiteral("video/x-matroska");
#endif
}

} // namespace

bool KisAnimationRender::render(KisDocument *doc, KisViewManager *viewManager, KisAnimationRenderingOptions encoderOptions) {
#ifdef Q_OS_ANDROID
    // The user may cancel the dialog prompting them for a video file, so bail
    // out if we don't have one here. We can't create an implicit one next to
    // the document on Android because of file system restrictions.
    if (encoderOptions.shouldEncodeVideo && encoderOptions.videoFileName.isEmpty()) {
        return false;
    }
#endif

    bool isTemporaryFramesDirectory = false;
    QString framesDirectory;
#ifdef Q_OS_ANDROID
    // Android uses weird content URIs instead of file paths and isn't allowed
    // to scribble around in the file system without asking the user for access.
    // We'll have to take the frames directory as it is given and if we don't
    // get one then we'll create a temporary directory to stick our frames into.
    std::unique_ptr<QTemporaryDir> tempDir;
    if (encoderOptions.shouldDeleteSequence || encoderOptions.directory.isEmpty()) {
        tempDir = std::make_unique<QTemporaryDir>();
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(tempDir->isValid(), false);
        framesDirectory = tempDir->path();
        isTemporaryFramesDirectory = true;
    } else {
        framesDirectory = encoderOptions.directory;
    }
#else
    framesDirectory = encoderOptions.resolveAbsoluteFramesDirectory();
#endif

    const QString frameMimeType = encoderOptions.frameMimeType;
    const QString extension = KisMimeDatabase::suffixesForMimeType(frameMimeType).first();
    const QString baseFileName = QString("%1/%2.%3").arg(framesDirectory, encoderOptions.basename, extension);

#ifdef Q_OS_ANDROID
    QString videoType = encoderOptions.videoFormatKey;
#else
    QString videoType = encoderOptions.videoMimeType;
#endif
    if (mustHaveEvenDimensions(videoType, encoderOptions.renderMode())) {
        if (hasEvenDimensions(encoderOptions.width, encoderOptions.height) != true) {
            encoderOptions.width = encoderOptions.width + (encoderOptions.width & 0x1);
            encoderOptions.height = encoderOptions.height + (encoderOptions.height & 0x1);
        }
    }

    const QSize scaledSize = doc->image()->bounds().size().scaled(encoderOptions.width, encoderOptions.height, Qt::IgnoreAspectRatio);

    if (mustHaveEvenDimensions(videoType, encoderOptions.renderMode())) {
        if (hasEvenDimensions(scaledSize.width(), scaledSize.height()) != true) {
            QString type = looksLikeMp4(videoType) ? "Mpeg4 (.mp4) " : "Matroska (.mkv) ";

            qWarning() << type <<"requires width and height to be even, resize and try again!";
            doc->setErrorMessage(i18n("%1 requires width and height to be even numbers.  Please resize or crop the image before exporting.", type));
            QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Could not render animation:\n%1", doc->errorMessage()));

            return false;
        }
    }

    const bool batchMode = false; // TODO: fetch correctly!
    KisAsyncAnimationFramesSaveDialog exporter(doc->image(),
                                               KisTimeSpan::fromTimeToTime(encoderOptions.firstFrame,
                                                                      encoderOptions.lastFrame),
                                               baseFileName,
                                               encoderOptions.sequenceStart,
                                               encoderOptions.wantsOnlyUniqueFrameSequence && !encoderOptions.shouldEncodeVideo,
                                               encoderOptions.frameExportConfig);
    exporter.setBatchMode(batchMode);

    KisAsyncAnimationFramesSaveDialog::Result result =
        exporter.regenerateRange(viewManager->mainWindow()->viewManager());

    bool delayReturnSuccess = (result == KisAsyncAnimationFramesSaveDialog::RenderComplete);

    // the folder could have been read-only or something else could happen
    if ((encoderOptions.shouldEncodeVideo || encoderOptions.wantsOnlyUniqueFrameSequence) &&
        result == KisAsyncAnimationFramesSaveDialog::RenderComplete) {

        const QString savedFilesMask = exporter.savedFilesMask();

        if (encoderOptions.shouldEncodeVideo) {
            bool videoFileWriteAllowed = true;
            // Android's weird file system doesn't work this way, the target
            // file path is going to be a sandbox URL. Making it absolute,
            // creating a directory above it or checking for its existence
            // neither make sense nor are they necessary.
#ifndef Q_OS_ANDROID
            const QString videoOutputFilePath = encoderOptions.resolveAbsoluteVideoFilePath();
            KIS_SAFE_ASSERT_RECOVER_NOOP(QFileInfo(videoOutputFilePath).isAbsolute());

            const QFileInfo videoOutputFile(videoOutputFilePath);
            QDir outputDir(videoOutputFile.absolutePath());

            if (!outputDir.exists()) {
                outputDir.mkpath(videoOutputFile.absolutePath());
            }
            KIS_SAFE_ASSERT_RECOVER_NOOP(outputDir.exists());

            // If file exists at output path, prompt user for overwrite..
            if (videoOutputFile.exists()) {
                QMessageBox videoOverwritePrompt;

                videoOverwritePrompt.setText(i18n("Overwrite existing video?"));
                videoOverwritePrompt.setInformativeText(i18n("A file already exists at the path where you want to render your video [%1]... \n\
                                                              Are you sure you want to overwrite the existing file?", videoOutputFilePath));
                videoOverwritePrompt.setStandardButtons(QMessageBox::Ok | QMessageBox::Abort);

                videoFileWriteAllowed = videoOverwritePrompt.exec() == QMessageBox::Ok ? true : false;
            }
#endif

            // Write the video..
            if (videoFileWriteAllowed) {
                KisImportExportErrorCode exportResult = ImportExportCodes::OK;

                // Let's not mess with the file on Android like this, it's slow
                // and could cause weird behavior depending on the provider.
                // We'll notice that the file can't be opened later anyway.
#ifndef Q_OS_ANDROID
                QFile videoFile(videoOutputFilePath);
                if (!videoFile.open(QIODevice::WriteOnly)) {
                    qWarning() << "Could not open" << videoFile.fileName() << "for writing! Do you have permission to write to this file?";
                    exportResult = KisImportExportErrorCannotWrite(videoFile.error());
                } else {
                    videoFile.close();
                }
#endif

                if (exportResult.isOk()) {
                    QScopedPointer<KisAnimationVideoSaver> encoder(new KisAnimationVideoSaver(doc, batchMode));
                    exportResult = encoder->convert(doc,
                                                    framesDirectory,
                                                    savedFilesMask,
                                                    exporter.savedFiles(),
                                                    encoderOptions,
                                                    batchMode);
                }

                if (!exportResult.isOk()) {
                    QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Could not render animation:\n%1", exportResult.errorMessage()));

                    delayReturnSuccess = false; // Delay return to clean up exported frames.
                }
            }
        }

        //File cleanup
        if (!isTemporaryFramesDirectory) {
            QDir d(framesDirectory);

            if (encoderOptions.shouldDeleteSequence || !delayReturnSuccess) {
                QStringList savedFiles = exporter.savedFiles();

                Q_FOREACH(const QString &f, savedFiles) {
                    if (d.exists(f)) {
                        d.remove(f);
                    }
                }
            } else if(encoderOptions.wantsOnlyUniqueFrameSequence) {
                const QStringList fileNames = exporter.savedFiles();
                const QStringList uniqueFrameNames = exporter.savedUniqueFiles();

                Q_FOREACH(const QString &f, fileNames) {
                    if (!uniqueFrameNames.contains(f)) {
                        d.remove(f);
                    }
                }
            }

            // We don't generate palette files on Android, that's done in memory.
#ifndef Q_OS_ANDROID
            QStringList paletteFiles = d.entryList(QStringList() << "KritaTempPalettegen_*.png", QDir::Files);

            Q_FOREACH(const QString &f, paletteFiles) {
                d.remove(f);
            }
#endif
        }
    } else if (result == KisAsyncAnimationFramesSaveDialog::RenderTimedOut) {
        QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Rendering error"), "Animation frame rendering has timed out. Output files are incomplete.\nTry to increase \"Frame Rendering Timeout\" or reduce \"Frame Rendering Clones Limit\" in Krita settings");
    } else if (result == KisAsyncAnimationFramesSaveDialog::RenderFailed) {
        QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Rendering error"), i18n("Failed to render animation frames! Output files are incomplete."));
    } 

    return delayReturnSuccess;
}

bool KisAnimationRender::mustHaveEvenDimensions(const QString &videoType,
                                                KisAnimationRenderingOptions::RenderMode renderMode)
{
    return renderMode != KisAnimationRenderingOptions::RENDER_FRAMES_ONLY
        && (looksLikeMp4(videoType) || looksLikeMatroska(videoType));
}

bool KisAnimationRender::hasEvenDimensions(int width, int height)
{
    return !((width & 0x1) || (height & 0x1));
}
