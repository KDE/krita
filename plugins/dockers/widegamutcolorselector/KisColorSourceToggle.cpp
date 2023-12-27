/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisColorSourceToggle.h"

#include <QPainter>

class KisColorSourceToggle::Private
{
public:
    QColor foregroundColor;
    QColor backgroundColor;
};

KisColorSourceToggle::KisColorSourceToggle(QWidget *parent)
    : QAbstractButton(parent)
    , m_d(new Private)
{
    setCheckable(true);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

KisColorSourceToggle::~KisColorSourceToggle()
{}

void KisColorSourceToggle::setForegroundColor(const QColor &color)
{
    m_d->foregroundColor = color;
    update();
}

void KisColorSourceToggle::setBackgroundColor(const QColor &color)
{
    m_d->backgroundColor = color;
    update();
}

void KisColorSourceToggle::paintEvent(QPaintEvent */*e*/)
{
    int length = qMin(width(), height());
    int patchSize = length * 6 / 10;
    QRect fgRect(1, 1, patchSize, patchSize);
    QRect bgRect(length - patchSize - 2, length - patchSize - 2, patchSize, patchSize);
    QPainter painter(this);
    painter.translate(0.5, 0.5);
    painter.setRenderHint(QPainter::Antialiasing);
    if (isChecked()) {
        // inactive patch (lowered)
        painter.setPen(QPen(palette().shadow(), 1));
        painter.setBrush(m_d->foregroundColor);
        painter.drawRect(fgRect);
        // active patch (raised & highlighted)
        painter.setPen(QPen(palette().highlightedText(), 3));
        painter.setBrush(m_d->backgroundColor);
        painter.drawRect(bgRect);
        painter.setPen(QPen(palette().shadow(), 1));
        painter.drawRect(bgRect);
    }
    else {
        // inactive patch (lowered)
        painter.setPen(QPen(palette().shadow(), 1));
        painter.setBrush(m_d->backgroundColor);
        painter.drawRect(bgRect);
        // active patch (raised & highlighted)
        painter.setPen(QPen(palette().highlightedText(), 3));
        painter.setBrush(m_d->foregroundColor);
        painter.drawRect(fgRect);
        painter.setPen(QPen(palette().shadow(), 1));
        painter.drawRect(fgRect);
    }
}

QSize KisColorSourceToggle::sizeHint() const
{
    return QSize(24, 24);
}
