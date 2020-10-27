/* This file is part of the KDE project
 * Copyright (C) 2019 Wolthera van Hövell tot Westerflier<griffinvalley@gmail.com>
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

#ifndef KISSTORAGECHOOSERWIDGET_H
#define KISSTORAGECHOOSERWIDGET_H

#include <QWidget>
#include <QAbstractItemDelegate>
#include <KisPopupButton.h>

#include "kritaresourcewidgets_export.h"

class KRITARESOURCEWIDGETS_EXPORT KisStorageChooserDelegate  : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    explicit KisStorageChooserDelegate(QObject *parent = 0);
    ~KisStorageChooserDelegate() override {}

    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

    QSize sizeHint ( const QStyleOptionViewItem &, const QModelIndex & ) const override;

};


class KRITARESOURCEWIDGETS_EXPORT KisStorageChooserWidget : public KisPopupButton
{
    Q_OBJECT
public:
    KisStorageChooserWidget(QWidget *parent = 0);

    ~KisStorageChooserWidget();

private Q_SLOTS:
    void activated(const QModelIndex &index);

};


#endif // KISSTORAGECHOOSERWIDGET_H
