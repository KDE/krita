/*
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "IconShape.h"

#include <KoViewConverter.h>

#include <QPainter>
#include <KIcon>

IconShape::IconShape(const QString &icon)
{
    m_icon = KIcon(icon).pixmap(22);
    setSize(m_icon.size());
}

IconShape::~IconShape()
{
}

void IconShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    applyConversion(painter, converter);
    painter.drawPixmap(QRect( QPoint(0,0), m_icon.size()), m_icon);
}


