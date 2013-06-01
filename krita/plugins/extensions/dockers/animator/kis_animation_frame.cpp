#include "kis_animation_frame.h"

KisAnimationFrame::KisAnimationFrame(int x, int y, int width, int height, int type) : QGraphicsItem()
{
    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;
    m_type = type;
}

KisAnimationFrame::~KisAnimationFrame(){

}

QRectF KisAnimationFrame::boundingRect() const{
    return QRectF(m_x, m_y, m_width, m_height);
}

QPainterPath KisAnimationFrame::shape() const{
    QPainterPath path;
    path.addRect(m_x, m_y, m_width, m_height);
    return path;
}

void KisAnimationFrame::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    //TODO: Replace with switch
    if(this->getFrameType() == SELECTEDFRAME){
        painter->fillRect(boundingRect(), QBrush(Qt::blue));
    }

    if(this->getFrameType() == KEYFRAME){
        painter->fillRect(boundingRect(), QBrush(Qt::gray));
        QBrush brush(Qt::red);
        painter->setBrush(brush);
        painter->drawEllipse(this->getX()+2,this->getY()+7,5,5);
    }

    if(this->getFrameType() == BLANKFRAME){
        painter->fillRect(boundingRect(), QBrush(Qt::gray));
        painter->setPen(QPen(Qt::red));
        painter->drawEllipse(this->getX()+2,this->getY()+7,5,5);
    }

    if(this->getFrameType() == CONTINUEFRAME){
        painter->fillRect(boundingRect(), QBrush(Qt::gray));
    }
}

int KisAnimationFrame::getFrameType(){
    return m_type;
}

int KisAnimationFrame::getX(){
    return m_x;
}

int KisAnimationFrame::getY(){
    return m_y;
}

KisAnimationFrame* KisAnimationFrame::getNextFrame(){
    return m_nextFrame;
}

KisAnimationFrame* KisAnimationFrame::getPreviousFrame(){
    return m_previousFrame;
}

void KisAnimationFrame::setNextFrame(KisAnimationFrame *nextFrame){
    m_nextFrame = nextFrame;
}

void KisAnimationFrame::setPreviousFrame(KisAnimationFrame *previousFrame){
    m_previousFrame = previousFrame;
}
