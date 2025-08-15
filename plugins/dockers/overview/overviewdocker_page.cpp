/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "overviewdocker_dock.h"
#include "overviewwidget.h"
#include "overviewdocker_page.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QStatusBar>
#include <QApplication>

#include <KisAngleSelector.h>
#include <klocalizedstring.h>
#include "kis_canvas2.h"
#include <KisViewManager.h>
#include <kactioncollection.h>
#include <kis_action.h>
#include <kis_zoom_manager.h>
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_signal_compressor.h"
#include "kis_canvas_controller.h"
#include "kis_icon_utils.h"
#include "kis_signals_blocker.h"
#include <KoZoomWidget.h>
#include <kis_icon_utils.h>

#include <kconfiggroup.h>
#include <ksharedconfig.h>

OverviewDockerPage::OverviewDockerPage(QWidget *parent, const char *name, Qt::WindowFlags f)
    : QWidget(parent, f)
    , m_lastOverviewMousePos(0.0, 0.0)
{
    setObjectName(name);

    m_overviewWidget = new OverviewWidget(this);
    m_overviewWidget->setMinimumHeight(50);
    m_overviewWidget->setBackgroundRole(QPalette::Base);
    // paints background role before paint()
    m_overviewWidget->setAutoFillBackground(true);
    m_overviewWidget->setAttribute(Qt::WA_AcceptTouchEvents, true);
    m_overviewWidget->installEventFilter(this);
    connect(m_overviewWidget, SIGNAL(signalDraggingStarted()), SLOT(on_overviewWidget_signalDraggingStarted()));
    connect(m_overviewWidget, SIGNAL(signalDraggingFinished()), SLOT(on_overviewWidget_signalDraggingFinished()));

    m_controlsContainer = new QWidget(this);

    m_controlsLayout = new QVBoxLayout;
    m_controlsLayout->setContentsMargins(2, 2, 2, 2);
    m_controlsLayout->setSpacing(2);
    m_controlsContainer->setLayout(m_controlsLayout);

    m_showControlsTimer.setSingleShot(true);

    m_showControlsAnimation.setEasingCurve(QEasingCurve(QEasingCurve::InOutCubic));
    connect(&m_showControlsAnimation, &QVariantAnimation::valueChanged, this, &OverviewDockerPage::layoutMainWidgets);

    KConfigGroup config = KSharedConfig::openConfig()->group("OverviewDocker");
    m_pinControls = config.readEntry("pinControls", true);
    m_areControlsHidden = !m_pinControls;

    setEnabled(false);
}

OverviewDockerPage::~OverviewDockerPage()
{
    KConfigGroup config = KSharedConfig::openConfig()->group("OverviewDocker");
    config.writeEntry("pinControls", m_pinControls);
}

void OverviewDockerPage::setCanvas(KoCanvasBase * canvas)
{
    if(m_canvas == canvas)
        return;

    setEnabled(canvas != nullptr);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->image()->disconnect(this);
        m_canvas = nullptr;
    }

    if (m_zoomSlider) {
        m_controlsSecondRowLayout->removeWidget(m_zoomSlider);
        delete m_zoomSlider;
        m_zoomSlider = nullptr;
    }

    if (m_rotateAngleSelector) {
        m_controlsSecondRowLayout->removeWidget(m_rotateAngleSelector);
        delete m_rotateAngleSelector;
        m_rotateAngleSelector = nullptr;
    }

    if (m_mirrorCanvas) {
        m_controlsSecondRowLayout->removeWidget(m_mirrorCanvas);
        delete m_mirrorCanvas;
        m_mirrorCanvas = nullptr;
    }

    if (m_pinControlsButton) {
        m_controlsSecondRowLayout->removeWidget(m_pinControlsButton);
        delete m_pinControlsButton;
        m_pinControlsButton = nullptr;
    }

    // Delete the stretch
    while (m_controlsSecondRowLayout && m_controlsSecondRowLayout->count() && m_controlsSecondRowLayout->itemAt(0)->spacerItem()) {
        delete m_controlsSecondRowLayout->takeAt(0);
    }

    m_controlsLayout->removeItem(m_controlsSecondRowLayout);

    delete m_controlsSecondRowLayout;
    m_controlsSecondRowLayout = nullptr;

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    m_overviewWidget->setCanvas(m_canvas);
    if (m_canvas && m_canvas->viewManager()) {
        bool usePrintResolutionMode = m_canvas->imageView()->canvasController()->usePrintResolutionMode();
        m_zoomSlider = m_canvas->imageView()->zoomManager()->zoomAction()->createWidget(m_canvas->imageView()->KisView::statusBar());
        KoZoomWidget* zoomWidget = static_cast<KoZoomWidget*>(m_zoomSlider);
        zoomWidget->setZoomInputFlat(false);
        zoomWidget->setUsePrintResolutionMode(usePrintResolutionMode);
        m_controlsLayout->addWidget(m_zoomSlider);

        m_rotateAngleSelector = new KisAngleSelector();
        m_rotateAngleSelector->setRange(-360.00, 360.0);
        m_rotateAngleSelector->setAngle(m_canvas->rotationAngle());
        m_rotateAngleSelector->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);
        m_rotateAngleSelector->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_ContextMenu);
        connect(m_rotateAngleSelector, SIGNAL(angleChanged(qreal)), this, SLOT(rotateCanvasView(qreal)), Qt::UniqueConnection);
        connect(m_canvas->canvasController()->proxyObject, SIGNAL(documentRotationChanged(qreal)), this, SLOT(updateRotationSlider(qreal)));

        m_mirrorCanvas = new QToolButton();
        QList<QAction *> actions = m_canvas->viewManager()->actionCollection()->actions();
        Q_FOREACH(QAction* action, actions) {
            if (action->objectName()=="mirror_canvas") {
                m_mirrorCanvas->setDefaultAction(action);
            }
        }
        m_mirrorCanvas->setIcon(KisIconUtils::loadIcon("mirror-view-16"));
        m_mirrorCanvas->setAutoRaise(true);
        connect(m_mirrorCanvas, SIGNAL(toggled(bool)), this, SLOT(mirrorUpdateIcon()));

        m_pinControlsButton = new QToolButton;
        m_pinControlsButton->setCheckable(true);
        m_pinControlsButton->setChecked(m_pinControls);
        m_pinControlsButton->setToolTip(
            i18nc("Make the controls in the overview docker auto-hide or always visible", "Pin navigation controls")
        );
        m_pinControlsButton->setIcon(KisIconUtils::loadIcon("krita_tool_reference_images"));
        m_pinControlsButton->setAutoRaise(true);
        connect(m_pinControlsButton, SIGNAL(toggled(bool)), SLOT(setPinControls(bool)));

        m_controlsSecondRowLayout = new QHBoxLayout();

        m_controlsSecondRowLayout->addWidget(m_rotateAngleSelector);
        m_controlsSecondRowLayout->addStretch();
        m_controlsSecondRowLayout->addWidget(m_mirrorCanvas);
        m_controlsSecondRowLayout->addStretch();
        m_controlsSecondRowLayout->addWidget(m_pinControlsButton);
        m_controlsLayout->addLayout(m_controlsSecondRowLayout);

        //m_zoomSlider->setVisible(true);
        m_rotateAngleSelector->setVisible(true);

        // Show/hide the controls
        if (m_pinControls) {
            showControls(0);
        } else {
            if (m_cursorIsHover) {
                showControls(0);
            } else {
                hideControls(0);
            }
        }
    }
}

void OverviewDockerPage::unsetCanvas()
{
    setEnabled(false);
    m_canvas = nullptr;
    m_overviewWidget->setCanvas(0);
}

void OverviewDockerPage::mirrorUpdateIcon()
{
    if(!m_mirrorCanvas) return;
    m_mirrorCanvas->setIcon(KisIconUtils::loadIcon("mirror-view-16"));
}

void OverviewDockerPage::rotateCanvasView(qreal rotation)
{
    if (!m_canvas) return;
    KisCanvasController *canvasController =
            dynamic_cast<KisCanvasController*>(m_canvas->viewManager()->canvasBase()->canvasController());
    if (canvasController) {
        canvasController->rotateCanvas(rotation-m_canvas->rotationAngle());
    }
}

void OverviewDockerPage::updateRotationSlider(qreal angle)
{
    if (qAbs(m_rotateAngleSelector->angle() - angle) > 0.01) {
        KisSignalsBlocker l(m_rotateAngleSelector);
        m_rotateAngleSelector->setAngle(m_canvas->rotationAngle());
    }
}

void OverviewDockerPage::setPinControls(bool pin)
{
    m_pinControls = pin;
}

void OverviewDockerPage::resizeEvent(QResizeEvent*)
{
    layoutMainWidgets();
}

void OverviewDockerPage::leaveEvent(QEvent*)
{
    m_cursorIsHover = false;
    if (isEnabled() && !m_pinControls) {
        hideControls(0);
        m_cumulatedMouseDistanceSquared = 0.0;
    }
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
void OverviewDockerPage::enterEvent(QEvent*)
#else
void OverviewDockerPage::enterEvent(QEnterEvent*)
#endif
{
    m_cursorIsHover = true;
    if (isEnabled() && !m_pinControls) {
        showControls(showControlsTimerDuration);
    }
}

bool OverviewDockerPage::event(QEvent *e)
{
    if (e->type() == QEvent::StyleChange || e->type() == QEvent::FontChange) {
        resizeEvent(nullptr);
    }
    return QWidget::event(e);
}

bool OverviewDockerPage::eventFilter(QObject *o, QEvent *e)
{
    if (!isEnabled()) {
        return false;
    }

    if (o == m_overviewWidget) {
        // Filter out the mouse events if we are touching the overview widget
        // and the event was not synthesized in this function  from the touch events
        if (e->type() == QEvent::MouseButtonPress) {
            if (m_isTouching) {
                return static_cast<QMouseEvent*>(e)->source() != Qt::MouseEventSynthesizedByApplication;
            }

        } else if (e->type() == QEvent::MouseButtonRelease) {
            if (m_isTouching) {
                return static_cast<QMouseEvent*>(e)->source() != Qt::MouseEventSynthesizedByApplication;
            }

        } else if (e->type() == QEvent::MouseMove) {
            if (m_isTouching) {
                return static_cast<QMouseEvent*>(e)->source() != Qt::MouseEventSynthesizedByApplication;
            }
            if (!m_overviewWidget->isDragging() && m_areControlsHidden && !m_pinControls) {
                QMouseEvent *me = static_cast<QMouseEvent*>(e);
                constexpr double showControlsAreaRadiusSquared = showControlsAreaRadius * showControlsAreaRadius;
                const QPointF d = me->localPos() - m_lastOverviewMousePos;
                const double distanceSquared = d.x() * d.x() + d.y() * d.y();
                if (distanceSquared > m_cumulatedMouseDistanceSquared) {
                    if (distanceSquared >= showControlsAreaRadiusSquared) {
                        showControls(showControlsTimerDuration);
                        m_lastOverviewMousePos = me->localPos();
                        m_cumulatedMouseDistanceSquared = 0.0;
                    } else {
                        m_cumulatedMouseDistanceSquared = distanceSquared;
                    }
                }
            }

        } else if (e->type() == QEvent::TouchBegin) {
            if (!m_isTouching) {
                QTouchEvent *te = static_cast<QTouchEvent*>(e);
                m_isTouching = true;
                // Store the first touch point. We will only track this one
                m_touchPointId = te->touchPoints().first().id();
                m_lastTouchPos = te->touchPoints().first().pos();
            }
            // Accept the event so that other touch events keep coming
            e->accept();
            return true;

        } else if (e->type() == QEvent::TouchUpdate) {
            if (!m_isTouching) {
                return true;
            }
            QTouchEvent *te = static_cast<QTouchEvent*>(e);
            // Get the touch point position
            QPointF currentPosition;
            for (const QTouchEvent::TouchPoint &touchPoint : te->touchPoints()) {
                if (touchPoint.id() == m_touchPointId) {
                    // If the touch point wasn't moved, this event wasn't
                    // generated from our touch point
                    if (touchPoint.state() == Qt::TouchPointStationary) {
                        return true;
                    }
                    currentPosition = touchPoint.pos();
                    break;
                }
            }
            if (!m_isDraggingWithTouch) {
                // Compute distance
                const QPointF delta = currentPosition - m_lastTouchPos;
                const qreal distanceSquared = delta.x() * delta.x() + delta.y() * delta.y();
                if (distanceSquared >= touchDragDistanceSquared) {
                    m_isDraggingWithTouch = true;
                    // synthesize mouse press event
                    QMouseEvent *se = new QMouseEvent(QEvent::MouseButtonPress,
                                                      m_lastTouchPos, QPointF(), QPointF(),
                                                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier,
                                                      Qt::MouseEventSynthesizedByApplication);
                    qApp->sendEvent(m_overviewWidget, se);
                }
            }
            if (m_isDraggingWithTouch) {
                // Synthesize mouse move event
                QMouseEvent *se = new QMouseEvent(QEvent::MouseMove,
                                                  currentPosition, QPointF(), QPointF(),
                                                  Qt::LeftButton, Qt::LeftButton, Qt::NoModifier,
                                                  Qt::MouseEventSynthesizedByApplication);
                qApp->sendEvent(m_overviewWidget, se);
                // Update
                m_lastTouchPos = currentPosition;
            }
            return true;

        } else if (e->type() == QEvent::TouchEnd || e->type() == QEvent::TouchCancel) {
            if (!m_isTouching) {
                return true;
            }
            QTouchEvent *te = static_cast<QTouchEvent*>(e);
            if (e->type() == QEvent::TouchEnd) {
                // If the touch point is not in the released state
                // then this event wasn't generated from our touch point
                for (const QTouchEvent::TouchPoint &touchPoint : te->touchPoints()) {
                    if (touchPoint.id() == m_touchPointId) {
                        if (touchPoint.state() != Qt::TouchPointReleased) {
                            return true;
                        }
                        break;
                    }
                }
            }
            // If we are dragging then synthesize mouse release event.
            // Show/hide the controls otherwise
            if (m_isDraggingWithTouch) {
                QMouseEvent *se = new QMouseEvent(QEvent::MouseButtonRelease,
                                                  m_lastTouchPos, QPointF(), QPointF(),
                                                  Qt::LeftButton, Qt::LeftButton, Qt::NoModifier,
                                                  Qt::MouseEventSynthesizedByApplication);
                qApp->sendEvent(m_overviewWidget, se);
            } else if (e->type() == QEvent::TouchEnd) {
                m_pinControls = m_areControlsHidden;
                KisSignalsBlocker blocker(m_pinControlsButton);
                m_pinControlsButton->setChecked(m_pinControls);
                if (m_areControlsHidden) {
                    showControls(0);
                } else {
                    hideControls(0);
                }
            }
            // Reset
            m_isTouching = false;
            m_isDraggingWithTouch = false;
            return true;
        }
    }
    return false;
}

void OverviewDockerPage::layoutMainWidgets()
{
    this->setMinimumHeight(m_overviewWidget->minimumHeight() +
                             m_controlsContainer->minimumSizeHint().height());

    if (m_showControlsAnimation.state() == QVariantAnimation::Running) {
        const qreal pageHeight = static_cast<qreal>(this->height());
        const qreal controlsContainerHeight = static_cast<qreal>(m_controlsContainer->sizeHint().height());
        const qreal animationProgress = m_showControlsAnimation.currentValue().toReal();
        const int widgetLimitPosition = static_cast<int>(std::round(pageHeight - animationProgress * controlsContainerHeight));
        m_overviewWidget->setGeometry(0, 0, this->width(), widgetLimitPosition);
        m_controlsContainer->setGeometry(0, widgetLimitPosition, this->width(), static_cast<int>(controlsContainerHeight));
    } else {
        const int controlsContainerHeight = m_controlsContainer->sizeHint().height();
        if (m_pinControls || !m_areControlsHidden) {
            const int widgetLimitPosition = this->height() - controlsContainerHeight;
            m_overviewWidget->setGeometry(0, 0, this->width(), widgetLimitPosition);
            m_controlsContainer->setGeometry(0, widgetLimitPosition, this->width(), controlsContainerHeight);
        } else {
            m_overviewWidget->setGeometry(0, 0, this->width(), this->height());
            m_controlsContainer->setGeometry(0, this->height(), this->width(), controlsContainerHeight);
        }
    }
}

void OverviewDockerPage::showControls(int delay) const
{
    auto animFunction =
        [this]() -> void
        {
            int animationDuration;
            qreal animationStartValue;

            if (m_areControlsHidden) {
                if (m_showControlsAnimation.state() == QVariantAnimation::Running) {
                    m_showControlsAnimation.stop();
                    animationDuration =
                        static_cast<int>(std::round((1.0 - m_showControlsAnimation.currentValue().toReal()) * showControlsAnimationDuration));
                    animationStartValue = m_showControlsAnimation.currentValue().toReal();
                } else {
                    animationDuration = showControlsAnimationDuration;
                    animationStartValue = 0.0;
                }
            } else {
                animationDuration = 1;
                animationStartValue = 1.0;
            }

            m_areControlsHidden = false;
            m_showControlsAnimation.setStartValue(animationStartValue);
            m_showControlsAnimation.setEndValue(1.0);
            m_showControlsAnimation.setDuration(animationDuration);
            m_showControlsAnimation.start();
        };

    delay = qMax(delay, 0);

    m_showControlsTimer.disconnect();
    connect(&m_showControlsTimer, &QTimer::timeout, animFunction);
    m_showControlsTimer.start(delay);
}

void OverviewDockerPage::hideControls(int delay) const
{
    auto animFunction =
        [this]() -> void
        {
            int animationDuration;
            qreal animationStartValue;

            if (!m_areControlsHidden) {
                if (m_showControlsAnimation.state() == QVariantAnimation::Running) {
                    m_showControlsAnimation.stop();
                    animationDuration =
                        static_cast<int>(std::round(m_showControlsAnimation.currentValue().toReal() * showControlsAnimationDuration));
                    animationStartValue = m_showControlsAnimation.currentValue().toReal();
                } else {
                    animationDuration = showControlsAnimationDuration;
                    animationStartValue = 1.0;
                }
            } else {
                animationDuration = 1;
                animationStartValue = 0.0;
            }

            m_areControlsHidden = true;
            m_showControlsAnimation.setStartValue(animationStartValue);
            m_showControlsAnimation.setEndValue(0.0);
            m_showControlsAnimation.setDuration(animationDuration);
            m_showControlsAnimation.start();
        };

    delay = qMax(delay, 0);

    m_showControlsTimer.disconnect();
    connect(&m_showControlsTimer, &QTimer::timeout, animFunction);
    m_showControlsTimer.start(delay);
}

void OverviewDockerPage::on_overviewWidget_signalDraggingStarted()
{
    if (!m_pinControls && m_areControlsHidden && m_showControlsTimer.isActive()) {
        m_showControlsTimer.stop();
    }
}

void OverviewDockerPage::on_overviewWidget_signalDraggingFinished()
{
    if (!m_pinControls && m_areControlsHidden && !m_isTouching) {
        showControls(showControlsTimerDuration);
    }
}
