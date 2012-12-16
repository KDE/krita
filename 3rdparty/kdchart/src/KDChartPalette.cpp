/****************************************************************************
** Copyright (C) 2001-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Chart library.
**
** Licensees holding valid commercial KD Chart licenses may use this file in
** accordance with the KD Chart Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#include "KDChartPalette.h"
#include <QBrush>
#include <QVector>

#include <KDABLibFakes>

using namespace KDChart;

namespace {

    static Palette makeDefaultPalette() {
        Palette p;

        p.addBrush( Qt::red );
        p.addBrush( Qt::green );
        p.addBrush( Qt::blue );
        p.addBrush( Qt::cyan );
        p.addBrush( Qt::magenta );
        p.addBrush( Qt::yellow );
        p.addBrush( Qt::darkRed );
        p.addBrush( Qt::darkGreen );
        p.addBrush( Qt::darkBlue );
        p.addBrush( Qt::darkCyan );
        p.addBrush( Qt::darkMagenta );
        p.addBrush( Qt::darkYellow );

        return p;
    }

    static Palette makeSubduedPalette() {
        Palette p;

        p.addBrush( QColor( 0xe0,0x7f,0x70 ) );
        p.addBrush( QColor( 0xe2,0xa5,0x6f ) );
        p.addBrush( QColor( 0xe0,0xc9,0x70 ) );
        p.addBrush( QColor( 0xd1,0xe0,0x70 ) );
        p.addBrush( QColor( 0xac,0xe0,0x70 ) );
        p.addBrush( QColor( 0x86,0xe0,0x70 ) );
        p.addBrush( QColor( 0x70,0xe0,0x7f ) );
        p.addBrush( QColor( 0x70,0xe0,0xa4 ) );
        p.addBrush( QColor( 0x70,0xe0,0xc9 ) );
        p.addBrush( QColor( 0x70,0xd1,0xe0 ) );
        p.addBrush( QColor( 0x70,0xac,0xe0 ) );
        p.addBrush( QColor( 0x70,0x86,0xe0 ) );
        p.addBrush( QColor( 0x7f,0x70,0xe0 ) );
        p.addBrush( QColor( 0xa4,0x70,0xe0 ) );
        p.addBrush( QColor( 0xc9,0x70,0xe0 ) );
        p.addBrush( QColor( 0xe0,0x70,0xd1 ) );
        p.addBrush( QColor( 0xe0,0x70,0xac ) );
        p.addBrush( QColor( 0xe0,0x70,0x86 ) );

        return p;
    }

    static Palette makeRainbowPalette() {
        Palette p;

        p.addBrush( QColor(255,  0,196) );
        p.addBrush( QColor(255,  0, 96) );
        p.addBrush( QColor(255, 128,64) );
        p.addBrush( Qt::yellow );
        p.addBrush( Qt::green );
        p.addBrush( Qt::cyan );
        p.addBrush( QColor( 96, 96,255) );
        p.addBrush( QColor(160,  0,255) );
        for( int i = 8 ; i < 16 ; ++i )
            p.addBrush( p.getBrush(i-8).color().light(), i );

        return p;
    }

}

#define d d_func()

class Palette::Private
{
public:
    explicit Private() {}
    ~Private() {}

    QVector<QBrush> brushes;
};

const Palette& Palette::defaultPalette()
{
    static const Palette palette = makeDefaultPalette();
    return palette;
}

const Palette& Palette::subduedPalette()
{
    static const Palette palette = makeSubduedPalette();
    return palette;
}

const Palette& Palette::rainbowPalette()
{
    static const Palette palette = makeRainbowPalette();
    return palette;
}

Palette::Palette( QObject *parent )
  : QObject( parent ), _d( new Private )
{

}

Palette::~Palette()
{
    delete _d; _d = 0;
}



Palette::Palette( const Palette& r )
    : QObject(), _d( new Private( *r.d ) )
{
}

Palette& Palette::operator=( const Palette& r )
{
    Palette copy( r );
    copy.swap( *this );

    // emit changed() ?
    return *this;
}

bool Palette::isValid() const
{
  return d->brushes.size() >= 1;
}

int Palette::size() const
{
  return d->brushes.size();
}

void Palette::addBrush( const QBrush& brush, int position )
{
  if ( position < 0 || position >= size() ) {
    d->brushes.append( brush );
  } else {
    d->brushes.insert( position, brush );
  }
  emit changed();
}

QBrush Palette::getBrush( int position ) const
{
  if ( !isValid() ) return QBrush();
  return d->brushes.at( position % size() );
}

void Palette::removeBrush( int position )
{
  if ( position < 0 || position >= size() ) return;
  d->brushes.remove( position );
  emit changed();
}

