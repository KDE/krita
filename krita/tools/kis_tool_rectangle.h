/*
 *  gradienttool.h - part of KImageShop
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
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

#ifndef __rectangletool_h__
#define __rectangletool_h__

#include <qpoint.h>
#include <qrect.h>

#include "kis_tool.h"

class QPainter;
class KisDoc;
class KisView;
class KisCanvas;

class RectangleTool : public KisTool {
public:
	RectangleTool(KisDoc *doc, KisCanvas *canvas);
	virtual ~RectangleTool();

	virtual void optionsDialog();
	virtual QDomElement saveSettings(QDomDocument& doc) const;
	virtual bool loadSettings(QDomElement& elem);

	virtual void setupAction(QObject *collection);

	virtual void mousePress( QMouseEvent* event );
	virtual void mouseMove( QMouseEvent* event );
	virtual void mouseRelease( QMouseEvent* event );

public slots:
	virtual void toolSelect();
    
protected:
	void drawRectangle(const QPoint&, const QPoint&);

protected:
	int m_lineThickness;

	QPoint m_dragStart;
	QPoint m_dragEnd;
	QRect m_final_lines;

	bool m_dragging;
};

#endif //__linetool_h__

