/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisResourceItemDelegate.h"

#include <QPainter>
#include <QDebug>

#include "KisResourceModel.h"

KisResourceItemDelegate::KisResourceItemDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
    , m_checkerPainter(4)
{
}

void KisResourceItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    if (!index.isValid()) return;

    painter->save();

    qreal devicePixelRatioF = painter->device()->devicePixelRatioF();

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    QRect innerRect = option.rect.adjusted(2, 2, -2, -2);

    QImage thumbnail = index.data(Qt::UserRole + KisAbstractResourceModel::Thumbnail).value<QImage>();
    thumbnail.setDevicePixelRatio(devicePixelRatioF);

    QSize imageSize = thumbnail.size();

    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    QString resourceType = index.data(Qt::UserRole + KisAbstractResourceModel::ResourceType).toString();
    // XXX: don't use a hardcoded string here to identify the resource type
    if (resourceType == ResourceType::Gradients) {
        m_checkerPainter.paint(*painter, innerRect, innerRect.topLeft());
        thumbnail = thumbnail.scaled(innerRect.size()*devicePixelRatioF, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        painter->drawImage(innerRect.topLeft(), thumbnail);
    }
    else if (resourceType == ResourceType::Patterns) {
        painter->fillRect(innerRect, Qt::white); // no checkers, they are confusing with patterns.
        if (imageSize.height() > innerRect.height() || imageSize.width() > innerRect.width()) {
            thumbnail = thumbnail.scaled(innerRect.size()*devicePixelRatioF, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        QBrush patternBrush(thumbnail);
        patternBrush.setTransform(QTransform::fromTranslate(innerRect.x(), innerRect.y()));
        painter->fillRect(innerRect, patternBrush);
    }
    else if (resourceType == ResourceType::Workspaces || resourceType == ResourceType::WindowLayouts) {
        // TODO: thumbnails for workspaces and window layouts?
        painter->fillRect(innerRect, Qt::white);
        QString name = index.data(Qt::UserRole + KisAbstractResourceModel::Name).toString();
        QPen before = painter->pen();
        painter->setPen(Qt::black);
        painter->drawText(innerRect, Qt::TextWordWrap, name.split("_").join(" "));
        painter->setPen(before);
    }
    else {
        painter->fillRect(innerRect, Qt::white); // no checkers, they are confusing with patterns.
        if (imageSize.height() > innerRect.height()*devicePixelRatioF || imageSize.width() > innerRect.width()*devicePixelRatioF) {
            thumbnail = thumbnail.scaled(innerRect.size()*devicePixelRatioF, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        } else if(imageSize.height() < innerRect.height()*devicePixelRatioF || imageSize.width() < innerRect.width()*devicePixelRatioF) {
            thumbnail = thumbnail.scaled(innerRect.size()*devicePixelRatioF, Qt::KeepAspectRatio, Qt::FastTransformation);
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

QSize KisResourceItemDelegate::sizeHint(const QStyleOptionViewItem &optionItem, const QModelIndex &) const
{
    return optionItem.decorationSize;
}
