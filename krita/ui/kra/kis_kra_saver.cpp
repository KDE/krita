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

#include <KoDocumentInfo.h>
#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoIccColorProfile.h>
#include <KoStore.h>

#include <kis_annotation.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_layer.h>
#include <kis_adjustment_layer.h>

#include "kis_doc2.h"


using namespace KRA;

class KisKraSaver::Private
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
    if (image->profile() && image->profile()-> valid())
        imageElement.setAttribute(PROFILE, image->profile()->name());
    imageElement.setAttribute(X_RESOLUTION, image->xRes()*72.0);
    imageElement.setAttribute(Y_RESOLUTION, image->yRes()*72.0);

    quint32 count = 1; // We don't save the root layer, but it does count
    KisSaveXmlVisitor visitor(doc, imageElement, count, true);

    image->rootLayer()->accept(visitor);
    m_d->nodeFileNames = visitor.nodeFileNames();
    return imageElement;
}

bool KisKraSaver::saveBinaryData(KoStore* store, KisImageWSP image, const QString & uri, bool external)
{
    QString location;

    // Save the layers data
    quint32 count = 0;

    KisKraSaveVisitor visitor(image, store, count, m_d->imageName, m_d->nodeFileNames);

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
            const KoIccColorProfile* iccprofile = dynamic_cast<const KoIccColorProfile*>(profile);
            if (iccprofile && !iccprofile->rawData().isEmpty())
                annotation = new  KisAnnotation(ICC, iccprofile->name(), iccprofile->rawData());
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
    return true;
}
