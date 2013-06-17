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

#include "kis_layer_contents.h"
#include <QPainter>

KisLayerContents::KisLayerContents(KisFrameBox *parent)
{
    this->setFixedHeight(20);
    this->setFixedWidth(2000);
    this->setParent(parent);
}

void KisLayerContents::paintEvent(QPaintEvent *event){
    QPainter painter(this);

    painter.setPen(Qt::lightGray);

    for(int i = 0; i < 200; i++){
        if(i%10 == 0){
            painter.setPen(Qt::red);
            painter.drawRect(QRectF(10*i,0,9,height()-1));
        }
        else{
            painter.setPen(Qt::lightGray);
            painter.drawRect(QRectF(10*i,0,10,height()-1));
        }

    }
}
