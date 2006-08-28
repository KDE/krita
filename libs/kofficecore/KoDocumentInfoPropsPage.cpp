/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>
   Copyright (C) 2004 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "KoDocumentInfoPropsPage.h"
#include "KoDocumentInfo.h"
#include "KoDocumentInfoDlg.h"
#include <KoXmlReader.h>
#include <ktar.h>
#include <ktempfile.h>
#include <kdeversion.h>
#include <kfilterdev.h>

#include <QBuffer>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

class KoDocumentInfoPropsPage::KoDocumentInfoPropsPagePrivate
{
public:
    KoDocumentInfo *m_info;
    KoDocumentInfoDlg *m_dlg;
    KUrl m_url;
    KTar *m_src;
    KTar *m_dst;

    const KArchiveFile *m_docInfoFile;
};

KoDocumentInfoPropsPage::KoDocumentInfoPropsPage( KPropertiesDialog *props,
        const QStringList & )
: KPropsDlgPlugin( props )
{
    d = new KoDocumentInfoPropsPagePrivate;
    d->m_info = new KoDocumentInfo( this );
    d->m_url = props->item()->url();
    d->m_dlg = 0;

    if ( !d->m_url.isLocalFile() )
        return;

    d->m_dst = 0;

#ifdef __GNUC__
#warning TODO port this to KoStore !!!
#endif
    d->m_src = new KTar( d->m_url.path(), "application/x-gzip" );

    if ( !d->m_src->open( QIODevice::ReadOnly ) )
        return;

    const KArchiveDirectory *root = d->m_src->directory();
    if ( !root )
        return;

    const KArchiveEntry *entry = root->entry( "documentinfo.xml" );

    if ( entry && entry->isFile() )
    {
        d->m_docInfoFile = static_cast<const KArchiveFile *>( entry );

        QByteArray data = d->m_docInfoFile->data();
        QBuffer buffer( &data );
        buffer.open( QIODevice::ReadOnly );

        KoXmlDocument doc;
        doc.setContent( &buffer );

        d->m_info->load( doc );
    }

    d->m_dlg = new KoDocumentInfoDlg( props, d->m_info );
    connect( d->m_dlg, SIGNAL( changed() ),
            this, SIGNAL( changed() ) );
}

KoDocumentInfoPropsPage::~KoDocumentInfoPropsPage()
{
    delete d->m_info;
    delete d->m_src;
    delete d->m_dst;
    delete d->m_dlg;
    delete d;
}

void KoDocumentInfoPropsPage::applyChanges()
{
    const KArchiveDirectory *root = d->m_src->directory();
    if ( !root )
        return;

    struct stat statBuff;

    if ( stat( QFile::encodeName( d->m_url.path() ), &statBuff ) != 0 )
        return;

    KTempFile tempFile( d->m_url.path(), QString::null, statBuff.st_mode );

    tempFile.setAutoDelete( true );

    if ( tempFile.status() != 0 )
        return;

    if ( !tempFile.close() )
        return;

    d->m_dst = new KTar( tempFile.name(), "application/x-gzip" );

    if ( !d->m_dst->open( QIODevice::WriteOnly ) )
        return;

    KMimeType::Ptr mimeType = KMimeType::findByUrl( d->m_url, 0, true );
    if ( mimeType && dynamic_cast<KFilterDev *>( d->m_dst->device() ) != 0 )
    {
        QByteArray appIdentification( "KOffice " ); // We are limited in the number of chars.
        appIdentification += mimeType->name().toLatin1();
        appIdentification += '\004'; // Two magic bytes to make the identification
        appIdentification += '\006'; // more reliable (DF)
        d->m_dst->setOrigFileName( appIdentification );
    }

    bool docInfoSaved = false;

    QStringList entries = root->entries();
    QStringList::ConstIterator it = entries.begin();
    QStringList::ConstIterator end = entries.end();
    for (; it != end; ++it )
    {
        const KArchiveEntry *entry = root->entry( *it );

        assert( entry );

        if ( entry->name() == "documentinfo.xml" ||
                ( !docInfoSaved && !entries.contains( "documentinfo.xml" ) ) )
        {
            d->m_dlg->slotApply();

            QBuffer buffer;
            buffer.open( QIODevice::WriteOnly );
            QTextStream str( &buffer );
            str << d->m_info->save();
            buffer.close();

            kDebug( 30003 ) << "writing documentinfo.xml" << endl;
            d->m_dst->writeFile( "documentinfo.xml", entry->user(), entry->group(),
                    buffer.buffer().data(), buffer.buffer().size() );

            docInfoSaved = true;
        }
        else
            copy( QString::null, entry );
    }

    d->m_dst->close();

    QDir dir;
    dir.rename( tempFile.name(), d->m_url.path() );

    delete d->m_dst;
    d->m_dst = 0;
}

void KoDocumentInfoPropsPage::copy( const QString &path, const KArchiveEntry *entry )
{
    kDebug( 30003 ) << "copy " << entry->name() << endl;
    if ( entry->isFile() )
    {
        const KArchiveFile *file = static_cast<const KArchiveFile *>( entry );
        kDebug( 30003 ) << "file :" << entry->name() << endl;
        kDebug( 30003 ) << "full path is: " << path << entry->name() << endl;
        d->m_dst->writeFile( path + entry->name(), entry->user(), entry->group(),
                file->data().data(), file->size() );
    }
    else
    {
        const KArchiveDirectory *dir = static_cast<const KArchiveDirectory*>( entry );
        kDebug( 30003 ) << "dir : " << entry->name() << endl;
        kDebug( 30003 ) << "full path is: " << path << entry->name() << endl;

        QString p = path + entry->name();
        if ( p != "/" )
        {
            d->m_dst->writeDir( p, entry->user(), entry->group() );
            p.append( "/" );
        }

        QStringList entries = dir->entries();
        QStringList::ConstIterator it = entries.begin();
        QStringList::ConstIterator end = entries.end();
        for (; it != end; ++it )
            copy( p, dir->entry( *it ) );
    }
}

#include "KoDocumentInfoPropsPage.moc"
