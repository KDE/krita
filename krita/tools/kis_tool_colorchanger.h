/*
 *  kis_tool_colorchanger.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf
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

#ifndef __KIS_TOOL_COLORCHANGER_H__
#define __KIS_TOOL_COLORCHANGER_H__

#include <qpoint.h>

#include "kis_view.h"
#include "kis_tool.h"
#include "kis_tool_paint.h"
#include "kis_layer.h"


class KisToolColorChanger : public KisToolPaint {

	typedef KisToolPaint super;
	Q_OBJECT

public:
	KisToolColorChanger();
	virtual ~KisToolColorChanger();
  
	virtual void setup(KActionCollection *collection);

// 	virtual QDomElement saveSettings(QDomDocument& doc) const;
// 	virtual bool loadSettings(QDomElement& elem);
// 	virtual void setCursor();

	virtual void mousePress(QMouseEvent*); 

	bool changeColors(int startx, int starty);
      
protected:
                
	// new colors (desired)
	int nRed;
	int nGreen;
	int nBlue;

	// source colors (existing)

	int fillOpacity;
	bool layerAlpha;

	int toleranceRed;
	int toleranceGreen;    
	int toleranceBlue;

	KisCanvasSubject * m_subject;
};

#endif //__KIS_TOOL_COLORCHANGER_H__

