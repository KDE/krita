/*
 *  kis_tool_brush.h - part of KImageShop
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

#ifndef __brushtool_h__
#define __brushtool_h__

#include <qcursor.h>
#include <qpoint.h>

#include "kis_tool.h"

class KToggleAction;

class KisBrush;
class KisDoc;
class KisImageCmd;

class BrushTool : public KisTool {
	typedef KisTool super;

public:
	BrushTool(KisDoc *doc, KisBrush *brush);
	virtual ~BrushTool();
  
	virtual void setupAction(QObject *collection);
	virtual QDomElement saveSettings(QDomDocument& doc) const;
	virtual bool loadSettings(QDomElement& elem);

	virtual void optionsDialog();
	virtual void setBrush(KisBrush *brush);
	
	virtual bool paint(const QPoint& pos);
	bool paintColor(const QPoint& pos);
	bool paintCanvas(const QPoint& pos);

	virtual void mousePress(QMouseEvent*); 
	virtual void mouseMove(QMouseEvent*);
	virtual void mouseRelease(QMouseEvent*);

public slots:
	virtual void toolSelect();

protected:
	virtual QCursor defaultCursor() const;

protected:
	KToggleAction *m_toggle;
	KisImageCmd *m_cmd;
	QPoint m_dragStart;
	bool m_dragging;
	float m_dragdist;
	int m_red; 
	int m_blue; 
	int m_green;
	int m_brushWidth; 
	int m_brushHeight;
	QSize m_brushSize;
	QPoint m_hotSpot;
	int m_hotSpotX; 
	int m_hotSpotY;
	int m_spacing;
	bool m_alpha;
};

#endif //__brushtool_h__

