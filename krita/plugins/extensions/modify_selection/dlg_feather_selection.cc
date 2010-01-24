/*
 *  dlg_feather_selection.cc - part of Krita
 *
 *  Copyright (c) 2009 Edward Apap <schumifer@hotmail.com>
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

#include "dlg_feather_selection.h"

#include <math.h>

#include <klocale.h>
#include <kis_debug.h>


DlgFeatherSelection::DlgFeatherSelection(QWidget *  parent, const char * name)
        : KDialog(parent)
{
    setCaption(i18n("Feather Selection"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setObjectName(name);
    m_page = new WdgFeatherSelection(this);
    Q_CHECK_PTR(m_page);
    m_page->setObjectName("feather_selection");

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    connect(this, SIGNAL(okClicked()), this, SLOT(okClicked()));
}

DlgFeatherSelection::~DlgFeatherSelection()
{
    delete m_page;
}

qint32 DlgFeatherSelection::radius()
{
    return m_page->radiusSpinBox->value();
}

// SLOTS

void DlgFeatherSelection::okClicked()
{
    accept();
}

#include "dlg_feather_selection.moc"
