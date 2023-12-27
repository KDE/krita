/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <KisSafeBlockingQueueConnectionProxy.h>
#include <kis_signal_auto_connection.h>
#include "kis_image_view_converter.h"
#include "kis_default_bounds_base.h"
#include "KoColorConversionTransformation.h"

class KoColorProfile;
class KoShapeManager;
class KoToolProxy;
class KoViewConverter;
class KUndo2Command;
class QWidget;
class KoUnit;
class KisImageViewConverter;
class KoColorSpace;

class KisShapeLayerCanvasBase : public KoCanvasBase
{
    Q_OBJECT
public:
    KisShapeLayerCanvasBase(KisShapeLayer *parent);
    KisShapeLayerCanvasBase(const KisShapeLayerCanvasBase &rhs, KisShapeLayer *parent);

    virtual void setImage(KisImageWSP image);
    void prepareForDestroying();
    virtual void forceRepaint() = 0;
    virtual bool hasPendingUpdates() const = 0;
    virtual KisPaintDeviceSP projection() const = 0;

    virtual void forceRepaintWithHiddenAreas() { forceRepaint(); }

    bool hasChangedWhileBeingInvisible();
    virtual void rerenderAfterBeingInvisible() = 0;
    virtual void resetCache() = 0;

    KoShapeManager *shapeManager() const override;
    const KoViewConverter *viewConverter() const override;
    KoViewConverter *viewConverter() override;

    void gridSize(QPointF *offset, QSizeF *spacing) const override;
    bool snapToGrid() const override;
    void addCommand(KUndo2Command *command) override;
    KoSelectedShapesProxy *selectedShapesProxy() const override;
    KoToolProxy * toolProxy() const override;
    QWidget* canvasWidget() override;
    const QWidget* canvasWidget() const override;
    KoUnit unit() const override;
    void setCursor(const QCursor &) override {}

protected:
    QScopedPointer<KoShapeManager> m_shapeManager;
    QScopedPointer<KoSelectedShapesProxy> m_selectedShapesProxy;
    bool m_hasChangedWhileBeingInvisible {false};
    bool m_isDestroying {false};

    KisImageViewConverter m_viewConverter;
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

    KisShapeLayerCanvas(const KoColorSpace *cs, KisDefaultBoundsBaseSP defaultBounds, KisShapeLayer *parent);
    KisShapeLayerCanvas(const KisShapeLayerCanvas &rhs, KisShapeLayer *parent);
    ~KisShapeLayerCanvas() override;

    /// This canvas won't render onto a widget, but a projection
    void setProjection(KisPaintDeviceSP projection);
    KisPaintDeviceSP projection() const override;

    void setImage(KisImageWSP image) override;
    void updateCanvas(const QRectF& rc) override;
    void updateCanvas(const QVector<QRectF> &region);
    void forceRepaint() override;
    bool hasPendingUpdates() const override;

    void forceRepaintWithHiddenAreas() override;

    void resetCache() override;
    void rerenderAfterBeingInvisible() override;


private Q_SLOTS:
    friend class KisRepaintShapeLayerLayerJob;
    void repaint();
    void slotStartAsyncRepaint();
    void slotImageSizeChanged();

private:
    KisPaintDeviceSP m_projection;
    KisShapeLayer *m_parentLayer {0};

    KisThreadSafeSignalCompressor m_asyncUpdateSignalCompressor;
    volatile bool m_hasUpdateInCompressor = false;
    bool m_hasUpdateOnSetImage = false;
    KisSafeBlockingQueueConnectionProxy<void> m_safeForcedConnection;

    bool m_forceUpdateHiddenAreasOnly = false;
    QRegion m_dirtyRegion;
    QMutex m_dirtyRegionMutex;
    KoShapeManager::PaintJobsOrder m_paintJobsOrder;

    QRect m_cachedImageRect;

    KisImageWSP m_image;
    KisSignalAutoConnectionsStore m_imageConnections;
};

#endif
