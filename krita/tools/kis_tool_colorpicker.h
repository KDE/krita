/*
 *  colorpicker.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __colorpicker_h__
#define __colorpicker_h__

#include <qpoint.h>

#include <koColor.h>

#include "kis_tool.h"

class ColorPicker : public KisTool {
public:
	ColorPicker(KisDoc *doc);
	virtual ~ColorPicker();
  
	KoColor pick(int x, int y);
	virtual void setupAction(QObject *collection);

	virtual void mousePress(QMouseEvent*); 
};

#endif //__colorpicker_h__

