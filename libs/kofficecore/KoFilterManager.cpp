/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
                 2000, 2001 Werner Trobin <trobin@kde.org>
   Copyright (C) 2004 Nicolas Goutte <goutte@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/


#include <KoFilterManager.h>
#include <KoFilterManager_p.h>

#include <QFile>
#include <QLabel>
#include <QVBoxLayout>
#include <q3ptrlist.h>
#include <QApplication>
#include <QByteArray>
#include <Q3ValueList>

#include <klocale.h>
#include <kmessagebox.h>
#include <KoDocument.h>
#include <klibloader.h>
#include <klistbox.h>
#include <kmimetype.h>
#include <kdebug.h>

#include <queue>

#include <unistd.h>

class KoFilterManager::Private
{
public:
    bool m_batch;
};


KoFilterChooser::KoFilterChooser (QWidget *parent, const QStringList &mimeTypes, const QString &nativeFormat)
    : KDialog ( parent ),
    m_mimeTypes (mimeTypes)
{
    setObjectName( "kofilterchooser" );
    setButtons( Ok|Cancel );
    setInitialSize( QSize (300, 350) );
    setButtons( KDialog::Ok | KDialog::Cancel );
    setDefaultButton( KDialog::Ok );
    setCaption( i18n ("Choose Filter") );
    setModal( true );

    QWidget *page = new QWidget (this);
    setMainWidget (page);

    QLabel *filterLabel = new QLabel( i18n ("Select a filter:"), page );
    m_filterList = new KListBox (page, "filterlist");

    QVBoxLayout *layout = new QVBoxLayout( page );
    layout->addWidget (filterLabel);
    layout->addWidget (m_filterList);
    page->setLayout( layout );

    Q_ASSERT (!m_mimeTypes.isEmpty ());
    for (QStringList::ConstIterator it = m_mimeTypes.begin ();
            it != m_mimeTypes.end ();
            it++)
    {
        KMimeType::Ptr mime = KMimeType::mimeType (*it);
        m_filterList->insertItem (mime->comment ());
    }

    if (nativeFormat == "application/x-kword")
    {
        const int index = m_mimeTypes.indexOf( "text/plain" );
        if (index > -1)
            m_filterList->setCurrentItem (index);
    }

    if (m_filterList->currentItem () == -1)
        m_filterList->setCurrentItem (0);

    m_filterList->centerCurrentItem ();
    m_filterList->setFocus ();

    connect (m_filterList, SIGNAL (selected (int)), this, SLOT (slotOk ()));
}

KoFilterChooser::~KoFilterChooser ()
{
}

QString KoFilterChooser::filterSelected ()
{
    const int item = m_filterList->currentItem ();

    if (item > -1)
        return m_mimeTypes [item];
    else
        return QString::null;
}


// static cache for filter availability
QMap<QString, bool> KoFilterManager::m_filterAvailable;

const int KoFilterManager::s_area = 30500;


KoFilterManager::KoFilterManager( KoDocument* document ) :
    m_document( document ), m_parentChain( 0 ), m_graph( "" ), d( 0 )
{
    d = new KoFilterManager::Private;
    d -> m_batch = false;
    if ( document )
        QObject::connect( this, SIGNAL( sigProgress( int ) ),
                          document, SIGNAL( sigProgress( int ) ) );
}


KoFilterManager::KoFilterManager( const QString& url, const QByteArray& mimetypeHint,
                                  KoFilterChain* const parentChain ) :
    m_document( 0 ), m_parentChain( parentChain ), m_importUrl( url ), m_importUrlMimetypeHint( mimetypeHint ),
    m_graph( "" ), d( 0 )
{
    d = new KoFilterManager::Private;
    d -> m_batch = false;
}

KoFilterManager::~KoFilterManager()
{
    delete d;
}

QString KoFilterManager::import( const QString& url, KoFilter::ConversionStatus& status )
{
    // Find the mime type for the file to be imported.
    KUrl u;
    u.setPath( url );
    KMimeType::Ptr t = KMimeType::findByUrl( u, 0, true );
    if ( t->name() == KMimeType::defaultMimeType() ) {
        kError(s_area) << "No mimetype found for " << url << endl;
        status = KoFilter::BadMimeType;
        return QString::null;
    }

    m_graph.setSourceMimeType( t->name().toLatin1() );  // .latin1() is okay here (Werner)
    if ( !m_graph.isValid() ) {
        bool userCancelled = false;

        kWarning(s_area) << "Can't open " << t->name () << ", trying filter chooser" << endl;
        if ( m_document )
        {
	    if ( !m_document->isAutoErrorHandlingEnabled() )
	    {
		status = KoFilter::BadConversionGraph;
		return QString::null;
	    }
            QByteArray nativeFormat = m_document->nativeFormatMimeType ();

            QApplication::setOverrideCursor( Qt::ArrowCursor );
            KoFilterChooser chooser(0,
                                    KoFilterManager::mimeFilter (nativeFormat, KoFilterManager::Import, m_document->extraNativeMimeTypes()),
                                    nativeFormat);
            if (chooser.exec ())
            {
                QByteArray f = chooser.filterSelected ().toLatin1();

                if (f == nativeFormat)
                {
                    status = KoFilter::OK;
                    QApplication::restoreOverrideCursor();
                    return url;
                }

                m_graph.setSourceMimeType (f);
            }
            else
                userCancelled = true;
            QApplication::restoreOverrideCursor();
        }

        if (!m_graph.isValid())
        {
            kError(s_area) << "Couldn't create a valid graph for this source mimetype: "
                            << t->name() << endl;
            importErrorHelper( t->name(), userCancelled );
            status = KoFilter::BadConversionGraph;
            return QString::null;
        }
    }

    KoFilterChain::Ptr chain( 0 );
    // Are we owned by a KoDocument?
    if ( m_document ) {
        QByteArray mimeType = m_document->nativeFormatMimeType();
        QStringList extraMimes = m_document->extraNativeMimeTypes();
        int i=0, n = extraMimes.count();
        chain = m_graph.chain( this, mimeType );
        while( !chain && i<n) {
            mimeType = extraMimes[i].toUtf8();
            chain = m_graph.chain( this, mimeType );
            ++i;
        }
    }
    else {
        kError(s_area) << "You aren't supposed to use import() from a filter!" << endl;
        status = KoFilter::UsageError;
        return QString::null;
    }

    if ( !chain ) {
        kError(s_area) << "Couldn't create a valid filter chain!" << endl;
        importErrorHelper( t->name() );
        status = KoFilter::BadConversionGraph;
        return QString::null;
    }

    // Okay, let's invoke the filters one after the other
    m_direction = Import; // vital information!
    m_importUrl = url;  // We want to load that file
    m_exportUrl = QString::null;  // This is null for sure, as embedded stuff isn't
                                  // allowed to use that method
    status = chain->invokeChain();

    m_importUrl = QString::null;  // Reset the import URL

    if ( status == KoFilter::OK )
        return chain->chainOutput();
    return QString::null;
}

KoFilter::ConversionStatus KoFilterManager::exp0rt( const QString& url, QByteArray& mimeType )
{
    bool userCancelled = false;

    // The import url should already be set correctly (null if we have a KoDocument
    // file manager and to the correct URL if we have an embedded manager)
    m_direction = Export; // vital information!
    m_exportUrl = url;

    KoFilterChain::Ptr chain;
    if ( m_document ) {
        // We have to pick the right native mimetype as source.
        QStringList nativeMimeTypes;
        nativeMimeTypes.append( m_document->nativeFormatMimeType() );
        nativeMimeTypes += m_document->extraNativeMimeTypes();
        QStringList::ConstIterator it = nativeMimeTypes.begin();
        const QStringList::ConstIterator end = nativeMimeTypes.end();
        for ( ; !chain && it != end; ++it )
        {
            m_graph.setSourceMimeType( (*it).toLatin1() );
            if ( m_graph.isValid() )
                chain = m_graph.chain( this, mimeType );
        }
    }
    else if ( !m_importUrlMimetypeHint.isEmpty() ) {
        kDebug(s_area) << "Using the mimetype hint: '" << m_importUrlMimetypeHint << "'" << endl;
        m_graph.setSourceMimeType( m_importUrlMimetypeHint );
    }
    else {
        KUrl u;
        u.setPath( m_importUrl );
        KMimeType::Ptr t = KMimeType::findByUrl( u, 0, true );
        if ( t->name() == KMimeType::defaultMimeType() ) {
            kError(s_area) << "No mimetype found for " << m_importUrl << endl;
            return KoFilter::BadMimeType;
        }
        m_graph.setSourceMimeType( t->name().toLatin1() );

        if ( !m_graph.isValid() ) {
            kWarning(s_area) << "Can't open " << t->name () << ", trying filter chooser" << endl;

            QApplication::setOverrideCursor( Qt::ArrowCursor );
            KoFilterChooser chooser(0, KoFilterManager::mimeFilter ());
            if (chooser.exec ())
                m_graph.setSourceMimeType (chooser.filterSelected ().toLatin1 ());
            else
                userCancelled = true;

            QApplication::restoreOverrideCursor();
        }
    }

    if (!m_graph.isValid ())
    {
        kError(s_area) << "Couldn't create a valid graph for this source mimetype." << endl;
        if (!userCancelled) KMessageBox::error( 0L, i18n("Could not export file."), i18n("Missing Export Filter") );
        return KoFilter::BadConversionGraph;
    }

    if ( !chain ) // already set when coming from the m_document case
        chain = m_graph.chain( this, mimeType );

    if ( !chain ) {
        kError(s_area) << "Couldn't create a valid filter chain to " << mimeType << " !" << endl;
        KMessageBox::error( 0L, i18n("Could not export file."), i18n("Missing Export Filter") );
        return KoFilter::BadConversionGraph;
    }

    return chain->invokeChain();
}

namespace  // in order not to mess with the global namespace ;)
{
    // This class is needed only for the static mimeFilter method
    class Vertex
    {
    public:
        Vertex( const QByteArray& mimeType ) : m_color( White ), m_mimeType( mimeType ) {}

        enum Color { White, Gray, Black };
        Color color() const { return m_color; }
        void setColor( Color color ) { m_color = color; }

        QByteArray mimeType() const { return m_mimeType; }

        void addEdge( Vertex* vertex ) { if ( vertex ) m_edges.append( vertex ); }
        Q3PtrList<Vertex> edges() const { return m_edges; }

    private:
        Color m_color;
        QByteArray m_mimeType;
        Q3PtrList<Vertex> m_edges;
    };

    // Some helper methods for the static stuff
    // This method builds up the graph in the passed ascii dict
    void buildGraph( Q3AsciiDict<Vertex>& vertices, KoFilterManager::Direction direction )
    {
        QStringList stopList; // Lists of mimetypes that are considered end of chains
        stopList << "text/plain";
        stopList << "text/csv";
        stopList << "text/x-tex";
        stopList << "text/html";

        vertices.setAutoDelete( true );

        // partly copied from build graph, but I don't see any other
        // way without crude hacks, as we have to obey the direction here
        Q3ValueList<KoDocumentEntry> parts( KoDocumentEntry::query(false, QString::null) );
        Q3ValueList<KoDocumentEntry>::ConstIterator partIt( parts.begin() );
        Q3ValueList<KoDocumentEntry>::ConstIterator partEnd( parts.end() );

        while ( partIt != partEnd ) {
            QStringList nativeMimeTypes = ( *partIt ).service()->property( "X-KDE-ExtraNativeMimeTypes" ).toStringList();
            nativeMimeTypes += ( *partIt ).service()->property( "X-KDE-NativeMimeType" ).toString();
            QStringList::ConstIterator it = nativeMimeTypes.begin();
            const QStringList::ConstIterator end = nativeMimeTypes.end();
            for ( ; it != end; ++it )
                if ( !(*it).isEmpty() )
                    vertices.insert( (*it).toLatin1(), new Vertex( (*it).toLatin1() ) );
            ++partIt;
        }

        Q3ValueList<KoFilterEntry::Ptr> filters = KoFilterEntry::query(); // no constraint here - we want *all* :)
        Q3ValueList<KoFilterEntry::Ptr>::ConstIterator it = filters.begin();
        const Q3ValueList<KoFilterEntry::Ptr>::ConstIterator end = filters.end();

        for ( ; it != end; ++it ) {

            QStringList impList; // Import list
            QStringList expList; // Export list

            // Now we have to exclude the "stop" mimetypes (in the right direction!)
            if ( direction == KoFilterManager::Import ) {
                // Import: "stop" mime type should not appear in export
		foreach( QString testIt, ( *it )->export_ )
		{
	          if( !stopList.contains( testIt ) )
		    expList.append( testIt );
		}

                impList = ( *it )->import;
            }
            else {
                // Export: "stop" mime type should not appear in import
		foreach( QString testIt, (*it)->import )
		{
		  if( !stopList.contains( testIt ) )
		    impList.append( testIt );
		}

                expList = ( *it )->export_;
            }

            if ( impList.empty() || expList.empty() )
            {
                // This filter cannot be used under these conditions
                kDebug( 30500 ) << "Filter: " << ( *it )->service()->name() << " ruled out" << endl;
                continue;
            }

            // First add the "starting points" to the dict
            QStringList::ConstIterator importIt = impList.begin();
            const QStringList::ConstIterator importEnd = impList.end();
            for ( ; importIt != importEnd; ++importIt ) {
                const QByteArray key = ( *importIt ).toLatin1();  // latin1 is okay here (werner)
                // already there?
                if ( !vertices[ key ] )
                    vertices.insert( key, new Vertex( key ) );
            }

            // Are we allowed to use this filter at all?
            if ( KoFilterManager::filterAvailable( *it ) ) {
                QStringList::ConstIterator exportIt = expList.begin();
                const QStringList::ConstIterator exportEnd = expList.end();

                for ( ; exportIt != exportEnd; ++exportIt ) {
                    // First make sure the export vertex is in place
                    const QByteArray key = ( *exportIt ).toLatin1();  // latin1 is okay here
                    Vertex* exp = vertices[ key ];
                    if ( !exp ) {
                        exp = new Vertex( key );
                        vertices.insert( key, exp );
                    }
                    // Then create the appropriate edges depending on the
                    // direction (import/export)
                    // This is the chunk of code which actually differs from the
                    // graph stuff (apart from the different vertex class)
                    importIt = impList.begin(); // ### TODO: why only the first one?
                    if ( direction == KoFilterManager::Import ) {
                        for ( ; importIt != importEnd; ++importIt )
                            exp->addEdge( vertices[ ( *importIt ).toLatin1() ] );
                    } else {
                        for ( ; importIt != importEnd; ++importIt )
                            vertices[ ( *importIt ).toLatin1() ]->addEdge( exp );
                    }
                }
            }
            else
                kDebug( 30500 ) << "Filter: " << ( *it )->service()->name() << " does not apply." << endl;
        }
    }

    // This method runs a BFS on the graph to determine the connected
    // nodes. Make sure that the graph is "cleared" (the colors of the
    // nodes are all white)
    QStringList connected( const Q3AsciiDict<Vertex>& vertices, const QByteArray& mimetype )
    {
        if ( mimetype.isEmpty() )
            return QStringList();
        Vertex *v = vertices[ mimetype ];
        if ( !v )
            return QStringList();

        v->setColor( Vertex::Gray );
        std::queue<Vertex*> queue;
        queue.push( v );
        QStringList connected;

        while ( !queue.empty() ) {
            v = queue.front();
            queue.pop();
            Q3PtrList<Vertex> edges = v->edges();
            Q3PtrListIterator<Vertex> it( edges );
            for ( ; it.current(); ++it ) {
                if ( it.current()->color() == Vertex::White ) {
                    it.current()->setColor( Vertex::Gray );
                    queue.push( it.current() );
                }
            }
            v->setColor( Vertex::Black );
            connected.append( v->mimeType() );
        }
        return connected;
    }
}

// The static method to figure out to which parts of the
// graph this mimetype has a connection to.
QStringList KoFilterManager::mimeFilter( const QByteArray& mimetype, Direction direction, const QStringList& extraNativeMimeTypes )
{
    //kDebug(s_area) << "mimetype=" << mimetype << " extraNativeMimeTypes=" << extraNativeMimeTypes << endl;
    Q3AsciiDict<Vertex> vertices;
    buildGraph( vertices, direction );

    // TODO maybe use the fake vertex trick from the method below, to make the search faster?

    QStringList nativeMimeTypes;
    nativeMimeTypes.append( QString::fromLatin1( mimetype ) );
    nativeMimeTypes += extraNativeMimeTypes;

    // Add the native mimetypes first so that they are on top.
    QStringList lst = nativeMimeTypes;

    // Now look for filters which output each of those natives mimetypes
    foreach( QString natit, nativeMimeTypes )
    {
       const QStringList outMimes = connected( vertices, natit.toLatin1() );
     //kDebug(s_area) << k_funcinfo << "output formats connected to mime " << natit << " : " << outMimes << endl;
      foreach( QString mit, outMimes )
      {
        if ( !lst.contains( mit ) ) // append only if not there already. Qt4: QSet<QString>?
          lst.append( mit );
      }
    }

    return lst;
}

QStringList KoFilterManager::mimeFilter()
{
    Q3AsciiDict<Vertex> vertices;
    buildGraph( vertices, KoFilterManager::Import );

    Q3ValueList<KoDocumentEntry> parts( KoDocumentEntry::query(false, QString::null) );
    Q3ValueList<KoDocumentEntry>::ConstIterator partIt( parts.begin() );
    Q3ValueList<KoDocumentEntry>::ConstIterator partEnd( parts.end() );

    if ( partIt == partEnd )
        return QStringList();

    // To find *all* reachable mimetypes, we have to resort to
    // a small hat trick, in order to avoid multiple searches:
    // We introduce a fake vertex, which is connected to every
    // single KOffice mimetype. Due to that one BFS is enough :)
    // Now we just need an... ehrm.. unique name for our fake mimetype
    Vertex *v = new Vertex( "supercalifragilistic/x-pialadocious" );
    vertices.insert( "supercalifragilistic/x-pialadocious", v );
    while ( partIt != partEnd ) {
        QStringList nativeMimeTypes = ( *partIt ).service()->property( "X-KDE-ExtraNativeMimeTypes" ).toStringList();
        nativeMimeTypes += ( *partIt ).service()->property( "X-KDE-NativeMimeType" ).toString();
        QStringList::ConstIterator it = nativeMimeTypes.begin();
        const QStringList::ConstIterator end = nativeMimeTypes.end();
        for ( ; it != end; ++it )
            if ( !(*it).isEmpty() )
                v->addEdge( vertices[ (*it).toLatin1() ] );
        ++partIt;
    }
    QStringList result = connected( vertices, "supercalifragilistic/x-pialadocious" );

    // Finally we have to get rid of our fake mimetype again
    result.removeAll( "supercalifragilistic/x-pialadocious" );
    return result;
}

// Here we check whether the filter is available. This stuff is quite slow,
// but I don't see any other convenient (for the user) way out :}
bool KoFilterManager::filterAvailable( KoFilterEntry::Ptr entry )
{
    if ( !entry )
        return false;
    if ( entry->available != "check" )
        return true;

    //kDebug( 30500 ) << "Checking whether " << entry->service()->name() << " applies." << endl;
    // generate some "unique" key
    QString key( entry->service()->name() );
    key += " - ";
    key += entry->service()->library();

    if ( !m_filterAvailable.contains( key ) ) {
        //kDebug( 30500 ) << "Not cached, checking..." << endl;

        KLibrary* library = KLibLoader::self()->library( QFile::encodeName( entry->service()->library() ) );
        if ( !library ) {
            kWarning( 30500 ) << "Huh?? Couldn't load the lib: "
                               << KLibLoader::self()->lastErrorMessage() << endl;
            m_filterAvailable[ key ] = false;
            return false;
        }

        // This code is "borrowed" from klibloader ;)
        QByteArray symname = "check_" + library->name().toLatin1();
        void* sym = library->symbol( symname );
        if ( !sym )
        {
            kWarning( 30500 ) << "The library " << library->name()
                               << " does not offer a check_" << library->name()
                               << " function." << endl;
            m_filterAvailable[ key ] = false;
        }
        else {
            typedef int (*t_func)();
            t_func check = (t_func)sym;
            m_filterAvailable[ key ] = check() == 1;
        }
    }
    return m_filterAvailable[ key ];
}

void KoFilterManager::importErrorHelper( const QString& mimeType, const bool suppressDialog )
{
    QString tmp = i18n("Could not import file of type\n%1", mimeType );
    // ###### FIXME: use KLibLoader::lastErrorMessage() here
    if (!suppressDialog) KMessageBox::error( 0L, tmp, i18n("Missing Import Filter") );
}

void KoFilterManager::setBatchMode( const bool batch )
{
    d->m_batch = batch;
}

bool KoFilterManager::getBatchMode( void ) const
{
    return d->m_batch;
}

#include <KoFilterManager.moc>
#include <KoFilterManager_p.moc>
