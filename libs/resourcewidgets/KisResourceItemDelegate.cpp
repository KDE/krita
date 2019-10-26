/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    QRect innerRect = option.rect.adjusted(2, 1, -2, -1);

    QImage thumbnail = index.data(Qt::DecorationRole).value<QImage>();
    QSize imageSize = thumbnail.size();

    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    QString resourceType = index.data(Qt::UserRole + KisResourceModel::ResourceType).toString();
    // XXX: don't use a hardcoded string here to identify the resource type
    if (resourceType == ResourceType::Gradients) {
        m_checkerPainter.paint(*painter, innerRect);
        thumbnail = thumbnail.scaled(innerRect.width(), innerRect.height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        painter->drawImage(innerRect.topLeft(), thumbnail);
    }
    else if (resourceType == ResourceType::Patterns) {
        painter->fillRect(innerRect, Qt::white); // no checkers, they are confusing with patterns.
        if (imageSize.height() > innerRect.height() || imageSize.width() > innerRect.width()) {
            thumbnail = thumbnail.scaled(innerRect.width(), innerRect.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        painter->fillRect(innerRect, QBrush(thumbnail));
    }
    else {
        painter->fillRect(innerRect, Qt::white); // no checkers, they are confusing with patterns.
        if (imageSize.height() > innerRect.height() || imageSize.width() > innerRect.width()) {
            thumbnail = thumbnail.scaled(innerRect.width(), innerRect.height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        QPoint topleft(innerRect.topLeft());

        if (thumbnail.width() < innerRect.width()) {
            topleft.setX(topleft.x() + (innerRect.width() - thumbnail.width()) / 2);
        }
        if (thumbnail.height() < innerRect.height()) {
            topleft.setY(topleft.y() + (innerRect.height() - thumbnail.height()) / 2);
        }
        painter->drawImage(topleft, thumbnail);
    }


    painter->restore();
}

QSize KisResourceItemDelegate::sizeHint(const QStyleOptionViewItem &optionItem, const QModelIndex &) const
{
    return optionItem.decorationSize;
}
