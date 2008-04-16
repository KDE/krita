/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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

#include "widgets/kis_preset_widget.h"

#include <QPainter>
#include <QIcon>
#include <QWidget>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_paintop_preset.h>
#include <kis_paint_device.h>
#include <kis_painter.h>

KisPresetWidget::KisPresetWidget(QWidget *parent, const char *name)
    : KisPopupButton(parent)
{
    setObjectName(name);
    m_preset = 0;
    m_canvas = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8(), "preset preview canvas");
}

void KisPresetWidget::slotSetPaintOp( const KoID & paintOp )
{
    m_paintOp = paintOp;
}


void KisPresetWidget::slotSetItem( KisPaintOpPreset * preset )
{
    m_preset = preset;
    update();
}

void KisPresetWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    qint32 cw = width();
    qint32 ch = height();
    p.fillRect( 0, 0, cw, ch, Qt::white); // XXX: use a palette for this instead of white?

/*
    draw a sinus with min pressure at start, max pressure in the middle, min pressure at end
*/

    p.setPen(Qt::gray);
    p.drawRect(0, 0, cw + 1, ch + 1);
}


#include "kis_preset_widget.moc"
