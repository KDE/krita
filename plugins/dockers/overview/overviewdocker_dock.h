/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
