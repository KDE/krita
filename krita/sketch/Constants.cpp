/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#include "Constants.h"

#include <QApplication>
#include <QWidget>

Constants::Constants(QObject* parent)
    : QObject(parent)
{
    m_gridWidth = qApp->activeWindow()->width() / gridColumns();
    m_gridHeight = qApp->activeWindow()->height() / gridHeight();
    m_toolbarButtonSize = m_gridHeight;
}

int Constants::animationDuration() const
{
    return 150;
}

qreal Constants::gridHeight() const
{
    return m_gridHeight;
}

void Constants::setGridHeight( qreal height )
{
    m_gridHeight = height;
    m_toolbarButtonSize = m_gridHeight;
    emit gridSizeChanged();
}

qreal Constants::gridWidth() const
{
    return m_gridWidth;
}

void Constants::setGridWidth( qreal width )
{
    m_gridWidth = width;
    m_toolbarButtonSize = m_gridHeight;
    emit gridSizeChanged();
}

qreal Constants::toolbarButtonSize() const
{
    return m_toolbarButtonSize;
}

int Constants::gridRows() const
{
    if (isLandscape())
        return 18;
    return 12;
}

int Constants::gridColumns() const
{
    return 12;
}

qreal Constants::defaultMargin() const
{
    return 0.1 * m_gridHeight;
}

qreal Constants::smallFontSize() const
{
    return 0.2 * m_gridHeight;
}

qreal Constants::defaultFontSize() const
{
    return 0.3 * m_gridHeight;
}

qreal Constants::largeFontSize() const
{
    return 0.4 * m_gridHeight;
}

qreal Constants::hugeFontSize() const
{
    return 0.9 * m_gridHeight;
}

bool Constants::isLandscape() const
{
	// If user switches certain settings in windows, activeWindow can become null.
	if(qApp->activeWindow())
	    return qApp->activeWindow()->height() > qApp->activeWindow()->width();
	return true;
}

