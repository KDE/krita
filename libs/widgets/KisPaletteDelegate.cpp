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

#include "KisPaletteDelegate.h"

#include <QPen>
#include <QPainter>

#include <kis_global.h>
#include "kis_debug.h"
#include <KisPaletteModel.h>


KisPaletteDelegate::KisPaletteDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{ }

KisPaletteDelegate::~KisPaletteDelegate()
{ }

void KisPaletteDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    painter->save();

    if (!index.isValid())
        return;

    const int minSize = qMin(option.rect.width(), option.rect.height());
    const bool isSelected = option.state & QStyle::State_Selected;
    const int borderWidth = 1;

    if (qvariant_cast<bool>(index.data(KisPaletteModel::IsHeaderRole))) {
        QString name = qvariant_cast<QString>(index.data(Qt::DisplayRole));
        if (isSelected) {
            painter->fillRect(option.rect, option.palette.highlight());
        }
        QRect paintRect = kisGrowRect(option.rect, -borderWidth);
        painter->drawText(paintRect, name);
    } else {
        if (isSelected) {
            painter->fillRect(option.rect, option.palette.highlight());
        } else {
            option.palette.background();
        }
        QRect paintRect = kisGrowRect(option.rect, -borderWidth);
        QBrush brush = qvariant_cast<QBrush>(index.data(Qt::BackgroundRole));
        painter->fillRect(paintRect, brush);
    }

    painter->restore();
}

QSize KisPaletteDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &) const
{
    return option.decorationSize;
}
