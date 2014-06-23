/*
 *  Copyright (c) 2014 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_onion_skin_loader.h"

#include <kis_animation.h>
#include <kis_paint_layer.h>
#include <kis_image.h>
#include <kis_types.h>
#include <kis_group_layer.h>
#include <kranim/kis_kranim_loader.h>

KisOnionSkinLoader::KisOnionSkinLoader(KisAnimationDoc *doc, QObject *parent) :
    QObject(parent)
{
    m_doc = doc;
}

void KisOnionSkinLoader::loadOnionSkins()
{
    KisImageWSP image = m_doc->currentImage();

    QRect frame = m_doc->currentFramePosition();
    KisAnimation* animation = m_doc->getAnimation();

    QString location = "";
    bool hasFile = false;

    for(int i = 1 ; i < m_doc->numberOfLayers() ; i++) {
        location = m_doc->getPreviousKeyFrameFile(frame.x(), i * 20);
        hasFile = m_doc->getStore()->hasFile(location);

        kWarning() << location;

        if(hasFile) {
            KisLayerSP newLayer = new KisPaintLayer(image.data(), image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
            newLayer->setName("Onion Skin " + QString::number(i + 1));
            image->addNode(newLayer.data(), image->rootLayer().data());
            m_doc->kranimLoader()->loadFrame(newLayer, m_doc->getStore(), location);
        }

        location = m_doc->getNextKeyFrameFile(frame.x(), i * 20);
        hasFile = m_doc->getStore()->hasFile(location);

        if(hasFile) {
            KisLayerSP newLayer = new KisPaintLayer(image.data(), image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
            newLayer->setName("Onion Skin " + QString::number(i + 1));
            image->addNode(newLayer.data(), image->rootLayer().data());
            m_doc->kranimLoader()->loadFrame(newLayer, m_doc->getStore(), location);
        }
    }
}
