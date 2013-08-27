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
#include <QObject>

struct KisAnimationPlayer::Private
{
public:
    KisAnimationDoc* doc;
    KisAnimationStore* store;
    KisAnimation* animation;
    QList<KisImageWSP>* cache;
    bool cached;
    bool playing;
};

KisAnimationPlayer::KisAnimationPlayer(KisAnimationDoc *doc)
{
    d->doc = doc;
    d->store = doc->getStore();
    d->cached = false;
    d->playing = false;
    //d->animation = doc->getAnimation();
}

void KisAnimationPlayer::createCache(int length)
{
    d->cached = true;

    d->animation = d->doc->getAnimation();

    QString location;

    for(int frame = 0 ; frame < length ; frame++) {

        location = "frame" + QString::number(frame * 10) + "layer0";

        if(d->store->hasFile(location)) {
            KisImageWSP currentImage = new KisImage(d->doc->createUndoStore(), d->animation->width(), d->animation->height(), d->animation->colorSpace(), d->animation->name());

            connect(currentImage.data(), SIGNAL(sigImageModified()), d->doc, SLOT(setImageModified()));
            currentImage->setResolution(d->animation->resolution(), d->animation->resolution());

            KisLayerSP currentLayer = new KisPaintLayer(currentImage.data(), currentImage->nextLayerName(), d->animation->bgColor().opacityU8(), d->animation->colorSpace());
            currentLayer->setName("testFrame");

            currentLayer->paintDevice()->setDefaultPixel(d->animation->bgColor().data());

            currentImage->addNode(currentLayer.data(), currentImage->rootLayer().data());

            d->cache->append(currentImage);
        }
    }
}

void KisAnimationPlayer::dropCache()
{
    d->cache = 0;
    delete d->cache;
    d->cached = false;
}

bool KisAnimationPlayer::isCached()
{
    return d->cached;
}

bool KisAnimationPlayer::isPlaying()
{
    return d->playing;
}

void KisAnimationPlayer::play()
{
    d->playing = true;
}

void KisAnimationPlayer::pause()
{
    d->playing = false;
}

void KisAnimationPlayer::stop()
{
    d->playing = false;
}
