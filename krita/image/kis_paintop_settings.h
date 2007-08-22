/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_PAINTOP_SETTINGS_H_
#define KIS_PAINTOP_SETTINGS_H_

#include "kis_types.h"
#include <KoPointerEvent.h>
#include <QWidget>
#include "krita_export.h"
#include "kis_layer.h"

/**
 * This class is used to cache the settings (and the associated widget) for a paintop
 * between two creation. There is one KisPaintOpSettings per input device (mouse, tablet, etc...).
 */
class KRITAIMAGE_EXPORT KisPaintOpSettings {

public:
    KisPaintOpSettings() {}
    KisPaintOpSettings(QWidget *parent) { Q_UNUSED(parent); }
    virtual ~KisPaintOpSettings() {}

    /**
     * This function is called by a tool when the mouse is pressed. It's usefull if
     * the paintop needs mouse interaction for instance in the case of the duplicate op.
     * If the tool is supposed to ignore the event, the paint op should call e->accept();
     * and if the tool is supposed to use the event e->ignore(); should be called.
     */
    virtual void mousePressEvent(KoPointerEvent *e);

    /**
     * Call this function when the layer is changed
     */
    virtual void setLayer(KisLayerSP layer )
        {
            m_layer = layer;
        }

    /**
     * @return a pointer to the widget displaying the settings
     */
    virtual QWidget *widget() const { return 0; }
    /**
     * Call this function when the paint op is selected or the tool is activated
     */
    virtual void activate();

protected:

    KisLayerSP m_layer;
};

#endif
