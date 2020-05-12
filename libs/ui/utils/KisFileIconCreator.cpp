/*
 *  Copyright (c) 2019 2020 Agata Cacko <cacko.azh@gmail.com>
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


#include "KisFileIconCreator.h"
#include <KoStore.h>
#include <KisMimeDatabase.h>
#include <KisDocument.h>
#include <KisPart.h>
#include <QFileInfo>

#include <kis_debug.h>


KisFileIconCreator::KisFileIconCreator()
{
}

bool KisFileIconCreator::createFileIcon(QString recentFileUrlPath, QIcon &icon, qreal devicePixelRatioF)
{
    QFileInfo fi(recentFileUrlPath);
    if (fi.exists()) {
        QString mimeType = KisMimeDatabase::mimeTypeForFile(recentFileUrlPath);
        if (mimeType == KisDocument::nativeFormatMimeType()
               || mimeType == "image/openraster") {

            QScopedPointer<KoStore> store(KoStore::createStore(recentFileUrlPath, KoStore::Read));
            if (store) {
                QString thumbnailpath;
                if (store->hasFile(QString("Thumbnails/thumbnail.png"))){
                    thumbnailpath = QString("Thumbnails/thumbnail.png");
                } else if (store->hasFile(QString("preview.png"))) {
                    thumbnailpath = QString("preview.png");
                }
                if (!thumbnailpath.isEmpty() && store->open(thumbnailpath)) {

                    QByteArray bytes = store->read(store->size());
                    store->close();
                    QImage img;
                    img.loadFromData(bytes);
                    img.setDevicePixelRatio(devicePixelRatioF);

                    icon = QIcon(QPixmap::fromImage(img));
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
            doc.reset(KisPart::instance()->createDocument());
            doc->setFileBatchMode(true);
            bool r = doc->openUrl(QUrl::fromLocalFile(recentFileUrlPath), KisDocument::DontAddToRecent);
            if (r) {
                KisPaintDeviceSP projection = doc->image()->projection();
                icon = QIcon(QPixmap::fromImage(projection->createThumbnail(48, 48, projection->exactBounds())));
                return true;

            } else {
                return false;
            }
        } else {
            QImage img;
            img.setDevicePixelRatio(devicePixelRatioF);
            img.load(recentFileUrlPath);
            if (!img.isNull()) {
                icon = QIcon(QPixmap::fromImage(img.scaledToWidth(48)));
                return true;
            } else {
                return false;
            }
        }
    } else {
        return false;
    }
}
