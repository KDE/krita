/*
 *  SPDX-FileCopyrightText: 2018 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisReferenceImageCollection.h"

#include <QIODevice>
#include <QMessageBox>

#include <libs/store/KoStore.h>
#include <KisReferenceImage.h>
#include <libs/store/KoStoreDevice.h>

const QString METADATA_FILE = "reference_images.xml";

KisReferenceImageCollection::KisReferenceImageCollection(const QVector<KisReferenceImage *> &references)
    : references(references)
{}

const QVector<KisReferenceImage*> &KisReferenceImageCollection::referenceImages() const
{
    return references;
}

bool KisReferenceImageCollection::save(QIODevice *io)
{
    QScopedPointer<KoStore> store(KoStore::createStore(io, KoStore::Write, "application/x-krita-reference-images]", KoStore::Zip));
    if (store.isNull()) return false;

    QDomDocument doc;
    QDomElement root = doc.createElement("referenceimages");
    doc.insertBefore(root, QDomNode());

    std::sort(references.begin(), references.end(), KoShape::compareShapeZIndex);

    int nextId = 0;
    Q_FOREACH(KisReferenceImage *reference, references) {
        reference->saveXml(doc, root, nextId++);

        if (reference->embed()) {
            bool ok = reference->saveImage(store.data());
            if (!ok) return false;
        }
    }

    if (!store->open(METADATA_FILE)) {
        return false;
    }

    KoStoreDevice xmlDev(store.data());
    xmlDev.write(doc.toByteArray());
    xmlDev.close();
    store->close();

    return true;
}

bool KisReferenceImageCollection::load(QIODevice *io)
{
    QScopedPointer<KoStore> store(KoStore::createStore(io, KoStore::Read, "application/x-krita-reference-images", KoStore::Zip));
    if (!store || store->bad()) {
        return false;
    }

    if (!store->hasFile(METADATA_FILE) || !store->open(METADATA_FILE)) {
        return false;
    }

    QByteArray xml = store->device()->readAll();
    store->close();

    QDomDocument doc;
    doc.setContent(xml);
    QDomElement root = doc.documentElement();

    QStringList failures;

    QDomElement element = root.firstChildElement("referenceimage");
    while (!element.isNull()) {
        KisReferenceImage *reference = KisReferenceImage::fromXml(element);

        if (reference->loadImage(store.data())) {
            references.append(reference);
        } else {
            failures << (reference->embed() ? reference->internalFile() : reference->filename());
            delete reference;
        }
        element = element.nextSiblingElement("referenceimage");
    }

    if (!failures.isEmpty()) {
        QMessageBox::warning(
                0,
                i18nc("@title:window", "Krita"),
                i18n("The following reference images could not be loaded:\n%1", failures.join('\n')),
                QMessageBox::Ok, QMessageBox::Ok
        );

    }

    return true;
}
