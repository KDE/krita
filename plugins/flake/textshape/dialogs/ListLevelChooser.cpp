/* This file is part of the KDE project
 * Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#include "ListLevelChooser.h"

#include <QPushButton>
#include <QPainter>
#include <QDebug>

ListLevelChooser::ListLevelChooser(const int offset, QWidget *parent)
    : QPushButton("", parent)
    , m_offset(offset)
{
    setFlat(true);
    setMinimumSize(QSize(256, 20));
}

void ListLevelChooser::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPushButton::paintEvent(event);
    QPainter painter(this);
    painter.save();
    painter.setPen(QPen(painter.pen().brush(), 1, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin));
    QRect rectang = rect();
    //painter.fillRect(rectang, QBrush(QColor(Qt::white)));
    painter.translate(m_offset, 1.5);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawText(rectang, Qt::AlignVCenter, QString::fromUtf8("●"));

    int lineY = rectang.y() + (rectang.height() / 2);
    painter.drawLine(13, lineY, rectang.bottomRight().x() - m_offset - 15, lineY);

    painter.restore();
}
