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
    d->player = new KisAnimationPlayer(this);
    d->noLayers = 1;
}

KisAnimationDoc::~KisAnimationDoc()
{
    delete d;
}

void KisAnimationDoc::loadAnimationFile(KisAnimation *animation, KisAnimationStore *store)
{
    kWarning() << "Laoding animation file";
    d->store = store;

    d->image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
    connect(d->image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    d->image->setResolution(animation->resolution(), animation->resolution());

    d->currentFramePosition = QRect(0, 0, 10, 20);
    d->currentFrame = new KisPaintLayer(d->image.data(), d->image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
    d->currentFrame->setName("Layer " + QString::number(d->noLayers));
    d->currentFrame->paintDevice()->setDefaultPixel(animation->bgColor().data());
    d->image->addNode(d->currentFrame.data(), d->image->rootLayer().data());

    d->kranimLoader->loadFrame(d->currentFrame, d->store, "frame0layer0");

    setCurrentImage(d->image);
}

void KisAnimationDoc::frameSelectionChanged(QRect frame)
{
    KisAnimation* animation = dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();

    if (!d->saved) {
        d->kranimSaver = new KisKranimSaver(this);
        this->preSaveAnimation();
        return;
    }

    // Dump the content of the current frame
    d->kranimSaver->saveFrame(d->store, d->currentFrame, this->getParentFramePosition(d->currentFramePosition.x(), d->currentFramePosition.y()));

    if(d->currentFramePosition == this->getParentFramePosition(frame.x(), frame.y())) {
        return;
    }

    QString location = this->getFrameFile(frame.x(), frame.y());

    bool hasFile = d->store->hasFile(location);

    if(hasFile) {

        d->image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
        connect(d->image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
        d->image->setResolution(animation->resolution(), animation->resolution());

        d->currentFramePosition = frame;
        d->currentFrame = new KisPaintLayer(d->image.data(), d->image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
        d->currentFrame->setName("Layer " + QString::number((d->currentFramePosition.y() / 20) + 1));
        d->currentFrame->paintDevice()->setDefaultPixel(animation->bgColor().data());
        d->image->addNode(d->currentFrame.data(), d->image->rootLayer().data());

        //Load all the layers here

        d->kranimLoader->loadFrame(d->currentFrame, d->store, location);

        setCurrentImage(d->image);
    }
}

QRect KisAnimationDoc::getParentFramePosition(int frame, int layer)
{
    QDomNodeList list = d->frameElement.childNodes();

    QList<int> frameNumbers;

    for(int i = 0 ; i < list.length() ; i++) {
        QDomNode node = list.at(i);

        if(node.attributes().namedItem("layer").nodeValue().toInt() == layer) {
            frameNumbers.append(node.attributes().namedItem("number").nodeValue().toInt());
        }
    }

    qSort(frameNumbers);

    if(frameNumbers.contains(frame)) {
        QRect parentFramePos(frame, layer, 10, 20);
        return parentFramePos;
    }

    int frameN;
    for(int i = 0 ; i < frameNumbers.length() ; i++) {
        if(frameNumbers.at(i) < frame) {
            frameN = frameNumbers.at(i);
        }
    }

    QRect parentFramePos(frameN, layer, 10, 20);

    return parentFramePos;
}

QString KisAnimationDoc::getFrameFile(int frame, int layer)
{
    QRect parentPos = this->getParentFramePosition(frame, layer);
    QString location = "frame" + QString::number(parentPos.x()) + "layer" + QString::number(parentPos.y());
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

void KisAnimationDoc::addBlankFrame(QRect frame)
{

    KisAnimation* animation = dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();

    if(d->currentFramePosition.x() == 0 && d->currentFramePosition.y() == 0) {
        d->kranimSaver->saveFrame(d->store, this->image()->projection(), d->currentFramePosition);
    }

    d->kranimSaver->saveFrame(d->store, d->currentFrame, d->currentFramePosition);

    d->image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
    connect(d->image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    d->image->setResolution(animation->resolution(), animation->resolution());

    d->currentFramePosition = frame;
    d->currentFrame = new KisPaintLayer(d->image.data(), d->image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
    d->currentFrame->setName("Layer " + QString::number((d->currentFramePosition.y() / 20) + 1));
    d->currentFrame->paintDevice()->setDefaultPixel(animation->bgColor().data());
    d->image->addNode(d->currentFrame.data(), d->image->rootLayer().data());

    connect(d->image.data(), SIGNAL(sigImageModified()), this, SLOT(slotFrameModified()));
    this->updateXML();

    setCurrentImage(d->image);
}

void KisAnimationDoc::addPaintLayer()
{
    KisAnimation* animation = dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();

    int layer = d->currentFramePosition.y() + 20;
    int frame = 0;

    d->noLayers++;

    d->currentFramePosition = QRect(frame, layer, 10, 20);
    d->currentFrame = new KisPaintLayer(d->image.data(), d->image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
    d->currentFrame->setName("Layer " + QString::number(d->noLayers));
    d->currentFrame->paintDevice()->setDefaultPixel(animation->bgColor().data());
    d->image->addNode(d->currentFrame.data(), d->image->rootLayer().data());

    this->updateXML();
}

void KisAnimationDoc::slotFrameModified()
{
    emit sigFrameModified();
}

void KisAnimationDoc::addKeyFrame(QRect frame)
{

    KisAnimation* animation = dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();

    if(d->currentFramePosition.x() == 0 && d->currentFramePosition.y() == 0) {
        d->kranimSaver->saveFrame(d->store, this->image()->projection(), d->currentFramePosition);
    }

    d->kranimSaver->saveFrame(d->store, d->currentFrame, d->currentFramePosition);

    d->image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
    connect(d->image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    d->image->setResolution(animation->resolution(), animation->resolution());

    d->currentFramePosition = frame;
    d->image->addNode(d->currentFrame.data(), d->image->rootLayer().data());

    this->updateXML();

    setCurrentImage(d->image);
}

bool KisAnimationDoc::completeSaving(KoStore *store)
{
    return true;
}

QDomDocument KisAnimationDoc::saveXML()
{
    return QDomDocument();
}

bool KisAnimationDoc::loadXML(const KoXmlDocument &doc, KoStore *store)
{
    return true;
}

bool KisAnimationDoc::completeLoading(KoStore *store)
{
    return true;
}

void KisAnimationDoc::preSaveAnimation()
{
    KisAnimation* animation = dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();

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

    this->updateXML();

    d->saved = true;
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
