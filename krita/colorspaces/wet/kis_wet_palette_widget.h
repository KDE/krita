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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_WET_PALETTE_WIDGET_H
#define KIS_WET_PALETTE_WIDGET_H

#include <qwidget.h>
#include <qpushbutton.h>

#include <koffice_export.h>

#include "kis_canvas_subject.h"
#include "kis_canvas_observer.h"

class QColor;
class KIntNumInput;
class KDoubleNumInput;

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
    void slotWetnessChanged(int);
    void slotStrengthChanged(double);

private:
    void update(KisCanvasSubject*);

private:
    KisCanvasSubject *m_subject;
    KDoubleNumInput* m_strength;
    KIntNumInput* m_wetness;
};

#endif
