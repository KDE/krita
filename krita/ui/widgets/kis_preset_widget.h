/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KIS_PRESET_WIDGET_H
#define KIS_PRESET_WIDGET_H


#include <KoID.h>

#include "widgets/kis_popup_button.h"

class QWidget;
class KisPaintOpPreset;


#include "kis_types.h"

/**
 * Private class that shows the paintop settings preset as a
 * brush stroke and pops down the paintop settings widget
 * when clicked for use in the paintop box.
 */
class KisPresetWidget : public KisPopupButton
{

    Q_OBJECT

public:

    KisPresetWidget(QWidget *parent = 0, const char *name = 0);

public slots:

    void setPreset(KisPaintOpPresetSP preset);
    void setDrawArrow(bool v);
    void updatePreview();

protected:

    virtual void paintEvent(QPaintEvent *);

private:
    QImage m_image;
    KisPaintOpPresetSP m_preset;
    bool m_drawArrow;

};

#endif
