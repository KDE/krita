#include "kis_timeline_cells.h"
#include <QPainter>

KisTimelineCells::KisTimelineCells(KisTimeline *parent) /*: QWidget(parent)*/
{
    this->m_dock = parent;
    this->m_type = "tracks";

    this->m_cache = NULL;
    this->m_frameLength = 240;
    this->m_shortScrub = false;
    this->m_fps = 12;
    this->m_startY = 0;
    this->m_endY = 0;
    this->m_mouseMoveY = 0;
    this->m_startLayerNumber = -1;
    this->m_offsetX = 0;
    this->m_offsetY = 20;
    this->m_frameOffset = 0;
    this->m_selectionOffset = 0;
    this->m_layerOffset = 0;
    this->m_frameclicked = -1;

    this->m_frameSize = 12;
    this->m_fontSize = 12;
    this->m_layerHeight = 20;
    this->m_layerCount = 3;

    this->setMinimumSize(500, 4*this->m_layerHeight);
    this->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    this->setAttribute(Qt::WA_OpaquePaintEvent, false);
}

int KisTimelineCells::getFrameNumber(int x){
    return m_frameOffset+1+(x - m_offsetX)/m_frameSize;
}

int KisTimelineCells::getFrameX(int frameNumber){
    return m_offsetX + (frameNumber - m_frameOffset)*m_frameSize;
}

int KisTimelineCells::getLayerNumber(int y){
    int layerNumber = m_layerOffset + (y- m_offsetY)/m_layerHeight;
    if(y < m_offsetY){
        layerNumber  -1;
    }
    return layerNumber;
}

int KisTimelineCells::getLayerY(int layerNumber){
    return m_offsetY + (layerNumber - m_layerOffset)*m_layerHeight;
}

void KisTimelineCells::updateFrame(int frameNumber){
    int x = getFrameX(frameNumber);
    update(x - m_frameSize, 0, m_frameSize +1, height());
}

void KisTimelineCells::updateContent(){
    drawContent();
    update();
}

void KisTimelineCells::drawContent(){
    QPainter painter(this);

    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::darkGray);
    painter.drawRect(QRect(0,0,width(), height()));

    for(int i = 0; i < this->m_layerCount; i++){
        if(m_type == "tracks"){
            this->paintTrack(painter, i, m_offsetX, getLayerY(i), width()-m_offsetX, getLayerHeight(), false, m_frameSize);
        }
        if(m_type == "layers"){
            this->paintLabel(painter, i, 0, getLayerY(i), getLayerHeight(), width() - 1, false, m_layerCount);
        }
    }
}

void KisTimelineCells::paintLabel(QPainter &painter, int layer, int x, int y, int height, int width, bool selected, int allLayers){
    painter.setPen(Qt::darkGray);
    painter.setBrush(Qt::lightGray);
    painter.drawRect(x, y, width, height);
}

void KisTimelineCells::paintTrack(QPainter &painter, int layer, int x, int y, int width, int height, bool selected, int frameSize){

}

void KisTimelineCells::paintEvent(QPaintEvent *event){
    //QPainter painter(this);
    //painter.setPen(Qt::red);
    //painter.drawRect(0, 0, 25, 25);
    this->drawContent();
}

void KisTimelineCells::resizeEvent(QResizeEvent *event){

}

void KisTimelineCells::mousePressEvent(QMouseEvent *event){

}

void KisTimelineCells::mouseMoveEvent(QMouseEvent *event){

}

void KisTimelineCells::mouseReleaseEvent(QMouseEvent *event){

}

void KisTimelineCells::mouseDoubleClickEvent(QMouseEvent *event){

}

void KisTimelineCells::lengthChange(QString){

}

void KisTimelineCells::frameSizeChange(int){

}

void KisTimelineCells::fontSizeChange(int){

}

void KisTimelineCells::scrubChange(int){

}

void KisTimelineCells::labelChange(int){

}

void KisTimelineCells::hScrollChange(int){

}

void KisTimelineCells::vScrollChange(int){

}
