/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */


#include <QPainter>
#include <QDebug>
#include <QStyledItemDelegate>

#include "KisResourceItemDelegate.h"
#include "KisResourceModel.h"
#include <KisResourceThumbnailCache.h>
#include <KoIcon.h>

KisResourceItemDelegate::KisResourceItemDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
    , m_checkerPainter(4)
    , m_showText(false)
    , m_isWidget(false)
{
}

void KisResourceItemDelegate::setShowText(bool showText)
{
    m_showText = showText;
}

void KisResourceItemDelegate::setIsWidget(bool isWidget)
{
    m_isWidget = isWidget;
}

void KisResourceItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    if (!index.isValid()) return;
        painter->save();

    if (m_showText) {
        QRect paintRect = QRect(option.rect.x(), option.rect.y(), option.rect.height(), option.rect.height());
        QString resourceDisplayName = index.data(Qt::UserRole + KisAbstractResourceModel::Name).toString().replace("_", " "); // don't need underscores that might be part of the file name
        painter->drawText(paintRect.width() + 10, option.rect.y() + option.rect.height() - 10, resourceDisplayName);

        if (m_isWidget) { // Bundle Creator selected list...
            QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));

            QImage preview;
            if (!icon.isNull()) {
                QSize iconSize = option.decorationSize; // or specify a desired size
                QPixmap pixmap = icon.pixmap(iconSize);
                preview = pixmap.toImage();
            }

            painter->drawImage(paintRect.x(), paintRect.y(), preview);
        } else {
            m_thumbnailPainter.paint(painter, index, paintRect, option.palette, false, true);
        }

        if (option.state & QStyle::State_Selected) {
            painter->setCompositionMode(QPainter::CompositionMode_HardLight);
            painter->setOpacity(1.0);
            painter->fillRect(option.rect, option.palette.highlight());

            // highlight is not strong enough to pick out preset. draw border around it.
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter->setPen(QPen(option.palette.highlight(), 4, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
            QRect selectedBorder = option.rect.adjusted(2 , 2, -2, -2); // constrict the rectangle so it doesn't bleed into other presets
            painter->drawRect(selectedBorder);
        }
    } else {
        m_thumbnailPainter.paint(painter, index, option.rect, option.palette, option.state & QStyle::State_Selected, true);
    }
    painter->restore();
}


QSize KisResourceItemDelegate::sizeHint(const QStyleOptionViewItem &optionItem, const QModelIndex &index) const
{
    return optionItem.decorationSize;
}
