/*
 *  kis_tool.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#ifndef __kis_tool_h__
#define __kis_tool_h__

#include <qcursor.h>
#include <qdom.h>
#include <qimage.h>
#include <qobject.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpointarray.h>
#include <qrect.h>

#include <ksharedptr.h>

#include "kis_brush.h"
#include "kis_cursor.h"
#include "kis_pattern.h"

class QEvent;
class QPaintEvent;
class QPainter;
class QMouseEvent;
class KisCanvas;
class KisDoc;
class KisView;

class KisTool : public QObject, public KShared {
Q_OBJECT

public:
	KisTool(KisDoc *doc, const char *name = 0);
	virtual ~KisTool();

	virtual void setupAction(QObject *collection) = 0;

	virtual QString settingsName() const;
	virtual QDomElement saveSettings(QDomDocument& doc) const;
	virtual bool loadSettings(QDomElement& elem);

	virtual void optionsDialog();
	virtual void clearOld();
	virtual bool shouldRepaint() const;
	virtual bool willModify() const;

	virtual QCursor cursor() const;
	virtual void setCursor(const QCursor& cursor);
	virtual void setCursor();

	virtual void setPattern(KisPattern *pattern);
	virtual void setBrush(KisBrush *b);
	virtual void setChecked(bool check);
	virtual bool setClip();
	
	void setSelectCursor();
	void setMoveCursor();

	KisView* getCurrentView();
	const KisView* getCurrentView() const;

public slots:
	virtual void toolSelect();
	virtual void paintEvent(QPaintEvent *e);
	virtual void enterEvent(QEvent *e);
	virtual void leaveEvent(QEvent *e);
	virtual void mousePress(QMouseEvent *e);
	virtual void mouseMove(QMouseEvent *e);
	virtual void mouseRelease(QMouseEvent *e);

protected:
	int zoomed(int n) const;
	QPoint zoomed(const QPoint& point) const;
	int zoomedX(int n) const;
	int zoomedY(int n) const;

	QRect getDrawRect(const QPointArray& points) const;
	QPointArray zoomPointArray(const QPointArray& points) const;

	void setClipImage();
	void dragSelectImage(const QPoint& dragPoint, const QPoint& hotSpot);
	bool pasteClipImage(const QPoint& pos);

protected:
	KisDoc *m_doc;
	KisPattern *m_pattern;
	KisBrush *m_brush;
	KisCanvas *m_canvas;
	QCursor m_cursor;
	QPixmap m_clipPixmap;
	QImage m_clipImage;

	unsigned int m_opacity; 
	bool m_usePattern;
	bool m_useGradient;
	bool m_useRegions;
	bool m_fillSolid;

private:
	KisTool(const KisTool&);
	KisTool& operator=(const KisTool&);

private:
	unsigned int m_paintThreshold;
	bool m_paintWithPattern; 
	bool m_paintWithGradient;
};

#endif

