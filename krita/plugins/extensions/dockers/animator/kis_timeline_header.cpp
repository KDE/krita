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

#include "kis_timeline_header.h"

#include <QPainter>
#include <QString>

KisTimelineHeader::KisTimelineHeader(KisFrameBox *parent)
{
    this->setFixedHeight(20);
    this->setFixedWidth(10000);
    this->setParent(parent);
}

void KisTimelineHeader::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    for(int i = 0; i < 1000; i++) {
        if(i % 5 == 0){
            painter.drawText(10 * i, 15, QString("%1").arg(i));
        }
    }
}
