#include "kis_timeline.h"

KisTimeline::KisTimeline(qreal height, QGraphicsItem *parent, QGraphicsScene *scene, qreal zoom, qreal linePer, qreal step, qreal stepPer) : QGraphicsItem() {
    m_currentScene = scene;
    m_height = height;
    m_zoom = zoom;
    m_linePer = linePer;
    m_step = step;
    m_stepPer = stepPer;
    setAcceptHoverEvents(true);
}

KisTimeline::~KisTimeline(){

}

QRectF KisTimeline::boundingRect() const{
    return QRectF(0, 0, m_currentScene->width(), m_height);
}

QPainterPath KisTimeline::shape() const{
    QPainterPath path;
    path.addRect(0, 0, m_currentScene->width(), m_height);
    return path;
}

void KisTimeline::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->setFont(QFont("", 7));

    for(qreal x = m_currentScene->sceneRect().left(); x < m_currentScene->sceneRect().width(); x+=m_step){
        qreal z = x*m_zoom;

        if(qreal(x/(m_stepPer*100)) == int(x/(m_stepPer*100))){
            painter->setPen(Qt::red);
            painter->drawLine(z, 1, z, 3);
            painter->drawLine(z, m_height -4, z, m_currentScene->sceneRect().height());
        }
        else{
            painter->setPen(Qt::gray);
            painter->drawLine(z,0,z,m_currentScene->sceneRect().height());
        }
    }

    for(qreal x = m_currentScene->sceneRect().left(); x < m_currentScene->sceneRect().width(); x+=m_stepPer){
        painter->drawText((x*100*m_zoom-19 > 1 ? x*100*m_zoom-19 : x*100*m_zoom-16), 0, 38, m_height, Qt::AlignCenter, QString("%1").arg(x*m_linePer/m_stepPer));
    }
}
