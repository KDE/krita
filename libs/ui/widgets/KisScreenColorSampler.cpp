/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QDesktopWidget>
#include <QGuiApplication>
#include <QApplication>
#include <QScreen>
#include <QColor>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWindow>
#include <QTimer>

#include <kis_canvas2.h>

#include "kis_shared_ptr.h"
#include "kis_icon.h"
#include "kis_image.h"
#include "kis_wrapped_rect.h"
#include "KisDocument.h"
#include "KisPart.h"
#include "KisReferenceImagesLayer.h"
#include "KisScreenColorSampler.h"
#include "KisDlgInternalColorSelector.h"

struct KisScreenColorSampler::Private
{
    QPushButton *screenColorSamplerButton = 0;
    QLabel *lblScreenColorInfo = 0;

    KoColor currentColor = KoColor();
    KoColor beforeScreenColorSampling = KoColor();

    KisScreenColorSamplingEventFilter *colorSamplingEventFilter = 0;

#ifdef Q_OS_WIN32
    QTimer *updateTimer = 0;
    QWindow dummyTransparentWindow;
#endif
};

KisScreenColorSampler::KisScreenColorSampler(bool showInfoLabel, QWidget *parent) : KisScreenColorSamplerBase(parent), m_d(new Private)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_d->screenColorSamplerButton = new QPushButton();

    m_d->screenColorSamplerButton->setMinimumHeight(25);
    layout->addWidget(m_d->screenColorSamplerButton);

    if (showInfoLabel) {
        m_d->lblScreenColorInfo = new QLabel(QLatin1String("\n"));
        layout->addWidget(m_d->lblScreenColorInfo);
    }

    connect(m_d->screenColorSamplerButton, SIGNAL(clicked()), SLOT(sampleScreenColor()));

    updateIcons();

#ifdef Q_OS_WIN32
    m_d->updateTimer = new QTimer(this);
    m_d->dummyTransparentWindow.resize(1, 1);
    m_d->dummyTransparentWindow.setFlags(Qt::Tool | Qt::FramelessWindowHint);
    connect(m_d->updateTimer, SIGNAL(timeout()), SLOT(updateColorSampling()));
#endif
}

KisScreenColorSampler::~KisScreenColorSampler()
{
}

void KisScreenColorSampler::updateIcons()
{
    m_d->screenColorSamplerButton->setIcon(kisIcon("krita_tool_color_sampler"));
}

KoColor KisScreenColorSampler::currentColor()
{
    return m_d->currentColor;
}

void KisScreenColorSampler::sampleScreenColor()
{
    if (!m_d->colorSamplingEventFilter)
        m_d->colorSamplingEventFilter = new KisScreenColorSamplingEventFilter(this);
    this->installEventFilter(m_d->colorSamplingEventFilter);
    // If user pushes Escape, the last color before sampling will be restored.
    m_d->beforeScreenColorSampling = currentColor();
    grabMouse(Qt::CrossCursor);

#ifdef Q_OS_WIN32 // excludes WinCE and WinRT
    // On Windows mouse tracking doesn't work over other processes's windows
    m_d->updateTimer->start(30);

    // HACK: Because mouse grabbing doesn't work across processes, we have to have a dummy,
    // invisible window to catch the mouse click, otherwise we will click whatever we clicked
    // and loose focus.
    m_d->dummyTransparentWindow.show();
#endif
    grabKeyboard();
    /* With setMouseTracking(true) the desired color can be more precisely sampled,
     * and continuously pushing the mouse button is not necessary.
     */
    setMouseTracking(true);

    m_d->screenColorSamplerButton->setDisabled(true);

    const QPoint globalPos = QCursor::pos();
    setCurrentColor(grabScreenColor(globalPos));
    updateColorLabelText(globalPos);
}

void KisScreenColorSampler::setCurrentColor(KoColor c)
{
    m_d->currentColor = c;
}

KoColor KisScreenColorSampler::grabScreenColor(const QPoint &p)
{
    // First check whether we're clicking on a Krita window for some real color sampling
    Q_FOREACH(KisView *view, KisPart::instance()->views()) {
        const KisCanvas2 *canvas = view->canvasBase();
        const QWidget *canvasWidget = canvas->canvasWidget();
        QPoint widgetPoint = canvasWidget->mapFromGlobal(p);

        if (canvasWidget->visibleRegion().contains(widgetPoint)) {
            KisImageWSP image = view->image();

            if (image) {
                QPointF imagePoint = canvas->coordinatesConverter()->widgetToImage(widgetPoint);
                // sample from reference images first
                KisSharedPtr<KisReferenceImagesLayer> referenceImageLayer = view->document()->referenceImagesLayer();

                if (referenceImageLayer && canvas->referenceImagesDecoration()->visible()) {
                    QColor color = referenceImageLayer->getPixel(imagePoint);
                    if (color.isValid()) {
                        return KoColor(color, image->colorSpace());
                    }
                }

                if (image->wrapAroundModePermitted()) {
                    imagePoint = KisWrappedRect::ptToWrappedPt(imagePoint.toPoint(), image->bounds());
                }
                KoColor sampledColor = KoColor();
                image->projection()->pixel(imagePoint.x(), imagePoint.y(), &sampledColor);
                return sampledColor;
            }
        }
    }

    // And otherwise, we'll check the desktop
    const QDesktopWidget *desktop = QApplication::desktop();
    const QPixmap pixmap = QGuiApplication::screens().at(desktop->screenNumber())->grabWindow(desktop->winId(),
                                                                                              p.x(), p.y(), 1, 1);
    QImage i = pixmap.toImage();
    KoColor col = KoColor();
    col.fromQColor(QColor::fromRgb(i.pixel(0, 0)));
    return col;
}

void KisScreenColorSampler::updateColorLabelText(const QPoint &globalPos)
{
    if (m_d->lblScreenColorInfo) {
        KoColor col = grabScreenColor(globalPos);
        QString colname = KoColor::toQString(col);
        QString location = QString::number(globalPos.x())+QString(", ")+QString::number(globalPos.y());
        m_d->lblScreenColorInfo->setWordWrap(true);
        m_d->lblScreenColorInfo->setText(location+QString(": ")+colname);
    }
}

bool KisScreenColorSampler::handleColorSamplingMouseMove(QMouseEvent *e)
{
    // If the cross is visible the grabbed color will be black most of the times
    //cp->setCrossVisible(!cp->geometry().contains(e->pos()));

    continueUpdateColorSampling(e->globalPos());
    return true;
}

bool KisScreenColorSampler::handleColorSamplingMouseButtonRelease(QMouseEvent *e)
{
    setCurrentColor(grabScreenColor(e->globalPos()));
    Q_EMIT sigNewColorSampled(currentColor());
    releaseColorSampling();
    return true;
}

bool KisScreenColorSampler::handleColorSamplingKeyPress(QKeyEvent *e)
{
    if (e->matches(QKeySequence::Cancel)) {
        releaseColorSampling();
        setCurrentColor(m_d->beforeScreenColorSampling);
    } else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        setCurrentColor(grabScreenColor(QCursor::pos()));
        releaseColorSampling();
    }
    e->accept();
    return true;
}

void KisScreenColorSampler::releaseColorSampling()
{

    removeEventFilter(m_d->colorSamplingEventFilter);
    releaseMouse();
#ifdef Q_OS_WIN32
    m_d->updateTimer->stop();
    m_d->dummyTransparentWindow.setVisible(false);
#endif
    releaseKeyboard();
    setMouseTracking(false);

    if (m_d->lblScreenColorInfo) {
        m_d->lblScreenColorInfo->setText(QLatin1String("\n"));
    }

    m_d->screenColorSamplerButton->setDisabled(false);
}

void KisScreenColorSampler::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
}

void KisScreenColorSampler::updateColorSampling()
{
    static QPoint lastGlobalPos;
    QPoint newGlobalPos = QCursor::pos();
    if (lastGlobalPos == newGlobalPos)
        return;
    lastGlobalPos = newGlobalPos;

    if (!rect().contains(mapFromGlobal(newGlobalPos))) { // Inside the dialog mouse tracking works, handleColorSamplingMouseMove will be called
        continueUpdateColorSampling(newGlobalPos);
#ifdef Q_OS_WIN32
        m_d->dummyTransparentWindow.setPosition(newGlobalPos);
#endif
    }
}

void KisScreenColorSampler::continueUpdateColorSampling(const QPoint &globalPos)
{
    const KoColor color = grabScreenColor(globalPos);
    // QTBUG-39792, do not change standard, custom color selectors while moving as
    // otherwise it is not possible to pre-select a custom cell for assignment.
    setCurrentColor(color);
    updateColorLabelText(globalPos);
}

// Event filter to be installed on the dialog while in color-sampling mode.
KisScreenColorSamplingEventFilter::KisScreenColorSamplingEventFilter(KisScreenColorSampler *w, QObject *parent)
    : QObject(parent)
    , m_w(w)
{}

bool KisScreenColorSamplingEventFilter::eventFilter(QObject *, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseMove:
        return m_w->handleColorSamplingMouseMove(static_cast<QMouseEvent *>(event));
    case QEvent::MouseButtonRelease:
        return m_w->handleColorSamplingMouseButtonRelease(static_cast<QMouseEvent *>(event));
    case QEvent::KeyPress:
        return m_w->handleColorSamplingKeyPress(static_cast<QKeyEvent *>(event));
    default:
        break;
    }
    return false;
}

// Register the color sampler factory with the internal color selector
struct ColorSamplerRegistrar {
    ColorSamplerRegistrar()
    {
        KisDlgInternalColorSelector::setScreenColorSamplerFactory(KisScreenColorSampler::createScreenColorSampler);
    }
};

static ColorSamplerRegistrar s_colorSamplerRegistrar;

