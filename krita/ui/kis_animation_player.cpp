/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
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
 */

#include "kis_animation_player.h"
#include <./kranimstore/kis_animation_store.h>
#include <QTimer>
#include <kis_animation.h>
#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_animation_part.h>
#include <kranim/kis_kranim_loader.h>

#include <QObject>
#include <QTimer>

KisAnimationPlayer::KisAnimationPlayer(KisAnimationDoc *doc)
{
    m_doc = doc;
    m_playing = false;
    m_currentFrame = 0;
    m_loop = true;
}

void KisAnimationPlayer::updateFrame()
{
    if(m_currentFrame > 14) {
        if(m_loop) {
            m_currentFrame = 0;
        } else {
            this->stop();
            return;
        }
    }

    int numberOfLayers = m_doc->numberOfLayers();

    KisImageWSP image = m_doc->image();

    m_doc->removePreviousLayers();

    for(int layer = 0 ; layer < numberOfLayers ; layer++) {
        KisLayerSP newLayer = m_cache.at(m_currentFrame).value(layer);
        image->addNode(newLayer, image->rootLayer().data());
        m_doc->addCurrentLoadedLayer(newLayer);
    }

    image->refreshGraph();
    m_currentFrame++;
}

void KisAnimationPlayer::play(bool cache)
{
    qDebug() << "Play";
    m_playing = true;

    if(cache) {
        this->cache();
    }

    m_currentFrame = 0;

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(updateFrame()));
    m_timer->start(100);
}

void KisAnimationPlayer::pause()
{
    qDebug() << "Pause";
    m_playing = false;
    m_timer->stop();
    m_cache.clear();
}

void KisAnimationPlayer::stop()
{
    qDebug() << "Stop";
    m_playing = false;
    m_timer->stop();
    m_cache.clear();
    m_doc->frameSelectionChanged(m_doc->currentFramePosition());
}

bool KisAnimationPlayer::isPlaying()
{
    return m_playing;
}

void KisAnimationPlayer::cache()
{
    QString location = "";
    bool hasFile = false;

    KisAnimation* animation = m_doc->getAnimation();
    KisImageWSP image = m_doc->image();

    int numberOfLayers = m_doc->numberOfLayers();

    int currentFrame = 0;

    QHash<int, KisLayerSP> layersMap;

    while(true) {

        if(currentFrame == 15) {
            break;
        }

        layersMap.clear();

        for(int layer = 0 ; layer < numberOfLayers ; layer++) {
            location = m_doc->getFrameFile(currentFrame * 10, layer * 20);
            hasFile = m_doc->getStore()->hasFile(location);

            if(hasFile) {
                KisLayerSP newLayer = new KisPaintLayer(image.data(), image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
                m_doc->kranimLoader()->loadFrame(newLayer, m_doc->getStore(), location);
                layersMap[layer] = newLayer;
            }
        }

        m_cache.append(layersMap);
        currentFrame++;
    }
}
