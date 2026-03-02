/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */


#include <QPainter>
#include <QDebug>

#include "KisResourceItemDelegate.h"
#include "KisResourceModel.h"
#include <KisResourceThumbnailCache.h>
#include <KisResourceModelProvider.h>
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

void KisResourceItemDelegate::setNeedIndexConversion(bool isWidget)
{
    m_isWidget = isWidget;
}

QModelIndex KisResourceItemDelegate::convertToGlobalModelIndexIfNeeded(const QModelIndex &localIndex) const
{
    if (!m_isWidget) return localIndex;

    const int resourceId = localIndex.data(Qt::UserRole + KisAllResourcesModel::Id).toInt();
    const QString resourceType = localIndex.data(Qt::UserRole + KisAllResourcesModel::ResourceType).toString();

    auto *model = KisResourceModelProvider::resourceModel(resourceType);
    QModelIndex globalIndex = model->indexForResourceId(resourceId);
    return globalIndex;
}

void KisResourceItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option, const QModelIndex &index) const
{
    if (!index.isValid()) return;
        painter->save();

    if (m_showText) {
        QRect paintRect = QRect(option.rect.x(), option.rect.y(), option.rect.height(), option.rect.height());
        QString resourceDisplayName = index.data(Qt::UserRole + KisAbstractResourceModel::Name).toString().replace("_", " "); // don't need underscores that might be part of the file name
        painter->drawText(paintRect.width() + 10, option.rect.y() + option.rect.height() - 10, resourceDisplayName);

        const QModelIndex globalIndex = convertToGlobalModelIndexIfNeeded(index);
        m_thumbnailPainter.paint(painter, globalIndex, paintRect, option.palette, false, true);

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
        const QModelIndex globalIndex = convertToGlobalModelIndexIfNeeded(index);
        m_thumbnailPainter.paint(painter, globalIndex, option.rect, option.palette, option.state & QStyle::State_Selected, true);
    }
    painter->restore();
}


QSize KisResourceItemDelegate::sizeHint(const QStyleOptionViewItem &optionItem, const QModelIndex &index) const
{
    Q_UNUSED(index);
    return optionItem.decorationSize;
}
