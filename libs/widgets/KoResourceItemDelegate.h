/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KORESOURCEITEMDELEGATE_H
#define KORESOURCEITEMDELEGATE_H

#include <QAbstractItemDelegate>
#include "KoCheckerBoardPainter.h"

#include "kritawidgets_export.h"

/// The resource item delegate for rendering the resource preview
class KRITAWIDGETS_EXPORT KoResourceItemDelegate : public QAbstractItemDelegate
{
public:
    explicit KoResourceItemDelegate(QObject *parent = 0);
    ~KoResourceItemDelegate() override {}
    /// reimplemented
    void paint( QPainter *, const QStyleOptionViewItem &, const QModelIndex & ) const override;
    /// reimplemented
    QSize sizeHint ( const QStyleOptionViewItem &, const QModelIndex & ) const override;
private:
    KoCheckerBoardPainter m_checkerPainter;
};

#endif // KORESOURCEITEMDELEGATE_H
