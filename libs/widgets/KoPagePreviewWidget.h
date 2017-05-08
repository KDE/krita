/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Gary Cramblitt <garycramblitt@comcast.net>
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

#ifndef KO_PAGE_PREVIEW_WIDGET
#define KO_PAGE_PREVIEW_WIDGET

#include "kritawidgets_export.h"

#include <QWidget>

// Needed for building on Windows (cannot use forward declarations)
#include <KoPageLayout.h>
#include <KoColumns.h>

/// A widget to preview the KoPageLayout and KoColumns data structures.
class KRITAWIDGETS_EXPORT KoPagePreviewWidget : public QWidget {
    Q_OBJECT
public:
    explicit KoPagePreviewWidget(QWidget *parent = 0);
    ~KoPagePreviewWidget() override;

protected:
    void paintEvent(QPaintEvent *event) override;

public Q_SLOTS:
    void setPageLayout(const KoPageLayout &layout);
    void setColumns(const KoColumns &columns);

private:
    void drawPage(QPainter &painter, qreal zoom, const QRect &dimensions, bool left);

private:
    class Private;
    Private * const d;
};

#endif
