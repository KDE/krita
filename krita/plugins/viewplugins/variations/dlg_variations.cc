/*
 *  dlg_variations.cc - part of KimageShop^WKrayon^WKrita
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

#include "dlg_variations.h"
#include "wdg_variations.h"
//Added by qt3to4:
#include <QPixmap>


DlgVariations::DlgVariations( QWidget *  parent,
                    const char * name)
    : super (parent, name, true, i18n("Color Range"), Ok | Cancel, Ok)
{
    m_previewPix = QPixmap();
    m_page = new WdgVariations(this, "variations");
    Q_CHECK_PTR(m_page);
    setCaption(i18n("Variations"));
    setMainWidget(m_page);
    resize(m_page -> size());

    connect(this, SIGNAL(okClicked()),
        this, SLOT(okClicked()));
}

DlgVariations::~DlgVariations()
{
    delete m_page;
}

void DlgVariations::setPixmap(QPixmap pix) 
{
    m_previewPix = pix;
    m_previewPix.detach();
}

void DlgVariations::okClicked()
{
    accept();
}

#include "dlg_variations.moc"
