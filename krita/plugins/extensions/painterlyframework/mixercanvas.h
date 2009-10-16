/* This file is part of the KDE project

   Copyright (C) 2007 Emanuele Tamponi <emanuele@valinor.it>

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
#include <kis_paint_layer.h>
#include <KoCanvasBase.h>

class KisPaintDevice;
class KisCanvasResourceProvider;
class KoColorSpace;
class KoShapeManager;
class KoToolProxy;
class KoViewConverter;
class QImage;
class QMouseEvent;
class QPaintEvent;
class QRectF;
class QRegion;
class QResizeEvent;
class QTabletEvent;
class QUndoCommand;

class MixerCanvas : public QFrame, public KoCanvasBase
{
    Q_OBJECT

public:
    MixerCanvas(QWidget *parent);
    ~MixerCanvas();

    void setResources(KisCanvasResourceProvider *rp);
    void setLayer(const KoColorSpace *cs);

    KisPaintLayer *layer();
    KisPaintDevice *device();
    void setToolProxy(KoToolProxy *proxy);

    KoToolProxy *toolProxy() const {
        return m_toolProxy;
    }

// Events to be redirected to the MixerTool
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void tabletEvent(QTabletEvent *event);
// QFrame events
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);

// Implement KoCanvasBase
public:
    // These methods are not needed.
    void gridSize(qreal *, qreal *) const {
        Q_ASSERT(false);
    }
    bool snapToGrid() const {
        Q_ASSERT(false); return false;
    }
    KoShapeManager *shapeManager() const {
        Q_ASSERT(false); return 0;
    }
    const KoViewConverter *viewConverter() const {
        Q_ASSERT(false); return 0;
    }
    void updateInputMethodInfo() {
        Q_ASSERT(false);
    }
    void updateCanvas(const QRectF &) {}
    // Not needed but defined in the cpp source
    KoUnit unit() const;
    void addCommand(QUndoCommand*);

    QWidget* canvasWidget() {
        return this;
    }
    const QWidget* canvasWidget() const {
        return this;
    }

    void updateCanvas(const QRegion& region);

public slots:
    void slotClear();
    void slotResourceChanged(int key, const QVariant &value);

private slots:

    void checkCurrentPaintop();
    void checkCurrentLayer();

private:
    bool m_tabletPressed;
    KoToolProxy *m_toolProxy;

    KisPaintLayerSP m_layer;

    bool m_dirty;
    QImage m_image;

};

#endif // MIXERCANVAS_H_
