/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
 * Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KISRESOURCEITEMVIEW_H
#define KISRESOURCEITEMVIEW_H

#include <QTableView>
#include <QScroller>

#include <KisKineticScroller.h>

#include "KisIconToolTip.h"

class QEvent;
class QModelIndex;

#include "kritaresourcewidgets_export.h"

/// The resource view
class KRITARESOURCEWIDGETS_EXPORT KisResourceItemView : public QTableView
{
    Q_OBJECT

public:

    enum ViewMode {
        FIXED_COLUMNS,  /// The number of columns is fixed
        FIXED_ROWS     /// The number of rows is fixed
    };

    explicit KisResourceItemView(QWidget *parent = 0);
    ~KisResourceItemView() override { disconnect(); }

public Q_SLOTS:
    void slotScrollerStateChange(QScroller::State state){ KisKineticScroller::updateCursor(this, state); }

Q_SIGNALS:

    void sigSizeChanged();

Q_SIGNALS:

    void currentResourceChanged(const QModelIndex &);
    void currentResourceClicked(const QModelIndex &);

    void contextMenuRequested(const QPoint &);

protected:

    friend class KisResourceItemChooser;

    void contextMenuEvent(QContextMenuEvent *event) override;
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

    void mousePressEvent(QMouseEvent *event) override;
    bool viewportEvent(QEvent *event) override;

    /**
     * This will draw a number of rows based on the number of columns if m_viewMode is FIXED_COLUMNS
     * And it will draw a number of columns based on the number of rows if m_viewMode is FIXED_ROWS
     */
    void resizeEvent(QResizeEvent *event) override;

    void setViewMode(ViewMode mode);

    void updateView();


private Q_SLOTS:
    void slotItemClicked(const QModelIndex &index);

private:
    KisIconToolTip m_tip;
    QModelIndex m_beforeClickIndex;
    ViewMode m_viewMode;
};

#endif // KORESOURCEITEMVIEW_H
