/*
 *  kis_tool_eraser.h - part of KImageShop
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

#ifndef __erasertool_h__
#define __erasertool_h__

#include <qpoint.h>

#include "kis_tool.h"

class KisBrush;
class KisDoc;

class EraserTool : public KisTool {
public:
	EraserTool(KisDoc *doc, KisBrush *brush);
	virtual ~EraserTool();
  
	virtual void setupAction(QObject *collection);
	virtual QDomElement saveSettings(QDomDocument& doc) const;
	virtual bool loadSettings(QDomElement& elem);

	virtual void optionsDialog();

	virtual void mousePress(QMouseEvent*); 
	virtual void mouseMove(QMouseEvent*);
	virtual void mouseRelease(QMouseEvent*);

	void setBrush(KisBrush *_brush);
	bool paint(QPoint pos);

protected:
	QPoint  m_dragStart;
	bool    m_dragging;
	float   m_dragdist;
};

#endif
