/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _GRID_DOCK_H_
#define _GRID_DOCK_H_

#include <QDockWidget>
#include <KoCanvasObserverBase.h>

class QVBoxLayout;
class KisCanvas2;
class GridConfigWidget;

class GridDockerDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    GridDockerDock();
    QString observerName() { return "GridDockerDock"; }
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();

private:
    GridConfigWidget *m_configWidget;
    KisCanvas2 *m_canvas;
};


#endif
