/*
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KIS_DLG_PAINTOFFSET_H_
#define KIS_DLG_PAINTOFFSET_H_

#include <kdialogbase.h>

class KisDlgPaintOffset : public KDialogBase {
	Q_OBJECT
	typedef KDialogBase super;

public:
	KisDlgPaintOffset(Q_INT32 xoff, Q_INT32 yoff, QWidget *parent, const char *name);
	virtual ~KisDlgPaintOffset();

public:
	Q_INT32 xoff() const;
	Q_INT32 yoff() const;

private slots:
	void xOffsetValue(int value);
	void yOffsetValue(int value);

private:
	Q_INT32 m_xoff;
	Q_INT32 m_yoff;
};

#endif // KIS_DLG_PAINTOFFSET_H_

