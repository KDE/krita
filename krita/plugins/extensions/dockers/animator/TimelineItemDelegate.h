/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef TIMELINEITEMDELEGATE_H
#define TIMELINEITEMDELEGATE_H

#include <QStyledItemDelegate>

class TimelineItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit TimelineItemDelegate(QObject *parent = 0);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &optionViewItem, const QModelIndex & index) const;

};

#endif // TIMELINEITEMDELEGATE_H
