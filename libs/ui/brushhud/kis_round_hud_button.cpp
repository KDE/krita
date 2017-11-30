/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_round_hud_button.h"

#include <QPaintEvent>
#include <QPainter>

#include "kis_global.h"



struct KisRoundHudButton::Private
{
    Private() : isHighlighted(false) {}

    bool isHighlighted;
    QIcon onIcon;
    QIcon offIcon;
};

KisRoundHudButton::KisRoundHudButton(QWidget *parent)
    : QAbstractButton(parent),
      m_d(new Private)
{
    setMouseTracking(true);
}

KisRoundHudButton::~KisRoundHudButton()
{
}

void KisRoundHudButton::setOnOffIcons(const QIcon &on, const QIcon &off)
{
    m_d->onIcon = on;
    m_d->offIcon = off;
}

void KisRoundHudButton::paintEvent(QPaintEvent */*event*/)
{
    const int borderWidth = 3;
    const QPointF center = QRectF(rect()).center();
    const qreal radius = 0.5 * (center.x() + center.y()) - borderWidth;

    const QPen fgPen(palette().color(m_d->isHighlighted ? QPalette::Highlight : QPalette::WindowText), borderWidth);
    const QBrush bgBrush(palette().brush(isDown() || (isCheckable() && isChecked()) ? QPalette::Mid : QPalette::Window));

    QPainter painter(this);
    painter.setPen(fgPen);
    painter.setBrush(bgBrush);
    painter.setRenderHints(QPainter::Antialiasing);

    painter.drawEllipse(center, radius, radius);

    if (!icon().isNull()) {
        const QIcon::Mode mode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
        const QIcon::State state = isCheckable() && isChecked() ? QIcon::On : QIcon::Off;
        const QSize size = iconSize();

        QPixmap pixmap = icon().pixmap(size, mode, state);

        QPointF iconOffset(0.5 * (width() - size.width()),
                           0.5 * (height() - size.height()));

        painter.drawPixmap(iconOffset, pixmap);
    }

    if (!m_d->onIcon.isNull()) {
        const QIcon::Mode mode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
        const QIcon icon = isCheckable() && isChecked() ? m_d->onIcon : m_d->offIcon;
        const QSize size = iconSize();

        QPixmap pixmap = icon.pixmap(size, mode);
        QPointF iconOffset(0.5 * (width() - size.width()),
                           0.5 * (height() - size.height()));

        painter.drawPixmap(iconOffset, pixmap);
    }
}

bool KisRoundHudButton::hitButton(const QPoint &pos) const
{
    const int borderWidth = 3;
    const QPointF center = QRectF(rect()).center();
    const qreal radius = 0.5 * (center.x() + center.y()) - borderWidth;

    return kisDistance(center, pos) < radius;
}

void KisRoundHudButton::mouseMoveEvent(QMouseEvent *event)
{
    m_d->isHighlighted = hitButton(event->pos());
    QAbstractButton::mouseMoveEvent(event);
}

void KisRoundHudButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_d->isHighlighted = false;

    QAbstractButton::leaveEvent(event);
}
