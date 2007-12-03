/* This file is part of the KDE project
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
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

#include "CreateNewBookmarkDialog.h"

CreateNewBookmark::CreateNewBookmark(const QList<QString> &nameList, const QString &suggestedName, QWidget *parent)
    : QWidget(parent)
{
    widget.setupUi(this);
    widget.bookmarkName->setCompleter(0);
    widget.bookmarkName->insertItems(0, nameList);
    widget.bookmarkName->setEditText(suggestedName);
    connect( widget.bookmarkName, SIGNAL( editTextChanged(const QString &) ), this, SIGNAL( bookmarkNameChanged(const QString &) ) );
}

QString CreateNewBookmark::bookmarkName()
{
    return widget.bookmarkName->currentText();
}

CreateNewBookmarkDialog::CreateNewBookmarkDialog(const QList<QString> &nameList, const QString &suggestedName, QWidget *parent)
    : KDialog(parent),
    m_nameList(nameList)
{
    if (suggestedName.isEmpty() || m_nameList.contains(suggestedName)) {
        enableButtonOk( false );
        ui = new CreateNewBookmark( nameList, "", this );
    }
    else {
        ui = new CreateNewBookmark( nameList, suggestedName, this );
    }
    setMainWidget( ui );
    setCaption(i18n("Create New Bookmark") );
    setModal( true );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    showButtonSeparator( true );
    connect( ui, SIGNAL( bookmarkNameChanged(const QString &) ), this, SLOT( nameChanged(const QString &) ) );
}

QString CreateNewBookmarkDialog::newBookmarkName()
{
    return ui->bookmarkName();
}

void CreateNewBookmarkDialog::nameChanged(const QString &bookmarkName)
{
    if (!m_nameList.contains(bookmarkName))
        enableButtonOk( !bookmarkName.isEmpty() );
    else
        enableButtonOk( false );
}

#include <CreateNewBookmarkDialog.moc>

