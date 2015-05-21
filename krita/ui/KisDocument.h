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



#include <KoPageLayout.h>
#include "KoGridData.h"
#include "KoGuidesData.h"
#include <KoXmlReader.h>
#include <KoDocumentBase.h>
#include <kundo2stack.h>

#include <klocale.h>
#include <kglobal.h>

#include <kis_types.h>
#include <kis_painting_assistant.h>
#include <kis_debug.h>

#include "krita_export.h"

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

class KisUndoStore;
class KisPaintingAssistant;
class KisPart;

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
    Q_PROPERTY(bool backupFile READ backupFile WRITE setBackupFile)
    Q_PROPERTY(int pageCount READ pageCount)

protected:

    explicit KisDocument();

public:

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
    virtual bool reload();

    /**
     * Reimplemented from KisParts::ReadWritePart for internal reasons
     * (for the autosave functionality)
     */
    virtual bool openUrl(const KUrl & url);

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
    bool importDocument(const KUrl &url);

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
    bool exportDocument(const KUrl &url);

    /**
     * @brief Sets whether the document can be edited or is read only.
     *
     * This recursively applied to all child documents and
     * KisView::updateReadWrite is called for every attached
     * view.
     */
    virtual void setReadWrite(bool readwrite = true);

    /**
     * To be preferred when a document exists. It is fast when calling
     * it multiple times since it caches the result that readNativeFormatMimeType()
     * delivers.
     * This comes from the X-KDE-NativeMimeType key in the .desktop file.
     */
    virtual QByteArray nativeFormatMimeType() const { return KIS_MIME_TYPE; }

    /**
     * Returns the OASIS OpenDocument mimetype of the document, if supported
     * This comes from the X-KDE-NativeOasisMimeType key in the
     * desktop file
     *
     * @return the oasis mimetype or, if it hasn't one, the nativeformatmimetype.
     */
    virtual QByteArray nativeOasisMimeType() const  { return ""; }

    /// Checks whether a given mimetype can be handled natively.
    bool isNativeFormat(const QByteArray& mimetype) const;

    /// Returns a list of the mimetypes considered "native", i.e. which can
    /// be saved by KisDocument without a filter, in *addition* to the main one
    virtual QStringList extraNativeMimeTypes() const { return QStringList() << KIS_MIME_TYPE; }


    /// Enum values used by specialOutputFlag - note that it's a bitfield for supportedSpecialFormats
    enum { /*SaveAsCalligra1dot1 = 1,*/ // old and removed
        SaveAsDirectoryStore = 2,
        SaveAsFlatXML = 4,
        SaveEncrypted = 8
                        // bitfield! next value is 16
    };

    /**
     * Return the set of SupportedSpecialFormats that the application wants to
     * offer in the "Save" file dialog.
     */
    virtual int supportedSpecialFormats() const;

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
     * @param specialOutputFlag is for "save as older version" etc.
     */
    void setOutputMimeType(const QByteArray & mimeType, int specialOutputFlag = 0);
    QByteArray outputMimeType() const;
    int specialOutputFlag() const;

    /**
     * Returns true if this document was the result of opening a foreign
     * file format and if the user hasn't yet saved the document (in any
     * format).
     *
     * Used by KisMainWindow to warn the user when s/he lazily presses
     * CTRL+S to save in the same foreign format, putting all his/her
     * formatting at risk (normally an export confirmation only comes up
     * with Save As).
     *
     * @param exporting specifies whether this is the setting for a
     * File --> Export or File --> Save/Save As operation.
     */
    bool confirmNonNativeSave(const bool exporting) const;
    void setConfirmNonNativeSave(const bool exporting, const bool on);


    /**
     * @return true if saving/exporting should inhibit the option dialog
     */
    bool saveInBatchMode() const;

    /**
     * @param batchMode if true, do not show the option dialog when saving or exporting.
     */
    void setSaveInBatchMode(const bool batchMode);

    /**
     * Sets the error message to be shown to the user (use i18n()!)
     * when loading or saving fails.
     * If you asked the user about something and they chose "Cancel",
     * set the message to the magic string "USER_CANCELED", to skip the error dialog.
     */
    void setErrorMessage(const QString& errMsg);

    /**
     * Return the last error message. Usually KisDocument takes care of
     * showing it; this method is mostly provided for non-interactive use.
     */
    QString errorMessage() const;


    /**
     * Show the last error message in a message box.
     * The dialog box will mention a loading problem.
     * openUrl/openFile takes care of doing it, but not loadNativeFormat itself,
     * so this is often called after loadNativeFormat returned false.
     */
    void showLoadingErrorDialog();


    /**
     * @brief Generates a preview picture of the document
     * @note The preview is used in the File Dialog and also to create the Thumbnail
     */
    virtual QPixmap generatePreview(const QSize& size);

    /**
     *  Tells the document that its title has been modified, either because
     *  the modified status changes (this is done by setModified() ) or
     *  because the URL or the document-info's title changed.
     */
    void setTitleModified();

    /**
     *  @return true if the document is empty.
     */
    virtual bool isEmpty() const;

    /**
     *  @brief Sets the document to empty.
     *
     *  Used after loading a template
     *  (which is not empty, but not the user's input).
     *
     *  @see isEmpty()
     */
    virtual void setEmpty();

    /**
     *  @brief Loads a document from a store.
     *
     *  You should never have to reimplement.
     *
     *  @param store The store to load from
     *  @param url An internal url, like tar:/1/2
     */
    virtual bool loadFromStore(KoStore *store, const QString& url);

    /// Unused
    virtual bool loadOdf(KoOdfReadStore & odfStore);
    /// Unused
    virtual bool saveOdf(SavingContext &documentContext);

    /**
     *  @brief Saves a sub-document to a store.
     *
     *  You should not have to reimplement this.
     */
    virtual bool saveToStore(KoStore *store, const QString& path);

    /**
     *  Reimplement this method to load the contents of your Calligra document,
     *  from the XML document. This is for the pre-Oasis file format (maindoc.xml).
     */
    virtual bool loadXML(const KoXmlDocument & doc, KoStore *store);

    /**
     *  Reimplement this to save the contents of the %Calligra document into
     *  a QDomDocument. The framework takes care of saving it to the store.
     */
    virtual QDomDocument saveXML();

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
     *  The first thing to do in loadOasis is get hold of the office:body tag, then its child.
     *  If the child isn't the expected one, the error message can indicate what it is instead.
     *  This method returns a translated name for the type of document,
     *  e.g. i18n("Word Processing") for office:text.
     */
    static QString tagNameToDocumentType(const QString& localName);

    /**
     *  Loads a document in the native format from a given URL.
     *  Reimplement if your native format isn't XML.
     *
     *  @param file the file to load - usually KReadOnlyPart::m_file or the result of a filter
     */
    virtual bool loadNativeFormat(const QString & file);

    /**
     *  Saves the document in native format, to a given file
     *  You should never have to reimplement.
     *  Made public for writing templates.
     */
    virtual bool saveNativeFormat(const QString & file);

    /**
     * Saves the document in the native format to the given store.
     */
    bool saveNativeFormatCalligra(KoStore *store);

    /**
     * Activate/deactivate/configure the autosave feature.
     * @param delay in seconds, 0 to disable
     */
    void setAutoSave(int delay);

    /**
     * Checks whether the document is currently in the process of autosaving
     */
    bool isAutosaving() const;

    /**
     * Set whether the next openUrl call should check for an auto-saved file
     * and offer to open it. This is usually true, but can be turned off
     * (e.g. for the preview module). This only checks for names auto-saved
     * files, unnamed auto-saved files are only checked on KisApplication startup.
     */
    void setCheckAutoSaveFile(bool b);

    /**
     * Set whether the next openUrl call should show error message boxes in case
     * of errors. This is usually the case, but e.g. not when generating thumbnail
     * previews.
     */
    void setAutoErrorHandlingEnabled(bool b);

    /**
     * Checks whether error message boxes should be shown.
     */
    bool isAutoErrorHandlingEnabled() const;

    /**
     * Retrieve the default value for autosave in seconds.
     * Called by the applications to use the correct default in their config
     */
    static int defaultAutoSave();

    /**
     * @return the information concerning this document.
     * @see KoDocumentInfo
     */
    KoDocumentInfo *documentInfo() const;

    /**
     * @return the object to report progress to.
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
     * Return true if url() is a real filename, false if url() is
     * an internal url in the store, like "tar:/..."
     */
    virtual bool isStoredExtern() const;

    /**
     * @return the page layout associated with this document (margins, pageSize, etc).
     * Override this if you want to provide different sized pages.
     *
     * @see KoPageLayout
     */
    virtual KoPageLayout pageLayout(int pageNumber = 0) const;
    virtual void setPageLayout(const KoPageLayout &pageLayout);

    /**
     * Performs a cleanup of unneeded backup files
     */
    void removeAutoSaveFiles();

    void setBackupFile(bool _b);

    bool backupFile()const;

    /**
     * Returns true if this document or any of its internal child documents are modified.
     */
    bool isModified() const;

    /**
     * Returns true during loading (openUrl can be asynchronous)
     */
    bool isLoading() const;

    /**
     * Sets the backup path of the document
     */
    void setBackupPath(const QString & _path);

    /**
     * @return path to the backup document
     */
    QString backupPath()const;

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
     * to set the url to KUrl()
     */
    void resetURL();

    /**
     * Set when you want an external embedded document to be stored internally
     */
    void setStoreInternal(bool i);

    /**
     * @return true when external embedded documents are stored internally
     */
    bool storeInternal() const;

    bool hasExternURL() const;

    /**
     * @internal (public for KisMainWindow)
     */
    void setMimeTypeAfterLoading(const QString& mimeType);

    /**
     * @return returns the number of pages in the document.
     */
    virtual int pageCount() const;

    /**
     * Returns the unit used to display all measures/distances.
     */
    KoUnit unit() const;

    /**
     * Sets the unit used to display all measures/distances.
     */
    void setUnit(const KoUnit &unit);

    /**
     * Save the unit to the settings writer
     *
     * @param settingsWriter
     */
    void saveUnitOdf(KoXmlWriter *settingsWriter) const;

    bool loadNativeFormatFromByteArray(QByteArray &data);

    /// return the grid data for this document.
    KoGridData &gridData();

    /// returns the guides data for this document.
    KoGuidesData &guidesData();

    void clearUndoHistory();


    /**
     *  Sets the modified flag on the document. This means that it has
     *  to be saved or not before deleting it.
     */
    virtual void setModified(bool _mod);

    /**
     * Initialize an empty document using default values
     */
    virtual void initEmpty();

    /**
     * Returns the global undo stack
     */
    KUndo2Stack *undoStack();


    /**
     * Set the output stream to report profile information to.
     */
    void setProfileStream(QTextStream *profilestream);

    /**
     * Set the output stream to report profile information to.
     */
    void setProfileReferenceTime(const QTime& referenceTime);

public Q_SLOTS:

    /**
     * Adds a command to the undo stack and executes it by calling the redo() function.
     * @param command command to add to the undo stack
     */
    virtual void addCommand(KUndo2Command *command);

    /**
     * Begins recording of a macro command. At the end endMacro needs to be called.
     * @param text command description
     */
    virtual void beginMacro(const KUndo2MagicString &text);

    /**
     * Ends the recording of a macro command.
     */
    virtual void endMacro();

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

private:

    friend class KisPart;

    /**
     * Generate a name for the document.
     */
    QString newObjectName();

    QString autoSaveFile(const QString & path) const;
    void setDisregardAutosaveFailure(bool disregardFailure);

    /**
     *  Loads a document from KReadOnlyPart::m_file (KParts takes care of downloading
     *  remote documents).
     *  Applies a filter if necessary, and calls loadNativeFormat in any case
     *  You should not have to reimplement, except for very special cases.
     *
     * NOTE: this method also creates a new KisView instance!
     *
     * This method is called from the KReadOnlyPart::openUrl method.
     */
    virtual bool openFile();

    /**
     * This method is called by @a openFile() to allow applications to setup there
     * own KoProgressUpdater-subTasks which are then taken into account for the
     * displayed progressbar during loading.
     */
    virtual void setupOpenFileSubProgress();

    /**
     *  Saves a document to KReadOnlyPart::m_file (KParts takes care of uploading
     *  remote documents)
     *  Applies a filter if necessary, and calls saveNativeFormat in any case
     *  You should not have to reimplement, except for very special cases.
     */
    virtual bool saveFile();

    /**
     *  Overload this function if you have to load additional files
     *  from a store. This function is called after loadXML()
     *  and after loadChildren() have been called.
     */
    virtual bool completeLoading(KoStore *store);

    /**
     *  If you want to write additional files to a store,
     *  then you must do it here.
     *  In the implementation, you should prepend the document
     *  url (using url().url()) before the filename, so that everything is kept relative
     *  to this document. For instance it will produce urls such as
     *  tar:/1/pictures/picture0.png, if the doc url is tar:/1
     *  But do this ONLY if the document is not stored extern (see isStoredExtern() ).
     *  If it is, then the pictures should be saved to tar:/pictures.
     */
    virtual bool completeSaving(KoStore *store);


    /** @internal */
    virtual void setModified();

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

    QString localFilePath() const;
    void setLocalFilePath( const QString &localFilePath );

    KoDocumentInfoDlg* createDocumentInfoDialog(QWidget *parent, KoDocumentInfo *docInfo) const;

    bool isReadWrite() const;

    KUrl url() const;
    void setUrl(const KUrl &url);

    virtual bool closeUrl(bool promptToSave = true);

    virtual bool saveAs( const KUrl &url );

public Q_SLOTS:

    virtual bool save();
    bool waitSaveComplete();

Q_SIGNALS:

    void completed();
    void canceled(const QString &);

private Q_SLOTS:

    void setImageModified();

    void slotAutoSave();

    /// Called by the undo stack when undo or redo is called
    void slotUndoStackIndexChanged(int idx);

protected:

    bool oldLoadAndParse(KoStore *store, const QString& filename, KoXmlDocument& doc);

public:

    /**
     * Create a new image that has this document as a parent and
     * replace the current image with this image.
     */
    bool newImage(const QString& name, qint32 width, qint32 height, const KoColorSpace * cs, const KoColor &bgColor, const QString &imageDescription, const double imageResolution);

    /**
     * Create a new image that has this document as a parent and
     * replace the current image with this image.
     */
    bool newImage(const QString& name, qint32 width, qint32 height, const KoColorSpace * cs, const KoColor &bgColor, bool backgroundAsLayer,
                  int numberOfLayers, const QString &imageDescription, const double imageResolution);

    /**
     * Create a new image that has this document as a parent and
     * replace the current image with this image.
     */
    KisImageWSP newImage(const QString& name, qint32 width, qint32 height, const KoColorSpace * colorspace);

    KisImageWSP image() const;

    /**
     * Makes an otherwise empty document ready for import/export
     */
    void prepareForImport();

    /**
     * Set the current image to the specified image and turn undo on.
     */
    void setCurrentImage(KisImageWSP image);

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

    /**
      *@return a list of all the assistants in all current views
      */
    QList<KisPaintingAssistant *> assistants();

    /**
     * @return a list of assistants loaded from a document
     */
    QList<KisPaintingAssistant *> preLoadedAssistants();


private:

    void init();

    bool saveToStream(QIODevice *dev);

    QString checkImageMimeTypes(const QString &mimeType, const KUrl& url) const;

    bool loadNativeFormatFromStoreInternal(KoStore *store);

    bool savePreview(KoStore *store);

    QString prettyPathOrUrl() const;

    bool saveToUrl();
    bool openUrlInternal(const KUrl &url);

    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void _k_slotJobFinished( KJob * job ))
    Q_PRIVATE_SLOT(d, void _k_slotStatJobFinished(KJob*))
    Q_PRIVATE_SLOT(d, void _k_slotGotMimeType(KIO::Job *job, const QString &mime))
    Q_PRIVATE_SLOT(d, void _k_slotUploadFinished( KJob * job ))

};

Q_DECLARE_METATYPE(KisDocument*)

#endif
