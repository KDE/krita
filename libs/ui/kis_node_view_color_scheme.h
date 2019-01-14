/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_NODE_VIEW_COLOR_SCHEME_H
#define __KIS_NODE_VIEW_COLOR_SCHEME_H

#include <QScopedPointer>
#include <QColor>

#include "kritaui_export.h"

class QTreeView;
class QStyleOptionViewItem;
class QRect;

class KRITAUI_EXPORT KisNodeViewColorScheme
{
public:
    KisNodeViewColorScheme();
    ~KisNodeViewColorScheme();

    static KisNodeViewColorScheme* instance();

    QColor gridColor(const QStyleOptionViewItem &option, QTreeView *view);

    int visibilitySize() const;
    int visibilityMargin() const;

    int thumbnailSize() const;
    int thumbnailMargin() const;

    int decorationSize() const;
    int decorationMargin() const;

    int textMargin() const;

    int iconSize() const;
    int iconMargin() const;

    int border() const;

    int rowHeight() const;
    int visibilityColumnWidth() const;
    int indentation() const;

    QRect relVisibilityRect() const;
    QRect relThumbnailRect() const;
    QRect relDecorationRect() const;
    QRect relExpandButtonRect() const;

    QColor colorLabel(int index) const;
    QVector<QColor> allColorLabels() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_NODE_VIEW_COLOR_SCHEME_H */
