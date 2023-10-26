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
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    if (!(option.state & QStyle::State_Enabled)) {
        painter->setOpacity(0.2);
    }

    if (!index.isValid()) {
        painter->restore();
        return;
    }

    if (m_isWidget) {

        bool dirty = index.data(Qt::UserRole + KisAbstractResourceModel::Dirty).toBool();
        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));

        QImage preview;
        if (!icon.isNull()) {
            QSize iconSize = option.decorationSize; // or specify a desired size
            QPixmap pixmap = icon.pixmap(iconSize);
            preview = pixmap.toImage();
        }

        if (preview.isNull()) {
            preview = QImage(512, 512, QImage::Format_RGB32);
            preview.fill(Qt::red);
        }

        qreal devicePixelRatioF = painter->device()->devicePixelRatioF();
        QRect paintRect = option.rect.adjusted(1, 1, -1, -1);

        if (!m_showText) {
            QSize pixSize(paintRect.height(), paintRect.height());

            QSize size = pixSize * devicePixelRatioF;
            Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio;
            Qt::TransformationMode transformMode = Qt::SmoothTransformation;
            QImage previewHighDpi = preview.scaled(size, aspectMode, transformMode);

            previewHighDpi.setDevicePixelRatio(devicePixelRatioF);
            painter->drawImage(paintRect.x(), paintRect.y(), previewHighDpi);
        }
        else {
            QSize pixSize(paintRect.height(), paintRect.height());

            QSize size = pixSize * devicePixelRatioF;
            Qt::AspectRatioMode aspectMode = Qt::IgnoreAspectRatio;
            Qt::TransformationMode transformMode = Qt::SmoothTransformation;
            QImage previewHighDpi = preview.scaled(size, aspectMode, transformMode);

            previewHighDpi.setDevicePixelRatio(devicePixelRatioF);
            painter->drawImage(paintRect.x(), paintRect.y(), previewHighDpi);

            // Put an asterisk after the preset if it is dirty. This will help in case the pixmap icon is too small

            QString dirtyPresetIndicator = QString("");
            if (dirty) {
                dirtyPresetIndicator = QString("*");
            }

            // QString presetDisplayName = index.data(Qt::UserRole + KisAbstractResourceModel::Name).toString().replace("_", " "); // don't need underscores that might be part of the file name
            QString presetDisplayName = index.data(Qt::DisplayRole).toString();
            painter->drawText(pixSize.width() + 10, option.rect.y() + option.rect.height() - 10, presetDisplayName.append(dirtyPresetIndicator));
        }

        if (dirty) {
            const QIcon icon = KisIconUtils::loadIcon("dirty-preset");
            QPixmap pixmap = icon.pixmap(QSize(16,16));
            painter->drawPixmap(paintRect.x() + 3, paintRect.y() + 3, pixmap);
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
        painter->restore();
        return;

    }

    bool dirty = index.data(Qt::UserRole + KisAbstractResourceModel::Dirty).toBool();
    QImage preview = KisResourceThumbnailCache::instance()->getImage(index);

    if (preview.isNull()) {
        preview = QImage(512, 512, QImage::Format_RGB32);
        preview.fill(Qt::red);
    }

    qreal devicePixelRatioF = painter->device()->devicePixelRatioF();
    QRect paintRect = option.rect.adjusted(1, 1, -1, -1);

    if (!m_showText) {
        QImage previewHighDpi =
            KisResourceThumbnailCache::instance()->getImage(index,
                                                             paintRect.size() * devicePixelRatioF,
                                                             Qt::IgnoreAspectRatio,
                                                             Qt::SmoothTransformation);
        previewHighDpi.setDevicePixelRatio(devicePixelRatioF);

        QImage destBackground(previewHighDpi.size(), QImage::Format_RGB32);
        destBackground.fill(Qt::white);
        destBackground.setDevicePixelRatio(devicePixelRatioF);

        painter->drawImage(paintRect.x(), paintRect.y(), destBackground);
        painter->drawImage(paintRect.x(), paintRect.y(), previewHighDpi);
    }
    else {
        QSize pixSize(paintRect.height(), paintRect.height());
        QImage previewHighDpi = KisResourceThumbnailCache::instance()->getImage(index,
                                                                                pixSize * devicePixelRatioF,
                                                                                Qt::IgnoreAspectRatio,
                                                                                Qt::SmoothTransformation);
        previewHighDpi.setDevicePixelRatio(devicePixelRatioF);

        QImage destBackground(previewHighDpi.size(), QImage::Format_RGB32);
        destBackground.fill(Qt::white);
        destBackground.setDevicePixelRatio(devicePixelRatioF);

        painter->drawImage(paintRect.x(), paintRect.y(), destBackground);
        painter->drawImage(paintRect.x(), paintRect.y(), previewHighDpi);

        // Put an asterisk after the preset if it is dirty. This will help in case the pixmap icon is too small

        QString dirtyPresetIndicator = QString("");
        if (dirty) {
            dirtyPresetIndicator = QString("*");
        }

        QString presetDisplayName = index.data(Qt::UserRole + KisAbstractResourceModel::Name).toString().replace("_", " "); // don't need underscores that might be part of the file name
        painter->drawText(pixSize.width() + 10, option.rect.y() + option.rect.height() - 10, presetDisplayName.append(dirtyPresetIndicator));
    }

    if (dirty) {
        const QIcon icon = KisIconUtils::loadIcon("dirty-preset");
        QPixmap pixmap = icon.pixmap(QSize(16,16));
        painter->drawPixmap(paintRect.x() + 3, paintRect.y() + 3, pixmap);
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
    painter->restore();

}


QSize KisResourceItemDelegate::sizeHint(const QStyleOptionViewItem &optionItem, const QModelIndex &index) const
{
    return optionItem.decorationSize;
}
