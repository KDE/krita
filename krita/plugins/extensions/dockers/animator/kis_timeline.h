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
