/* This file is part of the KDE project
 * Copyright (C) 2008 Fredy Yanardi <fyanardi@gmail.com>
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

#include "KoPAMasterPageDialog.h"

#include <QtGui/QListView>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>

#include <KLocale>
#include <KDebug>

#include "KoPADocument.h"
#include "KoPAMasterPage.h"
#include "KoPAPageThumbnailModel.h"

KoPAMasterPageDialog::KoPAMasterPageDialog(KoPADocument *document, KoPAMasterPage *activeMaster, QWidget *parent)
    : KDialog(parent),
    m_document(document)
{
    QSize iconSize(128, 128);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    if(m_document->pageType() == KoPageApp::Slide ) {
        layout->addWidget(new QLabel(i18n("Select a master slide design:"), mainWidget));
        setCaption(i18n("Master Slide"));
    } else {
        layout->addWidget(new QLabel(i18n("Select a master page design:"), mainWidget));
        setCaption(i18n("Master Page"));
    }

    m_listView = new QListView;
    m_listView->setDragDropMode(QListView::NoDragDrop);
    m_listView->setIconSize(iconSize);
    m_listView->setViewMode(QListView::IconMode);
    m_listView->setFlow(QListView::LeftToRight);
    m_listView->setWrapping(true);
    m_listView->setResizeMode(QListView::Adjust);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listView->setMovement(QListView::Static);
    m_listView->setMinimumSize(320, 200);

    m_pageThumbnailModel = new KoPAPageThumbnailModel(m_document->pages(true), m_listView);
    m_pageThumbnailModel->setIconSize(iconSize);
    m_listView->setModel(m_pageThumbnailModel);
    layout->addWidget(m_listView);

    int row = m_document->pageIndex(activeMaster);
    QModelIndex index = m_pageThumbnailModel->index(row, 0);
    m_listView->setCurrentIndex(index);

    connect(m_listView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(selectionChanged()));

    mainWidget->setLayout(layout);
    setMainWidget(mainWidget);
    setModal(true);
    setButtons(Ok|Cancel);
    setDefaultButton(Ok);
}

KoPAMasterPageDialog::~KoPAMasterPageDialog()
{
    // delete m_pageThumbnailModel;
}

KoPAMasterPage *KoPAMasterPageDialog::selectedMasterPage()
{
    QModelIndex index = m_listView->currentIndex();
    KoPAPageBase *page = static_cast<KoPAPageBase *>(index.internalPointer());
    KoPAMasterPage *masterPage = dynamic_cast<KoPAMasterPage *>(page);
    Q_ASSERT(masterPage);
    return masterPage;
}

void KoPAMasterPageDialog::selectionChanged()
{
    // TODO: user shouldn't be able to deselect any item
    enableButtonOk(m_listView->selectionModel()->hasSelection());
}

#include <KoPAMasterPageDialog.moc>

