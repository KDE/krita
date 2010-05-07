/* Copyright 2008  Peter Simonsson <peter.simonsson@gmail.com>
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

#include "KoExistingDocumentPane.h"

#include <kfilewidget.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kdialog.h>
#include <kdebug.h>

#include <QGridLayout>
#include <QStringList>

KoExistingDocumentPane::KoExistingDocumentPane(QWidget* parent, const QStringList& mimeFilter)
        : QWidget(parent)
{
    QGridLayout* layout = new QGridLayout(this);
    layout->setSpacing(KDialog::spacingHint());
    layout->setMargin(0);

    m_fileWidget = new KFileWidget(KUrl("kfiledialog:///OpenDialog"), this);
    m_fileWidget->setOperationMode(KFileWidget::Opening);
    m_fileWidget->setFilter(mimeFilter.join(" "));
    layout->addWidget(m_fileWidget, 0, 0, 1, -1);

    layout->setColumnStretch(0, 10);

    m_openButton = new KPushButton(i18n("Open Document"), this);
    layout->addWidget(m_openButton, 1, 1);

    connect(m_openButton, SIGNAL(clicked()),
            m_fileWidget, SLOT(slotOk()));
    connect(m_fileWidget, SIGNAL(accepted()),
            this, SLOT(onAccepted()));
}

void KoExistingDocumentPane::onAccepted()
{
    m_fileWidget->accept();
    emit openExistingUrl(m_fileWidget->selectedUrl());
}

#include <KoExistingDocumentPane.moc>
