/*
 *  kis_tool_pen.h - part of KImageShop
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

#ifndef __pentool_h__
#define __pentool_h__

#include <qpen.h>
#include <qpoint.h>
#include "kis_view.h"
#include "kis_canvas.h"
#include "kis_tool.h"

class KisBrush;
class KisDoc;
class KisImageCmd;
class KisFrameBuffer;

class PenTool : public KisTool {
public:
	PenTool(KisDoc *doc, KisCanvas *canvas, KisBrush *brush);
	virtual ~PenTool();
  
	virtual void setupAction(QObject *collection);
	virtual QDomElement saveSettings(QDomDocument& doc) const;
	virtual bool loadSettings(QDomElement& elem);
	virtual void optionsDialog();
	virtual void setBrush(KisBrush *brush);

	virtual void mousePress(QMouseEvent*); 
	virtual void mouseMove(QMouseEvent*);
	virtual void mouseRelease(QMouseEvent*);

	bool paint(const QPoint& pos);

protected:
	KisImageCmd *m_cmd;

	QPoint m_dragStart;
	bool m_dragging;
	float m_dragdist;

	KisFrameBuffer *m_fb;

	// tool options
	int m_lineThickness;
	int m_penColorThreshold;
};

#endif //__pentool_h__

