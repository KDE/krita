/*
 *  SPDX-FileCopyrightText: 2019-2020 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisFileIconCreator.h"

#include <QFileInfo>
#include <QApplication>

#include <KoStore.h>

#include <KisMimeDatabase.h>
#include <KisDocument.h>
#include <KisPart.h>
#include <KisPreviewFileDialog.h>
#include <QFileInfo>

#include <krita_utils.h>
#include <kis_debug.h>

namespace
{

struct KisFileIconRegistrar {
    KisFileIconRegistrar() {
        KisPreviewFileDialog::s_iconCreator = new KisFileIconCreator();
    }
};


static KisFileIconRegistrar s_registrar;


QIcon createIcon(const QImage &source, const QSize &iconSize, bool dontUpsize = false)
{
    QImage result;
    int maxIconSize = qMax(iconSize.width(), iconSize.height());
    if (dontUpsize) {
        if (source.width() < iconSize.width() || source.height() < iconSize.height()) {
            maxIconSize = qMax(source.width(), source.height());
        }
    }
    QSize iconSizeSquare = QSize(maxIconSize, maxIconSize);

    QSize scaled = source.size().scaled(iconSize, Qt::KeepAspectRatio);
    qreal scale = scaled.width()/source.width();

    if (scale >= 2) {
        // it can be treated like pixel art
        // first scale with NN
        int scaleInt = qRound(scale);
        result = source.scaled(scaleInt*source.size(), Qt::KeepAspectRatio, Qt::FastTransformation);
        // them with smooth transformation to make sure the whole icon is filled in
        result = result.scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        result = source.scaled(iconSizeSquare, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    result = result.convertToFormat(QImage::Format_ARGB32) // add transparency
        .copy((result.width() - maxIconSize) / 2, (result.height() - maxIconSize) / 2, maxIconSize, maxIconSize);

    // draw faint outline
    QPainter painter(&result);
    QColor textColor = qApp->palette().color(QPalette::Text);
    QColor backgroundColor = qApp->palette().color(QPalette::Background);
    QColor blendedColor = KritaUtils::blendColors(textColor, backgroundColor, 0.2);
    painter.setPen(blendedColor);
    painter.drawRect(result.rect().adjusted(0, 0, -1, -1));

    return QIcon(QPixmap::fromImage(result));
}

}


bool KisFileIconCreator::createFileIcon(QString path, QIcon &icon, qreal devicePixelRatioF, QSize iconSize, bool dontUpsize)
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

                    icon = createIcon(img, iconSize, dontUpsize);
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
                icon = createIcon(thumbnail, iconSize, dontUpsize);
                return true;
            } else {
                return false;
            }
        } else {
            QImage img;
            img.setDevicePixelRatio(devicePixelRatioF);
            img.load(path);
            if (!img.isNull()) {
                icon = createIcon(img, iconSize, dontUpsize);
                return true;
            } else {
                return false;
            }
        }
    } else {
        return false;
    }
}
