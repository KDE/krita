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

#include <kis_preset_live_preview_view.h>
#include <QDebug>
#include <QGraphicsPixmapItem>
#include "kis_paintop_settings.h"

KisPresetLivePreviewView::KisPresetLivePreviewView(QWidget *parent): QGraphicsView(parent)
{
}

KisPresetLivePreviewView::~KisPresetLivePreviewView()
{
}

void KisPresetLivePreviewView::setup()
{
    noPreviewText = 0;
    sceneImageItem = 0;

    setCursor(Qt::SizeAllCursor);

    setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );


    // layer image needs to be big enough to get an entire stroke of data at 1,000px
    m_canvasSize.setWidth(1200);
    m_canvasSize.setHeight(400);

    m_canvasCenterPoint.setX(m_canvasSize.width()*0.5);
    m_canvasCenterPoint.setY(m_canvasSize.height()*0.5);

    m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    m_image = new KisImage(0, m_canvasSize.width(), m_canvasSize.height(), m_colorSpace, "stroke sample image");


    m_layer = new KisPaintLayer(m_image, "livePreviewStrokeSample", OPACITY_OPAQUE_U8, m_colorSpace);
    m_brushPreviewPainter = new KisPainter(m_layer->paintDevice());

    // set scene for the view
    m_brushPreviewScene = new QGraphicsScene();
    setScene(m_brushPreviewScene);

    // points for drawing an S curve
    // we are going to paint the stroke right in the middle of the canvas to make sure
    // everything is captured for big brush strokes
    //TODO: we need to update these points according to the brush size. Larger brushes need larger strokes
    m_curvePointPI1.setPos(QPointF(m_canvasCenterPoint.x() - (this->width()*0.4),
                                   m_canvasCenterPoint.y()));
    m_curvePointPI1.setPressure(0.0);

    m_curvePointPI2.setPos(QPointF(m_canvasCenterPoint.x() + (this->width()*0.4),
                                   m_canvasCenterPoint.y()));

    m_curvePointPI2.setPressure(1.0);


    scaleFactor = 1.0;
    slotZoomViewOut();// zoomed out a bit by default for now until we get something better
    slotZoomViewOut();
    slotZoomViewOut();
    slotZoomViewOut();

}

void KisPresetLivePreviewView::setCurrentPreset(KisPaintOpPresetSP preset)
{
    m_currentPreset = preset;
}

void KisPresetLivePreviewView::paintStroke()
{
    m_brushPreviewPainter->setPaintOpPreset(m_currentPreset, m_layer, m_image);


    // scale the viewport if we are changing brush size
    if (m_currentBrushSize != m_currentPreset->settings()->paintOpSize()) {
        m_currentBrushSize = m_currentPreset->settings()->paintOpSize();
        zoomToBrushSize();
    }


    // clean up "no preview" text object if it exists
    if (noPreviewText) {
        this->scene()->removeItem(noPreviewText);
        noPreviewText = 0;
    }


    if (m_currentPreset->paintOp().id() == "colorsmudge" || m_currentPreset->paintOp().id() == "deformbrush") {

        // easier to see deformations and smudging with alternating stripes in the background
        int grayStrips = 20;
        for (int i=0; i < grayStrips; i++ ) {

            float sectionPercent = 1.0 / (float)grayStrips;
            bool isAlternating = i % 2;
            KoColor fillColor;

            if (isAlternating) {
                fillColor.fromQColor(QColor(80,80,80));
            } else {
                fillColor.fromQColor(QColor(140,140,140));
            }


            m_brushPreviewPainter->fill(m_layer->image()->width()*sectionPercent*i,
                                        0,
                                        m_layer->image()->width()*(sectionPercent*i +sectionPercent),
                                        m_layer->image()->height(),
                                        fillColor);

        }

        m_brushPreviewPainter->setPaintColor(KoColor(Qt::white, m_colorSpace));

    }
    else if (m_currentPreset->paintOp().id() == "roundmarker" || m_currentPreset->paintOp().id() == "experimentbrush"){
        // roundbrush (quick) -- this isn't showing anything, disable showing preview
        // experimentbrush -- this creates artifacts that carry over to other previews and messes up their display

        if(sceneImageItem) {
            this->scene()->removeItem(sceneImageItem);
            sceneImageItem = 0;
        }



        slotResetViewZoom();

        QFont font;
        font.setPixelSize(14);
        font.setBold(false);

        noPreviewText = this->scene()->addText(i18n("No Preview for this engine"),font);


        return;

    }
    else {

        // fill with gray first to clear out what existed before
        m_brushPreviewPainter->fill(0,0,
                                  m_layer->image()->width(),
                                  m_layer->image()->height(),
                                  KoColor(palette().color(QPalette::Background) , m_colorSpace));

        m_brushPreviewPainter->setPaintColor(KoColor(palette().color(QPalette::Text), m_colorSpace));
    }


    m_brushPreviewPainter->paintBezierCurve(m_curvePointPI1,
                                            QPointF(m_canvasCenterPoint.x(),
                                                    m_canvasCenterPoint.y()-this->height()),
                                            QPointF(m_canvasCenterPoint.x(),
                                                     m_canvasCenterPoint.y()+this->height()),
                                            m_curvePointPI2, &m_currentDistance);

    // crop the layer so a brush stroke won't go outside of the area
    m_layer->paintDevice()->crop(0,0, m_layer->image()->width(), m_layer->image()->height()); // in case of a super big brush
    temp_image = m_layer->paintDevice()->convertToQImage(0);


    // only add the object once...then just update the pixmap so we can move the preview around
    if (!sceneImageItem) {
        sceneImageItem = m_brushPreviewScene->addPixmap(QPixmap::fromImage(temp_image));
        sceneImageItem->setFlag(QGraphicsItem::ItemIsSelectable);
        sceneImageItem->setFlag(QGraphicsItem::ItemIsMovable);
        sceneImageItem->setFlag(QGraphicsItem::ItemSendsGeometryChanges);
        sceneImageItem->setPos(-m_canvasCenterPoint.x(), -m_canvasCenterPoint.y());
    } else {
        sceneImageItem->setPixmap(QPixmap::fromImage(temp_image));
    }

}

void KisPresetLivePreviewView::slotResetViewZoom()
{
    scaleFactor = 1.0;
    resetMatrix();
    this->scale(scaleFactor, scaleFactor);
}

void KisPresetLivePreviewView::slotZoomViewOut()
{
    scaleFactor = scaleFactor * 0.9;
    this->scale(scaleFactor, scaleFactor);
}

void KisPresetLivePreviewView::zoomToBrushSize() {

    // m_currentBrushSize.
    // when the zooming will start and stop
    float minBrushVal = 1.0;
    float maxBrushVal = 250.0;

    // range of scale values
    qreal minScale = 1.0;
    qreal maxScale = 0.1;


    // find the slope of the line (slope-intercept form)
    float slope = (maxScale-minScale) / (maxBrushVal-minBrushVal);  // y2-y1 / x2-x1
    float yIntercept = minScale - slope * minBrushVal;  // y1 − m * x1


    // finally calculate our zoom level
    float thresholdValue = qBound(minBrushVal, m_currentBrushSize, maxBrushVal);
    scaleFactor = thresholdValue * slope + yIntercept; // y = mx + b

    resetMatrix();
    this->scale(scaleFactor,scaleFactor);
}

