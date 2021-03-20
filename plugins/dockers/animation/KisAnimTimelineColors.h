/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __TIMELINE_COLOR_SCHEME_H
#define __TIMELINE_COLOR_SCHEME_H

#include <QScopedPointer>

class QColor;
class QBrush;
class QFont;
class QSize;

class KisAnimTimelineColors
{
public:
    KisAnimTimelineColors();
    ~KisAnimTimelineColors();

    static KisAnimTimelineColors* instance();

    QColor selectorColor() const;
    QColor selectionColor() const;
    QColor activeLayerBackground() const;


    QBrush headerEmpty() const;
    QBrush headerCachedFrame() const;
    QBrush headerActive() const;

    QColor onionSkinsSliderEnabledColor() const;
    QColor onionSkinsSliderDisabledColor() const;
    QColor onionSkinsButtonColor() const;

    QFont getOnionSkinsFont(const QString &maxString, const QSize &availableSize) const;
};

#endif /* __TIMELINE_COLOR_SCHEME_H */
