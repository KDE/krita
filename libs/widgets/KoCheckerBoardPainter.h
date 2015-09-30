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

#ifndef KOCHECKERBOARDPAINTER_H
#define KOCHECKERBOARDPAINTER_H

#include <QPixmap>
#include <QColor>
#include "kritawidgets_export.h"

class QPainter;

class KRITAWIDGETS_EXPORT KoCheckerBoardPainter
{
public:
    explicit KoCheckerBoardPainter( int checkerSize );
    void setCheckerColors( const QColor &lightColor, const QColor &darkColor );
    void setCheckerSize( int checkerSize );
    void paint( QPainter &painter, const QRectF &rect ) const;

private:
    void createChecker();
    int m_checkerSize;
    QPixmap m_checker;
    QColor m_lightColor;
    QColor m_darkColor;
};

#endif // KOCHECKERBOARDPAINTER_H
