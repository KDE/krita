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
#if !defined KIS_DLG_DIMENSION_H_
#define KIS_DLG_DIMENSION_H_

#include <qsize.h>
#include <kdialogbase.h>

class KisDlgDimension : public KDialogBase {
	Q_OBJECT
	typedef KDialogBase super;

public:
	KisDlgDimension(Q_INT32 maxWidth, 
		Q_INT32 defWidth, 
		Q_INT32 maxHeight, 
		Q_INT32 defHeight, 
		QWidget *parent = 0, 
		const char *name = 0);
	virtual ~KisDlgDimension();

public:
	QSize getSize() const;

private slots:
	void widthChanged(int val);
	void heightChanged(int val);

private:
	KisDlgDimension(const KisDlgDimension&);
	KisDlgDimension& operator=(const KisDlgDimension&);

private:
	Q_INT32 m_width;
	Q_INT32 m_height;
};

#endif // KIS_DLG_DIMENSION_H_

