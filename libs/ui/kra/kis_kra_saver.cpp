/* This file is part of the KDE project
 * Copyright 2008 (C) Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kis_kra_saver.h"

#include "kis_kra_tags.h"
#include "kis_kra_save_visitor.h"
#include "kis_kra_savexml_visitor.h"

#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <QStringList>

#include <QUrl>
#include <QBuffer>

#include <KoDocumentInfo.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoColor.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

#include <kis_annotation.h>
#include <kis_image.h>
#include <kis_image_animation_interface.h>
#include <kis_group_layer.h>
#include <kis_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_layer_composition.h>
#include <kis_painting_assistants_decoration.h>
#include <kis_psd_layer_style_resource.h>
#include "kis_png_converter.h"
#include "kis_keyframe_channel.h"
#include <kis_time_range.h>
#include "KisDocument.h"
#include <string>
#include "kis_dom_utils.h"
#include "kis_grid_config.h"
#include "kis_guides_config.h"


using namespace KRA;

struct KisKraSaver::Private
{
public:
    KisDocument* doc;
    QMap<const KisNode*, QString> nodeFileNames;
    QMap<const KisNode*, QString> keyframeFilenames;
    QString imageName;
    QStringList errorMessages;
};

KisKraSaver::KisKraSaver(KisDocument* document)
        : m_d(new Private)
{
    m_d->doc = document;

    m_d->imageName = m_d->doc->documentInfo()->aboutInfo("title");
    if (m_d->imageName.isEmpty()) {
        m_d->imageName = i18n("Unnamed");
    }
}

KisKraSaver::~KisKraSaver()
{
    delete m_d;
}

QDomElement KisKraSaver::saveXML(QDomDocument& doc,  KisImageWSP image)
{
    QDomElement imageElement = doc.createElement("IMAGE"); // Legacy!

    Q_ASSERT(image);
    imageElement.setAttribute(NAME, m_d->imageName);
    imageElement.setAttribute(MIME, NATIVE_MIMETYPE);
    imageElement.setAttribute(WIDTH, image->width());
    imageElement.setAttribute(HEIGHT, image->height());
    imageElement.setAttribute(COLORSPACE_NAME, image->colorSpace()->id());
    imageElement.setAttribute(DESCRIPTION, m_d->doc->documentInfo()->aboutInfo("comment"));
    // XXX: Save profile as blob inside the image, instead of the product name.
    if (image->profile() && image->profile()-> valid()) {
        imageElement.setAttribute(PROFILE, image->profile()->name());
    }
    imageElement.setAttribute(X_RESOLUTION, image->xRes()*72.0);
    imageElement.setAttribute(Y_RESOLUTION, image->yRes()*72.0);

    quint32 count = 1; // We don't save the root layer, but it does count
    KisSaveXmlVisitor visitor(doc, imageElement, count, m_d->doc->url().toLocalFile(), true);
    visitor.setSelectedNodes(m_d->doc->activeNodes());

    image->rootLayer()->accept(visitor);
    m_d->errorMessages.append(visitor.errorMessages());

    m_d->nodeFileNames = visitor.nodeFileNames();
    m_d->keyframeFilenames = visitor.keyframeFileNames();

    saveBackgroundColor(doc, imageElement, image);
    saveCompositions(doc, imageElement, image);
    saveAssistantsList(doc,imageElement);
    saveGrid(doc,imageElement);
    saveGuides(doc,imageElement);

    QDomElement animationElement = doc.createElement("animation");
    KisDomUtils::saveValue(&animationElement, "framerate", image->animationInterface()->framerate());
    KisDomUtils::saveValue(&animationElement, "range", image->animationInterface()->fullClipRange());
    KisDomUtils::saveValue(&animationElement, "currentTime", image->animationInterface()->currentUITime());
    imageElement.appendChild(animationElement);

    return imageElement;
}

bool KisKraSaver::saveKeyframes(KoStore *store, const QString &uri, bool external)
{
    QMap<const KisNode*, QString>::iterator it;

    for (it = m_d->keyframeFilenames.begin(); it != m_d->keyframeFilenames.end(); it++) {
        const KisNode *node = it.key();
        QString filename = it.value();

        QString location =
                (external ? QString() : uri)
                + m_d->imageName + LAYER_PATH + filename;

        if (!saveNodeKeyframes(store, location, node)) {
            return false;
        }
    }

    return true;
}

bool KisKraSaver::saveNodeKeyframes(KoStore *store, QString location, const KisNode *node)
{
    QDomDocument doc = KisDocument::createDomDocument("krita-keyframes", "keyframes", "1.0");
    QDomElement root = doc.documentElement();

    KisKeyframeChannel *channel;
    Q_FOREACH (channel, node->keyframeChannels()) {
        QDomElement element = channel->toXML(doc, m_d->nodeFileNames[node]);
        root.appendChild(element);
    }

    if (store->open(location)) {
        QByteArray xml = doc.toByteArray();
        store->write(xml);
        store->close();
    } else {
        m_d->errorMessages << i18n("could not save keyframes");
        return false;
    }

    return true;
}

bool KisKraSaver::saveBinaryData(KoStore* store, KisImageWSP image, const QString & uri, bool external, bool autosave)
{
    QString location;

    // Save the layers data
    KisKraSaveVisitor visitor(store, m_d->imageName, m_d->nodeFileNames);

    if (external)
        visitor.setExternalUri(uri);

    image->rootLayer()->accept(visitor);

    m_d->errorMessages.append(visitor.errorMessages());
    if (!m_d->errorMessages.isEmpty()) {
        return false;
    }

    // saving annotations
    // XXX this only saves EXIF and ICC info. This would probably need
    // a redesign of the dtd of the krita file to do this more generally correct
    // e.g. have <ANNOTATION> tags or so.
    KisAnnotationSP annotation = image->annotation("exif");
    if (annotation) {
        location = external ? QString() : uri;
        location += m_d->imageName + EXIF_PATH;
        if (store->open(location)) {
            store->write(annotation->annotation());
            store->close();
        }
    }
    if (image->profile()) {
        const KoColorProfile *profile = image->profile();
        KisAnnotationSP annotation;
        if (profile) {
            QByteArray profileRawData = profile->rawData();
            if (!profileRawData.isEmpty()) {
                if (profile->type() == "icc") {
                    annotation = new KisAnnotation(ICC, profile->name(), profile->rawData());
                } else {
                    annotation = new KisAnnotation(PROFILE, profile->name(), profile->rawData());
                }
            }
        }

        if (annotation) {
            location = external ? QString() : uri;
            location += m_d->imageName + ICC_PATH;
            if (store->open(location)) {
                store->write(annotation->annotation());
                store->close();
            }
        }
    }

    {
        KisPSDLayerStyleCollectionResource collection("not-nexists.asl");
        KIS_ASSERT_RECOVER_NOOP(!collection.valid());
        collection.collectAllLayerStyles(image->root());
        if (collection.valid()) {
            location = external ? QString() : uri;
            location += m_d->imageName + LAYER_STYLES_PATH;

            if (store->open(location)) {
                QBuffer aslBuffer;
                aslBuffer.open(QIODevice::WriteOnly);
                collection.saveToDevice(&aslBuffer);
                aslBuffer.close();

                store->write(aslBuffer.buffer());
                store->close();
            }
        }
    }

    if (!autosave) {
        KisPaintDeviceSP dev = image->projection();
        if (!KisPNGConverter::isColorSpaceSupported(dev->colorSpace())) {
            dev = new KisPaintDevice(*dev.data());
            KUndo2Command *cmd = dev->convertTo(KoColorSpaceRegistry::instance()->rgb8());
            delete cmd;
        }

        KisPNGConverter::saveDeviceToStore("mergedimage.png", image->bounds(), image->xRes(), image->yRes(), dev, store);
    }

    saveAssistants(store, uri,external);
    return true;
}

QStringList KisKraSaver::errorMessages() const
{
    return m_d->errorMessages;
}

void KisKraSaver::saveBackgroundColor(QDomDocument& doc, QDomElement& element, KisImageWSP image)
{
    QDomElement e = doc.createElement("ProjectionBackgroundColor");
    KoColor color = image->defaultProjectionColor();
    QByteArray colorData = QByteArray::fromRawData((const char*)color.data(), color.colorSpace()->pixelSize());
    e.setAttribute("ColorData", QString(colorData.toBase64()));
    element.appendChild(e);
}

void KisKraSaver::saveCompositions(QDomDocument& doc, QDomElement& element, KisImageWSP image)
{
    if (!image->compositions().isEmpty()) {
        QDomElement e = doc.createElement("compositions");
        Q_FOREACH (KisLayerComposition* composition, image->compositions()) {
            composition->save(doc, e);
        }
        element.appendChild(e);
    }
}

bool KisKraSaver::saveAssistants(KoStore* store, QString uri, bool external)
{
    QString location;
    QMap<QString, int> assistantcounters;
    QByteArray data;
    QList<KisPaintingAssistant*> assistants =  m_d->doc->assistants();
    QMap<KisPaintingAssistantHandleSP, int> handlemap;
    if (!assistants.isEmpty()) {
        Q_FOREACH (KisPaintingAssistant* assist, assistants){
            if (!assistantcounters.contains(assist->id())){
                assistantcounters.insert(assist->id(),0);
            }
            location = external ? QString() : uri;
            location += m_d->imageName + ASSISTANTS_PATH;
            location += QString(assist->id()+"%1.assistant").arg(assistantcounters[assist->id()]);
            data = assist->saveXml(handlemap);
            store->open(location);
            store->write(data);
            store->close();
            assistantcounters[assist->id()]++;
        }

    }
    return true;
}

bool KisKraSaver::saveAssistantsList(QDomDocument& doc, QDomElement& element)
{
    int count_ellipse = 0, count_perspective = 0, count_ruler = 0, count_vanishingpoint = 0,count_infiniteruler = 0, count_parallelruler = 0, count_concentricellipse = 0, count_fisheyepoint = 0, count_spline = 0;
    QList<KisPaintingAssistant*> assistants =  m_d->doc->assistants();
    if (!assistants.isEmpty()) {
        QDomElement assistantsElement = doc.createElement("assistants");
        Q_FOREACH (KisPaintingAssistant* assist, assistants){
            if (assist->id() == "ellipse"){
                assist->saveXmlList(doc, assistantsElement, count_ellipse);
                count_ellipse++;
            }
            else if (assist->id() == "spline"){
                assist->saveXmlList(doc, assistantsElement, count_spline);
                count_spline++;
            }
            else if (assist->id() == "perspective"){
                assist->saveXmlList(doc, assistantsElement, count_perspective);
                count_perspective++;
            }
            else if (assist->id() == "vanishing point"){
                assist->saveXmlList(doc, assistantsElement, count_vanishingpoint);
                count_vanishingpoint++;
            }
            else if (assist->id() == "infinite ruler"){
                assist->saveXmlList(doc, assistantsElement, count_infiniteruler);
                count_infiniteruler++;
            }
            else if (assist->id() == "parallel ruler"){
                assist->saveXmlList(doc, assistantsElement, count_parallelruler);
                count_parallelruler++;
            }
            else if (assist->id() == "concentric ellipse"){
                assist->saveXmlList(doc, assistantsElement, count_concentricellipse);
                count_concentricellipse++;
            }
            else if (assist->id() == "fisheye-point"){
                assist->saveXmlList(doc, assistantsElement, count_fisheyepoint);
                count_fisheyepoint++;
            }
            else if (assist->id() == "ruler"){
                assist->saveXmlList(doc, assistantsElement, count_ruler);
                count_ruler++;
            }
        }
        element.appendChild(assistantsElement);
    }
    return true;
}

bool KisKraSaver::saveGrid(QDomDocument& doc, QDomElement& element)
{
    KisGridConfig config = m_d->doc->gridConfig();

    if (!config.isDefault()) {
        QDomElement gridElement = config.saveDynamicDataToXml(doc, "grid");
        element.appendChild(gridElement);
    }

    return true;
}

bool KisKraSaver::saveGuides(QDomDocument& doc, QDomElement& element)
{
    KisGuidesConfig guides = m_d->doc->guidesConfig();

    if (guides.hasGuides()) {
        QDomElement guidesElement = guides.saveToXml(doc, "guides");
        element.appendChild(guidesElement);
    }

    return true;
}

