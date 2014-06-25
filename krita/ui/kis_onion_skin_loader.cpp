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
#include <QBitArray>

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

    QBitArray prevChanFlags = this->prevFramesChannelFlags();
    QBitArray nextChanFlags = this->nextFramesChannelFlags();

    QList<int>* prevOnionSkinOpacityVal = animation->prevOnionSkinOpacityValues();
    QList<int>* nextOnionSkinOpacityVal = animation->nextOnionSkinOpacityValues();

    int currentFrame;
    int numberOfOnionSkins;

    for(int i = 1 ; i < m_doc->numberOfLayers() ; i++) {

        currentFrame = frame.x();
        numberOfOnionSkins = prevOnionSkinOpacityVal->length();

        for(int j = 0 ; j < numberOfOnionSkins ; j++) {

            location = m_doc->getPreviousKeyFrameFile(currentFrame, i * 20);
            hasFile = m_doc->getStore()->hasFile(location);

            if(hasFile) {
                KisLayerSP newLayer = new KisPaintLayer(image.data(), image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
                newLayer->setName("Onion Skin " + QString::number(i + 1));

                newLayer->setOpacity(this->normalizeOpacityValue(prevOnionSkinOpacityVal->at(numberOfOnionSkins - j - 1)));
                newLayer->setChannelFlags(prevChanFlags);
                newLayer->setUserLocked(true);

                image->addNode(newLayer.data(), image->rootLayer().data());
                m_doc->kranimLoader()->loadFrame(newLayer, m_doc->getStore(), location);
            }

            currentFrame = m_doc->getPreviousKeyFramePosition(currentFrame, i * 20).x();

            // A hack to prevent same onion skin multiple times
            // when there are no previous onion skins left.
            if(currentFrame == m_doc->getPreviousKeyFramePosition(currentFrame, i * 20).x()) {
                break;
            }
        }

        currentFrame = frame.x();
        numberOfOnionSkins = nextOnionSkinOpacityVal->length();

        for(int j = 0 ; j < nextOnionSkinOpacityVal->length() ; j++) {

            location = m_doc->getNextKeyFrameFile(currentFrame, i * 20);
            hasFile = m_doc->getStore()->hasFile(location);

            if(hasFile) {
                KisLayerSP newLayer = new KisPaintLayer(image.data(), image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
                newLayer->setName("Onion Skin " + QString::number(i + 1));

                newLayer->setOpacity(this->normalizeOpacityValue(nextOnionSkinOpacityVal->at(j)));
                newLayer->setChannelFlags(nextChanFlags);
                newLayer->setUserLocked(true);

                image->addNode(newLayer.data(), image->rootLayer().data());
                m_doc->kranimLoader()->loadFrame(newLayer, m_doc->getStore(), location);
            }

            currentFrame = m_doc->getNextKeyFramePosition(currentFrame, i * 20).x();

            // A hack to prevent same onion skin multiple times
            // when there are no next onion skins left.
            if(currentFrame == m_doc->getNextKeyFramePosition(currentFrame, i * 20).x()) {
                break;
            }
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

int KisOnionSkinLoader::normalizeOpacityValue(int val)
{
    return (val * 255) / 100;
}

void KisOnionSkinLoader::setNextFramesColor()
{
    kWarning() << "Setting next onion skin color";
}

void KisOnionSkinLoader::setPrevFramesColor()
{
    kWarning() << "Setting previous onion skin color";
}

void KisOnionSkinLoader::setNextFramesNumber()
{
    kWarning() << "Setting next onion skins number";
}

void KisOnionSkinLoader::setPrevFramesNumber()
{
    kWarning() << "Setting previous onion skins number";
}

void KisOnionSkinLoader::setNextFramesOpacity()
{
    kWarning() << "Setting next onion skin opacity";
}

void KisOnionSkinLoader::setPrevFramesOpacity()
{
    kWarning() << "Setting previous onion skin opacity";
}
