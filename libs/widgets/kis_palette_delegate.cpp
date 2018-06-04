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

#include "kis_palette_delegate.h"

#include <QPen>
#include <QPainter>

#include <kis_global.h>
#include "kis_debug.h"
#include <KisPaletteModel.h>


KisPaletteDelegate::KisPaletteDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
}

KisPaletteDelegate::~KisPaletteDelegate()
{
}

void KisPaletteDelegate::setCrossedKeyword(const QString &value)
{
    m_crossedKeyword = value;
}

void KisPaletteDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    painter->save();

    if (! index.isValid())
        return;

    const bool isSelected = option.state & QStyle::State_Selected;
    const int minSize = qMin(option.rect.width(), option.rect.height());
    const int maxWidth = qBound(2, minSize / 6, 4);
    const int width = isSelected ? maxWidth : 1;

    if (qvariant_cast<bool>(index.data(KisPaletteModel::IsHeaderRole))) {
        QString name = qvariant_cast<QString>(index.data(Qt::DisplayRole));
        if (isSelected) {
            painter->fillRect(option.rect, option.palette.highlight());
        }
        QRect paintRect = kisGrowRect(option.rect, -width);
        painter->drawText(paintRect, name);
    } else {
        if (isSelected) {
            painter->fillRect(option.rect, option.palette.highlight());
        }
        QRect paintRect = kisGrowRect(option.rect, -width);
        QBrush brush = qvariant_cast<QBrush>(index.data(Qt::BackgroundRole));
        painter->fillRect(paintRect, brush);
    }
    painter->restore();

    QString name = qvariant_cast<QString>(index.data(Qt::DisplayRole));
    if (!m_crossedKeyword.isNull() && name.toLower().contains(m_crossedKeyword)) {
        QRect crossRect = kisGrowRect(option.rect, -maxWidth);

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(Qt::white, 2.5));
        painter->drawLine(crossRect.topLeft(), crossRect.bottomRight());
        painter->setPen(QPen(Qt::red, 1.0));
        painter->drawLine(crossRect.topLeft(), crossRect.bottomRight());
        painter->restore();
    }
}

QSize KisPaletteDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const
{
    return option.decorationSize;
}
