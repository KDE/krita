/*
 *  kis_cmb_composite.cc - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qcombobox.h>

#include <klocale.h>

#include "kis_cmb_composite.h"

KisCmbComposite::KisCmbComposite(QWidget * parent, const char * name) 
	: QComboBox( parent, name )
{

	insertItem(i18n("Over"));
	insertItem(i18n("In"));
	insertItem(i18n("Atop"));
	insertItem(i18n("Xor"));
	insertItem(i18n("Plus"));
	insertItem(i18n("Minus"));
	insertItem(i18n("Add"));
	insertItem(i18n("Subtract"));
	insertItem(i18n("Diff"));
	insertItem(i18n("Mult"));
	insertItem(i18n("Bumpmap"));
	insertItem(i18n("Copy"));
	insertItem(i18n("Copy red"));
	insertItem(i18n("Copy green"));
	insertItem(i18n("Copy blue"));
	insertItem(i18n("Copy opacity"));
	insertItem(i18n("Clear"));
	insertItem(i18n("Dissolve"));
	insertItem(i18n("Displace"));
	insertItem(i18n("Modulate"));
	insertItem(i18n("Threshold"));
	insertItem(i18n("Modulate"));
	
}

KisCmbComposite::~KisCmbComposite()
{
}

#include "kis_cmb_composite.moc"

