/* This file is part of the KDE project
 * 
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_WET_PALETTE_WIDGET_H
#define KIS_WET_PALETTE_WIDGET_H

#include "qwidget.h"

#include "kis_canvas_subject.h"
#include "kis_canvas_observer.h"

#include <koffice_export.h>

class KoFrameButton;
class QGridLayout;
class QColor;
class KoColorSlider;
class QLabel;
class QSpinBox;
class KDualColorButton;
class KoColorSlider;
class QColor;

class KRITAUI_EXPORT KisWetPaletteWidget
	 : public QWidget,
	   public KisCanvasObserver
{
	Q_OBJECT
	typedef QWidget super;

public:
	KisWetPaletteWidget(QWidget *parent = 0L, const char *name = 0);
	virtual ~KisWetPaletteWidget() {}

protected slots:

	void slotFGColorSelected(const QColor& c);
	void slotBGColorSelected(const QColor& c);

private:
	void update(KisCanvasSubject*);

private:
	KisCanvasSubject *m_subject;

	QColor m_fgColor;
	QColor m_bgColor;
};

#endif
