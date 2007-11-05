/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright     2007       David Faure <faure@kde.org>

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

#ifndef __ko_query_trader_h__
#define __ko_query_trader_h__

#include <kservice.h>
#include <ksharedptr.h>
#include <qlist.h>
#include <komain_export.h>

class QObject;
class QStringList;
class KoDocument;
class KoFilter;
class KoFilterChain;

/**
 *  Represents an available KOffice component
 *  that supports the document interface.
 */
class KOMAIN_EXPORT KoDocumentEntry
{

public:
    /**
     * Represents an invalid entry (as returned by queryByMimeType for instance)
     */
    explicit KoDocumentEntry();
    /**
     * Represents a valid entry
     */
    explicit KoDocumentEntry( const KService::Ptr& service );
    ~KoDocumentEntry();

  KService::Ptr service() const { return m_service; }

  /**
   * @return TRUE if the service pointer is null
   */
  bool isEmpty() const { return m_service.isNull(); }

  /**
   * @return name of the associated service
   */
  QString name() const { return m_service->name(); }

  /**
   *  Mimetypes (and other service types) which this document can handle.
   */
  QStringList mimeTypes() const { return m_service->serviceTypes(); }

  /**
   *  @return TRUE if the document can handle the requested mimetype.
   */
  bool supportsMimeType( const QString & _mimetype ) const
  { return mimeTypes().contains( _mimetype ); }

  /**
   *  Uses the factory of the component to create
   *  a document. If that is not possible, 0 is returned.
   */
  KoDocument* createDoc( QString* errorMsg = 0, KoDocument* parent = 0 ) const;

  enum QueryFlag {
    AllEntries = 0,
    /**
     * OnlyEmbeddableDocuments specifies if only KOffice Parts should be
     * listed which are embeddable into other koDocuments, or all (if false)
     * (eg.: it makes no sense to embed Kexi into KWord,
     *  but it makes sense to embed it into KoShell)
     */
    OnlyEmbeddableDocuments = 1
    // bitfield
  };
  Q_DECLARE_FLAGS(QueryFlags, QueryFlag)

  /**
   *  This function will query ksycoca to find all available components.
   *  The result will only contain parts, which are embeddable into a document
   *
   *  @param _constr is a constraint expression as used by KTrader.
   *                 You can use it to set additional restrictions on the available
   *                 components.
   */
  static QList<KoDocumentEntry> query( QueryFlags flags, const QString &  _constr = QString() );

  /**
   *  This is a convenience function.
   *
   *  @return a document entry for the KOffice component that supports
   *          the requested mimetype and fits the user best.
   */
  static KoDocumentEntry queryByMimeType( const QString & mimetype );

private:
  KService::Ptr m_service;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KoDocumentEntry::QueryFlags)

/**
 *  Represents an available filter.
 */
class KOMAIN_EXPORT KoFilterEntry : public KShared
{

public:
  typedef KSharedPtr<KoFilterEntry> Ptr;

  //KoFilterEntry() : weight( 0 ) { m_service = 0L; } // for QList
  explicit KoFilterEntry( const KService::Ptr& service );
  ~KoFilterEntry() { }

  KoFilter* createFilter( KoFilterChain* chain, QObject* parent = 0 );

  /**
   *  The imported mimetype(s).
   */
  QStringList import;

  /**
   *  The exported mimetype(s).
   */
  QStringList export_;

  /**
   *  The "weight" of this filter path. Has to be > 0 to be valid.
   */
  unsigned int weight;

  /**
   *  Do we have to check during runtime?
   */
  QString available;

  /**
   *  @return TRUE if the filter can import the requested mimetype.
   */
  bool imports( const QString& _mimetype ) const
  { return ( import.contains( _mimetype ) ); }

  /**
   *  @return TRUE if the filter can export the requested mimetype.
   */
  bool exports( const QString& _m ) const
  { return ( export_.contains( _m ) ); }

  /**
   *  This function will query KDED to find all available filters.
   *
   *  @param _constr is a constraint expression as used by KDEDs trader interface.
   *                 You can use it to set additional restrictions on the available
   *                 components.
   */
  static QList<KoFilterEntry::Ptr> query( const QString& _constr = QString() );

  KService::Ptr service() const { return m_service; }

private:
  KService::Ptr m_service;
};

#endif
