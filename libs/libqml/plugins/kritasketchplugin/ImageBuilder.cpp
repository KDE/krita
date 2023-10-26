/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "ImageBuilder.h"
#include "DocumentManager.h"

#include <QApplication>
#include <QDesktopWidget>

#include <KoCompositeOpRegistry.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_config.h>
#include <kis_clipboard.h>
#include <kis_layer.h>
#include <kis_painter.h>
#include "kis_paint_layer.h"
#include "kis_import_catcher.h"


ImageBuilder::ImageBuilder(QObject* parent)
: QObject(parent)
{
}

ImageBuilder::~ImageBuilder()
{

}

QString ImageBuilder::createBlankImage(int width, int height, int resolution)
{
    DocumentManager::instance()->newDocument(width, height, resolution / 72.0f);
    return QString("temp://%1x%2").arg(width).arg(height);
}

QString ImageBuilder::createBlankImage(const QVariantMap& options)
{
    DocumentManager::instance()->newDocument(options);
    return QString("temp://custom");
}

QString ImageBuilder::createImageFromClipboard()
{
    QSize sz = KisClipboard::instance()->clipSize();
    KisPaintDeviceSP clipDevice = KisClipboard::instance()->clip(QRect(0, 0, sz.width(), sz.height()), false);

    if (clipDevice) {
        connect(DocumentManager::instance(), SIGNAL(documentChanged()), SLOT(createImageFromClipboardDelayed()));
        DocumentManager::instance()->newDocument(sz.width(), sz.height(), 1.0);
    }
    else {
        sz.setWidth(qApp->desktop()->width());
        sz.setHeight(qApp->desktop()->height());
        DocumentManager::instance()->newDocument(sz.width(), sz.height(), 1.0f);
    }
    return QString("temp://%1x%2").arg(sz.width()).arg(sz.height());
}

void ImageBuilder::createImageFromClipboardDelayed()
{
    DocumentManager::instance()->disconnect(this, SLOT(createImageFromClipboardDelayed()));

    KisPaintDeviceSP clip = KisClipboard::instance()->clip(QRect(), true);
    KisImageWSP image = DocumentManager::instance()->document()->image();
    if (image && image->root() && image->root()->firstChild()) {
        KisNodeSP node = image->root()->firstChild();
        while (node && (!dynamic_cast<KisPaintLayer*>(node.data()) || node->userLocked())) {
            node = node->nextSibling();
        }

        if (!node) {
            KisPaintLayerSP newLayer = new KisPaintLayer(image, image->nextLayerName(), OPACITY_OPAQUE_U8);
            image->addNode(newLayer);
            node = newLayer;
        }

        KIS_SAFE_ASSERT_RECOVER_RETURN(node);

        KisPaintLayer * layer = dynamic_cast<KisPaintLayer*>(node.data());
        KIS_SAFE_ASSERT_RECOVER_RETURN(layer);

        layer->setOpacity(OPACITY_OPAQUE_U8);
        const QRect r = clip->exactBounds();
        KisImportCatcher::adaptClipToImageColorSpace(clip, image);

        KisPainter::copyAreaOptimized(QPoint(), clip, layer->paintDevice(), r);
        layer->setDirty();

    }
}

QString ImageBuilder::createImageFromTemplate(const QVariantMap& options)
{
    DocumentManager::instance()->newDocument(options);
    return QString("temp://%1").arg(options.value("template").toString());
}
