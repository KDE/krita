/*
  *  Copyright (c) 2017 Scott Petrovic <scottpetrovic@gmail.com>
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

#ifndef _KIS_PRESET_LIVE_PREVIEW_
#define _KIS_PRESET_LIVE_PREVIEW_

#include <QImage>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainterPath>
#include <QGraphicsPixmapItem>

#include "kis_paintop_preset.h"
#include "KoColorSpaceRegistry.h"
#include "kis_paint_layer.h"
#include "kis_painter.h"
#include "kis_distance_information.h"
#include "kis_painting_information_builder.h"
#include <kis_image.h>
#include <kis_types.h>
#include <KoColor.h>
#include "kis_signal_compressor.h"

/**
 * Widget for displaying a live brush preview of your
 * selected brush. It listens for signalsetting changes
 * that the brush preset outputs and updates the preview
 * accordingly. This class can be added to a UI file
 * similar to how a QGraphicsView is added
 */
class KisPresetLivePreviewView : public QGraphicsView
{
    Q_OBJECT


public:

    KisPresetLivePreviewView(QWidget *parent);
    ~KisPresetLivePreviewView();

    /**
     * @brief one time setup for initialization of many variables.
     * This live preview might be in a UI file, so make sure to
     * call this before starting to use it
     */
    void setup();

    /**
     * @brief set the current preset from resource manager for the live preview to use.
     * Good to call this every stroke update in case the preset has changed
     * @param preset the current preset from the resource manager
     */
    void setCurrentPreset(KisPaintOpPresetSP preset);
    void requestUpdateStroke();

private Q_SLOTS:
    void updateStroke();
    void slotPreviewGenerationCompleted();

private:

    /// internally sets the image area for brush preview
    KisImageSP m_image;

    /// internally sets the layer area for brush preview
    KisLayerSP m_layer;

    /// internally sets the color space for brush preview
    const KoColorSpace *m_colorSpace;

    /// the color which is used for rendering the stroke
    KoColor m_paintColor;

    /// the scene that can add items like text and the brush stroke image
    QGraphicsScene *m_brushPreviewScene;

    /// holds the preview brush stroke data
    QGraphicsPixmapItem *m_sceneImageItem;

    /// holds the 'no preview available' text object
    QGraphicsTextItem *m_noPreviewText;

    /// holds the width and height of the image of the brush preview
    /// Probably can later add set functions to make this customizable
    /// It is hard-coded to 1200 x 400 for right now for image size
    QRect m_canvasSize;

    /// convenience variable used internally when positioning the objects
    /// and points in the scene
    QPointF m_canvasCenterPoint;

    /// internal variables for constructing the stroke start and end shape
    /// there are two points that construct the "S" curve with this
    KisDistanceInformation m_currentDistance;
    QPainterPath m_curvedLine;
    KisPaintInformation m_curvePointPI1;
    KisPaintInformation m_curvePointPI2;

    /// internally stores the current preset.
    /// See setCurrentPreset(KisPaintOpPresetSP preset)
    /// for setting this externally
    KisPaintOpPresetSP m_currentPreset;

    /// holds the current zoom(scale) level of scene
    float m_scaleFactor;

    /// internal reference for internal brush size
    /// used to check if our brush size has changed
    /// do zooming and other things internally if it has changed
    float m_currentBrushSize = 1.0;

    bool m_previewGenerationInProgress = false;
    KisSignalCompressor m_updateCompressor;

    /// the range of brush sizes that will control zooming in/out
    const float m_minBrushVal = 10.0;
    const float m_maxBrushVal = 100.0;

    /// range of scale values. 1.0 == 100%
    const qreal m_minScale = 1.0;
    const qreal m_maxScale = 0.3;

    /// multiplier that is used for lengthening the brush stroke points
    const float m_minStrokeScale = 0.4; // for smaller brush stroke
    const float m_maxStrokeScale = 1.0; // for larger brush stroke


    /**
     * @brief works as both clearing the previous stroke, providing
     * striped backgrounds for smudging brushes, and text if there is no preview
     */
    void paintBackground();

    /**
     * @brief creates and performs the actual stroke that goes on top of the background
     * this is internally and should always be called after the paintBackground()
     */
    void setupAndPaintStroke();

};

#endif
