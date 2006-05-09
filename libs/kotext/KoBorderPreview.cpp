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

#include "KoBorderPreview.h"
#include <QPainter>

KoBorderPreview::KoBorderPreview( QWidget* parent )
    :Q3Frame(parent)
{
}

void KoBorderPreview::mousePressEvent( QMouseEvent *_ev )
{
    emit choosearea(_ev);
}

void KoBorderPreview::setBorder( KoBorder::BorderType which, const KoBorder& border)
{
    switch( which ) {
    case KoBorder::TopBorder:
        setTopBorder( border );
        break;
    case KoBorder::BottomBorder:
        setBottomBorder( border );
        break;
    case KoBorder::LeftBorder:
        setLeftBorder( border );
        break;
    case KoBorder::RightBorder:
        setRightBorder( border );
        break;
    default:
        kError() << "KoBorderPreview: unknown border type" << endl;
    }
}

void KoBorderPreview::drawContents( QPainter* painter )
{
    QRect r = contentsRect();
    QFontMetrics fm( font() );

    painter->fillRect( r.x() + fm.width( 'W' ), r.y() + fm.height(), r.width() - 2 * fm.width( 'W' ),
                       r.height() - 2 * fm.height(), Qt::white );
    painter->setClipRect( r.x() + fm.width( 'W' ), r.y() + fm.height(), r.width() - 2 * fm.width( 'W' ),
                          r.height() - 2 * fm.height() );

    bool leftdouble = m_leftBorder.width() > 0 && m_leftBorder.getStyle() == KoBorder::DOUBLE_LINE;
    bool rightdouble = m_rightBorder.width() > 0 && m_rightBorder.getStyle() == KoBorder::DOUBLE_LINE;
    bool topdouble = m_topBorder.width() > 0 && m_topBorder.getStyle() == KoBorder::DOUBLE_LINE;
    bool bottomdouble = m_bottomBorder.width() > 0 && m_bottomBorder.getStyle() == KoBorder::DOUBLE_LINE;

    if ( m_topBorder.width() > 0 ) {
        painter->setPen( setBorderPen( m_topBorder ) );
        painter->drawLine( r.x() + 20, r.y() + 30, r.right() - 19, r.y() + 30 );
        if ( m_topBorder.getStyle()==KoBorder::DOUBLE_LINE)
            painter->drawLine( int(r.x() + 20 + ( leftdouble ? m_leftBorder.width() + 1 : 0) ),
                               int(r.y() + 30 + m_topBorder.width()+1),
                               int(r.right() - 19 - ( rightdouble ? m_rightBorder.width() + 1 : 0) ),
                               int(r.y() + 30 + m_topBorder.width()+1)
                             );
    }

    if ( m_bottomBorder.width() > 0 ) {
        painter->setPen( setBorderPen( m_bottomBorder ) );
        painter->drawLine( r.x() + 20, r.bottom() - 30, r.right() - 19, r.bottom() - 30 );
        if ( m_bottomBorder.getStyle()==KoBorder::DOUBLE_LINE)
            painter->drawLine( int(r.x() + 20 + ( leftdouble ? m_leftBorder.width() + 1 : 0) ),
                               int(r.bottom() - 30 - m_bottomBorder.width()-1),
                               int(r.right() - 19 - ( rightdouble ? m_rightBorder.width() + 1 : 0) ),
                               int(r.bottom() - 30 - m_bottomBorder.width() - 1)
                             );
    }

    if ( m_leftBorder.width() > 0 ) {
        painter->setPen( setBorderPen( m_leftBorder ) );
        painter->drawLine( r.x() + 20, r.y() + 30, r.x() + 20, r.bottom() - 29 );
        if ( m_leftBorder.getStyle()==KoBorder::DOUBLE_LINE)
            painter->drawLine( int(r.x() + 20 + m_leftBorder.width() +1),
                               int(r.y() + 30 + ( topdouble ? m_topBorder.width() + 1 : 0) ),
                               int(r.x() + 20 + m_leftBorder.width() +1),
                               int(r.bottom() - 29 - ( bottomdouble ? m_bottomBorder.width() + 1 : 0) )
                             );
    }

    if ( m_rightBorder.width() > 0 ) {
        painter->setPen( setBorderPen( m_rightBorder ) );
        painter->drawLine( r.right() - 20, r.y() + 30, r.right() - 20, r.bottom() - 29 );
        if ( m_rightBorder.getStyle()==KoBorder::DOUBLE_LINE)
            painter->drawLine( int(r.right() - 20 - m_rightBorder.width() - 1 ),
                               int(r.y() + 30 + ( topdouble ? m_topBorder.width() + 1 : 0) ),
                               int(r.right() - 20 - m_rightBorder.width() - 1),
                               int(r.bottom() - 29 - ( bottomdouble ? m_bottomBorder.width() + 1 : 0) )
                             );
    }
}

QPen KoBorderPreview::setBorderPen( KoBorder _brd )
{
    QPen pen( Qt::black, 1, Qt::SolidLine );

    pen.setWidth( static_cast<int>( _brd.penWidth() ) );
    pen.setColor( _brd.color );

    switch ( _brd.getStyle() ) {
    case KoBorder::SOLID:
        pen.setStyle( Qt::SolidLine );
        break;
    case KoBorder::DASH:
        pen.setStyle( Qt::DashLine );
        break;
    case KoBorder::DOT:
        pen.setStyle( Qt::DotLine );
        break;
    case KoBorder::DASH_DOT:
        pen.setStyle( Qt::DashDotLine );
        break;
    case KoBorder::DASH_DOT_DOT:
        pen.setStyle( Qt::DashDotDotLine );
        break;
    case KoBorder::DOUBLE_LINE:
        pen.setStyle( Qt::SolidLine );
        break;
    }

    return QPen( pen );
}

#include "KoBorderPreview.moc"
