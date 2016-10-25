/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef LIBKIS_DOCKWIDGET_H
#define LIBKIS_DOCKWIDGET_H

#include <QDockWidget>

#include "kritalibkis_export.h"
#include "libkis.h"

#include <KoCanvasObserverBase.h>

class KoCanvasBase;

/**
 * DockWidget
 */
class KRITALIBKIS_EXPORT DockWidget : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
    Q_DISABLE_COPY(DockWidget)

public:
    explicit DockWidget();
    virtual ~DockWidget();

protected Q_SLOTS: // Krita API

    virtual void setCanvas(KoCanvasBase* canvas);
    virtual void unsetCanvas();

protected Q_SLOTS: // PyKRita API

    Canvas* canvas() const;
    virtual void canvasChanged(Canvas *canvas) = 0;

private:
    struct Private;
    Private *const d;

};

#endif // LIBKIS_DOCKWIDGET_H
