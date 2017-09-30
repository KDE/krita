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

KisPresetLivePreviewView::KisPresetLivePreviewView(QWidget *parent): QGraphicsView(parent)
{
}

KisPresetLivePreviewView::~KisPresetLivePreviewView()
{
}


void KisPresetLivePreviewView::setup()
{
    scaleFactor = 1.0;

    noPreviewText = 0;
    sceneImageItem = 0;

    setCursor(Qt::SizeAllCursor);

    setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );


    // layer image needs to be big enough to get an entire stroke of data at 1,000px
    m_canvasSize.setWidth(800);
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
    m_curvePointPI1.setPos(QPointF(m_canvasCenterPoint.x()-this->width()*0.5,
                                   m_canvasCenterPoint.y()));
    m_curvePointPI1.setPressure(0.0);

    m_curvePointPI2.setPos(QPointF(m_canvasCenterPoint.x()+this->width()*0.5,
                                   m_canvasCenterPoint.y()));

    m_curvePointPI2.setPressure(1.0);

}

void KisPresetLivePreviewView::setCurrentPreset(KisPaintOpPresetSP preset)
{
    m_currentPreset = preset;
}

void KisPresetLivePreviewView::paintStroke()
{
    m_brushPreviewPainter->setPaintOpPreset(m_currentPreset, m_layer, m_image);

    // clean up "no preview" text object if it exists
    if (noPreviewText) {
        this->scene()->removeItem(noPreviewText);
        noPreviewText = 0;
    }


    if (m_currentPreset->paintOp().id() == "colorsmudge") {

        // have grey and black strips for this
        KoColor gray;
        gray.fromQColor(QColor(80,80,80));

        KoColor darkGray;
        darkGray.fromQColor(QColor(140,140,140));



        m_brushPreviewPainter->fill(0,
                                    0,
                                    m_layer->image()->width()*0.20,
                                    m_layer->image()->height(),
                                    gray);


        m_brushPreviewPainter->fill(m_layer->image()->width()*0.20,
                                    0,
                                    m_layer->image()->width()*0.4,
                                    m_layer->image()->height(),
                                    darkGray);


        m_brushPreviewPainter->fill(m_layer->image()->width()*0.4,
                                    0,
                                  m_layer->image()->width()*0.6,
                                  m_layer->image()->height(),
                                  gray);


        m_brushPreviewPainter->fill(m_layer->image()->width()*0.6,
                                    0,
                                  m_layer->image()->width()*0.8,
                                  m_layer->image()->height(),
                                  darkGray);

        m_brushPreviewPainter->fill(m_layer->image()->width()*0.8,
                                    0,
                                  m_layer->image()->width(),
                                  m_layer->image()->height(),
                                  gray);



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


    //m_brushPreviewPainter->paintLine(m_pi1, m_pi2, &m_currentDistance); // option to display line (works)
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


