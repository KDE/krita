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
    static constexpr int showControlsTimerDuration {500};
    // Hou much the cursor has to move to prevent the showing animation
    static constexpr double showControlsAreaRadius {4.0};
    static constexpr double showControlsAnimationDuration {150.0};
    static constexpr double touchDragDistance {8.0};
    static constexpr double touchDragDistanceSquared {touchDragDistance * touchDragDistance};

    QVBoxLayout *m_controlsLayout {nullptr};
    QHBoxLayout *m_controlsSecondRowLayout {nullptr};
    QWidget *m_page {nullptr};
    OverviewWidget *m_overviewWidget {nullptr};
    QWidget *m_controlsContainer {nullptr};
    QWidget *m_zoomSlider {nullptr};
    KisAngleSelector *m_rotateAngleSelector {nullptr};
    QToolButton *m_mirrorCanvas {nullptr};
    QToolButton *m_pinControlsButton {nullptr};
    QPointer<KisCanvas2> m_canvas;
    bool m_pinControls {true};
    bool m_cursorIsHover {false};
    bool m_isTouching {false};
    bool m_isDraggingWithTouch {false};
    int m_touchPointId {0};
    QPointF m_lastTouchPos;
    mutable QVariantAnimation m_showControlsAnimation;
    mutable QTimer m_showControlsTimer;
    mutable bool m_areControlsHidden {false};
    QPointF m_lastOverviewMousePos;
    double m_cumulatedMouseDistanceSquared {0.0};

    void layoutMainWidgets();

private Q_SLOTS:
    void showControls(int delay) const;
    void hideControls(int delay) const;

    void on_overviewWidget_signalDraggingStarted();
    void on_overviewWidget_signalDraggingFinished();
};


#endif
