/* This file is part of the KDE project
 * Copyright (C) 2009 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#include "MusicWidget.h"

#include <QPainter>

#include "../core/Sheet.h"

MusicWidget::MusicWidget(QWidget* parent)
    : QWidget(parent), m_renderer(&m_style), m_sheet(0), m_scale(1.0), m_lastSystem(0)
{
}

void MusicWidget::setSheet(MusicCore::Sheet* sheet)
{
    m_sheet = sheet;
    engrave();
}

MusicCore::Sheet* MusicWidget::sheet() const
{
    return m_sheet;
}

void MusicWidget::setScale(qreal scale)
{
    m_scale = scale;
    engrave();
}

qreal MusicWidget::scale() const
{
    return m_scale;
}

void MusicWidget::engrave()
{
    if (m_sheet) {
        m_engraver.engraveSheet(m_sheet, 0, QSizeF((width() - 1) / m_scale, height() / m_scale), true, &m_lastSystem);
    }
}

void MusicWidget::paintEvent(QPaintEvent*)
{
    if (!m_sheet) return;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);

    painter.scale(m_scale, m_scale);
    m_renderer.renderSheet(painter, m_sheet, 0, m_lastSystem);
}

void MusicWidget::resizeEvent(QResizeEvent*)
{
    engrave();
}
