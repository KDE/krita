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

#include "sequence_generator.h"

#include <kis_node.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include <kis_types.h>
#include <kis_animation.h>

#include <QImage>
#include <QRect>
#include <QHash>

SequenceGenerator::SequenceGenerator(KisAnimationDoc *doc, QString filename)
{
    m_doc = doc;
    m_dirname = filename;
}

bool SequenceGenerator::generate(bool keyFramesOnly, int startFrame, int stopFrame)
{
    QDir dir;
    dir.mkdir(m_dirname);

    KisAnimation* animation = m_doc->getAnimation();

    this->cache(keyFramesOnly, startFrame, stopFrame);

    int numberOfLayers = m_doc->numberOfLayers();

    for(int i = startFrame; i <= stopFrame ; i++) {

        KisImageWSP image = m_doc->image();

        m_doc->removePreviousLayers();

        for(int layer = 0 ; layer < numberOfLayers ; layer++) {
            KisLayerSP newLayer = m_cache.at(i - 1).value(layer);
            image->addNode(newLayer, image->rootLayer().data());
            m_doc->addCurrentLoadedLayer(newLayer);
        }

        image->refreshGraph();

        QImage outImage = image->convertToQImage(0, 0, animation->width(), animation->height(), animation->colorSpace()->profile());

        QString filename = QString::number(i) + ".png";
        outImage.save(m_dirname + "/" + filename);
    }

    m_cache.clear();

    return true;
}

void SequenceGenerator::cache(bool keyFramesOnly, int startFrame, int stopFrame)
{
    QString location = "";
    bool hasFile = false;

    KisAnimation* animation = m_doc->getAnimation();
    KisImageWSP image = m_doc->image();

    int numberOfLayers = m_doc->numberOfLayers();

    int currentFrame = startFrame;

    QHash<int, KisLayerSP> layersMap;

    while(true) {

        if(currentFrame == stopFrame + 1) {
            break;
        }

        layersMap.clear();

        for(int layer = 0 ; layer < numberOfLayers ; layer++) {
            location = m_doc->getFrameFile(currentFrame * 10, layer * 20);
            hasFile = m_doc->storeHasFile(location);

            if(hasFile) {
                KisLayerSP newLayer = new KisPaintLayer(image.data(), image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
                m_doc->loadFrame(newLayer, location);
                layersMap[layer] = newLayer;
            }
        }

        m_cache.append(layersMap);
        currentFrame++;
    }
}
