/*
 *  kis_cmb_imagetype.h - part of KImageShop/Krayon/Krita
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

#ifndef KIS_CMB_IMAGETYPE_H_
#define KIS_CMB_IMAGETYPE_H_

#include "qcombobox.h"

#include "kis_global.h"

/**
 * A combobox filled with the various image types defined in kis_global.
 */

class KisCmbImageType : public QComboBox
{
	typedef QComboBox super;

	Q_OBJECT

 public: 

	KisCmbImageType(QWidget * parent = 0, const char * name = 0 );
	virtual ~KisCmbImageType();

};
#endif
