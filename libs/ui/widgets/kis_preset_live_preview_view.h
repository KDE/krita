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


class KisPresetLivePreviewView : public QGraphicsView
{
    Q_OBJECT


public:

    KisPresetLivePreviewView(QWidget *parent);
    ~KisPresetLivePreviewView();

    void setup();
    void setCurrentPreset(KisPaintOpPresetSP preset);
    void paintStroke();


public Q_SLOTS:
    void slotResetViewZoom();
    void slotZoomToOneHundredPercent();



private:
    KisImageSP m_image;
    KisLayerSP m_layer;
    const KoColorSpace *m_colorSpace;
    KisPainter *m_brushPreviewPainter;

    QGraphicsScene* m_brushPreviewScene;
    QPixmap* m_pixMapToDrawOn;
    QGraphicsPixmapItem *sceneImageItem;
    QGraphicsTextItem * noPreviewText;

    QPen m_penSettings;
    KisDistanceInformation m_currentDistance;

    QRect m_canvasSize;
    QPointF m_canvasCenterPoint;

    // for constructing the stroke shape
    QPointF m_startPoint;
    QPointF m_endPoint;
    KisPaintInformation m_pi1;
    KisPaintInformation m_pi2;
    KisPaintInformation m_paintDabStart;
    KisPaintInformation m_paintDabEnd;

    QPainterPath m_curvedLine;
    KisPaintInformation m_curvePointPI1;
    KisPaintInformation m_curvePointPI2;

    KisPaintOpPresetSP m_currentPreset;

    QImage temp_image;

    float scaleFactor;
    float m_currentBrushSize = 1.0;

    void zoomToBrushSize();



};

#endif
