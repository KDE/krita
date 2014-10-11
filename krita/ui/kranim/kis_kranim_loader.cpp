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

#include "kis_kranim_loader.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_types.h"

struct KisKranimLoader::Private
{
public:
    KisAnimationDoc* document;
    QString animationName;
};

KisKranimLoader::KisKranimLoader(KisAnimationDoc *doc)
    :m_d(new Private())
{
    m_d->document = doc;
}

KisKranimLoader::~KisKranimLoader()
{
    delete m_d;
}

void KisKranimLoader::loadBinaryData(KoStore *store, KisImageWSP image, const QString &uri, bool external)
{
    Q_UNUSED(store);
    Q_UNUSED(image);
    Q_UNUSED(uri);
    Q_UNUSED(external);
}

KisImageWSP KisKranimLoader::loadXML(const KoXmlElement &elem)
{
    Q_UNUSED(elem);

    KisImageWSP image = 0;
    return image;
}

void KisKranimLoader::loadFrame(KisNodeSP layer, KisAnimationStore *store, QString location)
{
    KisPaintDeviceSP dev = layer->paintDevice();

    if(store->hasFile(location)) {
        store->openFileReading(location);
        QIODevice* file = store->getDevice(location);
        if(!dev->read(file)) {
            dev->disconnect();
        }
        store->closeFile();

        int pixelSize = dev->colorSpace()->pixelSize();

        quint8* defPixel = new quint8[pixelSize];

        store->openFileReading(location + ".defaultpixel");
        store->readFromFile((char*)defPixel, pixelSize);
        store->closeFile();

        dev->setDefaultPixel(defPixel);
        delete[] defPixel;
    }
}
