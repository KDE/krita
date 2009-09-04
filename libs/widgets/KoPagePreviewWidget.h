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

#ifndef KOFFICE_PAGE_PREVIEW_WIDGET
#define KOFFICE_PAGE_PREVIEW_WIDGET

#include "kowidgets_export.h"

#include <KoPageLayout.h>

#include <QWidget>

/// A widget to preview the KoPageLayout and KoColumns data structures.
class KOWIDGETS_EXPORT KoPagePreviewWidget : public QWidget {
    Q_OBJECT
public:
    KoPagePreviewWidget(QWidget *parent = 0);
    ~KoPagePreviewWidget();

protected:
    void paintEvent(QPaintEvent *event);

public slots:
    void setPageLayout(const KoPageLayout &layout);
    void setColumns(const KoColumns &columns);

private:
    void drawPage(QPainter &painter, qreal zoom, const QRect &dimensions, bool left);

private:
    class Private;
    Private * const d;
};

#endif
