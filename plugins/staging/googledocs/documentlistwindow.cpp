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

#include "documentlistwindow.h"

#include <QList>
#include <QListWidgetItem>
#include <QDebug>

#include "googledocumentservice.h"
#include "googledocumentlist.h"
#include "googledocument.h"

DocumentListWindow::DocumentListWindow(GoogleDocumentService *service, GoogleDocumentList *gList)
        : m_docListDialog(new Ui_ListDialog),
          m_gService(service)
{
    m_docListDialog->setupUi(this);
    connect(m_docListDialog->listView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(getClickedDocument(const QModelIndex &)));
    connect(m_docListDialog->okButton, SIGNAL(clicked()), this, SLOT(fetchDocument()));
    connect(m_docListDialog->closeButton, SIGNAL(clicked()), this, SLOT(hideDialog()));

    m_docListDialog->listView->setModel(gList->documentModel());
    m_docListDialog->listView->hideColumn(1);
    m_docListDialog->listView->setItemsExpandable(false);
    show();
    m_docListDialog->listView->setColumnWidth(0, m_docListDialog->listView->rect().width() * 0.75);
}

DocumentListWindow::~DocumentListWindow()
{
    delete m_docListDialog;
}

void DocumentListWindow::fetchDocument()
{
    int selectedRow = m_docListDialog->listView->currentIndex().row();
    qDebug() << m_docListDialog->listView->model()->index(selectedRow, 2).data();
    m_gService->downloadDocument(m_docListDialog->listView->model()->index(selectedRow, 1).data().toString(),
                                 m_docListDialog->listView->model()->index(selectedRow, 2).data().toString());
    m_docListDialog->okButton->setEnabled(false);
}

void DocumentListWindow::getClickedDocument( const QModelIndex & index)
{
    Q_UNUSED(index);
}

QString DocumentListWindow::currentDocument()
{
    int selectedRow = m_docListDialog->listView->currentIndex().row();
    QString name  = m_docListDialog->listView->model()->index(selectedRow, 0).data().toString();
    QString type = m_docListDialog->listView->model()->index(selectedRow, 2).data().toString();
    QString ext;

    if (QString::compare(type, "document", Qt::CaseInsensitive) == 0 ) {
            ext = ".odt";
    } else if (QString::compare(type, "spreadsheet", Qt::CaseInsensitive) == 0 ) {
        ext = ".ods";
    } else if (QString::compare(type, "presentation", Qt::CaseInsensitive) == 0 ) {
        ext = ".ppt";
    }

    return (name + ext);
}

void DocumentListWindow::hideDialog()
{
    m_docListDialog->okButton->setEnabled(true);
    hide();
}
