/*
  Copyright 2008 Brad Hards <bradh@frogmouth.net>
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

#include "EmfParser.h"

#include <QColor>
#include <QDebug>
#include <QFile>

namespace Libemf
{


// ================================================================


Parser::Parser() : mOutput( 0 )
{
}

Parser::~Parser()
{
}


bool Parser::load( const QString &fileName )
{
    QFile *file = new QFile( fileName );

    if ( ! file->exists() ) {
        qWarning( "Request to load file (%s) that does not exist",
		  qPrintable(file->fileName()) );
	delete file;
        return false;
    }

    if ( ! file->open( QIODevice::ReadOnly ) ) {
        qWarning() << "Request to load file (" << file->fileName()
		   << ") that cannot be opened";
        delete file;
        return false;
    }

    QDataStream stream( file );
    bool result = loadFromStream( stream );

    delete file;

    return result;
}

bool Parser::loadFromStream( QDataStream &stream ) 
{
    stream.setByteOrder( QDataStream::LittleEndian );

    Header *header = new Header( stream );
    if ( ! header->isValid() ) {
        qWarning( "Failed to parse header, perhaps not an EMF file" );
        delete header;
        return false;
    }

    mOutput->init( header );

    int numRecords = header->recordCount();
    for ( int i = 1; i < numRecords; ++i ) {
        // qDebug() << "Record" << i << "of" << numRecords;
        if ( ! readRecord( stream ) ) {
            break;
        }
    }

    mOutput->cleanup( header );

    delete header;

    return true;
}

void Parser::setOutput( AbstractOutput *output )
{
    mOutput = output;
}

void Parser::soakBytes( QDataStream &stream, int numBytes )
{
    quint8 scratch;
    for ( int i = 0; i < numBytes; ++i ) {
        stream >> scratch;
    }
}

void Parser::outputBytes( QDataStream &stream, int numBytes )
{
    quint8 scratch;
    for ( int i = 0; i < numBytes; ++i ) {
        stream >> scratch;
	qDebug("byte(%i):%c", i, scratch);
    }
}

/**
   The supported EMF Record Types

   Refer to [MS-EMF] Section 2.1.1 for more information.
*/
enum RecordType {
    EMR_POLYLINE               = 0x00000004,
    EMR_POLYPOLYGON            = 0x00000008,
    EMR_SETWINDOWEXTEX         = 0x00000009,
    EMR_SETWINDOWORGEX         = 0x0000000A,
    EMR_SETVIEWPORTEXTEX       = 0x0000000B,
    EMR_SETVIEWPORTORGEX       = 0x0000000C,
    EMR_SETBRUSHORGEX          = 0x0000000D,
    EMR_EOF                    = 0x0000000E,
    EMR_SETPIXELV              = 0x0000000F,
    EMR_SETMAPMODE             = 0x00000011,
    EMR_SETBKMODE              = 0x00000012,
    EMR_SETPOLYFILLMODE        = 0x00000013,
    EMR_SETROP2                = 0x00000014,
    EMR_SETSTRETCHBLTMODE      = 0x00000015,
    EMR_SETTEXTALIGN           = 0x00000016,
    EMR_SETTEXTCOLOR           = 0x00000018,
    EMR_SETBKCOLOR             = 0x00000019,
    EMR_MOVETOEX               = 0x0000001B,
    EMR_SETMETARGN             = 0x0000001C,
    EMR_INTERSECTCLIPRECT      = 0x0000001E,
    EMR_SAVEDC                 = 0x00000021,
    EMR_RESTOREDC              = 0x00000022,
    EMR_SETWORLDTRANSFORM      = 0x00000023,
    EMR_MODIFYWORLDTRANSFORM   = 0x00000024,
    EMR_SELECTOBJECT           = 0x00000025,
    EMR_CREATEPEN              = 0x00000026,
    EMR_CREATEBRUSHINDIRECT    = 0x00000027,
    EMR_DELETEOBJECT           = 0x00000028,
    EMR_ELLIPSE                = 0x0000002A,
    EMR_RECTANGLE              = 0x0000002B,
    EMR_ARC                    = 0x0000002D,
    EMR_CHORD                  = 0x0000002E,
    EMR_PIE                    = 0x0000002F,
    EMR_SELECTPALLETTE         = 0x00000030,
    EMR_LINETO                 = 0x00000036,
    EMR_ARCTO                  = 0x00000037,
    EMR_SETMITERLIMIT          = 0x0000003A,
    EMR_BEGINPATH              = 0x0000003B,
    EMR_ENDPATH                = 0x0000003C,
    EMR_CLOSEFIGURE            = 0x0000003D,
    EMR_FILLPATH               = 0x0000003E,
    EMR_STROKEANDFILLPATH      = 0x0000003F,
    EMR_STROKEPATH             = 0x00000040,
    EMR_SETCLIPPATH            = 0x00000043,
    EMR_COMMENT                = 0x00000046,
    EMR_EXTSELECTCLIPRGN       = 0x0000004B,
    EMR_BITBLT                 = 0x0000004C,
    EMR_STRETCHDIBITS          = 0x00000051,
    EMR_EXTCREATEFONTINDIRECTW = 0x00000052,
    EMR_EXTTEXTOUTA            = 0x00000053,
    EMR_EXTTEXTOUTW            = 0x00000054,
    EMR_POLYBEZIER16           = 0x00000055,
    EMR_POLYGON16              = 0x00000056,
    EMR_POLYLINE16             = 0x00000057,
    EMR_POLYBEZIERTO16         = 0x00000058,
    EMR_POLYLINETO16           = 0x00000059,
    EMR_POLYPOLYLINE16         = 0x0000005A,
    EMR_POLYPOLYGON16          = 0x0000005B,
    EMR_EXTCREATEPEN           = 0x0000005F,
    EMR_SETICMMODE             = 0x00000062,
    EMR_SETLAYOUT              = 0x00000073
};

bool Parser::readRecord( QDataStream &stream )
{
    if ( ! mOutput ) {
        qWarning() << "Output device not set";
        return false;
    }
    quint32 type;
    quint32 size;

    stream >> type;
    stream >> size;

    switch ( type ) {
        case EMR_POLYLINE:
        {
            QRect bounds;
            stream >> bounds;
            quint32 count;
            stream >> count;
            QList<QPoint> aPoints;
            for (quint32 i = 0; i < count; ++i) {
                QPoint point;
                stream >> point;
                aPoints.append( point );
            }
            mOutput->polyLine( bounds, aPoints );
        }
        break;
        case EMR_SETWINDOWEXTEX:
        {
            QSize size;
            stream >> size;
            mOutput->setWindowExtEx( size );
        }
        break;
        case EMR_SETWINDOWORGEX:
        {
            QPoint origin;
            stream >> origin;
            mOutput->setWindowOrgEx( origin );
        }
        break;
        case EMR_SETVIEWPORTEXTEX:
        {
            QSize size;
            stream >> size;
            mOutput->setViewportExtEx( size );
        }
        break;
        case EMR_SETVIEWPORTORGEX:
        {
            QPoint origin;
            stream >> origin;
            mOutput->setViewportOrgEx( origin );
        }
        break;
        case EMR_SETBRUSHORGEX:
        {
            QPoint origin;
            stream >> origin;
            qDebug() << "EMR_SETBRUSHORGEX" << origin;
        }
        break;
        case EMR_EOF:
        {
            mOutput->eof();
            soakBytes( stream, size-8 ); // because we already took 8.
            return false;
        }
        break;
        case EMR_SETPIXELV:
        {
            QPoint point;
            quint8 red, green, blue, reserved;
            stream >> point;
            stream >> red >> green >> blue >> reserved;
            mOutput->setPixelV( point, red, green, blue, reserved );
        }
        break;
    case EMR_SETMAPMODE:
	{
	    quint32 mapMode;
	    stream >> mapMode;
	    mOutput->setMapMode( mapMode );
	}
        break;
    case EMR_SETBKMODE:
	{
	    quint32 backgroundMode;
	    stream >> backgroundMode;
            mOutput->setBkMode( backgroundMode );
	}
        break;
    case EMR_SETPOLYFILLMODE:
	{
	    quint32 PolygonFillMode;
	    stream >> PolygonFillMode;
	    mOutput->setPolyFillMode( PolygonFillMode );
	}
	break;
        case EMR_SETROP2:
        {
            quint32 ROP2Mode;
            stream >> ROP2Mode;
            qDebug() << "EMR_SETROP2" << ROP2Mode;
        }
        break;
        case EMR_SETSTRETCHBLTMODE:
        {
            quint32 stretchMode;
            stream >> stretchMode;
            mOutput->setStretchBltMode( stretchMode );

        }
        break;
        case EMR_SETTEXTALIGN:
        {
            quint32 textAlignMode;
            stream >> textAlignMode;
            mOutput->setTextAlign( textAlignMode );
        }
        break;
    case EMR_SETTEXTCOLOR:
	{
	    quint8 red, green, blue, reserved;
	    stream >> red >> green >> blue >> reserved;
	    mOutput->setTextColor( red, green, blue, reserved );
	}
	break;
    case EMR_SETBKCOLOR:
	{
	    quint8 red, green, blue, reserved;
	    stream >> red >> green >> blue >> reserved;
            mOutput->setBkColor( red, green, blue, reserved );
	}
        break;
    case EMR_MOVETOEX:
	{
	    quint32 x, y;
	    stream >> x >> y;
	    mOutput->moveToEx( x, y );
            //qDebug() << "xx EMR_MOVETOEX" << x << y;
	}
	break;
        case EMR_SETMETARGN:
        {
            // Takes no arguments
            mOutput->setMetaRgn();
        }
        break;
    case EMR_INTERSECTCLIPRECT:
    {
        QRect clip;
        stream >> clip;
        qDebug() << "EMR_INTERSECTCLIPRECT" << clip;
    }
    case EMR_SAVEDC:
    {
        mOutput->saveDC();
    }
    break;
    case EMR_RESTOREDC:
    {
        qint32 savedDC;
        stream >> savedDC;
        mOutput->restoreDC( savedDC );
    }
    break;
    case EMR_SETWORLDTRANSFORM:
	{
	    float M11, M12, M21, M22, Dx, Dy;
	    stream >> M11;
	    stream >> M12;
	    stream >> M21;
	    stream >> M22;
	    stream >> Dx;
	    stream >> Dy;
	    mOutput->setWorldTransform( M11, M12, M21, M22, Dx, Dy );
	}
	break;
    case EMR_MODIFYWORLDTRANSFORM:
	{
	    float M11, M12, M21, M22, Dx, Dy;
	    stream >> M11;
	    stream >> M12;
	    stream >> M21;
	    stream >> M22;
	    stream >> Dx;
	    stream >> Dy;
	    quint32 ModifyWorldTransformMode;
	    stream >> ModifyWorldTransformMode;
	    mOutput->modifyWorldTransform( ModifyWorldTransformMode, M11, M12,
					   M21, M22, Dx, Dy );
	}
	break;
    case EMR_SELECTOBJECT:
	quint32 ihObject;
	stream >> ihObject;
	mOutput->selectObject( ihObject );
        break;
    case EMR_CREATEPEN:
	{
	    quint32 ihPen;
	    stream >> ihPen;

	    quint32 penStyle;
	    stream >> penStyle;

	    quint32 x, y;
	    stream >> x;
	    stream >> y; // unused

	    quint8 red, green, blue, reserved;
	    stream >> red >> green >> blue;
	    stream >> reserved; // unused;

	    mOutput->createPen( ihPen, penStyle, x, y, red, green, blue, reserved );

	    break;
	}
    case EMR_CREATEBRUSHINDIRECT:
	{
	    quint32 ihBrush;
	    stream >> ihBrush;

	    quint32 BrushStyle;
	    stream >> BrushStyle;

	    quint8 red, green, blue, reserved;
	    stream >> red >> green >> blue;
	    stream >> reserved; // unused;

	    quint32 BrushHatch;
	    stream >> BrushHatch;

	    mOutput->createBrushIndirect( ihBrush, BrushStyle, red, green, blue, reserved, BrushHatch );

	    break;
	}
    case EMR_DELETEOBJECT:
	{
	    quint32 ihObject;
	    stream >> ihObject;
	    mOutput->deleteObject( ihObject );
	}
        break;
    case EMR_ELLIPSE:
        {
            QRect box;
            stream >> box;
            mOutput->ellipse( box );
        }
        break;
    case EMR_RECTANGLE:
        {
            QRect box;
            stream >> box;
            mOutput->rectangle( box );
            //qDebug() << "xx EMR_RECTANGLE" << box;
        }
        break;
    case EMR_ARC:
        {
            QRect box;
            QPoint start, end;
            stream >> box;
            stream >> start >> end;
            mOutput->arc( box, start, end );
        }
        break;
    case EMR_CHORD:
        {
            QRect box;
            QPoint start, end;
            stream >> box;
            stream >> start >> end;
            mOutput->chord( box, start, end );
        }
        break;
     case EMR_PIE:
        {
            QRect box;
            QPoint start, end;
            stream >> box;
            stream >> start >> end;
            mOutput->pie( box, start, end );
        }
        break;
    case EMR_SELECTPALLETTE:
    {
        quint32 ihPal;
        stream >> ihPal;
        qDebug() << "EMR_SELECTPALLETTE" << ihPal;
    }
    break;
    case EMR_SETMITERLIMIT:
        {
            float miterLimit;
            stream >> miterLimit;
            qDebug() << "EMR_SETMITERLIMIT" << miterLimit;
        }
	break;
    case EMR_BEGINPATH:
	mOutput->beginPath();
	break;
    case EMR_ENDPATH:
	mOutput->endPath();
	break;
    case EMR_CLOSEFIGURE:
	mOutput->closeFigure();
	break;
    case EMR_FILLPATH:
	{
	    QRect bounds;
	    stream >> bounds;
	    mOutput->fillPath( bounds );
            //qDebug() << "xx EMR_FILLPATH" << bounds;
	}
	break;
    case EMR_STROKEANDFILLPATH:
        {
            QRect bounds;
            stream >> bounds;
            mOutput->strokeAndFillPath( bounds );
            //qDebug() << "xx EMR_STROKEANDFILLPATHPATH" << bounds;
        }
        break;
    case EMR_STROKEPATH:
	{
	    QRect bounds;
	    stream >> bounds;
	    mOutput->strokePath( bounds );
            //qDebug() << "xx EMR_STROKEPATH" << bounds;
	}
	break;
    case EMR_SETCLIPPATH:
        {
            quint32 regionMode;
            stream >> regionMode;
            mOutput->setClipPath( regionMode );
        }
        break;
    case EMR_LINETO:
	{
	    quint32 x, y;
	    stream >> x >> y;
	    QPoint finishPoint( x, y );
	    mOutput->lineTo( finishPoint );
            //qDebug() << "xx EMR_LINETO" << x << y;
	}
	break;
    case EMR_ARCTO:
        {
            QRect box;
            stream >> box;
            QPoint start;
            stream >> start;
            QPoint end;
            stream >> end;
            mOutput->arcTo( box, start, end );
        }
        break;
        case EMR_COMMENT:
        {
            quint32 dataSize;
            stream >> dataSize;
            quint32 maybeIdentifier;
            stream >> maybeIdentifier;
            if ( maybeIdentifier == 0x2B464D45 ) {
                // EMFPLUS
                qDebug() << "EMR_COMMENT_EMFPLUS";
                soakBytes( stream, size-16 ); // because we already took 16.
            } else if ( maybeIdentifier == 0x00000000 ) {
                qDebug() << "EMR_EMFSPOOL";
                soakBytes( stream, size-16 ); // because we already took 16.
            } else if ( maybeIdentifier ==  0x43494447 ) {
                quint32 commentType;
                stream >> commentType;
                qDebug() << "EMR_COMMENT_PUBLIC" << commentType;
                soakBytes( stream, size-20 ); // because we already took 20.
            } else {
                qDebug() << "EMR_COMMENT" << dataSize << maybeIdentifier;
                soakBytes( stream, size-16 ); // because we already took 16.
            }
        }
	break;
    case EMR_EXTSELECTCLIPRGN:
	qDebug() << "EMR_EXTSELECTCLIPRGN";
	soakBytes( stream, size-8 ); // because we already took 8.
	break;
    case EMR_BITBLT:
	{
	    BitBltRecord bitBltRecord( stream );
	    mOutput->bitBlt( bitBltRecord );
	}
	break;
    case EMR_STRETCHDIBITS:
	{
	    StretchDiBitsRecord stretchDiBitsRecord( stream );
	    mOutput->stretchDiBits( stretchDiBitsRecord );
	}
	break;
    case EMR_EXTCREATEFONTINDIRECTW:
	{
	    ExtCreateFontIndirectWRecord extCreateFontIndirectWRecord( stream, size );
	    mOutput->extCreateFontIndirectW( extCreateFontIndirectWRecord );
	}
	break;
    case EMR_EXTTEXTOUTA:
        {
            ExtTextOutARecord extTextOutARecord( stream, size );
            mOutput->extTextOutA( extTextOutARecord );
        }
        break;
    case EMR_EXTTEXTOUTW:
	{
	    // qDebug() << "size:" << size;
	    size -= 8;
	    soakBytes( stream, 16 ); // the Bounds
	    size -= 16;
	    soakBytes( stream, 4 ); // iGraphicsMode
	    size -= 4;

	    quint32 exScale;
	    stream >> exScale;
	    // qDebug() << "exScale:" << exScale;
	    size -= 4;

	    quint32 eyScale;
	    stream >> eyScale;
	    // qDebug() << "eyScale:" << eyScale;
	    size -= 4;

            EmrTextObject emrText( stream, size, EmrTextObject::SixteenBitChars );
            mOutput->extTextOutW( emrText.referencePoint(), emrText.textString() );
	}
	break;
        case EMR_SETLAYOUT:
        {
            quint32 layoutMode;
            stream >> layoutMode;
            mOutput->setLayout( layoutMode );
        }
        break;
    case EMR_POLYBEZIER16:
	{
	    QRect bounds;
	    stream >> bounds;
	    quint32 count;
	    stream >> count;
	    QList<QPoint> aPoints;
	    for (quint32 i = 0; i < count; ++i) {
		quint16 x, y;
		stream >> x;
		stream >> y;
		aPoints.append( QPoint( x, y ) );
	    }
	    mOutput->polyBezier16( bounds, aPoints );
	}
        break;
    case EMR_POLYGON16:
	{
	    QRect bounds;
	    stream >> bounds;
	    quint32 count;
	    stream >> count;
	    QList<QPoint> aPoints;
	    for (quint32 i = 0; i < count; ++i) {
		quint16 x, y;
		stream >> x;
		stream >> y;
		aPoints.append( QPoint( x, y ) );
	    }
	    mOutput->polygon16( bounds, aPoints );
	}
	break;
    case EMR_POLYLINE16:
	{
	    QRect bounds;
	    stream >> bounds;
	    quint32 count;
	    stream >> count;
	    QList<QPoint> aPoints;
	    for (quint32 i = 0; i < count; ++i) {
		quint16 x, y;
		stream >> x;
		stream >> y;
		aPoints.append( QPoint( x, y ) );
	    }
	    mOutput->polyLine16( bounds, aPoints );
	}
        break;
    case EMR_POLYBEZIERTO16:
	{
	    QRect bounds;
	    stream >> bounds;
	    quint32 count;
	    stream >> count;
	    QList<QPoint> aPoints;
	    for (quint32 i = 0; i < count; ++i) {
		quint16 x, y;
		stream >> x;
		stream >> y;
		aPoints.append( QPoint( x, y ) );
	    }
	    mOutput->polyBezierTo16( bounds, aPoints );
	}
        break;
    case EMR_POLYLINETO16:
	{
	    QRect bounds;
	    stream >> bounds;
	    quint32 count;
	    stream >> count;
	    QList<QPoint> aPoints;
	    for (quint32 i = 0; i < count; ++i) {
		quint16 x, y;
		stream >> x;
		stream >> y;
		aPoints.append( QPoint( x, y ) );
	    }
	    mOutput->polyLineTo16( bounds, aPoints );
	}
        break;
    case EMR_POLYPOLYLINE16:
        {
            QRect bounds;
            stream >> bounds;
            quint32 numberOfPolylines;
            stream >> numberOfPolylines;
            quint32 count; // number of points in all polylines
            stream >> count;
            QList< QVector< QPoint > > aPoints;
            for ( quint32 i = 0; i < numberOfPolylines; ++i ) {
                quint32 polylinePointCount;
                stream >> polylinePointCount;
                QVector<QPoint> polylinePoints( polylinePointCount );
                aPoints.append( polylinePoints );
            }
            for ( quint32 i = 0; i < numberOfPolylines; ++i ) {
                for ( int j = 0; j < aPoints[i].size(); ++j ) {
                    quint16 x, y;
                    stream >> x >> y;
                    aPoints[i].replace( j,  QPoint( x, y ) );
                }
            }
            mOutput->polyPolyLine16( bounds, aPoints );
        }
        break;
    case EMR_POLYPOLYGON16:
        {
            QRect bounds;
            stream >> bounds;
            quint32 numberOfPolygons;
            stream >> numberOfPolygons;
            quint32 count; // number of points in all polygons
            stream >> count;
            QList< QVector< QPoint > > aPoints;
            for ( quint32 i = 0; i < numberOfPolygons; ++i ) {
                quint32 polygonPointCount;
                stream >> polygonPointCount;
                QVector<QPoint> polygonPoints( polygonPointCount );
                aPoints.append( polygonPoints );
            }
            for ( quint32 i = 0; i < numberOfPolygons; ++i ) {
                for ( int j = 0; j < aPoints[i].size(); ++j ) {
                    quint16 x, y;
                    stream >> x >> y;
                    aPoints[i].replace( j,  QPoint( x, y ) );
                }
            }
            mOutput->polyPolygon16( bounds, aPoints );
        }
        break;
    case EMR_EXTCREATEPEN:
	{
	    quint32 ihPen;
	    stream >> ihPen;

	    quint32 offBmi, cbBmi, offBits, cbBits;
	    stream >> offBmi >> cbBmi >> offBits >> cbBits;

	    quint32 penStyle;
	    stream >> penStyle;

	    quint32 width;
	    stream >> width;

	    quint32 brushStyle;
	    stream >> brushStyle;

	    quint8 red, green, blue, reserved;
	    stream >> red >> green >> blue;
	    stream >> reserved; // unused;

	    // TODO: There is more stuff to parse here

	    // TODO: this needs to go to an extCreatePen() output method
	    mOutput->createPen( ihPen, penStyle, width, 0, red, green, blue, reserved );
	    soakBytes( stream, size-44 );
	}
        break;
        case EMR_SETICMMODE:
        {
            quint32 icmMode;
            stream >> icmMode;
            qDebug() << "EMR_SETICMMODE:" << icmMode;
        }
        break;
    default:
        qDebug() << "unknown type:" << type;
	soakBytes( stream, size-8 ); // because we already took 8.
	Q_ASSERT( type );
    }

    return true;
}

}
