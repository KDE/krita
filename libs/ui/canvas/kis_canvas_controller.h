/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CANVAS_CONTROLLER_H
#define KIS_CANVAS_CONTROLLER_H

#include <KoCanvasControllerWidget.h>
#include <libs/flake/KoCanvasSupervisor.h>

#include "kritaui_export.h"
#include "kis_types.h"
#include "KisWraparoundAxis.h"

class KisCanvasState;
class KConfigGroup;
class KisView;
class KisCanvasState;

class KRITAUI_EXPORT KisCanvasController : public KoCanvasControllerWidget
{
    Q_OBJECT

public:
    KisCanvasController(QPointer<KisView>parent, KoCanvasSupervisor *observerProvider, KisKActionCollection * actionCollection);
    ~KisCanvasController() override;

    void ensureVisibleDoc(const QRectF &docRect, bool smooth) override;
    void setCanvas(KoCanvasBase *canvas) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void activate() override;

    QPointF currentCursorPosition() const override;

    KisCanvasState canvasState() const override;
    KoZoomState zoomState() const override;

    QPointF preferredCenter() const override;
    void setPreferredCenter(const QPointF &viewPoint) override;

    void zoomIn() override;
    void zoomIn(const QPoint &center) override;
    void zoomOut() override;
    void zoomOut(const QPoint &center) override;

    void syncOnReferencesChange(const QRectF &referencesRect);
    void syncOnImageResolutionChange();
    void syncOnImageSizeChange(const QPointF &oldStillPoint, const QPointF &newStillPoint);

public:
    bool wrapAroundMode() const;
    WrapAroundAxis wrapAroundModeAxis() const;
    bool levelOfDetailMode() const;

    void saveCanvasState(KisPropertiesConfiguration &config) const;
    void restoreCanvasState(const KisPropertiesConfiguration &config);

    void resetScrollBars() override;

    void updateScreenResolution(QWidget *parentWidget);
    bool usePrintResolutionMode();

    qreal effectiveCanvasResolutionX() const;
    qreal effectiveCanvasResolutionY() const;

public Q_SLOTS:
    void setUsePrintResolutionMode(bool value);

    void mirrorCanvas(bool enable);
    void mirrorCanvasAroundCursor(bool enable);
    void mirrorCanvasAroundCanvas(bool enable);

    void beginCanvasRotation();
    void endCanvasRotation();
    void rotateCanvas(qreal angle, const QPointF &center, bool isNativeGesture = false);
    void rotateCanvas(qreal angle);
    void rotateCanvasRight15();
    void rotateCanvasLeft15();
    qreal rotation() const;
    void resetCanvasRotation();

    void slotToggleWrapAroundMode(bool value);
    void slotSetWrapAroundModeAxis(WrapAroundAxis axis);
    void slotSetWrapAroundModeAxisHV();
    void slotSetWrapAroundModeAxisH();
    void slotSetWrapAroundModeAxisV();
    void slotTogglePixelGrid(bool value);
    void slotToggleLevelOfDetailMode(bool value);

protected:
    void updateCanvasOffsetInternal(const QPointF &newOffset) override;
    void updateCanvasWidgetSizeInternal(const QSize &newSize) override;
    void updateCanvasZoomInternal(KoZoomMode::Mode mode, qreal zoom, qreal resolutionX, qreal resolutionY, const QPointF &stillPoint) override;
    void zoomToInternal(const QRect &viewRect) override;

private:
    void mirrorCanvasImpl(const QPointF &widgetPoint, bool enable);

Q_SIGNALS:
    void documentSizeChanged();

Q_SIGNALS:
    void sigUsePrintResolutionModeChanged(bool value);


private:
    struct Private;
    Private * const m_d;
};

#endif /* KIS_CANVAS_CONTROLLER_H */
