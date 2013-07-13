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

#include "kis_kranim_saver.h"
#include "kis_kranim_tags.h"
#include "kis_animation_doc.h"
#include "kis_animation_part.h"

using namespace KRANIM;

struct KisKranimSaver::Private{
public:
    KisAnimationDoc* doc;
    QString animationName;
};

KisKranimSaver::KisKranimSaver(KisAnimationDoc *document) : m_d(new Private)
{
    m_d->doc = document;
    m_d->animationName = "Untitled-animation";
}

KisKranimSaver::~KisKranimSaver(){
    delete m_d;
}

QDomElement KisKranimSaver::saveXML(QDomDocument &doc, KisImageWSP image){
    QDomElement imageElement = doc.createElement("ANIMATION");

    //Q_ASSERT(image);
    imageElement.setAttribute(NAME, m_d->animationName);
    imageElement.setAttribute(MIME, NATIVE_MIMETYPE);

    return imageElement;
}

bool KisKranimSaver::saveBinaryData(KoStore *store, KisImageWSP image, const QString &uri, bool external){
    kWarning() << "Saving binary data";
    return true;
}
