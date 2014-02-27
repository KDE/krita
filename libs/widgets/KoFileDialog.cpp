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
            DialogType dialogType_,
            const QString caption_,
            const QString defaultDir_,
            const QString uniqueName_)
        : parent(parent_)
        , dialogType(dialogType_)
        , uniqueName(uniqueName_)
        , caption(caption_)
        , directory(defaultDir_)
        , filterType(FilterName)
        , filterList(QStringList())
        , defaultFilter(QString())
    {
    }

    QWidget *parent;
    DialogType dialogType;
    QString uniqueName;
    QString caption;
    QString directory;
    FilterType filterType;
    QStringList filterList;
    QString defaultFilter;
};

KoFileDialog::KoFileDialog(QWidget *parent,
                           KoFileDialog::DialogType dialogType,
                           const QString &caption,
                           const QString &defaultDir,
                           const QString uniqueName)
    : d(new Private(parent, dialogType, caption,
        getDefaultDir(defaultDir, uniqueName), uniqueName))
{
}

KoFileDialog::~KoFileDialog()
{
    delete d;
}

void KoFileDialog::setNameFilter(const QString &filter)
{
    d->filterType = FilterName;
    d->filterList.clear();
    d->filterList << filter;
    d->defaultFilter.clear();
}

void KoFileDialog::setNameFilters(const QStringList &filterList,
                                  const QString &defaultFilter)
{
    d->filterType = FilterName;
    d->filterList = filterList;
    d->defaultFilter = defaultFilter;
}

void KoFileDialog::setMimeTypeFilters(const QStringList &filterList,
                                      const QString &defaultFilter)
{
    d->filterType = FilterMime;
    d->filterList = filterList;
    d->defaultFilter = defaultFilter;
}

QFileDialog* KoFileDialog::getDialog(KoFileDialog::ReturnType returnType)
{
    QFileDialog* dialog = new QFileDialog(d->parent, d->caption, d->directory);

    if (d->dialogType == FileSaveDialog) {
        dialog->setAcceptMode(QFileDialog::AcceptSave);
        dialog->setFileMode(QFileDialog::AnyFile);
    } else {
        dialog->setAcceptMode(QFileDialog::AcceptOpen);
        if (d->dialogType == DirectoryImportDialog ||
            d->dialogType == DirectoryOpenDialog) {
            dialog->setFileMode(QFileDialog::Directory);
            dialog->setOption(QFileDialog::ShowDirsOnly, true);
        } else {
            if (returnType == ReturnOne) {
                dialog->setFileMode(QFileDialog::ExistingFile);
            } else {
                dialog->setFileMode(QFileDialog::ExistingFiles);
            }
        }
    }

    if (d->dialogType == FileSaveDialog ||
        d->dialogType == FileOpenDialog ||
        d->dialogType == FileImportDialog) {
        // add "All Supported Formats" filter
        if (d->filterType == FilterMime) {
            d->filterType = FilterName;
            d->filterList = getFilterString(d->filterList, true);
            d->defaultFilter = getFilterString(d->defaultFilter);
        }
    }

    if (d->filterType = FilterMime) {
#if QT_VERSION >= 0x050200
        dialog->setMimeTypeFilters(d->filterList);
        if (!d->defaultFilter.isEmpty())
            dialog->selectMimeTypeFilter(d->defaultFilter);
#else
        dialog->setNameFilters(getFilterString(d->filterList));
        if (!d->defaultFilter.isEmpty())
            dialog->selectNameFilter(getFilterString(d->defaultFilter));
#endif
    } else {
        dialog->setNameFilters(d->filterList);
        if (!d->defaultFilter.isEmpty())
            dialog->selectNameFilter(d->defaultFilter);
    }

    if (d->dialogType == DirectoryImportDialog ||
        d->dialogType == FileImportDialog ||
        d->dialogType == FileSaveDialog) {
        dialog->setWindowModality(Qt::WindowModal);
    }

#ifdef Q_WS_X11
    if (qgetenv("KDE_FULL_SESSION").size() == 0) {
        dialog->setOption(QFileDialog::DontUseNativeDialog);
    }
#endif

    return dialog;
}

QString KoFileDialog::getQString()
{
    QFileDialog* dialog = getDialog(ReturnOne);
    QString result;
    if (dialog->exec() == QDialog::Accepted) {
        result = dialog->selectedFiles().first();
        saveDefaultDir(result, d->uniqueName);
    }
    delete dialog;
    return result;
}

QStringList KoFileDialog::getQStringList()
{
    QFileDialog* dialog = getDialog(ReturnList);
    QStringList result;
    if (dialog->exec() == QDialog::Accepted) {
        result = dialog->selectedFiles();
        saveDefaultDir(result.first(), d->uniqueName);
    }
    delete dialog;
    return result;
}

QUrl KoFileDialog::getQUrl()
{
    QFileDialog* dialog = getDialog(ReturnOne);
    QUrl result;
    if (dialog->exec() == QDialog::Accepted) {
#if QT_VERSION >= 0x050200
        result = dialog->selectedUrls().first();
#else
        result = QUrl(dialog->selectedFiles().first());
#endif
        saveDefaultDir(result.path(), d->uniqueName);
    }
    delete dialog;
    return result;
}

QList<QUrl> KoFileDialog::getQUrlList()
{
    QFileDialog* dialog = getDialog(ReturnList);
    QList<QUrl> result;
    if (dialog->exec() == QDialog::Accepted) {
#if QT_VERSION >= 0x050200
        result = dialog->selectedUrls();
#else
        foreach(QString url, dialog->selectedFiles()) {
            result << QUrl(url);
        }
#endif
        saveDefaultDir(result.first().path(), d->uniqueName);
    }
    delete dialog;
    return result;
}

KUrl KoFileDialog::getKUrl()
{
    QFileDialog* dialog = getDialog(ReturnOne);
    KUrl result;
    if (dialog->exec() == QDialog::Accepted) {
#if QT_VERSION >= 0x050200
        result = dialog->selectedUrls().first();
#else
        result = QUrl(dialog->selectedFiles().first());
#endif
        saveDefaultDir(result.path(), d->uniqueName);
    }
    delete dialog;
    return result;
}

const QStringList KoFileDialog::getFilterString(const QStringList &mimeList,
                                                  bool withAllSupported)
{
    QStringList ret;
    if (withAllSupported) {
        ret << QString(i18n("All supported formats") + " ( ");
    }

    QString oneFilter;
    for (QStringList::ConstIterator it = mimeList.begin(); it != mimeList.end(); ++it) {
        KMimeType::Ptr type = KMimeType::mimeType( *it );
        if(!type)
            continue;
        QString oneFilter(type->comment() + " ( ");
        QStringList patterns = type->patterns();
        QStringList::ConstIterator jt;
        for (jt = patterns.begin(); jt != patterns.end(); ++jt) {
            oneFilter.append(*jt + " ");
            if (withAllSupported) {
                ret[0].append(*jt + " ");
            }
        }
        oneFilter.append(")");
        ret << oneFilter;
    }

    if (withAllSupported) {
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

const QString KoFileDialog::getDefaultDir(const QString &defaultDir, const QString &dialogName)
{
    if (dialogName.isEmpty()) return defaultDir;

    KConfigGroup group = KGlobal::config()->group("File Dialogs");
    QString dir = group.readEntry(dialogName);
    return dir.isEmpty() ? defaultDir : dir;

}

void KoFileDialog::saveDefaultDir(const QString &fileName, const QString &dialogName)
{
    if (dialogName.isEmpty()) return;

    QFileInfo fileInfo(fileName);
    KConfigGroup group = KGlobal::config()->group("File Dialogs");
    group.writeEntry(dialogName, fileInfo.absolutePath());

}
