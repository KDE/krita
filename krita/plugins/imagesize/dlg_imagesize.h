/*
 *  dlg_imagesize.h -- part of KimageShop^WKrayon^WKrita
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 */
#if !defined DLG_IMAGESIZE
#define DLG_IMAGESIZE

#include <qpixmap.h>

#include <kdialogbase.h>

#include "wdg_imagesize.h"


/**
 * This dialog allows the user to create a selection mask based
 * on a (range of) colors.
 */
class DlgImageSize: public KDialogBase {
	typedef KDialogBase super;
	Q_OBJECT

public:

	DlgImageSize(QWidget * parent = 0,
			 const char* name = 0);
	~DlgImageSize();

	void setWidth(Q_UINT32 w);
	void setMaximumWidth(Q_UINT32 w);
	Q_INT32 width();
	
	void setHeight(Q_UINT32 h);
	void setMaximumHeight(Q_UINT32 h);
	Q_INT32 height();
	
	void setXRes(double x);
	void setYRes(double y);
	
	bool scale();
       

private slots:

	void okClicked();
	void slotWidthChanged(int w);
	void slotHeightChanged(int h);
	void slotScaleTypeWChanged(int i);
	void slotScaleTypeHChanged(int i);
	void slotConstrainToggled(bool b);
private:

	void blockAll();
	void unblockAll();

	WdgImageSize * m_page;
	double m_oldW, m_oldH;   // always in pixels
	double m_origW, m_origH; // always in pixels
	double m_maxW, m_maxH;   // always in pixels
	
	bool m_lock;

};

#endif // DLG_IMAGESIZE
