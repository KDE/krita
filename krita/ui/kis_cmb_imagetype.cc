/*
 *  kis_cmb_imageType.cc - part of KImageShop/Krayon/Krita
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
#include <kdebug.h>

#include "kis_cmb_imagetype.h"
#include "kis_colorspace_registry.h"

KisCmbImageType::KisCmbImageType(QWidget * parent, const char * name) 
	: super( false, parent, name )
{
	insertStringList(KisColorSpaceRegistry::singleton()->listColorSpaceNames());
}

KisCmbImageType::~KisCmbImageType()
{
}


#include "kis_cmb_imagetype.moc"

