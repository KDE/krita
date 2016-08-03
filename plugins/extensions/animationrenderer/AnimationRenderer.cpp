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
#include <kis_node_manager.h>
#include <kis_image_manager.h>
#include <kis_action.h>
#include <kis_image_animation_interface.h>
#include <kis_properties_configuration.h>
#include <kis_config.h>
#include <kis_animation_exporter.h>
#include <KisDocument.h>
#include <KisMimeDatabase.h>
#include <kis_time_range.h>
#include <KisImportExportManager.h>
#include <KisFilterChain.h>

#include "DlgAnimationRenderer.h"


K_PLUGIN_FACTORY_WITH_JSON(AnimaterionRendererFactory, "kritaanimationrenderer.json", registerPlugin<AnimaterionRenderer>();)

AnimaterionRenderer::AnimaterionRenderer(QObject *parent, const QVariantList &)
    : KisViewPlugin(parent)
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
    KisImageWSP image = m_view->image();

    if (!image) return;
    if (!image->animationInterface()->hasAnimation()) return;

    KisDocument *doc = m_view->document();
    doc->setFileProgressProxy();
    doc->setFileProgressUpdater(i18n("Export frames"));

    DlgAnimationRenderer dlgAnimationRenderer(doc, m_view->mainWindow());

    dlgAnimationRenderer.setCaption(i18n("Render Animation"));

    KisConfig kisConfig;
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->fromXML(kisConfig.exportConfiguration("IMAGESEQUENCE"));
    // Override the saved start/end with the ones from the image in case of using gui.
    cfg->setProperty("first_frame", image->animationInterface()->playbackRange().start());
    cfg->setProperty("last_frame", image->animationInterface()->playbackRange().end());
    dlgAnimationRenderer.setSequenceConfiguration(cfg);

    cfg->clearProperties();
    cfg->fromXML(kisConfig.exportConfiguration("ANIMATION_RENDERER"));
    dlgAnimationRenderer.setVideoConfiguration(cfg);

    cfg->clearProperties();
    cfg->fromXML(kisConfig.exportConfiguration("FFMPEG_CONFIG"));
    dlgAnimationRenderer.setEncoderConfiguration(cfg);


    if (dlgAnimationRenderer.exec() == QDialog::Accepted) {
        KisPropertiesConfigurationSP sequenceConfig = dlgAnimationRenderer.getSequenceConfiguration();
        kisConfig.setExportConfiguration("IMAGESEQUENCE", *sequenceConfig.data());
        QString mimetype = sequenceConfig->getString("mimetype");
        QString extension = KisMimeDatabase::suffixesForMimeType(mimetype).first();
        QString baseFileName = QString("%1/%2.%3").arg(sequenceConfig->getString("directory"))
                .arg(sequenceConfig->getString("basename"))
                .arg(extension);

        KisAnimationExportSaver exporter(doc, baseFileName, sequenceConfig->getInt("first_frame"), sequenceConfig->getInt("last_frame"), sequenceConfig->getInt("sequence_start"));
        bool success = exporter.exportAnimation(dlgAnimationRenderer.getFrameExportConfiguration());
        Q_ASSERT(success);
        QString savedFilesMask = exporter.savedFilesMask();

        KisPropertiesConfigurationSP videoConfig = dlgAnimationRenderer.getVideoConfiguration();
        if (videoConfig) {
            kisConfig.setExportConfiguration("ANIMATION_RENDERER", *videoConfig.data());

            KisPropertiesConfigurationSP encoderConfig = dlgAnimationRenderer.getEncoderConfiguration();
            if (encoderConfig) {
                kisConfig.setExportConfiguration("FFMPEG_CONFIG", *encoderConfig.data());
                encoderConfig->setProperty("savedFilesMask", savedFilesMask);
            }

            QSharedPointer<KisImportExportFilter> encoder = dlgAnimationRenderer.encoderFilter();
            KisFilterChainSP chain(new KisFilterChain(doc->importExportManager()));
            chain->setOutputFile(videoConfig->getString("filename"));
            encoder->setChain(chain);
            KisImportExportFilter::ConversionStatus res = encoder->convert(KisDocument::nativeFormatMimeType(), encoderConfig->getString("mimetype").toLatin1(), encoderConfig);
            if (res != KisImportExportFilter::OK) {
                QMessageBox::critical(0, i18nc("@title:window", "Krita"), i18n("Could not render animation:\n%1", doc->errorMessage()));
            }
        }
    }

    doc->clearFileProgressUpdater();
    doc->clearFileProgressProxy();

}

void AnimaterionRenderer::slotRenderSequenceAgain()
{
    KisImageWSP image = m_view->image();

    if (!image) return;
    if (!image->animationInterface()->hasAnimation()) return;

    KisDocument *doc = m_view->document();
    doc->setFileProgressProxy();
    doc->setFileProgressUpdater(i18n("Export frames"));

    KisConfig kisConfig;
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->fromXML(kisConfig.exportConfiguration("IMAGESEQUENCE"));
    QString mimetype = cfg->getString("mimetype");
    QString extension = KisMimeDatabase::suffixesForMimeType(mimetype).first();
    QString baseFileName = QString("%1/%2.%3").arg(cfg->getString("directory"))
            .arg(cfg->getString("basename"))
            .arg(extension);
    KisAnimationExportSaver exporter(doc, baseFileName, cfg->getInt("first_frame"), cfg->getInt("last_frame"), cfg->getInt("sequence_start"));
    bool success = exporter.exportAnimation();
    Q_ASSERT(success);

    doc->clearFileProgressUpdater();
    doc->clearFileProgressProxy();

}

#include "AnimationRenderer.moc"
