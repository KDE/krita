/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
                 2000, 2001 Werner Trobin <trobin@kde.org>
   Copyright (C) 2004 Nicolas Goutte <goutte@kde.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>

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

#include "KoFilterManager.h"
#include "KoFilterManager_p.h"

#include <QVBoxLayout>
#include <QListWidget>

#include <KLocale>
#include <KSqueezedTextLabel>
#include <KMimeType>

#include <unistd.h>

KoFilterChooser::KoFilterChooser(QWidget *parent, const QStringList &mimeTypes, const QString &nativeFormat, const KUrl &url)
        : KDialog(parent),
        m_mimeTypes(mimeTypes)
{
    setObjectName("kofilterchooser");
    setInitialSize(QSize(300, 350));
    setButtons(KDialog::Ok|KDialog::Cancel);
    setDefaultButton(KDialog::Ok);
    setCaption(i18n("Choose Filter"));
    setModal(true);

    QWidget *page = new QWidget(this);
    setMainWidget(page);

    QVBoxLayout *layout = new QVBoxLayout(page);
    if (url.isValid()) {
        KSqueezedTextLabel *l = new KSqueezedTextLabel(url.path(), page);
        layout->addWidget(l);
    }
    m_filterList = new QListWidget(page);
    layout->addWidget(m_filterList);
    page->setLayout(layout);

    Q_ASSERT(!m_mimeTypes.isEmpty());
    for (QStringList::ConstIterator it = m_mimeTypes.constBegin();
            it != m_mimeTypes.constEnd();
            it++) {
        KMimeType::Ptr mime = KMimeType::mimeType(*it);
        const QString name = mime ? mime->comment() : *it;
        if (! name.isEmpty())
            m_filterList->addItem(name);
    }

    if (nativeFormat == "application/x-kword") {
        const int index = m_mimeTypes.indexOf("text/plain");
        if (index > -1)
            m_filterList->setCurrentRow(index);
    }

    if (m_filterList->currentRow() == -1)
        m_filterList->setCurrentRow(0);

    m_filterList->setFocus();

    connect(m_filterList, SIGNAL(selected(int)), this, SLOT(accept()));
    resize(QSize(520, 400));//.expandedTo(minimumSizeHint()));
}

KoFilterChooser::~KoFilterChooser()
{
}

QString KoFilterChooser::filterSelected()
{
    const int item = m_filterList->currentRow();

    if (item > -1)
        return m_mimeTypes [item];
    else
        return QString();
}

#include <KoFilterManager_p.moc>
