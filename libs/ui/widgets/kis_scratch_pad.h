/* This file is part of the KDE project
 * Copyright 2010 (C) Boudewijn Rempt <boud@valdyas.org>
 * Copyright 2011 (C) Dmitry Kazakov <dimula73@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
    void paintCustomImage(const QImage& loadedImage);

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

    void pick(KoPointerEvent *event);

    void updateTransformations();

    QTransform documentToWidget() const;
    QTransform widgetToDocument() const;

private:
    enum Mode {
        PAINTING,
        HOVERING,
        PANNING,
        PICKING
    };

    Mode modeFromButton(Qt::MouseButton button) const;

private:

    KoColor m_defaultColor;
    Mode m_toolMode;
    KisPaintLayerSP m_paintLayer;
    const KoColorProfile* m_displayProfile;
    QCursor m_cursor;
    QRect m_cutoutOverlay;
    QBrush m_checkBrush;
    KisCanvasResourceProvider* m_resourceProvider;

    KisUpdateScheduler *m_updateScheduler;
    KisUndoStore *m_undoStore;
    KisPostExecutionUndoAdapter *m_undoAdapter;
    KisNodeGraphListener *m_nodeListener;
    KisScratchPadEventFilter *m_eventFilter;

    KisToolFreehandHelper *m_helper;
    KisPaintingInformationBuilder *m_infoBuilder;

    QTransform m_scaleTransform;
    QTransform m_translateTransform;

    QPointF m_panDocPoint;
    int m_scaleBorderWidth;

    QImage m_presetImage;
};

#endif // KIS_SCRATCH_PAD_H
