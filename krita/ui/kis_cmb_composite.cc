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
	: super( false, parent, name )
{
	insertItem(i18n("Normal"));
	insertItem(i18n("In"));
	insertItem(i18n("Out"));
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
#if 0
	insertItem(i18n("Modulate"));
	insertItem(i18n("Threshold"));
#endif
	insertItem(i18n("No composition"));
#if 0
	insertItem(i18n("Darken"));
	insertItem(i18n("Lighten"));
	insertItem(i18n("Hue"));
	insertItem(i18n("Saturate"));
	insertItem(i18n("Colorize"));
	insertItem(i18n("Luminize"));
	insertItem(i18n("Screen"));
	insertItem(i18n("Overlay"));
#endif
	insertItem(i18n("Copy cyan"));
	insertItem(i18n("Copy magenta"));
	insertItem(i18n("Copy yellow"));
	insertItem(i18n("Copy black"));
	
}

KisCmbComposite::~KisCmbComposite()
{
}


#include "kis_cmb_composite.moc"

