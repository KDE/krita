/*
 *  dlg_oilpaint.h -- part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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
#ifndef DLG_OILPAINTFILTER
#define DLG_OILPAINTFILTER

#include <qpixmap.h>

#include <kdialogbase.h>

#include "wdg_oilpaintfilter.h"

class DlgOilPaintFilter: public KDialogBase {
	typedef KDialogBase super;
	Q_OBJECT

public:

	DlgOilPaintFilter(QWidget * parent = 0,
			 const char* name = 0);
	~DlgOilPaintFilter();

	void setBrushSize(Q_UINT32 dropSize);
	Q_UINT32 brushSize();
        void setSmooth(Q_UINT32 number);
        Q_UINT32 smooth();
   
private slots:

	void okClicked();
        
private:
	WdgOilPaintFilter * m_page;
	Q_UINT32 m_oldBrushSize;	
	Q_UINT32 m_oldSmooth;
        bool m_lock;

};

#endif // DLG_OILPAINT
