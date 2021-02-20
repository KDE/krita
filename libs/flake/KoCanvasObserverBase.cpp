/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoCanvasObserverBase.h"
#include <QPointer>
#include <KoCanvasBase.h>

class KoCanvasObserverBasePrivate
{
public:
    KoCanvasObserverBasePrivate()
        : canvas(0)
    {}

    ~KoCanvasObserverBasePrivate()
    {}

    QPointer<KoCanvasBase> canvas;
};

KoCanvasObserverBase::KoCanvasObserverBase()
    : d(new KoCanvasObserverBasePrivate)
{
}

KoCanvasObserverBase::~KoCanvasObserverBase()
{
    delete d;
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

KoCanvasBase* KoCanvasObserverBase::observedCanvas() const
{
    return d->canvas;
}
