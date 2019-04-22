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
#ifndef KIS_SHAPE_LAYER_CANVAS_H
#define KIS_SHAPE_LAYER_CANVAS_H

#include <QMutex>
#include <QRegion>
#include <KoCanvasBase.h>

#include <kis_types.h>
#include "kis_thread_safe_signal_compressor.h"
#include <KoSelectedShapesProxy.h>
#include <KoShapeManager.h>
#include <kis_image_view_converter.h>

class KoShapeManager;
class KoToolProxy;
class KoViewConverter;
class KUndo2Command;
class QWidget;
class KoUnit;
class KisImageViewConverter;
class KisSignalCompressor;


class KisShapeLayerCanvasBase : public KoCanvasBase
{

public:
    KisShapeLayerCanvasBase(KisShapeLayer *parent, KisImageWSP image);

    virtual void setImage(KisImageWSP image) = 0;
    void prepareForDestroying();
    virtual void forceRepaint() = 0;
    virtual bool hasPendingUpdates() const = 0;

    bool hasChangedWhileBeingInvisible();
    virtual void rerenderAfterBeingInvisible() = 0;
    virtual void resetCache() = 0;

    KoShapeManager *shapeManager() const override;
    KoViewConverter *viewConverter() const override;

    void gridSize(QPointF *offset, QSizeF *spacing) const override;
    bool snapToGrid() const override;
    void addCommand(KUndo2Command *command) override;
    KoSelectedShapesProxy *selectedShapesProxy() const override;
    KoToolProxy * toolProxy() const override;
    QWidget* canvasWidget() override;
    const QWidget* canvasWidget() const override;
    KoUnit unit() const override;
    void updateInputMethodInfo() override {}
    void setCursor(const QCursor &) override {}

    void setUpdatesBlocked(bool value);
    bool updatesBlocked() const;

protected:
    QScopedPointer<KisImageViewConverter> m_viewConverter;
    QScopedPointer<KoShapeManager> m_shapeManager;
    QScopedPointer<KoSelectedShapesProxy> m_selectedShapesProxy;
    bool m_hasChangedWhileBeingInvisible {false};
    bool m_isDestroying {false};
    bool m_updatesBlocked {false};
};

/**
 * KisShapeLayerCanvas is a special canvas implementation that Krita
 * uses for non-krita shapes to request updates on.
 *
 * Do NOT give this canvas to tools or to the KoCanvasController, it's
 * not made for that.
 */
class KisShapeLayerCanvas : public KisShapeLayerCanvasBase
{
    Q_OBJECT
public:

    KisShapeLayerCanvas(KisShapeLayer *parent, KisImageWSP image);
    ~KisShapeLayerCanvas() override;

    /// This canvas won't render onto a widget, but a projection
    void setProjection(KisPaintDeviceSP projection) {
        m_projection = projection;
    }

    void setImage(KisImageWSP image) override;
    void updateCanvas(const QRectF& rc) override;
    void updateCanvas(const QVector<QRectF> &region);
    void forceRepaint() override;
    bool hasPendingUpdates() const override;

    void resetCache() override;
    void rerenderAfterBeingInvisible() override;

private Q_SLOTS:
    friend class KisRepaintShapeLayerLayerJob;
    void repaint();
    void slotStartAsyncRepaint();
    void slotStartDirectSyncRepaint();
    void slotImageSizeChanged();

Q_SIGNALS:
    void forwardRepaint();

private:
    void updateUpdateCompressorDelay();

private:
    KisPaintDeviceSP m_projection;
    KisShapeLayer *m_parentLayer {0};
    KisSignalCompressor m_canvasUpdateCompressor;

    KisThreadSafeSignalCompressor m_asyncUpdateSignalCompressor;
    volatile bool m_hasUpdateInCompressor = false;
    volatile bool m_hasDirectSyncRepaintInitiated = false;

    QRegion m_dirtyRegion;
    QMutex m_dirtyRegionMutex;

    QRect m_cachedImageRect;

    KisImageWSP m_image;
};

#endif
