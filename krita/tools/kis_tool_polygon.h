/*
 *  kis_tool_polygon.h - part of Krayon
 *
 *  Copyright (c) 2001 Toshitaka Fujioka <fujioka@kde.org>
 *  Copyright (c) 2004 Boudewijn Remot <boud@valdyas.org>
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

#ifndef __KIS_TOOL_POLYGON__
#define __KIS_TOOL_POLYGON__

#include <qpoint.h>

#include "kis_tool.h"
#include "kis_tool_paint.h"

class KisToolPolygon : public KisToolPaint {

	typedef KisToolPaint super;
	Q_OBJECT

public:
	KisToolPolygon();
	virtual ~KisToolPolygon();

	virtual void setup(KActionCollection *collection);
// 	virtual QDomElement saveSettings(QDomDocument& doc) const;
// 	virtual bool loadSettings(QDomElement& elem);

// 	virtual void toolSelect();
// 	virtual void optionsDialog();

	virtual void mousePress(QMouseEvent *event);
	virtual void mouseMove(QMouseEvent *event);
	virtual void mouseRelease(QMouseEvent *event);
    
protected:
	void drawPolygon( const QPoint&, const QPoint& );

private:

	int lineThickness;
	int cornersValue;
	int sharpnessValue;

	bool checkPolygon;
	bool checkConcavePolygon;
        
	QPoint m_dragStart;
	QPoint m_dragEnd;
	QPoint mStart;
	QPoint mFinish;
    
	bool m_dragging;
	bool m_done;
    
	QPointArray drawPoints;

	KisCanvasSubject * m_subject;
};

#endif //__KIS_TOOL_POLYGON__
