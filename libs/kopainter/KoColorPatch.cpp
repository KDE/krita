/**
 * Copyright (c) 2006 Casper Boemann (cbr@boemann.dk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include <QPainter>

#include "KoColorPatch.h"

KoColorPatch::KoColorPatch( QWidget *parent ) : QFrame( parent )
{
    setFrameStyle( QFrame::Panel | QFrame::Sunken );
}

KoColorPatch::~KoColorPatch()
{
}

void KoColorPatch::setColor(const KoColor c)
{
    m_color = c;

    update();
}

void KoColorPatch::paintEvent ( QPaintEvent* pe )
{
    QColor qc;
    m_color.toQColor(&qc);

    QFrame::paintEvent(pe);
    QPainter painter( this );
    painter.setPen(qc);
    painter.setBrush(QBrush(qc));
    painter.drawRect(contentsRect());
}

#include "KoColorPatch.moc"
