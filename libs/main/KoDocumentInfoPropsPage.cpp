/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>
   Copyright (C) 2004, 2010 David Faure <faure@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#include "KoDocumentInfoPropsPage.h"

#include "KoOdfReadStore.h"
#include "KoStore.h"
#include "KoDocumentInfo.h"
#include "KoDocumentInfoDlg.h"
#include <KoXmlReader.h>
#include <ktar.h>
#include <ktemporaryfile.h>

#include <kfilterdev.h>
#include <kdebug.h>

#include <QBuffer>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

class KoDocumentInfoPropsPage::KoDocumentInfoPropsPagePrivate
{
public:
    KoDocumentInfo *m_info;
    KoDocumentInfoDlg *m_dlg;
    KUrl m_url;
    KoStore *m_src;
    KoStore *m_dst;

    const KArchiveFile *m_docInfoFile;
};

KoDocumentInfoPropsPage::KoDocumentInfoPropsPage(KPropertiesDialog *props,
        const QVariantList &)
        : KPropertiesDialogPlugin(props)
        , d(new KoDocumentInfoPropsPagePrivate)
{
    d->m_info = new KoDocumentInfo(this);
    d->m_url = props->item().url();
    d->m_dlg = 0;

    if (!d->m_url.isLocalFile())
        return;

    d->m_dst = 0;

    d->m_src = KoStore::createStore(d->m_url.toLocalFile(), KoStore::Read);

    if (d->m_src->bad()) {
        return; // the store will be deleted in the dtor
    }

    // OASIS/OOo file format?
    if (d->m_src->hasFile("meta.xml")) {
        KoXmlDocument metaDoc;
        KoOdfReadStore oasisStore(d->m_src);
        QString lastErrorMessage;
        if (oasisStore.loadAndParse("meta.xml", metaDoc, lastErrorMessage)) {
            d->m_info->loadOasis(metaDoc);
        }
    }
    // Old calligra file format?
    else if (d->m_src->hasFile("documentinfo.xml")) {
        if (d->m_src->open("documentinfo.xml")) {
            KoXmlDocument doc;
            if (doc.setContent(d->m_src->device()))
                d->m_info->load(doc);
        }
    }

    d->m_dlg = new KoDocumentInfoDlg(props, d->m_info);
    d->m_dlg->setReadOnly(true);
    // "Steal" the pages from the document info dialog
    Q_FOREACH(KPageWidgetItem* page, d->m_dlg->pages()) {
        KPageWidgetItem* myPage = new KPageWidgetItem(page->widget(), page->header());
        myPage->setIcon(page->icon());
        props->addPage(myPage);
    }
}

KoDocumentInfoPropsPage::~KoDocumentInfoPropsPage()
{
    delete d->m_info;
    delete d->m_src;
    delete d->m_dst;
    delete d->m_dlg;
    delete d;
}

void KoDocumentInfoPropsPage::applyChanges()
{
    // Unused in Calligra
}

#include <KoDocumentInfoPropsPage.moc>
