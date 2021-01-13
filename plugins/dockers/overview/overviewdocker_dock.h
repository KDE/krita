/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _OVERVIEW_DOCK_H_
#define _OVERVIEW_DOCK_H_

#include <QPointer>
#include <QDockWidget>
#include <KoCanvasObserverBase.h>

#include <kis_canvas2.h>

class QVBoxLayout;
class QHBoxLayout;
class QToolButton;
class OverviewWidget;
class KisAngleSelector;

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
    QVBoxLayout *m_bottomLayout;
    QHBoxLayout *m_horizontalLayout;
    OverviewWidget *m_overviewWidget;
    QWidget *m_zoomSlider;
    KisAngleSelector *m_rotateAngleSelector;
    QToolButton *m_mirrorCanvas;
    QPointer<KisCanvas2> m_canvas;
};


#endif
