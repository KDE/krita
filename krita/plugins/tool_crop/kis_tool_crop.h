/*
 *  kis_tool_crop.h - part of Krita
 *
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

#ifndef KIS_TOOL_CROP_H_
#define KIS_TOOL_CROP_H_

#include <qpoint.h>
#include <qregion.h>

#include <kis_tool.h>
#include <kis_tool_non_paint.h>
#include <kis_tool_factory.h>

class QRect;

/**
 * Crop tool
 *
 * TODO: - crop from selection
 *       - crop all layers
 *       - crop single layer
 *       - (when moving to Qt 4: replace rectangle with  darker, dimmer overlay layer
 *         like we have for selections right now)
 *       - Move crop selection with corner markers
 * BUGS: - No undo yet
 */
class KisToolCrop : public KisToolNonPaint {

	typedef KisToolNonPaint super;
	Q_OBJECT

public:
	KisToolCrop();
	virtual ~KisToolCrop();

	virtual void update(KisCanvasSubject *subject);

	virtual QWidget* createOptionWidget(QWidget* parent);
	virtual QWidget* optionWidget();

	virtual void setup(KActionCollection *collection);

	virtual void paint(QPainter& gc);
	virtual void paint(QPainter& gc, const QRect& rc);
	virtual void buttonPress(KisButtonPressEvent *e);
	virtual void move(KisMoveEvent *e);
	virtual void buttonRelease(KisButtonReleaseEvent *e);

private:
	void clearRect();
	void paintOutline();
	void paintOutline(QPainter& gc, const QRect& rc);
	void cropLayer(KisLayerSP layer, QRect rc);
        QRegion handles(QRect rect);
        void paintOutlineWithHandles();
        void paintOutlineWithHandles(QPainter& gc, const QRect& rc);

private slots:

	void crop();
	void setStartX(int x) { m_startPos.setX(x); paintOutline(); }
	void setStartY(int y) { m_startPos.setY(y); paintOutline(); }
	void setEndX(int x) { m_endPos.setX(x); paintOutline(); }
	void setEndY(int y) { m_endPos.setY(y); paintOutline(); }

private:
	KisCanvasSubject *m_subject;
	QPoint m_startPos;
	QPoint m_endPos;
	bool m_selecting;

	QWidget * m_optWidget;
        
        QRegion m_handlesRegion;
};

class KisToolCropFactory : public KisToolFactory {
	typedef KisToolFactory super;
public:
	KisToolCropFactory(KActionCollection * ac ) : super(ac) {};
	virtual ~KisToolCropFactory(){};

	virtual KisTool * createTool() { KisTool * t = new KisToolCrop(); t -> setup(m_ac); return t;}
	virtual KisID id() { return KisID("crop", i18n("Crop tool")); }
};



#endif // KIS_TOOL_CROP_H_

