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
#ifndef KIS_DLG_CREATE_IMG_H_
#define KIS_DLG_CREATE_IMG_H_

#include <qspinbox.h>

#include <kdialogbase.h>

#include <koColor.h>

#include "kis_global.h"
#include "dialogs/wdgnewimage.h"

class QButtonGroup;


class KisDlgCreateImg : public KDialogBase {
	typedef KDialogBase super;
	Q_OBJECT

public:
	KisDlgCreateImg(Q_INT32 maxWidth, Q_INT32 defWidth, 
			Q_INT32 maxHeight, Q_INT32 defHeight, 
			QString colorStrategyName, 
			QString imageName,
			QWidget *parent = 0, const char *name = 0);
	virtual ~KisDlgCreateImg();

public:
	KoColor backgroundColor() const;
	QUANTUM backgroundOpacity() const;
	QString colorStrategyName() const;
	Q_INT32 imgWidth() const;
	Q_INT32 imgHeight() const;
	QString imgName() const;
	double imgResolution() const;
	QString imgDescription() const;
	KisProfileSP profile() const;

private slots:

	void fillCmbProfiles(const QString &);

private:

	WdgNewImage * m_page;
};



#endif // KIS_DLG_CREATE_IMG_H_

