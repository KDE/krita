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


K_PLUGIN_FACTORY_WITH_JSON(AnimaterionRendererFactory, "kritaanimationrenderer.json", registerPlugin<AnimaterionRenderer>();)

AnimaterionRenderer::AnimaterionRenderer(QObject *parent, const QVariantList &)
    : KisActionPlugin(parent)
{
    // Shows the big dialog
    KisAction *action = createAction("render_animation");
    action->setActivationFlags(KisAction::IMAGE_HAS_ANIMATION);
    connect(action,  SIGNAL(triggered()), this, SLOT(slotRenderAnimation()));

    // Re-renders the image sequence as defined in the last render
    action = createAction("render_image_sequence_again");
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

    KisConfig kisConfig(true);
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->fromXML(kisConfig.exportConfiguration("IMAGESEQUENCE"));
    dlgAnimationRenderer.setSequenceConfiguration(cfg);

    cfg->clearProperties();
    cfg->fromXML(kisConfig.exportConfiguration("ANIMATION_RENDERER"));
    dlgAnimationRenderer.setVideoConfiguration(cfg);

    cfg->clearProperties();
    cfg->fromXML(kisConfig.exportConfiguration("FFMPEG_CONFIG"));
    dlgAnimationRenderer.setEncoderConfiguration(cfg);

    // update the UI to show the selected export options
    dlgAnimationRenderer.updateExportUIOptions();


    if (dlgAnimationRenderer.exec() == QDialog::Accepted) {
        KisPropertiesConfigurationSP sequenceConfig = dlgAnimationRenderer.getSequenceConfiguration();
        kisConfig.setExportConfiguration("IMAGESEQUENCE", sequenceConfig);
        QString mimetype = sequenceConfig->getString("mimetype");
        QString extension = KisMimeDatabase::suffixesForMimeType(mimetype).first();
        QString baseFileName = QString("%1/%2.%3").arg(sequenceConfig->getString("directory"))
                .arg(sequenceConfig->getString("basename"))
                .arg(extension);

        /**
         * Check if the dimensions make sense before we even try to batch save.
         */
        KisPropertiesConfigurationSP encoderConfig = dlgAnimationRenderer.getEncoderConfiguration();
        if (encoderConfig) {
            if ((image->height()%2 || image->width()%2)
                    && (encoderConfig->getString("mimetype") == "video/mp4" ||
                        encoderConfig->getString("mimetype") == "video/x-matroska")) {
                QString m = "Mastroska (.mkv)";
                if (encoderConfig->getString("mimetype")== "video/mp4") {
                    m = "Mpeg4 (.mp4)";
                }
                qWarning() << m <<"requires width and height to be even, resize and try again!";
                doc->setErrorMessage(i18n("%1 requires width and height to be even numbers.  Please resize or crop the image before exporting.", m));
                QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not render animation:\n%1", doc->errorMessage()));
                return;
            }
        }

        const bool batchMode = false; // TODO: fetch correctly!

        KisAsyncAnimationFramesSaveDialog exporter(doc->image(),
                                                   KisTimeRange::fromTime(sequenceConfig->getInt("first_frame"), sequenceConfig->getInt("last_frame")),
                                                   baseFileName,
                                                   sequenceConfig->getInt("sequence_start"),
                                                   dlgAnimationRenderer.getFrameExportConfiguration());
        exporter.setBatchMode(batchMode);


        KisAsyncAnimationFramesSaveDialog::Result result =
            exporter.regenerateRange(viewManager()->mainWindow()->viewManager());

        // the folder could have been read-only or something else could happen
        if (result == KisAsyncAnimationFramesSaveDialog::RenderComplete) {
            QString savedFilesMask = exporter.savedFilesMask();

            KisPropertiesConfigurationSP videoConfig = dlgAnimationRenderer.getVideoConfiguration();
            if (videoConfig) {
                kisConfig.setExportConfiguration("ANIMATION_RENDERER", videoConfig);

                KisPropertiesConfigurationSP encoderConfig = dlgAnimationRenderer.getEncoderConfiguration();
                if (encoderConfig) {
                    kisConfig.setExportConfiguration("FFMPEG_CONFIG", encoderConfig);
                    encoderConfig->setProperty("savedFilesMask", savedFilesMask);
                }

                const QString fileName = videoConfig->getString("filename");
                QString resultFile = fileName;
                KIS_SAFE_ASSERT_RECOVER_NOOP(QFileInfo(resultFile).isAbsolute())

                {
                    const QFileInfo info(resultFile);
                    QDir dir(info.absolutePath());

                    if (!dir.exists()) {
                        dir.mkpath(info.absolutePath());
                    }
                    KIS_SAFE_ASSERT_RECOVER_NOOP(dir.exists());
                }

                QSharedPointer<KisImportExportFilter> encoder = dlgAnimationRenderer.encoderFilter();
                encoder->setMimeType(mimetype.toLatin1());
                QFile fi(resultFile);

                KisImportExportFilter::ConversionStatus res;
                if (!fi.open(QIODevice::WriteOnly)) {
                    qWarning() << "Could not open" << fi.fileName() << "for writing!";
                    doc->setErrorMessage(i18n("Could not open %1 for writing!", fi.fileName()));
                    res = KisImportExportFilter::CreationError;
                }
                else {
                    encoder->setFilename(fi.fileName());
                    res = encoder->convert(doc, &fi, encoderConfig);
                    fi.close();
                }
                if (res != KisImportExportFilter::OK) {
                    QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not render animation:\n%1", doc->errorMessage()));
                }
                if (videoConfig->getBool("delete_sequence", false)) {
                    QDir d(sequenceConfig->getString("directory"));
                    QStringList sequenceFiles = d.entryList(QStringList() << sequenceConfig->getString("basename") + "*." + extension, QDir::Files);
                    Q_FOREACH(const QString &f, sequenceFiles) {
                        d.remove(f);
                    }
                }
            }
        } else if (result == KisAsyncAnimationFramesSaveDialog::RenderFailed) {
            viewManager()->mainWindow()->viewManager()->showFloatingMessage(i18n("Failed to render animation frames!"), QIcon());
        }
    }
}

void AnimaterionRenderer::slotRenderSequenceAgain()
{
    KisImageWSP image = viewManager()->image();

    if (!image) return;
    if (!image->animationInterface()->hasAnimation()) return;

    KisDocument *doc = viewManager()->document();

    KisConfig kisConfig(false);
    KisPropertiesConfigurationSP sequenceConfig = new KisPropertiesConfiguration();
    sequenceConfig->fromXML(kisConfig.exportConfiguration("IMAGESEQUENCE"));
    QString mimetype = sequenceConfig->getString("mimetype");
    QString extension = KisMimeDatabase::suffixesForMimeType(mimetype).first();
    QString baseFileName = QString("%1/%2.%3").arg(sequenceConfig->getString("directory"))
            .arg(sequenceConfig->getString("basename"))
            .arg(extension);

    const bool batchMode = false; // TODO: fetch correctly!
    KisAsyncAnimationFramesSaveDialog exporter(doc->image(),
                                               KisTimeRange::fromTime(sequenceConfig->getInt("first_frame"), sequenceConfig->getInt("last_frame")),
                                               baseFileName,
                                               sequenceConfig->getInt("sequence_start"),
                                               0);
    exporter.setBatchMode(batchMode);
    bool success = exporter.regenerateRange(0) == KisAsyncAnimationFramesSaveDialog::RenderComplete;
    KIS_SAFE_ASSERT_RECOVER_NOOP(success);
}

#include "AnimationRenderer.moc"
