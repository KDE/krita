/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoCheckerBoardPainter.h"
#include <QPainter>

KoCheckerBoardPainter::KoCheckerBoardPainter( int checkerSize )
: m_checkerSize(checkerSize), m_lightColor( Qt::lightGray ), m_darkColor( Qt::darkGray )
{
    createChecker();
}

void KoCheckerBoardPainter::setCheckerColors( const QColor &lightColor, const QColor &darkColor )
{
    m_lightColor = lightColor;
    m_darkColor = darkColor;
    createChecker();
}

void KoCheckerBoardPainter::setCheckerSize( int checkerSize )
{
    m_checkerSize = checkerSize;
    createChecker();
}

void KoCheckerBoardPainter::paint( QPainter &painter, const QRectF &rect ) const
{
    painter.fillRect( rect, QBrush(m_checker));
}

void KoCheckerBoardPainter::createChecker()
{
    m_checker = QPixmap( 2*m_checkerSize, 2*m_checkerSize );
    QPainter p(&m_checker);
    p.fillRect(0, 0, m_checkerSize, m_checkerSize, m_lightColor);
    p.fillRect(m_checkerSize, 0, m_checkerSize, m_checkerSize, m_darkColor);
    p.fillRect(0, m_checkerSize, m_checkerSize, m_checkerSize, m_darkColor);
    p.fillRect(m_checkerSize, m_checkerSize, m_checkerSize, m_checkerSize, m_lightColor);
    p.end();
}
