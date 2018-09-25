/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KisAsyncAnimationFramesSaveDialog.h"

#include <kis_image.h>
#include <kis_time_range.h>

#include <KisAsyncAnimationFramesSavingRenderer.h>
#include "kis_properties_configuration.h"

#include "KisMimeDatabase.h"

#include <QFileInfo>
#include <QDir>
#include <QMessageBox>

struct KisAsyncAnimationFramesSaveDialog::Private {
    Private(KisImageSP _image,
            const KisTimeRange &_range,
            const QString &baseFilename,
            int _sequenceNumberingOffset,
            KisPropertiesConfigurationSP _exportConfiguration)
        : originalImage(_image),
          range(_range),
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
    KisTimeRange range;

    QString filenamePrefix;
    QString filenameSuffix;
    QByteArray outputMimeType;

    int sequenceNumberingOffset;
    KisPropertiesConfigurationSP exportConfiguration;
};

KisAsyncAnimationFramesSaveDialog::KisAsyncAnimationFramesSaveDialog(KisImageSP originalImage,
                                                                     const KisTimeRange &range,
                                                                     const QString &baseFilename,
                                                                     int sequenceNumberingOffset,
                                                                     KisPropertiesConfigurationSP exportConfiguration)
    : KisAsyncAnimationRenderDialogBase("Saving frames...", originalImage, 0),
      m_d(new Private(originalImage, range, baseFilename, sequenceNumberingOffset, exportConfiguration))
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
                QMessageBox::warning(0,
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
                    QMessageBox::critical(0,
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
    // TODO: optimize!
    QList<int> result;
    for (int i = m_d->range.start(); i <= m_d->range.end(); i++) {
        result.append(i);
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
