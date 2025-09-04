/*
 *  SPDX-FileCopyrightText: 2013-2014 Yue Liu <yue.liu@mail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LIBKIS_FILEDIALOG_H
#define LIBKIS_FILEDIALOG_H

#include "kritalibkis_export.h"

#include <QDialog>

/**
 * Wrapper around KoFileDialog, which is a wrapper around QFileDialog,
 * providing native file dialogs on KDE/Gnome/Windows/OSX/etc.
 * FileDialog respects Krita's "Don't use native file dialogs" setting.
 */
class KRITALIBKIS_EXPORT FileDialog : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(FileDialog)

public:
    enum DialogType
    {
        OpenFile,
        OpenFiles,
        OpenDirectory,
        ImportFile,
        ImportFiles,
        ImportDirectory,
        SaveFile
    };

    /**
     * @brief constructor
     * @param parent The parent of the file dialog
     * @param dialogType usage of the file dialog
     * Valid arguments:
     * <ul>
     * <li>FileDialog.DialogType.OpenFile</li>
     * <li>FileDialog.DialogType.OpenFiles</li>
     * <li>FileDialog.DialogType.OpenDirectory</li>
     * <li>FileDialog.DialogType.ImportFile</li>
     * <li>FileDialog.DialogType.ImportFiles</li>
     * <li>FileDialog.DialogType.ImportDirectory</li>
     * <li>FileDialog.DialogType.SaveFile</li>
     * <ul>
     * @param dialogName the name for the file dialog. This will be used to open
     * the filedialog in the last open location, instead of the specified directory.
     *
     * @return The name of the entry user selected in the file dialog
     *
     */
    FileDialog(QWidget *parent = nullptr,
               const FileDialog::DialogType = DialogType::OpenFile,
               const QString &dialogName = "");

    ~FileDialog() override;

    // Set the text in the dialog's title bar. 
    // Not all native dialogs show this.
    void setCaption(const QString &caption);

    /**
     * @brief setDefaultDir set the default directory to defaultDir.
     *
     * @param defaultDir a path to a file or directory
     */
    void setDefaultDir(const QString &defaultDir, bool force = false);

    /**
     * @brief setDirectoryUrl set the default URI to defaultUri.
     * @param defaultUri a Uri to a file from some ContentProvider
     */
    void setDirectoryUrl(const QUrl &defaultUri);

    /**
     * @brief setImageFilters sets the name filters for the file dialog to all
     * image formats Qt's QImageReader supports.
     */
    void setImageFilters();

    /**
     * @brief setMimeTypeFilters Update the list of file filters from mime types.
     * @param mimeTypeList A list of mime types that forms the basis of this dialog's file filters
     * @param defaultMimeType Sets the default filter based on this mime type
     */
    void setMimeTypeFilters(const QStringList &mimeTypeList,
                            QString defaultMimeType = QString());

    // Set the file type filter
    void setNameFilter(const QString &filter);

    //Set the selected file type filter
    void selectNameFilter(const QString &filter);

    /// Show the file dialog and return multiple file names the user selected
    QStringList filenames();

    /// Show the file dialog and return the file name the user selected
    QString filename();

    /**
     * @brief Create and show a file dialog and return the name of an existing file selected by the user
     * @param parent Dialog parent widget
     * @param caption Dialog caption
     * @param directory Starting directory for the file dialog
     * @param filter Name filters for files shown
     * @param selectedFilter The selected name filter
     * @param dialogName Internal name of the dialog used for remembering the opened directory
     * @return the name of an existing file selected by the user
     */
    static QString getOpenFileName(QWidget *parent = nullptr, const QString &caption = QString(), const QString &directory = QString(), const QString &filter = QString(), const QString &selectedFilter = QString(), const QString &dialogName = QString());
    
    /**
     * @brief Create and show a file dialog and return the name of multiple existing files selected by the user
     * @param parent Dialog parent widget
     * @param caption Dialog caption
     * @param directory Starting directory for the file dialog
     * @param filter Name filters for files shown
     * @param selectedFilter The selected name filter
     * @param dialogName Internal name of the dialog used for remembering the opened directory
     * @return the name of multiple existing files selected by the user
     */
    static QStringList getOpenFileNames(QWidget *parent = nullptr, const QString &caption = QString(), const QString &directory = QString(), const QString &filter = QString(), const QString &selectedFilter = QString(), const QString &dialogName = QString());
    
    /**
     * @brief Create and show a file dialog and return the name of an existing directory selected by the user
     * @param parent Dialog parent widget
     * @param caption Dialog caption
     * @param directory Starting directory for the file dialog
     * @param dialogName Internal name of the dialog used for remembering the opened directory
     * @return the name of an existing directory selected by the user
     */
    static QString getExistingDirectory(QWidget *parent = nullptr, const QString &caption = QString(), const QString &directory = QString(), const QString &dialogName = QString());
    
    /**
     * @brief Create and show a file dialog and return the name of a file to save to selected by the user
     * @param parent Dialog parent widget
     * @param caption Dialog caption
     * @param directory Starting directory for the file dialog
     * @param filter Name filters for files shown
     * @param selectedFilter The selected name filter
     * @param dialogName Internal name of the dialog used for remembering the opened directory
     * @return the name of a file to save to selected by the user
     */
    static QString getSaveFileName(QWidget *parent = nullptr, const QString &caption = QString(), const QString &directory = QString(), const QString &filter = QString(), const QString &selectedFilter = QString(), const QString &dialogName = QString());

    /**
     * @brief selectedNameFilter returns the name filter the user selected, either
     *    directory or by clicking on it.
     * @return
     */
    QString selectedNameFilter() const;

    QString selectedMimeType() const;

public Q_SLOTS:
    // Set default file extension matching the filter.
    void onFilterSelected(const QString &filter);

private:
    //static KoFileDialog* createDialog(QWidget *parent, QString caption, QString defaultDir, QString filter, QString selectedFilter, QString dialogName, KoFileDialog::DialogType type);

    struct Private;
    QScopedPointer<Private> const d;
};

#endif /* LIBKIS_FILEDIALOG_H */
