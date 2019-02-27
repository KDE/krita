/*
 * Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#include "AnimationRenderer.h"

#include <QMessageBox>

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <kis_image.h>
#include <KisViewManager.h>
#include <KoUpdater.h>
#include <kis_node_manager.h>
#include <kis_image_manager.h>
#include <kis_action.h>
#include <kis_image_animation_interface.h>
#include <kis_properties_configuration.h>
#include <kis_config.h>
#include <KisDocument.h>
#include <KisMimeDatabase.h>
#include <kis_time_range.h>
#include <KisImportExportManager.h>

#include "DlgAnimationRenderer.h"
#include <dialogs/KisAsyncAnimationFramesSaveDialog.h>

#include "video_saver.h"
#include "KisAnimationRenderingOptions.h"

K_PLUGIN_FACTORY_WITH_JSON(AnimaterionRendererFactory, "kritaanimationrenderer.json", registerPlugin<AnimaterionRenderer>();)

AnimaterionRenderer::AnimaterionRenderer(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
{
    // Shows the big dialog
    KisAction *action = createAction("render_animation");
    action->setActivationFlags(KisAction::IMAGE_HAS_ANIMATION);
    connect(action,  SIGNAL(triggered()), this, SLOT(slotRenderAnimation()));

    // Re-renders the image sequence as defined in the last render
    action = createAction("render_animation_again");
    action->setActivationFlags(KisAction::IMAGE_HAS_ANIMATION);
    connect(action,  SIGNAL(triggered()), this, SLOT(slotRenderSequenceAgain()));
}

AnimaterionRenderer::~AnimaterionRenderer()
{
}

void AnimaterionRenderer::slotRenderAnimation()
{
    KisImageWSP image = viewManager()->image();

    if (!image) return;
    if (!image->animationInterface()->hasAnimation()) return;

    KisDocument *doc = viewManager()->document();

    DlgAnimationRenderer dlgAnimationRenderer(doc, viewManager()->mainWindow());

    dlgAnimationRenderer.setCaption(i18n("Render Animation"));

    if (dlgAnimationRenderer.exec() == QDialog::Accepted) {
        KisAnimationRenderingOptions encoderOptions = dlgAnimationRenderer.getEncoderOptions();
        renderAnimationImpl(doc, encoderOptions);
    }
}

void AnimaterionRenderer::slotRenderSequenceAgain()
{
    KisImageWSP image = viewManager()->image();

    if (!image) return;
    if (!image->animationInterface()->hasAnimation()) return;

    KisDocument *doc = viewManager()->document();

    KisConfig cfg(true);

    KisPropertiesConfigurationSP settings = cfg.exportConfiguration("ANIMATION_EXPORT");

    KisAnimationRenderingOptions encoderOptions;
    encoderOptions.fromProperties(settings);

    renderAnimationImpl(doc, encoderOptions);
}

void AnimaterionRenderer::renderAnimationImpl(KisDocument *doc, KisAnimationRenderingOptions encoderOptions)
{
    const QString frameMimeType = encoderOptions.frameMimeType;
    const QString framesDirectory = encoderOptions.resolveAbsoluteFramesDirectory();
    const QString extension = KisMimeDatabase::suffixesForMimeType(frameMimeType).first();
    const QString baseFileName = QString("%1/%2.%3").arg(framesDirectory)
            .arg(encoderOptions.basename)
            .arg(extension);


    /**
     * The dialog should ensure that the size of the video is even
     */
    KIS_SAFE_ASSERT_RECOVER(
        !((encoderOptions.width & 0x1 || encoderOptions.height & 0x1)
          && (encoderOptions.videoMimeType == "video/mp4" ||
              encoderOptions.videoMimeType == "video/x-matroska"))) {

        encoderOptions.width = encoderOptions.width + (encoderOptions.width & 0x1);
        encoderOptions.height = encoderOptions.height + (encoderOptions.height & 0x1);
    }

    const QSize scaledSize =
        doc->image()->bounds().size().scaled(
            encoderOptions.width, encoderOptions.height,
            Qt::KeepAspectRatio);

    if ((scaledSize.width() & 0x1 || scaledSize.height() & 0x1)
            && (encoderOptions.videoMimeType == "video/mp4" ||
                encoderOptions.videoMimeType == "video/x-matroska")) {
        QString m = "Mastroska (.mkv)";
        if (encoderOptions.videoMimeType == "video/mp4") {
            m = "Mpeg4 (.mp4)";
        }
        qWarning() << m <<"requires width and height to be even, resize and try again!";
        doc->setErrorMessage(i18n("%1 requires width and height to be even numbers.  Please resize or crop the image before exporting.", m));
        QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not render animation:\n%1", doc->errorMessage()));
        return;
    }

    const bool batchMode = false; // TODO: fetch correctly!
    KisAsyncAnimationFramesSaveDialog exporter(doc->image(),
                                               KisTimeRange::fromTime(encoderOptions.firstFrame,
                                                                      encoderOptions.lastFrame),
                                               baseFileName,
                                               encoderOptions.sequenceStart,
                                               encoderOptions.frameExportConfig);
    exporter.setBatchMode(batchMode);


    KisAsyncAnimationFramesSaveDialog::Result result =
        exporter.regenerateRange(viewManager()->mainWindow()->viewManager());

    // the folder could have been read-only or something else could happen
    if (encoderOptions.shouldEncodeVideo &&
        result == KisAsyncAnimationFramesSaveDialog::RenderComplete) {

        const QString savedFilesMask = exporter.savedFilesMask();

        const QString resultFile = encoderOptions.resolveAbsoluteVideoFilePath();
        KIS_SAFE_ASSERT_RECOVER_NOOP(QFileInfo(resultFile).isAbsolute())

        {
            const QFileInfo info(resultFile);
            QDir dir(info.absolutePath());

            if (!dir.exists()) {
                dir.mkpath(info.absolutePath());
            }
            KIS_SAFE_ASSERT_RECOVER_NOOP(dir.exists());
        }

        KisImportExportFilter::ConversionStatus res;
        QFile fi(resultFile);
        if (!fi.open(QIODevice::WriteOnly)) {
            qWarning() << "Could not open" << fi.fileName() << "for writing!";
            doc->setErrorMessage(i18n("Could not open %1 for writing!", fi.fileName()));
            res = KisImportExportFilter::CreationError;
        } else {
            fi.close();
        }

        QScopedPointer<VideoSaver> encoder(new VideoSaver(doc, batchMode));
        res = encoder->convert(doc, savedFilesMask, encoderOptions, batchMode);

        if (res != KisImportExportFilter::OK) {
            QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not render animation:\n%1", doc->errorMessage()));
        }

        if (encoderOptions.shouldDeleteSequence) {
            QDir d(framesDirectory);
            QStringList sequenceFiles = d.entryList(QStringList() << encoderOptions.basename + "*." + extension, QDir::Files);
            Q_FOREACH(const QString &f, sequenceFiles) {
                d.remove(f);
            }
        }

    } else if (result == KisAsyncAnimationFramesSaveDialog::RenderFailed) {
        viewManager()->mainWindow()->viewManager()->showFloatingMessage(i18n("Failed to render animation frames!"), QIcon());
    }
}

#include "AnimationRenderer.moc"
