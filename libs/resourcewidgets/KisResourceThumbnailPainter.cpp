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
        // margin has empty space...which we want to be the color palette backround
        painter->fillRect(rect, palette.background());
    }

    if (selected) {
        painter->fillRect(rect, palette.highlight());
    }

    QRect innerRect = addMargin ? rect.adjusted(2, 2, -2, -2) : rect;
    QSize imageSize = thumbnail.size();

    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (resourceType == ResourceType::Gradients) {
        m_checkerPainter.paint(*painter, innerRect, innerRect.topLeft());
        if (!thumbnail.isNull()) {
            thumbnail = KisResourceThumbnailCache::instance()->getImage(index,
                                                                         innerRect.size() * devicePixelRatioF,
                                                                         Qt::IgnoreAspectRatio,
                                                                         Qt::SmoothTransformation);
            thumbnail.setDevicePixelRatio(devicePixelRatioF);
            painter->drawImage(innerRect.topLeft(), thumbnail);
        }
    } else if (resourceType == ResourceType::Patterns) {
        painter->fillRect(innerRect, Qt::white); // no checkers, they are confusing with patterns.
        if (!thumbnail.isNull() && (imageSize.height() > innerRect.height() || imageSize.width() > innerRect.width())) {
            thumbnail = KisResourceThumbnailCache::instance()->getImage(index,
                                                                         innerRect.size() * devicePixelRatioF,
                                                                         Qt::KeepAspectRatio,
                                                                         Qt::SmoothTransformation);
            thumbnail.setDevicePixelRatio(devicePixelRatioF);
        }
        QBrush patternBrush(thumbnail);
        patternBrush.setTransform(QTransform::fromTranslate(innerRect.x(), innerRect.y()));
        painter->fillRect(innerRect, patternBrush);
    } else if (resourceType == ResourceType::Workspaces || resourceType == ResourceType::WindowLayouts) {
        // TODO: thumbnails for workspaces and window layouts?
        painter->fillRect(innerRect, Qt::white);
        QPen before = painter->pen();
        painter->setPen(Qt::black);
        painter->drawText(innerRect, Qt::TextWordWrap, name.split("_").join(" "));
        painter->setPen(before);
    } else {
        painter->fillRect(innerRect, Qt::white); // no checkers, they are confusing with patterns.
        if (!thumbnail.isNull()) {
            if (imageSize.height() > innerRect.height()*devicePixelRatioF || imageSize.width() > innerRect.width()*devicePixelRatioF) {
                thumbnail =
                    KisResourceThumbnailCache::instance()->getImage(index,
                                                                     innerRect.size() * devicePixelRatioF,
                                                                     Qt::KeepAspectRatio,
                                                                     Qt::SmoothTransformation);
                thumbnail.setDevicePixelRatio(devicePixelRatioF);
            } else if (imageSize.height() < innerRect.height() * devicePixelRatioF
                       || imageSize.width() < innerRect.width() * devicePixelRatioF) {
                thumbnail =
                    KisResourceThumbnailCache::instance()->getImage(index,
                                                                     innerRect.size() * devicePixelRatioF,
                                                                     Qt::KeepAspectRatio,
                                                                     Qt::FastTransformation);
                thumbnail.setDevicePixelRatio(devicePixelRatioF);
            }
        }
        QPoint topleft(innerRect.topLeft());

        if (thumbnail.width() < innerRect.width()*devicePixelRatioF) {
            topleft.setX(topleft.x() + (innerRect.width() - thumbnail.width()/devicePixelRatioF) / 2);
        }
        if (thumbnail.height() < innerRect.height()*devicePixelRatioF) {
            topleft.setY(topleft.y() + (innerRect.height() - thumbnail.height()/devicePixelRatioF) / 2);
        }
        painter->drawImage(topleft, thumbnail);
    }

    painter->restore();
}
