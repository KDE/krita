/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
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
#include "StaffElementPreviewWidget.h"

#include "../MusicStyle.h"
#include "../Renderer.h"

#include "../core/Staff.h"
#include "../core/Clef.h"

#include <QPainter>
#include <QBrush>

using namespace MusicCore;

StaffElementPreviewWidget::StaffElementPreviewWidget(QWidget* parent)
    : QWidget(parent), m_style(0), m_renderer(0)
{
    m_staff = new Staff(0);
    m_clef = new Clef(m_staff, 0, Clef::Trebble, 2, 0);
}

StaffElementPreviewWidget::~StaffElementPreviewWidget()
{
    if (m_renderer) delete m_renderer;
}

void StaffElementPreviewWidget::setMusicStyle(MusicStyle* style)
{
    m_style = style;
    if (m_renderer) delete m_renderer;
    m_renderer = new MusicRenderer(m_style);
}

QSize StaffElementPreviewWidget::sizeHint() const
{
    return QSize(180, 75);
}

void StaffElementPreviewWidget::setStaffElement(MusicCore::StaffElement* se)
{
    m_element = se;
    update();
}

Staff* StaffElementPreviewWidget::staff()
{
    return m_staff;
}

void StaffElementPreviewWidget::paintEvent(QPaintEvent * event)
{
    Q_UNUSED( event );
    QPainter painter(this);
    painter.fillRect(rect(), QBrush(Qt::white));
    if (!m_style) return;
    painter.translate(0, height() / 2);
     
    painter.scale(1.5, 1.5);
    painter.setPen(m_style->staffLinePen());
    for (int i = -2; i <= 2; i++) {
        painter.drawLine(QPointF(0, 5.0 * i), QPointF(width(), 5.0 * i));
    }
     
    m_style->renderClef(painter, 5, 5.0 /* staff line distance */, MusicCore::Clef::Trebble);
     
    MusicRenderer::RenderState state;
    state.clef = m_clef;
    m_renderer->renderStaffElement(painter, m_element, QPointF(20.0 + m_clef->width(), -10.0), state);
}


#include <StaffElementPreviewWidget.moc>
