/*
 *  kis_dlg_transform.cc - part of Krita
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include <qwmatrix.h>

#include <klocale.h>

#include "dialogs/wdgmatrix.h"
#include "kis_dlg_transform.h"

/**
   This is a temporary dialog that allows the user to enter the
   parameters that define a rotation,scaling, shearing or a raw
   QWMatrix.
*/

KisDlgTransform::KisDlgTransform( QWidget *  parent,
				  const char * name)
	: super (parent, name, true, "", Ok | Cancel)
{
	m_page = new WdgMatrix(this);
	setCaption(i18n("Transform Current Layer"));
	setMainWidget(m_page);
	resize(m_page -> sizeHint());
}

QWMatrix & KisDlgTransform::matrix()
{
	delete m_page;
}

#include "kis_dlg_transform.moc"

