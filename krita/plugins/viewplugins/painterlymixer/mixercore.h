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

#ifndef MIXER_CANVAS_H_
#define MIXER_CANVAS_H_

#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>

#include "kis_paint_device.h"

class QFrame;
class KisPaintOp;
class MixerTool;

class MixerCanvas : public QFrame, public KoCanvasBase {
    Q_OBJECT

public:
    MixerCanvas(QWidget *parent);
    ~MixerCanvas();

    // Initializes the canvas KisPaintDevice, and add overlays to it.
    void initDevice(KoColorSpace *cs, KoCanvasResourceProvider *rp);
    // Initializes the color wells and various spots in the Spots Frame.
    void initSpots(QFrame *sf);

// Events to be redirected to the MixerTool
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void tabletEvent(QTabletEvent *event);
    void paintEvent(QPaintEvent *event);

// Implement KoCanvasBase
public:
    // These methods are not needed.
    void gridSize(double *, double *) const {Q_ASSERT(false);}
    bool snapToGrid() const {Q_ASSERT(false); return false;}
    void addCommand(QUndoCommand *) {Q_ASSERT(false);}
    KoShapeManager *shapeManager() const {Q_ASSERT(false); return 0;}
    KoToolProxy * toolProxy() const {Q_ASSERT(false); return m_toolProxy;}
    const KoViewConverter *viewConverter() const {Q_ASSERT(false); return 0;}
    KoUnit unit() const {Q_ASSERT(false); return KoUnit();}
    void updateInputMethodInfo() {Q_ASSERT(false);}

    QWidget* canvasWidget() {return this;}
    void updateCanvas(const QRectF& rc);

private:
    bool m_tabletPressed;
    MixerTool *m_tool;
    KoToolProxy *m_toolProxy;

    KisPaintDeviceSP m_canvasDev;

};


#include <KoTool.h>

class MixerTool : public KoTool {
    Q_OBJECT

public:
    MixerTool(MixerCanvas *canvas, KisPaintDeviceSP device, KoCanvasResourceProvider *rp);
    ~MixerTool();

// Implement KoTool
public:
    // This is not needed
    void paint(QPainter &, const KoViewConverter &) {}

    void mousePressEvent(KoPointerEvent *e);
    void mouseReleaseEvent(KoPointerEvent *e);
    void mouseMoveEvent(KoPointerEvent *e);

private:
    /*
    Update all the properties that cannot be stored in the KisPaintOp, storing
    them in the mixer. Initialize the properties that the stroke would have in order to mix
    colors on it in a painterly way. It's called inside mouseMoveEvent.
    */
    void updatePainterlyOverlays(KisPaintDeviceSP stroke, KoPointerEvent *e);

    /*
    Merge the canvas contents with the stroke contents, actually mixing the colors.
    */
    void mixColors(KisPaintDeviceSP stroke);

    /*
    Updates the information of the paintop (color, painterly information for next iteration and such)
    */
    void updateResources(KisPaintDeviceSP stroke);

private:
    KisPaintDeviceSP m_canvasDev;
    KoCanvasResourceProvider *m_resources;
};

#endif // MIXER_CANVAS_H_
