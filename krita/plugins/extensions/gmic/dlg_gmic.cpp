/*
 *  dlg_gmic.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
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

#include "dlg_gmic.h"

#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QColor>

#include <kconfig.h>
#include <klocale.h>
#include <knuminput.h>
#include <kis_debug.h>
#include <kcolorbutton.h>

DlgGmic::DlgGmic(const QString & /*imageCS*/,
                 const QString & /*layerCS*/,
                 QWidget *  parent,
                 const char * name)
    : KDialog(parent)
{
    setCaption(i18n("Apply G'Mic Action"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setObjectName(name);
    m_page = new WdgGmic(this, "gmic");
    Q_CHECK_PTR(m_page);
    setMainWidget(m_page);
    resize(m_page->sizeHint());

    connect(this, SIGNAL(okClicked()),
            this, SLOT(okClicked()));
}

DlgGmic::~DlgGmic()
{
    delete m_page;
}

QString DlgGmic::gmicScript()
{
    return m_page->txtGmic->text();
}

void DlgGmic::okClicked()
{
    accept();
}



#include "dlg_gmic.moc"
