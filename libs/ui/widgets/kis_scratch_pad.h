/* This file is part of the KDE project
 * Copyright 2010 (C) Boudewijn Rempt <boud@valdyas.org>
 * Copyright 2011 (C) Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_SCRATCH_PAD_H
#define KIS_SCRATCH_PAD_H

#include <QImage>
#include <QWidget>
#include <QRect>

#include <KoColor.h>


#include <brushengine/kis_paintop_preset.h>
#include <kis_types.h>
#include <kis_gradient_painter.h>
#include <kritaui_export.h>
#include <KisDisplayConfig.h>
#include <KisMultiSurfaceStateManager.h>

class QColor;

class KoPointerEvent;

class KisCanvasResourceProvider;
class KisUpdateScheduler;
class KisUndoStore;
class KisPostExecutionUndoAdapter;
class KisScratchPadEventFilter;
class KisPaintingInformationBuilder;
class KisToolFreehandHelper;
class KisNodeGraphListener;
class KisScreenMigrationTracker;


/**
 * A scratchpad is a painting canvas with only one zoomlevel and based on
 * a paint layer, not on a KisImage. It can have a blank, tiled background or
 * a gradient background.
 */
class KRITAUI_EXPORT KisScratchPad : public QWidget
{

    Q_OBJECT

public:
    void setupScratchPad(KisCanvasResourceProvider* resourceProvider,
                         const QColor &defaultColor);

    KisScratchPad(QWidget *parent = 0);
    ~KisScratchPad() override;

    /// set the specified rect as the area taken for @see cutoutOverlay
    void setCutoutOverlayRect(const QRect&rc);

    /**
     * keep track of if our scratchpad is in paint, pan, or color sample mode
     * Set to true if there is a GUI controlling current mode
     * If this is false, the modes are only changed with various mouse click shortcuts
     */
    void setModeManually(bool value);

    /**
     * @brief change the mode explicitly to paint, mix, or pan
     * @param what mode to change it to
     */
    void setModeType(QString modeName);

    /**
     * @brief return True if the scratchpad zoom level stay in sync with canvas
     */
    bool canvasZoomLink();

    /**
     * @brief should the scratchpad zoom level stay in sync with canvas
     * @param should we link zoom level
     */
    void setCanvasZoomLink(bool value);

    /**
     * @brief allow to manually set scratchpad scale when NOT linked to canvas zoom
     * @param scaleX zoom level for X
     * @param scaleY zoom level for Y
     * @return if scale is applied, return true otherwise false
     */
    bool setScale(qreal scaleX, qreal scaleY);

    /**
     * @brief return current scale X applied on scratchpad (whatever the zoom source is - canvas zoom or set manually)
     */
    qreal scaleX();
    /**
     * @brief return current scale Y applied on scratchpad (whatever the zoom source is - canvas zoom or set manually)
     */
    qreal scaleY();

    /**
     * @brief calculate and apply scale to fit content in viewport
     */
    void scaleToFit();
    /**
     * @brief reset scale value (to 1.0) and reinit position
     */
    void scaleReset();

    /**
     * @brief pan scratchpad content to given position
     * @param x absissa position to pan to
     * @param y ordinate position to pan to
     */
    void panTo(qint32 x, qint32 y);

    /**
     * @brief pan scratchpad content to center content in viewport
     */
    void panCenter();

    /// return the contents of the area under the cutoutOverlay rect
    QImage cutoutOverlay() const;

    // A callback for our own node graph listener
    void imageUpdated(const QRect &rect);

    // A callback for scratch pad default bounds
    QRect imageBounds() const;
    QRect viewportBounds() const;

    // A callback for scratch pad content bounds
    QRect contentBounds() const;


    // Called by the event filter
    void wheelDelta(QWheelEvent *event);
    void pointerPress(KoPointerEvent *event);
    void pointerRelease(KoPointerEvent *event);
    void pointerMove(KoPointerEvent *event);
    void resetState();

public Q_SLOTS:
    void fillDefault();
    void fillPattern(QTransform transform);
    void fillGradient(const QPoint &gradientVectorStart,
                      const QPoint &gradientVectorEnd,
                      KisGradientPainter::enumGradientShape gradientShape,
                      KisGradientPainter::enumGradientRepeat gradientRepeat,
                      bool reverseGradient,
                      bool dither);
    void fillGradient();
    void fillBackground();
    void fillForeground();
    void fillTransparent();

    void setFillColor(QColor newColor);

    /// Fill the area with what is on your current canvas
    void fillLayer(bool fullContent);
    void fillDocument();
    void fillDocument(bool fullContent);

    /**
     * Set the icon of the current preset
     */
    void setPresetImage(const QImage& image);

    /**
     * Paint the icon of the current preset inside the
     * cutout overlay
     *
     * \see setPresetImage
     */
    void paintPresetImage();

    /**
     * Paint the icon of a custom image that is being loaded
     *
     */
    void paintCustomImage(const QImage & loadedImage);


    void loadScratchpadImage(QImage image);

    QImage copyScratchpadImageData();

Q_SIGNALS:
    /**
     * @brief signal is emitted when scratchpad scale is changed (from zoom canvas or manually)
     * @param scale updated scale value
     */
    void scaleChanged(qreal scale);

    /**
     * @brief signal is emitted when scratchpad content is changed (stroke or fill)
     */
    void contentChanged();

    /**
     * @brief signal is emitted when scratchpad viewport has been modified (pan, zoom)
     * @param rect new viewport bounds
     */
    void viewportChanged(const QRect rect);

private Q_SLOTS:
    void slotScreenChanged(QScreen *screen);
    void setOnScreenResolution(qreal scaleX, qreal scaleY);
    void slotUpdateCanvas(const QRect &rect);
    void slotConfigChanged();

Q_SIGNALS:
    void colorSelected(const KoColor& color);
    void sigUpdateCanvas(const QRect &rect);

protected:
    void paintEvent ( QPaintEvent * event ) override;
    void resizeEvent( QResizeEvent *event) override;

private:
    void beginStroke(KoPointerEvent *event);
    void doStroke(KoPointerEvent *event);
    void endStroke(KoPointerEvent *event);

    void beginPan(KoPointerEvent *event);
    void doPan(KoPointerEvent *event);
    void endPan(KoPointerEvent *event);

    void sample(KoPointerEvent *event);

    void updateTransformations();

    bool setScaleImpl(qreal scaleX, qreal scaleY);

    void updateViewport();
    bool updateViewportImpl();

    void assignNewSurfaceState(const KisMultiSurfaceStateManager::State &newState);

    QTransform documentToWidget() const;
    QTransform widgetToDocument() const;

    friend class KisScratchPadPaintingInformationBuilder;

private:
    friend KisScratchPadEventFilter;

    void resetWheelDelta();

private:
    enum Mode {
        PAINTING,
        HOVERING,
        PANNING,
        SAMPLING
    };

    Mode modeFromButton(Qt::MouseButton button) const;

private:

    KoColor m_defaultColor;
    Mode m_toolMode;
    bool isModeManuallySet;
    bool isMouseDown;
    bool m_linkCanvasZoomLevel;

    // canvasScaleX & canvasScaleY: keep in memory current canvas zoom level
    qreal m_canvasScaleX;
    qreal m_canvasScaleY;

    // scratchpadScaleX & scratchpadScaleY: keep in memory current scratchpad zoom level
    qreal m_scratchpadScaleX;
    qreal m_scratchpadScaleY;

    // store accumulated mouse delta from mouseWheel
    int m_accumulatedMouseDelta;

    // keep viewport in memory
    QRect m_viewport;

    KisPaintLayerSP m_paintLayer;
    const KoColorProfile* m_displayProfile;
    KisDisplayConfig m_displayConfig;
    QCursor m_cursor;
    QCursor m_colorSamplerCursor;
    QRect m_cutoutOverlay;
    QBrush m_checkBrush;
    KisCanvasResourceProvider* m_resourceProvider;

    KisUpdateScheduler *m_updateScheduler;
    KisUndoStore *m_undoStore;
    KisPostExecutionUndoAdapter *m_undoAdapter;
    KisNodeGraphListener *m_nodeListener;
    KisScratchPadEventFilter *m_eventFilter;

    QScopedPointer<KisToolFreehandHelper> m_helper;
    KisPaintingInformationBuilder *m_infoBuilder;

    QTransform m_scaleTransform;
    QTransform m_translateTransform;

    QPointF m_panDocPoint;
    int m_scaleBorderWidth;

    QImage m_presetImage;
    QScopedPointer<KisScreenMigrationTracker> m_screenMigrationTracker;
    KisMultiSurfaceStateManager m_multiSurfaceStateManager;
    KisMultiSurfaceStateManager::State m_multiSurfaceState;
};

#endif // KIS_SCRATCH_PAD_H
