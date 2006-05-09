/* This file is part of the KOffice project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2006 Thomas Zander <zander@kde.org>
   Copyright (C) 2006 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KOBORDERPREVIEW_H
#define KOBORDERPREVIEW_H

#include <Q3Frame>
#include "KoBorder.h"

class KOTEXT_EXPORT KoBorderPreview : public Q3Frame/*QGroupBox*/
{
    Q_OBJECT

public:
    KoBorderPreview( QWidget* );
    ~KoBorderPreview() {}

    KoBorder leftBorder()const { return m_leftBorder; }
    void setLeftBorder( const KoBorder& _leftBorder )
	{ m_leftBorder = _leftBorder; update(); }
    KoBorder rightBorder() const { return m_rightBorder; }
    void setRightBorder( const KoBorder& _rightBorder )
	{ m_rightBorder = _rightBorder; update(); }
    KoBorder topBorder()const { return m_topBorder; }
    void setTopBorder( const KoBorder& _topBorder )
	{ m_topBorder = _topBorder; update(); }
    KoBorder bottomBorder()const { return m_bottomBorder; }
    void setBottomBorder( const KoBorder& _bottomBorder )
	{ m_bottomBorder = _bottomBorder; update(); }

    void setBorder( KoBorder::BorderType which, const KoBorder& border);

protected:
    virtual void mousePressEvent( QMouseEvent* _ev );
    void drawContents( QPainter* );
    QPen setBorderPen( KoBorder _brd );

    KoBorder m_leftBorder, m_rightBorder, m_topBorder, m_bottomBorder;

signals:
    void choosearea(QMouseEvent * _ev);

};

#endif /* KOBORDERPREVIEW_H */

