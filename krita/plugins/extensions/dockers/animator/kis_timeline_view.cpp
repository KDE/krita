#include "kis_timeline_view.h"

KisTimelineView::KisTimelineView(QWidget *parent) : QGraphicsView(parent)
{
    setFrameShape(QFrame::Box);
    setFrameShadow(QFrame::Raised);
    setRenderHint(QPainter::Antialiasing, false);

    m_timelineScene = new QGraphicsScene;
    setScene(m_timelineScene);
    setFixedHeight(100);
}

KisTimelineView::~KisTimelineView(){

}

void KisTimelineView::init(){
    m_timelineScene->setSceneRect(0,0,m_numberOfFrames*10, 90);
    m_tl = new KisTimeline(17, 0, m_timelineScene, 1.0, 10.0, 10.0, 1.0);
    m_timelineScene->addItem(m_tl);
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
    this->removePreviousSelection();
    int x = event->x();
    int y = event->y();
    QPointF f = this->mapToScene(x, y);
    x = (int)f.x() - ((int)f.x()%10);
    y = (int)f.y() - ((int)f.y()%20);
    m_selectedFrame = new KisAnimationFrame(x, y, 10, 20, KisAnimationFrame::SELECTEDFRAME);
    m_timelineScene->addItem(m_selectedFrame);
}

void KisTimelineView::removePreviousSelection(){
    m_timelineScene->removeItem(m_selectedFrame);
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
