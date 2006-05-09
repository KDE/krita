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
    enum CounterType{ NONE,NUM,ALPHAB_L,ALPHAB_U,ROM_NUM_L,ROM_NUM_U};

    KoSpinBox( int minValue, int maxValue, int step = 1,
           QWidget * parent = 0 );
    KoSpinBox( QWidget * parent = 0 );
    virtual ~KoSpinBox();
    virtual QString mapValueToText( int value );
    virtual int mapTextToValue( bool * ok );
    void setCounterType(CounterType _type);
 private:
    CounterType m_Etype;

};

/******************************************************************/
/* class KPagePreview                                            */
/******************************************************************/
class KPagePreview : public Q3GroupBox
{
    Q_OBJECT

public:
    KPagePreview( QWidget* );
    ~KPagePreview() {}

    void setLeft( double _left )
    { left = _left; update(); }
    void setRight( double _right )
    { right = _right; update(); }
    void setFirst( double _first )
    { first = _first; update(); }
    void setSpacing( double _spacing )
    { spacing = _spacing; update(); }
    void setBefore( double _before )
    { before = _before; update(); }
    void setAfter( double _after )
    { after = _after; update(); }

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
    KPagePreview2( QWidget* );
    ~KPagePreview2() {}

    void setAlign( int _align )
    { align = _align; update(); }

protected:
    void drawContents( QPainter* );

    int align;

};

#endif
