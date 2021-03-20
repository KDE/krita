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

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

#include <KisMimeDatabase.h>
#include <KoJsonTrader.h>

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
        , swapExtensionOrder(false)
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
    QString defaultFilter;
    QScopedPointer<KisPreviewFileDialog> fileDialog;
    QString mimeType;
    bool swapExtensionOrder;
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

void KoFileDialog::setMimeTypeFilters(const QStringList &mimeTypeList, QString defaultMimeType)
{
    d->filterList = getFilterStringListFromMime(mimeTypeList, true);

    QString defaultFilter;

    if (!defaultMimeType.isEmpty()) {
        QString suffix = KisMimeDatabase::suffixesForMimeType(defaultMimeType).first();

        if (!d->proposedFileName.isEmpty()) {
            d->proposedFileName = QFileInfo(d->proposedFileName).completeBaseName() + "." + suffix;
        }

        QStringList defaultFilters = getFilterStringListFromMime(QStringList() << defaultMimeType, false);
        if (defaultFilters.size() > 0) {
            defaultFilter = defaultFilters.first();
        }
    }

    d->defaultFilter = defaultFilter;
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
#ifdef Q_OS_WIN
    dontUseNative = false;
#endif

    bool optionDontUseNative = group.readEntry("DontUseNativeFileDialog", dontUseNative);
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

        if (d->type == ImportDirectory || d->type == OpenDirectory){
            d->fileDialog->setFileMode(QFileDialog::Directory);
            d->fileDialog->setOption(QFileDialog::ShowDirsOnly, true);
        }
        else { // open / import file(s)
            if (d->type == OpenFile
                    || d->type == ImportFile)
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
#endif
        if (allowModal) {
            d->fileDialog->setWindowModality(Qt::WindowModal);
        }
    }
    d->fileDialog->resetIconProvider();
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
        // combobox as they stand, are very hard to scroll on a touch device
        mimeSelector.setOption(QInputDialog::UseListViewForComboBoxItems);

        if (mimeSelector.exec() == QDialog::Accepted) {
            const QString selectedFilter = mimeSelector.textValue();
            int start = selectedFilter.indexOf("*.") + 1;
            int end = selectedFilter.indexOf(" ", start);
            int n = end - start;
            extension = selectedFilter.mid(start, n);
            if (!extension.contains(".")) {
                extension = "." + extension;
            }
            d->fileDialog->selectNameFilter(selectedFilter);

            // HACK: discovered by looking into the code
            d->fileDialog->setWindowTitle(d->proposedFileName.isEmpty() ? "Untitled" + extension : d->proposedFileName);
        } else {
            return url;
        }
    }
#endif

    if (d->fileDialog->exec() == QDialog::Accepted) {
        url = d->fileDialog->selectedFiles().first();
    }

    if (!url.isEmpty()) {
        QString suffix = QFileInfo(url).suffix();
        if (KisMimeDatabase::mimeTypeForSuffix(suffix).isEmpty()) {
            suffix = "";
        }

        if (d->type == SaveFile && suffix.isEmpty()) {
            QString selectedFilter;
            // index 0 is all supported; if that is chosen, saveDocument will automatically make it .kra
            for (int i = 1; i < d->filterList.size(); ++i) {
                if (d->filterList[i].startsWith(d->fileDialog->selectedNameFilter())) {
                    selectedFilter = d->filterList[i];
                    break;
                }
            }
            int start = selectedFilter.indexOf("*.") + 1;
            int end = selectedFilter.indexOf(" ", start);
            int n = end - start;
            QString extension = selectedFilter.mid(start, n);
            if (!(extension.contains(".") || url.endsWith("."))) {
                extension = "." + extension;
            }
// We can only write to the Uri that was returned, we don't have permission to change the Uri.
#ifndef Q_OS_ANDROID
            url = url + extension;
#endif
        }

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

    QStringList entries = nameFilter.mid(nameFilter.indexOf("(") + 1).split(" ",QString::SkipEmptyParts );
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

const QStringList KoFileDialog::getFilterStringListFromMime(const QStringList &_mimeList,
                                                            bool withAllSupportedEntry)
{
    QStringList mimeSeen;

    // 1
    QString allSupported;
    // 2
    QString kritaNative;
    // 3
    QString ora;

    QStringList ret;
    QStringList mimeList = _mimeList;
    mimeList.sort();
    Q_FOREACH(const QString &mimeType, mimeList) {
        if (!mimeSeen.contains(mimeType)) {
            QString description = KisMimeDatabase::descriptionForMimeType(mimeType);
            if (description.isEmpty() && !mimeType.isEmpty()) {
                description = mimeType.split("/")[1];
                if (description.startsWith("x-")) {
                    description = description.remove(0, 2);
                }
            }


            QString oneFilter;
            QStringList patterns = KisMimeDatabase::suffixesForMimeType(mimeType);
            QStringList globPatterns;
            Q_FOREACH(const QString &pattern, patterns) {
                if (pattern.startsWith(".")) {
                    globPatterns << "*" + pattern;
                }
                else if (pattern.startsWith("*.")) {
                    globPatterns << pattern;
                }
                else {
                    globPatterns << "*." + pattern;
                }
            }

            Q_FOREACH(const QString &glob, globPatterns) {
                if (d->swapExtensionOrder) {
                    oneFilter.prepend(glob + " ");
                    if (withAllSupportedEntry) {
                        allSupported.prepend(glob + " ");
                    }
#ifdef Q_OS_LINUX
                    if (qgetenv("XDG_CURRENT_DESKTOP") == "GNOME") {
                        oneFilter.prepend(glob.toUpper() + " ");
                        if (withAllSupportedEntry) {
                            allSupported.prepend(glob.toUpper() + " ");
                        }
                    }
#endif

                }
                else {
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
            }

            Q_ASSERT(!description.isEmpty());

            oneFilter = description + " ( " + oneFilter + ")";


            if (mimeType == "application/x-krita") {
                kritaNative = oneFilter;
                continue;
            }
            if (mimeType == "image/openraster") {
                ora = oneFilter;
                continue;
            }
            else {
                ret << oneFilter;
            }
            mimeSeen << mimeType;
        }
    }

    ret.sort();
    ret.removeDuplicates();

    if (!ora.isEmpty()) ret.prepend(ora);
    if (!kritaNative.isEmpty())  ret.prepend(kritaNative);
    if (!allSupported.isEmpty()) ret.prepend(i18n("All supported formats") + " ( " + allSupported + (")"));

    return ret;

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
