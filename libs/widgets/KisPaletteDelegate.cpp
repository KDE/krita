/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QPen>
#include <QPainter>
#include <QtMath> // qBound

#include <kis_global.h>
#include <KisPaletteModel.h>
#include "kis_debug.h"

#include "KisPaletteDelegate.h"

const int KisPaletteDelegate::BORDER_WIDTH = 3;

KisPaletteDelegate::KisPaletteDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{ }

KisPaletteDelegate::~KisPaletteDelegate()
{ }

void KisPaletteDelegate::paintCrossedLine(const QStyleOptionViewItem &option, QPainter *painter) const
{
    QRect crossRect = kisGrowRect(option.rect, -qBound(2, option.rect.width() / 6, 4));

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::white, 2.5));
    painter->drawLine(crossRect.topLeft(), crossRect.bottomRight());
    painter->setPen(QPen(Qt::red, 1.0));
    painter->drawLine(crossRect.topLeft(), crossRect.bottomRight());
    painter->restore();
}

void KisPaletteDelegate::paintNonCrossed(QPainter */*painter*/, const QStyleOptionViewItem &/*option*/,
                                         const QModelIndex &/*index*/, const bool /*isSelected*/) const
{
}

void KisPaletteDelegate::paintGroupName(QPainter *painter, const QStyleOptionViewItem &option,
                                        const QModelIndex &index, const bool isSelected) const
{
    QString name = qvariant_cast<QString>(index.data(Qt::DisplayRole));
    if (isSelected) {
        painter->fillRect(option.rect, option.palette.highlight());
    }
    QRect paintRect = kisGrowRect(option.rect, -BORDER_WIDTH);
    painter->setBrush(QBrush(Qt::lightGray));
    painter->drawText(paintRect, name);
}

void KisPaletteDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    painter->save();
    const bool isSelected = option.state & QStyle::State_Selected;

    if (qvariant_cast<bool>(index.data(KisPaletteModel::IsGroupNameRole))) {
        paintGroupName(painter, option, index, isSelected);
    } else {
        QRect paintRect = option.rect;
        if (isSelected) {
            painter->fillRect(option.rect, option.palette.highlight());
            paintRect = kisGrowRect(option.rect, -BORDER_WIDTH);
        }
        if (qvariant_cast<bool>(index.data(KisPaletteModel::CheckSlotRole))) {
            QBrush brush = qvariant_cast<QBrush>(index.data(Qt::BackgroundRole));
            painter->fillRect(paintRect, brush);
        } else {
            QBrush lightBrush(Qt::gray);
            QBrush darkBrush(Qt::darkGray);
            painter->fillRect(paintRect, lightBrush);
            painter->fillRect(QRect(paintRect.topLeft(), paintRect.center()), darkBrush);
            painter->fillRect(QRect(paintRect.center(), paintRect.bottomRight()), darkBrush);
        }

        QString name = qvariant_cast<QString>(index.data(Qt::DisplayRole));
        if (!m_crossedKeyword.isNull() && name.toLower().contains(m_crossedKeyword)) {
            paintCrossedLine(option, painter);
        }
    }

    painter->restore();
}

QSize KisPaletteDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &) const
{
    return option.decorationSize;
}
