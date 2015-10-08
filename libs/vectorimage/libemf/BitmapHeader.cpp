/*
 * Copyright 2010 Inge Wallin <inge@lysator.liu.se>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "BitmapHeader.h"

#include <QDataStream>
//#include <QColor>
//#include <QImage>
//#include <QRect> // also provides QSize
//#include <QString>

#include <VectorImageDebug.h>


namespace Libemf
{

static void soakBytes( QDataStream &stream, int numBytes )
{
    quint8 scratch;
    for ( int i = 0; i < numBytes; ++i ) {
        stream >> scratch;
    }
}


BitmapHeader::BitmapHeader( QDataStream &stream, int size )
{
    m_headerType = BitmapInfoHeader;  // The default

    int  read = 40;             // Keep track of how many bytes we have read;

    // Read the data that is present in a BitmapInfoHeader (size 40)
    stream >> m_headerSize;
    stream >> m_width;
    stream >> m_height;
    stream >> m_planes;         // 16 bits
    stream >> m_bitCount;       // 16 bits
    stream >> m_compression;
    stream >> m_imageSize;

    stream >> m_xPelsPerMeter;
    stream >> m_yPelsPerMeter;
    stream >> m_colorUsed;
    stream >> m_colorImportant;

#if 0
    debugVectorImage << "Width:" << m_width;
    debugVectorImage << "Height:" << m_height;
    debugVectorImage << "planes:" << m_planes;
    debugVectorImage << "BitCount:" << m_bitCount;
    debugVectorImage << "Compression:" << m_compression;
    debugVectorImage << "ImageSize:" << m_imageSize;
    debugVectorImage << "Colors used:" << m_colorUsed;
#endif
    // BitmapV4Header (size 40+68 = 108)
    if (size >= 108) {
        m_headerType = BitmapV4Header;
        read = 108;

        stream >> m_redMask;
        stream >> m_greenMask;
        stream >> m_blueMask;
        stream >> m_alphaMask;
        stream >> m_colorSpaceType;

        // FIXME sometime: Implement the real CIEXYZTriple
        for (int i = 0; i < 9; ++i)
            stream >> m_endpoints[i];

        stream >> m_gammaRed;
        stream >> m_gammaGreen;
        stream >> m_gammaBlue;
    }

    // BitmapV5Header (size 108+16 = 124)
    if (size >= 124) {
        m_headerType = BitmapV5Header;
        read = 124;

        stream >> m_intent;
        stream >> m_profileData;
        stream >> m_profileSize;
        stream >> m_reserved;
    }

#if 0
    debugVectorImage << "header type:" << m_headerType;
    debugVectorImage << "header size:" << size;
    debugVectorImage << "read bytes: " << read;
#endif
    // Read away the overshot from the size parameter;
    if (size > read)
        soakBytes(stream, size - read);
}

BitmapHeader::~BitmapHeader()
{
}



} // namespace Libemf
