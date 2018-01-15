
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
#include <strokes/freehand_stroke.h>
#include <strokes/KisFreehandStrokeInfo.h>

KisPresetLivePreviewView::KisPresetLivePreviewView(QWidget *parent): QGraphicsView(parent)
{

}

KisPresetLivePreviewView::~KisPresetLivePreviewView()
{
    delete m_noPreviewText;
    delete m_brushPreviewScene;
}

void KisPresetLivePreviewView::setup()
{
    // initializing to 0 helps check later if they actually have something in them
    m_noPreviewText = 0;
    m_sceneImageItem = 0;

    setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );


    // layer image needs to be big enough to get an entire stroke of data
    m_canvasSize.setWidth(this->width());
    m_canvasSize.setHeight(this->height());

    m_canvasCenterPoint.setX(m_canvasSize.width()*0.5);
    m_canvasCenterPoint.setY(m_canvasSize.height()*0.5);

    m_colorSpace = KoColorSpaceRegistry::instance()->rgb8();
    m_image = new KisImage(0, m_canvasSize.width(), m_canvasSize.height(), m_colorSpace, "stroke sample image");


    m_layer = new KisPaintLayer(m_image, "livePreviewStrokeSample", OPACITY_OPAQUE_U8, m_colorSpace);

    // set scene for the view
    m_brushPreviewScene = new QGraphicsScene();
    setScene(m_brushPreviewScene);

}

void KisPresetLivePreviewView::setCurrentPreset(KisPaintOpPresetSP preset)
{
    m_currentPreset = preset;
}

void KisPresetLivePreviewView::updateStroke()
{
    paintBackground();

    // do not paint a stroke if we are any of these engines (they have some issue currently)
    if (m_currentPreset->paintOp().id() == "roundmarker" ||
        m_currentPreset->paintOp().id() == "experimentbrush" ||
        m_currentPreset->paintOp().id() == "duplicate") {

        return;
    }

    setupAndPaintStroke();


    // crop the layer so a brush stroke won't go outside of the area
    m_layer->paintDevice()->crop(0,0, m_layer->image()->width(), m_layer->image()->height());

    QImage m_temp_image;
    m_temp_image = m_layer->paintDevice()->convertToQImage(0);


    // only add the object once...then just update the pixmap so we can move the preview around
    if (!m_sceneImageItem) {
        m_sceneImageItem = m_brushPreviewScene->addPixmap(QPixmap::fromImage(m_temp_image));
    } else {
        m_sceneImageItem->setPixmap(QPixmap::fromImage(m_temp_image));
    }

}



void KisPresetLivePreviewView::paintBackground()
{
    // clean up "no preview" text object if it exists. we will add it later if we need it
    if (m_noPreviewText) {
        this->scene()->removeItem(m_noPreviewText);
        m_noPreviewText = 0;
    }


    if (m_currentPreset->paintOp().id() == "colorsmudge" ||
        m_currentPreset->paintOp().id() == "deformbrush" ||
        m_currentPreset->paintOp().id() == "filter") {

        // easier to see deformations and smudging with alternating stripes in the background
        // paint the whole background with alternating stripes
        // filter engine may or may not show things depending on the filter...but it is better than nothing

        int grayStrips = 20;
        for (int i=0; i < grayStrips; i++ ) {

            float sectionPercent = 1.0 / (float)grayStrips;
            bool isAlternating = i % 2;
            KoColor fillColor(m_layer->paintDevice()->colorSpace());

            if (isAlternating) {
                fillColor.fromQColor(QColor(80,80,80));
            } else {
                fillColor.fromQColor(QColor(140,140,140));
            }


            const QRect fillRect(m_layer->image()->width()*sectionPercent*i,
                                 0,
                                 m_layer->image()->width()*(sectionPercent*i +sectionPercent),
                                 m_layer->image()->height());
            m_layer->paintDevice()->fill(fillRect, fillColor);
        }

        m_paintColor = KoColor(Qt::white, m_colorSpace);

    }
    else if (m_currentPreset->paintOp().id() == "roundmarker" ||
             m_currentPreset->paintOp().id() == "experimentbrush" ||
             m_currentPreset->paintOp().id() == "duplicate" ) {

        // cases where we will not show a preview for now
        // roundbrush (quick) -- this isn't showing anything, disable showing preview
        // experimentbrush -- this creates artifacts that carry over to other previews and messes up their display
        // duplicate (clone) brush doesn't have a preview as it doesn't show anything)

        if(m_sceneImageItem) {
            this->scene()->removeItem(m_sceneImageItem);
            m_sceneImageItem = 0;
        }

        QFont font;
        font.setPixelSize(14);
        font.setBold(false);

        m_noPreviewText = this->scene()->addText(i18n("No Preview for this engine"),font);
        m_noPreviewText->setPos(50, this->height()/4);

        return;

    }
    else {

        // fill with gray first to clear out what existed from previous preview
        m_layer->paintDevice()->fill(m_image->bounds(), KoColor(palette().color(QPalette::Background) , m_colorSpace));
        m_paintColor = KoColor(palette().color(QPalette::Text), m_colorSpace);
    }
}

void KisPresetLivePreviewView::setupAndPaintStroke()
{
    // limit the brush stroke size. larger brush strokes just don't look good and are CPU intensive
    // we are making a proxy preset and setting it to the painter...otherwise setting the brush size of the original preset
    // will fire off signals that make this run in an infinite loop
    qreal originalPresetSize = m_currentPreset->settings()->paintOpSize();
    qreal previewSize = qBound(3.0, m_currentPreset->settings()->paintOpSize(), 25.0 ); // constrain live preview brush size
    KisPaintOpPresetSP proxy_preset = m_currentPreset->clone();
    proxy_preset->settings()->setPaintOpSize(previewSize);


    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(m_image,
                                 m_layer);

    resources->setBrush(proxy_preset);
    resources->setFGColorOverride(m_paintColor);
    KisFreehandStrokeInfo *strokeInfo = new KisFreehandStrokeInfo();

    KisStrokeStrategy *stroke =
        new FreehandStrokeStrategy(resources, strokeInfo, kundo2_noi18n("temp_stroke"));

    KisStrokeId strokeId = m_image->startStroke(stroke);



    //m_brushPreviewPainter->setPaintOpPreset(proxy_preset, m_layer, m_image);



    // paint the stroke. The sketchbrush gets a different shape than the others to show how it works
    if (m_currentPreset->paintOp().id() == "sketchbrush") {

        KisPaintInformation pointOne;
        pointOne.setPressure(0.0);
        pointOne.setPos(QPointF(m_canvasCenterPoint.x() - (this->width() * 0.4),
                                m_canvasCenterPoint.y() - (this->height()*0.2) ));

        KisPaintInformation pointTwo;
        pointTwo.setPressure(1.0);
        pointTwo.setPos(QPointF(m_canvasCenterPoint.x() + (this->width() * 0.4),
                                m_canvasCenterPoint.y() + (this->height()*0.2) ));


        m_image->addJob(strokeId,
            new FreehandStrokeStrategy::Data(0,
                                             pointOne,
                                             QPointF(m_canvasCenterPoint.x() + this->width(),
                                                     m_canvasCenterPoint.y() - (this->height()*0.2) ),
                                             QPointF(m_canvasCenterPoint.x() - this->width(),
                                                     m_canvasCenterPoint.y() + (this->height()*0.2) ),
                                             pointTwo));

    } else {

        // paint an S curve
        m_curvePointPI1.setPos(QPointF(m_canvasCenterPoint.x() - (this->width()*0.45),
                                       m_canvasCenterPoint.y() + (this->height()*0.2)));
        m_curvePointPI1.setPressure(0.0);


        m_curvePointPI2.setPos(QPointF(m_canvasCenterPoint.x() + (this->width()*0.4),
                                       m_canvasCenterPoint.y() - (this->height()*0.2)   ));

        m_curvePointPI2.setPressure(1.0);

        m_image->addJob(strokeId,
            new FreehandStrokeStrategy::Data(0,
                                             m_curvePointPI1,
                                             QPointF(m_canvasCenterPoint.x(),
                                                     m_canvasCenterPoint.y()-this->height()),
                                             QPointF(m_canvasCenterPoint.x(),
                                                     m_canvasCenterPoint.y()+this->height()),
                                             m_curvePointPI2));
    }

    m_image->addJob(strokeId, new FreehandStrokeStrategy::UpdateData(true));
    m_image->endStroke(strokeId);
    m_image->waitForDone();


    // even though the brush is cloned, the proxy_preset still has some connection to the original preset which will mess brush sizing
    // we need to return brush size to normal.The normal brush sends out a lot of extra signals, so keeping the proxy for now
    proxy_preset->settings()->setPaintOpSize(originalPresetSize);

}

