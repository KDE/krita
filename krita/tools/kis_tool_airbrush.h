/*
 *  kis_tool_airbrush.h - part of KImageShop
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __airbrushtool_h__
#define __airbrushtool_h__

#include <qpoint.h>
#include <qpointarray.h>

#include "kis_tool.h"

class KisBrush;
class KisDoc;

class AirBrushTool : public KisTool {
	Q_OBJECT
    
public:
	AirBrushTool(KisDoc *doc, KisBrush *brush);
	virtual ~AirBrushTool();
  
	virtual void setupAction(QObject *collection);
	virtual QDomElement saveSettings(QDomDocument& doc) const;
	virtual bool loadSettings(QDomElement& elem);

	virtual void setBrush(KisBrush *brush);
	virtual void optionsDialog();
    
	virtual void mousePress(QMouseEvent*); 
	virtual void mouseMove(QMouseEvent*);
	virtual void mouseRelease(QMouseEvent*);

	bool paint(QPoint pos, bool timeout);

protected slots:
	void timeoutPaint();  

protected:
	QTimer *m_timer;
	QMemArray<int> m_brushArray; // array of points in brush
	int nPoints;  // number of points marked in array
    
	QPoint  pos; 
	QPoint 	m_dragStart;
	bool   	m_dragging;
	float   m_dragdist;
	int     density; 

	unsigned int brushWidth;
	unsigned int brushHeight;
};

#endif //__airbrushtool_h__
