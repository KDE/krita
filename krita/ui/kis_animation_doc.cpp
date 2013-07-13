/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

#define APP_MIMETYPE "application/x-krita-animation"

KisAnimationDoc::KisAnimationDoc() : KisDoc2(new KisAnimationPart)
{

}

KisAnimationDoc::~KisAnimationDoc(){

}

QByteArray KisAnimationDoc::mimeType() const{
    return APP_MIMETYPE;
}

void KisAnimationDoc::addFrame(){

    KisAnimation* animation = dynamic_cast<KisAnimationPart*>(this->documentPart())->animation();
    KisImageWSP image = new KisImage(createUndoStore(), animation->width(), animation->height(), animation->colorSpace(), animation->name());
    connect(image.data(), SIGNAL(sigImageModified()), this, SLOT(setImageModified()));
    image->setResolution(animation->resolution(), animation->resolution());

    KisPaintLayerSP layer = new KisPaintLayer(image.data(), image->nextLayerName(), animation->bgColor().opacityU8(), animation->colorSpace());

    layer->paintDevice()->setDefaultPixel(animation->bgColor().data());
    image->addNode(layer.data(), image->rootLayer().data());
    setCurrentImage(image);
}

bool KisAnimationDoc::completeLoading(KoStore *store)
{
    qDebug() << "completeLoading called";
    return true;
}

bool KisAnimationDoc::completeSaving(KoStore *)
{
    qDebug() << "completeSaving called";
    return true;
}

QDomDocument KisAnimationDoc::saveXML()
{
    qDebug() << "saveXML called";
    return QDomDocument();

}

bool KisAnimationDoc::loadXML(const KoXmlDocument &doc, KoStore *store)
{
    qDebug() << "loadXML called";
    return true;

}

#include "kis_animation_doc.moc"
