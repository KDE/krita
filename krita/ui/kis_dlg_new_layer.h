/*
 *  Copyright (c) 2000 Michael Koch <koch@kde.org>
 *  Copyright (c) 2000 Patrick Julien <freak@codepimps.org>
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
#if !defined  KIS_DLG_NEW_LAYER_H_
#define KIS_DLG_NEW_LAYER_H_

class QSpinBox;
#include <kdialogbase.h>

class NewLayerDialog : public KDialogBase {
	typedef KDialogBase super;
	Q_OBJECT

public:
	NewLayerDialog(Q_INT32 maxWidth, Q_INT32 defWidth, Q_INT32 maxHeight, Q_INT32 defHeight, QWidget *parent = 0, const char *name = 0);

	Q_INT32 layerWidth() const { return m_width -> value(); };
	Q_INT32 layerHeight() const { return m_height -> value(); };

private:
	QSpinBox *m_width;
	QSpinBox *m_height;
};

#endif // KIS_DLG_NEW_LAYER_H_

