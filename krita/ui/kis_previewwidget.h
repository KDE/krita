/*
 *  kis_previewwidget.h - part of Krita
 *
 *  Copyright (c) 2001 John Califf  <jcaliff@compuzone.net>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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

#ifndef __kis_previewwidget_h__
#define __kis_previewwidget_h__

#include <qwidget.h>
#include <qpixmap.h>

#include <kpixmapio.h>

#include "kis_types.h"

#include "dialogs/kis_previewwidgetbase.h"

class KisUndoAdapter;

/**
 * A widget that can be used by plugins to show a preview of the effect of the
 * plugin to the user. It would be nice if every plugin that needs to show a preview
 * (maybe not those that create a new image) would use this. This can prevent the distracting
 * effect the GIMP has with a different preview for almost every filter.
 */
class KisPreviewWidget : public PreviewWidgetBase
{
	Q_OBJECT

public:
	/** Constructs the widget */

	KisPreviewWidget( QWidget* parent = 0, const char* name = 0 );

	/** @return the layer, so the dialog can apply its effect on it. */
	KisLayerSP getLayer();

	/**
	 * returns the zoom factor. This could be useful if the filter has to rely on
	 * the whole layer. With this and getPos(), there is enough information to
	 * paint the preview from a source different from the layer in @see getLayer */
	double getZoom() { return m_zoom; }

	/** returns the 'vector' the image in the preview has been moved by. @see getZoom */
	QPoint getPos() { return m_startDrag; }
    

public slots:

	/** Sets the preview to use the layer specified as argument */
	void slotSetPreview(KisLayerSP lay);

	/** 
	 * This should be called at the beginning of the effect. This ensures that 
	 * the layer in the preview widget is in the right state. */
	void slotRenewLayer();

	/**
	 * Call this when the effect has finished updating the layer. Makes the preview
	 * repaint itself. */
	void slotUpdate();

signals:
	/** This is emitted when the position or zoom factor of the widget has changed */
	void updated();
  
protected:
	virtual void paintEvent(QPaintEvent*);
	virtual void mousePressEvent(QMouseEvent * e);
	virtual void mouseMoveEvent(QMouseEvent * e);
	virtual void mouseReleaseEvent(QMouseEvent * e);

private:

	void render(QPainter &painter, KisImageSP image, double zoomX, double zoomY);
	void updateWidgets(QPoint delta);
	KisLayerSP m_layer;
	KisImageSP m_original, m_preview;
	QPoint m_startDrag, m_pos;
	KisUndoAdapter* m_undo;
	KisLayerSP layerNew1 /*original image*/, layerNew2/*preview image*/;
	bool m_moved;
	double m_zoom;

private slots:

	void zoomIn();
	void zoomOut();

private:
	QPixmap m_pixmap;
	KPixmapIO m_pixio;
};

#endif
