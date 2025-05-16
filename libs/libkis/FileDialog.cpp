/*
 *  SPDX-FileCopyrightText: 2013-2014 Yue Liu <yue.liu@mail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "FileDialog.h"

#include "KoFileDialog.h"

struct FileDialog::Private {
    Private() {}

    KoFileDialog *dialog;
};

FileDialog::FileDialog(QWidget *parent,
                       FileDialog::DialogType type,
                       const QString &dialogName)
    : QDialog(parent)
    , d(new Private)
{
    d->dialog = new KoFileDialog(this, (KoFileDialog::DialogType) type, dialogName);
}

FileDialog::~FileDialog()
{
    delete d;
}

void FileDialog::setCaption(const QString &caption)
{
    d->dialog->setCaption(caption);
}

void FileDialog::setDefaultDir(const QString &defaultDir, bool force)
{
    d->dialog->setDefaultDir(defaultDir, force);
}

void FileDialog::setDirectoryUrl(const QUrl &defaultUri)
{
    d->dialog->setDirectoryUrl(defaultUri);
}

void FileDialog::setImageFilters()
{
    d->dialog->setImageFilters();
}

QString FileDialog::selectedNameFilter() const
{
    return d->dialog->selectedNameFilter();
}

QString FileDialog::selectedMimeType() const
{
    return d->dialog->selectedMimeType();
}

void FileDialog::setNameFilter(const QString &filter)
{
    d->dialog->setNameFilter(filter);
}

void FileDialog::selectNameFilter(const QString &filter)
{
    d->dialog->selectNameFilter(filter);
}

QString FileDialog::filename()
{
    return d->dialog->filename();
}

QStringList FileDialog::filenames()
{
    return d->dialog->filenames();
}

void FileDialog::setMimeTypeFilters(const QStringList &mimeTypeList, QString defaultMimeType)
{
    d->dialog->setMimeTypeFilters(mimeTypeList, defaultMimeType);
}

KoFileDialog* createDialog(QWidget *parent, QString caption, QString defaultDir, QString filter, QString selectedFilter, QString dialogName, KoFileDialog::DialogType type)
{
    KoFileDialog *dialog = new KoFileDialog(parent, type, dialogName);
    if (!caption.isEmpty()) dialog->setCaption(caption);
    if (!defaultDir.isEmpty()) dialog->setDefaultDir(defaultDir);
    if (!filter.isEmpty()) dialog->setNameFilter(filter);
    if (!selectedFilter.isEmpty()) dialog->selectNameFilter(filter);

    return dialog;
}

QString FileDialog::getOpenFileName(QWidget *parent, const QString &caption, const QString &directory, const QString &filter, const QString &selectedFilter, const QString &dialogName)
{
    return createDialog(parent, caption, directory, filter, selectedFilter, dialogName, KoFileDialog::OpenFile)->filename();
}

QStringList FileDialog::getOpenFileNames(QWidget *parent, const QString &caption, const QString &directory, const QString &filter, const QString &selectedFilter, const QString &dialogName)
{
    return createDialog(parent, caption, directory, filter, selectedFilter, dialogName, KoFileDialog::OpenFile)->filenames();
}

QString FileDialog::getExistingDirectory(QWidget *parent, const QString &caption, const QString &directory, const QString &dialogName)
{
    return createDialog(parent, caption, directory, QString(), QString(), dialogName, KoFileDialog::OpenDirectory)->filename();
}

QString FileDialog::getSaveFileName(QWidget *parent, const QString &caption, const QString &directory, const QString &filter, const QString &selectedFilter, const QString &dialogName)
{
    return createDialog(parent, caption, directory, filter, selectedFilter, dialogName, KoFileDialog::SaveFile)->filename();
}

void FileDialog::onFilterSelected(const QString &filter)
{
    d->dialog->onFilterSelected(filter);
}
