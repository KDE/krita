/*
 * SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_canvas_drop.h"

#include <KLocalizedString>
#include <QAction>
#include <QUrl>

class KisCanvasDrop::Private
{
public:
    QAction *insertAsNewLayer;
    QAction *insertManyLayers;
    QAction *insertAsNewFileLayer;
    QAction *insertManyFileLayers;
    QAction *openInNewDocument;
    QAction *openManyDocuments;
    QAction *insertAsReferenceImage;
    QAction *insertAsReferenceImages;
    QAction *cancel;
};

KisCanvasDrop::KisCanvasDrop(QWidget *parent)
    : QMenu(parent)
    , d(new Private)
{
    setObjectName("drop_popup");

    d->insertAsNewLayer = addAction(i18n("Insert as New Layer"));
    d->insertAsNewFileLayer = addAction(i18n("Insert as New File Layer"));
    d->openInNewDocument = addAction(i18n("Open in New Document"));
    d->insertAsReferenceImage = addAction(i18n("Insert as Reference Image"));

    d->insertManyLayers = addAction(i18n("Insert Many Layers"));
    d->insertManyFileLayers = addAction(i18n("Insert Many File Layers"));
    d->openManyDocuments = addAction(i18n("Open Many Documents"));
    d->insertAsReferenceImages = addAction(i18n("Insert as Reference Images"));

    addSeparator();

    d->cancel = addAction(i18n("Cancel"));
}

KisCanvasDrop::Action KisCanvasDrop::dropAs(const QMimeData &data, QPoint pos)
{
    const auto &urls = data.urls();

    d->insertAsNewLayer->setEnabled(data.hasImage() || urls.count() == 1);
    d->insertAsNewFileLayer->setEnabled(urls.count() == 1);
    d->openInNewDocument->setEnabled(urls.count() == 1);
    d->insertAsReferenceImage->setEnabled(data.hasImage() || urls.count() == 1);

    d->insertManyLayers->setEnabled(urls.count() > 1);
    d->insertManyFileLayers->setEnabled(urls.count() > 1);
    d->openManyDocuments->setEnabled(urls.count() > 1);
    d->insertAsReferenceImages->setEnabled(urls.count() > 1);

    const QAction *action = exec(pos);

    if (action == d->insertAsNewLayer) {
        return KisCanvasDrop::INSERT_AS_NEW_LAYER;
    } else if (action == d->insertAsNewFileLayer) {
        return KisCanvasDrop::INSERT_AS_NEW_FILE_LAYER;
    } else if (action == d->openInNewDocument) {
        return KisCanvasDrop::OPEN_IN_NEW_DOCUMENT;
    } else if (action == d->insertAsReferenceImage) {
        return KisCanvasDrop::INSERT_AS_REFERENCE_IMAGE;
    } else if (action == d->insertManyLayers) {
        return KisCanvasDrop::INSERT_MANY_LAYERS;
    } else if (action == d->insertManyFileLayers) {
        return KisCanvasDrop::INSERT_MANY_FILE_LAYERS;
    } else if (action == d->openManyDocuments) {
        return KisCanvasDrop::OPEN_MANY_DOCUMENTS;
    } else if (action == d->insertAsReferenceImages) {
        return KisCanvasDrop::INSERT_AS_REFERENCE_IMAGES;
    }

    return KisCanvasDrop::NONE;
}
