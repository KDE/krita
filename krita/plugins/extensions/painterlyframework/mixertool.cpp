/* This file is part of the KDE project
 *
 * Copyright (C) 2007 Emanuele Tamponi <emanuele@valinor.it>
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#include "mixertool.h"

#include <QCursor>
#include <QRegion>
#include <QString>

#include <KoPointerEvent.h>
#include <KoColor.h>

#include <kis_debug.h>
#include <kis_painter.h>
#include <kis_paintop_registry.h>
#include <kis_canvas_resource_provider.h>
#include <kis_cursor.h>

#include "mixercanvas.h"

struct MixerTool::Private {
    
    Private(MixerCanvas* mixer)
        : mixer(mixer)
    {
        cursor = KisCursor::load("tool_freehand_cursor.png", 5, 5);
    }
    
    MixerCanvas *mixer;
    KoColor      foregroundColor;
    KoColor      backgroundColor;
    bool         mixing;
    QCursor      cursor;
};



MixerTool::MixerTool(MixerCanvas* mixer)
    : KoTool(mixer)
    , m_d( new Private(mixer))
{

    activate();
}

MixerTool::~MixerTool()
{
    delete m_d;
}

void MixerTool::activate(bool temporary)
{
    Q_UNUSED(temporary)
    m_d->mixing = false;

    useCursor(m_d->cursor);
    m_d->foregroundColor = canvas()->resourceProvider()->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
    m_d->backgroundColor = canvas()->resourceProvider()->resource(KoCanvasResource::BackgroundColor).value<KoColor>();
}

void MixerTool::deactivate()
{
}

void MixerTool::resourceChanged(int key, const QVariant & res)
{
    switch (key) {
    case(KoCanvasResource::ForegroundColor):
        m_d->foregroundColor = res.value<KoColor>();
        break;
    case(KoCanvasResource::BackgroundColor):
        m_d->backgroundColor = res.value<KoColor>();
        break;
    default:
        ;
    }
}

void MixerTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(painter);
    Q_UNUSED(converter);

    // XXX: paint a sample circle instead?
}

void MixerTool::mousePressEvent(KoPointerEvent *event)
{
    Q_UNUSED(event);
    m_d->mixing = true;
}

void MixerTool::mouseMoveEvent(KoPointerEvent *event)
{
    Q_UNUSED(event);
    if (m_d->mixing) {

    }
}

void MixerTool::mouseReleaseEvent(KoPointerEvent *event)
{
    // XXX: We want to be able to set a color source for the other paintops that
    //      contains the impure blend under the current cursor.
    m_d->mixer->resourceProvider()->setResource(KoCanvasResource::ForegroundColor, event->pos());
    m_d->mixing = false;
}

void MixerTool::setDirty(const QRegion& region)
{
    m_d->mixer->updateCanvas(region);
}

#include "mixertool.moc"
