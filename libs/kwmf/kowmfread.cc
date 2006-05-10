/* This file is part of the KDE libraries
 * Copyright (c) 2003 thierry lorthiois (lorthioist@wanadoo.fr)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QFile>
#include <kdebug.h>

#include "kowmfread.h"
#include "kowmfreadprivate.h"

KoWmfRead::KoWmfRead() {
    mKwmf = new KoWmfReadPrivate();
}

KoWmfRead::~KoWmfRead() {
    delete mKwmf;
}


bool KoWmfRead::load( const QString& filename )
{
    QFile file( filename );

    if ( !file.open( QIODevice::ReadOnly ) )
    {
        kDebug() << "KoWmfRead : Cannot open file " << QFile::encodeName(filename) << endl;
        return false;
    }

    bool ret = mKwmf->load( file.readAll() );
    file.close();

    return ret;
}


bool KoWmfRead::load( const QByteArray& array )
{
    return mKwmf->load( array );
}


bool KoWmfRead::play(  )
{
    return mKwmf->play( this );
}


bool KoWmfRead::isValid( void ) const {
    return mKwmf->mValid;
}


bool KoWmfRead::isStandard( void ) const {
    return mKwmf->mStandard;
}


bool KoWmfRead::isPlaceable( void ) const {
    return mKwmf->mPlaceable;
}


bool KoWmfRead::isEnhanced( void ) const {
    return mKwmf->mEnhanced;
}


QRect KoWmfRead::boundingRect( void ) const {
    return mKwmf->mBBox;
}


int KoWmfRead::defaultDpi( void ) const {
    if ( mKwmf->mPlaceable ) {
        return mKwmf->mDpi;
    }
    else {
        return  0;
    }
}


void KoWmfRead::setDebug( int nbrFunc ) {
    mKwmf->mNbrFunc = nbrFunc;
}

