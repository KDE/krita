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
    KisLayerSP newFrame;
    QRect newFramePosition;
    bool saved;
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
    kWarning() << frame;
    if (!d->saved) {
        kWarning() << this->documentPart()->url();
        d->kranimSaver = new KisKranimSaver(this);
        this->preSaveAnimation();
    }
}

void KisAnimationDoc::addBlankFrame(QRect frame)
{

    //Save the previous frame over here
    this->documentPart()->save();

    KisAnimation* animation = dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();

    KisImageWSP image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
    connect(image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    image->setResolution(animation->resolution(), animation->resolution());

    d->newFramePosition = frame;
    d->newFrame = new KisPaintLayer(image.data(), image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());
    d->newFrame->setName("testFrame");
    d->newFrame->paintDevice()->setDefaultPixel(animation->bgColor().data());
    image->addNode(d->newFrame/*.data()*/, image->rootLayer().data());

    //Load all the layers here

    setCurrentImage(image);
}

void KisAnimationDoc::addKeyFrame(QRect frame)
{
    kWarning() << "Adding keyframe";
}

bool KisAnimationDoc::completeSaving(KoStore *store)
{
    QString uri = url().url();

    if(!d->saved) {
        d->kranimSaver->saveBinaryData(store, this->image(), uri, isStoredExtern());
    }
    else {
        QDomElement e = d->doc.createElement("frame");
        e.setAttribute("number", d->newFramePosition.x());
        e.setAttribute("layer", d->newFramePosition.y());
        d->root.appendChild(e);
        d->kranimSaver->saveFrame(store, d->newFrame, d->newFramePosition);
    }

    return true;
}

QDomDocument KisAnimationDoc::saveXML()
{
    if (!d->saved) {
        d->doc = createDomDocument("animation", CURRENT_DTD_VERSION);

        d->root = d->doc.documentElement();

        d->root.setAttribute("editor","Krita Animation");

        d->root.setAttribute("syntaxVersion", "1");

        d->root.appendChild(d->kranimSaver->saveMetaData(d->doc));
        //m_d_anim->root.appendChild(m_d_anim->kranimSaver->saveXML(m_d_anim->doc, this->image()));
    }

    return d->doc;
}

bool KisAnimationDoc::loadXML(const KoXmlDocument &doc, KoStore *store)
{
    KoXmlElement root;
    QString attr;
    KoXmlNode node;
    return true;
}

bool KisAnimationDoc::completeLoading(KoStore *store)
{
    d->kranimLoader = new KisKranimLoader(this);
    return true;
}

void KisAnimationDoc::preSaveAnimation()
{
    KisAnimation* animation = dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();

    KUrl url = this->documentPart()->url();

    QByteArray nativeFormat = this->nativeFormatMimeType();
    QStringList mimeFilter = KoFilterManager::mimeFilter(nativeFormat, KoFilterManager::Export,
                                                         this->extraNativeMimeTypes(KoDocument::ForExport));
    kWarning() << nativeFormat;
    if(!mimeFilter.contains(nativeFormat)) {
        kWarning() << "No output filter";
    }
    this->setOutputMimeType(nativeFormat, 0);

    KUrl newUrl(url.directory() + "/" + animation->name() + ".kranim");

    d->saved = this->documentPart()->saveAs(newUrl);
}

#include "kis_animation_doc.moc"
