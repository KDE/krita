/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>
   Copyright (C) 2004 David Faure <faure@kde.org>
   Copyright (C) 2006 Martin Pfeiffer <hubipete@gmx.net>

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

#ifndef KO_DOCUMENT_INFO_H
#define KO_DOCUMENT_INFO_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>

#include <kconfig.h>
#include <koffice_export.h>

class QDomDocument;
class QDomElement;
class QDomNode;
class KoStore;
class KoXmlWriter;

/**
 * @short The class containing all meta information about a document
 *
 * @author Torben Weis <weis@kde.org>
 * @author David Faure <faure@kde.org>
 * @author Martin Pfeiffer <hubipete@gmx.net>
 * @see KoDocumentInfoDlg
 *
 * This class contains the meta information for a document. They are
 * stored in two QMap and can be accessed through setAuthorInfo(),
 * setAboutInfo(), aboutInfo() and authorInfo().
 */
class KOFFICECORE_EXPORT KoDocumentInfo : public QObject
{
  Q_OBJECT

  public:
    /**
     * The constructor
     * @param parent a pointer to the parent object
     */
    KoDocumentInfo( QObject* parent = 0 );

    /** The destructor */
    ~KoDocumentInfo();

    /**
     * Load the KoDocumentInfo from an OASIS document
     * @param metaDoc the QDomDocument with the metaInformation
     * @return true if success
     */
    bool loadOasis( const QDomDocument& metaDoc );

    /**
     * Save the KoDocumentInfo to an OASIS document
     * @param store a pointer to a KoStore to save in
     * @return true if success
     */
    bool saveOasis( KoStore* store );

    /**
     * Load the KoDocumentInfo from an KOffice-1.3 DomDocument
     * @param doc the QDomDocument to load from
     * @return true if success
     */
    bool load( const QDomDocument& doc );

    /**
     * Save the KoDocumentInfo to an KOffice-1.3 DomDocument
     * @return the QDomDocument to which was saved
     */
    QDomDocument save();

    /**
     * Set information about the author
     * @param info the kind of information to set
     * @param data the data to set for this information
     */
    void setAuthorInfo( const QString& info, const QString& data );

    /**
     * Obtain information about the author
     * @param info the kind of information to obtain
     * @return a QString with the information
     */
    QString authorInfo( const QString& info ) const;

    /**
     * Set information about the document
     * @param info the kind of information to set
     * @param data the data to set for this information
     */
    void setAboutInfo( const QString& info, const QString& data );

    /**
     * Obtain information about the document
     * @param info the kind of information to obtain
     * @return a QString with the information
     */
    QString aboutInfo( const QString& info ) const;

    /** Resets part of the meta data */
    void resetMetaData();

  private:
    /**
     * Load the information about the document from an OASIS file
     * @param metaDoc a reference to the information node
     * @return true if success
     */
    bool loadOasisAboutInfo( const QDomNode& metaDoc );

    /**
     * Save the information about the document to an OASIS file
     * @param xmlWriter a reference to the KoXmlWriter to write in
     * @return true if success
     */
    bool saveOasisAboutInfo( KoXmlWriter &xmlWriter );

    /**
     * Load the information about the document from a KOffice-1.3 file
     * @param e the element to load from
     * @return true if success
     */
    bool loadAboutInfo( const QDomElement& e );

    /**
     * Save the information about the document to a KOffice-1.3 file
     * @param doc the QDomDocument to save in
     * @return the QDomElement to which was saved
     */
    QDomElement saveAboutInfo( QDomDocument& doc );

    /**
     * Load the information about the document from an OASIS file
     * @param metaDoc a reference to the information node
     * @return true if success
     */
    bool loadOasisAuthorInfo( const QDomNode& metaDoc );

    /**
     * Load the information about the document from a KOffice-1.3 file
     * @param e the element to load from
     * @return true if success
     */
    bool loadAuthorInfo( const QDomElement& e );

    /**
     * Save the information about the author to a KOffice-1.3 file
     * @param doc the QDomDocument to save in
     * @return the QDomElement to which was saved
     */
    QDomElement saveAuthorInfo( QDomDocument& doc );

    /**
     * Save the information about the document to an OASIS file
     * @param xmlWriter a reference to the KoXmlWriter to write in
     * @return true if success
     */
    bool saveOasisAuthorInfo( KoXmlWriter &xmlWriter );

    /** Takes care of saving the per-editing-cycle data correctly */
    void saveParameters();

    /** A QStringList containing all tags for the document information */
    QStringList m_aboutTags;
    /** A QStringList containing all tags for the author information */
    QStringList m_authorTags;
    /** The map containing information about the author */
    QMap<QString, QString> m_authorInfo;
    /** The map containing information about the document */
    QMap<QString, QString> m_aboutInfo;
};

#endif
