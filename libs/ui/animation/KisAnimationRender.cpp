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
#include <QMessageBox>

#include "krita_container_utils.h"

#include "KisVideoSaver.h"

void KisAnimationRender::render(KisDocument *doc, KisViewManager *viewManager, KisAnimationRenderingOptions encoderOptions) {
    const QString frameMimeType = encoderOptions.frameMimeType;
    const QString framesDirectory = encoderOptions.resolveAbsoluteFramesDirectory();
    const QString extension = KisMimeDatabase::suffixesForMimeType(frameMimeType).first();
    const QString baseFileName = QString("%1/%2.%3").arg(framesDirectory)
                                                    .arg(encoderOptions.basename)
                                                    .arg(extension);

    if (mustHaveEvenDimensions(encoderOptions.videoMimeType, encoderOptions.renderMode())) {
        if (hasEvenDimensions(encoderOptions.width, encoderOptions.height) != true) {
            encoderOptions.width = encoderOptions.width + (encoderOptions.width & 0x1);
            encoderOptions.height = encoderOptions.height + (encoderOptions.height & 0x1);
        }
    }

    const QSize scaledSize = doc->image()->bounds().size().scaled(encoderOptions.width, encoderOptions.height, Qt::IgnoreAspectRatio);

    if (mustHaveEvenDimensions(encoderOptions.videoMimeType, encoderOptions.renderMode())) {
        if (hasEvenDimensions(scaledSize.width(), scaledSize.height()) != true) {
            QString type = encoderOptions.videoMimeType == "video/mp4" ? "Mpeg4 (.mp4) " : "Mastroska (.mkv) ";
            qWarning() << type <<"requires width and height to be even, resize and try again!";
            doc->setErrorMessage(i18n("%1 requires width and height to be even numbers.  Please resize or crop the image before exporting.", type));
            QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Could not render animation:\n%1", doc->errorMessage()));
            return;
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

    // the folder could have been read-only or something else could happen
    if ((encoderOptions.shouldEncodeVideo || encoderOptions.wantsOnlyUniqueFrameSequence) &&
        result == KisAsyncAnimationFramesSaveDialog::RenderComplete) {

        const QString savedFilesMask = exporter.savedFilesMask();

        if (encoderOptions.shouldEncodeVideo) {
            const QString resultFile = encoderOptions.resolveAbsoluteVideoFilePath();
            KIS_SAFE_ASSERT_RECOVER_NOOP(QFileInfo(resultFile).isAbsolute());

            {
                const QFileInfo info(resultFile);
                QDir dir(info.absolutePath());

                if (!dir.exists()) {
                    dir.mkpath(info.absolutePath());
                }
                KIS_SAFE_ASSERT_RECOVER_NOOP(dir.exists());
            }

            KisImportExportErrorCode res;
            QFile fi(resultFile);
            if (!fi.open(QIODevice::WriteOnly)) {
                qWarning() << "Could not open" << fi.fileName() << "for writing!";
                res = KisImportExportErrorCannotWrite(fi.error());
            } else {
                fi.close();
            }

            QScopedPointer<KisAnimationVideoSaver> encoder(new KisAnimationVideoSaver(doc, batchMode));
            res = encoder->convert(doc, savedFilesMask, encoderOptions, batchMode);

            if (!res.isOk()) {
                QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Could not render animation:\n%1", res.errorMessage()));
            }
        }

        //File cleanup
        QDir d(framesDirectory);

        if (encoderOptions.shouldDeleteSequence) {

            QStringList sequenceFiles = d.entryList(QStringList() << encoderOptions.basename + "*." + extension, QDir::Files);
            Q_FOREACH(const QString &f, sequenceFiles) {
                d.remove(f);
            }
        } else if(encoderOptions.wantsOnlyUniqueFrameSequence) {

            const QList<int> uniques = exporter.getUniqueFrames();
            QStringList uniqueFrameNames = getNamesForFrames(encoderOptions.basename, extension, encoderOptions.sequenceStart, uniques);
            QStringList sequenceFiles = d.entryList(QStringList() << encoderOptions.basename + "*." + extension, QDir::Files);

            //Filter out unique files.
            KritaUtils::filterContainer(sequenceFiles, [uniqueFrameNames](QString &framename){
                return !uniqueFrameNames.contains(framename);
            });

            Q_FOREACH(const QString &f, sequenceFiles) {
                d.remove(f);
            }

        }

        QStringList paletteFiles = d.entryList(QStringList() << "KritaTempPalettegen_*.png", QDir::Files);

        Q_FOREACH(const QString &f, paletteFiles) {
            d.remove(f);
        }

    } else if (result == KisAsyncAnimationFramesSaveDialog::RenderTimedOut) {
        QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Rendering error"), "Animation frame rendering has timed out. Output files are incomplete.\nTry to increase \"Frame Rendering Timeout\" or reduce \"Frame Rendering Clones Limit\" in Krita settings");
    } else if (result == KisAsyncAnimationFramesSaveDialog::RenderFailed) {
        QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Rendering error"), i18n("Failed to render animation frames! Output files are incomplete."));
    }
}

QString KisAnimationRender::getNameForFrame(const QString &basename, const QString &extension, int sequenceStart, int frame)
{
    QString frameNumberText = QString("%1").arg(frame + sequenceStart, 4, 10, QChar('0'));
    return basename + frameNumberText + "." + extension;
}

QStringList KisAnimationRender::getNamesForFrames(const QString &basename, const QString &extension, int sequenceStart, const QList<int> &frames)
{
    QStringList list;
    Q_FOREACH(const int &i, frames) {
        list.append(getNameForFrame(basename, extension, sequenceStart, i));
    }
    return list;
}

bool KisAnimationRender::mustHaveEvenDimensions(const QString &mimeType, KisAnimationRenderingOptions::RenderMode renderMode)
{
    return (mimeType == "video/mp4" || mimeType == "video/x-matroska") && renderMode != KisAnimationRenderingOptions::RENDER_FRAMES_ONLY;
}

bool KisAnimationRender::hasEvenDimensions(int width, int height)
{
    return !((width & 0x1) || (height & 0x1));
}
