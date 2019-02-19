/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#ifndef _OVERVIEW_DOCK_H_
#define _OVERVIEW_DOCK_H_

#include <QPointer>
#include <QDockWidget>
#include <kis_slider_spin_box.h>
#include <KoCanvasObserverBase.h>

#include <kis_canvas2.h>

class QVBoxLayout;
class QHBoxLayout;
class QToolButton;
class OverviewWidget;

class OverviewDockerDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    OverviewDockerDock();
    QString observerName() override { return "OverviewDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

public Q_SLOTS:
    void rotateCanvasView(qreal rotation);
    void updateSlider();

private:
    QVBoxLayout *m_layout;
    QHBoxLayout *m_horizontalLayout;
    OverviewWidget *m_overviewWidget;
    QWidget *m_zoomSlider;
    KisDoubleSliderSpinBox *m_rotateSlider;
    QToolButton *m_mirrorCanvas;
    QPointer<KisCanvas2> m_canvas;
};


#endif
