/*
 *  kis_tool_rectangle.h - part of KImageShop^WKrayon^WKrita
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
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
#include "kis_types.h"

class QPainter;
class KisPainter;

class KisToolRectangle : public KisToolPaint {

	typedef KisToolPaint super;
	Q_OBJECT

public:
	KisToolRectangle();
	virtual ~KisToolRectangle();

        //
        // KisCanvasObserver interface
        //

        virtual void update (KisCanvasSubject *subject);

        //
        // KisToolPaint interface
        //

// 	virtual QString settingsName() const;
// 	virtual QDomElement saveSettings(QDomDocument& doc) const;
// 	virtual bool loadSettings(QDomElement& elem);

	virtual void setup(KActionCollection *collection);

	virtual void buttonPress(KisButtonPressEvent *event);
	virtual void move(KisMoveEvent *event);
	virtual void buttonRelease(KisButtonReleaseEvent *event);

public slots:
//	virtual void toolSelect();

protected:
	virtual void draw(const KisPoint&, const KisPoint&);
	//virtual void draw(KisPainter *gc, const QRect& rc);

protected:
	int m_lineThickness;

	KisPoint m_dragStart;
	KisPoint m_dragEnd;
	QRect m_final_lines;

	bool m_dragging;
	KisImageSP m_currentImage;

};

#endif // __KIS_TOOL_RECTANGLE_H__

