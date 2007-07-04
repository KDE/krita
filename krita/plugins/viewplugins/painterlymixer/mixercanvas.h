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

#ifndef MIXERCANVAS_H_
#define MIXERCANVAS_H_

#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>

#include "kis_paint_device.h"

#include "kis_painterly_information.h"

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

#endif // MIXERCANVAS_H_
