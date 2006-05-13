/* This file is part of the KOffice libraries
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

#ifndef __koffice_filter_h__
#define __koffice_filter_h__

#include <QObject>
#include <QMap>
#include <q3ptrstack.h>
#include <koffice_export.h>
class QIODevice;
class KoFilterChain;

/**
 * @brief The base class for import and export filters.
 *
 * Derive your filter class from this base class and implement
 * the @ref convert() method. Don't forget to specify the Q_OBJECT
 * macro in your class even if you don't use signals or slots.
 * This is needed as filters are created on the fly.
 * The m_chain member allows access to the @ref KoFilterChain
 * which invokes the filter to query for input/output.
 *
 * @note Take care: The m_chain pointer is invalid while the constructor
 * runs due to the implementation -- @em don't use it in the constructor.
 * After the constructor, when running the @ref convert() method it's
 * guaranteed to be valid, so no need to check against 0.
 *
 * @author Werner Trobin <trobin@kde.org>
 * @todo the class has no constructor and therefore cannot initialize its private class
 */
class KOFFICECORE_EXPORT KoFilter : public QObject
{
    Q_OBJECT

    friend class KoFilterEntry;  // needed for the filter chain pointer :(
    friend class KoFilterChain;

public:
    /**
     * This enum is used to signal the return state of your filter.
     * Return OK in @ref convert() in case everything worked as expected.
     * Feel free to add some more error conditions @em before the last item
     * if it's needed.
     */
    enum ConversionStatus { OK, StupidError, UsageError, CreationError, FileNotFound,
                            StorageCreationError, BadMimeType, BadConversionGraph,
                            EmbeddedDocError, WrongFormat, NotImplemented,
                            ParsingError, InternalError, UnexpectedEOF,
                            UnexpectedOpcode, UserCancelled, OutOfMemory,
                            PasswordProtected,
                            JustInCaseSomeBrokenCompilerUsesLessThanAByte = 255 };

    virtual ~KoFilter();

    /**
     * The filter chain calls this method to perform the actual conversion.
     * The passed mimetypes should be a pair of those you specified in your
     * .desktop file.
     * You @em have to implement this method to make the filter work.
     *
     * @param from The mimetype of the source file/document
     * @param to The mimetype of the destination file/document
     * @return The error status, see the @ref #ConversionStatus enum.
     *         KoFilter::OK means that everything is alright.
     */
    virtual ConversionStatus convert( const QByteArray& from, const QByteArray& to ) = 0;

signals:
    /**
     * Emit this signal with a value in the range of 1...100 to have some
     * progress feedback for the user in the statusbar of the application.
     *
     * @param value The actual progress state. Should always remain in
     * the range 1..100.
     */
    void sigProgress( int value );

protected:
    /**
     * This is the constructor your filter has to call, obviously.
     */
    KoFilter( QObject* parent = 0 );

    /**
     * Use this pointer to access all information about input/output
     * during the conversion. @em Don't use it in the constructor -
     * it's invalid while constructing the object!
     */
    KoFilterChain* m_chain;

private:
    KoFilter( const KoFilter& rhs );
    KoFilter& operator=( const KoFilter& rhs );

    class Private;
    Private* d;
};


/**
 * The base class for all @em import filters embedding other filters. Right
 * now we don't support embedding for export filters, but if there's a
 * request for that feature please don't hesitate to contact Werner Trobin
 * <trobin@kde.org>.
 *
 * To make use of embedding features you have to know that there are two kinds
 * of embedding for filters: embedding the output of a different filter (library)
 * or embedding the output of several internal filters (no separate library).
 * The first case is the simpler one. You just have to override savePartContents()
 * and call @ref #embedPart to trigger the embedding process. One example for such
 * a filter is Kontour's MSOD (MS Office Drawing) filter.
 *
 * The more complex case is embedding various streams from within the same filter
 * library. This is neccesary for OLE like files (at least with the current design
 * of the OLEFilter). In this case you have to use @ref #startInternalEmbedding and
 * @ref #endInternalEmbedding accordingly. Try to use the previous method if possible.
 *
 * If you're using this class you can also setup a signal/slot communication
 * between parent and child filter. To make that work you simply have to define
 * signals and slots along the following rules:
 * Signals should be named "commSignal\<name\>" where \<name\> is the name of the signal,
 * slots should be named "commSlot\<name\>". The connection will be done automatically
 * if names and signatures are matching.
 *
 * @author Werner Trobin
 * @todo the class has no constructor and therefore cannot initialize its private class
 */
class KOFFICECORE_EXPORT KoEmbeddingFilter : public KoFilter
{
    Q_OBJECT

    friend class KoFilterChain;

public:
    virtual ~KoEmbeddingFilter();

    /**
     * @internal
     * This method returns the last recently used part index at the
     * current directory level. It can be (and is ;) used to generate
     * the per-directory-unique address for the next part we have to save.
     * It will get updated automatically, you most likely don't have to
     * care about that one at all.
     */
    int lruPartIndex() const;

    /**
     * A static helper method to determine the mimetype via the
     * file extension. It allows to go from "wmf" to image/x-wmf
     * and so on. Note that you should only pass the pure extension
     * and not a whole pattern like "*.doc" or so.
     */
    static QString mimeTypeByExtension( const QString& extension );

protected:
    /**
     * Constructs a filter. Note that the m_chain pointer is 0 inside
     * the constructor. Most likely your constructor will be empty.
     */
    KoEmbeddingFilter();

    /**
     * Embed some document using an external filter (i.e. a different
     * filter library). This method works according to the template method
     * pattern and calls savePartContents() during execution.
     * Call this method when you want to convert some data using one or more
     * KOffice filters selected via the filter manager.
     * This is the way to go when it comes to embedding unless you have very
     * special requirements.
     *
     * @param from The mimetype of the source data
     * @param to The mimetype of the destination part. If this field is set
     *           to "" the filter manager will try to find the best native
     *           KOffice mimetype. When the method returns this parameter will
     *           hold the string of the used mimetype.
     * @param status Returns the error status of the filter
     * @param key Optional key field to allow custom keys inside the part
     *        map (see @ref #internalPartReference). If this field is left
     *        empty we generate a key from the part number (e.g. 1 -> "1")
     * @return The number of the part (can be used to refer to the part from
     *         within the embedding filter).
     */
    int embedPart( const QByteArray& from, QByteArray& to,
                   KoFilter::ConversionStatus& status,
                   const QString& key = QString::null );

    /**
     * Method to perform "internal" embedding of parts in olefilter-style.
     * This method can be used to signal the start of a new embedding
     * level within your filter. Very evil, but what shall I say ;)
     * Unless you really have to you should always use @ref #embedPart as
     * it's easier to use and not as hacky.
     *
     * @param key The key we use to store reference/mimetype of your new part
     * @param mimeType The mimetype of the part you're about to embed
     */
    void startInternalEmbedding( const QString& key, const QByteArray& mimeType );

    /**
     * This method signals the end of an internal embedding session.
     * You have to call that exactly as often as you call @ref #startInternalEmbedding
     * or you'll mess up the internal stack and your file will be invalid.
     * Again: use @ref #embedPart if you can :-)
     */
    void endInternalEmbedding();

    /**
     * Query the internal part map for the reference of the part
     * matching the given key. Note that you can use that plain
     * simple int to refer to the respective part (when used as string).
     *
     * @param key The key you would like to look up
     * @return The reference or -1 if we didn't find a part with the
     *         given key
     */
    int internalPartReference( const QString& key ) const;

    /**
     * Query the internal part map for the mimetype of the part
     * matching the given key.
     *
     * @param key The key you would like to look up
     * @return The mimetype, might be empty if the part matching
     *         the given key doesn't exist.
     */
    QByteArray internalPartMimeType( const QString& key ) const;

private:
    /**
     * Holds the directory's number and the mimetype of the part
     * for internal parts. This is all we need to locate a part.
     * @internal
     */
    struct PartReference
    {
        PartReference( int index = -1, const QByteArray& mimeType = "" );
        bool isValid() const;

        int m_index;
        QByteArray m_mimeType;
    };

    /**
     * This struct keeps track of the last used index for a
     * child part and all references to existing children
     * We use it to build a whole stack, one PartState per
     * embedding level.
     * @internal
     */
    struct PartState
    {
        PartState();

        int m_lruPartIndex;
        QMap<QString, PartReference> m_partReferences;
    };

    /// Better do not copy the filters
    KoEmbeddingFilter( const KoEmbeddingFilter& rhs );
    /// Better do not assign the filters
    KoEmbeddingFilter& operator=( const KoEmbeddingFilter& rhs );

    /**
     * This method will be called by @ref #embedPart as soon as it
     * needs the data of the part (template method pattern). You
     * have to override that and simply save the part data to the
     * (already opened) file.
     * No need to override that when you're not using @ref #embedPart
     * (as you should ;)
     *
     * @param file An already opened file
     */
    virtual void savePartContents( QIODevice* file );

    /**
     * Internal methods to support the start/endInternalEmbedding
     * methods (we have to change directories and stuff).
     * These methods are declared friends of the KoFilterChain
     */
    void filterChainEnterDirectory( const QString& directory ) const;
    /**
     * Internal methods to support the start/endInternalEmbedding
     * methods (we have to change directories and stuff).
     * These methods are declared friends of the KoFilterChain
     */
    void filterChainLeaveDirectory() const;

    /**
     * A stack which keeps track of the current part references.
     * We push one PartState structure for every embedding level.
     */
    Q3PtrStack<PartState> m_partStack;

    class Private;
    Private* d;
};

#endif
