/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version..
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
#include "kis_animation_part.h"
#include "kis_animation.h"
#include <kis_paint_layer.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kranim/kis_kranim_saver.h>
#include <kranim/kis_kranim_loader.h>
#include <KoFilterManager.h>
#include <kranimstore/kis_animation_store.h>
#include <kis_animation_player.h>
#include <QList>
#include <QHash>
#include <QThread>
#include <kis_onion_skin_loader.h>

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
    KisKranimSaver *kranimSaver;
    KisKranimLoader *kranimLoader;
    KisLayerSP currentFrame;
    QRect currentFramePosition;
    bool saved;
    KisAnimationStore* store;
    KisAnimationPlayer* player;
    KisImageWSP image;
    QDomElement frameElement;
    int noLayers;
};

KisAnimationDoc::KisAnimationDoc()
    : KisDoc2(new KisAnimationPart),
      d(new KisAnimationDocPrivate())
{
    setMimeType(APP_MIMETYPE);
    d->kranimLoader = 0;
    d->saved = false;
    //d->player = new KisAnimationPlayer(this);
    d->noLayers = 1;
}

KisAnimationDoc::~KisAnimationDoc()
{
    delete d;
}

void KisAnimationDoc::loadAnimationFile(KisAnimation *animation, KisAnimationStore *store, QDomDocument doc)
{
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

    // Load the first frame of the animation
    d->image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
    connect(d->image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    d->image->setResolution(animation->resolution(), animation->resolution());

    QString location = "";
    bool hasFile = false;
    QRect frame = QRect(0, 0, 10, 20);

    for(int i = 0 ; i < d->noLayers ; i++) {
        location = this->getFrameFile(frame.x(), i * 20);
        hasFile = d->store->hasFile(location);

        if(hasFile) {

            KisLayerSP newLayer = new KisPaintLayer(d->image.data(), d->image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
            newLayer->setName("Layer " + QString::number(i + 1));
            d->image->addNode(newLayer.data(), d->image->rootLayer().data());
            d->kranimLoader->loadFrame(newLayer, d->store, location);

            // Current frame
            if(frame.y() == i * 20) {
                d->currentFramePosition = frame;
                d->currentFrame = newLayer;
            }
        }
    }

    this->updateActiveFrame();

    setCurrentImage(d->image);

    emit sigImportFinished(timelineMap);
}

void KisAnimationDoc::frameSelectionChanged(QRect frame)
{
    KisAnimation* animation = this->getAnimation();

    if (!d->saved) {
        d->kranimSaver = new KisKranimSaver(this);
        this->preSaveAnimation();
    }

    // Dump the content of the current frame
    d->kranimSaver->saveFrame(d->store, d->currentFrame, this->getParentFramePosition(d->currentFramePosition.x(), d->currentFramePosition.y()));

    if(d->currentFramePosition == this->getParentFramePosition(frame.x(), frame.y())) {
        return;
    }

    QString location = "";
    bool hasFile = false;

    d->image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
    connect(d->image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    d->image->setResolution(animation->resolution(), animation->resolution());

    for(int i = 0 ; i < d->noLayers ; i++) {
        location = this->getFrameFile(frame.x(), i * 20);
        hasFile = d->store->hasFile(location);

        if(hasFile) {

            KisLayerSP newLayer = new KisPaintLayer(d->image.data(), d->image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
            newLayer->setName("Layer " + QString::number(i + 1));
            d->image->addNode(newLayer.data(), d->image->rootLayer().data());
            d->kranimLoader->loadFrame(newLayer, d->store, location);

            // Current frame
            if(frame.y() == i * 20) {
                d->currentFramePosition = frame;
                d->currentFrame = newLayer;
            }
        }
    }

    this->loadOnionSkins();

    this->updateActiveFrame();
    setCurrentImage(d->image);
}

void KisAnimationDoc::addBlankFrame(QRect frame)
{
    KisAnimation* animation = this->getAnimation();

    if(d->currentFramePosition.x() == 0 && d->currentFramePosition.y() == 0) {
        d->kranimSaver->saveFrame(d->store, this->image()->projection(), d->currentFramePosition);
    }

    d->kranimSaver->saveFrame(d->store, d->currentFrame, d->currentFramePosition);

    d->image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
    connect(d->image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    d->image->setResolution(animation->resolution(), animation->resolution());

    d->currentFramePosition = frame;

    int x = frame.x();
    int y = frame.y() / 20;

    QString location = "";
    bool hasFile = false;

    // Load frames from layers below
    for(int i = 0 ; i < y ; i++) {
        location = this->getFrameFile(x, i * 20);
        hasFile = d->store->hasFile(location);

        if(hasFile) {
            KisLayerSP newLayer = new KisPaintLayer(d->image.data(), d->image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
            newLayer->setName("Layer " + QString::number(i + 1));
            d->image->addNode(newLayer.data(), d->image->rootLayer().data());
            d->kranimLoader->loadFrame(newLayer, d->store, location);
        }
    }

    // Load the new frame
    d->currentFrame = new KisPaintLayer(d->image.data(), d->image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
    d->currentFrame->setName("Layer " + QString::number((d->currentFramePosition.y() / 20) + 1));

    if(d->currentFramePosition.y() == 0) {
        d->currentFrame->paintDevice()->setDefaultPixel(animation->bgColor().data());
    }

    d->image->addNode(d->currentFrame.data(), d->image->rootLayer().data());

    // Load the frames from layers above
    for(int i = y + 1; i < d->noLayers ; i++) {
        location = this->getFrameFile(x, i * 20);
        hasFile = d->store->hasFile(location);

        if(hasFile) {
            KisLayerSP newLayer = new KisPaintLayer(d->image.data(), d->image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
            newLayer->setName("Layer " + QString::number(i + 1));
            d->image->addNode(newLayer.data(), d->image->rootLayer().data());
            d->kranimLoader->loadFrame(newLayer, d->store, location);
        }
    }

    this->updateXML();
    this->updateActiveFrame();

    setCurrentImage(d->image);
}

void KisAnimationDoc::addKeyFrame(QRect frame)
{
    KisAnimation* animation = this->getAnimation();

    if(d->currentFramePosition.x() == 0 && d->currentFramePosition.y() == 0) {
        d->kranimSaver->saveFrame(d->store, this->image()->projection(), d->currentFramePosition);
    }

    d->kranimSaver->saveFrame(d->store, d->currentFrame, d->currentFramePosition);

    d->image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
    connect(d->image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    d->image->setResolution(animation->resolution(), animation->resolution());

    d->currentFramePosition = frame;

    int x = frame.x();
    int y = frame.y() / 20;

    QString location = "";
    bool hasFile = false;

    // Load the frames from layers below
    for(int i = 0 ; i < y ; i++) {
        location = this->getFrameFile(x, i * 20);
        hasFile = d->store->hasFile(location);

        if(hasFile) {
            KisLayerSP newLayer = new KisPaintLayer(d->image.data(), d->image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
            newLayer->setName("Layer " + QString::number(i + 1));
            d->image->addNode(newLayer.data(), d->image->rootLayer().data());
            d->kranimLoader->loadFrame(newLayer, d->store, location);
        }
    }

    // Load the cloned frame
    location = this->getFrameFile(frame.x(), frame.y());
    d->currentFrame = new KisPaintLayer(d->image.data(), d->image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
    d->currentFrame->setName("Layer " + QString::number((d->currentFramePosition.y() / 20) + 1));

    if(d->currentFramePosition.y() == 0) {
        d->currentFrame->paintDevice()->setDefaultPixel(animation->bgColor().data());
    }

    d->image->addNode(d->currentFrame.data(), d->image->rootLayer().data());
    d->kranimLoader->loadFrame(d->currentFrame, d->store, location);

    // Load the frames from layers above
    for(int i = y + 1; i < d->noLayers ; i++) {
        location = this->getFrameFile(x, i * 20);
        hasFile = d->store->hasFile(location);

        if(hasFile) {
            KisLayerSP newLayer = new KisPaintLayer(d->image.data(), d->image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
            newLayer->setName("Layer " + QString::number(i + 1));
            d->image->addNode(newLayer.data(), d->image->rootLayer().data());
            d->kranimLoader->loadFrame(newLayer, d->store, location);
        }
    }

    this->updateXML();
    this->updateActiveFrame();

    setCurrentImage(d->image);
}

void KisAnimationDoc::breakFrame(QRect frame, bool blank)
{
    // Implement breaking of frame and save both the frames.

    if(blank) {
        // Blank frame
        this->addBlankFrame(frame);
    } else {
        // Duplicate frame
        this->addKeyFrame(frame);
    }
}

void KisAnimationDoc::addPaintLayer()
{
    KisAnimation* animation = this->getAnimation();

    if(!d->saved) {
        d->kranimSaver = new KisKranimSaver(this);
        this->preSaveAnimation();
    }

    d->kranimSaver->saveFrame(d->store, d->currentFrame, this->getParentFramePosition(d->currentFramePosition.x(), d->currentFramePosition.y()));

    QString location = "";
    bool hasFile = false;

    d->image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
    connect(d->image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    d->image->setResolution(animation->resolution(), animation->resolution());

    int layer = d->currentFramePosition.y() + 20;
    int frame = 0;

    for(int i = 0 ; i < d->noLayers ; i++) {
        location = this->getFrameFile(frame, i * 20);
        hasFile = d->store->hasFile(location);

        if(hasFile) {

            KisLayerSP newLayer = new KisPaintLayer(d->image.data(), d->image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
            newLayer->setName("Layer " + QString::number(i + 1));
            d->image->addNode(newLayer.data(), d->image->rootLayer().data());
            d->kranimLoader->loadFrame(newLayer, d->store, location);
        }
    }

    d->noLayers++;

    d->currentFramePosition = QRect(frame, layer, 10, 20);
    d->currentFrame = new KisPaintLayer(d->image.data(), d->image->nextLayerName(), OPACITY_OPAQUE_U8, animation->colorSpace());
    d->currentFrame->setName("Layer " + QString::number(d->noLayers));

    d->image->addNode(d->currentFrame.data(), d->image->rootLayer().data());

    this->updateXML();
    this->updateActiveFrame();

    setCurrentImage(d->image);
}

void KisAnimationDoc::addVectorLayer()
{

}

void KisAnimationDoc::loadOnionSkins()
{
    KisAnimation* animation = this->getAnimation();

    if(animation->onionSkinningEnabled()) {

        //QThread* thread = new QThread(this);
        KisOnionSkinLoader* loader = new KisOnionSkinLoader(this);

        loader->loadOnionSkins();

        //connect(thread, SIGNAL(started()), loader, SLOT(loadOnionSkins()));
        //connect(thread, SIGNAL(finished()), loader, SLOT(deleteLater()));
        //loader->moveToThread(thread);

        //thread->start();
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
    QRect parentPos = this->getParentFramePosition(frame, layer);
    QString location = "frame" + QString::number(parentPos.x()) + "layer" + QString::number(parentPos.y());
    return location;
}

QString KisAnimationDoc::getPreviousKeyFrameFile(int frame, int layer)
{
    QRect prevKeyFramePos = this->getPreviousKeyFramePosition(frame, layer);
    QString location = "frame" + QString::number(prevKeyFramePos.x()) + "layer" + QString::number(prevKeyFramePos.y());
    return location;
}

QString KisAnimationDoc::getNextKeyFrameFile(int frame, int layer)
{
    QRect nextKeyFramePos = this->getNextKeyFramePosition(frame, layer);
    QString location = "frame" + QString::number(nextKeyFramePos.x()) + "layer" + QString::number(nextKeyFramePos.y());
    return location;
}

void KisAnimationDoc::updateXML()
{
    QDomElement frameElement = d->doc.createElement("frame");
    frameElement.setAttribute("number", d->currentFramePosition.x());
    frameElement.setAttribute("layer", d->currentFramePosition.y());
    d->frameElement.appendChild(frameElement);

    d->store->openFileWriting("maindoc.xml");
    d->store->writeDataToFile(d->doc.toByteArray());
    d->store->closeFile();
}

void KisAnimationDoc::preSaveAnimation()
{
    KisAnimation* animation = this->getAnimation();

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
    d->currentFrame = this->m_layer;

    this->updateXML();

    d->saved = true;
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

KisLayerSP KisAnimationDoc::currentFrame()
{
    return d->currentFrame;
}

KisImageWSP KisAnimationDoc::currentImage()
{
    return d->image;
}

int KisAnimationDoc::numberOfLayers()
{
    return d->noLayers;
}

KisKranimLoader* KisAnimationDoc::kranimLoader()
{
    return d->kranimLoader;
}

void KisAnimationDoc::updateActiveFrame()
{
    this->setPreActivatedNode(d->currentFrame);
}

KisAnimationStore* KisAnimationDoc::getStore()
{
    return d->store;
}

KisAnimation* KisAnimationDoc::getAnimation()
{
    return dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();
}

void KisAnimationDoc::play()
{
    d->player->play();
}

void KisAnimationDoc::pause()
{
    if(d->player->isPlaying()) {
        d->player->pause();
    }
}

void KisAnimationDoc::stop()
{
    if(d->player->isPlaying()) {
        d->player->stop();
    }
}

#include "kis_animation_doc.moc"
