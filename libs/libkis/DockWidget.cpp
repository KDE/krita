/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "DockWidget.h"
#include <QDebug>

#include <KoCanvasBase.h>

#include "Canvas.h"

struct DockWidget::Private {
    Private() {}

    Canvas *canvas {0};
};

DockWidget::DockWidget()
    : KDDockWidgets::DockWidget()
    , d(new Private)
{
}

DockWidget::~DockWidget()
{
    delete d;
}

Canvas* DockWidget::canvas() const
{
    return d->canvas;
}

void DockWidget::setCanvas(KoCanvasBase* canvas)
{
    delete d->canvas;
    d->canvas = new Canvas(canvas);
    canvasChanged(d->canvas);
}

void DockWidget::unsetCanvas()
{
    canvasChanged(0);
    delete d->canvas;
    d->canvas = 0;
}
