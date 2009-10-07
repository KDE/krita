/* This file is part of the KDE project
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoShapeLoadingContext.h"
#include "KoShape.h"
#include "KoShapeContainer.h"
#include "KoSharedLoadingData.h"
#include "KoShapeControllerBase.h"
#include "KoDataCenter.h"
#include "KoImageCollection.h"
#include "KoLoadingShapeUpdater.h"

#include <kdebug.h>

uint qHash(const KoShapeLoadingContext::AdditionalAttributeData & attributeData)
{
    return qHash(attributeData.name);
}

static QSet<KoShapeLoadingContext::AdditionalAttributeData> s_additionlAttributes;

class KoShapeLoadingContext::Private
{
public:
    Private(KoOdfLoadingContext &c, const QMap<QString, KoDataCenter *> & dataCenterMap)
            : context(c)
            , zIndex(0)
            , dataCenterMap(dataCenterMap) {}
    ~Private() {
        foreach(KoSharedLoadingData * data, sharedData) {
            delete data;
        }
    }
    KoOdfLoadingContext &context;
    QMap<QString, KoShapeLayer*> layers;
    QMap<QString, KoShape*> drawIds;
    QMap<QString, KoSharedLoadingData*> sharedData;
    int zIndex;
    QMap<QString, KoDataCenter *> dataCenterMap;
    QMap<QString, KoLoadingShapeUpdater*> updaterById;
    QMap<KoShape *, KoLoadingShapeUpdater*> updaterByShape;
};

KoShapeLoadingContext::KoShapeLoadingContext(KoOdfLoadingContext & context, const QMap<QString, KoDataCenter *> & dataCenterMap)
        : d(new Private(context, dataCenterMap))
{
}

KoShapeLoadingContext::~KoShapeLoadingContext()
{
    delete d;
}

KoOdfLoadingContext & KoShapeLoadingContext::odfLoadingContext()
{
    return d->context;
}

KoShapeLayer * KoShapeLoadingContext::layer(const QString & layerName)
{
    return d->layers.value(layerName, 0);
}

void KoShapeLoadingContext::addLayer(KoShapeLayer * layer, const QString & layerName)
{
    d->layers[ layerName ] = layer;
}

void KoShapeLoadingContext::clearLayers()
{
    d->layers.clear();
}

void KoShapeLoadingContext::addShapeId(KoShape * shape, const QString & id)
{
    d->drawIds.insert(id, shape);
    QMap<QString, KoLoadingShapeUpdater*>::iterator it(d->updaterById.find(id));
    while (it != d->updaterById.end() && it.key() == id) {
        d->updaterByShape.insertMulti(shape, it.value());
        it = d->updaterById.erase(it);
    }
}

KoShape * KoShapeLoadingContext::shapeById(const QString & id)
{
    return d->drawIds.value(id, 0);
}

// TODO make sure to remove the shape from the loading context when loading for it failed and it was deleted. This can also happen when the parent is deleted
void KoShapeLoadingContext::updateShape(const QString & id, KoLoadingShapeUpdater * shapeUpdater)
{
    d->updaterById.insertMulti(id, shapeUpdater);
}

void KoShapeLoadingContext::shapeLoaded(KoShape * shape)
{
    QMap<KoShape*, KoLoadingShapeUpdater*>::iterator it(d->updaterByShape.find(shape));
    while (it != d->updaterByShape.end() && it.key() == shape) {
        it.value()->update(shape);
        delete it.value();
        it = d->updaterByShape.erase(it);
    }
}

KoImageCollection * KoShapeLoadingContext::imageCollection()
{
    return dynamic_cast<KoImageCollection*>(d->dataCenterMap.value("ImageCollection", 0));
}

int KoShapeLoadingContext::zIndex()
{
    return d->zIndex++;
}

void KoShapeLoadingContext::setZIndex(int index)
{
    d->zIndex = index;
}

void KoShapeLoadingContext::addSharedData(const QString & id, KoSharedLoadingData * data)
{
    QMap<QString, KoSharedLoadingData*>::iterator it(d->sharedData.find(id));
    // data will not be overwritten
    if (it == d->sharedData.end()) {
        d->sharedData.insert(id, data);
    } else {
        kWarning(30006) << "The id" << id << "is already registered. Data not inserted";
        Q_ASSERT(it == d->sharedData.end());
    }
}

KoSharedLoadingData * KoShapeLoadingContext::sharedData(const QString & id) const
{
    KoSharedLoadingData * data = 0;
    QMap<QString, KoSharedLoadingData*>::const_iterator it(d->sharedData.find(id));
    if (it != d->sharedData.constEnd()) {
        data = it.value();
    }
    return data;
}

void KoShapeLoadingContext::addAdditionalAttributeData(const AdditionalAttributeData & attributeData)
{
    s_additionlAttributes.insert(attributeData);
}

QSet<KoShapeLoadingContext::AdditionalAttributeData> KoShapeLoadingContext::additionalAttributeData()
{
    return s_additionlAttributes;
}

KoDataCenter * KoShapeLoadingContext::dataCenter(const QString & dataCenterName)
{
    return d->dataCenterMap.value(dataCenterName, 0);
}

QMap<QString, KoDataCenter *> KoShapeLoadingContext::dataCenterMap() const
{
    return d->dataCenterMap;
}
