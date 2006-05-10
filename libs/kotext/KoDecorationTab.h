/* This file is part of the KDE project
   Copyright (C)  2001,2002,2003 Montel Laurent <lmontel@mandrakesoft.com>

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

#ifndef __kohigdecorationtab_h__
#define __kohigdecorationtab_h__

#include <kodecorationtabbase.h>

#include <QColor>

class KoDecorationTab : public KoDecorationTabBase
{
    Q_OBJECT

public:
    KoDecorationTab( QWidget* parent=0, const char* name=0, Qt::WFlags fl=0 );
    ~KoDecorationTab();

    QColor getTextColor() const;
    QColor getBackgroundColor() const;
    double getShadowDistanceX() const;
    double getShadowDistanceY() const;
    QColor getShadowColor() const;

    void setTextColor( const QColor &color );
    void setBackgroundColor( const QColor &color );
    void setShadow( double shadowDistanceX, double shadowDistanceY, const QColor& shadowColor );

signals:
    void fontColorChanged( const QColor& );
    void backgroundColorChanged( const QColor&  );
    void shadowColorChanged( const QColor&  );
    void shadowDistanceChanged( double );
    void shadowDirectionChanged( int  );
    void shadowChanged();

protected:
    enum {
        SD_LEFT_UP = 1,
        SD_UP = 2,
        SD_RIGHT_UP = 3,
        SD_RIGHT = 4,
        SD_RIGHT_BOTTOM = 5,
        SD_BOTTOM = 6,
        SD_LEFT_BOTTOM = 7,
        SD_LEFT = 8
    } ShadowDirection;

    double shadowDistanceX( short int sd, double dist ) const;
    double shadowDistanceY( short int sd, double dist ) const;

};

#endif
