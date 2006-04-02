/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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
#ifndef __koDocumentChild_h__
#define __koDocumentChild_h__

#include <KoChild.h>
#include <koffice_export.h>
class KoView;
class QDomDocument;
class QDomElement;
class KUrl;
class KoStore;
class KoDocument;
class KoDocumentChildPrivate;
class KoDocumentEntry;
class KoXmlWriter;

/**
 * @brief Holds an embedded object.
 *
 * In addition to its base class @ref KoChild it cares about the content
 * of an embedded document. That means it supports operations like
 * loading and saving.
 *
 * If you need a representation for embedded documents in your KOffice
 * component then you should inherit from this class.
 */
class KOFFICECORE_EXPORT KoDocumentChild : public KoChild
{
  Q_OBJECT
public:
  KoDocumentChild( KoDocument* parent, KoDocument* doc, const QRect& geometry );

  /**
   * When using this constructor you must call @ref #setDocument before
   * you can call any other function of this class.
   */
  KoDocumentChild( KoDocument* parent );

  virtual ~KoDocumentChild();

  /**
   * Call this function only directly after calling the constructor
   * that takes only a parent as argument.
   */
  virtual void setDocument( KoDocument *doc, const QRect &geometry );

  /**
   * @return document contained in this child
   *
   * @see KoDocument
   */
  KoDocument *document() const;

  /**
   * @return parent document of this child
   *
   * @see KoDocument
   */
  KoDocument *parentDocument() const;

  virtual KoDocument* hitTest( const QPoint& p, KoView* view, const QMatrix& _matrix = QMatrix() );

  /**
   * @note Can be empty (which is why it doesn't return a const KUrl &)
   */
  KUrl url() const;

  /**
   *  Writes the "object" tag, but does NOT write the content of the
   *  embedded documents. Saving the embedded documents themselves
   *  is done in @ref KoDocument::saveChildren. This function just stores information
   *  about the position and id of the embedded document and should be
   *  called from within KoDocument::saveXML.
   *
   *  The "object" element is not added to the document. It is just created
   *  and returned.
   *
   *  @return the element containing the "object" tag.
   */
  virtual QDomElement save( QDomDocument& doc, bool uppercase=false );

  /**
   * Save an embedded object to OASIS.
   * This method sets the attributes for the draw:object element in the parent XML document.
   * It also prepares the embedded object for being saved into the store at
   * the end of saving (see saveOasisToStore).
   * Note that @p name is only used for "internal" documents (not extern).
   */
  void saveOasisAttributes( KoXmlWriter &xmlWriter, const QString& name );

  /**
   * Save an embedded object to an OASIS store.
   * This is called automatically by the parent KoDocument's saveOasis
   */
  virtual bool saveOasis( KoStore* store, KoXmlWriter* manifestWriter );

  /**
   *  Parses the "object" tag. This does NOT mean creating the child documents.
   *  AFTER the 'parser' has finished parsing, you must use @ref #loadDocument
   *  to actually load the embedded documents.
   *
   *  What you should have in mind is that this method is called from within
   *  @ref KoDocument::loadXML while @ref #loadDocument is called from within
   *  @ref KoDocument::loadChildren, respectively from your implementation
   *  of these methods.
   */
  virtual bool load( const QDomElement& element, bool uppercase=false );

  void loadOasis( const QDomElement &frameElement, const QDomElement& objectElement );

  /**
   *  Actually loads the document from the disk/net or from the store,
   *  depending on @ref #url
   */
  virtual bool loadDocument( KoStore* );

  /**
   *  Actually loads the document from the disk/net or from the store
   *  depending on the document's url
   */
  virtual bool loadOasisDocument( KoStore* store, const QDomDocument& manifestDoc );

  virtual bool isStoredExtern() const;

  /**
   * This document (child) is deleted.
   */
  bool isDeleted() const;
  void setDeleted( bool on = true );

protected: // Should be private, but KWord needs access to the variables
    // because it reimplements load/save (for uppercase tags)

  /**
   *  Holds the source of this object, for example "file:/home/weis/image.gif"
   *  or "tar:/table1/2" if it is stored in a koffice store. This variable is
   *  set after parsing the OBJECT tag in @ref #load and is reset after
   *  calling @ref #loadDocument.
   */
  QString m_tmpURL;

  /**
   * This variable is
   *  set after parsing the OBJECT tag in @ref #load and is reset after
   *  calling @ref #loadDocument.
   */
  QRect m_tmpGeometry;

  /**
   * This variable is
   *  set after parsing the OBJECT tag in @ref #load and is reset after
   *  calling @ref #loadDocument.
   */
  QString m_tmpMimeType;

private:
  bool createUnavailDocument( KoStore* store, bool doOpenURL, const QString& mimeType );
  bool createAndLoadDocument( KoStore* store, bool doOpenURL, bool oasis, const QString& mimeType );
  bool finishLoadingDocument( KoStore* store, KoDocument* doc, bool doOpenURL, bool oasis );

private:
  KoDocumentChildPrivate *d;
};

#endif
