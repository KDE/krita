/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2004-2006 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2007-2009, 2011 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
   SPDX-FileCopyrightText: 2010 Benjamin Port <port.benjamin@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoShapeSavingContext.h"
#include "KoDataCenterBase.h"

#include "KoShapeLayer.h"
#include "KoImageData.h"
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
    QSet<KoDataCenterBase *> dataCenters;
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

QString KoShapeSavingContext::imageHref(const KoImageData *image)
{
    QMap<qint64, QString>::iterator it(d->imageNames.find(image->key()));
    if (it == d->imageNames.end()) {
        QString suffix = image->suffix();
        if (suffix.isEmpty()) {
            it = d->imageNames.insert(image->key(), QString("Pictures/image%1").arg(++d->imageId));
        }
        else {
            it = d->imageNames.insert(image->key(), QString("Pictures/image%1.%2").arg(++d->imageId).arg(suffix));
        }
    }
    return it.value();
}

QString KoShapeSavingContext::imageHref(const QImage &image)
{
    // TODO this can be optimized to recognize images which have the same content
    // Also this can use quite a lot of memory as the qimage are all kept until
    // they are saved to the store in memory
    QString href = QString("Pictures/image%1.png").arg(++d->imageId);
    d->images.insert(href, image);
    return href;
}

QMap<qint64, QString> KoShapeSavingContext::imagesToSave()
{
    return d->imageNames;
}

QString KoShapeSavingContext::markerRef(const KoMarker */*marker*/)
{
    return QString();
}

void KoShapeSavingContext::addDataCenter(KoDataCenterBase * dataCenter)
{
    if (dataCenter) {
        d->dataCenters.insert(dataCenter);
    }
}

bool KoShapeSavingContext::saveDataCenter(KoStore *store, KoXmlWriter* manifestWriter)
{
    bool ok = true;
    Q_FOREACH (KoDataCenterBase *dataCenter, d->dataCenters) {
        ok = ok && dataCenter->completeSaving(store, manifestWriter, this);
        //debugFlake << "ok" << ok;
    }

    // Save images
    for (QMap<QString, QImage>::iterator it(d->images.begin()); it != d->images.end(); ++it) {
        if (store->open(it.key())) {
            KoStoreDevice device(store);
            ok = ok && it.value().save(&device, "PNG");
            store->close();
            // TODO error handling
            if (ok) {
                const QString mimetype = KisMimeDatabase::mimeTypeForFile(it.key(), false);
                manifestWriter->addManifestEntry(it.key(), mimetype);
            }
            else {
                warnFlake << "saving image failed";
            }
        }
        else {
            ok = false;
            warnFlake << "saving image failed: open store failed";
        }
    }
    return ok;
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
