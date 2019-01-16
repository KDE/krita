/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KISGRIDVIEW_H
#define KISGRIDVIEW_H

#include <QObject>
#include <QWidget>
#include <QAbstractItemView>
#include <QScopedPointer>

#include <kritaresourcewidgets_export.h>

class KRITARESOURCEWIDGETS_EXPORT KisGridView : public QAbstractItemView
{
public:
    KisGridView(QWidget *parent = 0);
    ~KisGridView() override;

    QModelIndex indexAt(const QPoint &point) const override;
    void scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint = EnsureVisible) override;
    QRect visualRect(const QModelIndex &index) const override;

protected:

    void paintEvent(QPaintEvent *event) override;

    int horizontalOffset() const override;
    bool isIndexHidden(const QModelIndex &index) const override;
    QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags flags) override;
    int verticalOffset() const override;
    QRegion visualRegionForSelection(const QItemSelection &selection) const override;

private:
    struct Private;
    const QScopedPointer<Private> d;

};

#endif // KISGRIDVIEW_H
