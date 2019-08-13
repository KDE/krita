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

#include <FlakeDebug.h>

#include <KoConfig.h>
#include "KoCanvasSupervisor.h"

class KoCanvasControllerWidget;
class Viewport;
class KoCanvasBase;

class Q_DECL_HIDDEN KoCanvasControllerWidget::Private
{
public:

    Private(KoCanvasControllerWidget *qq, KoCanvasSupervisor *observerProvider)
        : q(qq)
        , observerProvider(observerProvider)
        , canvas(0)
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
    KoCanvasSupervisor *observerProvider;
    QPointer<KoCanvasBase> canvas;
    Viewport *viewportWidget;
    bool ignoreScrollSignals;
    bool zoomWithWheel;
    qreal vastScrollingFactor;
};

#endif
