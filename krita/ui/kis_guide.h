/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000-2001 theKompany.com & Dave Marotti
 * Copyright (C) 2002 Patrick Julien <freak@codepimps.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KIS_GUIDE_H_
#define KIS_GUIDE_H_

#include <qdom.h>
#include <qnamespace.h>
#include <qpixmap.h>
#include <qsize.h>
#include <qvaluelist.h>
#include <qwidget.h>
#include <ksharedptr.h>

class KisGuide : public KShared {
public:
	KisGuide(Qt::Orientation o);
	virtual ~KisGuide();

	Qt::Orientation orientation() { return orient; }
	double position(){ return pos; }
	bool isSelected() { return selected; }

public:
	double pos;
	QPixmap buffer;
	bool hasBuffer;
	bool selected;
	Qt::Orientation orient;
};

typedef KSharedPtr<KisGuide> KisGuideSP;

class KisGuideMgr {
	typedef QValueList<KisGuideSP> vKisGuideSP;
	typedef vKisGuideSP::iterator vKisGuideSP_it;
	typedef vKisGuideSP::const_iterator vKisGuideSP_cit;

public:
	KisGuideMgr();
	~KisGuideMgr();

public:
	KisGuideSP add(double pos, Qt::Orientation o);
	void remove(KisGuideSP gd);
	KisGuideSP find(double x, double y, double d);
	KisGuideSP findHorizontal(double y, double d);
	KisGuideSP findVertical(double x, double d);
	bool hasSelected() const;
	Q_INT32 selectedCount() const;

	void save(QDomElement& element);
	void load(const QDomElement& element);

	void select(KisGuideSP gd);
	void unselect(KisGuideSP gd);
	void selectAll();
	void unselectAll();
	void removeSelected();

	void resize(const QSize& size);
	void resize();
	void erase(QPaintDevice *device, QWidget *w, Q_INT32 xOffset, Q_INT32 yOffset, double zoom);
	void paint(QPaintDevice *device, QWidget *w, Q_INT32 xOffset, Q_INT32 yOffset, double zoom);
	void moveSelectedByX(double d);
	void moveSelectedByY(double d);

private:
	void resizeLinesPixmap(const QSize& size, QPixmap *vLine, QPixmap *hLine, QPixmap *linePattern);

private:
	QSize m_size;
	QPixmap m_vGuideLines;
	QPixmap m_hGuideLines;
	QPixmap m_pattern;
	QPixmap m_vGuideLinesSelected;
	QPixmap m_hGuideLinesSelected;
	QPixmap m_patternSelected;
	vKisGuideSP m_lines;
	vKisGuideSP m_slines;

private:
	static const char *s_xbm[];
	static const char *s_xbm_selected[];
};

#endif // KIS_GUIDE_H_

