/*
 *  Copyright (c) 2002 Patrick Julien <freak@ideasandassociates.com>
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
#if !defined KIS_DLG_LAYER_PROPERTIES_H_
#define KIS_DLG_LAYER_PROPERTIES_H_

#include <kdialog.h>

class QString;
class QWidget;
class KIntSpinBox;
class KLineEdit;
class KisPaintDevice;
class IntegerWidget;

class KisPaintPropertyDlg : public KDialog {
	typedef KDialog super;

public:
	KisPaintPropertyDlg(const QString& deviceName, int opacity, QWidget *parent = 0, const char *name = 0, WFlags f = 0);
	virtual ~KisPaintPropertyDlg();

	QString getName() const;
	int getOpacity() const;

private:
	KLineEdit *m_name;
	IntegerWidget *m_opacity;
};

#endif // KIS_DLG_LAYER_PROPERTIES_H_

