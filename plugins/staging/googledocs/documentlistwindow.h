/*
 *  Copyright (c) 2010 Mani Chandrasekar <maninc@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef DOCUMENTLISTWINDOW_H
#define DOCUMENTLISTWINDOW_H

#include <QDialog>

#include "ui_documentlist.h"

class GoogleDocumentService;
class GoogleDocument;
class GoogleDocumentList;

class DocumentListWindow : public QDialog
{
    Q_OBJECT

public:
    DocumentListWindow(GoogleDocumentService *service, GoogleDocumentList *gList);
    ~DocumentListWindow();
    QString currentDocument();

public slots:
    void hideDialog();

private slots:
    void getClickedDocument(const QModelIndex & index);
    void fetchDocument();

private:
    Ui_ListDialog *m_docListDialog;
    GoogleDocumentService *m_gService;
};

#endif // DOCUMENTLISTWINDOW_H
