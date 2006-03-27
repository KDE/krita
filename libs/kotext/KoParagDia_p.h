/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef koparagdia_p_h
#define koparagdia_p_h

// This file hides those definitions from "users" of koParagDia.h
// to reduce compile-time dependencies.

#include <q3groupbox.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3Frame>
#include <KoParagCounter.h>
#include <qspinbox.h>
class QWidget;
class QPainter;


/******************************************************************/
/* Class: KoSpinBox                                               */
/******************************************************************/
class KoSpinBox : public QSpinBox
{
    Q_OBJECT

public:
    enum counterType{ NONE,NUM,ALPHAB_L,ALPHAB_U,ROM_NUM_L,ROM_NUM_U};

    KoSpinBox( int minValue, int maxValue, int step = 1,
           QWidget * parent = 0, const char * name = 0 );
    KoSpinBox( QWidget * parent = 0, const char * name = 0 );
    virtual ~KoSpinBox();
    virtual QString mapValueToText( int value );
    virtual int mapTextToValue( bool * ok );
    void setCounterType(counterType _type);
 private:
    counterType m_Etype;

};

/******************************************************************/
/* class KPagePreview                                            */
/******************************************************************/
class KPagePreview : public Q3GroupBox
{
    Q_OBJECT

public:
    KPagePreview( QWidget*, const char* = 0L );
    ~KPagePreview() {}

    void setLeft( double _left )
    { left = _left; repaint( false ); }
    void setRight( double _right )
    { right = _right; repaint( false ); }
    void setFirst( double _first )
    { first = _first; repaint( false ); }
    void setSpacing( double _spacing )
    { spacing = _spacing; repaint( false ); }
    void setBefore( double _before )
    { before = _before; repaint( false ); }
    void setAfter( double _after )
    { after = _after; repaint( false ); }

protected:
    void drawContents( QPainter* );
    int convert(double input);

    double left, right, first, spacing, before, after;

};

/******************************************************************/
/* class KPagePreview2                                           */
/******************************************************************/

class KPagePreview2 : public Q3GroupBox
{
    Q_OBJECT

public:
    KPagePreview2( QWidget*, const char* = 0L );
    ~KPagePreview2() {}

    void setAlign( int _align )
    { align = _align; repaint( false ); }

protected:
    void drawContents( QPainter* );

    int align;

};

/******************************************************************/
/* class KoBorderPreview                                          */
/******************************************************************/

class KoBorderPreview : public Q3Frame/*QGroupBox*/
{
    Q_OBJECT

public:
    KoBorderPreview( QWidget*, const char* = 0L );
    ~KoBorderPreview() {}

    KoBorder leftBorder()const { return m_leftBorder; }
    void setLeftBorder( const KoBorder& _leftBorder ) 
	{ m_leftBorder = _leftBorder; repaint( true ); }
    KoBorder rightBorder() const { return m_rightBorder; }
    void setRightBorder( const KoBorder& _rightBorder )
	{ m_rightBorder = _rightBorder; repaint( true ); }
    KoBorder topBorder()const { return m_topBorder; }
    void setTopBorder( const KoBorder& _topBorder )
	{ m_topBorder = _topBorder; repaint( true ); }
    KoBorder bottomBorder()const { return m_bottomBorder; }
    void setBottomBorder( const KoBorder& _bottomBorder )
	{ m_bottomBorder = _bottomBorder; repaint( true ); }

    void setBorder( KoBorder::BorderType which, const KoBorder& border);

protected:
    virtual void mousePressEvent( QMouseEvent* _ev );
    void drawContents( QPainter* );
    QPen setBorderPen( KoBorder _brd );

    KoBorder m_leftBorder, m_rightBorder, m_topBorder, m_bottomBorder;
signals:
    void choosearea(QMouseEvent * _ev);

};

#endif
