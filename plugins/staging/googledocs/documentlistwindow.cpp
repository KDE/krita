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
#include <QtGui/QListWidgetItem>

#include "googledocumentservice.h"
#include "googledocumentlist.h"
#include "googledocument.h"

DocumentListWindow::DocumentListWindow(GoogleDocumentService *service, QList<GoogleDocument *> & gList)
        : m_docListDialog(new Ui_ListDialog),
          m_gService(service)
{
    m_docListDialog->setupUi(this);
    connect(m_docListDialog->listWidget, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(getClickedDocument(QListWidgetItem *)));
    connect(m_docListDialog->okButton, SIGNAL(clicked()), this, SLOT(fetchDocument()));

    m_documentList = gList;
    foreach (GoogleDocument *doc, m_documentList) {
        m_docListDialog->listWidget->addItem(doc->title());
    }

    show();
}

DocumentListWindow::~DocumentListWindow()
{
    delete m_docListDialog;
}

void DocumentListWindow::fetchDocument()
{
    int index = m_docListDialog->listWidget->currentRow();
    m_gService->downloadDocument(m_documentList[index]);
    m_docListDialog->okButton->setEnabled(false);
}

void DocumentListWindow::getClickedDocument(QListWidgetItem */*item*/)
{
}

QString DocumentListWindow::currentDocument()
{
    int index = m_docListDialog->listWidget->currentRow();
    QString ext = ".odt";

    if (QString::compare(m_documentList[index]->documentType(), "spreadsheet") == 0 )
        ext = ".ods";
    else if (QString::compare(m_documentList[index]->documentType(), "presentation") == 0 )
        ext = ".ppt";

    return (m_documentList[index]->title() + ext);
}

