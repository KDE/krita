/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>

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

#include "KoDecorationTab.h"
#include <KoGlobal.h>

#include <kcolorbutton.h>
#include <kpushbutton.h>
#include <knuminput.h>
#include <klocale.h>

#include <q3buttongroup.h>

#include "KoDecorationTab.moc"

KoDecorationTab::KoDecorationTab( QWidget* parent, const char* name, Qt::WFlags fl )
    : KoDecorationTabBase( parent, name, fl )
{
    shadowDistanceKDoubleNumInput->setRange(0, 9, 0.5, false);

    connect( textKColorButton, SIGNAL( changed( const QColor& ) ), this, SIGNAL( fontColorChanged( const QColor& ) ) );
    connect( backgroundKColorButton, SIGNAL( changed( const QColor& ) ), this, SIGNAL( backgroundColorChanged( const QColor& ) ) );
    connect( shadowKColorButton, SIGNAL( changed( const QColor& ) ), this, SIGNAL( shadowColorChanged( const QColor& ) ) );
    connect( shadowDistanceKDoubleNumInput, SIGNAL( valueChanged( double ) ), this, SIGNAL( shadowDistanceChanged( double ) ) );
    connect( shadowDirectionButtonGroup, SIGNAL( clicked( int ) ), this, SIGNAL( shadowDirectionChanged( int ) ) );
}

KoDecorationTab::~KoDecorationTab()
{
}

QColor KoDecorationTab::getTextColor() const
{
    return textKColorButton->color();
}

QColor KoDecorationTab::getBackgroundColor() const
{
    return backgroundKColorButton->color();
}

double KoDecorationTab::getShadowDistanceX() const
{
    short int sd = shadowDirectionButtonGroup->selectedId();
    double dist = shadowDistanceKDoubleNumInput->value();
    return shadowDistanceX( sd, dist );
}

double KoDecorationTab::getShadowDistanceY() const
{
    short int sd = shadowDirectionButtonGroup->selectedId();
    double dist = shadowDistanceKDoubleNumInput->value();
    return shadowDistanceY( sd, dist );
}

QColor KoDecorationTab::getShadowColor() const
{
    return shadowKColorButton->color();
}

void KoDecorationTab::setTextColor( const QColor &color )
{
    textKColorButton->setColor( color );
}

void KoDecorationTab::setBackgroundColor( const QColor &color )
{
	backgroundKColorButton->setColor( color );
}

void KoDecorationTab::setShadow( double shadowDistanceX, double shadowDistanceY, const QColor& shadowColor )
{
    short int sd = SD_RIGHT_UP;
    double dist = 0.0;

    if ( shadowDistanceX > 0 ) // right
        if ( shadowDistanceY == 0 )
            sd = SD_RIGHT;
        else
            sd = shadowDistanceY > 0 ? SD_RIGHT_BOTTOM : SD_RIGHT_UP;
    else if ( shadowDistanceX == 0 ) // top/bottom
        sd = shadowDistanceY > 0 ? SD_BOTTOM : SD_UP;
    else // left
        if ( shadowDistanceY == 0 )
            sd = SD_LEFT;
        else
            sd = shadowDistanceY > 0 ? SD_LEFT_BOTTOM : SD_LEFT_UP;

    shadowDirectionButtonGroup->setButton( sd );

    dist = qMax( QABS(shadowDistanceX), QABS(shadowDistanceY) );
    shadowDistanceKDoubleNumInput->setValue( dist );

    shadowKColorButton->setColor( shadowColor.isValid() ? shadowColor: Qt::gray  );

}

double KoDecorationTab::shadowDistanceX( short int sd, double dist ) const
{
    switch ( sd )
    {
    case SD_LEFT_BOTTOM:
    case SD_LEFT:
    case SD_LEFT_UP:
        return - dist;
    case SD_UP:
    case SD_BOTTOM:
        return 0;
    case SD_RIGHT_UP:
    case SD_RIGHT:
    case SD_RIGHT_BOTTOM:
        return dist;
    }
    return 0;
}

double KoDecorationTab::shadowDistanceY( short int sd, double dist ) const
{
    switch ( sd )
    {
    case SD_LEFT_UP:
    case SD_UP:
    case SD_RIGHT_UP:
        return - dist;
    case SD_LEFT:
    case SD_RIGHT:
        return 0;
    case SD_LEFT_BOTTOM:
    case SD_BOTTOM:
    case SD_RIGHT_BOTTOM:
        return dist;
    }
    return 0;
}
