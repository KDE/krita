/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Label.h"

#include <KoViewConverter.h>

#include <KDebug>

#include <QFontMetrics>
#include <QPainter>


Label::Label(QObject *parent)
    : QObject(parent)
{}

Label::~Label()
{}

void Label::setColor(const QColor& color)
{
    m_color = color;
}

void Label::setText(const QString& text)
{
    m_text = text;
}

void Label::setPosition(const QPointF& position, Qt::Alignment alignment)
{
    m_position = position;
    m_alignment = alignment;
}

QRectF Label::boundingRect() const
{
    QFontMetrics metrics = QFontMetrics(QFont());

    QRectF bRect = metrics.boundingRect(m_text);
    QPointF new_position = m_position;
    new_position.rx() -= bRect.width()/2.0;
    new_position.ry() -= bRect.height()/2.0;
    bRect.moveTo(new_position);
    bRect.adjust(-4.0, 0.0, 4.0, 0.0);

    return bRect;
}

void Label::paint(QPainter &painter) const
{
    painter.save();

    painter.setBrush(Qt::white);
    painter.setPen(m_color);

    QRectF bRect = boundingRect();

    qreal halfWidth = bRect.width() / 2.0;
    if (m_alignment & Qt::AlignRight) {
        bRect.translate(-halfWidth, 0.0);
    } else if (m_alignment & Qt::AlignLeft) {
        bRect.translate(halfWidth, 0.0);
    }

    qreal halfHeight = bRect.height() / 2.0;
    if  (m_alignment & Qt::AlignTop) {
        bRect.translate(0.0, -halfHeight);
    } else if (m_alignment & Qt::AlignBottom) {
        bRect.translate(0.0, halfHeight);
    }

    painter.drawRoundRect(bRect, 720.0 / bRect.width(), 720.0 /  bRect.height());
    painter.drawText(bRect, Qt::AlignHCenter | Qt::AlignVCenter, m_text);

    painter.restore();
}

