/*
 *  kis_tool_rectangle.h - part of KImageShop^WKrayon^WKrita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#ifndef __KIS_TOOL_RECTANGLE_H__
#define __KIS_TOOL_RECTANGLE_H__

#include <qpoint.h>
#include <qrect.h>

#include "kis_tool.h"
#include "kis_tool_paint.h"

class QPainter;
class KisPainter;

class KisToolRectangle : public KisToolPaint {

	typedef KisToolPaint super;
	Q_OBJECT

public:
	KisToolRectangle();
	virtual ~KisToolRectangle();

// 	virtual QString settingsName() const;
// 	virtual QDomElement saveSettings(QDomDocument& doc) const;
// 	virtual bool loadSettings(QDomElement& elem);

	virtual void setup(KActionCollection *collection);
// 	virtual void optionsDialog();

	virtual void mousePress(QMouseEvent *event);
	virtual void mouseMove(QMouseEvent *event);
	virtual void mouseRelease(QMouseEvent *event);

public slots:
//	virtual void toolSelect();
    
protected:
	virtual void draw(const QPoint&, const QPoint&);
	virtual void draw(KisPainter *gc, const QRect& rc);

protected:
	int m_lineThickness;

	QPoint m_dragStart;
	QPoint m_dragEnd;
	QRect m_final_lines;

	bool m_dragging;
private:
	KisCanvasSubject *m_subject;
	
};

#endif // __KIS_TOOL_RECTANGLE_H__

