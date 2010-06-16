/****************************************************************************
 ** Copyright (C) 2007 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/

#include "KDChartBackgroundAttributes.h"
#include <QPixmap>

#include <KDABLibFakes>

#define d d_func()


using namespace KDChart;

class BackgroundAttributes::Private
{
    friend class KDChart::BackgroundAttributes;
public:
    Private();
private:
    bool visible;
    QBrush brush;
    BackgroundPixmapMode pixmapMode;
    QPixmap pixmap;
};

BackgroundAttributes::Private::Private() :
    visible( false ),
    pixmapMode( BackgroundAttributes::BackgroundPixmapModeNone )
{
}


BackgroundAttributes::BackgroundAttributes()
    : _d( new Private() )
{
}

BackgroundAttributes::BackgroundAttributes( const BackgroundAttributes& r )
    : _d( new Private( *r.d ) )
{
}

BackgroundAttributes & BackgroundAttributes::operator=( const BackgroundAttributes& r )
{
    if( this == &r )
        return *this;

    *d = *r.d;

    return *this;
}

bool BackgroundAttributes::operator==( const BackgroundAttributes& r ) const
{
    return isEqualTo( r );
}


bool BackgroundAttributes::isEqualTo(
        const BackgroundAttributes& other, bool ignorePixmap ) const
{
    /*
    qDebug() << "BackgroundAttributes::operator==";
    qDebug() << "isVisible" << (isVisible() == other.isVisible());
    qDebug() << "brush"     << (brush() == other.brush());
    qDebug() << "pixmapMode"<< (pixmapMode() == other.pixmapMode());
    qDebug() << "pixmap"    << (pixmap().serialNumber() == other.pixmap().serialNumber());
    */
    return (
            isVisible()  == other.isVisible() &&
            brush()      == other.brush() &&
            pixmapMode() == other.pixmapMode() &&
            (ignorePixmap ||
            pixmap().serialNumber() == other.pixmap().serialNumber()) );
}


BackgroundAttributes::~BackgroundAttributes()
{
    delete _d; _d = 0;
}




void BackgroundAttributes::setVisible( bool visible )
{
    d->visible = visible;
}


bool BackgroundAttributes::isVisible() const
{
    return d->visible;
}

void BackgroundAttributes::setBrush( const QBrush &brush )
{
    d->brush = brush;
}

QBrush BackgroundAttributes::brush() const
{
    return d->brush;
}

void BackgroundAttributes::setPixmapMode( BackgroundPixmapMode mode )
{
    d->pixmapMode = mode;
}

BackgroundAttributes::BackgroundPixmapMode BackgroundAttributes::pixmapMode() const
{
    return d->pixmapMode;
}

void BackgroundAttributes::setPixmap( const QPixmap &backPixmap )
{
    d->pixmap = backPixmap;
}

QPixmap BackgroundAttributes::pixmap() const
{
    return d->pixmap;
}

#if !defined(QT_NO_DEBUG_STREAM)
QDebug operator<<(QDebug dbg, const KDChart::BackgroundAttributes& ba)
{
    dbg << "KDChart::BackgroundAttributes("
	<< "visible="<<ba.isVisible()
	<< "brush="<<ba.brush()
	<< "pixmapmode="<<ba.pixmapMode()
	<< "pixmap="<<ba.pixmap()
	<< ")";
    return dbg;
}
#endif /* QT_NO_DEBUG_STREAM */
