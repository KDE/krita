/* This file is part of the KDE libraries
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>
                 2002 Werner Trobin <trobin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <KoFilter.h>

#include <QFile>
//Added by qt3to4:

#include <kurl.h>
#include <kmimetype.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <KoFilterManager.h>


KoFilter::KoFilter( QObject* parent ) : QObject( parent ), m_chain( 0 )
{
}

KoFilter::~KoFilter()
{
}


KoEmbeddingFilter::~KoEmbeddingFilter()
{
    if ( m_partStack.count() != 1 )
        kWarning() << "Someone messed with the part stack" << endl;
    delete m_partStack.pop();
}

int KoEmbeddingFilter::lruPartIndex() const
{
    return m_partStack.top()->m_lruPartIndex;
}

QString KoEmbeddingFilter::mimeTypeByExtension( const QString& extension )
{
    // We need to resort to an ugly hack to determine the mimetype
    // from the extension, as kservicetypefactory.h isn't installed
    KUrl url;
    url.setPath( QString( "dummy.%1" ).arg( extension ) );
    KMimeType::Ptr m( KMimeType::findByUrl( url, 0, true, true ) );
    return m->name();
}

KoEmbeddingFilter::KoEmbeddingFilter() : KoFilter()
{
    m_partStack.push( new PartState() );
}

int KoEmbeddingFilter::embedPart( const QByteArray& from, QByteArray& to,
                                  KoFilter::ConversionStatus& status, const QString& key )
{
    ++( m_partStack.top()->m_lruPartIndex );

    KTempFile tempIn;
    tempIn.setAutoDelete( true );
    savePartContents( tempIn.file() );
    tempIn.file()->close();

    KoFilterManager *manager = new KoFilterManager( tempIn.name(), from, m_chain );
    status = manager->exp0rt( QString::null, to );
    delete manager;

    // Add the part to the current "stack frame", using the number as key
    // if the key string is empty
    PartReference ref( lruPartIndex(), to );
    m_partStack.top()->m_partReferences.insert( key.isEmpty() ? QString::number( lruPartIndex() ) : key, ref );

    return lruPartIndex();
}

void KoEmbeddingFilter::startInternalEmbedding( const QString& key, const QByteArray& mimeType )
{
    filterChainEnterDirectory( QString::number( ++( m_partStack.top()->m_lruPartIndex ) ) );
    PartReference ref( lruPartIndex(), mimeType );
    m_partStack.top()->m_partReferences.insert( key, ref );
    m_partStack.push( new PartState() );
}

void KoEmbeddingFilter::endInternalEmbedding()
{
    if ( m_partStack.count() == 1 ) {
        kError( 30500 ) << "You're trying to endInternalEmbedding more often than you started it" << endl;
        return;
    }
    delete m_partStack.pop();
    filterChainLeaveDirectory();
}

int KoEmbeddingFilter::internalPartReference( const QString& key ) const
{
    QMap<QString, PartReference>::const_iterator it = m_partStack.top()->m_partReferences.find( key );
    if ( it == m_partStack.top()->m_partReferences.end() )
        return -1;
    return it.value().m_index;
}

QByteArray KoEmbeddingFilter::internalPartMimeType( const QString& key ) const
{
    QMap<QString, PartReference>::const_iterator it = m_partStack.top()->m_partReferences.find( key );
    if ( it == m_partStack.top()->m_partReferences.end() )
        return QByteArray();
    return it.value().m_mimeType;
}

KoEmbeddingFilter::PartReference::PartReference( int index, const QByteArray& mimeType ) :
    m_index( index ), m_mimeType( mimeType )
{
}

bool KoEmbeddingFilter::PartReference::isValid() const
{
    return m_index != 1 && !m_mimeType.isEmpty();
}

KoEmbeddingFilter::PartState::PartState() : m_lruPartIndex( 0 )
{
}

void KoEmbeddingFilter::savePartContents( QIODevice* )
{
}

void KoEmbeddingFilter::filterChainEnterDirectory( const QString& directory ) const
{
    m_chain->enterDirectory( directory );
}

void KoEmbeddingFilter::filterChainLeaveDirectory() const
{
    m_chain->leaveDirectory();
}

#include <KoFilter.moc>
