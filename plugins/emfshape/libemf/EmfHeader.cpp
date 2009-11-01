/*
  Copyright 2008 Brad Hards <bradh@frogmouth.net>

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

#include "EmfHeader.h"

#include <QDebug>

namespace Libemf
{

/*****************************************************************************/
const quint32 ENHMETA_SIGNATURE = 0x464D4520;

Header::Header( QDataStream &stream )
{
    stream >> mType;
    stream >> mSize;
    stream >> mBounds;
    stream >> mFrame;
    stream >> mSignature;
    stream >> mVersion;
    stream >> mBytes;
    stream >> mRecords;
    stream >> mHandles;
    stream >> mReserved;
    stream >> m_nDescription;
    stream >> m_offDescription;
    stream >> m_nPalEntries;
    stream >> mDevice;
    stream >> mMillimeters;
    if ( ( ENHMETA_SIGNATURE == mSignature ) && ( m_nDescription != 0 ) ){
        // we have optional EmfDescription, but don't know how to read that yet.
    }

    // FIXME: We could need to read EmfMetafileHeaderExtension1 and
    //        ..2 here but we have no example of that.
    soakBytes( stream, mSize - 88 );
}

Header::~Header()
{
}

bool Header::isValid() const
{
    return ( ( 0x00000001 == mType ) && ( ENHMETA_SIGNATURE == mSignature ) );
}

QRect Header::bounds() const
{
    return mBounds;
}

quint32 Header::recordCount() const
{
    return mRecords;
}

void Header::soakBytes( QDataStream &stream, int numBytes )
{
    quint8 scratch;
    for ( int i = 0; i < numBytes; ++i ) {
        stream >> scratch;
    }
}

}
