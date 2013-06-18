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

#include "kis_animation_frame.h"
#include <QPainter>

KisAnimationFrame::KisAnimationFrame(KisLayerContents *parent, int type, int width)
{
    this->m_type = type;
    this->m_width = width;
    this->setParent(parent);
    this->m_parent = parent;
}

void KisAnimationFrame::paintEvent(QPaintEvent *event){

    QPainter painter(this);

    if(this->m_type == KisAnimationFrame::BLANKFRAME){
        painter.setPen(Qt::green);
        painter.drawRect(0,0, this->m_width-1, 19);
        painter.drawEllipse(QPointF(5,10),3,3);
    }

    if(this->m_type == KisAnimationFrame::KEYFRAME){
        painter.setPen(Qt::green);
        painter.drawRect(0,0,this->m_width-1, 19);
        painter.setBrush(Qt::green);
        painter.drawEllipse(QPointF(5,10),3,3);
    }

    if(this->m_type == KisAnimationFrame::SELECTION){
        painter.setPen(Qt::blue);
        painter.setBrush(Qt::blue);
        painter.drawRect(0,0,this->m_width-1, 19);
    }
}

int KisAnimationFrame::getWidth(){
    return m_width;
}

void KisAnimationFrame::setWidth(int width){
    this->m_width = width;
}

KisLayerContents* KisAnimationFrame::getParent(){
    return this->m_parent;
}

int KisAnimationFrame::getType(){
    return this->m_type;
}

void KisAnimationFrame::setType(int type){
    this->m_type = type;
}

void KisAnimationFrame::convertSelectionToFrame(int type){
    if(this->getType() == KisAnimationFrame::SELECTION){
        this->setType(type);
        this->repaint();
    }
}
