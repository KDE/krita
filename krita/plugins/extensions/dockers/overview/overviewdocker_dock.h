/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

#ifndef _OVERVIEW_DOCK_H_
#define _OVERVIEW_DOCK_H_

#include <QDockWidget>
#include <QTimer>

#include <KoCanvasObserverBase.h>

class KisCanvas2;
class QLabel;

class OverviewDockerDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    OverviewDockerDock( );
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas() { m_canvas = 0; }
public slots:
    void kickTimer();
    void startUpdateCanvasProjection();
protected:
    void resizeEvent(QResizeEvent *);
private:
    QLabel *m_preview;
    KisCanvas2* m_canvas;
    QTimer m_delayTimer;
};


#endif
