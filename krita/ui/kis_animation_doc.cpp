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
};

KisAnimationDoc::KisAnimationDoc()
    : KisDoc2(new KisAnimationPart),
      d(new KisAnimationDocPrivate())
{
    setMimeType(APP_MIMETYPE);
    d->kranimLoader = 0;
    d->saved = false;
}

KisAnimationDoc::~KisAnimationDoc()
{
    delete d;
}

void KisAnimationDoc::frameSelectionChanged(QRect frame)
{
    KisAnimation* animation = dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();

    if (!d->saved) {
        d->kranimSaver = new KisKranimSaver(this);
        this->preSaveAnimation();
        return;
    }

    QString location = "frame" + QString::number(frame.x()) +"layer" + QString::number(frame.y());

    kWarning() << location << d->store->hasFile(location);

    if(d->store->hasFile(location)) {
        d->kranimSaver->saveFrame(d->store, d->currentFrame, d->currentFramePosition);

        KisImageWSP image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
        connect(image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
        image->setResolution(animation->resolution(), animation->resolution());

        d->currentFramePosition = frame;
        d->currentFrame = new KisPaintLayer(image.data(), image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
        d->currentFrame->setName("testFrame");
        d->currentFrame->paintDevice()->setDefaultPixel(animation->bgColor().data());
        image->addNode(d->currentFrame.data(), image->rootLayer().data());

        //Load all the layers here

        d->kranimLoader->loadFrame(d->currentFrame, d->store, d->currentFramePosition);

        setCurrentImage(image);
    }
}

void KisAnimationDoc::addBlankFrame(QRect frame)
{

    KisAnimation* animation = dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();

    d->kranimSaver->saveFrame(d->store, d->currentFrame, d->currentFramePosition);

    KisImageWSP image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
    connect(image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    image->setResolution(animation->resolution(), animation->resolution());

    d->currentFramePosition = frame;
    d->currentFrame = new KisPaintLayer(image.data(), image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
    d->currentFrame->setName("testFrame");
    d->currentFrame->paintDevice()->setDefaultPixel(animation->bgColor().data());
    image->addNode(d->currentFrame.data(), image->rootLayer().data());
    kWarning() << "Layer added";
    //Load all the layers here

    setCurrentImage(image);
}

void KisAnimationDoc::addKeyFrame(QRect frame)
{

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

    KUrl url = this->documentPart()->url();

    d->store = new KisAnimationStore(url.directory() + "/" + animation->name() + ".kranim");

    d->doc = this->createDomDocument("krita-animation", "1.0");

    d->kranimSaver->saveMetaData(d->doc);

    d->store->openStore();
    d->store->openFileWriting("maindoc.xml");

    d->store->writeDataToFile(d->doc.toByteArray());

    d->store->closeFileWriting();
    d->store->closeStore();

    d->saved = true;

}

#include "kis_animation_doc.moc"
