/* This file is part of the Krita project
 *
 * Copyright (C) 2014 Boudewijn Rempt <boud@kogmbh.com>
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

#include <kis_properties_configuration.h>
#include <kis_types.h>
#include <kis_painting_assistant.h>
#include <kis_debug.h>

#include "kritaui_export.h"

class QString;

class KUndo2Command;
class KoUnit;

class KoColor;
class KoColorSpace;
class KoShapeBasedDocumentBase;
class KoShapeLayer;
class KoStore;
class KoOdfReadStore;
class KoDocumentInfo;
class KoProgressUpdater;
class KoProgressProxy;
class KoDocumentInfoDlg;
class KisImportExportManager;
class KisUndoStore;
class KisPaintingAssistant;
class KisPart;
class KisGridConfig;
class KisGuidesConfig;
class QDomDocument;

class KisPart;

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

public:

    enum OpenUrlFlags {
        OPEN_URL_FLAG_NONE                       = 1 << 0,
        OPEN_URL_FLAG_DO_NOT_ADD_TO_RECENT_FILES = 1 << 1,
    };

    /**
     *  Destructor.
     *
     * The destructor does not delete any attached KisView objects and it does not
     * delete the attached widget as returned by widget().
     */
    virtual ~KisDocument();

    /**
     * @brief reload Reloads the document from the original url
     * @return the result of loading the document
     */
    bool reload();

    /**
     * @brief openUrl Open an URL
     * @param url The URL to open
     * @param flags Control specific behavior
     * @return success status
     */
    bool openUrl(const QUrl &url, OpenUrlFlags flags = OPEN_URL_FLAG_NONE);

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
     *
     * @note This will call KisDocument::saveAs(). To differentiate this
     *       from an ordinary Save operation (in any reimplementation of
     *       saveFile()) call isExporting().
     */
    bool exportDocument(const QUrl &url, KisPropertiesConfigurationSP exportConfiguration = 0);

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
    QByteArray mimeType() const;

    /**
     * @brief Sets the mime type for the document.
     *
     * When choosing "save as" this is also the mime type
     * selected by default.
     */
    void setMimeType(const QByteArray & mimeType);

    /**
     * @brief Set the format in which the document should be saved.
     *
     * This is called on loading, and in "save as", so you shouldn't
     * have to call it.
     *
     * @param mimeType the mime type (format) to use.
     */
    void setOutputMimeType(const QByteArray & mimeType);
    QByteArray outputMimeType() const;

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
     *  Saves the document in native format, to a given file
     *  You should never have to reimplement.
     *  Made public for writing templates.
     */
    bool saveNativeFormat(const QString & file);

    /**
     * Activate/deactivate/configure the autosave feature.
     * @param delay in seconds, 0 to disable
     */
    void setAutoSaveDelay(int delay);

    /**
     * @return the information concerning this document.
     * @see KoDocumentInfo
     */
    KoDocumentInfo *documentInfo() const;

    /**
     * @return the object to report progress to.
     *
     * This is only not zero if loading or saving is in progress.
     *
     * One can add more KoUpdaters to it to make the progress reporting more
     * accurate. If no active progress reporter is present, 0 is returned.
     **/
    KoProgressUpdater *progressUpdater() const;

    /**
     * Set a custom progress proxy to use to report loading
     * progress to.
     */
    void setProgressProxy(KoProgressProxy *progressProxy);
    KoProgressProxy* progressProxy() const;

    /**
     * Performs a cleanup of unneeded backup files
     */
    void removeAutoSaveFiles();

    /**
     * @brief setBackupFile enable/disable saving a backup of the file on saving
     * @param saveBackup if true, Krita will save a backup of the file
     */
    void setBackupFile(bool saveBackup);

    /**
     * Returns true if this document or any of its internal child documents are modified.
     */
    bool isModified() const;

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

    void clearUndoHistory();


    /**
     *  Sets the modified flag on the document. This means that it has
     *  to be saved or not before deleting it.
     */
    void setModified(bool _mod);

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
     * Progress info while loading or saving. The value is in percents (i.e. a number between 0 and 100)
     * Your KisDocument-derived class should emit the signal now and then during load/save.
     * KisMainWindow will take care of displaying a progress bar automatically.
     */
    void sigProgress(int value);

    /**
     * Progress cancel button pressed
     * This is emitted by KisDocument
     */
    void sigProgressCanceled();

    /**
     * Emitted e.g. at the beginning of a save operation
     * This is emitted by KisDocument and used by KisView to display a statusbar message
     */
    void statusBarMessage(const QString& text);

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

private:

    friend class KisPart;
    friend class SafeSavingLocker;

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

    /**
     *  Saves a document
     *
     *  Applies a filter if necessary, and calls saveNativeFormat in any case
     *  You should not have to reimplement, except for very special cases.
     */
    bool saveFile(KisPropertiesConfigurationSP exportConfiguration = 0);


    /** @internal */
    void setModified();

    /**
     *  Returns whether or not the current openUrl() or openFile() call is
     *  actually an import operation (like File --> Import).
     *  This is for informational purposes only.
     */
    bool isImporting() const;

    /**
     *  Returns whether or not the current saveFile() call is actually an export
     *  operation (like File --> Export).
     *  If this function returns true during saveFile() and you are changing
     *  some sort of state, you _must_ restore it before the end of saveFile();
     *  otherwise, File --> Export will not work properly.
     */
    bool isExporting() const;

public:

    bool isAutosaving() const;

public:

    QString localFilePath() const;
    void setLocalFilePath( const QString &localFilePath );

    KoDocumentInfoDlg* createDocumentInfoDialog(QWidget *parent, KoDocumentInfo *docInfo) const;

    bool isReadWrite() const;

    QUrl url() const;
    void setUrl(const QUrl &url);

    bool closeUrl(bool promptToSave = true);

    bool saveAs(const QUrl &url, KisPropertiesConfigurationSP exportConfigration = 0);

    /**
     * Create a new image that has this document as a parent and
     * replace the current image with this image.
     */
    bool newImage(const QString& name, qint32 width, qint32 height, const KoColorSpace * cs, const KoColor &bgColor, bool backgroundAsLayer,
                  int numberOfLayers, const QString &imageDescription, const double imageResolution);


    KisImageWSP image() const;
    /**
     * Adds progressproxy for file operations
     */
    void setFileProgressProxy();

    /**
     * Clears progressproxy for file operations
     */
    void clearFileProgressProxy();

    /**
     * Adds progressupdater for file operations
     */
    void setFileProgressUpdater(const QString &text);

    /**
     * Clears progressupdater for file operations
     */
    void clearFileProgressUpdater();

    /**
     * Set the current image to the specified image and turn undo on.
     */
    void setCurrentImage(KisImageSP image);

    KisUndoStore* createUndoStore();

    /**
     * The shape controller matches internal krita image layers with
     * the flake shape hierarchy.
     */
    KoShapeBasedDocumentBase * shapeController() const;

    KoShapeLayer* shapeForNode(KisNodeSP layer) const;

    /**
     * @return a list of all layers that are active in all current views
     */
    vKisNodeSP activeNodes() const;

    /**
     * set the list of nodes that were marked as currently active
     */
    void setPreActivatedNode(KisNodeSP activatedNode);

    /**
     * @return the node that was set as active during loading
     */
    KisNodeSP preActivatedNode() const;

    QList<KisPaintingAssistantSP> assistants() const;
    void setAssistants(const QList<KisPaintingAssistantSP> value);

    bool save(KisPropertiesConfigurationSP exportConfiguration = 0);

Q_SIGNALS:

    void completed();
    void canceled(const QString &);

private Q_SLOTS:

    void setImageModified();

    void slotAutoSave();

    /// Called by the undo stack when undo or redo is called
    void slotUndoStackIndexChanged(int idx);


private:

    QString prettyPathOrUrl() const;

    bool openUrlInternal(const QUrl &url);

    class Private;
    Private *const d;
};

Q_DECLARE_METATYPE(KisDocument*)

#endif
