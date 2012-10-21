/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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

#include "SelectVideoWidget.h"

#include <KLocale>

#include <KFileWidget>
#include <Phonon/BackendCapabilities>

#include <QVBoxLayout>
#include <QCheckBox>

SelectVideoWidget::SelectVideoWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_fileWidget = new KFileWidget(KUrl("kfiledialog:///OpenVideoDialog"), this);
    m_fileWidget->setOperationMode(KFileWidget::Opening);
    m_fileWidget->setMimeFilter(Phonon::BackendCapabilities::availableMimeTypes());
    layout->addWidget(m_fileWidget);

    m_saveEmbedded = new QCheckBox(i18n("Save as part of document"));
    m_fileWidget->setCustomWidget("", m_saveEmbedded);
    setLayout(layout);
}

SelectVideoWidget::~SelectVideoWidget()
{

}

void SelectVideoWidget::accept()
{
    m_fileWidget->slotOk();
    m_fileWidget->accept();
}

void SelectVideoWidget::cancel()
{
    m_fileWidget->slotCancel();
}


KUrl SelectVideoWidget::selectedUrl() const
{
    return m_fileWidget->selectedUrl();
}

bool SelectVideoWidget::saveEmbedded()
{
    return m_saveEmbedded->isChecked();
}
