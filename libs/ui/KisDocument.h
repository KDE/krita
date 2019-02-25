/* This file is part of the Krita project
 *
 * Copyright (C) 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KISDOCUMENT_H
#define KISDOCUMENT_H

#include <QDateTime>
#include <QTransform>
#include <QList>

#include <klocalizedstring.h>

#include <KoPageLayout.h>
#include <KoDocumentBase.h>
#include <kundo2stack.h>
#include <KoColorSet.h>

#include <kis_image.h>
#include <KisImportExportFilter.h>
#include <kis_properties_configuration.h>
#include <kis_types.h>
#include <kis_painting_assistant.h>
#include <KisReferenceImage.h>
#include <kis_debug.h>
#include <KisImportExportUtils.h>
#include <kis_config.h>

#include "kritaui_export.h"

#include <memory>

class QString;

class KUndo2Command;
class KoUnit;

class KoColor;
class KoColorSpace;
class KoShapeControllerBase;
class KoShapeLayer;
class KoStore;
class KoOdfReadStore;
class KoDocumentInfo;
class KoDocumentInfoDlg;
class KisImportExportManager;
class KisUndoStore;
class KisPart;
class KisGridConfig;
class KisGuidesConfig;
class KisMirrorAxisConfig;
class QDomDocument;
class KisReferenceImagesLayer;

#define KIS_MIME_TYPE "application/x-krita"

/**
 *  The %Calligra document class
 *
 *  This class provides some functionality each %Calligra document should have.
 *
 *  @short The %Calligra document class
 */
class KRITAUI_EXPORT KisDocument : public QObject, public KoDocumentBase
{
    Q_OBJECT

protected:

    explicit KisDocument();

    /**
     * @brief KisDocument makes a deep copy of the document \p rhs.
     *        The caller *must* ensure that the image is properly
     *        locked and is in consistent state before asking for
     *        cloning.
     * @param rhs the source document to copy from
     */
    explicit KisDocument(const KisDocument &rhs);

public:
    enum OpenFlag {
        None = 0,
        DontAddToRecent = 0x1,
        RecoveryFile = 0x2
    };
    Q_DECLARE_FLAGS(OpenFlags, OpenFlag)

    /**
     *  Destructor.
     *
     * The destructor does not delete any attached KisView objects and it does not
     * delete the attached widget as returned by widget().
     */
    ~KisDocument() override;

    /**
     * @brief reload Reloads the document from the original url
     * @return the result of loading the document
     */
    bool reload();


    /**
     * @brief creates a clone of the document and returns it. Please make sure that you
     * hold all the necessary locks on the image before asking for a clone!
     */
    KisDocument* clone();

    /**
     * @brief openUrl Open an URL
     * @param url The URL to open
     * @param flags Control specific behavior
     * @return success status
     */
    bool openUrl(const QUrl &url, OpenFlags flags = None);

    /**
     * Opens the document given by @p url, without storing the URL
     * in the KisDocument.
     * Call this instead of openUrl() to implement KisMainWindow's
     * File --> Import feature.
     *
     * @note This will call openUrl(). To differentiate this from an ordinary
     *       Open operation (in any reimplementation of openUrl() or openFile())
     *       call isImporting().
     */
    bool importDocument(const QUrl &url);

    /**
     * Saves the document as @p url without changing the state of the
     * KisDocument (URL, modified flag etc.). Call this instead of
     * KisParts::ReadWritePart::saveAs() to implement KisMainWindow's
     * File --> Export feature.
     */
    bool exportDocument(const QUrl &url, const QByteArray &mimeType, bool showWarnings = false, KisPropertiesConfigurationSP exportConfiguration = 0);

    bool exportDocumentSync(const QUrl &url, const QByteArray &mimeType, KisPropertiesConfigurationSP exportConfiguration = 0);

private:
    bool exportDocumentImpl(const KritaUtils::ExportFileJob &job, KisPropertiesConfigurationSP exportConfiguration);

public:
    /**
     * @brief Sets whether the document can be edited or is read only.
     *
     * This recursively applied to all child documents and
     * KisView::updateReadWrite is called for every attached
     * view.
     */
    void setReadWrite(bool readwrite = true);

    /**
     * To be preferred when a document exists. It is fast when calling
     * it multiple times since it caches the result that readNativeFormatMimeType()
     * delivers.
     * This comes from the X-KDE-NativeMimeType key in the .desktop file.
     */
    static QByteArray nativeFormatMimeType() { return KIS_MIME_TYPE; }

    /// Checks whether a given mimetype can be handled natively.
    bool isNativeFormat(const QByteArray& mimetype) const;

    /// Returns a list of the mimetypes considered "native", i.e. which can
    /// be saved by KisDocument without a filter, in *addition* to the main one
    static QStringList extraNativeMimeTypes() { return QStringList() << KIS_MIME_TYPE; }

    /**
     * Returns the actual mimetype of the document
     */
    QByteArray mimeType() const override;

    /**
     * @brief Sets the mime type for the document.
     *
     * When choosing "save as" this is also the mime type
     * selected by default.
     */
    void setMimeType(const QByteArray & mimeType) override;

    /**
     * @return true if file operations should inhibit the option dialog
     */
    bool fileBatchMode() const;

    /**
     * @param batchMode if true, do not show the option dialog for file operations.
     */
    void setFileBatchMode(const bool batchMode);

    /**
     * Sets the error message to be shown to the user (use i18n()!)
     * when loading or saving fails.
     * If you asked the user about something and they chose "Cancel",
     */
    void setErrorMessage(const QString& errMsg);

    /**
     * Return the last error message. Usually KisDocument takes care of
     * showing it; this method is mostly provided for non-interactive use.
     */
    QString errorMessage() const;

    /**
     * Sets the warning message to be shown to the user (use i18n()!)
     * when loading or saving fails.
     */
    void setWarningMessage(const QString& warningMsg);

    /**
     * Return the last warning message set by loading or saving. Warnings
     * mean that the document could not be completely loaded, but the errors
     * were not absolutely fatal.
     */
    QString warningMessage() const;

    /**
     * @brief Generates a preview picture of the document
     * @note The preview is used in the File Dialog and also to create the Thumbnail
     */
    QPixmap generatePreview(const QSize& size);

    /**
     *  Tells the document that its title has been modified, either because
     *  the modified status changes (this is done by setModified() ) or
     *  because the URL or the document-info's title changed.
     */
    void setTitleModified();

    /**
     *  @brief Sets the document to empty.
     *
     *  Used after loading a template
     *  (which is not empty, but not the user's input).
     *
     *  @see isEmpty()
     */
    void setEmpty(bool empty = true);

    /**
     *  Return a correctly created QDomDocument for this KisDocument,
     *  including processing instruction, complete DOCTYPE tag (with systemId and publicId), and root element.
     *  @param tagName the name of the tag for the root element
     *  @param version the DTD version (usually the application's version).
     */
    QDomDocument createDomDocument(const QString& tagName, const QString& version) const;

    /**
     *  Return a correctly created QDomDocument for an old (1.3-style) %Calligra document,
     *  including processing instruction, complete DOCTYPE tag (with systemId and publicId), and root element.
     *  This static method can be used e.g. by filters.
     *  @param appName the app's instance name, e.g. words, kspread, kpresenter etc.
     *  @param tagName the name of the tag for the root element, e.g. DOC for words/kpresenter.
     *  @param version the DTD version (usually the application's version).
     */
    static QDomDocument createDomDocument(const QString& appName, const QString& tagName, const QString& version);

   /**
     *  Loads a document in the native format from a given URL.
     *  Reimplement if your native format isn't XML.
     *
     *  @param file the file to load - usually KReadOnlyPart::m_file or the result of a filter
     */
    bool loadNativeFormat(const QString & file);

    /**
     * Set standard autosave interval that is set by a config file
     */
    void setNormalAutoSaveInterval();

    /**
     * Set emergency interval that autosave uses when the image is busy,
     * by default it is 10 sec
     */
    void setEmergencyAutoSaveInterval();

    /**
     * Disable autosave
     */
    void setInfiniteAutoSaveInterval();

    /**
     * @return the information concerning this document.
     * @see KoDocumentInfo
     */
    KoDocumentInfo *documentInfo() const;

    /**
     * Performs a cleanup of unneeded backup files
     */
    void removeAutoSaveFiles(const QString &autosaveBaseName, bool wasRecovered);

    /**
     * Returns true if this document or any of its internal child documents are modified.
     */
    bool isModified() const override;

    /**
     * @return caption of the document
     *
     * Caption is of the form "[title] - [url]",
     * built out of the document info (title) and pretty-printed
     * document URL.
     * If the title is not present, only the URL it returned.
     */
    QString caption() const;

    /**
     * Sets the document URL to empty URL
     * KParts doesn't allow this, but %Calligra apps have e.g. templates
     * After using loadNativeFormat on a template, one wants
     * to set the url to QUrl()
     */
    void resetURL();

    /**
     * @internal (public for KisMainWindow)
     */
    void setMimeTypeAfterLoading(const QString& mimeType);

    /**
     * Returns the unit used to display all measures/distances.
     */
    KoUnit unit() const;

    /**
     * Sets the unit used to display all measures/distances.
     */
    void setUnit(const KoUnit &unit);

    KisGridConfig gridConfig() const;
    void setGridConfig(const KisGridConfig &config);

    /// returns the guides data for this document.
    const KisGuidesConfig& guidesConfig() const;
    void setGuidesConfig(const KisGuidesConfig &data);

    const KisMirrorAxisConfig& mirrorAxisConfig() const;
    void setMirrorAxisConfig(const KisMirrorAxisConfig& config);

    QList<KoColorSet *> &paletteList();
    void setPaletteList(const QList<KoColorSet *> &paletteList);

    void clearUndoHistory();


    /**
     *  Sets the modified flag on the document. This means that it has
     *  to be saved or not before deleting it.
     */
    void setModified(bool _mod);

    void setRecovered(bool value);
    bool isRecovered() const;

    void updateEditingTime(bool forceStoreElapsed);

    /**
     * Returns the global undo stack
     */
    KUndo2Stack *undoStack();


    /**
     * @brief importExportManager gives access to the internal import/export manager
     * @return the document's import/export manager
     */
    KisImportExportManager *importExportManager() const;

    /**
     * @brief serializeToNativeByteArray daves the document into a .kra file wtitten
     * to a memory-based byte-array
     * @return a byte array containing the .kra file
     */
    QByteArray serializeToNativeByteArray();


    /**
     * @brief isInSaving shown if the document has any (background) saving process or not
     * @return true if there is some saving in action
     */
    bool isInSaving() const;

public Q_SLOTS:

    /**
     * Adds a command to the undo stack and executes it by calling the redo() function.
     * @param command command to add to the undo stack
     */
    void addCommand(KUndo2Command *command);

    /**
     * Begins recording of a macro command. At the end endMacro needs to be called.
     * @param text command description
     */
    void beginMacro(const KUndo2MagicString &text);

    /**
     * Ends the recording of a macro command.
     */
    void endMacro();

Q_SIGNALS:

    /**
     * This signal is emitted when the unit is changed by setUnit().
     * It is common to connect views to it, in order to change the displayed units
     * (e.g. in the rulers)
     */
    void unitChanged(const KoUnit &unit);

    /**
     * Emitted e.g. at the beginning of a save operation
     * This is emitted by KisDocument and used by KisView to display a statusbar message
     */
    void statusBarMessage(const QString& text, int timeout = 0);

    /**
     * Emitted e.g. at the end of a save operation
     * This is emitted by KisDocument and used by KisView to clear the statusbar message
     */
    void clearStatusBarMessage();

    /**
    * Emitted when the document is modified
    */
    void modified(bool);

    void titleModified(const QString &caption, bool isModified);

    void sigLoadingFinished();

    void sigSavingFinished();

    void sigGuidesConfigChanged(const KisGuidesConfig &config);

    void sigBackgroundSavingFinished(KisImportExportFilter::ConversionStatus status, const QString &errorMessage);

    void sigCompleteBackgroundSaving(const KritaUtils::ExportFileJob &job, KisImportExportFilter::ConversionStatus status, const QString &errorMessage);

    void sigReferenceImagesChanged();

    void sigMirrorAxisConfigChanged();

private Q_SLOTS:
    void finishExportInBackground();
    void slotChildCompletedSavingInBackground(KisImportExportFilter::ConversionStatus status, const QString &errorMessage);
    void slotCompleteAutoSaving(const KritaUtils::ExportFileJob &job, KisImportExportFilter::ConversionStatus status, const QString &errorMessage);

    void slotCompleteSavingDocument(const KritaUtils::ExportFileJob &job, KisImportExportFilter::ConversionStatus status, const QString &errorMessage);

    void slotInitiateAsyncAutosaving(KisDocument *clonedDocument);

private:

    friend class KisPart;
    friend class SafeSavingLocker;

    bool initiateSavingInBackground(const QString actionName,
                                    const QObject *receiverObject, const char *receiverMethod,
                                    const KritaUtils::ExportFileJob &job,
                                    KisPropertiesConfigurationSP exportConfiguration,
                                    std::unique_ptr<KisDocument> &&optionalClonedDocument);

    bool initiateSavingInBackground(const QString actionName,
                                    const QObject *receiverObject, const char *receiverMethod,
                                    const KritaUtils::ExportFileJob &job,
                                    KisPropertiesConfigurationSP exportConfiguration);

    bool startExportInBackground(const QString &actionName, const QString &location,
                                 const QString &realLocation,
                                 const QByteArray &mimeType,
                                 bool showWarnings,
                                 KisPropertiesConfigurationSP exportConfiguration);

    /**
     * Activate/deactivate/configure the autosave feature.
     * @param delay in seconds, 0 to disable
     */
    void setAutoSaveDelay(int delay);

    /**
     * Generate a name for the document.
     */
    QString newObjectName();

    QString generateAutoSaveFileName(const QString & path) const;

    /**
     *  Loads a document
     *
     *  Applies a filter if necessary, and calls loadNativeFormat in any case
     *  You should not have to reimplement, except for very special cases.
     *
     * NOTE: this method also creates a new KisView instance!
     *
     * This method is called from the KReadOnlyPart::openUrl method.
     */
    bool openFile();

public:

    bool isAutosaving() const override;

public:

    QString localFilePath() const override;
    void setLocalFilePath( const QString &localFilePath );

    KoDocumentInfoDlg* createDocumentInfoDialog(QWidget *parent, KoDocumentInfo *docInfo) const;

    bool isReadWrite() const;

    QUrl url() const override;
    void setUrl(const QUrl &url) override;

    bool closeUrl(bool promptToSave = true);

    bool saveAs(const QUrl &url, const QByteArray &mimeType, bool showWarnings, KisPropertiesConfigurationSP exportConfigration = 0);

    /**
     * Create a new image that has this document as a parent and
     * replace the current image with this image.
     */
    bool newImage(const QString& name, qint32 width, qint32 height, const KoColorSpace * cs, const KoColor &bgColor, KisConfig::BackgroundStyle bgStyle,
                  int numberOfLayers, const QString &imageDescription, const double imageResolution);

    bool isSaving() const;
    void waitForSavingToComplete();


    KisImageWSP image() const;

    /**
     * @brief savingImage provides a detached, shallow copy of the original image that must be used when saving.
     * Any strokes in progress will not be applied to this image, so the result might be missing some data. On
     * the other hand, it won't block.
     *
     * @return a shallow copy of the original image, or 0 is saving is not in progress
     */
    KisImageSP savingImage() const;

    /**
     * Set the current image to the specified image and turn undo on.
     */
    void setCurrentImage(KisImageSP image, bool forceInitialUpdate = true);

    /**
     * Set the image of the document preliminary, before the document
     * has completed loading. Some of the document items (shapes) may want
     * to access image properties (bounds and resolution), so we should provide
     * it to them even before the entire image is loaded.
     *
     * Right now, the only use by KoShapeRegistry::createShapeFromOdf(), remove
     * after it is deprecated.
     */
    void hackPreliminarySetImage(KisImageSP image);

    KisUndoStore* createUndoStore();

    /**
     * The shape controller matches internal krita image layers with
     * the flake shape hierarchy.
     */
    KoShapeControllerBase * shapeController() const;

    KoShapeLayer* shapeForNode(KisNodeSP layer) const;

    /**
     * Set the list of nodes that was marked as currently active. Used *only*
     * for saving loading. Never use it for tools or processing.
     */
    void setPreActivatedNode(KisNodeSP activatedNode);

    /**
     * @return the node that was set as active during loading. Used *only*
     * for saving loading. Never use it for tools or processing.
     */
    KisNodeSP preActivatedNode() const;

    /// @return the list of assistants associated with this document
    QList<KisPaintingAssistantSP> assistants() const;

    /// @replace the current list of assistants with @param value
    void setAssistants(const QList<KisPaintingAssistantSP> &value);


    void setAssistantsGlobalColor(QColor color);
    QColor assistantsGlobalColor();



    /**
     * Get existing reference images layer or null if none exists.
     */
    KisSharedPtr<KisReferenceImagesLayer> referenceImagesLayer() const;

    void setReferenceImagesLayer(KisSharedPtr<KisReferenceImagesLayer> layer, bool updateImage);

    bool save(bool showWarnings, KisPropertiesConfigurationSP exportConfiguration);

    /**
     * Return the bounding box of the image and associated elements (e.g. reference images)
     */
    QRectF documentBounds() const;

Q_SIGNALS:

    void completed();
    void canceled(const QString &);

private Q_SLOTS:

    void setImageModified();

    void slotAutoSave();

    void slotUndoStackCleanChanged(bool value);

    void slotConfigChanged();


private:
    /**
     * @brief try to clone the image. This method handles all the locking for you. If locking
     *        has failed, no cloning happens
     * @return cloned document on success, null otherwise
     */
    KisDocument *lockAndCloneForSaving();

    QString exportErrorToUserMessage(KisImportExportFilter::ConversionStatus status, const QString &errorMessage);

    QString prettyPathOrUrl() const;

    bool openUrlInternal(const QUrl &url);

    void slotAutoSaveImpl(std::unique_ptr<KisDocument> &&optionalClonedDocument);

    class Private;
    Private *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KisDocument::OpenFlags)
Q_DECLARE_METATYPE(KisDocument*)

#endif
