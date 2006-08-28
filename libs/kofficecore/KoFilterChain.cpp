/* This file is part of the KOffice libraries
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>

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

#include <QMetaObject>
#include <QMetaMethod>
//Added by qt3to4:
#include <Q3StrList>
#include <Q3ValueList>
#include <QByteArray>
#include <ktempfile.h>
#include <kmimetype.h>
#include <KoFilterChain.h>
#include <KoQueryTrader.h>
#include <KoFilterManager.h>  // KoFilterManager::filterAvailable, private API
#include <KoDocument.h>
#include <kdebug.h>

#include <priorityqueue.h>

#include <limits.h> // UINT_MAX

// Those "defines" are needed in the setupConnections method below.
// Please always keep the strings and the length in sync!
namespace {
    const char* const SIGNAL_PREFIX = "commSignal";
    const int SIGNAL_PREFIX_LEN = 10;
    const char* const SLOT_PREFIX = "commSlot";
    const int SLOT_PREFIX_LEN = 8;
}


KoFilterChain::ChainLink::ChainLink( KoFilterChain* chain, KoFilterEntry::Ptr filterEntry,
                                     const QByteArray& from, const QByteArray& to ) :
    m_chain( chain ), m_filterEntry( filterEntry ), m_from( from ), m_to( to ),
    m_filter( 0 ), d( 0 )
{
}

KoFilter::ConversionStatus KoFilterChain::ChainLink::invokeFilter( const ChainLink* const parentChainLink )
{
    if ( !m_filterEntry ) {
        kError( 30500 ) << "This filter entry is null. Strange stuff going on." << endl;
        return KoFilter::CreationError;
    }

    m_filter = m_filterEntry->createFilter( m_chain );

    if ( !m_filter ) {
        kError( 30500 ) << "Couldn't create the filter." << endl;
        return KoFilter::CreationError;
    }

    if ( parentChainLink )
        setupCommunication( parentChainLink->m_filter );

    KoFilter::ConversionStatus status = m_filter->convert( m_from, m_to );
    delete m_filter;
    m_filter=0;
    return status;
}

void KoFilterChain::ChainLink::dump() const
{
    kDebug( 30500 ) << "   Link: " << m_filterEntry->service()->name() << endl;
}

int KoFilterChain::ChainLink::lruPartIndex() const
{
    if ( m_filter && m_filter->inherits( "KoEmbeddingFilter" ) )
        return static_cast<KoEmbeddingFilter*>( m_filter )->lruPartIndex();
    return -1;
}

void KoFilterChain::ChainLink::setupCommunication( const KoFilter* const parentFilter ) const
{
    // progress information
    QObject::connect( m_filter, SIGNAL( sigProgress( int ) ),
                      m_chain->manager(), SIGNAL( sigProgress( int ) ) );

    if ( !parentFilter )
        return;

    const QMetaObject* const parent = parentFilter->metaObject();
    const QMetaObject* const child = m_filter->metaObject();
    if ( !parent || !child )
        return;

    setupConnections( parentFilter, m_filter );
    setupConnections( m_filter, parentFilter );
}

void KoFilterChain::ChainLink::setupConnections( const KoFilter* sender, const KoFilter* receiver ) const
{
    const QMetaObject* const parent = sender->metaObject();
    const QMetaObject* const child = receiver->metaObject();
    if ( !parent || !child )
        return;

    int senderMethodCount = parent->methodCount();
    for ( int i=0; i < senderMethodCount; ++i ) {
        QMetaMethod signal = parent->method( i );
        if ( signal.methodType() != QMetaMethod::Signal )
            continue;
        // ### untested (QMetaMethod::signature())
        if ( strncmp( signal.signature(), SIGNAL_PREFIX, SIGNAL_PREFIX_LEN ) == 0 ) {
            int receiverMethodCount = child->methodCount();
            for ( int j=0; j < receiverMethodCount; ++j ) {
                QMetaMethod slot = child->method( j );
                if ( slot.methodType() != QMetaMethod::Slot )
                    continue;
                if ( strncmp( slot.signature(), SLOT_PREFIX, SLOT_PREFIX_LEN ) == 0 ) {
                  if ( strcmp( signal.signature() + SIGNAL_PREFIX_LEN, slot.signature() + SLOT_PREFIX_LEN ) == 0 ) {
                        QByteArray signalString;
                        signalString.setNum( QSIGNAL_CODE );
                        signalString += signal.signature();
                        QByteArray slotString;
                        slotString.setNum( QSLOT_CODE );
                        slotString += slot.signature();
                        QObject::connect( sender, signalString, receiver, slotString );
                    }
                }
            }
        }
    }
}


KoFilterChain::~KoFilterChain()
{
    if ( filterManagerParentChain() && filterManagerParentChain()->m_outputStorage )
        filterManagerParentChain()->m_outputStorage->leaveDirectory();
    manageIO(); // Called for the 2nd time in a row -> clean up
}

KoFilter::ConversionStatus KoFilterChain::invokeChain()
{
    KoFilter::ConversionStatus status = KoFilter::OK;

    m_state = Beginning;
    int count = m_chainLinks.count();

    // This is needed due to nasty Microsoft design
    const ChainLink* parentChainLink = 0;
    if ( filterManagerParentChain() )
        parentChainLink = filterManagerParentChain()->m_chainLinks.current();

    // No iterator here, as we need m_chainLinks.current() in outputDocument()
    m_chainLinks.first();
    for ( ; count > 1 && m_chainLinks.current() && status == KoFilter::OK;
          m_chainLinks.next(), --count ) {
        status = m_chainLinks.current()->invokeFilter( parentChainLink );
        m_state = Middle;
        manageIO();
    }

    if ( !m_chainLinks.current() ) {
        kWarning( 30500 ) << "Huh?? Found a null pointer in the chain" << endl;
        return KoFilter::StupidError;
    }

    if ( status == KoFilter::OK ) {
        if ( m_state & Beginning )
            m_state |= End;
        else
            m_state = End;
        status = m_chainLinks.current()->invokeFilter( parentChainLink );
        manageIO();
    }

    m_state = Done;
    if (status == KoFilter::OK)
      finalizeIO();
    return status;
}

QString KoFilterChain::chainOutput() const
{
    if ( m_state == Done )
        return m_inputFile; // as we already called manageIO()
    return QString::null;
}

QString KoFilterChain::inputFile()
{
    if ( m_inputQueried == File )
        return m_inputFile;
    else if ( m_inputQueried != Nil ) {
        kWarning( 30500 ) << "You already asked for some different source." << endl;
        return QString::null;
    }
    m_inputQueried = File;

    if ( m_state & Beginning ) {
        if ( static_cast<KoFilterManager::Direction>( filterManagerDirection() ) ==
             KoFilterManager::Import )
            m_inputFile = filterManagerImportFile();
        else
            inputFileHelper( filterManagerKoDocument(), filterManagerImportFile() );
    }
    else
        if ( m_inputFile.isEmpty() )
            inputFileHelper( m_inputDocument, QString::null );

    return m_inputFile;
}

QString KoFilterChain::outputFile()
{
    // sanity check: No embedded filter should ask for a plain file
    // ###### CHECK: This will break as soon as we support exporting embedding filters
    if ( filterManagerParentChain() )
        kWarning( 30500 )<< "An embedded filter has to use storageFile()!" << endl;

    if ( m_outputQueried == File )
        return m_outputFile;
    else if ( m_outputQueried != Nil ) {
        kWarning( 30500 ) << "You already asked for some different destination." << endl;
        return QString::null;
    }
    m_outputQueried = File;

    if ( m_state & End ) {
        if ( static_cast<KoFilterManager::Direction>( filterManagerDirection() ) ==
             KoFilterManager::Import )
            outputFileHelper( false );  // This (last) one gets deleted by the caller
        else
            m_outputFile = filterManagerExportFile();
    }
    else
        outputFileHelper( true );

    return m_outputFile;
}

KoStoreDevice* KoFilterChain::storageFile( const QString& name, KoStore::Mode mode )
{
    // ###### CHECK: This works only for import filters. Do we want something like
    // that for export filters too?
    if ( m_outputQueried == Nil && mode == KoStore::Write && filterManagerParentChain() )
        return storageInitEmbedding( name );

    // Plain normal use case
    if ( m_inputQueried == Storage && mode == KoStore::Read &&
         m_inputStorage && m_inputStorage->mode() == KoStore::Read )
        return storageNewStreamHelper( &m_inputStorage, &m_inputStorageDevice, name );
    else if ( m_outputQueried == Storage && mode == KoStore::Write &&
              m_outputStorage && m_outputStorage->mode() == KoStore::Write )
        return storageNewStreamHelper( &m_outputStorage, &m_outputStorageDevice, name );
    else if ( m_inputQueried == Nil && mode == KoStore::Read )
        return storageHelper( inputFile(), name, KoStore::Read,
                              &m_inputStorage, &m_inputStorageDevice );
    else if ( m_outputQueried == Nil && mode == KoStore::Write )
        return storageHelper( outputFile(), name, KoStore::Write,
                              &m_outputStorage, &m_outputStorageDevice );
    else {
        kWarning( 30500 ) << "Oooops, how did we get here? You already asked for a"
                           << " different source/destination?" << endl;
        return 0;
    }
}

KoDocument* KoFilterChain::inputDocument()
{
    if ( m_inputQueried == Document )
        return m_inputDocument;
    else if ( m_inputQueried != Nil ) {
        kWarning( 30500 ) << "You already asked for some different source." << endl;
        return 0;
    }

    if ( ( m_state & Beginning ) &&
         static_cast<KoFilterManager::Direction>( filterManagerDirection() ) == KoFilterManager::Export &&
         filterManagerKoDocument() )
        m_inputDocument = filterManagerKoDocument();
    else if ( !m_inputDocument )
        m_inputDocument = createDocument( inputFile() );

    m_inputQueried = Document;
    return m_inputDocument;
}

KoDocument* KoFilterChain::outputDocument()
{
    // sanity check: No embedded filter should ask for a document
    // ###### CHECK: This will break as soon as we support exporting embedding filters
    if ( filterManagerParentChain() ) {
        kWarning( 30500 )<< "An embedded filter has to use storageFile()!" << endl;
        return 0;
    }

    if ( m_outputQueried == Document )
        return m_outputDocument;
    else if ( m_outputQueried != Nil ) {
        kWarning( 30500 ) << "You already asked for some different destination." << endl;
        return 0;
    }

    if ( ( m_state & End ) &&
         static_cast<KoFilterManager::Direction>( filterManagerDirection() ) == KoFilterManager::Import &&
         filterManagerKoDocument() )
        m_outputDocument = filterManagerKoDocument();
    else
        m_outputDocument = createDocument( m_chainLinks.current()->to() );

    m_outputQueried = Document;
    return m_outputDocument;
}

void KoFilterChain::dump() const
{
    kDebug( 30500 ) << "########## KoFilterChain with " << m_chainLinks.count() << " members:" << endl;
    Q3PtrListIterator<ChainLink> it( m_chainLinks );
    for ( ; it.current(); ++it )
        it.current()->dump();
    kDebug( 30500 ) << "########## KoFilterChain (done) ##########" << endl;
}

KoFilterChain::KoFilterChain( const KoFilterManager* manager ) :
    m_manager( manager ), m_state( Beginning ), m_inputStorage( 0 ),
    m_inputStorageDevice( 0 ), m_outputStorage( 0 ), m_outputStorageDevice( 0 ),
    m_inputDocument( 0 ), m_outputDocument( 0 ), m_inputTempFile( 0 ),
    m_outputTempFile( 0 ), m_inputQueried( Nil ), m_outputQueried( Nil ), d( 0 )
{
    // We "own" our chain links, the filter entries are implicitly shared
    m_chainLinks.setAutoDelete( true );
}

void KoFilterChain::appendChainLink( KoFilterEntry::Ptr filterEntry, const QByteArray& from, const QByteArray& to )
{
    m_chainLinks.append( new ChainLink( this, filterEntry, from, to ) );
}

void KoFilterChain::prependChainLink( KoFilterEntry::Ptr filterEntry, const QByteArray& from, const QByteArray& to )
{
    m_chainLinks.prepend( new ChainLink( this, filterEntry, from, to ) );
}

void KoFilterChain::enterDirectory( const QString& directory )
{
    // Only a little bit of checking as we (have to :} ) trust KoEmbeddingFilter
    // If the output storage isn't initialized yet, we perform that step(s) on init.
    if ( m_outputStorage )
        m_outputStorage->enterDirectory( directory );
    m_internalEmbeddingDirectories.append( directory );
}

void KoFilterChain::leaveDirectory()
{
    if ( m_outputStorage )
        m_outputStorage->leaveDirectory();
    if ( !m_internalEmbeddingDirectories.isEmpty() )
        m_internalEmbeddingDirectories.pop_back();
}

QString KoFilterChain::filterManagerImportFile() const
{
    return m_manager->importFile();
}

QString KoFilterChain::filterManagerExportFile() const
{
    return m_manager->exportFile();
}

KoDocument* KoFilterChain::filterManagerKoDocument() const
{
    return m_manager->document();
}

int KoFilterChain::filterManagerDirection() const
{
    return m_manager->direction();
}

KoFilterChain* const KoFilterChain::filterManagerParentChain() const
{
    return m_manager->parentChain();
}

void KoFilterChain::manageIO()
{
    m_inputQueried = Nil;
    m_outputQueried = Nil;

    delete m_inputStorageDevice;
    m_inputStorageDevice = 0;
    if ( m_inputStorage ) {
        m_inputStorage->close();
        delete m_inputStorage;
        m_inputStorage = 0;
    }
    if ( m_inputTempFile ) {
        m_inputTempFile->close();
        delete m_inputTempFile;  // autodelete
        m_inputTempFile = 0;
    }
    m_inputFile = QString::null;

    if ( !m_outputFile.isEmpty() ) {
        m_inputFile = m_outputFile;
        m_outputFile = QString::null;
        m_inputTempFile = m_outputTempFile;
        m_outputTempFile = 0;

        delete m_outputStorageDevice;
        m_outputStorageDevice = 0;
        if ( m_outputStorage ) {
            m_outputStorage->close();
            // Don't delete the storage if we're just pointing to the
            // storage of the parent filter chain
            if ( !filterManagerParentChain() || m_outputStorage->mode() != KoStore::Write )
                delete m_outputStorage;
            m_outputStorage = 0;
        }
    }

    if ( m_inputDocument != filterManagerKoDocument() )
        delete m_inputDocument;
    m_inputDocument = m_outputDocument;
    m_outputDocument = 0;
}

void KoFilterChain::finalizeIO()
{
    // In case we export (to a file, of course) and the last
    // filter chose to output a KoDocument we have to save it.
    // Should be very rare, but well...
    // Note: m_*input*Document as we already called manageIO()
    if ( m_inputDocument &&
         static_cast<KoFilterManager::Direction>( filterManagerDirection() ) == KoFilterManager::Export ) {
        kDebug( 30500 ) << "Saving the output document to the export file" << endl;
        m_inputDocument->saveNativeFormat( filterManagerExportFile() );
        m_inputFile = filterManagerExportFile();
    }
}

bool KoFilterChain::createTempFile( KTempFile** tempFile, bool autoDelete )
{
    if ( *tempFile ) {
        kError( 30500 ) << "Ooops, why is there already a temp file???" << endl;
        return false;
    }
    *tempFile = new KTempFile();
    ( *tempFile )->setAutoDelete( autoDelete );
    return ( *tempFile )->status() == 0;
}

void KoFilterChain::inputFileHelper( KoDocument* document, const QString& alternativeFile )
{
    if ( document ) {
        if ( !createTempFile( &m_inputTempFile ) ) {
            delete m_inputTempFile;
            m_inputTempFile = 0;
            m_inputFile = QString::null;
            return;
        }
        if ( !document->saveNativeFormat( m_inputTempFile->name() ) ) {
            delete m_inputTempFile;
            m_inputTempFile = 0;
            m_inputFile = QString::null;
            return;
        }
        m_inputFile = m_inputTempFile->name();
    }
    else
        m_inputFile = alternativeFile;
}

void KoFilterChain::outputFileHelper( bool autoDelete )
{
    if ( !createTempFile( &m_outputTempFile, autoDelete ) ) {
        delete m_outputTempFile;
        m_outputTempFile = 0;
        m_outputFile = QString::null;
    }
    else
        m_outputFile = m_outputTempFile->name();
}

KoStoreDevice* KoFilterChain::storageNewStreamHelper( KoStore** storage, KoStoreDevice** device,
                                                      const QString& name )
{
    delete *device;
    *device = 0;
    if ( ( *storage )->isOpen() )
        ( *storage )->close();
    if ( ( *storage )->bad() )
        return storageCleanupHelper( storage );
    if ( !( *storage )->open( name ) )
        return 0;

    *device = new KoStoreDevice( *storage );
    return *device;
}

KoStoreDevice* KoFilterChain::storageHelper( const QString& file, const QString& streamName,
                                             KoStore::Mode mode, KoStore** storage,
                                             KoStoreDevice** device )
{
    if ( file.isEmpty() )
        return 0;
    if ( *storage ) {
        kDebug( 30500 ) << "Uh-oh, we forgot to clean up..." << endl;
        return 0;
    }

    storageInit( file, mode, storage );

    if ( ( *storage )->bad() )
        return storageCleanupHelper( storage );

    // Seems that we got a valid storage, at least. Even if we can't open
    // the stream the "user" asked us to open, we nontheless change the
    // IOState from File to Storage, as it might be possible to open other streams
    if ( mode == KoStore::Read )
        m_inputQueried = Storage;
    else // KoStore::Write
        m_outputQueried = Storage;

    return storageCreateFirstStream( streamName, storage, device );
}

void KoFilterChain::storageInit( const QString& file, KoStore::Mode mode, KoStore** storage )
{
    QByteArray appIdentification( "" );
    if ( mode == KoStore::Write ) {
        // To create valid storages we also have to add the mimetype
        // magic "applicationIndentifier" to the storage.
        // As only filters with a KOffice destination should query
        // for a storage to write to, we don't check the content of
        // the mimetype here. It doesn't do a lot of harm if someome
        // "abuses" this method.
        appIdentification = m_chainLinks.current()->to();
    }
    *storage = KoStore::createStore( file, mode, appIdentification );
}

KoStoreDevice* KoFilterChain::storageInitEmbedding( const QString& name )
{
    if ( m_outputStorage ) {
        kWarning( 30500 ) << "Ooops! Something's really screwed here." << endl;
        return 0;
    }

    m_outputStorage = filterManagerParentChain()->m_outputStorage;

    if ( !m_outputStorage ) {
        // If the storage of the parent hasn't been initialized yet,
        // we have to do that here. Quite nasty...
        storageInit( filterManagerParentChain()->outputFile(), KoStore::Write, &m_outputStorage );

        // transfer the ownership
        filterManagerParentChain()->m_outputStorage = m_outputStorage;
        filterManagerParentChain()->m_outputQueried = Storage;
    }

    if ( m_outputStorage->isOpen() )
        m_outputStorage->close();  // to be on the safe side, should never happen
    if ( m_outputStorage->bad() )
        return storageCleanupHelper( &m_outputStorage );

    m_outputQueried = Storage;

    // Now that we have a storage we have to change the directory
    // and remember it for later!
    const int lruPartIndex = filterManagerParentChain()->m_chainLinks.current()->lruPartIndex();
    if ( lruPartIndex == -1 ) {
        kError( 30500 ) << "Huh! You want to use embedding features w/o inheriting KoEmbeddingFilter?" << endl;
        return storageCleanupHelper( &m_outputStorage );
    }

    if ( !m_outputStorage->enterDirectory( QString( "part%1" ).arg( lruPartIndex ) ) )
        return storageCleanupHelper( &m_outputStorage );

    return storageCreateFirstStream( name, &m_outputStorage, &m_outputStorageDevice );
}

KoStoreDevice* KoFilterChain::storageCreateFirstStream( const QString& streamName, KoStore** storage,
                                                        KoStoreDevice** device )
{
    // Before we go and create the first stream in this storage we
    // have to perform a little hack in case we're used by any ole-style
    // filter which utilizes internal embedding. Ugly, but well...
    if ( !m_internalEmbeddingDirectories.isEmpty() ) {
        QStringList::ConstIterator it = m_internalEmbeddingDirectories.begin();
        QStringList::ConstIterator end = m_internalEmbeddingDirectories.end();
        for ( ; it != end && ( *storage )->enterDirectory( *it ); ++it );
    }

    if ( !( *storage )->open( streamName ) )
        return 0;

    if ( *device ) {
        kDebug( 30500 ) << "Uh-oh, we forgot to clean up the storage device!" << endl;
        ( *storage )->close();
        return storageCleanupHelper( storage );
    }
    *device = new KoStoreDevice( *storage );
    return *device;
}

KoStoreDevice* KoFilterChain::storageCleanupHelper( KoStore** storage )
{
    // Take care not to delete the storage of the parent chain
    if ( *storage != m_outputStorage || !filterManagerParentChain() ||
         ( *storage )->mode() != KoStore::Write )
        delete *storage;
    *storage = 0;
    return 0;
}

KoDocument* KoFilterChain::createDocument( const QString& file )
{
    KUrl url;
    url.setPath( file );
    KMimeType::Ptr t = KMimeType::findByUrl( url, 0, true );
    if ( t->name() == KMimeType::defaultMimeType() ) {
        kError( 30500 ) << "No mimetype found for " << file << endl;
        return 0;
    }

    KoDocument *doc = createDocument( QByteArray( t->name().toLatin1() ) );

    if ( !doc || !doc->loadNativeFormat( file ) ) {
        kError( 30500 ) << "Couldn't load from the file" << endl;
        delete doc;
        return 0;
    }
    return doc;
}

KoDocument* KoFilterChain::createDocument( const QByteArray& mimeType )
{
    KoDocumentEntry entry = KoDocumentEntry::queryByMimeType(mimeType);

    if (entry.isEmpty())
    {
        kError( 30500 ) << "Couldn't find a part that can handle mimetype " << mimeType << endl;
    }

    QString errorMsg;
    KoDocument* doc = entry.createDoc( &errorMsg ); /*entries.first().createDoc();*/
    if ( !doc ) {
        kError( 30500 ) << "Couldn't create the document: " << errorMsg << endl;
        return 0;
    }
    return doc;
}


namespace KOffice {

    Edge::Edge( Vertex* vertex, KoFilterEntry::Ptr filterEntry ) :
        m_vertex( vertex ), m_filterEntry( filterEntry ), d( 0 )
    {
    }

    void Edge::relax( const Vertex* predecessor, PriorityQueue<Vertex>& queue )
    {
        if ( !m_vertex || !predecessor || !m_filterEntry )
            return;
        if ( m_vertex->setKey( predecessor->key() + m_filterEntry->weight ) ) {
            queue.keyDecreased( m_vertex ); // maintain the heap property
            m_vertex->setPredecessor( predecessor );
        }
    }

    void Edge::dump( const QByteArray& indent ) const
    {
        if ( m_vertex )
            kDebug( 30500 ) << indent << "Edge -> '" << m_vertex->mimeType()
                             << "' (" << m_filterEntry->weight << ")" << endl;
        else
            kDebug( 30500 ) << indent << "Edge -> '(null)' ("
                             << m_filterEntry->weight << ")" << endl;
    }


    Vertex::Vertex( const QByteArray& mimeType ) : m_predecessor( 0 ), m_mimeType( mimeType ),
        m_weight( UINT_MAX ), m_index( -1 ), d( 0 )
    {
        m_edges.setAutoDelete( true );  // we take ownership of added edges
    }

    bool Vertex::setKey( unsigned int key )
    {
        if ( m_weight > key ) {
            m_weight = key;
            return true;
        }
        return false;
    }

    void Vertex::reset()
    {
        m_weight = UINT_MAX;
        m_predecessor = 0;
    }

    void Vertex::addEdge( const Edge* edge )
    {
        if ( !edge || edge->weight() == 0 )
            return;
        m_edges.append( edge );
    }

    const Edge* Vertex::findEdge( const Vertex* vertex ) const
    {
        if ( !vertex )
            return 0;
        const Edge* edge = 0;
        Q3PtrListIterator<Edge> it( m_edges );

        for ( ; it.current(); ++it ) {
            if ( it.current()->vertex() == vertex &&
                 ( !edge || it.current()->weight() < edge->weight() ) )
                edge = it.current();
        }
        return edge;
    }

    void Vertex::relaxVertices( PriorityQueue<Vertex>& queue )
    {
        for ( Edge *e = m_edges.first(); e; e = m_edges.next() )
            e->relax( this, queue );
    }

    void Vertex::dump( const QByteArray& indent ) const
    {
        kDebug( 30500 ) << indent << "Vertex: " << m_mimeType << " (" << m_weight << "):" << endl;
        const QByteArray i( indent + "   " );
        Q3PtrListIterator<Edge> it( m_edges );
        for ( ; it.current(); ++it )
            it.current()->dump( i );
    }


    Graph::Graph( const QByteArray& from ) : m_vertices( 47 ), m_from( from ),
                                           m_graphValid( false ), d( 0 )
    {
        m_vertices.setAutoDelete( true );
        buildGraph();
        shortestPaths();  // Will return after a single lookup if "from" is invalid (->no check here)
    }

    void Graph::setSourceMimeType( const QByteArray& from )
    {
        if ( from == m_from )
            return;
        m_from = from;
        m_graphValid = false;

        // Initialize with "infinity" ...
        Q3AsciiDictIterator<Vertex> it( m_vertices );
        for ( ; it.current(); ++it )
            it.current()->reset();

        // ...and re-run the shortest path search for the new source mime
        shortestPaths();
    }

    KoFilterChain::Ptr Graph::chain( const KoFilterManager* manager, QByteArray& to ) const
    {
        if ( !isValid() || !manager )
            return KoFilterChain::Ptr();

        if ( to.isEmpty() ) {  // if the destination is empty we search the closest KOffice part
            to = findKOfficePart();
            if ( to.isEmpty() )  // still empty? strange stuff...
                return KoFilterChain::Ptr();
        }

        const Vertex* vertex = m_vertices[ to ];
        if ( !vertex || vertex->key() == UINT_MAX )
            return KoFilterChain::Ptr();

        KoFilterChain::Ptr ret( new KoFilterChain( manager ) );

        // Fill the filter chain with all filters on the path
        const Vertex* tmp = vertex->predecessor();
        while ( tmp ) {
            const Edge* const edge = tmp->findEdge( vertex );
            Q_ASSERT( edge );
            ret->prependChainLink( edge->filterEntry(), tmp->mimeType(), vertex->mimeType() );
            vertex = tmp;
            tmp = tmp->predecessor();
        }
        return ret;
    }

    void Graph::dump() const
    {
        kDebug( 30500 ) << "+++++++++ Graph::dump +++++++++" << endl;
        kDebug( 30500 ) << "From: " << m_from << endl;
        Q3AsciiDictIterator<Vertex> it( m_vertices );
        for ( ; it.current(); ++it )
            it.current()->dump( "   " );
        kDebug( 30500 ) << "+++++++++ Graph::dump (done) +++++++++" << endl;
    }

    // Query the trader and create the vertices and edges representing
    // available mime types and filters.
    void Graph::buildGraph()
    {
        // Make sure that all available parts are added to the graph
        Q3ValueList<KoDocumentEntry> parts( KoDocumentEntry::query() );
        Q3ValueList<KoDocumentEntry>::ConstIterator partIt( parts.begin() );
        Q3ValueList<KoDocumentEntry>::ConstIterator partEnd( parts.end() );

        while ( partIt != partEnd ) {
            QStringList nativeMimeTypes = ( *partIt ).service()->property( "X-KDE-ExtraNativeMimeTypes" ).toStringList();
            nativeMimeTypes += ( *partIt ).service()->property( "X-KDE-NativeMimeType" ).toString();
            QStringList::ConstIterator it = nativeMimeTypes.begin();
            QStringList::ConstIterator end = nativeMimeTypes.end();
            for ( ; it != end; ++it )
                if ( !(*it).isEmpty() )
                    m_vertices.insert( (*it).toLatin1(), new Vertex( (*it).toLatin1() ) );
            ++partIt;
        }

        Q3ValueList<KoFilterEntry::Ptr> filters( KoFilterEntry::query() ); // no constraint here - we want *all* :)
        Q3ValueList<KoFilterEntry::Ptr>::ConstIterator it = filters.begin();
        Q3ValueList<KoFilterEntry::Ptr>::ConstIterator end = filters.end();

        for ( ; it != end; ++it ) {
            // First add the "starting points" to the dict
            QStringList::ConstIterator importIt = ( *it )->import.begin();
            QStringList::ConstIterator importEnd = ( *it )->import.end();
            for ( ; importIt != importEnd; ++importIt ) {
                const QByteArray key = ( *importIt ).toLatin1();  // latin1 is okay here (werner)
                // already there?
                if ( !m_vertices[ key ] )
                    m_vertices.insert( key, new Vertex( key ) );
            }

            // Are we allowed to use this filter at all?
            if ( KoFilterManager::filterAvailable( *it ) ) {
                QStringList::ConstIterator exportIt = ( *it )->export_.begin();
                QStringList::ConstIterator exportEnd = ( *it )->export_.end();

                for ( ; exportIt != exportEnd; ++exportIt ) {
                    // First make sure the export vertex is in place
                    const QByteArray key = ( *exportIt ).toLatin1();  // latin1 is okay here
                    Vertex* exp = m_vertices[ key ];
                    if ( !exp ) {
                        exp = new Vertex( key );
                        m_vertices.insert( key, exp );
                    }
                    // Then create the appropriate edges
                    importIt = ( *it )->import.begin();
                    for ( ; importIt != importEnd; ++importIt )
                        m_vertices[ ( *importIt ).toLatin1() ]->addEdge( new Edge( exp, *it ) );
                }
            }
            else
                kDebug( 30500 ) << "Filter: " << ( *it )->service()->name() << " doesn't apply." << endl;
        }
    }

    // As all edges (=filters) are required to have a positive weight
    // we can use Dijkstra's shortest path algorithm from Cormen's
    // "Introduction to Algorithms" (p. 527)
    // Note: I did some adaptions as our data structures are slightly
    // different from the ones used in the book. Further we simply stop
    // the algorithm is we don't find any node with a weight != Infinity
    // (==UINT_MAX), as this means that the remaining nodes in the queue
    // aren't connected anyway.
    void Graph::shortestPaths()
    {
        // Is the requested start mime type valid?
        Vertex* from = m_vertices[ m_from ];
        if ( !from )
            return;

        // Inititalize start vertex
        from->setKey( 0 );

        // Fill the priority queue with all the vertices
        PriorityQueue<Vertex> queue( m_vertices );

        while ( !queue.isEmpty() ) {
            Vertex *min = queue.extractMinimum();
            // Did we already relax all connected vertices?
            if ( min->key() == UINT_MAX )
                break;
            min->relaxVertices( queue );
        }
        m_graphValid = true;
    }

    QByteArray Graph::findKOfficePart() const
    {
        // Here we simply try to find the closest KOffice mimetype
        Q3ValueList<KoDocumentEntry> parts( KoDocumentEntry::query() );
        Q3ValueList<KoDocumentEntry>::ConstIterator partIt( parts.begin() );
        Q3ValueList<KoDocumentEntry>::ConstIterator partEnd( parts.end() );

        const Vertex *v = 0;

        // Be sure that v gets initialized correctly
        while ( !v && partIt != partEnd ) {
            QStringList nativeMimeTypes = ( *partIt ).service()->property( "X-KDE-ExtraNativeMimeTypes" ).toStringList();
            nativeMimeTypes += ( *partIt ).service()->property( "X-KDE-NativeMimeType" ).toString();
            QStringList::ConstIterator it = nativeMimeTypes.begin();
            QStringList::ConstIterator end = nativeMimeTypes.end();
            for ( ; !v && it != end; ++it )
                if ( !(*it).isEmpty() )
                    v = m_vertices[ ( *it ).toLatin1() ];
            ++partIt;
        }
        if ( !v )
            return "";

        // Now we try to find the "cheapest" KOffice vertex
        while ( partIt != partEnd ) {
            QStringList nativeMimeTypes = ( *partIt ).service()->property( "X-KDE-ExtraNativeMimeTypes" ).toStringList();
            nativeMimeTypes += ( *partIt ).service()->property( "X-KDE-NativeMimeType" ).toString();
            QStringList::ConstIterator it = nativeMimeTypes.begin();
            QStringList::ConstIterator end = nativeMimeTypes.end();
            for ( ; !v && it != end; ++it ) {
                QString key = *it;
                if ( !key.isEmpty() ) {
                    Vertex* tmp = m_vertices[ key.toLatin1() ];
                    if ( !v || ( tmp && tmp->key() < v->key() ) )
                        v = tmp;
                }
            }
            ++partIt;
        }

        // It seems it already is a KOffice part
        if ( v->key() == 0 )
            return "";

        return v->mimeType();
    }

} // namespace KOffice
