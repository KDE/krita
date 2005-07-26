/*
 *  Copyright (c) 2004 Kivio Team
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_BIRDEYE_BOX_H
#define KIS_BIRDEYE_BOX_H

#include "wdgbirdeye.h"

class QPixmap;

class KisView;
class KisDoc;
class KisImage;
class KAction;
class KoZoomHandler;

class KisBirdEyeBox : public WdgBirdEye
{ 
	Q_OBJECT

public:

	KisBirdEyeBox(QWidget* parent=0, const char* name=0);
	~KisBirdEyeBox();

	bool eventFilter(QObject*, QEvent*);

public slots:

	void show();

	void zoomChanged(int);
	void zoomMinus();
	void zoomPlus();

protected slots:


	void updateVisibleArea();
	void canvasZoomChanged();
/* 	void slotUpdateView(KivioPage*); */
	void togglePageBorder(bool);
	void doAutoResizeMin();
	void doAutoResizeMax();
	void exposureValueChanged(double exposure);
	void exposureSliderPressed();
	void exposureSliderReleased();

signals:
	void exposureChanged(float exposure);

protected:

	void updateView();
	void handleMouseMove(QPoint);
	void handleMouseMoveAction(QPoint);
	void handleMousePress(QPoint);

private:
	KisView* m_view;
	KisDoc* m_doc;

	KAction* m_zoomIn;
	KAction* m_zoomOut;
	QPixmap* m_buffer;

	bool m_showPageBorders;

	QSize m_minSize;
	QSize m_maxSize;

	QRect m_area;
	AlignmentFlags m_apos;
	bool m_handlePress;
	QPoint m_lastPos;

	bool m_draggingExposureSlider;
};

#endif // KIS_BIRDEYE_BOX_H
