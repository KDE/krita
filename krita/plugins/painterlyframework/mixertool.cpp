/* This file is part of the KDE project
   Made by Emanuele Tamponi (emanuele@valinor.it)
   Copyright (C) 2007 Emanuele Tamponi

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "mixertool.h"

#include "mixercanvas.h"

#include <kis_painter.h>
#include <kis_paintop_registry.h>
#include <kis_resource_provider.h>
#include <KoPointerEvent.h>
#include <QCursor>
#include <QRegion>
#include <QString>

MixerTool::MixerTool(MixerCanvas *mixer, KisResourceProvider *rp)
    : KisToolFreehand(mixer, QCursor(), "Mixer Wrapper Tool"), m_mixer(mixer), m_resources(rp)
{
    activate();
}

MixerTool::~MixerTool()
{

}

void MixerTool::initPaint(KoPointerEvent *e) {

    KisToolFreehand::initPaint(e);

    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();

    if (!m_painter)
        return;

    KisPaintOp *op = r->paintOp(currentPaintOp(), currentPaintOpSettings(), m_painter, 0);

    if (!op)
        return;

    m_painter->setPaintOp(op);

}

void MixerTool::endPaint() {

    KisToolFreehand::endPaint();

}

void MixerTool::setDirty(const QRegion& region) {

    KisToolFreehand::setDirty(region);
    m_mixer->updateCanvas(region);

}

#include "mixertool.moc"
