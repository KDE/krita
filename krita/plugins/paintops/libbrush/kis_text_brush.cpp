/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_text_brush.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFontMetrics>
#include <QPainter>
#include <QPixmap>



void KisTextBrush::toXML(QDomDocument& doc, QDomElement& e) const
{
    Q_UNUSED(doc);

    e.setAttribute("brush_type", "kis_text_brush");
    e.setAttribute("brush_spacing", spacing());
    e.setAttribute("text_brush_text", m_txt);
    e.setAttribute("text_brush_font", m_font.toString());
}

void KisTextBrush::updateBrush()
{
    QFontMetrics metric(m_font);
    int w = metric.width(m_txt);
    int h = metric.height();
    QPixmap px(w, h);
    QPainter p;
    p.begin(&px);
    p.setFont(m_font);
    p.fillRect(0, 0, w, h, Qt::black);
    p.setPen(Qt::white);
    p.drawText(0, metric.ascent(), m_txt);
    p.end();
    setImage(px.toImage());
    setValid(true);
}

