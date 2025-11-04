/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006, 2008 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2007-2010 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2007-2008 C. Boemann <cbo@boemann.dk>
 * SPDX-FileCopyrightText: 2006-2007 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2009 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOCANVASCONTROLLERWIDGET_H
#define KOCANVASCONTROLLERWIDGET_H

#include "kritaui_export.h"

#include <optional>
#include <QAbstractScrollArea>
#include <QPointer>
#include "KoCanvasController.h"
#include <KoZoomMode.h>

class KoShape;
class KoCanvasBase;
class KoCanvasSupervisor;
class KisCanvasState;
class KoViewTransformStillPoint;

/**
 * KoCanvasController implementation for QWidget based canvases
 */
class KRITAUI_EXPORT KoCanvasControllerWidget : public QAbstractScrollArea, public KoCanvasController
{
    Q_OBJECT
public:

    /**
     * Constructor.
     * @param actionCollection the action collection for this widget
     * @param parent the parent this widget will belong to
     */
    explicit KoCanvasControllerWidget(KisKActionCollection * actionCollection, KoCanvasSupervisor *observerProvider, QWidget *parent = 0);
    ~KoCanvasControllerWidget() override;

    /**
     * Reimplemented from QAbstractScrollArea.
     */
    void scrollContentsBy(int dx, int dy) override;

    /// Reimplemented from KoCanvasController

    /**
     * Activate this canvascontroller
     */
    virtual void activate();

    void setCanvas(KoCanvasBase *canvas) override;

    KoCanvasBase *canvas() const override;

    /**
     * Change the actual canvas widget used by the current canvas. This allows the canvas widget
     * to be changed while keeping the current KoCanvasBase canvas and its associated resources as
     * they are. This might be used, for example, to switch from a QWidget to a QOpenGLWidget canvas.
     * @param widget the new canvas widget.
     */
    virtual void changeCanvasWidget(QWidget *widget);

    /**
     * will cause the toolOptionWidgetsChanged to be emitted and all
     * listeners to be updated to the new widget.
     *
     * FIXME: This doesn't belong her and it does an
     * inherits("KoView") so it too much tied to komain
     *
     * @param widgets the map of widgets
     */
    void setToolOptionWidgets(const QList<QPointer<QWidget> > &widgets);

    void zoomTo(const QRect &rect) override;

    void setZoom(KoZoomMode::Mode mode, qreal zoom) override;
    void setZoom(KoZoomMode::Mode mode, qreal zoom, qreal resolutionX, qreal resolutionY);

    void setZoom(KoZoomMode::Mode mode, qreal zoom, const KoViewTransformStillPoint &stillPoint);
    void setZoom(KoZoomMode::Mode mode, qreal zoom, qreal resolutionX, qreal resolutionY, const std::optional<KoViewTransformStillPoint> &docStillPoint);


    void pan(const QPoint &distance) override;

    virtual void panUp() override;
    virtual void panDown() override;
    virtual void panLeft() override;
    virtual void panRight() override;

    QPoint scrollBarValue() const override;

    /**
     * Used by KisCanvasController to correct the scrollbars position
     * after the rotation.
     */
    void setScrollBarValue(const QPoint &value) override;

    virtual KisCanvasState canvasState() const = 0;

    /**
     * \internal
     */
    class Private;
    KoCanvasControllerWidget::Private *priv();

protected:
    friend class KisZoomAndPanTest;

    /// reimplemented from QWidget
    void paintEvent(QPaintEvent *event) override;
    /// reimplemented from QWidget
    void resizeEvent(QResizeEvent *resizeEvent) override;
    /// reimplemented from QWidget
    void dragEnterEvent(QDragEnterEvent *event) override;
    /// reimplemented from QWidget
    void dropEvent(QDropEvent *event) override;
    /// reimplemented from QWidget
    void dragMoveEvent(QDragMoveEvent *event) override;
    /// reimplemented from QWidget
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    /// reimplemented from QWidget
    bool focusNextPrevChild(bool next) override;
    /// reimplemented from QAbstractScrollArea
    bool viewportEvent(QEvent *event) override;

    virtual void updateCanvasOffsetInternal(const QPointF &newOffset) = 0;
    virtual void updateCanvasWidgetSizeInternal(const QSize &newSize, qreal devicePixelRatio) = 0;
    virtual void updateCanvasZoomInternal(KoZoomMode::Mode mode, qreal zoom, qreal resolutionX, qreal resolutionY, const std::optional<KoViewTransformStillPoint> &docStillPoint) = 0;
    virtual void zoomToInternal(const QRect &viewRect) = 0;

    void emitSignals(const KisCanvasState &oldState, const KisCanvasState &newState);

private:
    Q_PRIVATE_SLOT(d, void activate())

    Private * const d;
};

#endif
