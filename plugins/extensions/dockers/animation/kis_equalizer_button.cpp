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

#include "kis_equalizer_button.h"

#include <QStyle>
#include <QPainter>
#include <QStyleOption>

#include <QApplication>


#include "timeline_color_scheme.h"
#include "kis_global.h"
#include "kis_debug.h"

struct KisEqualizerButton::Private
{
    Private(KisEqualizerButton *_q)
        : q(_q),
          isRightmost(false),
          isHovering(false) {}

    QRect boundingRect() const;
    QRect fillingRect() const;

    KisEqualizerButton *q;
    bool isRightmost;
    bool isHovering;
};

KisEqualizerButton::KisEqualizerButton(QWidget *parent)
    : QAbstractButton(parent),
      m_d(new Private(this))
{
    setFocusPolicy(Qt::WheelFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

KisEqualizerButton::~KisEqualizerButton()
{
}

void KisEqualizerButton::setRightmost(bool value)
{
    m_d->isRightmost = value;
}

QRect KisEqualizerButton::Private::boundingRect() const
{
    QRect bounds = q->rect().adjusted(0, 0, -isRightmost, 0);
    return bounds;
}

QRect KisEqualizerButton::Private::fillingRect() const
{
    const int offset = 3;
    QRect filling = boundingRect().adjusted(offset + 1, offset + 1,
                                            -offset, -offset);

    return filling;
}

void KisEqualizerButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    const QRect bounds = m_d->boundingRect();
    const QRect filling = m_d->fillingRect();
    const QColor backgroundColor = palette().color(QPalette::Base);

    QPainter p(this);

    { // draw border

        QStyleOptionViewItemV4 option; // empty!
        const int gridHint = style()->styleHint(QStyle::SH_Table_GridLineColor, &option, this);
        const QColor gridColor = static_cast<QRgb>(gridHint);
        const QPen gridPen(gridColor);

        p.setPen(gridPen);
        p.setBrush(backgroundColor);
        p.drawRect(bounds);
    }

    {
        QColor fillColor = TimelineColorScheme::instance()->onionSkinsButtonColor();
        QColor frameColor = TimelineColorScheme::instance()-> onionSkinsSliderColor();

        if (isChecked() || hasFocus() || m_d->isHovering) {
            p.setPen(hasFocus() || m_d->isHovering ? frameColor : Qt::transparent);
            p.setBrush(isChecked() ? fillColor : Qt::transparent);
            p.drawRect(filling);
        }
    }

    QString textValue = text();

    { // draw text
        QPalette::ColorRole textRole = QPalette::Text;

        //Draw text shadow, This will increase readability when the background of same color
        QRect shadowRect(bounds);
        shadowRect.translate(1,1);
        QColor textColor = palette().color(textRole);
        QColor shadowColor = (textColor.value() <= 128)
            ? QColor(255,255,255,160) : QColor(0,0,0,160);

        int flags = Qt::AlignCenter | Qt::TextHideMnemonic;

        p.setPen(shadowColor);
        p.drawText(shadowRect, flags, textValue);

        p.setPen(textColor);
        p.drawText(bounds, flags, textValue);
    }
}

QSize KisEqualizerButton::sizeHint() const
{
    QFontMetrics metrics(this->font());
    const int minHeight = metrics.height() + 10;
    return QSize(15, minHeight);
}

QSize KisEqualizerButton::minimumSizeHint() const
{
    QSize sh = sizeHint();
    return QSize(10, sh.height());
}

void KisEqualizerButton::enterEvent(QEvent *event)
{
    Q_UNUSED(event);

    m_d->isHovering = true;
    update();
}

void KisEqualizerButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);

    m_d->isHovering = false;
    update();
}
