/* This file is part of the Calligra libraries
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>
                 2002 Werner Trobin <trobin@kde.org>

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

#ifndef KIS_IMPORT_EXPORT_FILTER_H
#define KIS_IMPORT_EXPORT_FILTER_H

#include "KisImageBuilderResult.h"

#include <QObject>
#include <QIODevice>
#include <QMap>
#include <QPointer>
#include <QString>
#include <QPair>
#include <QList>
#include <KoID.h>
#include <QSharedPointer>
#include <kis_properties_configuration.h>
#include <kis_types.h>
#include <KisExportCheckBase.h>

class KoUpdater;
class KisDocument;
class KisConfigWidget;

#include "kritaui_export.h"
#include "KisImportExportErrorCode.h"

/**
 * @brief The base class for import and export filters.
 *
 * Derive your filter class from this base class and implement
 * the @ref convert() method. Don't forget to specify the Q_OBJECT
 * macro in your class even if you don't use signals or slots.
 * This is needed as filters are created on the fly.
 *
 * @note Take care: The m_chain pointer is invalid while the constructor
 * runs due to the implementation -- @em don't use it in the constructor.
 * After the constructor, when running the @ref convert() method it's
 * guaranteed to be valid, so no need to check against 0.
 *
 * @note If the code is compiled in debug mode, setting CALLIGRA_DEBUG_FILTERS
 * environment variable to any value disables deletion of temporary files while
 * importing/exporting. This is useful for testing purposes.
 *
 * @author Werner Trobin <trobin@kde.org>
 * @todo the class has no constructor and therefore cannot initialize its private class
 */
class KRITAUI_EXPORT KisImportExportFilter : public QObject
{
    Q_OBJECT
public:
    static const QString ImageContainsTransparencyTag;
    static const QString ColorModelIDTag;
    static const QString ColorDepthIDTag;
    static const QString sRGBTag;
public:
    /**
     * This enum is used to signal the return state of your filter.
     * Return OK in @ref convert() in case everything worked as expected.
     * Feel free to add some more error conditions @em before the last item
     * if it's needed.
     */
    enum ConversionStatus { OK,
                            UsageError,
                            CreationError,
                            FileNotFound,
                            StorageCreationError,
                            BadMimeType,
                            BadConversionGraph,
                            WrongFormat,
                            NotImplemented,
                            ParsingError,
                            InternalError,
                            UserCancelled,
                            InvalidFormat,
                            FilterCreationError,
                            ProgressCancelled,
                            UnsupportedVersion,
                            JustInCaseSomeBrokenCompilerUsesLessThanAByte = 255
                          };

    ~KisImportExportFilter() override;

    void setBatchMode(bool batchmode);
    void setFilename(const QString &filename);
    void setRealFilename(const QString &filename);
    void setMimeType(const QString &mime);
    void setUpdater(QPointer<KoUpdater> updater);
    QPointer<KoUpdater> updater();

    /**
     * The filter chain calls this method to perform the actual conversion.
     * The passed mimetypes should be a pair of those you specified in your
     * .desktop file.
     * You @em have to implement this method to make the filter work.
     *
     * @return The error status, see the @ref #ConversionStatus enum.
     *         KisImportExportFilter::OK means that everything is alright.
     */
    virtual KisImportExportErrorCode convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP configuration = 0) = 0;

    /**
     * Get the text version of the status value
     */
    static QString conversionStatusString(ConversionStatus status);

    /**
     * @brief defaultConfiguration defines the default settings for the given import export filter
     * @param from The mimetype of the source file/document
     * @param to The mimetype of the destination file/document
     * @return a serializable KisPropertiesConfiguration object
     */
    virtual KisPropertiesConfigurationSP defaultConfiguration(const QByteArray& from = "", const QByteArray& to = "") const;

    /**
     * @brief lastSavedConfiguration return the last saved configuration for this filter
     * @param from The mimetype of the source file/document
     * @param to The mimetype of the destination file/document
     * @return a serializable KisPropertiesConfiguration object
     */
    KisPropertiesConfigurationSP lastSavedConfiguration(const QByteArray &from = "", const QByteArray &to = "") const;

    /**
     * @brief createConfigurationWidget creates a widget that can be used to define the settings for a given import/export filter
     * @param parent the owner of the widget; the caller is responsible for deleting
     * @param from The mimetype of the source file/document
     * @param to The mimetype of the destination file/document
     *
     * @return the widget
     */
    virtual KisConfigWidget *createConfigurationWidget(QWidget *parent, const QByteArray& from = "", const QByteArray& to = "") const;

    /**
     * @brief generate and return the list of capabilities of this export filter. The list
     * @return returns the list of capabilities of this export filter
     */
    virtual QMap<QString, KisExportCheckBase*> exportChecks();

    /// Override and return false for the filters that use a library that cannot handle file handles, only file names.
    virtual bool supportsIO() const { return true; }

    /// Verify whether the given file is correct and readable
    virtual QString verify(const QString &fileName) const;

protected:
    /**
     * This is the constructor your filter has to call, obviously.
     */
    KisImportExportFilter(QObject *parent = 0);

    QString filename() const;
    QString realFilename() const;
    bool batchMode() const;
    QByteArray mimeType() const;

    void setProgress(int value);
    virtual void initializeCapabilities();
    void addCapability(KisExportCheckBase *capability);
    void addSupportedColorModels(QList<QPair<KoID, KoID> > supportedColorModels, const QString &name, KisExportCheckBase::Level level = KisExportCheckBase::PARTIALLY);

    QString verifyZiPBasedFiles(const QString &fileName, const QStringList &filesToCheck) const;

private:

    KisImportExportFilter(const KisImportExportFilter& rhs);
    KisImportExportFilter& operator=(const KisImportExportFilter& rhs);

    class Private;
    Private *const d;

};

#endif
