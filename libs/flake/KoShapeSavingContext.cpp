/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>

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

#include "KoShapeSavingContext.h"
#include "KoDataCenterBase.h"

#include "KoShapeLayer.h"
#include "KoImageData.h"

#include <KoXmlWriter.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

#include <kmimetype.h>
#include <kdebug.h>
#include <QMap>

class KoShapeSavingContextPrivate {
public:
    KoShapeSavingContextPrivate(KoXmlWriter&, KoGenStyles&, KoEmbeddedDocumentSaver&);

    KoXmlWriter *xmlWriter;
    KoShapeSavingContext::ShapeSavingOptions savingOptions;
    QMap<const KoShape *, QString> drawIds;
    QMap<const QTextBlockUserData*, QString> subIds;
    QList<const KoShapeLayer*> layers;
    QSet<KoDataCenterBase *> dataCenter;
    int drawId;
    int subId;
    QMap<QString, KoSharedSavingData*> sharedData;
    QMap<qint64, QString> imageNames;
    int imageId;
    QMap<QString, QImage> images;
    QHash<const KoShape *, QTransform> shapeOffsets;

    KoGenStyles& mainStyles;
    KoEmbeddedDocumentSaver& embeddedSaver;
};

KoShapeSavingContextPrivate::KoShapeSavingContextPrivate(KoXmlWriter &w,
        KoGenStyles &s, KoEmbeddedDocumentSaver &e)
        : xmlWriter(&w),
        savingOptions(0),
        drawId(0),
        subId(0),
        imageId(0),
        mainStyles(s),
        embeddedSaver(e)
{
}

KoShapeSavingContext::KoShapeSavingContext(KoXmlWriter &xmlWriter, KoGenStyles &mainStyles,
        KoEmbeddedDocumentSaver &embeddedSaver)
    : d(new KoShapeSavingContextPrivate(xmlWriter, mainStyles, embeddedSaver))
{
    // by default allow saving of draw:id
    addOption(KoShapeSavingContext::DrawId);
}

KoShapeSavingContext::~KoShapeSavingContext()
{
}

KoXmlWriter & KoShapeSavingContext::xmlWriter()
{
    return *d->xmlWriter;
}

void KoShapeSavingContext::setXmlWriter(KoXmlWriter &xmlWriter)
{
    d->xmlWriter = &xmlWriter;
}

KoGenStyles & KoShapeSavingContext::mainStyles()
{
    return d->mainStyles;
}

KoEmbeddedDocumentSaver &KoShapeSavingContext::embeddedSaver()
{
    return d->embeddedSaver;
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

QString KoShapeSavingContext::drawId(const KoShape *shape, bool insert)
{
    QMap<const KoShape *, QString>::iterator it(d->drawIds.find(shape));
    if (it == d->drawIds.end()) {
        if (insert == true) {
            it = d->drawIds.insert(shape, QString("shape%1").arg(++d->drawId));
        } else {
            return QString();
        }
    }
    return it.value();
}

void KoShapeSavingContext::clearDrawIds()
{
    d->drawIds.clear();
    d->drawId = 0;
}

QString KoShapeSavingContext::subId(const QTextBlockUserData *subItem, bool insert)
{
    QMap<const QTextBlockUserData*, QString>::iterator it(d->subIds.find(subItem));
    if (it == d->subIds.end()) {
        if (insert == true) {
            it = d->subIds.insert(subItem, QString("subitem%1").arg(++d->subId));
        } else {
            return QString();
        }
    }
    return it.value();
}

void KoShapeSavingContext::addLayerForSaving(const KoShapeLayer *layer)
{
    if (layer && ! d->layers.contains(layer))
        d->layers.append(layer);
}

void KoShapeSavingContext::saveLayerSet(KoXmlWriter &xmlWriter) const
{
    xmlWriter.startElement("draw:layer-set");
    foreach(const KoShapeLayer * layer, d->layers) {
        xmlWriter.startElement("draw:layer");
        xmlWriter.addAttribute("draw:name", layer->name());
        if (layer->isGeometryProtected())
            xmlWriter.addAttribute("draw:protected", "true");
        if (! layer->isVisible())
            xmlWriter.addAttribute("draw:display", "none");
        xmlWriter.endElement();  // draw:layer
    }
    xmlWriter.endElement();  // draw:layer-set
}

void KoShapeSavingContext::clearLayers()
{
    d->layers.clear();
}

QString KoShapeSavingContext::imageHref(KoImageData * image)
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

QString KoShapeSavingContext::imageHref(QImage &image)
{
    // TODO this can be optimized to recocnice images which have the same content
    // Also this can use quite a lot of memeory as the qimage are all kept until
    // the they are saved to the store in memory
    QString href = QString("Pictures/image%1.png").arg(++d->imageId);
    d->images.insert(href, image);
    return href;
}

QMap<qint64, QString> KoShapeSavingContext::imagesToSave()
{
    return d->imageNames;
}

void KoShapeSavingContext::addDataCenter(KoDataCenterBase * dataCenter)
{
    d->dataCenter.insert(dataCenter);
}

bool KoShapeSavingContext::saveDataCenter(KoStore *store, KoXmlWriter* manifestWriter)
{
    bool ok = true;
    foreach(KoDataCenterBase *dataCenter, d->dataCenter) {
        ok = ok && dataCenter->completeSaving(store, manifestWriter, this);
        //kDebug() << "ok" << ok;
    }

    // Save images
    for (QMap<QString, QImage>::iterator it(d->images.begin()); it != d->images.end(); ++it) {
        if (store->open(it.key())) {
            KoStoreDevice device(store);
            ok = ok && it.value().save(&device, "PNG");
            store->close();
            // TODO error handling
            if (ok) {
                const QString mimetype(KMimeType::findByPath(it.key(), 0 , true)->name());
                manifestWriter->addManifestEntry(it.key(), mimetype);
            }
            else {
                kWarning(30006) << "saving image failed";
            }
        }
        else {
            ok = false;
            kWarning(30006) << "saving image failed: open store failed";
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
        kWarning(30006) << "The id" << id << "is already registered. Data not inserted";
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
