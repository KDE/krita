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

#include "kis_color_selector.h"

#include "kis_color_selector_ring.h"
#include "kis_color_selector_triangle.h"

#include <cmath>

#include <QHBoxLayout>
#include <QColor>
#include <QPainter>
#include <QMouseEvent>


#include <KDebug>

KisColorSelector::KisColorSelector(QWidget* parent) : KisColorSelectorBase(parent)
{
    m_ring = new KisColorSelectorRing(this);
    m_triangle = new KisColorSelectorTriangle(this);
    
    connect(m_ring, SIGNAL(hueChanged(int)), m_triangle, SLOT(setHue(int)));
    
    setMinimumSize(80, 80);
}

QColor KisColorSelector::pickColorAt(int x, int y)
{
    return QColor();
}

KisColorSelectorBase* KisColorSelector::createPopup() const
{
    KisColorSelectorBase* popup = new KisColorSelector(0);
    popup->resize(256,256);
    return popup;
}

void KisColorSelector::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(0,0,width(),height(),QColor(128,128,128));
}

void KisColorSelector::resizeEvent(QResizeEvent* e) {
    m_ring->setGeometry(0,0,width(),height());
    m_triangle->setGeometry(0,0,width(),height());
    m_triangle->setRadius(m_ring->innerRadius());
    
    QWidget::resizeEvent(e);
}

void KisColorSelector::mousePressEvent(QMouseEvent* e)
{
    m_ring->mousePressEvent(e);
    if(!e->isAccepted())
        m_triangle->mousePressEvent(e);
}
