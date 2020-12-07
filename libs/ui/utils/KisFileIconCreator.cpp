/*
 *  Copyright (c) 2019 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include "KisFileIconCreator.h"
#include <KoStore.h>
#include <KisMimeDatabase.h>
#include <KisDocument.h>
#include <KisPart.h>
#include <QFileInfo>

#include <kis_debug.h>

namespace
{

QIcon createIcon(const QImage &source, const QSize &iconSize)
{
    QImage result;
    const int maxIconSize = qMax(iconSize.width(), iconSize.height());

    result = source.scaled(maxIconSize, maxIconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    result = result.convertToFormat(QImage::Format_ARGB32) // add transparency
        .copy((result.width() - maxIconSize) / 2, (result.height() - maxIconSize) / 2, maxIconSize, maxIconSize);

    // draw faint outline
    QPainter painter(&result);
    painter.setPen(QColor("#40808080"));
    painter.drawRect(result.rect().adjusted(0, 0, -1, -1));

    return QIcon(QPixmap::fromImage(result));
}

}


KisFileIconCreator::KisFileIconCreator()
{
}

bool KisFileIconCreator::createFileIcon(QString path, QIcon &icon, qreal devicePixelRatioF, QSize iconSize)
{
    iconSize *= devicePixelRatioF;
    QFileInfo fi(path);
    if (fi.exists()) {
        QString mimeType = KisMimeDatabase::mimeTypeForFile(path);
        if (mimeType == KisDocument::nativeFormatMimeType()
               || mimeType == "image/openraster") {

            QScopedPointer<KoStore> store(KoStore::createStore(path, KoStore::Read));
            if (store) {
                QString thumbnailpath;
                if (store->hasFile(QString("Thumbnails/thumbnail.png"))){
                    thumbnailpath = QString("Thumbnails/thumbnail.png");
                }
                else if (store->hasFile(QString("mergedimage.png"))) {
                    thumbnailpath = QString("mergedimage.png");
                }
                else if (store->hasFile(QString("preview.png"))) {
                    thumbnailpath = QString("preview.png");
                }
                if (!thumbnailpath.isEmpty() && store->open(thumbnailpath)) {

                    QByteArray bytes = store->read(store->size());
                    store->close();
                    QImage img;
                    img.loadFromData(bytes);
                    img.setDevicePixelRatio(devicePixelRatioF);

                    icon = createIcon(img, iconSize);
                    return true;

                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else if (mimeType == "image/tiff" || mimeType == "image/x-tiff") {
            // Workaround for a bug in Qt tiff QImageIO plugin
            QScopedPointer<KisDocument> doc;
            doc.reset(KisPart::instance()->createTemporaryDocument());
            doc->setFileBatchMode(true);
            bool r = doc->openUrl(QUrl::fromLocalFile(path), KisDocument::DontAddToRecent);
            if (r) {
                KisPaintDeviceSP projection = doc->image()->projection();
                const QRect bounds = projection->exactBounds();
                const float ratio = static_cast<float>(bounds.width()) / bounds.height();
                const int maxWidth = qMax(iconSize.width(), iconSize.height());
                const int maxHeight = static_cast<int>(maxWidth * ratio);
                const QImage &thumbnail = projection->createThumbnail(maxWidth, maxHeight, bounds);
                icon = createIcon(thumbnail, iconSize);
                return true;
            } else {
                return false;
            }
        } else {
            QImage img;
            img.setDevicePixelRatio(devicePixelRatioF);
            img.load(path);
            if (!img.isNull()) {
                icon = createIcon(img, iconSize);
                return true;
            } else {
                return false;
            }
        }
    } else {
        return false;
    }
}
