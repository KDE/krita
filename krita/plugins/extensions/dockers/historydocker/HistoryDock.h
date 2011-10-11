/* This file is part of the KDE project
 * Copyright (C) 2010 Matus Talcik <matus.talcik@gmail.com>
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
#ifndef _HISTORY_DOCK_H_
#define _HISTORY_DOCK_H_

#include <QDockWidget>
#include "KisUndoView.h"

#include <KoCanvasObserverBase.h>
#include <klocale.h>
#include <kundo2stack.h>

#include <KoShapeController.h>
#include <KoCanvasBase.h>
#include "kis_types.h"
#include "kis_canvas2.h"
#include "kis_view2.h"
#include "kis_image.h"
#include "kis_paint_device.h"

class HistoryDock : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    HistoryDock();

    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas() { historyCanvas = 0; }
private:
    KisUndoView* undoView;

    KoCanvasBase* historyCanvas;
};


#endif
