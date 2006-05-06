/*
 *  dlg_substrate.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <klocale.h>

#include "dlg_substrate.h"
#include "wdgsubstrate.h"


DlgSubstrate::DlgSubstrate( QWidget *  parent,
                    const char * name)
    : super (parent, name, true, i18n("Color Range"), Ok | Cancel, Ok)
{
    m_previewPix = QPixmap();
    m_page = new WdgSubstrate(this, "substrate");
    Q_CHECK_PTR(m_page);
    setCaption(i18n("Substrate"));
    setMainWidget(m_page);
    resize(m_page -> size());

    connect(this, SIGNAL(okClicked()),
        this, SLOT(okClicked()));
}

DlgSubstrate::~DlgSubstrate()
{
    delete m_page;
}

void DlgSubstrate::setPixmap(QPixmap pix)
{
    m_previewPix = pix;
    m_previewPix.detach();
}

void DlgSubstrate::okClicked()
{
    accept();
}

#include "dlg_substrate.moc"

