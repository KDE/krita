/*
  Copyright 2008 Brad Hards  <bradh@frogmouth.net>
  Copyright 2009 Inge Wallin <inge@lysator.liu.se>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "EmfOutputDebugStrategy.h"

#include <QDebug>

#include <math.h>

namespace Libemf
{




OutputDebugStrategy::OutputDebugStrategy()
{
}

OutputDebugStrategy::~OutputDebugStrategy()
{
}

void OutputDebugStrategy::init( const Header *header )
{
    qDebug() << "Initialising OutputDebugStrategy";
    qDebug() << "image size:" << header->bounds().size();
}

void OutputDebugStrategy::cleanup( const Header *header )
{
    qDebug() << "Cleanup OutputDebugStrategy";
    qDebug() << "image size:" << header->bounds().size();
}

void OutputDebugStrategy::eof()
{
    qDebug() << "EMR_EOF";
}

void OutputDebugStrategy::setPixelV( QPoint &point, quint8 red, quint8 green, quint8 blue, quint8 reserved )
{
    Q_UNUSED( reserved );
    qDebug() << "EMR_SETPIXELV:" << point << QColor( red, green, blue );
}

void OutputDebugStrategy::beginPath()
{
    qDebug() << "EMR_BEGINPATH";
}

void OutputDebugStrategy::closeFigure()
{
    qDebug() << "EMR_CLOSEFIGURE";
}

void OutputDebugStrategy::endPath()
{
    qDebug() << "EMR_ENDPATH";
}

void OutputDebugStrategy::saveDC()
{
    qDebug() << "EMR_SAVEDC";
}

void OutputDebugStrategy::restoreDC( qint32 savedDC )
{
    qDebug() << "EMR_RESTOREDC" << savedDC;
}

void OutputDebugStrategy::setMetaRgn()
{
    qDebug() << "EMR_SETMETARGN";
}

void OutputDebugStrategy::setWindowOrgEx( const QPoint &origin )
{
    qDebug() << "EMR_SETWINDOWORGEX" << origin;
}

void OutputDebugStrategy::setWindowExtEx( const QSize &size )
{
    qDebug() << "EMR_SETWINDOWEXTEX" << size;
}

void OutputDebugStrategy::setViewportOrgEx( const QPoint &origin )
{
    qDebug() << "EMR_SETVIEWPORTORGEX" << origin;
}

void OutputDebugStrategy::setViewportExtEx( const QSize &size )
{
    qDebug() << "EMR_SETVIEWPORTEXTEX" << size;
}

void OutputDebugStrategy::deleteObject( const quint32 ihObject )
{
    qDebug() << "EMR_DELETEOBJECT:" << ihObject;
}

void OutputDebugStrategy::arc( const QRect &box, const QPoint &start, const QPoint &end )
{
    qDebug() << "EMR_ARC" << box << start << end;
}

void OutputDebugStrategy::chord( const QRect &box, const QPoint &start, const QPoint &end )
{
    qDebug() << "EMR_CHORD" << box << start << end;
}

void OutputDebugStrategy::pie( const QRect &box, const QPoint &start, const QPoint &end )
{
    qDebug() << "EMR_PIE" << box << start << end;
}

void OutputDebugStrategy::ellipse( const QRect &box )
{
    qDebug() << "EMR_ELLIPSE:" << box;
}

void OutputDebugStrategy::rectangle( const QRect &box )
{
    qDebug() << "EMR_RECTANGLE:" << box;
}

void OutputDebugStrategy::modifyWorldTransform( quint32 mode, float M11, float M12,
					float M21, float M22, float Dx, float Dy )
{
    qDebug() << "EMR_MODIFYWORLDTRANSFORM:" << mode << QMatrix ( M11, M12, M21, M22, Dx, Dy );
}

void OutputDebugStrategy::setWorldTransform( float M11, float M12, float M21,
				     float M22, float Dx, float Dy )
{
    qDebug() << "EMR_SETWORLDTRANSFORM:" << QMatrix ( M11, M12, M21, M22, Dx, Dy );
}

void OutputDebugStrategy::setMapMode( quint32 mapMode )
{
    QString modeAsText;
    switch ( mapMode ) {
    case 0x1:
	modeAsText = QString( "map mode - text" );
	break;
    case 0x6:
	modeAsText = QString( "map mode - twips" );
	break;
    case 0x7:
	modeAsText = QString( "map mode - isotropic" );
	break;
    case 0x8:
	modeAsText = QString( "map mode - anisotropic" );
	break;
    default:
	modeAsText = QString( "unexpected map mode: %1").arg( mapMode );
    }
    qDebug() << "EMR_SETMAPMODE:" << modeAsText;

}

void OutputDebugStrategy::setBkMode( const quint32 backgroundMode )
{
    if ( backgroundMode == TRANSPARENT ) {
        qDebug() << "EMR_SETBKMODE: Transparent";
    } else if ( backgroundMode == OPAQUE ) {
        qDebug() << "EMR_SETBKMODE: Opaque";
    } else {
        qDebug() << "EMR_SETBKMODE: Unexpected value -" << backgroundMode;
        Q_ASSERT( 0 );
    }
}

void OutputDebugStrategy::setPolyFillMode( const quint32 polyFillMode )
{
    if ( polyFillMode == ALTERNATE ) {
	qDebug() << "EMR_SETPOLYFILLMODE: OddEvenFill";
    } else if ( polyFillMode == WINDING ) {
	qDebug() << "EMR_SETPOLYFILLMODE: WindingFill";
    } else {
	qDebug() << "EMR_SETPOLYFILLMODE: Unexpected value -" << polyFillMode;
	Q_ASSERT( 0 );
    }
}

void OutputDebugStrategy::setLayout( const quint32 layoutMode )
{
    qDebug() << "EMR_SETLAYOUT:" << layoutMode;
}

void OutputDebugStrategy::extCreateFontIndirectW( const ExtCreateFontIndirectWRecord &extCreateFontIndirectW )
{
    qDebug() << "EMR_CREATEFONTINDIRECTW:" << extCreateFontIndirectW.fontFace();
}

void OutputDebugStrategy::setTextAlign( const quint32 textAlignMode )
{
    qDebug() << "EMR_SETTEXTALIGN:" << textAlignMode;
}

void OutputDebugStrategy::setTextColor( const quint8 red, const quint8 green, const quint8 blue,
				const quint8 reserved )
{
    Q_UNUSED( reserved );
    qDebug() << "EMR_SETTEXTCOLOR" << QColor( red, green, blue );
}

void OutputDebugStrategy::setBkColor( const quint8 red, const quint8 green, const quint8 blue,
                              const quint8 reserved )
{
    Q_UNUSED( reserved );
    qDebug() << "EMR_SETBKCOLOR" << QColor( red, green, blue );
}

void OutputDebugStrategy::createPen( quint32 ihPen, quint32 penStyle, quint32 x, quint32 y,
			       quint8 red, quint8 green, quint8 blue, quint8 reserved )
{
    Q_UNUSED( y );
    Q_UNUSED( reserved );

    qDebug() << "EMR_CREATEPEN" << "ihPen:" << ihPen << ", penStyle:" << penStyle
             << "width:" << x << "color:" << QColor( red, green, blue );
}

void OutputDebugStrategy::createBrushIndirect( quint32 ihBrush, quint32 BrushStyle, quint8 red,
				       quint8 green, quint8 blue, quint8 reserved,
				       quint32 BrushHatch )
{
    Q_UNUSED( reserved );

    qDebug() << "EMR_CREATEBRUSHINDIRECT:" << ihBrush << "style:" << BrushStyle
             << "Colour:" << QColor( red, green, blue ) << ", Hatch:" << BrushHatch;
}

void OutputDebugStrategy::selectObject( const quint32 ihObject )
{
    qDebug() << "EMR_SELECTOBJECT" << ihObject;
}

void OutputDebugStrategy::extTextOutA( const ExtTextOutARecord &extTextOutA )
{
    qDebug() << "EMR_EXTTEXTOUTA:" << extTextOutA.referencePoint()
             << extTextOutA.textString();
}

void OutputDebugStrategy::extTextOutW( const QPoint &referencePoint, const QString &textString )
{
    qDebug() << "EMR_EXTTEXTOUTW:" << referencePoint << textString;
}

void OutputDebugStrategy::moveToEx( const quint32 x, const quint32 y )
{
    qDebug() << "EMR_MOVETOEX" << QPoint( x, y );
}

void OutputDebugStrategy::lineTo( const QPoint &finishPoint )
{
    qDebug() << "EMR_LINETO" << finishPoint;
}

void OutputDebugStrategy::arcTo( const QRect &box, const QPoint &start, const QPoint &end )
{
    qDebug() << "EMR_ARCTO" << box << start << end;
}

void OutputDebugStrategy::polygon16( const QRect &bounds, const QList<QPoint> points )
{
    qDebug() << "EMR_POLYGON16" << bounds << points;
}

void OutputDebugStrategy::polyLine( const QRect &bounds, const QList<QPoint> points )
{
    qDebug() << "EMR_POLYLINE" << bounds << points;
}

void OutputDebugStrategy::polyLine16( const QRect &bounds, const QList<QPoint> points )
{
    qDebug() << "EMR_POLYLINE16" << bounds << points;
}

void OutputDebugStrategy::polyPolyLine16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
    qDebug() << "EMR_POLYPOLYLINE16" << bounds << points;
}

void OutputDebugStrategy::polyPolygon16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
    qDebug() << "EMR_POLYPOLYGON16" << bounds << points;
}

void OutputDebugStrategy::polyLineTo16( const QRect &bounds, const QList<QPoint> points )
{
    qDebug() << "EMR_POLYLINETO16" << bounds << points;
}

void OutputDebugStrategy::polyBezier16( const QRect &bounds, const QList<QPoint> points )
{
    qDebug() << "EMR_POLYBEZIER16" << bounds << points;
}

void OutputDebugStrategy::polyBezierTo16( const QRect &bounds, const QList<QPoint> points )
{
    qDebug() << "EMR_POLYBEZIERTO16" << bounds << points;
}

void OutputDebugStrategy::fillPath( const QRect &bounds )
{
    qDebug() << "EMR_FILLPATH" << bounds;
}

void OutputDebugStrategy::strokeAndFillPath( const QRect &bounds )
{
    qDebug() << "EMR_STROKEANDFILLPATH" << bounds;
}

void OutputDebugStrategy::strokePath( const QRect &bounds )
{
    qDebug() << "EMR_STROKEPATH" << bounds;
}

void OutputDebugStrategy::setClipPath( quint32 regionMode )
{
   qDebug() << "EMR_SETCLIPPATH:" << regionMode;
}

void OutputDebugStrategy::bitBlt( BitBltRecord bitBltRecord )
{
    qDebug() << "EMR_BITBLT:" << bitBltRecord.destinationRectangle();
}

void OutputDebugStrategy::setStretchBltMode( const quint32 stretchMode )
{
    switch ( stretchMode ) {
    case 0x01:
        qDebug() << "EMR_STRETCHBLTMODE: STRETCH_ANDSCANS";
        break;
    case 0x02:
        qDebug() << "EMR_STRETCHBLTMODE: STRETCH_ORSCANS";
        break;
    case 0x03:
        qDebug() << "EMR_STRETCHBLTMODE: STRETCH_DELETESCANS";
        break;
    case 0x04:
        qDebug() << "EMR_STRETCHBLTMODE: STRETCH_HALFTONE";
        break;
    default:
        qDebug() << "EMR_STRETCHBLTMODE - unknown stretch mode:" << stretchMode;
    }
}

void OutputDebugStrategy::stretchDiBits( StretchDiBitsRecord stretchDiBitsRecord )
{
    qDebug() << "EMR_STRETCHDIBITS:" << stretchDiBitsRecord.sourceRectangle()
             << "," << stretchDiBitsRecord.destinationRectangle();
}


} // xnamespace...
