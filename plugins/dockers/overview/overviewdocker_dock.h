/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _OVERVIEW_DOCK_H_
#define _OVERVIEW_DOCK_H_

#include <QPointer>
#include <QDockWidget>
#include <QVariantAnimation>

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
    ~OverviewDockerDock() override;
    QString observerName() override { return "OverviewDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

public Q_SLOTS:
    void mirrorUpdateIcon();
    void rotateCanvasView(qreal rotation);
    void updateSlider();
    void setPinControls(bool pin);

protected:
    void resizeEvent(QResizeEvent*) override;
    void enterEvent(QEvent*) override;
    void leaveEvent(QEvent*) override;
    bool event(QEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    static constexpr int showControlsTimerDuration{200};
    // Hou much the cursor has to move to prevent the showing animation
    static constexpr double showControlsAreaRadius{4.0};
    static constexpr double showControlsAnimationDuration{150.0};

    QVBoxLayout *m_controlsLayout;
    QHBoxLayout *m_controlsSecondRowLayout;
    QWidget *m_page;
    OverviewWidget *m_overviewWidget;
    QWidget *m_controlsContainer;
    QWidget *m_zoomSlider;
    KisAngleSelector *m_rotateAngleSelector;
    QToolButton *m_mirrorCanvas;
    QToolButton *m_pinControlsButton;
    QPointer<KisCanvas2> m_canvas;
    bool m_pinControls;
    bool m_cursorIsHover;
    mutable QVariantAnimation m_showControlsAnimation;
    QTimer m_showControlsTimer;
    mutable bool m_areControlsHidden;
    QPointF m_lastOverviewMousePos;
    double m_cumulatedMouseDistanceSquared;

    void layoutMainWidgets();

private Q_SLOTS:
    void showControls() const;
    void hideControls() const;

    void on_overviewWidget_signalDraggingFinished();
};


#endif
