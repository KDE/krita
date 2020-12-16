/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    QColor colorFromLabelIndex(int index) const;
    QVector<QColor> allColorLabels() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_NODE_VIEW_COLOR_SCHEME_H */
