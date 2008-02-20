/* Copyright 2008  Peter Simonsson <peter.simonsson@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "KoExistingDocumentPane.h"

#include "KoFilterManager.h"
#include "KoDocument.h"

#include <kfilewidget.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <kdialog.h>
#include <kdebug.h>

#include <QGridLayout>
#include <QStringList>

KoExistingDocumentPane::KoExistingDocumentPane(QWidget* parent)
    : QWidget(parent)
{
    QGridLayout* layout = new QGridLayout(this);
    layout->setSpacing(KDialog::spacingHint());
    layout->setMargin(0);
    const QStringList mimeFilter = KoFilterManager::mimeFilter(KoDocument::readNativeFormatMimeType(),
            KoFilterManager::Import, KoDocument::readExtraNativeMimeTypes());

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

#include "KoExistingDocumentPane.moc"
