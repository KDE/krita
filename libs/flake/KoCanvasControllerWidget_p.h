/* This file is part of the KDE project
 *
 * Copyright (C) 2006, 2008-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (C) 2006, 2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007-2010 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2007 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
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
#ifndef KoCanvasControllerWidget_p_h
#define KoCanvasControllerWidget_p_h

#include "KoCanvasControllerWidget.h"

#include "KoCanvasControllerWidgetViewport_p.h"
#include "KoShape.h"
#include "KoViewConverter.h"
#include "KoCanvasBase.h"
#include "KoCanvasObserverBase.h"
#include "KoCanvasSupervisor.h"
#include "KoToolManager_p.h"

#include <ksharedconfig.h>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QEvent>
#include <QDockWidget>
#include <QTimer>

#include <KoConfig.h>

#ifdef HAVE_OPENGL
#include <QGLWidget>
#endif


class KoCanvasControllerWidget::Private
{
public:

    Private(KoCanvasControllerWidget *qq)
        : q(qq)
        , canvas(0)
        , lastActivatedCanvas(0)
        , ignoreScrollSignals(false)
        , zoomWithWheel(false)
        , vastScrollingFactor(0)
    {
    }

    /**
     * Gets called by the tool manager if this canvas controller is the current active canvas controller.
     */
    void setDocumentOffset();

    void resetScrollBars();
    void emitPointerPositionChangedSignals(QEvent *event);

    void activate();
    void unsetCanvas();

    KoCanvasControllerWidget *q;
    KoCanvasBase *canvas;
    KoCanvasBase *lastActivatedCanvas;
    Viewport *viewportWidget;
    bool ignoreScrollSignals;
    bool zoomWithWheel;
    qreal vastScrollingFactor;
};

#endif