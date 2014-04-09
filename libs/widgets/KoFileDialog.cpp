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
#include <QClipboard>

#include <kconfiggroup.h>
#include <kmimetype.h>
#include <klocale.h>

class KoFileDialog::Private
{
public:
    Private(QWidget *parent_,
            KoFileDialog::DialogType dialogType_,
            const QString caption_,
            const QString defaultDir_,
            const QString uniqueName_)
        : parent(parent_)
        , type(dialogType_)
        , uniqueName(uniqueName_)
        , caption(caption_)
        , directory(defaultDir_)
        , filterType(KoFileDialog::NameFilter)
        , filterList(QStringList())
        , defaultFilter(QString())
        , fileDialog(0)
    {
    }

    QWidget *parent;
    KoFileDialog::DialogType type;
    QString uniqueName;
    QString caption;
    QString directory;
    KoFileDialog::FilterType filterType;
    QStringList filterList;
    QString defaultFilter;
    QFileDialog *fileDialog;
};

KoFileDialog::KoFileDialog(QWidget *parent,
                           KoFileDialog::DialogType type,
                           const QString uniqueName)
    : d(new Private(parent, type, "", getUsedDir(uniqueName), uniqueName))
{
}

KoFileDialog::~KoFileDialog()
{
    delete d->fileDialog;
    delete d;
}

void KoFileDialog::setCaption(const QString &caption)
{
    d->caption = caption;
}

void KoFileDialog::setDefaultDir(const QString &defaultDir)
{
    if (d->directory.isEmpty() || d->directory.isNull())
        d->directory = defaultDir;
}

void KoFileDialog::setNameFilter(const QString &filter)
{
    d->filterType = KoFileDialog::NameFilter;
    d->filterList.clear();
    d->filterList << filter;
    d->defaultFilter.clear();
}

void KoFileDialog::setNameFilters(const QStringList &filterList,
                                  const QString &defaultFilter)
{
    d->filterType = KoFileDialog::NameFilter;
    d->filterList = filterList;
    d->defaultFilter = defaultFilter;
}

void KoFileDialog::setMimeTypeFilters(const QStringList &filterList,
                                      const QString &defaultFilter)
{
    d->filterType = KoFileDialog::MimeFilter;
    d->filterList = filterList;
    d->defaultFilter = defaultFilter;
}

void KoFileDialog::setHideNameFilterDetailsOption()
{
	if (!d->fileDialog) {
		createFileDialog();
	}
    d->fileDialog->setOption(QFileDialog::HideNameFilterDetails);
}

void KoFileDialog::createFileDialog()
{
    if (d->fileDialog) {
        delete d->fileDialog;
    }

    d->fileDialog = new QFileDialog(d->parent, d->caption, d->directory);

    if (d->type == SaveFile || d->type == SaveFiles) {
        d->fileDialog->setAcceptMode(QFileDialog::AcceptSave);
        d->fileDialog->setFileMode(QFileDialog::AnyFile);
    } else { // open / import
        d->fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
        if (d->type == ImportDirectory || d->type == ImportDirectories ||
            d->type == OpenDirectory || d->type == OpenDirectories) {
            d->fileDialog->setFileMode(QFileDialog::Directory);
            d->fileDialog->setOption(QFileDialog::ShowDirsOnly, true);
        } else { // open / import file(s)
            if (d->type == OpenFile || d->type == ImportFile) {
                d->fileDialog->setFileMode(QFileDialog::ExistingFile);
            } else { // files
                d->fileDialog->setFileMode(QFileDialog::ExistingFiles);
            }
        }
    }

    // add "All Supported Formats" filter
    if (d->type == OpenFile || d->type == OpenFiles ||
        d->type == ImportFile || d->type == ImportFiles) {
        if (d->filterType == MimeFilter) {
            d->filterType = NameFilter;
            d->filterList = getFilterString(d->filterList, true);
            d->defaultFilter = getFilterString(d->defaultFilter);
        }
    }

    if (d->filterType == MimeFilter) {
#if QT_VERSION >= 0x050200
        d->fileDialog->setMimeTypeFilters(d->filterList);
        if (!d->defaultFilter.isEmpty())
            d->fileDialog->selectMimeTypeFilter(d->defaultFilter);
#else
        d->fileDialog->setNameFilters(getFilterString(d->filterList));
        if (!d->defaultFilter.isEmpty())
            d->fileDialog->selectNameFilter(getFilterString(d->defaultFilter));
#endif
    } else {
        d->fileDialog->setNameFilters(d->filterList);
        if (!d->defaultFilter.isEmpty())
            d->fileDialog->selectNameFilter(d->defaultFilter);
    }

    if (d->type == ImportDirectory || d->type == ImportDirectories ||
        d->type == ImportFile || d->type == ImportFiles ||
        d->type == SaveFile || d->type == SaveFiles) {
        d->fileDialog->setWindowModality(Qt::WindowModal);
    }

#ifdef Q_WS_X11
    if (qgetenv("KDE_FULL_SESSION").size() == 0) {
        d->fileDialog->setOption(QFileDialog::DontUseNativeDialog);
    }
#endif
}

QString KoFileDialog::url()
{
    if (!d->fileDialog) {
        createFileDialog();
    }
    QString url;
    if (d->fileDialog->exec() == QDialog::Accepted) {
        url = d->fileDialog->selectedFiles().first();
        saveUsedDir(url, d->uniqueName);
    }
    return url;
}

void KoFileDialog::filterSelected(const QString &filter)
{
}

QStringList KoFileDialog::urls()
{
    if (!d->fileDialog) {
        createFileDialog();
    }
    QStringList urls;
    if (d->fileDialog->exec() == QDialog::Accepted) {
        urls = d->fileDialog->selectedFiles();
        saveUsedDir(urls.first(), d->uniqueName);
    }
    return urls;
}

const QStringList KoFileDialog::getFilterString(const QStringList &mimeList,
                                                  bool withAllSupportedEntry)
{
    QStringList ret;
    if (withAllSupportedEntry) {
        ret << QString(i18n("All supported formats") + " ( ");
    }

    for (QStringList::ConstIterator
         it = mimeList.begin(); it != mimeList.end(); ++it) {
        KMimeType::Ptr type = KMimeType::mimeType( *it );
        if(!type)
            continue;
        QString oneFilter(type->comment() + " ( ");
        QStringList patterns = type->patterns();
        QStringList::ConstIterator jt;
        for (jt = patterns.begin(); jt != patterns.end(); ++jt) {
            oneFilter.append(*jt + " ");
            if (withAllSupportedEntry) {
                ret[0].append(*jt + " ");
            }
        }
        oneFilter.append(")");
        ret << oneFilter;
    }

    if (withAllSupportedEntry) {
        ret[0].append(")");
    }
    //qDebug() << ret;
    return ret;
}

const QString KoFileDialog::getFilterString(const QString &defaultMime)
{
    QString filter;
    KMimeType::Ptr type = KMimeType::mimeType(defaultMime);
    if(type) {
        filter.append(type->comment() + " (");
        QStringList patterns = type->patterns();
        QStringList::ConstIterator jt;
        for (jt = patterns.begin(); jt != patterns.end(); ++jt)
            filter.append(*jt + " ");
        filter.append(")");
    }
    return filter;
}

const QString KoFileDialog::getUsedDir(const QString &dialogName)
{
    if (dialogName.isEmpty()) return "";

    KConfigGroup group = KGlobal::config()->group("File Dialogs");
    QString dir = group.readEntry(dialogName);
    return dir;
}

void KoFileDialog::saveUsedDir(const QString &fileName,
                               const QString &dialogName)
{
    if (dialogName.isEmpty()) return;

    QFileInfo fileInfo(fileName);
    KConfigGroup group = KGlobal::config()->group("File Dialogs");
    group.writeEntry(dialogName, fileInfo.absolutePath());

}
