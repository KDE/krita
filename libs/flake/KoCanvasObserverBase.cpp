/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoCanvasObserverBase.h"

class KoCanvasObserverBasePrivate
{
public:
    KoCanvasObserverBasePrivate() : canvas(0) {}
    ~KoCanvasObserverBasePrivate() {}

    KoCanvasBase* canvas;
};

KoCanvasObserverBase::KoCanvasObserverBase() : d(new KoCanvasObserverBasePrivate)
{
}

KoCanvasObserverBase::~KoCanvasObserverBase()
{
}

void KoCanvasObserverBase::setObservedCanvas(KoCanvasBase* canvas)
{
    d->canvas = canvas;
    setCanvas(canvas);
}

void KoCanvasObserverBase::unsetObservedCanvas()
{
    d->canvas = 0;
    unsetCanvas();
}

KoCanvasBase* KoCanvasObserverBase::observedCanvas()
{
    return d->canvas;
}
