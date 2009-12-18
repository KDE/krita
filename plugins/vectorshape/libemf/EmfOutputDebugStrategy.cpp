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

#include <math.h>

#include <KDebug>


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
    kDebug(33100) << "Initialising OutputDebugStrategy";
    kDebug(33100) << "image size:" << header->bounds().size();
}

void OutputDebugStrategy::cleanup( const Header *header )
{
    kDebug(33100) << "Cleanup OutputDebugStrategy";
    kDebug(33100) << "image size:" << header->bounds().size();
}

void OutputDebugStrategy::eof()
{
    kDebug(33100) << "EMR_EOF";
}

void OutputDebugStrategy::setPixelV( QPoint &point, quint8 red, quint8 green, quint8 blue, quint8 reserved )
{
    Q_UNUSED( reserved );
    kDebug(33100) << "EMR_SETPIXELV:" << point << QColor( red, green, blue );
}

void OutputDebugStrategy::beginPath()
{
    kDebug(33100) << "EMR_BEGINPATH";
}

void OutputDebugStrategy::closeFigure()
{
    kDebug(33100) << "EMR_CLOSEFIGURE";
}

void OutputDebugStrategy::endPath()
{
    kDebug(33100) << "EMR_ENDPATH";
}

void OutputDebugStrategy::saveDC()
{
    kDebug(33100) << "EMR_SAVEDC";
}

void OutputDebugStrategy::restoreDC( qint32 savedDC )
{
    kDebug(33100) << "EMR_RESTOREDC" << savedDC;
}

void OutputDebugStrategy::setMetaRgn()
{
    kDebug(33100) << "EMR_SETMETARGN";
}

void OutputDebugStrategy::setWindowOrgEx( const QPoint &origin )
{
    kDebug(33100) << "EMR_SETWINDOWORGEX" << origin;
}

void OutputDebugStrategy::setWindowExtEx( const QSize &size )
{
    kDebug(33100) << "EMR_SETWINDOWEXTEX" << size;
}

void OutputDebugStrategy::setViewportOrgEx( const QPoint &origin )
{
    kDebug(33100) << "EMR_SETVIEWPORTORGEX" << origin;
}

void OutputDebugStrategy::setViewportExtEx( const QSize &size )
{
    kDebug(33100) << "EMR_SETVIEWPORTEXTEX" << size;
}

void OutputDebugStrategy::deleteObject( const quint32 ihObject )
{
    kDebug(33100) << "EMR_DELETEOBJECT:" << ihObject;
}

void OutputDebugStrategy::arc( const QRect &box, const QPoint &start, const QPoint &end )
{
    kDebug(33100) << "EMR_ARC" << box << start << end;
}

void OutputDebugStrategy::chord( const QRect &box, const QPoint &start, const QPoint &end )
{
    kDebug(33100) << "EMR_CHORD" << box << start << end;
}

void OutputDebugStrategy::pie( const QRect &box, const QPoint &start, const QPoint &end )
{
    kDebug(33100) << "EMR_PIE" << box << start << end;
}

void OutputDebugStrategy::ellipse( const QRect &box )
{
    kDebug(33100) << "EMR_ELLIPSE:" << box;
}

void OutputDebugStrategy::rectangle( const QRect &box )
{
    kDebug(33100) << "EMR_RECTANGLE:" << box;
}

void OutputDebugStrategy::modifyWorldTransform( quint32 mode, float M11, float M12,
					float M21, float M22, float Dx, float Dy )
{
    kDebug(33100) << "EMR_MODIFYWORLDTRANSFORM:" << mode << QMatrix ( M11, M12, M21, M22, Dx, Dy );
}

void OutputDebugStrategy::setWorldTransform( float M11, float M12, float M21,
				     float M22, float Dx, float Dy )
{
    kDebug(33100) << "EMR_SETWORLDTRANSFORM:" << QMatrix ( M11, M12, M21, M22, Dx, Dy );
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
    kDebug(33100) << "EMR_SETMAPMODE:" << modeAsText;

}

void OutputDebugStrategy::setBkMode( const quint32 backgroundMode )
{
    if ( backgroundMode == TRANSPARENT ) {
        kDebug(33100) << "EMR_SETBKMODE: Transparent";
    } else if ( backgroundMode == OPAQUE ) {
        kDebug(33100) << "EMR_SETBKMODE: Opaque";
    } else {
        kDebug(33100) << "EMR_SETBKMODE: Unexpected value -" << backgroundMode;
        Q_ASSERT( 0 );
    }
}

void OutputDebugStrategy::setPolyFillMode( const quint32 polyFillMode )
{
    if ( polyFillMode == ALTERNATE ) {
	kDebug(33100) << "EMR_SETPOLYFILLMODE: OddEvenFill";
    } else if ( polyFillMode == WINDING ) {
	kDebug(33100) << "EMR_SETPOLYFILLMODE: WindingFill";
    } else {
	kDebug(33100) << "EMR_SETPOLYFILLMODE: Unexpected value -" << polyFillMode;
	Q_ASSERT( 0 );
    }
}

void OutputDebugStrategy::setLayout( const quint32 layoutMode )
{
    kDebug(33100) << "EMR_SETLAYOUT:" << layoutMode;
}

void OutputDebugStrategy::extCreateFontIndirectW( const ExtCreateFontIndirectWRecord &extCreateFontIndirectW )
{
    kDebug(33100) << "EMR_CREATEFONTINDIRECTW:" << extCreateFontIndirectW.fontFace();
}

void OutputDebugStrategy::setTextAlign( const quint32 textAlignMode )
{
    kDebug(33100) << "EMR_SETTEXTALIGN:" << textAlignMode;
}

void OutputDebugStrategy::setTextColor( const quint8 red, const quint8 green, const quint8 blue,
				const quint8 reserved )
{
    Q_UNUSED( reserved );
    kDebug(33100) << "EMR_SETTEXTCOLOR" << QColor( red, green, blue );
}

void OutputDebugStrategy::setBkColor( const quint8 red, const quint8 green, const quint8 blue,
                              const quint8 reserved )
{
    Q_UNUSED( reserved );
    kDebug(33100) << "EMR_SETBKCOLOR" << QColor( red, green, blue );
}

void OutputDebugStrategy::createPen( quint32 ihPen, quint32 penStyle, quint32 x, quint32 y,
			       quint8 red, quint8 green, quint8 blue, quint8 reserved )
{
    Q_UNUSED( y );
    Q_UNUSED( reserved );

    kDebug(33100) << "EMR_CREATEPEN" << "ihPen:" << ihPen << ", penStyle:" << penStyle
                  << "width:" << x << "color:" << QColor( red, green, blue );
}

void OutputDebugStrategy::createBrushIndirect( quint32 ihBrush, quint32 BrushStyle, quint8 red,
				       quint8 green, quint8 blue, quint8 reserved,
				       quint32 BrushHatch )
{
    Q_UNUSED( reserved );

    kDebug(33100) << "EMR_CREATEBRUSHINDIRECT:" << ihBrush << "style:" << BrushStyle
             << "Colour:" << QColor( red, green, blue ) << ", Hatch:" << BrushHatch;
}

void OutputDebugStrategy::selectObject( const quint32 ihObject )
{
    kDebug(33100) << "EMR_SELECTOBJECT" << ihObject;
}

void OutputDebugStrategy::extTextOutA( const ExtTextOutARecord &extTextOutA )
{
    kDebug(33100) << "EMR_EXTTEXTOUTA:" << extTextOutA.referencePoint()
             << extTextOutA.textString();
}

void OutputDebugStrategy::extTextOutW( const QPoint &referencePoint, const QString &textString )
{
    kDebug(33100) << "EMR_EXTTEXTOUTW:" << referencePoint << textString;
}

void OutputDebugStrategy::moveToEx( const quint32 x, const quint32 y )
{
    kDebug(33100) << "EMR_MOVETOEX" << QPoint( x, y );
}

void OutputDebugStrategy::lineTo( const QPoint &finishPoint )
{
    kDebug(33100) << "EMR_LINETO" << finishPoint;
}

void OutputDebugStrategy::arcTo( const QRect &box, const QPoint &start, const QPoint &end )
{
    kDebug(33100) << "EMR_ARCTO" << box << start << end;
}

void OutputDebugStrategy::polygon16( const QRect &bounds, const QList<QPoint> points )
{
    kDebug(33100) << "EMR_POLYGON16" << bounds << points;
}

void OutputDebugStrategy::polyLine( const QRect &bounds, const QList<QPoint> points )
{
    kDebug(33100) << "EMR_POLYLINE" << bounds << points;
}

void OutputDebugStrategy::polyLine16( const QRect &bounds, const QList<QPoint> points )
{
    kDebug(33100) << "EMR_POLYLINE16" << bounds << points;
}

void OutputDebugStrategy::polyPolyLine16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
    kDebug(33100) << "EMR_POLYPOLYLINE16" << bounds << points;
}

void OutputDebugStrategy::polyPolygon16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
    kDebug(33100) << "EMR_POLYPOLYGON16" << bounds << points;
}

void OutputDebugStrategy::polyLineTo16( const QRect &bounds, const QList<QPoint> points )
{
    kDebug(33100) << "EMR_POLYLINETO16" << bounds << points;
}

void OutputDebugStrategy::polyBezier16( const QRect &bounds, const QList<QPoint> points )
{
    kDebug(33100) << "EMR_POLYBEZIER16" << bounds << points;
}

void OutputDebugStrategy::polyBezierTo16( const QRect &bounds, const QList<QPoint> points )
{
    kDebug(33100) << "EMR_POLYBEZIERTO16" << bounds << points;
}

void OutputDebugStrategy::fillPath( const QRect &bounds )
{
    kDebug(33100) << "EMR_FILLPATH" << bounds;
}

void OutputDebugStrategy::strokeAndFillPath( const QRect &bounds )
{
    kDebug(33100) << "EMR_STROKEANDFILLPATH" << bounds;
}

void OutputDebugStrategy::strokePath( const QRect &bounds )
{
    kDebug(33100) << "EMR_STROKEPATH" << bounds;
}

void OutputDebugStrategy::setClipPath( quint32 regionMode )
{
   kDebug(33100) << "EMR_SETCLIPPATH:" << regionMode;
}

void OutputDebugStrategy::bitBlt( BitBltRecord bitBltRecord )
{
    kDebug(33100) << "EMR_BITBLT:" << bitBltRecord.destinationRectangle();
}

void OutputDebugStrategy::setStretchBltMode( const quint32 stretchMode )
{
    switch ( stretchMode ) {
    case 0x01:
        kDebug(33100) << "EMR_STRETCHBLTMODE: STRETCH_ANDSCANS";
        break;
    case 0x02:
        kDebug(33100) << "EMR_STRETCHBLTMODE: STRETCH_ORSCANS";
        break;
    case 0x03:
        kDebug(33100) << "EMR_STRETCHBLTMODE: STRETCH_DELETESCANS";
        break;
    case 0x04:
        kDebug(33100) << "EMR_STRETCHBLTMODE: STRETCH_HALFTONE";
        break;
    default:
        kDebug(33100) << "EMR_STRETCHBLTMODE - unknown stretch mode:" << stretchMode;
    }
}

void OutputDebugStrategy::stretchDiBits( StretchDiBitsRecord stretchDiBitsRecord )
{
    kDebug(33100) << "EMR_STRETCHDIBITS:" << stretchDiBitsRecord.sourceRectangle()
                  << "," << stretchDiBitsRecord.destinationRectangle();
}


} // xnamespace...
