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

#ifndef __koffice_filter_chain_h__
#define __koffice_filter_chain_h__

#include <q3cstring.h>
#include <q3asciidict.h>
#include <q3ptrlist.h>
#include <QStringList>
//Added by qt3to4:
#include <Q3StrList>

#include <KoFilter.h>
#include <KoQueryTrader.h>
#include <KoStoreDevice.h>
#include <koffice_export.h>

class KTempFile;
class KoFilterManager;
class KoDocument;
class Q3StrList;

namespace KOffice {
    class Graph;
}

/**
 * @brief This class represents a chain of plain @ref KoFilter instances.
 *
 * Instances of this class are shared, so please just hold
 * KoFilterChain::Ptr pointers to it.
 *
 * @author Werner Trobin <trobin@kde.org>
 * @todo the class has no constructor and therefore cannot initialize its private class
 */
class KOFFICECORE_EXPORT KoFilterChain : public KShared
{
    // Only KOffice::Graph is allowed to construct instances and
    // add chain links.
    friend class KOffice::Graph;
    friend class KoFilterManager;

public:
    typedef KSharedPtr<KoFilterChain> Ptr;

    virtual ~KoFilterChain();

    /**
     * The filter manager returned may be 0!
     */
    const KoFilterManager* manager() const { return m_manager; }

    /**
     * Starts the filtering process.
     * @return The return status of the conversion. KoFilter::OK
     * if everything is alright.
     */
    KoFilter::ConversionStatus invokeChain();

    /**
     * Tells the @ref KoFilterManager the output file of the
     * filter chain in case of an import operation. If it's
     * QString::null we directly manipulated the document.
     */
    QString chainOutput() const;

    /**
     * Get the current file to read from. This part of the API
     * is for the filters in our chain.
     */
    QString inputFile();
    /**
     * Get the current file to write to. This part of the API
     * is for the filters in our chain.
     */
    QString outputFile();

    /**
     * Get a file from a storage. May return 0!
     * This part of the API is for the filters in our chain.
     * If you call it multiple times with the same stream name
     * the stream will be closed and re-opened.
     * Note: @em Don't delete that @ref KoStoreDevice we return.
     * @param name The name of the stream inside the storage
     * @param mode Whether we want to read or write from/to the stream
     * @return The storage device to access the stream. May be 0!
     */
    KoStoreDevice* storageFile( const QString& name = "root", KoStore::Mode mode = KoStore::Read );

    /**
     * This method allows your filter to work directly on the
     * @ref KoDocument of the application.
     * This part of the API is for the filters in our chain.
     * @return The document containing the data. May return 0 on error.
     */
    KoDocument* inputDocument();
    /**
     * This method allows your filter to work directly on the
     * @ref KoDocument of the application.
     * This part of the API is for the filters in our chain.
     * @return The document you have to write to. May return 0 on error.
     */
    KoDocument* outputDocument();


    // debugging
    void dump() const;

private:
    // ### API for KOffice::Graph:
    // Construct a filter chain belonging to some KoFilterManager.
    // The parent filter manager may be 0.
    KoFilterChain( const KoFilterManager* manager );

    void appendChainLink( KoFilterEntry::Ptr filterEntry, const QByteArray& from, const QByteArray& to );
    void prependChainLink( KoFilterEntry::Ptr filterEntry, const QByteArray& from, const QByteArray& to );

    // ### API for KoEmbeddingFilter
    // This is needed as the embedding filter might have to influence
    // the way we change directories (e.g. in the olefilter case)
    // The ugly friend methods are needed, but I'd welcome and suggestions for
    // better design :}
    friend void KoEmbeddingFilter::filterChainEnterDirectory( const QString& directory ) const;
    void enterDirectory( const QString& directory );
    friend void KoEmbeddingFilter::filterChainLeaveDirectory() const;
    void leaveDirectory();

    // These methods are friends of KoFilterManager and provide access
    // to a private part of its API. As I don't want to include
    // koFilterManager.h in this header the direction is "int" here.
    QString filterManagerImportFile() const;
    QString filterManagerExportFile() const;
    KoDocument* filterManagerKoDocument() const;
    int filterManagerDirection() const;
    KoFilterChain* const filterManagerParentChain() const;


    // Helper methods which keep track of all the temp files, documents,
    // storages,... and properly delete them as soon as they are not
    // needed anymore.
    void manageIO();
    void finalizeIO();

    bool createTempFile( KTempFile** tempFile, bool autoDelete = true );

    void inputFileHelper( KoDocument* document, const QString& alternativeFile );
    void outputFileHelper( bool autoDelete );
    KoStoreDevice* storageNewStreamHelper( KoStore** storage, KoStoreDevice** device, const QString& name );
    KoStoreDevice* storageHelper( const QString& file, const QString& streamName,
                                  KoStore::Mode mode, KoStore** storage, KoStoreDevice** device );
    void storageInit( const QString& file, KoStore::Mode mode, KoStore** storage );
    KoStoreDevice* storageInitEmbedding( const QString& name );
    KoStoreDevice* storageCreateFirstStream( const QString& streamName, KoStore** storage, KoStoreDevice** device );
    KoStoreDevice* storageCleanupHelper( KoStore** storage );

    KoDocument* createDocument( const QString& file );
    KoDocument* createDocument( const QByteArray& mimeType );

    /**
     * A small private helper class with represents one single filter
     * (one link of the chain)
     * @internal
     */
    class ChainLink
    {

    public:
        ChainLink( KoFilterChain* chain, KoFilterEntry::Ptr filterEntry,
                   const QByteArray& from, const QByteArray& to );

        KoFilter::ConversionStatus invokeFilter( const ChainLink* const parentChainLink );

        QByteArray from() const { return m_from; }
        QByteArray to() const { return m_to; }

        // debugging
        void dump() const;

        // This hack is only needed due to crappy Microsoft design and
        // circular dependencies in their embedded files :}
        int lruPartIndex() const;

    private:
        ChainLink( const ChainLink& rhs );
        ChainLink& operator=( const ChainLink& rhs );

        void setupCommunication( const KoFilter* const parentFilter ) const;
        void setupConnections( const KoFilter* sender, const KoFilter* receiver ) const;

        KoFilterChain* m_chain;
        KoFilterEntry::Ptr m_filterEntry;
        QByteArray m_from, m_to;

        // This hack is only needed due to crappy Microsoft design and
        // circular dependencies in their embedded files :}
        KoFilter* m_filter;

        class Private;
        Private* d;
    };

    // "A whole is that which has beginning, middle, and end" - Aristotle
    // ...but we also need to signal "Done" state, Mr. Aristotle
    enum Whole { Beginning = 1, Middle = 2, End = 4, Done = 8 };

    // Don't copy or assign filter chains
    KoFilterChain( const KoFilterChain& rhs );
    KoFilterChain& operator=( const KoFilterChain& rhs );

    const KoFilterManager* const m_manager;
    Q3PtrList<ChainLink> m_chainLinks;

    // stuff needed for bookkeeping
    int m_state;

    QString m_inputFile;              // Did we pass around plain files?
    QString m_outputFile;

    KoStore* m_inputStorage;          // ...or was it a storage+device?
    KoStoreDevice* m_inputStorageDevice;
    KoStore* m_outputStorage;
    KoStoreDevice* m_outputStorageDevice;

    KoDocument* m_inputDocument;      // ...or even documents?
    KoDocument* m_outputDocument;

    KTempFile* m_inputTempFile;
    KTempFile* m_outputTempFile;

    // These two flags keep track of the input/output the
    // filter (=user) asked for
    enum IOState { Nil, File, Storage, Document };
    IOState m_inputQueried, m_outputQueried;

    // This stack keeps track of directories we have to enter and
    // leave due to internal embedding a la OLE filters. This serves
    // as a kind of "memory" even if we didn't initialize the store yet.
    // I know that it's ugly, and I'll try to clean up that hack
    // sooner or later (Werner)
    QStringList m_internalEmbeddingDirectories;

    class Private;
    Private* d;
};


// As we use quite generic classnames...
namespace KOffice
{
    class Vertex;
    template<class T> class PriorityQueue;

    /**
     * An internal class representing a filter (=edge) in the filter graph.
     * @internal
     */
    class Edge
    {

    public:
        // creates a new edge to "vertex" with the given weight.
        Edge( Vertex* vertex, KoFilterEntry::Ptr filterEntry );
        ~Edge() {}

        unsigned int weight() const { return m_filterEntry ? m_filterEntry->weight : 0; }
        KoFilterEntry::Ptr filterEntry() const { return m_filterEntry; }
        const Vertex* vertex() const { return m_vertex; }

        // Relaxes the "connected" vertex (i.e. the weight of the
        // connected vertex = "predec.->key()" (parameter) + weight of this edge
        // As this will only be called once we calculate the weight
        // of the edge "on the fly"
        // Note: We have to pass the queue as we have to call keyDecreased :}
        void relax( const Vertex* predecessor, PriorityQueue<Vertex>& queue );

        // debugging
        void dump( const QByteArray& indent ) const;

    private:
        Edge( const Edge& rhs );
        Edge& operator=( const Edge& rhs );

        Vertex* m_vertex;
        KoFilterEntry::Ptr m_filterEntry;

        class Private;
        Private* d;
    };


    /**
     * An internal class representing a mime type (=node, vertex) in the filter graph.
     * @internal
     */
    class Vertex
    {

    public:
        Vertex( const QByteArray& mimeType );
        ~Vertex() {}

        QByteArray mimeType() const { return m_mimeType; }

        // Current "weight" of the vertex - will be "relaxed" when
        // running the shortest path algorithm. Returns true if it
        // really has been "relaxed"
        bool setKey( unsigned int key );
        unsigned int key() const { return m_weight; }
        // Can be used to set the key back to "Infinity" (UINT_MAX)
        // and reset the predecessor of this vertex
        void reset();

        // Position in the heap, needed for a fast keyDecreased operation
        void setIndex( int index ) { m_index=index; }
        int index() const { return m_index; }

        // predecessor on the way from the source to the destination,
        // needed for the shortest path algorithm
        void setPredecessor( const Vertex* predecessor ) { m_predecessor=predecessor; }
        const Vertex* predecessor() const { return m_predecessor; }

        // Adds an outgoing edge to the vertex, transfers ownership
        void addEdge( const Edge* edge );
        // Finds the lightest(!) edge pointing to the given vertex, if any (0 if not found)
        // This means it will always search the whole list of edges
        const Edge* findEdge( const Vertex* vertex ) const;

        // This method is called when we need to relax all "our" edges.
        // We need to pass the queue as we have to notify it about key changes - ugly :(
        void relaxVertices( PriorityQueue<Vertex>& queue );

        // debugging
        void dump( const QByteArray& indent ) const;

    private:
        Vertex( const Vertex& rhs );
        Vertex& operator=( const Vertex& rhs );

        Q3PtrList<Edge> m_edges;
        const Vertex* m_predecessor;
        QByteArray m_mimeType;
        unsigned int m_weight; // "key" inside the queue
        int m_index; // position inside the queue, needed for a fast keyDecreased()

        class Private;
        Private* d;
    };


    /**
     * The main worker behind the scenes. Manages the creation of the graph,
     * processing the information in it, and creating the filter chains.
     * @internal
     * Only exported for unit tests.
     */
    class KOFFICECORE_EXPORT Graph
    {

    public:
        Graph( const QByteArray& from );
        ~Graph() {}

        bool isValid() const { return m_graphValid; }

        QByteArray sourceMimeType() const { return m_from; }
        void setSourceMimeType( const QByteArray& from );

        // Creates a chain from "from" to the "to" mimetype
        // If the "to" mimetype isEmpty() then we try to find the
        // closest KOffice mimetype and use that as destination.
        // After such a search "to" will contain the dest. mimetype (return value)
        // if the search was successful. Might return 0!
        KoFilterChain::Ptr chain( const KoFilterManager* manager, QByteArray& to ) const;

        // debugging
        void dump() const;

    private:
        Graph( const Graph& rhs );
        Graph& operator=( const Graph& rhs );

        void buildGraph();
        void shortestPaths();
        QByteArray findKOfficePart() const;

        Q3AsciiDict<Vertex> m_vertices;
        QByteArray m_from;
        bool m_graphValid;

        class Private;
        Private* d;
    };

} // namespace KOffice

#endif // __koffice_filter_chain_h__
