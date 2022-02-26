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

#include <kis_painting_tweaks.h>
#include <kis_debug.h>

namespace
{

struct KisFileIconRegistrar {
    KisFileIconRegistrar() {
        KisPreviewFileDialog::s_iconCreator = new KisFileIconCreator();
    }
};


static KisFileIconRegistrar s_registrar;


QIcon createIcon(const QImage &source, const QSize &iconSize)
{
    QImage result;

    QSize scaled = source.size().scaled(iconSize, Qt::KeepAspectRatio);
    qreal scale = scaled.width()/(qreal)(source.width());

    if (scale >= 2) {
        // it can be treated like pixel art
        // first scale with NN
        int scaleInt = qRound(scale);
        result = source.scaled(scaleInt*source.size(), Qt::KeepAspectRatio, Qt::FastTransformation);
        // them with smooth transformation to make sure the whole icon is filled in
        result = result.scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    } else {
        result = source.scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    // The cropping here centers the image with the difference
    // between the result and the expected width/height.
    // The final size must be compared with iconSize
    // because otherwise parts of the image may be chopped off if
    // result > iconSize.
    result = result.convertToFormat(QImage::Format_ARGB32) // add transparency
        .copy((result.width() - iconSize.width()) / 2, (result.height() - iconSize.height()) / 2, iconSize.width(), iconSize.height());

    // draw faint outline
    QPainter painter(&result);
    QColor textColor = qApp->palette().color(QPalette::Text);
    QColor backgroundColor = qApp->palette().color(QPalette::Background);
    QColor blendedColor = KisPaintingTweaks::blendColors(textColor, backgroundColor, 0.2);
    painter.setPen(blendedColor);
    painter.drawRect(result.rect().adjusted(0, 0, -1, -1));

    return QIcon(QPixmap::fromImage(result));
}

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
            QScopedPointer<KisDocument> doc(KisPart::instance()->createTemporaryDocument());
            doc->setFileBatchMode(true);
            bool r = doc->openPath(path, KisDocument::DontAddToRecent);
            if (r) {
                KisPaintDeviceSP projection = doc->image()->projection();
                const QRect bounds = projection->exactBounds();
                QSize imageSize = bounds.size();
                if (imageSize.width() > iconSize.width() || imageSize.height() > iconSize.height()) {
                    imageSize.scale(iconSize, Qt::KeepAspectRatio);
                }
                const QImage &thumbnail = projection->createThumbnail(imageSize.width(), imageSize.height(), bounds);
                icon = createIcon(thumbnail, iconSize);
                return true;
            } else {
                return false;
            }
        } else {
            QImage img;
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
