/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "overviewdocker_dock.h"
#include "overviewwidget.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QStatusBar>

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

OverviewDockerDock::OverviewDockerDock()
    : QDockWidget(i18n("Overview"))
    , m_zoomSlider(nullptr)
    , m_rotateAngleSelector(nullptr)
    , m_mirrorCanvas(nullptr)
    , m_pinControlsButton(nullptr)
    , m_canvas(nullptr)
    , m_cursorIsHover(false)
    , m_lastOverviewMousePos(0.0, 0.0)
    , m_cumulatedMouseDistanceSquared(0.0)
{
    m_page = new QWidget(this);

    m_overviewWidget = new OverviewWidget(m_page);
    m_overviewWidget->setMinimumHeight(50);
    m_overviewWidget->setBackgroundRole(QPalette::Base);
    // paints background role before paint()
    m_overviewWidget->setAutoFillBackground(true);
    m_overviewWidget->installEventFilter(this);
    connect(m_overviewWidget, SIGNAL(signalDraggingStarted()), &m_showControlsTimer, SLOT(stop()));
    connect(m_overviewWidget, SIGNAL(signalDraggingFinished()), SLOT(on_overviewWidget_signalDraggingFinished()));

    m_controlsContainer = new QWidget(m_page);

    m_controlsLayout = new QVBoxLayout;
    m_controlsLayout->setContentsMargins(2, 2, 2, 2);
    m_controlsLayout->setSpacing(2);
    m_controlsContainer->setLayout(m_controlsLayout);

    m_controlsSecondRowLayout = new QHBoxLayout();

    setWidget(m_page);

    connect(&m_showControlsTimer, SIGNAL(timeout()), SLOT(showControls()));

    m_showControlsAnimation.setEasingCurve(QEasingCurve(QEasingCurve::InOutCubic));
    connect(&m_showControlsAnimation, &QVariantAnimation::valueChanged, this, &OverviewDockerDock::layoutMainWidgets);

    KConfigGroup config = KSharedConfig::openConfig()->group("OverviewDocker");
    m_pinControls = config.readEntry("pinControls", true);
    m_areControlsHidden = !m_pinControls;

    setEnabled(false);
}

OverviewDockerDock::~OverviewDockerDock()
{
    KConfigGroup config = KSharedConfig::openConfig()->group("OverviewDocker");
    config.writeEntry("pinControls", m_pinControls);
}

void OverviewDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if(m_canvas == canvas)
        return;

    setEnabled(canvas != nullptr);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->image()->disconnect(this);
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
    while (m_controlsSecondRowLayout->count() && m_controlsSecondRowLayout->itemAt(0)->spacerItem()) {
        delete m_controlsSecondRowLayout->takeAt(0);
    }

    m_controlsLayout->removeItem(m_controlsSecondRowLayout);

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    m_overviewWidget->setCanvas(canvas);
    if (m_canvas && m_canvas->viewManager() && m_canvas->viewManager()->zoomController() && m_canvas->viewManager()->zoomController()->zoomAction()) {
        m_zoomSlider = m_canvas->viewManager()->zoomController()->zoomAction()->createWidget(m_canvas->imageView()->KisView::statusBar());
        static_cast<KoZoomWidget*>(m_zoomSlider)->setZoomInputFlat(false);
        m_controlsLayout->addWidget(m_zoomSlider);

        m_rotateAngleSelector = new KisAngleSelector();
        m_rotateAngleSelector->setRange(-360.00, 360.0);
        m_rotateAngleSelector->setAngle(m_canvas->rotationAngle());
        m_rotateAngleSelector->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);
        m_rotateAngleSelector->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_ContextMenu);
        connect(m_rotateAngleSelector, SIGNAL(angleChanged(qreal)), this, SLOT(rotateCanvasView(qreal)), Qt::UniqueConnection);
        connect(m_canvas->canvasController()->proxyObject, SIGNAL(canvasOffsetXChanged(int)), this, SLOT(updateSlider()));

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

        m_controlsSecondRowLayout->addWidget(m_rotateAngleSelector);
        m_controlsSecondRowLayout->addStretch();
        m_controlsSecondRowLayout->addWidget(m_mirrorCanvas);
        m_controlsSecondRowLayout->addStretch();
        m_controlsSecondRowLayout->addWidget(m_pinControlsButton);
        m_controlsLayout->addLayout(m_controlsSecondRowLayout);

        m_zoomSlider->setVisible(true);
        m_rotateAngleSelector->setVisible(true);

        if (m_pinControls) {
            m_showControlsAnimation.stop();
            m_showControlsAnimation.setStartValue(1.0);
            m_showControlsAnimation.setEndValue(0.0);
        } else {
            if (m_showControlsAnimation.state() != QVariantAnimation::Running) {
                m_showControlsAnimation.stop();
                if (m_cursorIsHover) {
                    m_showControlsAnimation.setStartValue(1.0);
                    m_showControlsAnimation.setEndValue(0.0);
                } else {
                    m_showControlsAnimation.setStartValue(0.0);
                    m_showControlsAnimation.setEndValue(1.0);
                }
            }
        }
        m_showControlsAnimation.setCurrentTime(0);

        layoutMainWidgets();
    }
}

void OverviewDockerDock::unsetCanvas()
{
    setEnabled(false);
    m_canvas = nullptr;
    m_overviewWidget->unsetCanvas();
}

void OverviewDockerDock::mirrorUpdateIcon()
{
    if(!m_mirrorCanvas) return;
    m_mirrorCanvas->setIcon(KisIconUtils::loadIcon("mirror-view-16"));
}

void OverviewDockerDock::rotateCanvasView(qreal rotation)
{
    if (!m_canvas) return;
    KisCanvasController *canvasController =
            dynamic_cast<KisCanvasController*>(m_canvas->viewManager()->canvasBase()->canvasController());
    if (canvasController) {
        canvasController->rotateCanvas(rotation-m_canvas->rotationAngle());
    }
}

void OverviewDockerDock::updateSlider()
{
    if (!m_canvas) return;
    KisSignalsBlocker l(m_rotateAngleSelector);

    m_rotateAngleSelector->setAngle(m_canvas->rotationAngle());
}

void OverviewDockerDock::setPinControls(bool pin)
{
    m_pinControls = pin;
    layoutMainWidgets();
}

void OverviewDockerDock::resizeEvent(QResizeEvent*)
{
    layoutMainWidgets();
}

void OverviewDockerDock::leaveEvent(QEvent*)
{
    m_cursorIsHover = false;
    if (isEnabled() && !m_pinControls) {
        m_showControlsTimer.stop();
        hideControls();
        m_cumulatedMouseDistanceSquared = 0.0;
    }
}

void OverviewDockerDock::enterEvent(QEvent*)
{
    m_cursorIsHover = true;
    if (isEnabled() && !m_pinControls) {
        if (m_showControlsAnimation.state() == QVariantAnimation::Running) {
            showControls();
        } else {
            m_showControlsTimer.start(showControlsTimerDuration);
        }
    }
}

bool OverviewDockerDock::event(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange) {
        if (m_pinControlsButton) {
            KisIconUtils::updateIcon(m_pinControlsButton);
        }
    } else if (e->type() == QEvent::StyleChange || e->type() == QEvent::FontChange) {
        resizeEvent(nullptr);
    }
    return QDockWidget::event(e);
}

bool OverviewDockerDock::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_overviewWidget && e->type() == QEvent::MouseMove) {
        if (isEnabled() && !m_overviewWidget->isDragging() && !m_pinControls && m_areControlsHidden) {
            QMouseEvent *me = static_cast<QMouseEvent*>(e);
            constexpr double showControlsAreaRadiusSquared = showControlsAreaRadius * showControlsAreaRadius;
            const QPointF d = me->localPos() - m_lastOverviewMousePos;
            const double distanceSquared = d.x() * d.x() + d.y() * d.y();
            if (distanceSquared > m_cumulatedMouseDistanceSquared) {
                if (distanceSquared >= showControlsAreaRadiusSquared) {
                    m_showControlsTimer.start(showControlsTimerDuration);
                    m_lastOverviewMousePos = me->localPos();
                    m_cumulatedMouseDistanceSquared = 0.0;
                } else {
                    m_cumulatedMouseDistanceSquared = distanceSquared;
                }
            }
        }
    }
    return false;
}

void OverviewDockerDock::layoutMainWidgets()
{
    m_page->setMinimumHeight(m_overviewWidget->minimumHeight() +
                             m_controlsContainer->minimumSizeHint().height());

    if (!m_pinControls) {
        const qreal pageHeight = static_cast<qreal>(m_page->height());
        const qreal controlsContainerHeight = static_cast<qreal>(m_controlsContainer->sizeHint().height());
        const qreal animationProgress = m_showControlsAnimation.currentValue().toReal();
        const int widgetLimitPosition = static_cast<int>(std::round(pageHeight - animationProgress * controlsContainerHeight));
        m_overviewWidget->setGeometry(0, 0, m_page->width(), widgetLimitPosition);
        m_controlsContainer->setGeometry(0, widgetLimitPosition, m_page->width(), static_cast<int>(controlsContainerHeight));
    } else {
        const int controlsContainerHeight = m_controlsContainer->sizeHint().height();
        const int widgetLimitPosition = m_page->height() - controlsContainerHeight;
        m_overviewWidget->setGeometry(0, 0, m_page->width(), widgetLimitPosition);
        m_controlsContainer->setGeometry(0, widgetLimitPosition, m_page->width(), controlsContainerHeight);
    }
}

void OverviewDockerDock::showControls() const
{
    m_showControlsAnimation.stop();
    // scale the animation duration in case the animation is in the middle
    const int animationDuration =
        static_cast<int>(std::round((1.0 - m_showControlsAnimation.currentValue().toReal()) * showControlsAnimationDuration));
    m_showControlsAnimation.setStartValue(m_showControlsAnimation.currentValue());
    m_showControlsAnimation.setEndValue(1.0);
    m_showControlsAnimation.setDuration(animationDuration);
    m_showControlsAnimation.start();
    m_areControlsHidden = false;
}

void OverviewDockerDock::hideControls() const
{
    m_showControlsAnimation.stop();
    // scale the animation duration in case the animation is in the middle
    const int animationDuration =
        static_cast<int>(std::round(m_showControlsAnimation.currentValue().toReal() * showControlsAnimationDuration));
    m_showControlsAnimation.setStartValue(m_showControlsAnimation.currentValue());
    m_showControlsAnimation.setEndValue(0.0);
    m_showControlsAnimation.setDuration(animationDuration);
    m_showControlsAnimation.start();
    m_areControlsHidden = true;
}

void OverviewDockerDock::on_overviewWidget_signalDraggingFinished()
{
    if (!m_pinControls && m_areControlsHidden) {
        m_showControlsTimer.start(showControlsTimerDuration);
    }
}
