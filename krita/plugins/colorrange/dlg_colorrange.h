/*
 *  dlg_colorrange.h -- part of KimageShop^WKrayon^WKrita
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
#if !defined DLG_COLORRANGE
#define DLG_COLORRANGE

#include <qpixmap.h>

#include <kdialogbase.h>

#include "wdg_colorrange.h"


/**
 * This dialog allows the user to create a selection mask based
 * on a (range of) colors.
 */
class DlgColorRange: public KDialogBase {
	typedef KDialogBase super;
	Q_OBJECT

public:

	DlgColorRange(QWidget * parent = 0,
			 const char* name = 0);
	~DlgColorRange();

	/**
	 * Set the initial preview pixmap
	 */
	void setPixmap(QPixmap pix);

private slots:

	void okClicked();

private:

	WdgColorRange * m_page;
	QPixmap m_previewPix;
};

#endif // DLG_COLORRANGE
