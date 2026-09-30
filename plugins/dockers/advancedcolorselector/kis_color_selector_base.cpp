/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_color_selector_base.h"

#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <QScreen>
#include <QTimer>
#include <QCursor>
#include <QPainter>
#include <QMimeData>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"

#include "kis_canvas2.h"
#include "kis_canvas_resource_provider.h"
#include "kis_node.h"
#include "KisViewManager.h"
#include <KisView.h>
#include "kis_image.h"
#include "kis_global.h"
#include "kis_display_color_converter.h"

#include <resources/KoGamutMask.h>

// for HAVE_WAYLAND
#include <KoConfig.h>

#if defined HAVE_WAYLAND && QT_VERSION >= QT_VERSION_CHECK(6, 11, 0)
#define HAVE_QT_PRIVATE_TOOLTIP_POSITION_API
#endif

#ifdef HAVE_QT_PRIVATE_TOOLTIP_POSITION_API
#include <QtWaylandClient/private/qwaylandwindow_p.h>
#endif


namespace {
    QWidget* findNearestDockerOrNativeParent(QWidget *w) {
        if (!w || w->inherits("QDockWidget") || w->windowHandle()) {
            return w;
        }

        return findNearestDockerOrNativeParent(w->parentWidget());
    }
}

class KisColorPreviewPopup : public QWidget {
public:
    KisColorPreviewPopup(KisColorSelectorBase* parent)
        : QWidget(parent), m_parent(parent)
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint | Qt::NoDropShadowWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        setQColor(QColor(0,0,0));
        m_baseColor = QColor(0,0,0,0);
        m_previousColor = QColor(0,0,0,0);
        m_lastUsedColor = QColor(0,0,0,0);
    }

    void showAndAdjustPosition()
    {
        updatePosition();
        show();
    }

    void updatePosition()
    {
        resize(100, 150);

        // make sure that all the platform-specific structures for the window
        // are created (i.e. QWaylandWindow) before trying to request them
        // via `windowHandle()->handle()`
        create();

#ifdef HAVE_QT_PRIVATE_TOOLTIP_POSITION_API
        if (auto waylandWindow = dynamic_cast<QNativeInterface::Private::QWaylandWindow *>(windowHandle()->handle())) {
            QWidget *referenceWidget = findNearestDockerOrNativeParent(parentWidget());
            if (!referenceWidget) {
                referenceWidget = parentWidget();
            }

            const QPoint parentPos = parentWidget()->window()->mapFromGlobal(referenceWidget->mapToGlobal(QPoint()));
            const QRect gravityRect(QRect(parentPos, QSize(referenceWidget->width(), 10)));

            waylandWindow->setParentControlGeometry(gravityRect);

            // well, it is technically not a "submenu", but this is the way
            // to make qt position it "around" the docker widget
            waylandWindow->setExtendedWindowType(QNativeInterface::Private::QWaylandWindow::SubMenu);
        } else
#endif /* HAVE_QT_PRIVATE_TOOLTIP_POSITION_API */
        {
            QPoint parentPos = m_parent->mapToGlobal(QPoint(0,0));
            const QRect availRect = this->screen()->availableGeometry();
            QPoint targetPos;
            if ( parentPos.x() - 100 > availRect.x() ) {
                targetPos =  QPoint(parentPos.x() - 100, parentPos.y());
            } else if ( parentPos.x() + m_parent->width() + 100 < availRect.right()) {
                targetPos = m_parent->mapToGlobal(QPoint(m_parent->width(), 0));
            } else if ( parentPos.y() - 100 > availRect.y() ) {
                targetPos =  QPoint(parentPos.x(), parentPos.y() - 100);
            } else {
                targetPos =  QPoint(parentPos.x(), parentPos.y() + m_parent->height());
            }
            move(targetPos.x(), targetPos.y());
        }
    }

    void initializeColors(const QColor& color, const QColor& lastUsedColor) {
        m_color = color;
        m_baseColor = color;
        m_previousColor = color;
        m_lastUsedColor = lastUsedColor;
        update();
    }

    void setQColor(const QColor& color)
    {
        m_color = color;
        update();
    }

    void setPreviousColor()
    {
        m_previousColor = m_baseColor;
    }

    void setBaseColor(const QColor& color)
    {
        m_baseColor = color;
        update();
    }

    void setLastUsedColor(const QColor& color)
    {
        m_lastUsedColor = color;
        update();
    }

protected:
    void paintEvent(QPaintEvent *e) override {
        Q_UNUSED(e);
        QPainter p(this);
        p.fillRect(0, 0, width(), width(), m_color);
        p.fillRect(50, width(), width(), height(), m_previousColor);
        p.fillRect(0, width(), 50, height(), m_lastUsedColor);
    }

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEvent *e) override
#else
    void enterEvent(QEnterEvent *e) override
#endif
    {
        m_parent->requestHideMyself();
        QWidget::enterEvent(e);
    }

    void leaveEvent(QEvent *e) override {
        m_parent->requestHideMyself();
        QWidget::leaveEvent(e);
    }

private:
    KisColorSelectorBase* m_parent;
    QColor m_color;
    QColor m_baseColor;
    QColor m_previousColor;
    QColor m_lastUsedColor;
};

KisColorSelectorBase::KisColorSelectorBase(QWidget *parent) :
    QWidget(parent),
    m_canvas(0),
    m_popup(0),
    m_parent(0),
    m_colorUpdateAllowed(true),
    m_colorUpdateSelf(false),
    m_hideTimer(new QTimer(this)),
    m_popupOnMouseOver(false),
    m_popupOnMouseClick(true),
    m_colorSpace(0),
    m_isPopup(false),
    m_hideOnMouseClick(false),
    m_colorPreviewPopup(new KisColorPreviewPopup(this))
{
    m_hideTimer->setInterval(200);
    m_hideTimer->setSingleShot(true);
    connect(m_hideTimer, SIGNAL(timeout()), this, SLOT(tryHideMyself()));

    using namespace std::placeholders; // For _1 placeholder
    auto function = std::bind(&KisColorSelectorBase::slotUpdateColorAndPreview, this, _1);
    m_updateColorCompressor.reset(new ColorCompressorType(25 /* ms */, function));

    // Color selectors don't have context menus. Setting this prevents any
    // long-presses from delaying inputs, see KisLongPressEventFilter.cpp.
    setContextMenuPolicy(Qt::PreventContextMenu);
}

KisColorSelectorBase::~KisColorSelectorBase()
{
    delete m_popup;
    delete m_colorPreviewPopup;
}

void KisColorSelectorBase::setPopupBehaviour(bool onMouseOver, bool onMouseClick)
{
    m_popupOnMouseClick = onMouseClick;
    m_popupOnMouseOver = onMouseOver;
    if(onMouseClick) {
        m_popupOnMouseOver = false;
    }

    if(m_popupOnMouseOver) {
        setMouseTracking(true);
    }
}

void KisColorSelectorBase::setColorSpace(const KoColorSpace *colorSpace)
{
    m_colorSpace = colorSpace;
}

void KisColorSelectorBase::setCanvas(KisCanvas2 *canvas)
{
    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
    }
    m_canvas = canvas;
    if (m_canvas) {
        connect(m_canvas->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
                SLOT(canvasResourceChanged(int,QVariant)), Qt::UniqueConnection);

        connect(m_canvas->displayColorConverter(), SIGNAL(displayConfigurationChanged()),
                SLOT(reset()), Qt::UniqueConnection);

        connect(canvas->imageView()->resourceProvider(), SIGNAL(sigFGColorUsed(KoColor)),
                this,                               SLOT(updateLastUsedColorPreview(KoColor)), Qt::UniqueConnection);

        if (m_canvas->viewManager() && m_canvas->viewManager()->canvasResourceProvider()) {
            const KoColor currentColor = Acs::currentColor(m_canvas->viewManager()->canvasResourceProvider(), Acs::Foreground);
            setColor(currentColor);

            auto colorHistory = m_canvas->viewManager()->canvasResourceProvider()->colorHistoryColors();
            const KoColor lastUsedColor = !colorHistory.isEmpty() ? colorHistory.first() : currentColor;

            const QColor currentQColor = converter()->toQColor(currentColor);
            const QColor lastUsedQColor = converter()->toQColor(lastUsedColor);

            m_colorPreviewPopup->initializeColors(currentQColor, lastUsedQColor);
        }
    }
    if (m_popup) {
        m_popup->setCanvas(canvas);
    }

    reset();
}

void KisColorSelectorBase::unsetCanvas()
{
    if (m_popup) {
        m_popup->unsetCanvas();
    }
    m_canvas = 0;
}



void KisColorSelectorBase::mousePressEvent(QMouseEvent* event)
{
    event->accept();

    if(!m_isPopup && m_popupOnMouseClick &&
       event->button() == Qt::MiddleButton) {

        lazyCreatePopup();

        m_colorUpdateSelf=false;
        showPopup(event->globalPos());

    } else if (m_isPopup && event->button() == Qt::MiddleButton) {
        if (m_colorPreviewPopup) {
            m_colorPreviewPopup->hide();
        }
        hide();
    } else {
        m_colorUpdateSelf=true;
        showColorPreview();
        event->ignore();
    }
}

void KisColorSelectorBase::mouseReleaseEvent(QMouseEvent *e) {

   Q_UNUSED(e);

    if (e->button() == Qt::MiddleButton) {
        e->accept();
    } else if (m_isPopup &&
               (m_hideOnMouseClick && !m_popupOnMouseOver) &&
               !m_hideTimer->isActive()) {
        if (m_colorPreviewPopup) {
            m_colorPreviewPopup->hide();
        }
        hide();
    }
}
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
void KisColorSelectorBase::enterEvent(QEvent *e)
#else
void KisColorSelectorBase::enterEvent(QEnterEvent *e)
#endif
{

    if (m_hideTimer->isActive()) {
        m_hideTimer->stop();
    }

    // do not show the popup when boxed in
    // the configuration dialog (m_canvas == 0)

    if (m_canvas &&
        !m_isPopup && m_popupOnMouseOver &&
        (!m_popup || m_popup->isHidden())) {

        lazyCreatePopup();

        const QPoint preferredCenter = mapToGlobal(rect().center());
        showPopup(preferredCenter);
    }

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QWidget::enterEvent(e);
#else
    QEnterEvent *enterEvent = dynamic_cast<QEnterEvent*>(e);
    QWidget::enterEvent(enterEvent);
#endif

}

void KisColorSelectorBase::leaveEvent(QEvent *e)
{
    m_hideTimer->start();
    QWidget::leaveEvent(e);
}

void KisColorSelectorBase::keyPressEvent(QKeyEvent *)
{
    tryHideMyself();
}

void KisColorSelectorBase::dragEnterEvent(QDragEnterEvent *e)
{
    if(e->mimeData()->hasColor())
        e->acceptProposedAction();
    if(e->mimeData()->hasText() && QColor(e->mimeData()->text()).isValid())
        e->acceptProposedAction();
}

void KisColorSelectorBase::dropEvent(QDropEvent *e)
{
    QColor color;
    if(e->mimeData()->hasColor()) {
        color = qvariant_cast<QColor>(e->mimeData()->colorData());
    }
    else if(e->mimeData()->hasText()) {
#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
        color.setNamedColor(e->mimeData()->text());
#else
        color.fromString(e->mimeData()->text());
#endif
        if(!color.isValid())
            return;
    }

    KoColor kocolor(color , KoColorSpaceRegistry::instance()->rgb8());
    updateColor(kocolor, Acs::Foreground, true);
}

void KisColorSelectorBase::updateColor(const KoColor &color, Acs::ColorRole role, bool needsExplicitColorReset)
{
    commitColor(color, role);

    if (needsExplicitColorReset) {
        setColor(color);
    }
}

void KisColorSelectorBase::requestUpdateColorAndPreview(const KoColor &color, Acs::ColorRole role)
{
    m_updateColorCompressor->start(qMakePair(color, role));
}

void KisColorSelectorBase::slotUpdateColorAndPreview(QPair<KoColor, Acs::ColorRole> color)
{
    updateColorPreview(color.first);
    updateColor(color.first, color.second, false);
}

void KisColorSelectorBase::setColor(const KoColor& color)
{
    Q_UNUSED(color);
}

void KisColorSelectorBase::lazyCreatePopup()
{
    if (!m_popup) {
        m_popup = createPopup();
        Q_ASSERT(m_popup);
        m_popup->setParent(this);

        // Setting Qt::BypassWindowManagerHint will prevent
        // the WM from showing another taskbar entry,
        // but will require that we handle window activation manually
        m_popup->setWindowFlags(Qt::FramelessWindowHint |
                                Qt::Popup |
                                Qt::NoDropShadowWindowHint |
                                Qt::BypassWindowManagerHint);
        m_popup->m_parent = this;
        m_popup->m_isPopup = true;
    }
    m_popup->setCanvas(m_canvas);
    m_popup->updateSettings();
}

void KisColorSelectorBase::showPopup(std::optional<QPoint> preferredCenter)
{
    // This slot may be called by some action,
    // so we need to be able to handle it
    lazyCreatePopup();

    const QPoint preferredGlobalCenterPos = preferredCenter ? *preferredCenter : QCursor::pos();

    m_popup->create();

    const QPoint preferredGlobalTopLeft =
        QPoint(preferredGlobalCenterPos.x() - m_popup->width() / 2, preferredGlobalCenterPos.y() - m_popup->height() / 2);

#ifdef HAVE_WAYLAND
    if (qApp->platformName() == "wayland") {
        m_popup->move(preferredGlobalTopLeft);
    } else
#endif
    {
        const QRect availRect = this->screen()->availableGeometry();
        QRect rc(preferredGlobalTopLeft, m_popup->size());
        rc = kisEnsureInRect(rc, availRect);
        m_popup->move(rc.topLeft());
    }

    if (m_colorPreviewPopup) {
        m_colorPreviewPopup->hide();
    }

    m_popup->show();
    m_popup->m_colorPreviewPopup->showAndAdjustPosition();
}

void KisColorSelectorBase::requestHideMyself()
{
    m_hideTimer->start();
}

void KisColorSelectorBase::tryHideMyself()
{
    m_colorPreviewPopup->hide();

    if (m_isPopup) {
        hide();
    }
}

void KisColorSelectorBase::commitColor(const KoColor& color, Acs::ColorRole role)
{
    if (!m_canvas)
        return;

    m_colorUpdateAllowed=false;

    if (role == Acs::Foreground)
        m_canvas->resourceManager()->setForegroundColor(color);
    else
        m_canvas->resourceManager()->setBackgroundColor(color);

    m_colorUpdateAllowed=true;
}

void KisColorSelectorBase::showColorPreview()
{
    if(m_colorPreviewPopup->isHidden()) {
        m_colorPreviewPopup->showAndAdjustPosition();
    }
}

void KisColorSelectorBase::updateColorPreview(const KoColor &color)
{
    m_colorPreviewPopup->setQColor(converter()->toQColor(color));
}

void KisColorSelectorBase::canvasResourceChanged(int key, const QVariant &v)
{
    if (key == KoCanvasResource::ForegroundColor || key == KoCanvasResource::BackgroundColor) {
        KoColor realColor(v.value<KoColor>());
        updateColorPreview(realColor);
        if (m_colorUpdateAllowed && !m_colorUpdateSelf) {
            setColor(realColor);
        }
    }
}

const KoColorSpace* KisColorSelectorBase::colorSpace() const
{
    return converter()->paintingColorSpace();
}

void KisColorSelectorBase::updateSettings()
{
    if(m_popup) {
        m_popup->updateSettings();
    }

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");


   int zoomSelectorOptions =  (int) cfg.readEntry("zoomSelectorOptions", 0) ;
   if (zoomSelectorOptions == 0)   {
       setPopupBehaviour(false, true);   // middle mouse button click will open zoom selector
   } else if (zoomSelectorOptions == 1)   {
       setPopupBehaviour(true, false);   // move over will open the zoom selector
   }
   else
   {
        setPopupBehaviour(false, false); // do not show zoom selector
   }

   if(m_isPopup) {
        m_hideOnMouseClick = cfg.readEntry("hidePopupOnClickCheck", false);
        const int zoomSize = cfg.readEntry("zoomSize", 280);
        resize(zoomSize, zoomSize);
    }

    reset();
}

void KisColorSelectorBase::reset()
{
    update();
}

void KisColorSelectorBase::updateBaseColorPreview(const KoColor &color)
{
    m_colorPreviewPopup->setBaseColor(converter()->toQColor(color));
}

void KisColorSelectorBase::updatePreviousColorPreview()
{
    m_colorPreviewPopup->setPreviousColor();
}

void KisColorSelectorBase::updateLastUsedColorPreview(const KoColor &color)
{
    m_colorPreviewPopup->setLastUsedColor(converter()->toQColor(color));
}

KisDisplayColorConverter* KisColorSelectorBase::converter() const
{
    return m_canvas ?
        m_canvas->displayColorConverter() :
                KisDisplayColorConverter::dumbConverterInstance();
}

void KisColorSelectorBase::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();
}

void KisColorSelectorBase::changeEvent(QEvent *event)
{
    // hide the popup when another window becomes active, e.g. due to alt+tab
    if(m_isPopup && event->type() == QEvent::ActivationChange && !isActiveWindow()) {
        tryHideMyself();
    }

    QWidget::changeEvent(event);
}

void KisColorSelectorBase::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // manual activation required due to Qt::BypassWindowManagerHint
    if(m_isPopup) {
        activateWindow();
    }
}
