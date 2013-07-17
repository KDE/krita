/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version..
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

#include "kis_opacity_selector.h"
#include <QPainter>

KisOpacitySelector::KisOpacitySelector(int x, int y, int width, int height, QGraphicsScene *scene, int frames)
{
    m_x = x;
    m_y = y;
    m_width = width;
    m_height = height;
    m_frames = frames;
    this->setPos(m_x, m_y);
    QList<int> l;
    for(int i = 0; i < frames; i++){
        l.append((i*50)/frames);
    }
    this->setOpacityValue(l);
    setAcceptHoverEvents(true);
}

KisOpacitySelector::~KisOpacitySelector(){

}

void KisOpacitySelector::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    int step = m_width / m_frames;
    int j = 0;
    int opacityVal = 0;
    int nextOpacityVal = 0;
    for(int i = 0; i < m_width; i+=step){
        painter->setPen(Qt::gray);
        painter->drawLine(i,0,i,m_height);
        painter->setPen(Qt::green);
        //To fix assertion
        if(j < m_opacityValues->length()){
            opacityVal = m_opacityValues->at(j);
        }else{
            opacityVal = 0;
        }

        if(j < m_opacityValues->length()-1){
            nextOpacityVal = m_opacityValues->at(j+1);
        }else{
            nextOpacityVal = 0;
        }

        painter->drawEllipse(i+(step/2),m_height - (opacityVal*(m_height))/100, 2,2);
        if(j<m_opacityValues->length()-1)
            painter->drawLine(i+(step/2),m_height-(opacityVal*(m_height))/100,i+(3*step/2), m_height-(nextOpacityVal*(m_height))/100);
        j++;
    }
}

QRectF KisOpacitySelector::boundingRect() const{
    return QRectF(m_x, m_y, m_width, m_height);
}

QPainterPath KisOpacitySelector::shape() const{
    QPainterPath path;
    path.addRect(m_x, m_y, m_width, m_height);
    return path;
}

void KisOpacitySelector::setOpacityValue(QList<int> l){
    m_opacityValues = new QList<int>(l);
}

void KisOpacitySelector::setOpacityValue(int frame, int val){
    m_opacityValues->replace(frame, val);
    this->update(m_x, m_y, m_width, m_height);
}

QList<int>* KisOpacitySelector::getOpacityValues(){
    return m_opacityValues;
}
