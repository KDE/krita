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

#include <QFrame>
#include <KoCanvasBase.h>

class KoCanvasResourceProvider;
class KoColorSpace;
class KisPaintDevice;
class KisPainterlyOverlay;
class KisPaintOp;
class MixerTool;

class MixerCanvas : public QFrame, public KoCanvasBase {
    Q_OBJECT

public:
    MixerCanvas(QWidget *parent);
    ~MixerCanvas();

    // Initializes the canvas KisPaintDevice, and add overlays to it.
	void setDevice(KoColorSpace *cs);
    KisPaintDevice *device() {return m_device.data();}
    KisPainterlyOverlay *overlay() {return m_overlay.data();}

    void setToolProxy(KoToolProxy *proxy) {m_toolProxy = proxy;}
    KoToolProxy* toolProxy() {return m_toolProxy;}

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
    KoToolProxy *m_toolProxy;

    KisPaintDeviceSP m_device;
	KisPainterlyOverlaySP m_overlay;

};

#endif // MIXERCANVAS_H_
