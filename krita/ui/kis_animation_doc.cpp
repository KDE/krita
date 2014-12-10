/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
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

#include "kis_animation_doc.h"
#include "KisPart.h"
#include "kis_animation.h"
#include "kis_paint_layer.h"
#include "kis_image.h"
#include "kis_group_layer.h"
#include "KisViewManager.h"
#include "kis_node_manager.h"
#include "kranim/kis_kranim_saver.h"
#include "kranim/kis_kranim_loader.h"
#include "kranimstore/kis_animation_store.h"
#include "kis_animation_layer.h"

#include <KisImportExportManager.h>

#include <QList>
#include <QHash>
#include <QThread>

QList<KisNodeSP> flattenLayerTree(QList<KisNodeSP> &nodes, KisNodeSP node)
{
    KisNodeSP child = node->firstChild();
    while (child) {
        nodes << child;
        if (child->childCount() > 0) {
            nodes << flattenLayerTree(nodes, child);
        }
        child = child->nextSibling();
    }
    return nodes;
}

#define APP_MIMETYPE "application/x-krita-animation"

static const char CURRENT_DTD_VERSION[] = "1.0";


class KisAnimationDoc::KisAnimationDocPrivate
{
public:
    KisAnimationDocPrivate()
        :kranimSaver(0),
         kranimLoader(0)
    {
    }

    ~KisAnimationDocPrivate()
    {
    }

    QDomDocument doc;
    QDomElement root;
    QDomElement frameElement;

    QVector<KisAnimationLayer *> layers;

    KisAnimation *animation;

    KisKranimSaver *kranimSaver;
    KisKranimLoader *kranimLoader;

    KisNodeSP currentFrame;
    QRect currentFramePosition;
    QList<KisNodeSP> currentLoadedLayers;

    KisAnimationStore* store;

    int noLayers;
    bool saved;

    QHash<int, bool> onionSkinStates;
    QHash<int, bool> lockStates;
    QHash<int, bool> visibilityStates;

    bool playing;
    QList<QHash<int, KisLayerSP> > cache;
    int currentFrameNumber;
    QTimer* timer;
    bool loop;
    int fps;
    int localPlaybackRange;
};

KisAnimationDoc::KisAnimationDoc()
    : KisDocument()
    , d(new KisAnimationDocPrivate())
{
    setMimeType(APP_MIMETYPE);

    d->kranimLoader = 0;
    d->saved = false;
    d->noLayers = 1;

    d->animation = new KisAnimation(this);

}

KisAnimationDoc::~KisAnimationDoc()
{
    delete d;
}

void KisAnimationDoc::loadAnimationFile(KisAnimation *animation, KisAnimationStore *store, QDomDocument doc)
{
    d->playing = false;
    d->currentFrameNumber = 0;

    d->currentLoadedLayers.append(image()->root()->firstChild());
    removePreviousLayers();

    // Set all the variables
    d->store = store;
    d->doc = doc;
    d->saved = true;
    d->kranimSaver = new KisKranimSaver(this);
    d->frameElement = d->doc.elementsByTagName("frames").at(0).toElement();
    d->root = d->doc.elementsByTagName("kritaanimation").at(0).toElement();

    d->noLayers = 0;

    // Calculate number of layers from the xmldoc
    QDomNodeList list = d->frameElement.childNodes();

    QHash<int, QList<QRect> > timelineMap;

    for(unsigned int i = 0 ; i < list.length() ; i++) {
        QDomNode node = list.at(i);

        int layerNumber = node.attributes().namedItem("layer").nodeValue().toInt();

        timelineMap[layerNumber] << QRect(node.attributes().namedItem("number").nodeValue().toInt(), layerNumber, 10, 20);
    }

    d->noLayers = timelineMap.size();

    QString location = "";
    bool hasFile = false;
    QRect frame = QRect(0, 0, 10, 20);

    d->currentLoadedLayers.clear();

    for(int i = 0 ; i < d->noLayers ; i++) {
        location = getFrameFile(frame.x(), i * 20);
        hasFile = d->store->hasFile(location);

        if(hasFile) {

            KisLayerSP newLayer = new KisPaintLayer(image().data(), image()->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
            newLayer->setName("Layer " + QString::number(i + 1));

            image()->addNode(newLayer.data(), image()->rootLayer().data());
            d->currentLoadedLayers.append(newLayer);

            d->kranimLoader->loadFrame(newLayer, d->store, location);

            // Current frame
            if(frame.y() == i * 20) {
                d->currentFramePosition = frame;
                d->currentFrame = newLayer;
            }
        }
    }

    updateActiveFrame();

    image()->refreshGraph();

    emit sigImportFinished(timelineMap);
}

void KisAnimationDoc::frameSelectionChanged(QRect frame, bool savePreviousFrame)
{
    KisAnimation* animation = getAnimation();

    if (!d->saved) {
        d->kranimSaver = new KisKranimSaver(this);
        preSaveAnimation();
    }

    if(savePreviousFrame) {
        // Dump the content of the current frame
        d->kranimSaver->saveFrame(d->store, d->currentFrame->paintDevice(), getParentFramePosition(d->currentFramePosition.x(), d->currentFramePosition.y()));
    }

    QString location = "";
    bool hasFile = false;

    removePreviousLayers();
    d->currentLoadedLayers.clear();

    for(int i = 0 ; i < d->noLayers ; i++) {
        location = getFrameFile(frame.x(), i * 20);
        hasFile = d->store->hasFile(location);

        if(hasFile) {

            KisLayerSP newLayer = new KisPaintLayer(image().data(), image()->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
            newLayer->setName("Layer " + QString::number(i + 1));

            image()->addNode(newLayer.data(), image()->rootLayer().data());
            d->currentLoadedLayers.append(newLayer);

            d->kranimLoader->loadFrame(newLayer, d->store, location);

            // Current frame
            if(frame.y() == i * 20) {
                d->currentFramePosition = frame;
                d->currentFrame = newLayer;
            }

            applyLayerStates(i, newLayer);
        }
    }

    loadOnionSkins();

    updateActiveFrame();

    image()->refreshGraph();
}

void KisAnimationDoc::addBlankFrame(QRect frame)
{
    KisAnimation* animation = getAnimation();

    if(d->currentFramePosition.x() == 0 && d->currentFramePosition.y() == 0) {
        d->kranimSaver->saveFrame(d->store, image()->projection(), d->currentFramePosition);
    }

    d->kranimSaver->saveFrame(d->store, d->currentFrame->paintDevice(), d->currentFramePosition);

    d->currentFramePosition = frame;

    int x = frame.x();
    int y = frame.y() / 20;

    QString location = "";
    bool hasFile = false;

    removePreviousLayers();
    d->currentLoadedLayers.clear();

    // Load frames from layers below
    for(int i = 0 ; i < y ; i++) {
        location = getFrameFile(x, i * 20);
        hasFile = d->store->hasFile(location);

        if(hasFile) {
            KisLayerSP newLayer = new KisPaintLayer(image().data(), image()->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
            newLayer->setName("Layer " + QString::number(i + 1));

            image()->addNode(newLayer.data(), image()->rootLayer().data());
            d->currentLoadedLayers.append(newLayer);

            d->kranimLoader->loadFrame(newLayer, d->store, location);

            applyLayerStates(i, newLayer);
        }
    }

    // Load the new frame
    d->currentFrame = new KisPaintLayer(image(), image()->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
    d->currentFrame->setName("Layer " + QString::number((d->currentFramePosition.y() / 20) + 1));

    if(d->currentFramePosition.y() == 0) {
        d->currentFrame->paintDevice()->setDefaultPixel(animation->bgColor().data());
    }

    image()->addNode(d->currentFrame.data(), image()->rootLayer().data());
    d->currentLoadedLayers.append(d->currentFrame);

    // Load the frames from layers above
    for(int i = y + 1; i < d->noLayers ; i++) {
        location = getFrameFile(x, i * 20);
        hasFile = d->store->hasFile(location);

        if(hasFile) {
            KisLayerSP newLayer = new KisPaintLayer(image().data(), image()->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
            newLayer->setName("Layer " + QString::number(i + 1));

            image()->addNode(newLayer.data(), image()->rootLayer().data());
            d->currentLoadedLayers.append(newLayer);

            d->kranimLoader->loadFrame(newLayer, d->store, location);

            applyLayerStates(i, newLayer);
        }
    }

    addFrameToXML();

    loadOnionSkins();

    updateActiveFrame();

    image()->refreshGraph();
}

void KisAnimationDoc::addKeyFrame(QRect frame)
{
    KisAnimation* animation = getAnimation();

    if(d->currentFramePosition.x() == 0 && d->currentFramePosition.y() == 0) {
        d->kranimSaver->saveFrame(d->store, image()->projection(), d->currentFramePosition);
    }

    d->kranimSaver->saveFrame(d->store, d->currentFrame->paintDevice(), d->currentFramePosition);

    d->currentFramePosition = frame;

    int x = frame.x();
    int y = frame.y() / 20;

    QString location = "";
    bool hasFile = false;

    removePreviousLayers();
    d->currentLoadedLayers.clear();

    // Load the frames from layers below
    for(int i = 0 ; i < y ; i++) {
        location = getFrameFile(x, i * 20);
        hasFile = d->store->hasFile(location);

        if(hasFile) {
            KisLayerSP newLayer = new KisPaintLayer(image().data(), image()->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
            newLayer->setName("Layer " + QString::number(i + 1));

            image()->addNode(newLayer.data(), image()->rootLayer().data());
            d->currentLoadedLayers.append(newLayer);

            d->kranimLoader->loadFrame(newLayer, d->store, location);

            applyLayerStates(i, newLayer);
        }
    }

    // Load the cloned frame
    location = getFrameFile(frame.x(), frame.y());
    d->currentFrame = new KisPaintLayer(image().data(), image()->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
    d->currentFrame->setName("Layer " + QString::number((d->currentFramePosition.y() / 20) + 1));

    if(d->currentFramePosition.y() == 0) {
        d->currentFrame->paintDevice()->setDefaultPixel(animation->bgColor().data());
    }

    image()->addNode(d->currentFrame.data(), image()->rootLayer().data());
    d->currentLoadedLayers.append(d->currentFrame);

    d->kranimLoader->loadFrame(d->currentFrame, d->store, location);

    // Load the frames from layers above
    for(int i = y + 1; i < d->noLayers ; i++) {
        location = getFrameFile(x, i * 20);
        hasFile = d->store->hasFile(location);

        if(hasFile) {
            KisLayerSP newLayer = new KisPaintLayer(image().data(), image()->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
            newLayer->setName("Layer " + QString::number(i + 1));

            image()->addNode(newLayer.data(), image()->rootLayer().data());
            d->currentLoadedLayers.append(newLayer);

            d->kranimLoader->loadFrame(newLayer, d->store, location);

            applyLayerStates(i, newLayer);
        }
    }

    addFrameToXML();

    loadOnionSkins();

    updateActiveFrame();

    image()->refreshGraph();
}

void KisAnimationDoc::breakFrame(QRect frame, bool blank)
{
    // Implement breaking of frame and save both the frames.

    if(blank) {
        // Blank frame
        addBlankFrame(frame);
    } else {
        // Duplicate frame
        addKeyFrame(frame);
    }
}

void KisAnimationDoc::removeFrame(QRect frame)
{
    deleteFrameFromXML(frame.x(), frame.y());
    saveXMLToDisk();
    frameSelectionChanged(d->currentFramePosition, false);
}

void KisAnimationDoc::removeLayer(int layer)
{
    d->noLayers--;

    deleteLayerFromXML(layer);
    saveXMLToDisk();

    int layerToLoad = 0;
    int frame = d->currentFramePosition.x();

    // Refresh the canvas
    d->currentFramePosition = QRect(frame, layerToLoad, 10, 20);

    frameSelectionChanged(d->currentFramePosition, false);
}

void KisAnimationDoc::moveLayerUp(int layer)
{
    QDomNodeList frames = d->frameElement.childNodes();
    int length = frames.length();

    QDomNode currentNode;

    QHash<int, QList<int> > filesToRename;
    QHash<int, QList<int> > topFilesToRename;

    for(int i = 0 ; i < length ; i++) {
        currentNode = frames.at(i);
        int layerNumber = currentNode.attributes().namedItem("layer").nodeValue().toInt();
        int frameNumber = currentNode.attributes().namedItem("number").nodeValue().toInt();

        if(layerNumber == layer + 20) {
            currentNode.attributes().namedItem("layer").setNodeValue(QString::number(layer));
            topFilesToRename[layerNumber].append(frameNumber);
            continue;
        }

        if(layerNumber == layer) {
            currentNode.attributes().namedItem("layer").setNodeValue(QString::number(layer + 20));
            filesToRename[layerNumber].append(frameNumber);
            continue;
        }
    }

    saveXMLToDisk();

    // Rename the files of the layer above to a temporary name
    length = topFilesToRename.keys().length();

    for(int i = 0 ; i < length ; i++) {
        int layerNumber = topFilesToRename.keys().at(i);
        QList<int> _layer = topFilesToRename.value(layerNumber);

        for(int j = 0 ; j < _layer.length() ; j++) {
            int frameNumber = _layer.at(j);
            d->kranimSaver->renameFrame(d->store, frameNumber, layerNumber, frameNumber, -20);
        }
    }

    // Rename the files of the layer to the new name
    length = filesToRename.keys().length();

    for(int i = 0 ; i < length ; i++) {
        int layerNumber = filesToRename.keys().at(i);
        QList<int> _layer = filesToRename.value(layerNumber);

        for(int j = 0 ; j < _layer.length() ; j++) {
            int frameNumber = _layer.at(j);
            d->kranimSaver->deleteFrame(d->store, frameNumber, layerNumber + 20);
            d->kranimSaver->renameFrame(d->store, frameNumber, layerNumber, frameNumber, layerNumber + 20);
        }
    }

    // Restore the temporary files to the new name
    length = topFilesToRename.keys().length();

    for(int i = 0 ; i < length ; i++) {
        int layerNumber = topFilesToRename.keys().at(i);
        QList<int> _layer = topFilesToRename.value(layerNumber);

        for(int j = 0 ; j < _layer.length() ; j++) {
            int frameNumber = _layer.at(j);
            d->kranimSaver->deleteFrame(d->store, frameNumber, layerNumber - 20);
            d->kranimSaver->renameFrame(d->store, frameNumber, -20, frameNumber, layerNumber - 20);
        }
    }

    QRect currPos = d->currentFramePosition;
    frameSelectionChanged(QRect(currPos.x(), currPos.y() + 20, 10, 20), false);
}

void KisAnimationDoc::moveLayerDown(int layer)
{
    QDomNodeList frames = d->frameElement.childNodes();
    int length = frames.length();

    QDomNode currentNode;
    QHash<int, QList<int> > filesToRename;
    QHash<int, QList<int> > bottomFilesToRename;

    for(int i = 0 ; i < length ; i++) {
        currentNode = frames.at(i);
        int layerNumber = currentNode.attributes().namedItem("layer").nodeValue().toInt();
        int frameNumber = currentNode.attributes().namedItem("number").nodeValue().toInt();

        if(layerNumber == layer - 20) {
            currentNode.attributes().namedItem("layer").setNodeValue(QString::number(layer));
            bottomFilesToRename[layerNumber].append(frameNumber);
            continue;
        }

        if(layerNumber == layer) {
            currentNode.attributes().namedItem("layer").setNodeValue(QString::number(layer - 20));
            filesToRename[layerNumber].append(frameNumber);
            continue;
        }
    }

    saveXMLToDisk();

    // Rename the files of layer below to a temporary name
    length = bottomFilesToRename.keys().length();

    for(int i = 0 ; i < length ; i++) {
        int layerNumber = bottomFilesToRename.keys().at(i);
        QList<int> _layer = bottomFilesToRename.value(layerNumber);

        for(int j = 0 ; j < _layer.length() ; j++) {
            int frameNumber = _layer.at(j);
            d->kranimSaver->renameFrame(d->store, frameNumber, layerNumber, frameNumber, -20);
        }
    }

    // Rename the files of the layer to the new name
    length = filesToRename.keys().length();

    for(int i = 0 ; i < length ; i++) {
        int layerNumber = filesToRename.keys().at(i);
        QList<int> _layer = filesToRename.value(layerNumber);

        for(int j = 0 ; j < _layer.length() ; j++) {
            int frameNumber = _layer.at(j);
            d->kranimSaver->deleteFrame(d->store, frameNumber, layerNumber - 20);
            d->kranimSaver->renameFrame(d->store, frameNumber, layerNumber, frameNumber, layerNumber - 20);
        }
    }

    // Restore the temporary files to the new name
    length = bottomFilesToRename.keys().length();

    for(int i = 0 ; i < length ; i++) {
        int layerNumber = bottomFilesToRename.keys().at(i);
        QList<int> _layer = bottomFilesToRename.value(layerNumber);

        for(int j = 0 ; j < _layer.length() ; j++) {
            int frameNumber = _layer.at(j);
            d->kranimSaver->deleteFrame(d->store, frameNumber, layerNumber + 20);
            d->kranimSaver->renameFrame(d->store, frameNumber, -20, frameNumber, layerNumber + 20);
        }
    }

    QRect currPos = d->currentFramePosition;
    frameSelectionChanged(QRect(currPos.x(), currPos.y() - 20, 10, 20), false);
}

void KisAnimationDoc::addPaintLayer()
{
    KisAnimation* animation = getAnimation();

    if(!d->saved) {
        d->kranimSaver = new KisKranimSaver(this);
        preSaveAnimation();
    }

    d->kranimSaver->saveFrame(d->store, d->currentFrame->paintDevice(), getParentFramePosition(d->currentFramePosition.x(), d->currentFramePosition.y()));

    QString location = "";
    bool hasFile = false;


    int layer = d->currentFramePosition.y() + 20;
    int frame = 0;

    removePreviousLayers();
    d->currentLoadedLayers.clear();

    for(int i = 0 ; i < d->noLayers ; i++) {
        location = getFrameFile(frame, i * 20);
        hasFile = d->store->hasFile(location);

        if(hasFile) {

            KisLayerSP newLayer = new KisPaintLayer(image().data(), image()->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
            newLayer->setName("Layer " + QString::number(i + 1));

            image()->addNode(newLayer.data(), image()->rootLayer().data());
            d->currentLoadedLayers.append(newLayer);

            d->kranimLoader->loadFrame(newLayer, d->store, location);
        }
    }

    d->noLayers++;

    d->currentFramePosition = QRect(frame, layer, 10, 20);
    d->currentFrame = new KisPaintLayer(image().data(), image()->nextLayerName(), OPACITY_OPAQUE_U8, animation->colorSpace());
    d->currentFrame->setName("Layer " + QString::number(d->noLayers));

    image()->addNode(d->currentFrame.data(), image()->rootLayer().data());
    d->currentLoadedLayers.append(d->currentFrame);

    addFrameToXML();

    updateActiveFrame();

    image()->refreshGraph();
}

void KisAnimationDoc::addVectorLayer()
{
}

void KisAnimationDoc::loadOnionSkins()
{
    if (getAnimation()->onionSkinningEnabled()) {
        loadOnionSkins(d->onionSkinStates);
    }
}

QRect KisAnimationDoc::getParentFramePosition(int frame, int layer)
{
    QDomNodeList list = d->frameElement.childNodes();

    QList<int> frameNumbers;

    int frameNo;

    for(unsigned int i = 0 ; i < list.length() ; i++) {
        QDomNode node = list.at(i);

        if(node.attributes().namedItem("layer").nodeValue().toInt() == layer) {

            frameNo = node.attributes().namedItem("number").nodeValue().toInt();

            // Add frames to list which come before, including itself
            if(frameNo <= frame) {
                frameNumbers.append(frameNo);
            }
        }
    }

    if(frameNumbers.length() == 0) {
        return QRect();
    }

    qSort(frameNumbers);

    // Return last of the keyframes in list, which is the parent of frame
    return QRect(frameNumbers.at(frameNumbers.length() - 1), layer, 10, 20);
}

QRect KisAnimationDoc::getPreviousKeyFramePosition(int frame, int layer)
{
    QDomNodeList list = d->frameElement.childNodes();

    QList<int> frameNumbers;

    int frameNo;

    for(unsigned int i = 0 ; i < list.length() ; i++) {
        QDomNode node = list.at(i);

        if(node.attributes().namedItem("layer").nodeValue().toInt() == layer) {

            frameNo = node.attributes().namedItem("number").nodeValue().toInt();

            // Add frames to list which come before, including itself
            if(frameNo <= frame) {
                frameNumbers.append(frameNo);
            }
        }
    }

    qSort(frameNumbers);

    // If first keyframe
    if(frameNumbers.length() <= 1) {
        return QRect(frame, layer, 10, 20);
    }

    // Return last second of the keyframes before given frame
    // Because even the given frame is in the list
    return QRect(frameNumbers.at(frameNumbers.length() - 2), layer, 10, 20);
}

QRect KisAnimationDoc::getNextKeyFramePosition(int frame, int layer)
{
    QDomNodeList list = d->frameElement.childNodes();

    QList<int> frameNumbers;

    int frameNo;

    for(unsigned int i = 0 ; i < list.length() ; i++) {
        QDomNode node = list.at(i);

        if(node.attributes().namedItem("layer").nodeValue().toInt() == layer) {

            frameNo = node.attributes().namedItem("number").nodeValue().toInt();

            // Add frames to list which come after
            if(frameNo > frame) {
                frameNumbers.append(frameNo);
            }
        }
    }

    qSort(frameNumbers);

    // If last keyframe
    if(frameNumbers.length() == 0) {
        return QRect(frame, layer, 10, 20);
    }

    // Return minimum of the keyframes after given frame
    return QRect(frameNumbers.at(0), layer, 10, 20);
}

QString KisAnimationDoc::getFrameFile(int frame, int layer)
{
    QRect parentPos = getParentFramePosition(frame, layer);

    if(parentPos == QRect()) {
        return "";
    }

    QString location = "frame" + QString::number(parentPos.x()) + "layer" + QString::number(parentPos.y());
    return location;
}

QString KisAnimationDoc::getPreviousKeyFrameFile(int frame, int layer)
{
    QRect prevKeyFramePos = getPreviousKeyFramePosition(frame, layer);
    QString location = "frame" + QString::number(prevKeyFramePos.x()) + "layer" + QString::number(prevKeyFramePos.y());
    return location;
}

QString KisAnimationDoc::getNextKeyFrameFile(int frame, int layer)
{
    QRect nextKeyFramePos = getNextKeyFramePosition(frame, layer);
    QString location = "frame" + QString::number(nextKeyFramePos.x()) + "layer" + QString::number(nextKeyFramePos.y());
    return location;
}

void KisAnimationDoc::addFrameToXML()
{
    int frame = d->currentFramePosition.x();
    int layer = d->currentFramePosition.y();

    deleteFrameFromXML(frame, layer);

    QDomElement frameElement = d->doc.createElement("frame");
    frameElement.setAttribute("number", frame);
    frameElement.setAttribute("layer", layer);
    d->frameElement.appendChild(frameElement);

    saveXMLToDisk();
}

void KisAnimationDoc::deleteFrameFromXML(int frame, int layer)
{
    QDomNode frameToDelete = getFrameElementFromXML(frame, layer);

    if(!frameToDelete.isNull()) {
        d->frameElement.removeChild(frameToDelete);
    }
}

void KisAnimationDoc::deleteLayerFromXML(int layer)
{
    QDomNodeList frames = d->frameElement.childNodes();
    QDomNode currentNode;

    QList<QDomNode> nodesToRemove;
    QHash<int, QList<int> > filesToRename;

    int length = frames.length();

    for(int i = 0 ; i < length ; i++) {
        currentNode = frames.at(i);
        int layerNumber = currentNode.attributes().namedItem("layer").nodeValue().toInt();
        int frameNumber = currentNode.attributes().namedItem("number").nodeValue().toInt();

        if(layerNumber == layer) {
            // Delete the file from the kranim directory
            d->kranimSaver->deleteFrame(d->store, frameNumber, layerNumber);

            nodesToRemove.append(currentNode);
            continue;
        }

        // Index of layers on top have to be decremented
        if(layerNumber > layer) {

            // Can't rename files over here since they might get deleted in next run of loop
            filesToRename[layerNumber].append(frameNumber);

            currentNode.attributes().namedItem("layer").setNodeValue(QString::number(layerNumber - 20));
        }
    }

    length = nodesToRemove.length();
    for(int i = 0 ; i < length ; i++) {
        d->frameElement.removeChild(nodesToRemove.at(i));
    }

    QList<int> layers = filesToRename.keys();
    int _layer = 0;
    int frame = 0;

    for(int i = 0 ; i < layers.length() ; i++) {
        _layer = layers.at(i);
        QList<int> frames = filesToRename.value(_layer);

        for(int j = 0 ; j < frames.length() ; j++) {
            frame = frames.at(j);
            d->kranimSaver->renameFrame(d->store, frame, _layer, frame, _layer - 20);
        }
    }
}

QDomNode KisAnimationDoc::getFrameElementFromXML(int frame, int layer)
{
    QDomNodeList frames = d->frameElement.childNodes();
    QDomNode requiredFrame;

    int length = frames.length();

    for(int i = 0 ; i < length ; i++) {
        QDomNode node = frames.at(i);
        int frameNumber = node.attributes().namedItem("number").nodeValue().toInt();
        int layerNumber = node.attributes().namedItem("layer").nodeValue().toInt();

        if(frameNumber == frame && layerNumber == layer) {
            requiredFrame = node;
            break;
        }
    }

    return requiredFrame;
}

void KisAnimationDoc::saveXMLToDisk()
{
    d->store->openFileWriting("maindoc.xml");
    d->store->writeDataToFile(d->doc.toByteArray());
    d->store->closeFile();
}

void KisAnimationDoc::preSaveAnimation()
{
    d->currentLoadedLayers.append(image()->root()->firstChild());

    KisAnimation* animation = getAnimation();

    QString filename = animation->location() + "/" + animation->name() + ".kranim";

    QFile* fout = new QFile(filename);

    int i = 1;

    while(fout->exists()) {
        filename = animation->location() + "/" + animation->name() + "("+ QString::number(i) +").kranim";
        i++;
        fout = new QFile(filename);
    }

    d->store = new KisAnimationStore(filename);
    d->store->setMimetype();

    QDomDocument doc;
    d->doc = doc;

    d->root = d->doc.createElement("kritaanimation");
    d->root.setAttribute("version", 1.0);
    d->doc.appendChild(d->root);

    d->kranimSaver->saveMetaData(d->doc, d->root);


    d->frameElement = d->doc.createElement("frames");
    d->root.appendChild(d->frameElement);

    d->store->openFileWriting("maindoc.xml");

    d->store->writeDataToFile(d->doc.toByteArray());

    d->store->closeFile();

    QRect initialFramePosition(0, 0, 10, 20);
    d->currentFramePosition = initialFramePosition;
    d->currentFrame = image()->root()->firstChild();

    addFrameToXML();

    d->saved = true;
}

void KisAnimationDoc::removePreviousLayers()
{
    int length = d->currentLoadedLayers.length();

    for(int i = 0 ; i < length ; i++) {
        image()->removeNode(d->currentLoadedLayers.at(i));
    }
}

void KisAnimationDoc::setImageModified()
{
    setModified(true);
    emit sigFrameModified();
}

QRect KisAnimationDoc::currentFramePosition()
{
    return d->currentFramePosition;
}

int KisAnimationDoc::numberOfLayers()
{
    return d->layers.size();
}

KisAnimationLayer *KisAnimationDoc::layer(int index)
{
    if (d->layers.size() > index) {
        return d->layers.at(index);
    }

    return 0;
}

int KisAnimationDoc::numberOfFrames() const
{
    int frame = 0;
    foreach(const KisAnimationLayer *layer, d->layers) {
        frame = qMax(frame, layer->maxFramePosition());
    }
    return frame;
}

void KisAnimationDoc::resetAnimationLayers()
{
    // Create the initial animation layer structure. Later, when the user
    // uses the layerbox or the menus to create layers, we'll have to update
    // this structure.
    QList<KisNodeSP> nodes;
    nodes = flattenLayerTree(nodes, image()->rootLayer());
    foreach(KisNodeSP node, nodes) {
        d->layers << new KisAnimationLayer(node);
    }
}

KisKranimLoader* KisAnimationDoc::kranimLoader()
{
    return d->kranimLoader;
}

void KisAnimationDoc::updateActiveFrame()
{
    setPreActivatedNode(d->currentFrame);
    QPointer<KisView> view = KisPart::instance()->views().first();
    if (view) {
        //dynamic_cast<KisViewManager*>(view.data())->nodeManager()->slotNonUiActivatedNode(d->currentFrame);
    }
}

KisAnimationStore* KisAnimationDoc::getStore()
{
    return d->store;
}

KisAnimation* KisAnimationDoc::getAnimation()
{
    return d->animation;
}

void KisAnimationDoc::play()
{
    if (!isPlaying() && d->saved) {
        d->playing = true;

        d->loop = getAnimation()->loopingEnabled();
        d->fps = getAnimation()->fps();
        d->localPlaybackRange = getAnimation()->localPlaybackRange();

        cache();

        d->currentFrameNumber = 0;

        d->timer = new QTimer(this);
        connect(d->timer, SIGNAL(timeout()), this, SLOT(updateFrame()));

        int frameInterval = 1000 / d->fps;
        d->timer->start(frameInterval);

    }
}

void KisAnimationDoc::pause()
{
    if (isPlaying()) {
        d->playing = false;
        d->timer->stop();
        d->cache.clear();

    }
}

void KisAnimationDoc::stop()
{
    if(isPlaying()) {
        d->playing = false;
        d->timer->stop();
        d->cache.clear();
        frameSelectionChanged(currentFramePosition());

    }
}

void KisAnimationDoc::onionSkinStateChanged()
{
    if(getAnimation()->onionSkinningEnabled()) {
        refreshOnionSkins();
    } else {
        frameSelectionChanged(currentFramePosition());
    }
}

void KisAnimationDoc::loadOnionSkins(QHash<int, bool> states)
{
    QRect frame = currentFramePosition();
    KisAnimation* animation = getAnimation();

    QString location = "";
    bool hasFile = false;

    QBitArray prevChanFlags = prevFramesChannelFlags();
    QBitArray nextChanFlags = nextFramesChannelFlags();

    QList<int>* prevOnionSkinOpacityVal = animation->prevOnionSkinOpacityValues();
    QList<int>* nextOnionSkinOpacityVal = animation->nextOnionSkinOpacityValues();

    int currentFrame;
    int numberOfOnionSkins;

    for(int i = 1 ; i < d->noLayers ; i++) {

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
            if(currentFrame == getPreviousKeyFramePosition(currentFrame, i * 20).x()) {
                break;
            }

            location = getPreviousKeyFrameFile(currentFrame, i * 20);
            hasFile = getStore()->hasFile(location);

            if(hasFile) {
                KisLayerSP newLayer = new KisPaintLayer(image().data(), image()->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
                newLayer->setName("Onion Skin " + QString::number(i + 1));

                newLayer->setPercentOpacity(prevOnionSkinOpacityVal->at(numberOfOnionSkins - j - 1));
                newLayer->setChannelFlags(prevChanFlags);
                newLayer->setUserLocked(true);

                image()->addNode(newLayer.data(), image()->rootLayer().data());
                addCurrentLoadedLayer(newLayer);

                kranimLoader()->loadFrame(newLayer, getStore(), location);
            }

            currentFrame = getPreviousKeyFramePosition(currentFrame, i * 20).x();

        }

        currentFrame = frame.x();
        numberOfOnionSkins = nextOnionSkinOpacityVal->length();

        for(int j = 0 ; j < nextOnionSkinOpacityVal->length() ; j++) {

            // A hack to prevent same onion skin multiple times
            // when there are no next onion skins left.
            if(currentFrame == getNextKeyFramePosition(currentFrame, i * 20).x()) {
                break;
            }

            location = getNextKeyFrameFile(currentFrame, i * 20);

            hasFile = getStore()->hasFile(location);

            if(hasFile) {
                KisLayerSP newLayer = new KisPaintLayer(image(), image()->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
                newLayer->setName("Onion Skin " + QString::number(i + 1));

                newLayer->setPercentOpacity(nextOnionSkinOpacityVal->at(j));
                newLayer->setChannelFlags(nextChanFlags);
                newLayer->setUserLocked(true);

                image()->addNode(newLayer.data(), image()->rootLayer().data());
                addCurrentLoadedLayer(newLayer);

                kranimLoader()->loadFrame(newLayer, getStore(), location);
            }

            currentFrame = getNextKeyFramePosition(currentFrame, i * 20).x();
        }
    }
}


void KisAnimationDoc::playbackStateChanged()
{
    if (isPlaying()) {
        refresh();
    }
}

void KisAnimationDoc::addCurrentLoadedLayer(KisLayerSP layer)
{
    d->currentLoadedLayers.append(layer);
}

bool KisAnimationDoc::storeHasFile(QString location)
{
    return d->store->hasFile(location);
}

void KisAnimationDoc::loadFrame(KisLayerSP layer, QString location)
{
    d->kranimLoader->loadFrame(layer, d->store, location);
}

QList<int> KisAnimationDoc::keyFramePositions()
{
    QDomNodeList list = d->frameElement.childNodes();

    QList<int> frameNumbers;

    int frameNo;

    for(unsigned int i = 0 ; i < list.length() ; i++) {
        QDomNode node = list.at(i);
        frameNo = node.attributes().namedItem("number").nodeValue().toInt();

        if(!frameNumbers.contains(frameNo)) {
            frameNumbers.append(frameNo);
        }
    }

    qSort(frameNumbers);

    return frameNumbers;
}

void KisAnimationDoc::onionSkinStateToggled(QHash<int, bool> states)
{
    d->onionSkinStates = states;
    refreshOnionSkins();
}

void KisAnimationDoc::visibilityStateToggled(QHash<int, bool> states)
{
    d->visibilityStates = states;
    frameSelectionChanged(d->currentFramePosition);
}

void KisAnimationDoc::lockStateToggled(QHash<int, bool> states)
{
    d->lockStates = states;
    frameSelectionChanged(d->currentFramePosition);
}

void KisAnimationDoc::applyLayerStates(int layerNumber, KisLayerSP layer)
{
    // Check visibility
    bool visibilityState = false;

    if(d->visibilityStates.keys().contains(layerNumber)) {
        visibilityState = d->visibilityStates[layerNumber];
    }

    layer->setVisible(!visibilityState);

    // Check layer lock
    bool layerLock = false;

    if(d->lockStates.keys().contains(layerNumber)) {
        layerLock = d->lockStates[layerNumber];
    }

    layer->setUserLocked(layerLock);
}

QBitArray KisAnimationDoc::nextFramesChannelFlags()
{
    QBitArray ba(4);

    ba.setBit(0, false);
    ba.setBit(1, true);
    ba.setBit(2, true);
    ba.setBit(3, true);

    return ba;
}

QBitArray KisAnimationDoc::prevFramesChannelFlags()
{
    QBitArray ba(4);

    ba.setBit(0, true);
    ba.setBit(1, true);
    ba.setBit(2, false);
    ba.setBit(3, true);

    return ba;
}

void KisAnimationDoc::refreshOnionSkins()
{
    frameSelectionChanged(currentFramePosition());
}

void KisAnimationDoc::updateFrame()
{
    if(d->currentFrameNumber > d->localPlaybackRange - 1) {
        if(d->loop) {
            d->currentFrameNumber = 0;
        } else {
            this->stop();
            return;
        }
    }

    removePreviousLayers();

    for (int layer = 0 ; layer < d->noLayers ; layer++) {
        KisLayerSP newLayer = d->cache.at(d->currentFrameNumber).value(layer);
        image()->addNode(newLayer, image()->rootLayer().data());
        addCurrentLoadedLayer(newLayer);
    }

    image()->refreshGraph();
    d->currentFrameNumber++;
}

bool KisAnimationDoc::isPlaying()
{
    return d->playing;
}

void KisAnimationDoc::refresh()
{
    // Cannot refresh range of loopback as it violates cache
    d->loop = getAnimation()->loopingEnabled();
    d->fps = getAnimation()->fps();
    d->timer->setInterval(1000 / d->fps);
}

void KisAnimationDoc::cache()
{
    QString location = "";
    bool hasFile = false;

    KisAnimation* animation = getAnimation();
    int currentFrame = 0;

    QHash<int, KisLayerSP> layersMap;

    while(true) {

        if(currentFrame == d->localPlaybackRange) {
            break;
        }

        layersMap.clear();

        for(int layer = 0 ; layer < d->noLayers ; layer++) {
            location = getFrameFile(currentFrame * 10, layer * 20);
            hasFile = getStore()->hasFile(location);

            if(hasFile) {
                KisLayerSP newLayer = new KisPaintLayer(image(), image()->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
                kranimLoader()->loadFrame(newLayer, getStore(), location);
                layersMap[layer] = newLayer;
            }
        }

        d->cache.append(layersMap);
        currentFrame++;
    }
}

#include "kis_animation_doc.moc"
