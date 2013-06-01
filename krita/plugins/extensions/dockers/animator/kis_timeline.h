#ifndef KIS_TIMELINE_H
#define KIS_TIMELINE_H

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QPainter>
#include <QDebug>
#include <QHelpEvent>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>

class KisTimeline : public QGraphicsItem
{
public:
    KisTimeline(qreal height, QGraphicsItem* parent = 0, QGraphicsScene *scene = 0, qreal zoom = 1.0, qreal linePer = 100.0, qreal step = 10.0, qreal stepPer = 100.0);
    ~KisTimeline();

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QGraphicsScene *m_currentScene;
    qreal m_width, m_height, m_zoom, m_linePer, m_step, m_stepPer;


};

#endif // KIS_TIMELINE_H
