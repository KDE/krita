/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    KisConfig cfg(false);
    cfg.setPasteBehaviour(PASTE_ASSUME_MONITOR);

    QSize sz = KisClipboard::instance()->clipSize();
    KisPaintDeviceSP clipDevice = KisClipboard::instance()->clip(QRect(0, 0, sz.width(), sz.height()), false);
    KisImageWSP image = DocumentManager::instance()->document()->image();
    if (image && image->root() && image->root()->firstChild()) {
        KisLayer * layer = dynamic_cast<KisLayer*>(image->root()->firstChild().data());
        Q_ASSERT(layer);
        layer->setOpacity(OPACITY_OPAQUE_U8);
        QRect r = clipDevice->exactBounds();

        KisPainter::copyAreaOptimized(QPoint(), clipDevice, layer->paintDevice(), r);
        layer->setDirty(QRect(0, 0, sz.width(), sz.height()));
    }
}

QString ImageBuilder::createImageFromWebcam(int width, int height, int resolution)
{
    Q_UNUSED(width); Q_UNUSED(height); Q_UNUSED(resolution);
    return QString();
}

QString ImageBuilder::createImageFromTemplate(const QVariantMap& options)
{
    DocumentManager::instance()->newDocument(options);
    return QString("temp://%1").arg(options.value("template").toString());
}
