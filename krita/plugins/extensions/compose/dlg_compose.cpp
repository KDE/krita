/*
 *  dlg_compose.cc - part of KimageShop^WKrayon^WKrita
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

#include "dlg_compose.h"
#include <QRadioButton>
#include <QCheckBox>
#include <q3buttongroup.h>
#include <QLabel>
#include <QComboBox>
#include <q3button.h>

#include <klocale.h>
#include <knuminput.h>
#include <kis_debug.h>

#include <kis_image.h>

DlgCompose::DlgCompose(KisImageWSP image, QWidget *  parent)
        : KDialog(parent),
        m_image(image)
{
    setCaption(i18n("compose Layer"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_page = new WdgCompose(this);
    Q_CHECK_PTR(m_page);
    setMainWidget(m_page);
    resize(m_page->sizeHint());

    connect(this, SIGNAL(okClicked()),
            this, SLOT(okClicked()));
}

DlgCompose::~DlgCompose()
{
}// SLOTS

void DlgCompose::okClicked()
{
    accept();
}

#include "dlg_compose.moc"
