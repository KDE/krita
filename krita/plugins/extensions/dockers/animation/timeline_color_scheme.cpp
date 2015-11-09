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

#include "timeline_color_scheme.h"

#include <QApplication>
#include <QColor>
#include <QBrush>
#include <QPalette>

#include "kis_debug.h"

#include <QGlobalStatic>
Q_GLOBAL_STATIC(TimelineColorScheme, s_instance)


struct TimelineColorScheme::Private
{
    QColor baseColor;
};


TimelineColorScheme::TimelineColorScheme()
    : m_d(new Private)
{
    m_d->baseColor = QColor(137, 192, 221);
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
    QColor bgColor = qApp->palette().color(QPalette::Base);

    int darkenCoeff = bgColor.value() < 128 ? 130 : 80;
    return color;
}

QBrush TimelineColorScheme::headerEmpty() const
{
    return qApp->palette().brush(QPalette::Button);
}

QBrush TimelineColorScheme::headerCachedFrame() const
{
    return headerEmpty().color().darker(115);
}

QBrush TimelineColorScheme::headerActive() const
{
    return selectorColor();
}

inline QColor blendColors(const QColor &c1, const QColor &c2, qreal r1) {
    const qreal r2 = 1.0 - r1;

    return QColor::fromRgbF(
        c1.redF() * r1 + c2.redF() * r2,
        c1.greenF() * r1 + c2.greenF() * r2,
        c1.blueF() * r1 + c2.blueF() * r2);
}

QColor TimelineColorScheme::frameColor(bool present, bool active)
{
    QColor color = Qt::transparent;

    if (present && !active) {
        color = m_d->baseColor;
    } else if (present && active) {
        QColor bgColor = qApp->palette().color(QPalette::Base);
        int darkenCoeff = bgColor.value() > 128 ? 130 : 80;
        color = m_d->baseColor.darker(darkenCoeff);
    } else if (!present && active) {
        QColor bgColor = qApp->palette().color(QPalette::Base);
        return blendColors(m_d->baseColor, bgColor, 0.2);
    }

    return color;
}

