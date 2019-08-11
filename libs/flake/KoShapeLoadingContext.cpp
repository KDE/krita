/* This file is part of the KDE project
   Copyright (C) 2007-2009, 2011 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2014-2015 Denis Kuplyakov <dener.kup@gmail.com>

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
#include "KoImageCollection.h"
#include "KoMarkerCollection.h"
#include "KoDocumentResourceManager.h"
#include "KoLoadingShapeUpdater.h"

#include <FlakeDebug.h>

uint qHash(const KoShapeLoadingContext::AdditionalAttributeData & attributeData)
{
    return qHash(attributeData.name);
}

static QSet<KoShapeLoadingContext::AdditionalAttributeData> s_additionlAttributes;

class Q_DECL_HIDDEN KoShapeLoadingContext::Private
{
public:
    Private(KoOdfLoadingContext &c, KoDocumentResourceManager *resourceManager)
            : context(c)
            , zIndex(0)
            , documentResources(resourceManager)
            , documentRdf(0)
            , sectionModel(0)
    {
    }

    ~Private() {
        Q_FOREACH (KoSharedLoadingData * data, sharedData) {
            delete data;
        }
    }

    KoOdfLoadingContext &context;
    QMap<QString, KoShapeLayer*> layers;
    QMap<QString, KoShape*> drawIds;
    QMap<QString, QPair<KoShape *, QVariant> > subIds;
    QMap<QString, KoSharedLoadingData *> sharedData; //FIXME: use QScopedPointer here to auto delete in destructor
    int zIndex;
    QMap<QString, KoLoadingShapeUpdater*> updaterById;
    QMap<KoShape *, KoLoadingShapeUpdater*> updaterByShape;
    KoDocumentResourceManager *documentResources;
    QObject *documentRdf;
    KoSectionModel *sectionModel;
};

KoShapeLoadingContext::KoShapeLoadingContext(KoOdfLoadingContext & context, KoDocumentResourceManager *documentResources)
        : d(new Private(context, documentResources))
{
    if (d->documentResources) {
        KoMarkerCollection *markerCollection = d->documentResources->resource(KoDocumentResourceManager::MarkerCollection).value<KoMarkerCollection*>();
        if (markerCollection) {
            //markerCollection->loadOdf(*this);
        }
    }
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

KoShape * KoShapeLoadingContext::shapeById(const QString &id)
{
    return d->drawIds.value(id, 0);
}

void KoShapeLoadingContext::addShapeSubItemId(KoShape *shape, const QVariant &subItem, const QString &id)
{
    d->subIds.insert(id, QPair<KoShape *, QVariant>(shape, subItem));
}

QPair<KoShape *, QVariant> KoShapeLoadingContext::shapeSubItemById(const QString &id)
{
    return d->subIds.value(id);
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
    return d->documentResources ? d->documentResources->imageCollection() : 0;
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
        warnFlake << "The id" << id << "is already registered. Data not inserted";
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

KoDocumentResourceManager *KoShapeLoadingContext::documentResourceManager() const
{
    return d->documentResources;
}

QObject *KoShapeLoadingContext::documentRdf() const
{
    return d->documentRdf;
}


void KoShapeLoadingContext::setDocumentRdf(QObject *documentRdf)
{
    d->documentRdf = documentRdf;
}

KoSectionModel *KoShapeLoadingContext::sectionModel()
{
    return d->sectionModel;
}

void KoShapeLoadingContext::setSectionModel(KoSectionModel *sectionModel)
{
    d->sectionModel = sectionModel;
}
