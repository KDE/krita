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

#include "kis_kranim_loader.h"
#include <kis_image.h>

struct KisKranimLoader::Private{
public:
    KisAnimationDoc* document;
    QString animationName;
};

KisKranimLoader::KisKranimLoader(KisAnimationDoc *doc)
    :m_d(new Private())
{
    m_d->document = doc;
}

KisKranimLoader::~KisKranimLoader(){
    delete m_d;
}

void KisKranimLoader::loadBinaryData(KoStore *store, KisImageWSP image, const QString &uri, bool external){
    kWarning() << "Load binary data";
}

KisImageWSP KisKranimLoader::loadXML(const KoXmlElement &elem){
    kWarning() << "Load XML";
    KisImageWSP image = 0;
    return image;
}
