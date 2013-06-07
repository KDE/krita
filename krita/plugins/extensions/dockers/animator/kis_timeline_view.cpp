/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_timeline_view.h"
#include <kdebug.h>

KisTimelineView::KisTimelineView(QWidget *parent) : QGraphicsView(parent)
{
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Raised);
    setRenderHint(QPainter::Antialiasing, false);
    m_selectedFrame = new KisAnimationFrame(0, 20, 10, 20, KisAnimationFrame::SELECTEDFRAME);
    m_timelineScene = new QGraphicsScene;
    setScene(m_timelineScene);
    m_timelineScene->addItem(m_selectedFrame);
    setFixedHeight(100);
}

KisTimelineView::~KisTimelineView(){

}

void KisTimelineView::init(){
    m_timelineScene->setSceneRect(0,0,m_numberOfFrames*10, 90);
    m_tl = new KisTimeline(17, 0, m_timelineScene, 1.0, 10.0, 10.0, 1.0);
    m_timelineScene->addItem(m_tl);

    //TODO: every time init is called data will be lost
    layerHeads = new QList<KisAnimationFrame*>();
    layerTails = new QList<KisAnimationFrame*>();

    //Add the default empty frame
    KisAnimationFrame* firstFrame = new KisAnimationFrame(0, 20, 10, 20, KisAnimationFrame::BLANKFRAME);
    layerHeads->append(firstFrame);
    layerTails->append(firstFrame);
    firstFrame->setPreviousFrame(NULL);
    firstFrame->setNextFrame(NULL);
    m_timelineScene->addItem(firstFrame);
}

QSize KisTimelineView::sizeHint() const{
    return QSize(800,100);
}

void KisTimelineView::setNumberOfFrames(int val){
    this->m_numberOfFrames = val;
    m_timelineScene->clear();
    this->init();
}

void KisTimelineView::mousePressEvent(QMouseEvent *event){
    if(m_selectedFrame){
        this->removePreviousSelection();
    }
    int x = event->x();
    int y = event->y();
    QPointF f = this->mapToScene(x, y);

    if(y < 20){
        return;
    }
    x = (int)f.x() - ((int)f.x()%10);
    y = (int)f.y() - ((int)f.y()%20);
    m_selectedFrame = new KisAnimationFrame(x, y, 10, 20, KisAnimationFrame::SELECTEDFRAME);
    m_timelineScene->addItem(m_selectedFrame);
}

void KisTimelineView::removePreviousSelection(){
        m_timelineScene->removeItem(m_selectedFrame);
}

void KisTimelineView::addFrame(){

    if(!m_selectedFrame){
        return;
    }

    int x, y;

    x = m_selectedFrame->getX();
    y = m_selectedFrame->getY();

    KisAnimationFrame* previousFrame = getPreviousFrameFrom(x, y);
    int oldx, oldy;
    oldx = previousFrame->getX();
    oldy = previousFrame->getY();
    int type = previousFrame->getFrameType();
    int  w = (x - oldx) +10;
    m_timelineScene->removeItem(previousFrame);
    kWarning() << "Removed";
    kWarning() << oldx;
    kWarning() << oldy;
    kWarning() << w;

    previousFrame = new KisAnimationFrame(oldx, oldy, w, 20, type);
    m_timelineScene->addItem(previousFrame);
}

void KisTimelineView::addKeyFrame(){
    int x,y;
    if(m_selectedFrame){
        x = m_selectedFrame->getX();
        y = m_selectedFrame->getY();
    }
    else{
        return;
    }

    if(getPreviousFrameFrom(x,y)){
        if(getPreviousFrameFrom(x,y)->getFrameType() == KisAnimationFrame::BLANKFRAME){
            addBlankFrame();
            return;
        }
    }

    KisAnimationFrame* newKeyFrame = new KisAnimationFrame(x,y,10,20,KisAnimationFrame::KEYFRAME);
    m_timelineScene->addItem(newKeyFrame);
}

void KisTimelineView::addBlankFrame(){
    int x,y;
    if(m_selectedFrame){
        x = m_selectedFrame->getX();
        y = m_selectedFrame->getY();
    }
    else{
        return;
    }
    KisAnimationFrame* newBlankFrame = new KisAnimationFrame(x,y,10,20,KisAnimationFrame::BLANKFRAME);
    m_timelineScene->addItem(newBlankFrame);
}

KisAnimationFrame *KisTimelineView::getPreviousFrameFrom(int x, int y){
    int layer = y/20 - 1;

    if(layer >= layerHeads->length()){
        return NULL;
    }

    KisAnimationFrame* currentFrame = layerHeads->at(layer);

    if(currentFrame){
        while(currentFrame->getX() <= x && currentFrame->getNextFrame()!= NULL){
            currentFrame = currentFrame->getNextFrame();
        }
    }
    return currentFrame;
}

KisAnimationFrame* KisTimelineView::getNextFrameFrom(int x, int y){
    int layer = y/20 - 1;

    if(layer >= layerHeads->length()){
        return NULL;
    }

    KisAnimationFrame* currentFrame = layerTails->at(layer);

    if(currentFrame){
        while(currentFrame->getX() >= x && currentFrame->getPreviousFrame()!= NULL){
            currentFrame = currentFrame->getPreviousFrame();
        }
    }
    return currentFrame;
}
