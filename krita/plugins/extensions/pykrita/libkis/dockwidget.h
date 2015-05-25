/*
 *  Copyright (c) 2015 Cyrille Berger <cberger@cberger.net>
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
#include <KoCanvasObserverBase.h>

#include "libkis_export.h"

class LIBKIS_EXPORT DockWidget : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    DockWidget();
    virtual ~DockWidget();
protected:
    virtual void setCanvas(KoCanvasBase* canvas);
    virtual void unsetCanvas();
};

#endif
