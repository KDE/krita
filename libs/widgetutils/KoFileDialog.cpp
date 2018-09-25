/* This file is part of the KDE project
   Copyright (C) 2013 - 2014 Yue Liu <yue.liu@mail.com>

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
 * Boston, MA 02110-1301, USA.
*/

#include "KoFileDialog.h"
#include <QDebug>
#include <QFileDialog>
#include <QApplication>
#include <QImageReader>
#include <QClipboard>

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
    QStringList filterList;
    QString defaultFilter;
    QScopedPointer<QFileDialog> fileDialog;
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
            d->proposedFileName = QFileInfo(d->proposedFileName).baseName() + "." + suffix;
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
    d->fileDialog.reset(new QFileDialog(d->parent, d->caption, d->defaultDirectory + "/" + d->proposedFileName));
    KConfigGroup group = KSharedConfig::openConfig()->group("File Dialogs");

    bool dontUseNative = true;
#ifdef Q_OS_UNIX
    if (qgetenv("XDG_CURRENT_DESKTOP") == "KDE") {
        dontUseNative = false;
    }
#endif
#ifdef Q_OS_WIN
    dontUseNative = false;
#endif

    d->fileDialog->setOption(QFileDialog::DontUseNativeDialog, group.readEntry("DontUseNativeFileDialog", dontUseNative));
    d->fileDialog->setOption(QFileDialog::DontConfirmOverwrite, false);
    d->fileDialog->setOption(QFileDialog::HideNameFilterDetails, true);

#ifdef Q_OS_OSX
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

    if (d->type == ImportDirectory ||
            d->type == ImportFile || d->type == ImportFiles ||
            d->type == SaveFile) {
        d->fileDialog->setWindowModality(Qt::WindowModal);
    }
}

QString KoFileDialog::filename()
{
    QString url;
    createFileDialog();
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
            url = url + extension;
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
