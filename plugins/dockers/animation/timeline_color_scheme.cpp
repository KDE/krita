/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "timeline_color_scheme.h"

#include <QApplication>
#include <QColor>
#include <QBrush>
#include <QPalette>
#include <QFont>
#include <QFontMetrics>

#include "kis_debug.h"
#include "krita_utils.h"

#include <QGlobalStatic>
Q_GLOBAL_STATIC(TimelineColorScheme, s_instance)


TimelineColorScheme::TimelineColorScheme()
{
}

TimelineColorScheme::~TimelineColorScheme()
{
}

TimelineColorScheme* TimelineColorScheme::instance()
{
    return s_instance;
}

QColor TimelineColorScheme::selectorColor() const
{
    return QColor(223, 148, 51);
}

QColor TimelineColorScheme::selectionColor() const
{
    //return qApp->palette().color(QPalette::Highlight);
    return selectorColor();
}

QColor TimelineColorScheme::activeLayerBackground() const
{
    QColor color =  qApp->palette().color(QPalette::Highlight);
    return color;
}

QBrush TimelineColorScheme::headerEmpty() const
{
    return qApp->palette().brush(QPalette::Button);
}

QBrush TimelineColorScheme::headerCachedFrame() const
{
    QColor bgColor = qApp->palette().color(QPalette::Base);
    int darkenCoeff = bgColor.value() > 128 ? 150 : 50;
    return headerEmpty().color().darker(darkenCoeff);
}

QBrush TimelineColorScheme::headerActive() const
{
    return selectorColor();
}

QColor TimelineColorScheme::onionSkinsSliderEnabledColor() const
{
    return qApp->palette().color(QPalette::Highlight);
}

QColor TimelineColorScheme::onionSkinsSliderDisabledColor() const
{
    return qApp->palette().color(QPalette::Disabled, QPalette::HighlightedText);
}

QColor TimelineColorScheme::onionSkinsButtonColor() const
{
    QColor bgColor = qApp->palette().color(QPalette::Base);
    const int lighterCoeff = bgColor.value() > 128 ? 120 : 80;
    return qApp->palette().color(QPalette::Highlight).lighter(lighterCoeff);
}

QFont TimelineColorScheme::getOnionSkinsFont(const QString &maxString, const QSize &availableSize) const
{
    QFont font = qApp->font();

    while(font.pointSize() > 8) {
        QFontMetrics fm(font);

        QRect rc = fm.boundingRect(maxString);

        if (rc.width() > availableSize.width() ||
            rc.height() > availableSize.height()) {

            font.setPointSize(font.pointSize() - 1);
        } else {
            break;
        }
    }

    return font;
}
