/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    bool isOver = hitButton(event->pos());
    if (isOver != m_d->isHighlighted) {
        m_d->isHighlighted = isOver;
        update();
    }
    
    QAbstractButton::mouseMoveEvent(event);
}

void KisRoundHudButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (m_d->isHighlighted) {
        m_d->isHighlighted = false;
        update();
    }

    QAbstractButton::leaveEvent(event);
}
