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
    setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );

    m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    m_image = new KisImage(0, this->width(), this->height(), m_colorSpace, "stroke sample image");

    m_layer = new KisPaintLayer(m_image, "livePreviewStrokeSample", OPACITY_OPAQUE_U8, m_colorSpace);
    m_brushPreviewPainter = new KisPainter(m_layer->paintDevice());
    m_pixMapToDrawOn = new QPixmap(this->width(), this->height());

    // set scene for the view
    m_brushPreviewScene = new QGraphicsScene();

    // calculate the shape for a simple straight line stroke
    m_startPoint.setX(10);
    m_startPoint.setY(m_pixMapToDrawOn->height() * 0.5);
    m_endPoint.setX(m_pixMapToDrawOn->width() - 20);
    m_endPoint.setY(m_pixMapToDrawOn->height() * 0.5);

    // points for drawing a line
    m_pi1.setPos(m_startPoint);
    m_pi1.setPressure(0.0);
    m_pi2.setPos(m_endPoint);
    m_pi2.setPressure(1.0);

    m_paintDabStart.setPos(QPointF(30,this->height()*.5));
    m_paintDabStart.setPressure(1.0);
    m_paintDabEnd.setPos(QPointF(33,this->height()*.5));
    m_paintDabEnd.setPressure(1.0);

    // points for drawing an S curve
    m_curvePointPI1.setPos(QPointF(this->height()-20, 20));
    m_curvePointPI1.setPressure(0.0);
    m_curvePointPI2.setPos(QPointF(this->width()-20, this->height()*.5));
    m_curvePointPI2.setPressure(1.0);

}

void KisPresetLivePreviewView::setCurrentPreset(KisPaintOpPresetSP preset)
{
    m_currentPreset = preset;
}

void KisPresetLivePreviewView::paintStroke()
{
    m_brushPreviewPainter->setPaintOpPreset(m_currentPreset, m_layer, m_image);

    // TODO: there is some weird clearing issue that is not working right at the start
    // there is going to have to be some reset or clear that happens to the layer or
    // image data to prevent it


    // different blending modes seem to give it issues when switching
    // roundmarker - no preview
    // paintbrush, sketch, spray, hatchingbrush -- leave as normal curve
    // colorsmudge -- could use multiple backgrounds maybe
    // deform -
    // hairybrush -- having a rough time. need to test more


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

    } else {
        // fill with gray first to clear out what existed before
        m_brushPreviewPainter->fill(0,0,
                                  m_layer->image()->width(),
                                  m_layer->image()->height(),
                                  KoColor(palette().color(QPalette::Background) , m_colorSpace));

        m_brushPreviewPainter->setPaintColor(KoColor(palette().color(QPalette::Text), m_colorSpace));
    }


    //m_brushPreviewPainter->paintLine(m_pi1, m_pi2, &m_currentDistance); // option to display line (works)
    m_brushPreviewPainter->paintBezierCurve(m_curvePointPI1, QPointF(150,0), QPointF(150,50), m_curvePointPI2, &m_currentDistance);

    // crop the layer so a brush stroke won't go outside of the area
    m_layer->paintDevice()->crop(0,0, this->width(), this->height()); // in case stroke goes outside area

    // TODO: instead of cropping. try to keep re-using the pixmap item instead of keeping adding more items to scene

    temp_image = m_layer->paintDevice()->convertToQImage(0);

    setScene(m_brushPreviewScene);


    //m_brushPreviewScene->clear(); // hopefully this clear everything else
    //this->resetMatrix();

    m_brushPreviewScene->addPixmap(QPixmap::fromImage(temp_image));

}



