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
Boston, MA 02110-1301, USA.
*/

#ifndef KIS_IMPORT_EXPORT_MANAGER_H
#define KIS_IMPORT_EXPORT_MANAGER_H

#include <QObject>
#include <QMap>
#include <QByteArray>
#include <QUrl>

#include "KisFilterGraph.h"

#include "kritaui_export.h"
class KisFilterChain;
class KisDocument;
class KoProgressUpdater;

/**
 *  @brief The class managing all the filters.
 *
 *  This class manages all filters for a %Calligra application. Normally
 *  you will not have to use it, since KisMainWindow takes care of loading
 *  and saving documents.
 *
 *  @ref KisFilter
 *
 *  @author Kalle Dalheimer <kalle@kde.org>
 *  @author Torben Weis <weis@kde.org>
 *  @author Werner Trobin <trobin@kde.org>
 */
class KRITAUI_EXPORT KisImportExportManager : public QObject
{
    Q_OBJECT
public:
    /**
     * This enum is used to distinguish the import/export cases
     */
    enum Direction { Import = 1,  Export = 2 };

    /**
     * Create a filter manager for a document
     */
    explicit KisImportExportManager(KisDocument *document);

    /**
     * Create a filter manager for the Shape Collection docker.
     * @param mimeType the mimetype to import to.
     */
    explicit KisImportExportManager(const QByteArray& mimeType);

    /**
     * Create a filter manager for a filter which wants to embed something.
     * The path it passes is the file to convert. You cannot use
     * the @ref importDocument() method -- use @ref exportDocument() to convert
     * the file to the destination mimetype you prefer.
     *
     * @param path The location you want to export
     */
    explicit KisImportExportManager(const QString& location);

    virtual ~KisImportExportManager();

    /**
     * Imports the specified document and returns the resultant filename
     * (most likely some file in /tmp).
     * @p path can be either a URL or a filename.
     * @p documentMimeType gives importDocument a hint about what type
     * the document may be. It can be left empty.
     * @p status signals the success/error of the conversion.
     * If the QString which is returned isEmpty() and the status is OK,
     * then we imported the file directly into the document.
     */
    QString importDocument(const QString& location,
                           const QString& documentMimeType,
                           KisImportExportFilter::ConversionStatus& status);

    /**
     * @brief Exports the given file/document to the specified URL/mimetype.
     *
     * If @p mimeType is empty, then the closest matching Calligra part is searched
     * and when the method returns @p mimeType contains this mimetype.
     * Oh, well, export is a C++ keyword ;)
     */
    KisImportExportFilter::ConversionStatus exportDocument(const QString& location, QByteArray& mimeType);

    ///@name Static API
    //@{
    /**
     * Suitable for passing to KoFileDialog::setMimeTypeFilters. The default mime
     * gets set by the "users" of this method, as we do not have enough
     * information here.
     * Optionally, @p extraNativeMimeTypes are added after the native mimetype.
     */
    static QStringList mimeFilter(Direction direction);


    /**
     * Set the filter manager is batch mode (no dialog shown)
     * instead of the interactive mode (dialog shown)
     */
    void setBatchMode(const bool batch);

    /**
     * Get if the filter manager is batch mode (true)
     * or in interactive mode (true)
     */
    bool getBatchMode(void) const;

    void setProgresUpdater(KoProgressUpdater *updater);

    /**
     * Return the KoProgressUpdater or 0 if there is none.
     **/
    KoProgressUpdater *progressUpdater() const;

private:
    // === API for KisFilterChains === (internal)
    // The friend methods are private in KisFilterChain and
    // just forward calls to the methods here. Should be
    // pretty safe.
    friend QString KisFilterChain::filterManagerImportFile() const;
    QString importFile() const {
        return m_importFileName;
    }
    friend QString KisFilterChain::filterManagerExportFile() const;
    QString exportFile() const {
        return m_exportFileName;
    }
    friend KisDocument *KisFilterChain::filterManagerKisDocument() const;
    KisDocument *document() const {
        return m_document;
    }
    friend int KisFilterChain::filterManagerDirection() const;
    int direction() const {
        return static_cast<int>(m_direction);
    }

    // Private API
    KisImportExportManager(const KisImportExportManager& rhs);
    KisImportExportManager &operator=(const KisImportExportManager& rhs);

    // Convert file path string or URL string into QUrl
    QUrl locationToUrl(QString location) const;

    void importErrorHelper(const QString& mimeType, const bool suppressDialog = false);

    KisDocument *m_document;
    QString m_importFileName;
    QString m_exportFileName;
    CalligraFilter::Graph m_graph;
    Direction m_direction;

    /// A static cache for the availability checks of filters
    static QStringList m_importMimeTypes;
    static QStringList m_exportMimeTypes;

    class Private;
    Private * const d;
};

#endif  // __KO_FILTER_MANAGER_H__
