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
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qslider.h>
#include <qstring.h>

#include <klineedit.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <qvbox.h>

#include "kis_dlg_paint_properties.h"
#include "integerwidget.h"

KisPaintPropertyDlg::KisPaintPropertyDlg(const QString& deviceName, int opacity, QWidget *parent, const char *name, WFlags f)
    : super(parent, name, f, "", Ok | Cancel)
{
	QLabel *lbl;
        QVBox *page = makeVBoxMainWidget();

	m_name = new KLineEdit(deviceName, page);

	lbl = new QLabel( m_name, i18n("Name:"), page);

	m_opacity = new IntegerWidget(0, 100, page);
	m_opacity -> setValue(opacity * 100 / USHRT_MAX);
	m_opacity -> setTickmarks(QSlider::Below);
	m_opacity -> setTickInterval(13);

	lbl = new QLabel(m_opacity, i18n("Opacity:"), page);
}

KisPaintPropertyDlg::~KisPaintPropertyDlg()
{
}

QString KisPaintPropertyDlg::getName() const
{
	return m_name -> text();
}

int KisPaintPropertyDlg::getOpacity() const
{
	return m_opacity -> value() * USHRT_MAX / 100;
}

