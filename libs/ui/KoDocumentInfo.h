/* This file is part of the KDE project
   SPDX-FileCopyrightText: 1998, 1999, 2000 Torben Weis <weis@kde.org>
   SPDX-FileCopyrightText: 2004 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2006 Martin Pfeiffer <hubipete@gmx.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KO_DOCUMENT_INFO_H
#define KO_DOCUMENT_INFO_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>

#include "kritaui_export.h"
#include <QDomDocument>

class QDomDocument;
class QDomElement;
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
 * stored in two QMap and can be accessed through aboutInfo() and authorInfo().
 * The about info can be changed with setAboutInfo() and setAuthorInfo()
 */
class KRITAUI_EXPORT KoDocumentInfo : public QObject
{
    Q_OBJECT

public:
    /**
     * The constructor
     * @param parent a pointer to the parent object
     */
    explicit KoDocumentInfo(QObject *parent = 0);
    explicit KoDocumentInfo(const KoDocumentInfo &rhs, QObject *parent = 0);

    /** The destructor */
    ~KoDocumentInfo() override;
    /**
     * Load the KoDocumentInfo from an Calligra-1.3 DomDocument
     * @param doc the QDomDocument to load from
     * @return true if success
     */
    bool load(const QDomDocument& doc);

    /**
     * Save the KoDocumentInfo to an Calligra-1.3 DomDocument
     * @return the QDomDocument to which was saved
     */
    QDomDocument save(QDomDocument &doc);

    /**
     * Set information about the author.
     * This will override any information retrieved from the author profile
     * But it does not change the author profile
     * Note: authorInfo() will not return the new value until the document has been
     * saved by the user.(autosave doesn't count)
     * @param info the kind of information to set
     * @param data the data to set for this information
     */
    void setAuthorInfo(const QString& info, const QString& data);

    /**
     * Obtain information about the author
     * @param info the kind of information to obtain
     * @return a QString with the information
     */
    QString authorInfo(const QString& info) const;

    /**
     * @brief authorContactInfo
     * @return returns list of contact info for author.
     */
    QStringList authorContactInfo() const;

    /**
     * Set information about the document
     * @param info the kind of information to set
     * @param data the data to set for this information
     */
    void setAboutInfo(const QString& info, const QString& data);

    /**
     * Obtain information about the document
     * @param info the kind of information to obtain
     * @return a QString with the information
     */
    QString aboutInfo(const QString& info) const;

    /**
     * Obtain the generator of the document, as it was loaded from the document
     */
    QString originalGenerator() const;

    /**
     * Sets the original generator of the document. This does not affect what gets
     * saved to a document in the meta:generator field, it only changes what
     * originalGenerator() will return.
     */
    void setOriginalGenerator(const QString& generator);

    /** Resets part of the meta data */
    void resetMetaData();

    /** Takes care of updating the document info from configuration correctly */
    void updateParameters();

private:
    /// Bumps the editing cycles count and save date, and then calls updateParameters
    void updateParametersAndBumpNumCycles();

    /**
     * Set information about the author
     * This sets what is actually saved to file. The public method setAuthorInfo() can be used to set
     * values that override what is fetched from the author profile. During saveParameters() author
     * profile and any overrides is combined resulting in calls to this method.
     * @param info the kind of information to set
     * @param data the data to set for this information
     */
    void setActiveAuthorInfo(const QString& info, const QString& data);

    /**
     * Load the information about the document from an OASIS file
     * @param metaDoc a reference to the information node
     * @return true if success
     */
    bool loadOasisAboutInfo(const QDomNode& metaDoc);

    /**
     * Save the information about the document to an OASIS file
     * @param xmlWriter a reference to the KoXmlWriter to write in
     * @return true if success
     */
    bool saveOasisAboutInfo(KoXmlWriter &xmlWriter);

    /**
     * Load the information about the document from a Calligra-1.3 file
     * @param e the element to load from
     * @return true if success
     */
    bool loadAboutInfo(const QDomElement& e);

    /**
     * Save the information about the document to a Calligra-1.3 file
     * @param doc the QDomDocument to save in
     * @return the QDomElement to which was saved
     */
    QDomElement saveAboutInfo(QDomDocument& doc);

    /**
     * Load the information about the document from an OASIS file
     * @param metaDoc a reference to the information node
     * @return true if success
     */
    bool loadOasisAuthorInfo(const QDomNode& metaDoc);

    /**
     * Load the information about the document from a Calligra-1.3 file
     * @param e the element to load from
     * @return true if success
     */
    bool loadAuthorInfo(const QDomElement& e);

    /**
     * Save the information about the author to a Calligra-1.3 file
     * @param doc the QDomDocument to save in
     * @return the QDomElement to which was saved
     */
    QDomElement saveAuthorInfo(QDomDocument& doc);

    /**
     * Save the information about the document to an OASIS file
     * @param xmlWriter a reference to the KoXmlWriter to write in
     * @return true if success
     */
    bool saveOasisAuthorInfo(KoXmlWriter &xmlWriter);

    /** A QStringList containing all tags for the document information */
    QStringList m_aboutTags;
    /** A QStringList containing all tags for the author information */
    QStringList m_authorTags;
    /** A QStringList containing all valid contact tags */
    QStringList m_contactTags;
    /** A QMap with the contact modes and their type in the second string */
    QMap <QString, QString> m_contact;
    /** The map containing information about the author */
    QMap<QString, QString> m_authorInfo;
    /** The map containing information about the author set programmatically*/
    QMap<QString, QString> m_authorInfoOverride;
    /** The map containing information about the document */
    QMap<QString, QString> m_aboutInfo;
    /** The original meta:generator of the document */
    QString m_generator;

Q_SIGNALS:
    void infoUpdated(const QString &info, const QString &data);
};

#endif
