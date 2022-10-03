/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2013-2014 Yue Liu <yue.liu@mail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoFileDialog.h"
#include <QDebug>
#include <QFileDialog>
#include <KisPreviewFileDialog.h>
#include <QApplication>
#include <QImageReader>
#include <QClipboard>
#include <QInputDialog>
#include <QMessageBox>

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>
#include <kstandardguiitem.h>

#include <KisMimeDatabase.h>
#include <KoJsonTrader.h>
#include "WidgetUtilsDebug.h"

#include <kis_assert.h>

#ifdef Q_OS_MACOS
#include "KisMacosSecurityBookmarkManager.h"
#endif

class Q_DECL_HIDDEN KoFileDialog::Private
{
public:
    Private(QWidget *parent_,
            KoFileDialog::DialogType dialogType_,
            const QString caption_,
            const QString defaultDir_,
            const QString dialogName_)
        : parent(parent_)
        , type(dialogType_)
        , dialogName(dialogName_)
        , caption(caption_)
        , defaultDirectory(defaultDir_)
        , filterList(QStringList())
        , defaultFilter(QString())
    {
    }

    ~Private()
    {
    }

    QWidget *parent;
    KoFileDialog::DialogType type;
    QString dialogName;
    QString caption;
    QString defaultDirectory;
    QString proposedFileName;
    QUrl defaultUri;
    QStringList filterList;
    QMap<QString, QString> suffixes; // Filter description to extension, may lack some entries
    QString defaultFilter;
    QScopedPointer<KisPreviewFileDialog> fileDialog;
    QString mimeType;
};

KoFileDialog::KoFileDialog(QWidget *parent,
                           KoFileDialog::DialogType type,
                           const QString &dialogName)
    : d(new Private(parent, type, "", getUsedDir(dialogName), dialogName))
{
}

KoFileDialog::~KoFileDialog()
{
    delete d;
}

void KoFileDialog::setCaption(const QString &caption)
{
    d->caption = caption;
}

void KoFileDialog::setDefaultDir(const QString &defaultDir, bool force)
{
    if (!defaultDir.isEmpty()) {
        if (d->defaultDirectory.isEmpty() || force) {
            QFileInfo f(defaultDir);
            if (f.isDir()) {
                d->defaultDirectory = defaultDir;
            }
            else {
                d->defaultDirectory = f.absolutePath();
            }
        }
        if (!QFileInfo(defaultDir).isDir()) {
            d->proposedFileName = QFileInfo(defaultDir).fileName();
        }
    }
}

void KoFileDialog::setDirectoryUrl(const QUrl &defaultUri)
{
    d->defaultUri = defaultUri;
}

void KoFileDialog::setImageFilters()
{
    QStringList imageFilters;
    // add filters for all formats supported by QImage
    Q_FOREACH (const QByteArray &format, QImageReader::supportedImageFormats()) {
        imageFilters << QLatin1String("image/") + format;
    }
    setMimeTypeFilters(imageFilters);
}

QString KoFileDialog::selectedNameFilter() const
{
    return d->fileDialog->selectedNameFilter();
}

QString KoFileDialog::selectedMimeType() const
{
    return d->mimeType;
}

void KoFileDialog::createFileDialog()
{
    d->fileDialog.reset(new KisPreviewFileDialog(d->parent, d->caption, d->defaultDirectory + "/" + d->proposedFileName));
    if (!d->defaultUri.isEmpty()) {
        d->fileDialog->setDirectoryUrl(d->defaultUri);
    }
    connect(d->fileDialog.get(), SIGNAL(filterSelected(const QString&)), this, SLOT(onFilterSelected(const QString&)));

#ifdef Q_OS_MACOS
    KisMacosSecurityBookmarkManager *bookmarkmngr = KisMacosSecurityBookmarkManager::instance();
    if(bookmarkmngr->isSandboxed()) {
        connect(d->fileDialog.get(), SIGNAL(urlSelected  (const QUrl&)), bookmarkmngr, SLOT(addBookmarkAndCheckParentDir(const QUrl&)));
    }
#endif

    KConfigGroup group = KSharedConfig::openConfig()->group("File Dialogs");

    bool dontUseNative = true;
#ifdef Q_OS_ANDROID
    dontUseNative = false;
#endif
#ifdef Q_OS_UNIX
    if (qgetenv("XDG_CURRENT_DESKTOP") == "KDE") {
        dontUseNative = false;
    }
#endif
#ifdef Q_OS_MACOS
    dontUseNative = false;
#endif
#ifdef Q_OS_WIN
    dontUseNative = false;
#endif

    bool optionDontUseNative;
    if (!qEnvironmentVariable("APPIMAGE").isEmpty()) {
        // AppImages don't have access to platform plugins. BUG: 447805
        optionDontUseNative = false;
    } else {
        optionDontUseNative = group.readEntry("DontUseNativeFileDialog", dontUseNative);
    }

    d->fileDialog->setOption(QFileDialog::DontUseNativeDialog, optionDontUseNative);
    d->fileDialog->setOption(QFileDialog::DontConfirmOverwrite, false);
    d->fileDialog->setOption(QFileDialog::HideNameFilterDetails, dontUseNative ? true : false);


#ifdef Q_OS_MACOS
    QList<QUrl> urls = d->fileDialog->sidebarUrls();
    QUrl volumes = QUrl::fromLocalFile("/Volumes");
    if (!urls.contains(volumes)) {
        urls.append(volumes);
    }

    d->fileDialog->setSidebarUrls(urls);
#endif

    if (d->type == SaveFile) {
        d->fileDialog->setAcceptMode(QFileDialog::AcceptSave);
        d->fileDialog->setFileMode(QFileDialog::AnyFile);
    }
    else { // open / import

        d->fileDialog->setAcceptMode(QFileDialog::AcceptOpen);

        if (d->type == ImportDirectory || d->type == OpenDirectory) {
            d->fileDialog->setFileMode(QFileDialog::Directory);
            d->fileDialog->setOption(QFileDialog::ShowDirsOnly, true);
        }
        else { // open / import file(s)
            if (d->type == OpenFile || d->type == ImportFile)
            {
                d->fileDialog->setFileMode(QFileDialog::ExistingFile);
            }
            else { // files
                d->fileDialog->setFileMode(QFileDialog::ExistingFiles);
            }
        }
    }

#ifndef Q_OS_ANDROID
    d->fileDialog->setNameFilters(d->filterList);

    if (!d->proposedFileName.isEmpty()) {
        QString mime = KisMimeDatabase::mimeTypeForFile(d->proposedFileName, d->type == KoFileDialog::SaveFile ? false : true);

        QString description = KisMimeDatabase::descriptionForMimeType(mime);
        Q_FOREACH(const QString &filter, d->filterList) {
            if (filter.startsWith(description)) {
                d->fileDialog->selectNameFilter(filter);
                break;
            }
        }
    }
    else if (!d->defaultFilter.isEmpty()) {
        d->fileDialog->selectNameFilter(d->defaultFilter);
    }
#endif

    if (d->type == ImportDirectory ||
            d->type == ImportFile || d->type == ImportFiles ||
            d->type == SaveFile) {

        bool allowModal = true;
// MacOS do not declare native file dialog as modal BUG:413241.
#ifdef Q_OS_MACOS
        allowModal = optionDontUseNative;
//        if ( d->proposedFileName.isEmpty() ) {
//            d->fileDialog->selectFile("untitled.kra");
//        } else {
//            d->fileDialog->selectFile(d->proposedFileName);
//        }
//        qDebug() << d->proposedFileName.isEmpty() << d->proposedFileName << d->defaultDirectory;
#endif
        if (allowModal) {
            d->fileDialog->setWindowModality(Qt::WindowModal);
        }
    }
    d->fileDialog->resetIconProvider();

    // QFileDialog::filterSelected is not emitted with the initial value
    onFilterSelected(d->fileDialog->selectedNameFilter());
}

QString KoFileDialog::filename()
{
    QString url;
    createFileDialog();

#ifdef Q_OS_ANDROID
    if (d->type == SaveFile) {
        QString extension = ".kra";
        QInputDialog mimeSelector;
        mimeSelector.setLabelText(i18n("Save As:"));
        mimeSelector.setComboBoxItems(d->filterList);
        mimeSelector.setOkButtonText(KStandardGuiItem::ok().text());
        mimeSelector.setCancelButtonText(KStandardGuiItem::cancel().text());
        // combobox as they stand, are very hard to scroll on a touch device
        mimeSelector.setOption(QInputDialog::UseListViewForComboBoxItems);

        if (mimeSelector.exec() == QDialog::Accepted) {
            const QString selectedFilter = mimeSelector.textValue();
            int start = selectedFilter.indexOf("*.") + 1;
            int end = selectedFilter.indexOf(" ", start);
            int n = end - start;
            extension = selectedFilter.mid(start, n);
            if (!extension.startsWith(".")) {
                extension = "." + extension;
            }
            d->fileDialog->selectNameFilter(selectedFilter);

            const QString proposedFileBaseName = QFileInfo(d->proposedFileName).baseName();
            // HACK: discovered by looking into the code
            d->fileDialog->setWindowTitle(proposedFileBaseName.isEmpty() ? QString("Untitled" + extension)
                                                                         : proposedFileBaseName + extension);
        } else {
            return url;
        }
    }
#endif

    bool retryNeeded;
    do {
        retryNeeded = false;
        if (d->fileDialog->exec() == QDialog::Accepted) {
            url = d->fileDialog->selectedFiles().first();
        } else {
            url = QString();
            break;
        }

        // The Android native file selector does not know to add the .kra
        // extension (MIME type not registered), so just skip the whole file
        // suffix check for Android.
#ifndef Q_OS_ANDROID
        const QString suffix = QFileInfo(url).suffix();
        bool isValidSuffix = true;
        if (KisMimeDatabase::mimeTypeForSuffix(suffix).isEmpty()) {
            warnWidgetUtils << "Selected file name suffix" << suffix << "does not match known MIME types";
            isValidSuffix = false;
        }

        if (d->type == SaveFile && (suffix.isEmpty() || !isValidSuffix)) {
            QString extension;
            if (d->suffixes.contains(d->fileDialog->selectedNameFilter())) {
                extension = d->suffixes[d->fileDialog->selectedNameFilter()];
                if (!extension.isEmpty()) {
                    // Append the default file extension to the file name before
                    // relaunching the file selector. We do _not_ just append
                    // the extension and return the new file name because:
                    //  * it bypasses the file overwrite prompt provided by the
                    //    file selector.
                    //  * doing so will break sandboxed macOS and Android,
                    //    because access to user files is restricted and must
                    //    be done through the native file selector.
                    url.append('.').append(extension);
                    d->fileDialog->selectFile(url);
                }
            }
            if (extension.isEmpty()) {
                // Use the first extension of the selected filter just as a suggestion
                QString selectedFilter;
                // skip index 0 which is "All supported formats"
                for (int i = 1; i < d->filterList.size(); ++i) {
                    if (d->filterList[i].startsWith(d->fileDialog->selectedNameFilter())) {
                        selectedFilter = d->filterList[i];
                        break;
                    }
                }
                int start = selectedFilter.indexOf("*.") + 2;
                int end = selectedFilter.indexOf(" ", start);
                if (start != -1 + 2 && end != -1) {
                    extension = selectedFilter.mid(start, end - start);
                }
            }
            QMessageBox::warning(d->parent, d->caption,
                i18n("The selected file name does not have a file extension that Krita understands.\n"
                     "Make sure the file name ends in '.%1' for example.", extension));
            retryNeeded = true;

// We can only write to the Uri that was returned, we don't have permission to change the Uri.
#if !(defined(Q_OS_MACOS) || defined(Q_OS_ANDROID))
            url = url + extension;
#endif
        }
#endif
    } while (retryNeeded);

    if (!url.isEmpty()) {
        d->mimeType = KisMimeDatabase::mimeTypeForFile(url, d->type == KoFileDialog::SaveFile ? false : true);
        saveUsedDir(url, d->dialogName);
    }
    return url;
}

QStringList KoFileDialog::filenames()
{
    QStringList urls;

    createFileDialog();
    if (d->fileDialog->exec() == QDialog::Accepted) {
        urls = d->fileDialog->selectedFiles();
    }
    if (urls.size() > 0) {
        saveUsedDir(urls.first(), d->dialogName);
    }
    return urls;
}

QStringList KoFileDialog::splitNameFilter(const QString &nameFilter, QStringList *mimeList)
{
    Q_ASSERT(mimeList);

    QStringList filters;
    QString description;

    if (nameFilter.contains("(")) {
        description = nameFilter.left(nameFilter.indexOf("(") -1).trimmed();
    }

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QStringList entries = nameFilter.mid(nameFilter.indexOf("(") + 1).split(" ", Qt::SkipEmptyParts);
#else
    QStringList entries = nameFilter.mid(nameFilter.indexOf("(") + 1).split(" ", QString::SkipEmptyParts);
#endif

    entries.sort();
    Q_FOREACH (QString entry, entries) {

        entry = entry.remove("*");
        entry = entry.remove(")");

        QString mimeType = KisMimeDatabase::mimeTypeForSuffix(entry);
        if (mimeType != "application/octet-stream") {
            if (!mimeList->contains(mimeType)) {
                mimeList->append(mimeType);
                filters.append(KisMimeDatabase::descriptionForMimeType(mimeType) + " ( *" + entry + " )");
            }
        }
        else {
            filters.append(entry.remove(".").toUpper() + " " + description + " ( *." + entry + " )");
        }
    }
    return filters;
}

void KoFileDialog::setMimeTypeFilters(const QStringList &mimeTypeList, QString defaultMimeType)
{
    constexpr bool withAllSupportedEntry = true;
    QStringList mimeSeen;

    struct FilterData
    {
        QString descriptionOnly;
        QString fullLine;
        QString defaultSuffix;
    };

    FilterData defaultFilter {};
    // 1
    QString allSupported;
    // 2
    FilterData kritaNative {};
    // 3
    FilterData ora {};
    // remaining
    QVector<FilterData> otherFileTypes;
    // All files
    bool hasAllFilesFilter = false;

    QStringList mimeList = mimeTypeList;
    mimeList.sort();

    Q_FOREACH(const QString &mimeType, mimeList) {
        if (!mimeSeen.contains(mimeType)) {
            if (mimeType == QLatin1String("application/octet-stream")) {
                // QFileDialog uses application/octet-stream for the
                // "All files (*)" filter. We can do the same here.
                hasAllFilesFilter = true;
                mimeSeen << mimeType;
                continue;
            }
            QString description = KisMimeDatabase::descriptionForMimeType(mimeType);
            if (description.isEmpty() && !mimeType.isEmpty()) {
                description = mimeType.split("/")[1];
                if (description.startsWith("x-")) {
                    description = description.remove(0, 2);
                }
            }


            QString oneFilter;
            const QStringList suffixes = KisMimeDatabase::suffixesForMimeType(mimeType);
            KIS_SAFE_ASSERT_RECOVER(!suffixes.isEmpty()) {
                qCritical() << "KoFileDialog: Found no suffixes for mime type" << mimeType;
                warnWidgetUtils << "KoFileDialog: Found no suffixes for mime type" << mimeType;
                continue;
            }

            Q_FOREACH(const QString &suffix, suffixes) {
                const QString glob = QStringLiteral("*.") + suffix;
                oneFilter.append(glob + " ");
                if (withAllSupportedEntry) {
                    allSupported.append(glob + " ");
                }
#ifdef Q_OS_LINUX
                if (qgetenv("XDG_CURRENT_DESKTOP") == "GNOME") {
                    oneFilter.append(glob.toUpper() + " ");
                    if (withAllSupportedEntry) {
                        allSupported.append(glob.toUpper() + " ");
                    }
                }
#endif
            }

            Q_ASSERT(!description.isEmpty());

            FilterData filterData {};
            filterData.descriptionOnly = description;
            filterData.fullLine = description + " ( " + oneFilter + ")";
            filterData.defaultSuffix = suffixes.first();

            if (mimeType == QLatin1String("application/x-krita")) {
                kritaNative = filterData;
            } else if (mimeType == QLatin1String("image/openraster")) {
                ora = filterData;
            } else {
                otherFileTypes.append(filterData);
            }
            if (defaultMimeType == mimeType) {
                debugWidgetUtils << "KoFileDialog: Matched default MIME type to filter" << filterData.fullLine;
                defaultFilter = filterData;
            }
            mimeSeen << mimeType;
        }
    }

    QStringList retFilterList;
    QMap<QString, QString> retFilterToSuffixMap;
    auto addFilterItem = [&](const FilterData &filterData) {
        if (retFilterList.contains(filterData.fullLine)) {
            debugWidgetUtils << "KoFileDialog: Duplicated filter" << filterData.fullLine;
            return;
        }
        retFilterList.append(filterData.fullLine);
        // the "simplified" version that comes to "onFilterSelect" when details are disabled
        retFilterToSuffixMap.insert(filterData.descriptionOnly, filterData.defaultSuffix);
        // "full version" that comes when details are enabled
        retFilterToSuffixMap.insert(filterData.fullLine, filterData.defaultSuffix);
    };

    if (!allSupported.isEmpty()) {
        FilterData allFilter {};
        if (allSupported.contains("*.kra")) {
            allSupported.remove("*.kra ");
            allSupported.prepend("*.kra ");
            allFilter.defaultSuffix = QStringLiteral("kra");
        } else if (!defaultFilter.fullLine.isEmpty()) {
            const QString suffixToMove = QString("*.") + defaultFilter.defaultSuffix + " ";
            allSupported.remove(suffixToMove);
            allSupported.prepend(suffixToMove);
            allFilter.defaultSuffix = defaultFilter.defaultSuffix;
        } else {
            // XXX: we don't have a meaningful default suffix
            warnWidgetUtils << "KoFileDialog: No default suffix for 'All supported formats'";
            allFilter.defaultSuffix = QStringLiteral("");
        }
        allFilter.descriptionOnly = i18n("All supported formats");
        allFilter.fullLine = allFilter.descriptionOnly + " ( " + allSupported + ")";
        addFilterItem(allFilter);
    }
    if (!kritaNative.fullLine.isEmpty()) {
        addFilterItem(kritaNative);
    }
    if (!ora.fullLine.isEmpty()) {
        addFilterItem(ora);
    }

    std::sort(otherFileTypes.begin(), otherFileTypes.end(), [](const FilterData &a, const FilterData &b) {
        return a.descriptionOnly < b.descriptionOnly;
    });
    Q_FOREACH(const FilterData &filterData, otherFileTypes) {
        addFilterItem(filterData);
    }

    if (hasAllFilesFilter) {
        // Reusing Qt's existing "All files" translation
        retFilterList.append(QFileDialog::tr("All files (*)"));
    }

    d->filterList = retFilterList;
    d->suffixes = retFilterToSuffixMap;
    d->defaultFilter = defaultFilter.fullLine; // this can be empty
}

QString KoFileDialog::getUsedDir(const QString &dialogName)
{
    if (dialogName.isEmpty()) return "";

    KConfigGroup group =  KSharedConfig::openConfig()->group("File Dialogs");
    QString dir = group.readEntry(dialogName, "");
    return dir;
}

void KoFileDialog::saveUsedDir(const QString &fileName,
                               const QString &dialogName)
{

    if (dialogName.isEmpty()) return;

    QFileInfo fileInfo(fileName);
    KConfigGroup group =  KSharedConfig::openConfig()->group("File Dialogs");
    group.writeEntry(dialogName, fileInfo.absolutePath());

}

void KoFileDialog::onFilterSelected(const QString &filter)
{
    debugWidgetUtils << "KoFileDialog::onFilterSelected" << filter;

    // Setting default suffix for Android is broken as of Qt 5.12.0, returning the file
    // with extension added but no write permissions granted.
#ifndef Q_OS_ANDROID
    QFileDialog::FileMode mode = d->fileDialog->fileMode();
    if (mode != QFileDialog::FileMode::Directory && mode != QFileDialog::FileMode::DirectoryOnly) {
        // we do not need suffixes for directories
        if (d->suffixes.contains(filter)) {
            QString suffix = d->suffixes[filter];
            debugWidgetUtils << "  Setting default suffix to" << suffix;
            d->fileDialog->setDefaultSuffix(suffix);
        } else {
            warnWidgetUtils << "KoFileDialog::onFilterSelected: Cannot find suffix for filter" << filter;
            d->fileDialog->setDefaultSuffix("");
        }
    }
#endif
}
