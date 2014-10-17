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

#include "kis_animation.h"
#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_types.h"
#include "kis_group_layer.h"
#include "kranim/kis_kranim_loader.h"

#include <QBitArray>
#include <QHash>

KisOnionSkinLoader::KisOnionSkinLoader(KisAnimationDoc *doc, QObject *parent) :
    QObject(parent)
{
    m_doc = doc;
}

void KisOnionSkinLoader::loadOnionSkins(QHash<int, bool> states)
{
    KisImageWSP image = m_doc->image();

    QRect frame = m_doc->currentFramePosition();
    KisAnimation* animation = m_doc->getAnimation();

    QString location = "";
    bool hasFile = false;

    QBitArray prevChanFlags = this->prevFramesChannelFlags();
    QBitArray nextChanFlags = this->nextFramesChannelFlags();

    QList<int>* prevOnionSkinOpacityVal = animation->prevOnionSkinOpacityValues();
    QList<int>* nextOnionSkinOpacityVal = animation->nextOnionSkinOpacityValues();

    int currentFrame;
    int numberOfOnionSkins;

    for(int i = 1 ; i < m_doc->numberOfLayers() ; i++) {

        // Check if onion skin has to be loaded or not
        bool state = false;

        if(states.keys().contains(i)) {
            state = states[i];
        }

        if(state) {
            continue;
        }

        currentFrame = frame.x();
        numberOfOnionSkins = prevOnionSkinOpacityVal->length();

        for(int j = 0 ; j < numberOfOnionSkins ; j++) {

            // A hack to prevent same onion skin multiple times
            // when there are no previous onion skins left.
            if(currentFrame == m_doc->getPreviousKeyFramePosition(currentFrame, i * 20).x()) {
                break;
            }

            location = m_doc->getPreviousKeyFrameFile(currentFrame, i * 20);
            hasFile = m_doc->getStore()->hasFile(location);

            if(hasFile) {
                KisLayerSP newLayer = new KisPaintLayer(image.data(), image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
                newLayer->setName("Onion Skin " + QString::number(i + 1));

                newLayer->setPercentOpacity(prevOnionSkinOpacityVal->at(numberOfOnionSkins - j - 1));
                newLayer->setChannelFlags(prevChanFlags);
                newLayer->setUserLocked(true);

                image->addNode(newLayer.data(), image->rootLayer().data());
                m_doc->addCurrentLoadedLayer(newLayer);

                m_doc->kranimLoader()->loadFrame(newLayer, m_doc->getStore(), location);
            }

            currentFrame = m_doc->getPreviousKeyFramePosition(currentFrame, i * 20).x();

        }

        currentFrame = frame.x();
        numberOfOnionSkins = nextOnionSkinOpacityVal->length();

        for(int j = 0 ; j < nextOnionSkinOpacityVal->length() ; j++) {

            // A hack to prevent same onion skin multiple times
            // when there are no next onion skins left.
            if(currentFrame == m_doc->getNextKeyFramePosition(currentFrame, i * 20).x()) {
                break;
            }

            location = m_doc->getNextKeyFrameFile(currentFrame, i * 20);

            hasFile = m_doc->getStore()->hasFile(location);

            if(hasFile) {
                KisLayerSP newLayer = new KisPaintLayer(image.data(), image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
                newLayer->setName("Onion Skin " + QString::number(i + 1));

                newLayer->setPercentOpacity(nextOnionSkinOpacityVal->at(j));
                newLayer->setChannelFlags(nextChanFlags);
                newLayer->setUserLocked(true);

                image->addNode(newLayer.data(), image->rootLayer().data());
                m_doc->addCurrentLoadedLayer(newLayer);

                m_doc->kranimLoader()->loadFrame(newLayer, m_doc->getStore(), location);
            }

            currentFrame = m_doc->getNextKeyFramePosition(currentFrame, i * 20).x();
        }
    }
}

QBitArray KisOnionSkinLoader::nextFramesChannelFlags()
{
    QBitArray ba(4);

    ba.setBit(0, false);
    ba.setBit(1, true);
    ba.setBit(2, true);
    ba.setBit(3, true);

    return ba;
}

QBitArray KisOnionSkinLoader::prevFramesChannelFlags()
{
    QBitArray ba(4);

    ba.setBit(0, true);
    ba.setBit(1, true);
    ba.setBit(2, false);
    ba.setBit(3, true);

    return ba;
}

void KisOnionSkinLoader::refreshOnionSkins()
{
    m_doc->frameSelectionChanged(m_doc->currentFramePosition());
}
