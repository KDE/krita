/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisResourceThumbnailPainter.h"
#include "KisResourceModel.h"

#include <QPainter>
#include <QDebug>

#include <KisResourceThumbnailCache.h>

KisResourceThumbnailPainter::KisResourceThumbnailPainter(QObject *parent)
    : QObject(parent)
    , m_checkerPainter(4)
{
}

QImage KisResourceThumbnailPainter::getReadyThumbnail(const QModelIndex &index, QSize size, const QPalette& palette) const
{
    QImage thumbLabel = QImage(size, QImage::Format_ARGB32);
    thumbLabel.fill(Qt::white);
    QPainter painter(&thumbLabel);
    paint(&painter, index, QRect(QPoint(0, 0), size), palette, false, false);
    return thumbLabel;
}

void KisResourceThumbnailPainter::paint(QPainter *painter, const QModelIndex& index, QRect rect, const QPalette& palette, bool selected, bool addMargin) const
{
    const qreal devicePixelRatioF = painter->device()->devicePixelRatioF();

    QImage thumbnail = KisResourceThumbnailCache::instance()->getImage(index);
    thumbnail.setDevicePixelRatio(devicePixelRatioF);

    const QString resourceType = index.data(Qt::UserRole + KisAbstractResourceModel::ResourceType).toString();
    const QString name = index.data(Qt::UserRole + KisAbstractResourceModel::Tooltip).toString();

    painter->save();

    if(addMargin) {
        // margin has empty space...which we want to be the color palette background
        painter->fillRect(rect, palette.background());
    }

    if (selected) {
        painter->fillRect(rect, palette.highlight());
    }

    QRect innerRect = addMargin ? rect.adjusted(2, 2, -2, -2) : rect;
    QSize imageSize = thumbnail.size();
    QSize innerRectSizeDPI = innerRect.size() * devicePixelRatioF;

    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (resourceType == ResourceType::Gradients) {
        m_checkerPainter.paint(*painter, innerRect, innerRect.topLeft());
        if (!thumbnail.isNull()) {
            thumbnail = KisResourceThumbnailCache::instance()->getImage(index,
                                                                         innerRectSizeDPI,
                                                                         Qt::IgnoreAspectRatio,
                                                                         Qt::SmoothTransformation);
            thumbnail.setDevicePixelRatio(devicePixelRatioF);
            painter->drawImage(innerRect.topLeft(), thumbnail);
        }
    } else if (resourceType == ResourceType::Patterns) {
        painter->fillRect(innerRect, Qt::white); // no checkers, they are confusing with patterns.
        // for large patterns, scale down to no less than 50% zoom and crop it,
        // to see distinguishing details without being too zoomed in
        double scale = qMin(double(thumbnail.height()) / innerRectSizeDPI.height(), double(thumbnail.width()) / innerRectSizeDPI.width());
        if (!thumbnail.isNull() && (scale >= 1.0)) {
            scale = qMin(scale, 2.0);
            QRect sourceRect = QRect(0, 0, innerRectSizeDPI.width() * scale, innerRectSizeDPI.height() * scale);
            painter->drawImage(innerRect, thumbnail, sourceRect);
        } else {
            // crisply transform small patterns, which get scaled up to user-space,
            // preventing them from being too tiny to see on hi-dpi
            painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
            QBrush patternBrush(thumbnail);
            painter->setBrushOrigin(innerRect.topLeft());
            painter->fillRect(innerRect, patternBrush);
        }
    } else if (resourceType == ResourceType::WindowLayouts) {
        // TODO: thumbnails for window layouts?
        painter->fillRect(innerRect, Qt::white);
        QPen before = painter->pen();
        painter->setPen(Qt::black);
        painter->drawText(innerRect, Qt::TextWordWrap, name.split("_").join(" "));
        painter->setPen(before);
    } else {
        painter->fillRect(innerRect, Qt::white); // no checkers, they are confusing with patterns.
        if (!thumbnail.isNull() && !(thumbnail.height() == innerRectSizeDPI.height() &&
                                     thumbnail.width() == innerRectSizeDPI.width())) {
            bool needsUpscaling = thumbnail.height() < innerRectSizeDPI.height()
                                || thumbnail.width() < innerRectSizeDPI.width();

            thumbnail = KisResourceThumbnailCache::instance()->getImage(index,
                                                                        innerRectSizeDPI,
                                                                        Qt::KeepAspectRatio,
                                                                        needsUpscaling ? Qt::FastTransformation
                                                                                       : Qt::SmoothTransformation);
            thumbnail.setDevicePixelRatio(devicePixelRatioF);
        }
        QPoint topleft(innerRect.topLeft());

        if (thumbnail.width() < innerRectSizeDPI.width()) {
            topleft.setX(topleft.x() + (innerRect.width() - thumbnail.width()/devicePixelRatioF) / 2);
        }
        if (thumbnail.height() < innerRectSizeDPI.height()) {
            topleft.setY(topleft.y() + (innerRect.height() - thumbnail.height()/devicePixelRatioF) / 2);
        }
        painter->drawImage(topleft, thumbnail);
    }

    painter->restore();
}
