/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAsyncAnimationFramesSaveDialog.h"

#include <kis_image.h>
#include <kis_time_span.h>

#include <KisAsyncAnimationFramesSavingRenderer.h>
#include "kis_properties_configuration.h"

#include "KisMimeDatabase.h"

#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QApplication>

struct KisAsyncAnimationFramesSaveDialog::Private {
    Private(KisImageSP _image,
            const KisTimeSpan &_range,
            const QString &baseFilename,
            int _sequenceNumberingOffset,
            bool _onlyNeedsUniqueFrames,
            KisPropertiesConfigurationSP _exportConfiguration)
        : originalImage(_image),
          range(_range),
          onlyNeedsUniqueFrames(_onlyNeedsUniqueFrames),
          sequenceNumberingOffset(_sequenceNumberingOffset),
          exportConfiguration(_exportConfiguration)
    {
        int baseLength = baseFilename.lastIndexOf(".");
        if (baseLength > -1) {
            filenamePrefix = baseFilename.left(baseLength);
            filenameSuffix = baseFilename.right(baseFilename.length() - baseLength);
        } else {
            filenamePrefix = baseFilename;
        }

        outputMimeType = KisMimeDatabase::mimeTypeForFile(baseFilename, false).toLatin1();
    }

    KisImageSP originalImage;
    KisTimeSpan range;

    QString filenamePrefix;
    QString filenameSuffix;
    QByteArray outputMimeType;
    bool onlyNeedsUniqueFrames;

    int sequenceNumberingOffset;
    KisPropertiesConfigurationSP exportConfiguration;
};

KisAsyncAnimationFramesSaveDialog::KisAsyncAnimationFramesSaveDialog(KisImageSP originalImage,
                                                                     const KisTimeSpan &range,
                                                                     const QString &baseFilename,
                                                                     int sequenceNumberingOffset,
                                                                     bool onlyNeedsUniqeFrames,
                                                                     KisPropertiesConfigurationSP exportConfiguration)
    : KisAsyncAnimationRenderDialogBase(i18n("Saving frames..."), originalImage, 0),
      m_d(new Private(originalImage, range, baseFilename, sequenceNumberingOffset, onlyNeedsUniqeFrames, exportConfiguration))
{


}

KisAsyncAnimationFramesSaveDialog::~KisAsyncAnimationFramesSaveDialog()
{
}

KisAsyncAnimationRenderDialogBase::Result KisAsyncAnimationFramesSaveDialog::regenerateRange(KisViewManager *viewManager)
{
    QFileInfo info(savedFilesMaskWildcard());

    QDir dir(info.absolutePath());

    if (!dir.exists()) {
        dir.mkpath(info.absolutePath());
    }
    KIS_SAFE_ASSERT_RECOVER_NOOP(dir.exists());

    QStringList filesList = dir.entryList({ info.fileName() });

    if (!filesList.isEmpty()) {
        if (batchMode()) {
            return RenderFailed;
        }

        QStringList filesWithinRange;
        const int numberOfDigits = 4;
        Q_FOREACH(const QString &filename, filesList) {
            // Counting based on suffix, since prefix may include the path while filename doesn't
            int digitsPosition = filename.length() - m_d->filenameSuffix.length() - numberOfDigits;
            int fileNumber = filename.midRef(digitsPosition, numberOfDigits).toInt();
            auto frameNumber = fileNumber - m_d->sequenceNumberingOffset;
            if (m_d->range.contains(frameNumber)) {
                filesWithinRange.append(filename);
            }
        }
        filesList = filesWithinRange;

        QStringList truncatedList = filesList;

        while (truncatedList.size() > 3) {
            truncatedList.takeLast();
        }

        QString exampleFiles = truncatedList.join(", ");
        if (truncatedList.size() != filesList.size()) {
            exampleFiles += QString(", ...");
        }

        QMessageBox::StandardButton result =
                QMessageBox::warning(qApp->activeWindow(),
                                     i18n("Delete old frames?"),
                                     i18n("Frames with the same naming "
                                          "scheme exist in the destination "
                                          "directory. They are going to be "
                                          "deleted, continue?\n\n"
                                          "Directory: %1\n"
                                          "Files: %2",
                                          info.absolutePath(), exampleFiles),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);

        if (result == QMessageBox::Yes) {
            Q_FOREACH (const QString &file, filesList) {
                if (!dir.remove(file)) {
                    QMessageBox::critical(qApp->activeWindow(),
                                          i18n("Failed to delete"),
                                          i18n("Failed to delete an old frame file:\n\n"
                                               "%1\n\n"
                                               "Rendering cancelled.", dir.absoluteFilePath(file)));

                    return RenderFailed;
                }
            }
        } else {
            return RenderCancelled;
        }
    }

    return KisAsyncAnimationRenderDialogBase::regenerateRange(viewManager);
}

QList<int> KisAsyncAnimationFramesSaveDialog::calcDirtyFrames() const
{
    QList<int> result;
    for (int frame = m_d->range.start(); frame <= m_d->range.end(); frame++) {
        KisTimeSpan heldFrameTimeRange = KisTimeSpan::calculateIdenticalFramesRecursive(m_d->originalImage->root(), frame);

        if (!m_d->onlyNeedsUniqueFrames) {
            // Clamp holds that begin before the rendered range onto it
            heldFrameTimeRange &= m_d->range;
        }

        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(heldFrameTimeRange.isValid(), result);

        result.append(heldFrameTimeRange.start());

        if (heldFrameTimeRange.isInfinite()) {
            break;
        } else {
            frame = heldFrameTimeRange.end();
        }
    }
    return result;
}

KisAsyncAnimationRendererBase *KisAsyncAnimationFramesSaveDialog::createRenderer(KisImageSP image)
{
    return new KisAsyncAnimationFramesSavingRenderer(image,
                                                     m_d->filenamePrefix,
                                                     m_d->filenameSuffix,
                                                     m_d->outputMimeType,
                                                     m_d->range,
                                                     m_d->sequenceNumberingOffset,
                                                     m_d->onlyNeedsUniqueFrames,
                                                     m_d->exportConfiguration);
}

void KisAsyncAnimationFramesSaveDialog::initializeRendererForFrame(KisAsyncAnimationRendererBase *renderer, KisImageSP image, int frame)
{
    Q_UNUSED(renderer);
    Q_UNUSED(image);
    Q_UNUSED(frame);
}

QString KisAsyncAnimationFramesSaveDialog::savedFilesMask() const
{
    return m_d->filenamePrefix + "%04d" + m_d->filenameSuffix;
}

QString KisAsyncAnimationFramesSaveDialog::savedFilesMaskWildcard() const
{
    return m_d->filenamePrefix + "????" + m_d->filenameSuffix;
}

QList<int> KisAsyncAnimationFramesSaveDialog::getUniqueFrames() const
{
    return this->calcDirtyFrames();
}
