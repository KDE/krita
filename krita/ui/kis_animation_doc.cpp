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

#define APP_MIMETYPE "application/x-krita-animation"
static const char CURRENT_DTD_VERSION[] = "1.0";


class KisAnimationDoc::KisAnimationDocPrivate{
public:
    KisAnimationDocPrivate()
        :kranimSaver(0),
          kranimLoader(0)
    {
    }

    ~KisAnimationDocPrivate(){

    }

    KisKranimSaver* kranimSaver;
    KisKranimLoader* kranimLoader;
};

KisAnimationDoc::KisAnimationDoc()
    : KisDoc2(new KisAnimationPart),
      m_d_anim(new KisAnimationDocPrivate())
{
    setMimeType(APP_MIMETYPE);

    m_d_anim->kranimSaver = 0;
    m_d_anim->kranimLoader = 0;
}

KisAnimationDoc::~KisAnimationDoc(){

}

void KisAnimationDoc::addFrame()
{

    KisAnimation* animation = dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();
    KisImageWSP image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
    connect(image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    image->setResolution(animation->resolution(), animation->resolution());

    KisPaintLayerSP layer = new KisPaintLayer(image.data(), image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());

    layer->paintDevice()->setDefaultPixel(animation->bgColor().data());
    image->addNode(layer.data(), image->rootLayer().data());
    setCurrentImage(image);
}

bool KisAnimationDoc::completeSaving(KoStore *store)
{
    qDebug() << "completeSaving called";

    QString uri = url().url();

    m_d_anim->kranimSaver->saveBinaryData(store, this->image(), uri, isStoredExtern());
    delete m_d_anim->kranimSaver;
    m_d_anim->kranimSaver = 0;

    return true;
}

QDomDocument KisAnimationDoc::saveXML()
{
    qDebug() << "saveXML called";

    QDomDocument doc = createDomDocument("animation", CURRENT_DTD_VERSION);

    QDomElement root = doc.documentElement();

    root.setAttribute("editor","Krita Animation");

    root.setAttribute("syntaxVersion", "1");

    if(!m_d_anim->kranimSaver){
        m_d_anim->kranimSaver = 0;
    }

    m_d_anim->kranimSaver = new KisKranimSaver(this);
    root.appendChild(m_d_anim->kranimSaver->saveMetaData(doc));
    root.appendChild(m_d_anim->kranimSaver->saveXML(doc, this->image()));

    return doc;
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
    qDebug() << "completeLoading called";
    m_d_anim->kranimLoader = new KisKranimLoader(this);
    return true;
}
#include "kis_animation_doc.moc"
