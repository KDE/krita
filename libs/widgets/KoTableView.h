/*
 * Copyright (C) 2015 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KOTABLEVIEW_H
#define KOTABLEVIEW_H

#include <QTableView>

#include "kritawidgets_export.h"

class QEvent;
class QModelIndex;

/**
 * @brief The KoTableView class provides a QTableView with fixed columns or rows
 */
class KRITAWIDGETS_EXPORT KoTableView: public QTableView
{
    Q_OBJECT

public:
    enum ViewMode {
        FIXED_COLUMNS,  /// The number of columns is fixed
        FIXED_ROWS     /// The number of rows is fixed
    };

    explicit KoTableView(QWidget *parent = 0);
    ~KoTableView() override {}

    /** reimplemented
    * This will draw a number of rows based on the number of columns if m_viewMode is FIXED_COLUMNS
    * And it will draw a number of columns based on the number of rows if m_viewMode is FIXED_ROWS
    */
    void resizeEvent(QResizeEvent *event) override;


    void setViewMode(ViewMode mode);

    void updateView();

Q_SIGNALS:

    void sigSizeChanged();

private:
    ViewMode m_viewMode;
};

#endif // KOTABLEVIEW_H
