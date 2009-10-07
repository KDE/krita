/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoShapeSavingContext.h"
#include "KoShapeLayer.h"
#include "KoDataCenter.h"
#include "KoImageData.h"

#include <KoGenStyles.h>
#include <KoXmlWriter.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

#include <kmimetype.h>

#include <QtCore/QTime>
#include <kdebug.h>

KoShapeSavingContext::KoShapeSavingContext(KoXmlWriter &xmlWriter, KoGenStyles& mainStyles,
        KoEmbeddedDocumentSaver& embeddedSaver, SavingMode savingMode)
        : m_xmlWriter(&xmlWriter)
        , m_savingOptions(0)
        , m_drawId(0)
        , m_imageId(0)
        , m_mainStyles(mainStyles)
        , m_embeddedSaver(embeddedSaver)
        , m_savingMode(savingMode)
{
    // by default allow saving of draw:id
    addOption(KoShapeSavingContext::DrawId);
}

KoShapeSavingContext::~KoShapeSavingContext()
{
}

KoXmlWriter & KoShapeSavingContext::xmlWriter()
{
    return *m_xmlWriter;
}

void KoShapeSavingContext::setXmlWriter(KoXmlWriter &_xmlWriter)
{
    m_xmlWriter = &_xmlWriter;
}

KoGenStyles & KoShapeSavingContext::mainStyles()
{
    return m_mainStyles;
}

KoEmbeddedDocumentSaver & KoShapeSavingContext::embeddedSaver()
{
    return m_embeddedSaver;
}

bool KoShapeSavingContext::isSet(ShapeSavingOption option) const
{
    return m_savingOptions & option;
}

void KoShapeSavingContext::setOptions(KoShapeSavingOptions options)
{
    m_savingOptions = options;
}

KoShapeSavingContext::KoShapeSavingOptions KoShapeSavingContext::options() const
{
    return m_savingOptions;
}

void KoShapeSavingContext::addOption(ShapeSavingOption option)
{
    m_savingOptions = m_savingOptions | option;
}

void KoShapeSavingContext::removeOption(ShapeSavingOption option)
{
    if (isSet(option))
        m_savingOptions = m_savingOptions ^ option; // xor to remove it.
}

const QString KoShapeSavingContext::drawId(const KoShape * shape, bool insert)
{
    QMap<const KoShape *, QString>::iterator it(m_drawIds.find(shape));
    if (it == m_drawIds.end()) {
        if (insert == true) {
            it = m_drawIds.insert(shape, QString("shape%1").arg(++m_drawId));
        } else {
            return QString();
        }
    }
    return it.value();
}

void KoShapeSavingContext::clearDrawIds()
{
    m_drawIds.clear();
    m_drawId = 0;
}

void KoShapeSavingContext::addLayerForSaving(const KoShapeLayer * layer)
{
    if (layer && ! m_layers.contains(layer))
        m_layers.append(layer);
}

void KoShapeSavingContext::saveLayerSet(KoXmlWriter & xmlWriter) const
{
    xmlWriter.startElement("draw:layer-set");
    foreach(const KoShapeLayer * layer, m_layers) {
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
    m_layers.clear();
}

QString KoShapeSavingContext::imageHref(KoImageData * image)
{
    QMap<qint64, QString>::iterator it(m_imageNames.find(image->key()));
    if (it == m_imageNames.end()) {
        QString suffix = image->suffix();
        if ( suffix.isEmpty() ) {
            it = m_imageNames.insert(image->key(), QString("Pictures/image%1").arg(++m_imageId));
        }
        else {
            it = m_imageNames.insert(image->key(), QString("Pictures/image%1.%2").arg(++m_imageId).arg(suffix));
        }
    }
    return it.value();
}

QString KoShapeSavingContext::imageHref(QImage & image)
{
    // TODO this can be optimized to recocnice images which have the same content
    // Also this can use quite a lot of memeory as the qimage are all kept until
    // the they are saved to the store in memory
    QString href = QString("Pictures/image%1.png").arg(++m_imageId);
    m_images.insert(href, image);
    return href;
}

QMap<qint64, QString> KoShapeSavingContext::imagesToSave()
{
    return m_imageNames;
}

void KoShapeSavingContext::addDataCenter(KoDataCenter * dataCenter)
{
    m_dataCenter.insert(dataCenter);
}

bool KoShapeSavingContext::saveDataCenter(KoStore *store, KoXmlWriter* manifestWriter)
{
    bool ok = true;
    foreach(KoDataCenter *dataCenter, m_dataCenter) {
        ok = ok && dataCenter->completeSaving(store, manifestWriter, this);
        //kDebug() << "ok" << ok;
    }
    for ( QMap<QString, QImage>::iterator it(m_images.begin()); it != m_images.end(); ++it ) {
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

void KoShapeSavingContext::addSharedData(const QString & id, KoSharedSavingData * data)
{
    QMap<QString, KoSharedSavingData*>::iterator it(m_sharedData.find(id));
    // data will not be overwritten
    if (it == m_sharedData.end()) {
        m_sharedData.insert(id, data);
    } else {
        kWarning(30006) << "The id" << id << "is already registered. Data not inserted";
        Q_ASSERT(it == m_sharedData.end());
    }
}

KoSharedSavingData * KoShapeSavingContext::sharedData(const QString & id) const
{
    KoSharedSavingData * data = 0;
    QMap<QString, KoSharedSavingData*>::const_iterator it(m_sharedData.find(id));
    if (it != m_sharedData.end()) {
        data = it.value();
    }
    return data;
}

void KoShapeSavingContext::addShapeOffset(const KoShape * shape, const QMatrix & m)
{
    m_shapeOffsets.insert(shape, m);
}

void KoShapeSavingContext::removeShapeOffset(const KoShape * shape)
{
    m_shapeOffsets.remove(shape);
}

QMatrix KoShapeSavingContext::shapeOffset(const KoShape * shape) const
{
    return m_shapeOffsets.value(shape, QMatrix());
}
