/*
 *  kis_tool_fill.h - part of Krayon
 *
 *  Copyright (c) 2000 John Califf
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

#ifndef __filltool_h__
#define __filltool_h__

#include <qpoint.h>

#include "kis_view.h"
#include "kis_tool.h"
#include "kis_layer.h"

class KisDoc;

struct fillinfo
{
   int left;
   int right;
   int top;
   int bottom;
   unsigned char o_r, o_g, o_b, r, g, b;
};

struct fillpixelinfo
{
   int y, xl, xr, dy;
};

class FillTool : public KisTool {
public:
	FillTool(KisDoc *doc);
	virtual ~FillTool();
  
	virtual void setupAction(QObject *collection);
	virtual QDomElement saveSettings(QDomDocument& doc) const;
	virtual bool loadSettings(QDomElement& elem);
	virtual void optionsDialog();

	virtual void mousePress(QMouseEvent*); 

	// set custom cursor.
	virtual void setCursor();
    
	bool flood(int startX, int startY);
      
protected:
	// from gpaint
	int is_old_pixel_value(struct fillinfo *info, int x, int y);  
	void set_new_pixel_value(struct fillinfo *info, int x, int y);      
	void flood_fill(struct fillinfo *info, int x, int y);
	void seed_flood_fill(int x, int y, const QRect& frect);    

	// new colors (desired)
	int nRed;
	int nGreen;
	int nBlue;

	// source colors (existing)
	int sRed;
	int sGreen;
	int sBlue;

	bool layerAlpha;

	int toleranceRed;
	int toleranceGreen;    
	int toleranceBlue;

	KisLayer *fLayer;
};

#endif //__filltool_h__

