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
#include <kritaui_export.h>

class QColor;

class KoColorProfile;
class KoPointerEvent;

class KisCanvasResourceProvider;
class KisUpdateScheduler;
class KisUndoStore;
class KisPostExecutionUndoAdapter;
class KisScratchPadEventFilter;
class KisPaintingInformationBuilder;
class KisToolFreehandHelper;
class KisNodeGraphListener;


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
     * @brief should the scratchpad zoom level stay in sync with canvas
     * @param should we link zoom level
     */
    void linkCanvavsToZoomLevel(bool value);


    /// return the contents of the area under the cutoutOverlay rect
    QImage cutoutOverlay() const;

    // A callback for our own node graph listener
    void imageUpdated(const QRect &rect);

    // A callback for scratch pad default bounds
    QRect imageBounds() const;


    // Called by the event filter
    void pointerPress(KoPointerEvent *event);
    void pointerRelease(KoPointerEvent *event);
    void pointerMove(KoPointerEvent *event);

public Q_SLOTS:
    void fillDefault();
    void fillGradient();
    void fillBackground();
    void fillTransparent();

    void setFillColor(QColor newColor);

    /// Fill the area with what is on your current canvas
    void fillLayer();


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

private Q_SLOTS:
    void setOnScreenResolution(qreal scaleX, qreal scaleY);
    void setDisplayProfile(const KoColorProfile* colorProfile);
    void slotUpdateCanvas(const QRect &rect);

Q_SIGNALS:
    void colorSelected(const KoColor& color);
    void sigUpdateCanvas(const QRect &rect);

protected:
    void paintEvent ( QPaintEvent * event ) override;


private:
    void beginStroke(KoPointerEvent *event);
    void doStroke(KoPointerEvent *event);
    void endStroke(KoPointerEvent *event);

    void beginPan(KoPointerEvent *event);
    void doPan(KoPointerEvent *event);
    void endPan(KoPointerEvent *event);

    void sample(KoPointerEvent *event);

    void updateTransformations();

    QTransform documentToWidget() const;
    QTransform widgetToDocument() const;

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
    bool linkCanvasZoomLevel;
    KisPaintLayerSP m_paintLayer;
    const KoColorProfile* m_displayProfile;
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
};

#endif // KIS_SCRATCH_PAD_H
