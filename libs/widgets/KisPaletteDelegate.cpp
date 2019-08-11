/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    painter->save();

    if (!index.isValid())
        return;

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
