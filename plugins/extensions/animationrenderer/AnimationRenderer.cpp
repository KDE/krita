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

#include <klocalizedstring.h>
#include <kpluginfactory.h>

#include <kis_image.h>
#include <KisViewManager.h>
#include <kis_node_manager.h>
#include <kis_image_manager.h>
#include <kis_action.h>
#include <kis_image_animation_interface.h>
#include <kis_properties_configuration.h>
#include "DlgAnimationRenderer.h"
#include <kis_config.h>
#include <kis_animation_exporter.h>
#include <KisDocument.h>
#include <KisMimeDatabase.h>

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
    KisDocument *doc = m_view->document();
    if (!image) return;
    if (!image->animationInterface()->hasAnimation()) return;

    DlgAnimaterionRenderer dlgAnimaterionRenderer(image, m_view->mainWindow());

    dlgAnimaterionRenderer.setCaption(i18n("Render Animation"));

    if (dlgAnimaterionRenderer.exec() == QDialog::Accepted) {
        KisPropertiesConfigurationSP sequencecfg = dlgAnimaterionRenderer.getSequenceConfiguration();
        KisConfig kisConfig;
        kisConfig.setExportConfiguration("IMAGESEQUENCE", *sequencecfg.data());
        QString mimetype = sequencecfg->getString("mimetype");
        QString extension = KisMimeDatabase::suffixesForMimeType(mimetype).first();
        QString baseFileName = QString("%1/%2.%3").arg(sequencecfg->getString("directory"))
                .arg(sequencecfg->getString("basename"))
                .arg(extension);
        KisAnimationExportSaver exporter(doc, baseFileName, sequencecfg->getInt("first_frame"), sequencecfg->getInt("last_frame"), sequencecfg->getInt("sequence_start"));
        bool success = exporter.exportAnimation();
        Q_ASSERT(success);
    }
}

void AnimaterionRenderer::slotRenderSequenceAgain()
{

}

#include "AnimationRenderer.moc"
