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
#include <QMessageBox>

#include <KoDocumentInfo.h>
#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoStore.h>

#include <kis_annotation.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_layer_composition.h>
#include <kis_painting_assistants_manager.h>

#include "kis_doc2.h"


using namespace KRA;

struct KisKraSaver::Private
{
public:
    KisDoc2* doc;
    QMap<const KisNode*, QString> nodeFileNames;
    QString imageName;
};

KisKraSaver::KisKraSaver(KisDoc2* document)
        : m_d(new Private)
{
    m_d->doc = document;

    m_d->imageName = m_d->doc->documentInfo()->aboutInfo("title");
    if (m_d->imageName.isEmpty())
        m_d->imageName = "Unnamed";
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
    KisSaveXmlVisitor visitor(doc, imageElement, count, true);
    visitor.setSelectedNodes(m_d->doc->activeNodes());

    image->rootLayer()->accept(visitor);
    m_d->nodeFileNames = visitor.nodeFileNames();

    saveCompositions(doc, imageElement, image);

    return imageElement;
}

bool KisKraSaver::saveBinaryData(KoStore* store, KisImageWSP image, const QString & uri, bool external)
{
    QString location;

    // Save the layers data
    quint32 count = 0;

    KisKraSaveVisitor visitor(store, count, m_d->imageName, m_d->nodeFileNames);

    if (external)
        visitor.setExternalUri(uri);

    image->rootLayer()->accept(visitor);
    // saving annotations
    // XXX this only saves EXIF and ICC info. This would probably need
    // a redesign of the dtd of the krita file to do this more generally correct
    // e.g. have <ANNOTATION> tags or so.
    KisAnnotationSP annotation = image->annotation("exif");
    if (annotation) {
        location = external ? QString::null : uri;
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
            location = external ? QString::null : uri;
            location += m_d->imageName + ICC_PATH;
            if (store->open(location)) {
                store->write(annotation->annotation());
                store->close();
            }
        }
    }
    saveAssistants(store, uri,external);
    return true;
}

void KisKraSaver::saveCompositions(QDomDocument& doc, QDomElement& element, KisImageWSP image)
{
    QDomElement e = doc.createElement("compositions");
    foreach(KisLayerComposition* composition, image->compositions()) {
        composition->save(doc, e);
    }
    element.appendChild(e);
}

bool KisKraSaver::saveAssistants(KoStore* store, QString uri, bool external)
{
    QString location;
    QByteArray data_ellipse,data_perspective,data_spline,data_ruler,data_info;
    int count_ellipse = 0, count_perspective = 0, count_ruler = 0, count_spline = 0;
    QList<KisPaintingAssistant*> assistants =  m_d->doc->assistants();
    QMap<KisPaintingAssistantHandleSP, int> handlemap;
    if (!assistants.isEmpty()) {
        foreach(KisPaintingAssistant* assist, assistants){
            if (assist->id() == "ellipse"){
                location = external ? QString::null : uri;
                location += m_d->imageName + ASSISTANTS_PATH;
                location += QString("ellipse%1.assistant").arg(count_ellipse);
                data_ellipse = assist->saveXml(count_ellipse,handlemap);
                store->open(location);
                store->write(data_ellipse);
                store->close();
                count_ellipse++;
            }
            else if (assist->id() == "spline"){
                location = external ? QString::null : uri;
                location += m_d->imageName + ASSISTANTS_PATH;
                location += QString("spline%1.assistant").arg(count_spline);
                data_spline = assist->saveXml(count_spline,handlemap);
                store->open(location);
                store->write(data_spline);
                store->close();
                count_spline++;
            }
            else if(assist->id() == "perspective"){
                location = external ? QString::null : uri;
                location += m_d->imageName + ASSISTANTS_PATH;
                location += QString("perspective%1.assistant").arg(count_perspective);
                data_perspective = assist->saveXml(count_perspective,handlemap);
                store->open(location);
                store->write(data_perspective);
                store->close();
                count_perspective++;
            }
            else if(assist->id() == "ruler"){
                location = external ? QString::null : uri;
                location += m_d->imageName + ASSISTANTS_PATH;
                location += QString("ruler%1.assistant").arg(count_ruler);
                data_ruler = assist->saveXml(count_ruler,handlemap);
                store->open(location);
                store->write(data_ruler);
                store->close();
                count_ruler++;
            }
        }

    }
    return true;
}

bool KisKraSaver::saveAssistantsList(QDomDocument& doc, QDomElement& element)
{
    QList<KisPaintingAssistant*> assistants =  m_d->doc->assistants();
    if (!assistants.isEmpty()) {
        QDomElement assistantsElement = doc.createElement("assistants");
        foreach(KisPaintingAssistant* assist, assistants){
            if (assist->id() == "ellipse"){
                QDomElement assistantElement = doc.createElement("assistant");
                assistantElement.setAttribute("type", "ellipse");
                assistantElement.setAttribute("path", location);
                assistantsElement.appendChild(assistantElement);
            }
            else if (assist->id() == "spline"){
                QDomElement assistantElement = doc.createElement("assistant");
                assistantElement.setAttribute("type", "spline");
                assistantElement.setAttribute("path", location);
                assistantsElement.appendChild(assistantElement);
            }
            else if(assist->id() == "perspective"){
                QDomElement assistantElement = doc.createElement("assistant");
                assistantElement.setAttribute("type", "perspective");
                assistantElement.setAttribute("path", location);
                assistantsElement.appendChild(assistantElement);
            }
            else if(assist->id() == "ruler"){
                QDomElement assistantElement = doc.createElement("assistant");
                assistantElement.setAttribute("type", "ruler");
                assistantElement.setAttribute("path", location);
                assistantsElement.appendChild(assistantElement);
            }
        }
        element.appendChild(assistantsElement);
    }
    return true;
}
