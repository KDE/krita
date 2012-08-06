// vim: set tabstop=4 shiftwidth=4 noexpandtab:
/* This file is part of the KDE project
Copyright 2011 Aurélien Gâteau <agateau@kde.org>
Copyright 2011 Paul Mendez <paulestebanms@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.

*/

#include "KoContextBarButton.h"

// KDE
#include <kiconloader.h>
#include <KIcon>
#include <KGlobalSettings>

// Qt
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QPainterPath>

/** How lighter is the border of context bar buttons */
const int CONTEXTBAR_BORDER_LIGHTNESS = 140;

/** How darker is the background of context bar buttons */
const int CONTEXTBAR_BACKGROUND_DARKNESS = 80;

/** How lighter are context bar buttons when under mouse */
const int CONTEXTBAR_MOUSEOVER_LIGHTNESS = 120;

/** Radius of ContextBarButtons */
const int CONTEXTBAR_RADIUS = 50;

KoContextBarButton::KoContextBarButton(const QString &iconName, QWidget* parent)
: QToolButton(parent)
, m_isHovered(false)
, m_leftMouseButtonPressed(false)
, m_fadingValue(0)
, m_fadingTimeLine(0)
{
    const int size = IconSize(KIconLoader::Small);
    setIconSize(QSize(size, size));
    setAutoRaise(true);
    setIcon(KIcon(iconName));
}


KoContextBarButton::~KoContextBarButton()
{
}


void KoContextBarButton::paintEvent(QPaintEvent*)
{
    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    const QColor bgColor = palette().color(QPalette::Highlight);
    QColor color = bgColor.dark(CONTEXTBAR_BACKGROUND_DARKNESS);
    QColor borderColor = bgColor.light(CONTEXTBAR_BORDER_LIGHTNESS);

    if (opt.state & QStyle::State_MouseOver && opt.state & QStyle::State_Enabled) {
            color = color.light(CONTEXTBAR_MOUSEOVER_LIGHTNESS);
            borderColor = borderColor.lighter(CONTEXTBAR_MOUSEOVER_LIGHTNESS);
    }

    const QRectF rectF = QRectF(opt.rect).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainterPath path;
    path.addRoundRect(rectF, CONTEXTBAR_RADIUS, CONTEXTBAR_RADIUS);

    if (m_fadingValue < 255) {
        color.setAlpha(m_fadingValue);
    }

    // Background
    painter.fillPath(path, color);

    if (opt.state & QStyle::State_Raised && opt.state & QStyle::State_Enabled) {
        // Bottom shadow
        QLinearGradient gradient(rectF.bottomLeft(), rectF.bottomLeft() - QPoint(0, 5));
        gradient.setColorAt(0, QColor::fromHsvF(0, 0, 0, .3));
        gradient.setColorAt(1, Qt::transparent);
        painter.fillPath(path, gradient);

        // Left shadow
        gradient.setFinalStop(rectF.bottomLeft() + QPoint(3, 0));
        painter.fillPath(path, gradient);
    }
    else {
        // Top shadow
        QLinearGradient gradient(rectF.topLeft(), rectF.topLeft() + QPoint(0, 5));
        gradient.setColorAt(0, QColor::fromHsvF(0, 0, 0, .3));
        gradient.setColorAt(1, Qt::transparent);
        painter.fillPath(path, gradient);

        // Left shadow
        gradient.setFinalStop(rectF.topLeft() + QPoint(5, 0));
        painter.fillPath(path, gradient);
    }

    // Border
    painter.setPen(borderColor);
    painter.drawPath(path);

    // Content
    painter.drawControl(QStyle::CE_ToolButtonLabel, opt);
}

void KoContextBarButton::startFading()
{
    Q_ASSERT(!m_fadingTimeLine);

    const bool animate = KGlobalSettings::graphicEffectsLevel() & KGlobalSettings::SimpleAnimationEffects;
    const int duration = animate ? 300 : 1;

    m_fadingTimeLine = new QTimeLine(duration, this);
    connect(m_fadingTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(setFadingValue(int)));
    m_fadingTimeLine->setFrameRange(0, 255);
    m_fadingTimeLine->start();
    m_fadingValue = 0;
}

void KoContextBarButton::stopFading()
{
    if (m_fadingTimeLine) {
        m_fadingTimeLine->stop();
        delete m_fadingTimeLine;
        m_fadingTimeLine = 0;
    }
    m_fadingValue = 0;
}

void KoContextBarButton::enterEvent(QEvent *event)
{
    QToolButton::enterEvent(event);

    // if the mouse cursor is above the selection toggle, display
    // it immediately without fading timer
    m_isHovered = true;
    if (m_fadingTimeLine) {
        m_fadingTimeLine->stop();
    }
    m_fadingValue = 255;
    update();
}

void KoContextBarButton::leaveEvent(QEvent *event)
{
    QToolButton::leaveEvent(event);

    m_isHovered = false;
    update();
}

void KoContextBarButton::setFadingValue(int value)
{
    m_fadingValue = value;
    if (m_fadingValue >= 255) {
        Q_ASSERT(m_fadingTimeLine);
        m_fadingTimeLine->stop();
    }
    update();
}

void KoContextBarButton::showEvent(QShowEvent *event)
{
    stopFading();
    startFading();
    QToolButton::showEvent(event);
}

void KoContextBarButton::hideEvent(QHideEvent *event)
{
    stopFading();
    QToolButton::hideEvent(event);
}
// Self
#include "KoContextBarButton.moc"
