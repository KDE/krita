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
class QCursor;
class WdgToolCrop;

/**
 * Crop tool
 *
 * TODO: - crop from selection -- i.e, set crop outline to the exact bounds of the selection.
 *       - (when moving to Qt 4: replace rectangle with  darker, dimmer overlay layer
 *         like we have for selections right now)
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
	virtual void doubleClick(KisDoubleClickEvent *);


public slots:
	virtual void activate();
private:
	void clearRect();
	void cropLayer(KisLayerSP layer, QRect rc);
        QRegion handles(QRect rect);
        void paintOutlineWithHandles();
        void paintOutlineWithHandles(QPainter& gc, const QRect& rc);
        Q_INT32 mouseOnHandle (const QPoint currentViewPoint);
        void setMoveResizeCursor (Q_INT32 handle);
	void validateSelection(void);
	void setOptionWidgetStartX(Q_INT32 x);
	void setOptionWidgetStartY(Q_INT32 y);
	void setOptionWidgetEndX(Q_INT32 x);
	void setOptionWidgetEndY(Q_INT32 y);

private slots:

	void crop();
	void setStartX(int x);
	void setStartY(int y);
	void setEndX(int x);
	void setEndY(int y);

private:
	KisCanvasSubject *m_subject;
	QPoint m_startPos;
        QPoint m_endPos;
        bool m_selecting;

	WdgToolCrop* m_optWidget;

        Q_INT32 m_handleSize;
        QRegion m_handlesRegion;
        bool m_haveCropSelection;
        Q_INT32 m_dx, m_dy;
        Q_INT32 m_mouseOnHandleType;

        enum handleType
        {
                None = 0,
                UpperLeft = 1,
                UpperRight = 2,
                LowerLeft = 3,
                LowerRight = 4
        };
};

class KisToolCropFactory : public KisToolFactory {
	typedef KisToolFactory super;
public:
	KisToolCropFactory(KActionCollection * ac ) : super(ac) {};
	virtual ~KisToolCropFactory(){};

	virtual KisTool * createTool() { 
		KisTool * t = new KisToolCrop(); 
		Q_CHECK_PTR(t);
		t -> setup(m_ac); 
		return t;
	}
	virtual KisID id() { return KisID("crop", i18n("Crop tool")); }
};



#endif // KIS_TOOL_CROP_H_

