/* This file is part of the KDE project
   Made by Emanuele Tamponi (emanuele@valinor.it)
   Copyright (C) 2007 Emanuele Tamponi

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef MIXERTOOL_H_
#define MIXERTOOL_H_

#include <QPointF>

#include <KoCanvasResourceProvider.h>
#include <KoTool.h>

#include "kis_paint_device.h"

#include "kis_painterly_information.h"

class MixerCanvas;

class MixerTool : public KoTool {
    Q_OBJECT

public:
    MixerTool(MixerCanvas *canvas, KisPaintDevice* device, KoCanvasResourceProvider *rp);
    ~MixerTool();

// Implement KoTool
public:
    void paint(QPainter &, const KoViewConverter &) {} // This is not needed

    void mousePressEvent(KoPointerEvent *e);
    void mouseReleaseEvent(KoPointerEvent *e);
    void mouseMoveEvent(KoPointerEvent *e);

public:
    KisPainterlyInformation bristleInformation() {return m_info;}

private:
    /*
    Update all painterly overlays, and to the mixing. It's called when a non-painterly paintop is used.
    */
    void mixPaint(KisPaintDeviceSP stroke, KoPointerEvent *e);

    /*
    Updates the information of the paintop (color, painterly information for next iteration and such)
    */
    void updateResources(KisPaintDeviceSP stroke);

    /*
    If a painterly paintop is used, it will change the painterly properties of the paint device. We want
    to "restore" the status of the canvas to the better one possible in order to keep the paint in perfect
    shape.
    */
    void preserveProperties(KisPaintDeviceSP stroke);

private:
    KisPaintDevice* m_canvasDev;
    KoCanvasResourceProvider *m_resources;

    QPointF lastPos;

    // We keep a KisPainterlyInformation structure so we store painterly information
    // when we mix with non-painterly paintops.
    KisPainterlyInformation m_info;
};

#endif // MIXERTOOL_H_
