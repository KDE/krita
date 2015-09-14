/*
  Copyright 2008      Brad Hards  <bradh@frogmouth.net>
  Copyright 2009-2010 Inge Wallin <inge@lysator.liu.se>

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

#include <VectorImageDebug.h>

#include "EmfObjects.h"

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
    debugVectorImage << "Initialising OutputDebugStrategy";
    debugVectorImage << "image size:" << header->bounds().size();
}

void OutputDebugStrategy::cleanup( const Header *header )
{
    debugVectorImage << "Cleanup OutputDebugStrategy";
    debugVectorImage << "image size:" << header->bounds().size();
}

void OutputDebugStrategy::eof()
{
    debugVectorImage << "EMR_EOF";
}

void OutputDebugStrategy::setPixelV( QPoint &point, quint8 red, quint8 green, quint8 blue, quint8 reserved )
{
    Q_UNUSED( reserved );
    debugVectorImage << "EMR_SETPIXELV:" << point << QColor( red, green, blue );
}

void OutputDebugStrategy::beginPath()
{
    debugVectorImage << "EMR_BEGINPATH";
}

void OutputDebugStrategy::closeFigure()
{
    debugVectorImage << "EMR_CLOSEFIGURE";
}

void OutputDebugStrategy::endPath()
{
    debugVectorImage << "EMR_ENDPATH";
}

void OutputDebugStrategy::saveDC()
{
    debugVectorImage << "EMR_SAVEDC";
}

void OutputDebugStrategy::restoreDC( qint32 savedDC )
{
    debugVectorImage << "EMR_RESTOREDC" << savedDC;
}

void OutputDebugStrategy::setMetaRgn()
{
    debugVectorImage << "EMR_SETMETARGN";
}

void OutputDebugStrategy::setWindowOrgEx( const QPoint &origin )
{
    debugVectorImage << "EMR_SETWINDOWORGEX" << origin;
}

void OutputDebugStrategy::setWindowExtEx( const QSize &size )
{
    debugVectorImage << "EMR_SETWINDOWEXTEX" << size;
}

void OutputDebugStrategy::setViewportOrgEx( const QPoint &origin )
{
    debugVectorImage << "EMR_SETVIEWPORTORGEX" << origin;
}

void OutputDebugStrategy::setViewportExtEx( const QSize &size )
{
    debugVectorImage << "EMR_SETVIEWPORTEXTEX" << size;
}

void OutputDebugStrategy::deleteObject( const quint32 ihObject )
{
    debugVectorImage << "EMR_DELETEOBJECT:" << ihObject;
}

void OutputDebugStrategy::arc( const QRect &box, const QPoint &start, const QPoint &end )
{
    debugVectorImage << "EMR_ARC" << box << start << end;
}

void OutputDebugStrategy::chord( const QRect &box, const QPoint &start, const QPoint &end )
{
    debugVectorImage << "EMR_CHORD" << box << start << end;
}

void OutputDebugStrategy::pie( const QRect &box, const QPoint &start, const QPoint &end )
{
    debugVectorImage << "EMR_PIE" << box << start << end;
}

void OutputDebugStrategy::ellipse( const QRect &box )
{
    debugVectorImage << "EMR_ELLIPSE:" << box;
}

void OutputDebugStrategy::rectangle( const QRect &box )
{
    debugVectorImage << "EMR_RECTANGLE:" << box;
}

void OutputDebugStrategy::modifyWorldTransform( quint32 mode, float M11, float M12,
					float M21, float M22, float Dx, float Dy )
{
    debugVectorImage << "EMR_MODIFYWORLDTRANSFORM:" << mode << QTransform ( M11, M12, M21, M22, Dx, Dy );
}

void OutputDebugStrategy::setWorldTransform( float M11, float M12, float M21,
				     float M22, float Dx, float Dy )
{
    debugVectorImage << "EMR_SETWORLDTRANSFORM:" << QTransform ( M11, M12, M21, M22, Dx, Dy );
}

void OutputDebugStrategy::setMapMode( quint32 mapMode )
{
    QString modeAsText;
    switch ( mapMode ) {
    case MM_TEXT:
	modeAsText = QString( "map mode - text" );
	break;
    case MM_LOMETRIC:
	modeAsText = QString( "map mode - lometric" );
	break;
    case MM_HIMETRIC:
	modeAsText = QString( "map mode - himetric" );
	break;
    case MM_LOENGLISH:
	modeAsText = QString( "map mode - loenglish" );
	break;
    case MM_HIENGLISH:
	modeAsText = QString( "map mode - hienglish" );
	break;
    case MM_TWIPS:
	modeAsText = QString( "map mode - twips" );
	break;
    case MM_ISOTROPIC:
	modeAsText = QString( "map mode - isotropic" );
	break;
    case MM_ANISOTROPIC:
	modeAsText = QString( "map mode - anisotropic" );
	break;
    default:
	modeAsText = QString( "unexpected map mode: %1").arg( mapMode );
    }
    debugVectorImage << "EMR_SETMAPMODE:" << modeAsText;

}

void OutputDebugStrategy::setBkMode( const quint32 backgroundMode )
{
    if ( backgroundMode == TRANSPARENT ) {
        debugVectorImage << "EMR_SETBKMODE: Transparent";
    } else if ( backgroundMode == OPAQUE ) {
        debugVectorImage << "EMR_SETBKMODE: Opaque";
    } else {
        debugVectorImage << "EMR_SETBKMODE: Unexpected value -" << backgroundMode;
        Q_ASSERT( 0 );
    }
}

void OutputDebugStrategy::setPolyFillMode( const quint32 polyFillMode )
{
    if ( polyFillMode == ALTERNATE ) {
	debugVectorImage << "EMR_SETPOLYFILLMODE: OddEvenFill";
    } else if ( polyFillMode == WINDING ) {
	debugVectorImage << "EMR_SETPOLYFILLMODE: WindingFill";
    } else {
	debugVectorImage << "EMR_SETPOLYFILLMODE: Unexpected value -" << polyFillMode;
	Q_ASSERT( 0 );
    }
}

void OutputDebugStrategy::setLayout( const quint32 layoutMode )
{
    debugVectorImage << "EMR_SETLAYOUT:" << layoutMode;
}

void OutputDebugStrategy::extCreateFontIndirectW( const ExtCreateFontIndirectWRecord &extCreateFontIndirectW )
{
    debugVectorImage << "EMR_CREATEFONTINDIRECTW:" << extCreateFontIndirectW.fontFace();
}

void OutputDebugStrategy::setTextAlign( const quint32 textAlignMode )
{
    debugVectorImage << "EMR_SETTEXTALIGN:" << textAlignMode;
}

void OutputDebugStrategy::setTextColor( const quint8 red, const quint8 green, const quint8 blue,
				const quint8 reserved )
{
    Q_UNUSED( reserved );
    debugVectorImage << "EMR_SETTEXTCOLOR" << QColor( red, green, blue );
}

void OutputDebugStrategy::setBkColor( const quint8 red, const quint8 green, const quint8 blue,
                              const quint8 reserved )
{
    Q_UNUSED( reserved );
    debugVectorImage << "EMR_SETBKCOLOR" << QColor( red, green, blue );
}

void OutputDebugStrategy::createPen( quint32 ihPen, quint32 penStyle, quint32 x, quint32 y,
			       quint8 red, quint8 green, quint8 blue, quint8 reserved )
{
    Q_UNUSED( y );
    Q_UNUSED( reserved );

    debugVectorImage << "EMR_CREATEPEN" << "ihPen:" << ihPen << ", penStyle:" << penStyle
                  << "width:" << x << "color:" << QColor( red, green, blue );
}

void OutputDebugStrategy::createBrushIndirect( quint32 ihBrush, quint32 BrushStyle, quint8 red,
				       quint8 green, quint8 blue, quint8 reserved,
				       quint32 BrushHatch )
{
    Q_UNUSED( reserved );

    debugVectorImage << "EMR_CREATEBRUSHINDIRECT:" << ihBrush << "style:" << BrushStyle
             << "Colour:" << QColor( red, green, blue ) << ", Hatch:" << BrushHatch;
}

void OutputDebugStrategy::createMonoBrush( quint32 ihBrush, Bitmap *bitmap )
{
    debugVectorImage << "EMR_CREATEMONOBRUSH:" << ihBrush << "bitmap:" << bitmap;
}

void OutputDebugStrategy::selectObject( const quint32 ihObject )
{
    debugVectorImage << "EMR_SELECTOBJECT" << ihObject;
}

void OutputDebugStrategy::extTextOut( const QRect &bounds, const EmrTextObject &textObject )
{
    debugVectorImage << "EMR_EXTTEXTOUTW:" << bounds
                  << textObject.referencePoint()
                  << textObject.textString();
}

void OutputDebugStrategy::moveToEx( const qint32 x, const qint32 y )
{
    debugVectorImage << "EMR_MOVETOEX" << QPoint( x, y );
}

void OutputDebugStrategy::lineTo( const QPoint &finishPoint )
{
    debugVectorImage << "EMR_LINETO" << finishPoint;
}

void OutputDebugStrategy::arcTo( const QRect &box, const QPoint &start, const QPoint &end )
{
    debugVectorImage << "EMR_ARCTO" << box << start << end;
}

void OutputDebugStrategy::polygon16( const QRect &bounds, const QList<QPoint> points )
{
    debugVectorImage << "EMR_POLYGON16" << bounds << points;
}

void OutputDebugStrategy::polyLine( const QRect &bounds, const QList<QPoint> points )
{
    debugVectorImage << "EMR_POLYLINE" << bounds << points;
}

void OutputDebugStrategy::polyLine16( const QRect &bounds, const QList<QPoint> points )
{
    debugVectorImage << "EMR_POLYLINE16" << bounds << points;
}

void OutputDebugStrategy::polyPolyLine16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
    debugVectorImage << "EMR_POLYPOLYLINE16" << bounds << points;
}

void OutputDebugStrategy::polyPolygon16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
    debugVectorImage << "EMR_POLYPOLYGON16" << bounds << points;
}

void OutputDebugStrategy::polyLineTo16( const QRect &bounds, const QList<QPoint> points )
{
    debugVectorImage << "EMR_POLYLINETO16" << bounds << points;
}

void OutputDebugStrategy::polyBezier16( const QRect &bounds, const QList<QPoint> points )
{
    debugVectorImage << "EMR_POLYBEZIER16" << bounds << points;
}

void OutputDebugStrategy::polyBezierTo16( const QRect &bounds, const QList<QPoint> points )
{
    debugVectorImage << "EMR_POLYBEZIERTO16" << bounds << points;
}

void OutputDebugStrategy::fillPath( const QRect &bounds )
{
    debugVectorImage << "EMR_FILLPATH" << bounds;
}

void OutputDebugStrategy::strokeAndFillPath( const QRect &bounds )
{
    debugVectorImage << "EMR_STROKEANDFILLPATH" << bounds;
}

void OutputDebugStrategy::strokePath( const QRect &bounds )
{
    debugVectorImage << "EMR_STROKEPATH" << bounds;
}

void OutputDebugStrategy::setClipPath( quint32 regionMode )
{
   debugVectorImage << "EMR_SETCLIPPATH:" << regionMode;
}

void OutputDebugStrategy::bitBlt( BitBltRecord &bitBltRecord )
{
    debugVectorImage << "EMR_BITBLT:" << bitBltRecord.destinationRectangle();
}

void OutputDebugStrategy::setStretchBltMode( const quint32 stretchMode )
{
    switch ( stretchMode ) {
    case 0x01:
        debugVectorImage << "EMR_STRETCHBLTMODE: STRETCH_ANDSCANS";
        break;
    case 0x02:
        debugVectorImage << "EMR_STRETCHBLTMODE: STRETCH_ORSCANS";
        break;
    case 0x03:
        debugVectorImage << "EMR_STRETCHBLTMODE: STRETCH_DELETESCANS";
        break;
    case 0x04:
        debugVectorImage << "EMR_STRETCHBLTMODE: STRETCH_HALFTONE";
        break;
    default:
        debugVectorImage << "EMR_STRETCHBLTMODE - unknown stretch mode:" << stretchMode;
    }
}

void OutputDebugStrategy::stretchDiBits( StretchDiBitsRecord &stretchDiBitsRecord )
{
    debugVectorImage << "EMR_STRETCHDIBITS:" << stretchDiBitsRecord.sourceRectangle()
                  << "," << stretchDiBitsRecord.destinationRectangle();
}


} // xnamespace...
