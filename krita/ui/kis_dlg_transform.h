/*
 *  kis_dlg_transform.h - part of Krita
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

#ifndef KIS_DLG_TRANSFORM_H_
#define KIS_DLG_TRANSFORM_H_

#include <qwmatrix.h>

#include <kdialogbase.h>

class WdgMatrix;

/**
   This is a temporary dialog that allows the user to enter the
   parameters that define a rotation,scaling, shearing or a raw
   QWMatrix.
*/

class KisDlgTransform : public KDialogBase {
	typedef KDialogBase super;
	Q_OBJECT

public:

	KisDlgTransform( QWidget *  parent = 0,
			 const char * name = 0);
	virtual ~KisDlgTransform();

private:
	WdgMatrix * m_page;
};

#endif // KIS_DLG_TRANSFORM_H_
