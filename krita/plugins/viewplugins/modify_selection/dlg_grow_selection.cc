/*
 *  dlg_grow_selection.cc - part of Krita
 *
 *  Copyright (c) 2006 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#include <config.h>

#include <math.h>

#include <iostream>

using namespace std;

#include <qradiobutton.h>
#include <qcheckbox.h>
#include <q3buttongroup.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <klocale.h>
#include <knuminput.h>
#include <kdebug.h>

#include "dlg_grow_selection.h"
#include "wdg_grow_selection.h"

DlgGrowSelection::DlgGrowSelection( QWidget *  parent, const char * name) : super (parent, name, true, i18n("Grow Selection"), Ok | Cancel, Ok)
{
    m_page = new WdgGrowSelection(this, "grow_selection");
    Q_CHECK_PTR(m_page);

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    connect(this, SIGNAL(okClicked()), this, SLOT(okClicked()));
}

DlgGrowSelection::~DlgGrowSelection()
{
    delete m_page;
}

qint32 DlgGrowSelection::xradius()
{
    return m_page->radiusSpinBox->value();
}

qint32 DlgGrowSelection::yradius()
{
    return m_page->radiusSpinBox->value();
}


// SLOTS

void DlgGrowSelection::okClicked()
{
    accept();
}

#include "dlg_grow_selection.moc"
