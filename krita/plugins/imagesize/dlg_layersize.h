/*
 *  dlg_layersize.h -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Sven Langkamp <longamp@reallygood.de>
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
#ifndef DLG_LAYERSIZE
#define DLG_LAYERSIZE

#include <qpixmap.h>

#include <kdialogbase.h>

#include "wdg_layersize.h"

class DlgLayerSize: public KDialogBase {
	typedef KDialogBase super;
	Q_OBJECT

public:

	DlgLayerSize(QWidget * parent = 0,
			 const char* name = 0);
	~DlgLayerSize();

	void setWidth(Q_UINT32 w);
	void setWidthPercent(Q_UINT32 w);
	void setMaximumWidth(Q_UINT32 w);
	Q_INT32 width();
	
	void setHeight(Q_UINT32 h);
	void setHeightPercent(Q_UINT32 h);
	void setMaximumHeight(Q_UINT32 h);
	Q_INT32 height();	
	Q_INT32 filterType();

private slots:

	void okClicked();
	void slotWidthPixelsChanged(int w);
	void slotHeightPixelsChanged(int h);
	void slotWidthPercentChanged(int w);
	void slotHeightPercentChanged(int h);

private:

	void blockAll();
	void unblockAll();

	WdgLayerSize * m_page;
	double m_oldW, m_oldH;
	double m_oldWPercent, m_oldHPercent;
	double m_origW, m_origH;
	double m_maxW, m_maxH;
	
	bool m_lock;

};

#endif // DLG_IMAGESIZE
