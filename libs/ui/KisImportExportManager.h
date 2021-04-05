/*
 * SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_IMPORT_EXPORT_MANAGER_H
#define KIS_IMPORT_EXPORT_MANAGER_H

#include <QObject>
#include <QMap>
#include <QByteArray>
#include <QUrl>

#include "KisImportExportFilter.h"

#include "kritaui_export.h"

class KisDocument;
class KoProgressUpdater;

template <class T>
class QFuture;

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

public:

    ~KisImportExportManager() override;

    /**
     * Imports the specified document and returns the resultant filename
     * (most likely some file in /tmp).
     * @p path can be either a URL or a filename.
     * @p documentMimeType gives importDocument a hint about what type
     * the document may be. It can be left empty.
     *
     * @return  status signals the success/error of the conversion.
     * If the QString which is returned isEmpty() and the status is OK,
     * then we imported the file directly into the document.
     */
    KisImportExportErrorCode importDocument(const QString &location, const QString &mimeType);

    /**
     * @brief Exports the given file/document to the specified URL/mimetype.
     *
     * If @p mimeType is empty, then the closest matching Calligra part is searched
     * and when the method returns @p mimeType contains this mimetype.
     * Oh, well, export is a C++ keyword ;)
     */
    KisImportExportErrorCode exportDocument(const QString &location, const QString& realLocation, const QByteArray &mimeType, bool showWarnings = true, KisPropertiesConfigurationSP exportConfiguration = 0, bool isAdvancedExporting = false );

    QFuture<KisImportExportErrorCode> exportDocumentAsyc(const QString &location, const QString& realLocation, const QByteArray &mimeType, KisImportExportErrorCode &status, bool showWarnings = true, KisPropertiesConfigurationSP exportConfiguration = 0,bool isAdvancedExporting= false);

    ///@name Static API
    //@{
    /**
     * Suitable for passing to KoFileDialog::setMimeTypeFilters. The default mime
     * gets set by the "users" of this method, as we do not have enough
     * information here.
     * Optionally, @p extraNativeMimeTypes are added after the native mimetype.
     */
    static QStringList supportedMimeTypes(Direction direction);

    /**
     * @brief filterForMimeType loads the relevant import/export plugin and returns it. The caller
     * is responsible for deleting it!
     * @param mimetype the mimetype we want to import/export. If there's more than one plugin, the one
     * with the highest weight as defined in the json description will be taken
     * @param direction import or export
     * @return a pointer to the filter plugin or 0 if none could be found
     */
    static KisImportExportFilter *filterForMimeType(const QString &mimetype, Direction direction);

    /**
     * Fill necessary information for the export filter into the properties, e.g. if the image has
     * transparency or has sRGB profile.
     */
    static void fillStaticExportConfigurationProperties(KisPropertiesConfigurationSP exportConfiguration, KisImageSP image);

    /**
     * Get if the filter manager is batch mode (true)
     * or in interactive mode (true)
     */
    bool batchMode(void) const;

    void setUpdater(KoUpdaterPtr updater);

    static QString askForAudioFileName(const QString &defaultDir, QWidget *parent);

    static QString getUriForAdditionalFile(const QString &defaultUri, QWidget *parent);


private:

    struct ConversionResult;
    ConversionResult convert(Direction direction, const QString &location, const QString& realLocation, const QString &mimeType, bool showWarnings, KisPropertiesConfigurationSP exportConfiguration, bool isAsync, bool isAdvancedExporting= false);


    void fillStaticExportConfigurationProperties(KisPropertiesConfigurationSP exportConfiguration);
    bool askUserAboutExportConfiguration(QSharedPointer<KisImportExportFilter> filter, KisPropertiesConfigurationSP exportConfiguration, const QByteArray &from, const QByteArray &to, bool batchMode, const bool showWarnings, bool *alsoAsKra, bool isAdvancedExporting = false);

    KisImportExportErrorCode doImport(const QString &location, QSharedPointer<KisImportExportFilter> filter);

    KisImportExportErrorCode doExport(const QString &location, QSharedPointer<KisImportExportFilter> filter, KisPropertiesConfigurationSP exportConfiguration, bool alsoAsKra);
    KisImportExportErrorCode doExportImpl(const QString &location, QSharedPointer<KisImportExportFilter> filter, KisPropertiesConfigurationSP exportConfiguration);

    // Private API
    KisImportExportManager(const KisImportExportManager& rhs);
    KisImportExportManager &operator=(const KisImportExportManager& rhs);

    KisDocument *m_document;

    /// A static cache for the availability checks of filters
    static QStringList m_importMimeTypes;
    static QStringList m_exportMimeTypes;

    class Private;
    Private * const d;
};

#endif  // __KO_FILTER_MANAGER_H__
