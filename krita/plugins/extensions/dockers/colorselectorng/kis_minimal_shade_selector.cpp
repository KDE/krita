/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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

#include "kis_minimal_shade_selector.h"

#include <QPainter>

KisMinimalShadeSelector::KisMinimalShadeSelector(QWidget *parent) :
    QWidget(parent)
{
    setMinimumHeight(30);
    setMaximumHeight(30);
}

void KisMinimalShadeSelector::paintEvent(QPaintEvent *) {
    QPainter painter(this);

    QLinearGradient g1(0,0, width(), 0);
    g1.setColorAt(0, QColor(100,0,0));
    g1.setColorAt(0.5, QColor(255,0,0));
    g1.setColorAt(1, QColor(255,155,155));

    QLinearGradient g2(0,0, width(), 0);
    g2.setColorAt(0, QColor(155, 100, 0));
    g2.setColorAt(0.5, QColor(255, 0, 0));
    g2.setColorAt(1, QColor(155, 0, 100));

    painter.fillRect(0,0,width(), height()/2, QBrush(g1));
    painter.fillRect(0, height()/2, width(), height()/2, QBrush(g2));
}
