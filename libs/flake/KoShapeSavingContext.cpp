/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2004-2006 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2007-2009, 2011 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
   SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoShapeSavingContext.h"

#include "KoShapeLayer.h"
#include "KoMarker.h"

#include <KoXmlWriter.h>
#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoSharedSavingData.h>

#include <FlakeDebug.h>
#include <QUuid>
#include <QImage>
#include <KisMimeDatabase.h>

class KoShapeSavingContextPrivate {
public:
    KoShapeSavingContextPrivate(KoXmlWriter&);
    ~KoShapeSavingContextPrivate();

    KoXmlWriter *xmlWriter;
    KoShapeSavingContext::ShapeSavingOptions savingOptions;

    QList<const KoShapeLayer*> layers;
    QMap<QString, KoSharedSavingData*> sharedData;

    QMap<qint64, QString> imageNames;
    int imageId;
    QMap<QString, QImage> images;

    QHash<const KoShape *, QTransform> shapeOffsets;
    QMap<const KoMarker *, QString> markerRefs;

    QMap<QString, int> referenceCounters;
    QMap<QString, QList<const void*> > prefixedReferences;

};

KoShapeSavingContextPrivate::KoShapeSavingContextPrivate(KoXmlWriter &w)
    : xmlWriter(&w)
    , imageId(0)
{
}

KoShapeSavingContextPrivate::~KoShapeSavingContextPrivate()
{
    Q_FOREACH (KoSharedSavingData * data, sharedData) {
        delete data;
    }
}

KoShapeSavingContext::KoShapeSavingContext(KoXmlWriter &xmlWriter)
    : d(new KoShapeSavingContextPrivate(xmlWriter))
{
    // by default allow saving of draw:id + xml:id
    addOption(KoShapeSavingContext::DrawId);
}

KoShapeSavingContext::~KoShapeSavingContext()
{
    delete d;
}

KoXmlWriter & KoShapeSavingContext::xmlWriter()
{
    return *d->xmlWriter;
}

void KoShapeSavingContext::setXmlWriter(KoXmlWriter &xmlWriter)
{
    d->xmlWriter = &xmlWriter;
}

bool KoShapeSavingContext::isSet(ShapeSavingOption option) const
{
    return d->savingOptions & option;
}

void KoShapeSavingContext::setOptions(ShapeSavingOptions options)
{
    d->savingOptions = options;
}

KoShapeSavingContext::ShapeSavingOptions KoShapeSavingContext::options() const
{
    return d->savingOptions;
}

void KoShapeSavingContext::addOption(ShapeSavingOption option)
{
    d->savingOptions = d->savingOptions | option;
}

void KoShapeSavingContext::removeOption(ShapeSavingOption option)
{
    if (isSet(option))
        d->savingOptions = d->savingOptions ^ option; // xor to remove it.
}

void KoShapeSavingContext::addLayerForSaving(const KoShapeLayer *layer)
{
    if (layer && ! d->layers.contains(layer))
        d->layers.append(layer);
}

void KoShapeSavingContext::saveLayerSet(KoXmlWriter &xmlWriter) const
{
    xmlWriter.startElement("draw:layer-set");
    Q_FOREACH (const KoShapeLayer * layer, d->layers) {
        xmlWriter.startElement("draw:layer");
        xmlWriter.addAttribute("draw:name", layer->name());
        if (layer->isGeometryProtected())
            xmlWriter.addAttribute("draw:protected", "true");
        if (! layer->isVisible(false))
            xmlWriter.addAttribute("draw:display", "none");
        xmlWriter.endElement();  // draw:layer
    }
    xmlWriter.endElement();  // draw:layer-set
}

void KoShapeSavingContext::clearLayers()
{
    d->layers.clear();
}

QMap<qint64, QString> KoShapeSavingContext::imagesToSave()
{
    return d->imageNames;
}

QString KoShapeSavingContext::markerRef(const KoMarker */*marker*/)
{
    return QString();
}

void KoShapeSavingContext::addSharedData(const QString &id, KoSharedSavingData * data)
{
    QMap<QString, KoSharedSavingData*>::iterator it(d->sharedData.find(id));
    // data will not be overwritten
    if (it == d->sharedData.end()) {
        d->sharedData.insert(id, data);
    } else {
        warnFlake << "The id" << id << "is already registered. Data not inserted";
        Q_ASSERT(it == d->sharedData.end());
    }
}

KoSharedSavingData * KoShapeSavingContext::sharedData(const QString &id) const
{
    KoSharedSavingData * data = 0;
    QMap<QString, KoSharedSavingData*>::const_iterator it(d->sharedData.constFind(id));
    if (it != d->sharedData.constEnd()) {
        data = it.value();
    }
    return data;
}

void KoShapeSavingContext::addShapeOffset(const KoShape *shape, const QTransform &m)
{
    d->shapeOffsets.insert(shape, m);
}

void KoShapeSavingContext::removeShapeOffset(const KoShape *shape)
{
    d->shapeOffsets.remove(shape);
}

QTransform KoShapeSavingContext::shapeOffset(const KoShape *shape) const
{
    return d->shapeOffsets.value(shape, QTransform());
}
