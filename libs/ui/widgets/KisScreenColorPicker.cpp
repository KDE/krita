/*
 * Copyright (C) Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>, (C) 2016
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "KisPart.h"
#include "KisScreenColorPicker.h"
#include "KisDlgInternalColorSelector.h"

struct KisScreenColorPicker::Private
{

    QPushButton *screenColorPickerButton = 0;
    QLabel *lblScreenColorInfo = 0;

    KoColor currentColor = KoColor();
    KoColor beforeScreenColorPicking = KoColor();

    KisScreenColorPickingEventFilter *colorPickingEventFilter = 0;

#ifdef Q_OS_WIN32
    QTimer *updateTimer = 0;
    QWindow dummyTransparentWindow;
#endif
};

KisScreenColorPicker::KisScreenColorPicker(QWidget *parent) : KisScreenColorPickerBase(parent), m_d(new Private)
{
    QVBoxLayout *layout = new QVBoxLayout();
    this->setLayout(layout);
    m_d->screenColorPickerButton = new QPushButton();

    m_d->screenColorPickerButton->setMinimumHeight(25);
    this->layout()->addWidget(m_d->screenColorPickerButton);
    m_d->lblScreenColorInfo = new QLabel(QLatin1String("\n"));
    this->layout()->addWidget(m_d->lblScreenColorInfo);
    connect(m_d->screenColorPickerButton, SIGNAL(clicked()), SLOT(pickScreenColor()));

    updateIcons();

#ifdef Q_OS_WIN32
    m_d->updateTimer = new QTimer(this);
    m_d->dummyTransparentWindow.resize(1, 1);
    m_d->dummyTransparentWindow.setFlags(Qt::Tool | Qt::FramelessWindowHint);
    connect(m_d->updateTimer, SIGNAL(timeout()), SLOT(updateColorPicking()));
#endif
}

KisScreenColorPicker::~KisScreenColorPicker()
{
}

void KisScreenColorPicker::updateIcons()
{
    m_d->screenColorPickerButton->setIcon(kisIcon("krita_tool_color_picker"));
}

KoColor KisScreenColorPicker::currentColor()
{
    return m_d->currentColor;
}

void KisScreenColorPicker::pickScreenColor()
{
    if (!m_d->colorPickingEventFilter)
        m_d->colorPickingEventFilter = new KisScreenColorPickingEventFilter(this);
    this->installEventFilter(m_d->colorPickingEventFilter);
    // If user pushes Escape, the last color before picking will be restored.
    m_d->beforeScreenColorPicking = currentColor();
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
    /* With setMouseTracking(true) the desired color can be more precisely picked up,
     * and continuously pushing the mouse button is not necessary.
     */
    setMouseTracking(true);

    //emit to the rest of the dialog to disable.
    m_d->screenColorPickerButton->setDisabled(true);

    const QPoint globalPos = QCursor::pos();
    setCurrentColor(grabScreenColor(globalPos));
    updateColorLabelText(globalPos);
}

void KisScreenColorPicker::setCurrentColor(KoColor c)
{
    m_d->currentColor = c;
}

KoColor KisScreenColorPicker::grabScreenColor(const QPoint &p)
{
    // First check whether we're clicking on a Krita window for some real color picking
    Q_FOREACH(KisView *view, KisPart::instance()->views()) {
        QWidget *canvasWidget = view->canvasBase()->canvasWidget();
        QPoint widgetPoint = canvasWidget->mapFromGlobal(p);

        if (canvasWidget->rect().contains(widgetPoint)) {
            QPointF imagePoint = view->canvasBase()->coordinatesConverter()->widgetToImage(widgetPoint);
            KisImageWSP image = view->image();

            if (image) {
                if (image->wrapAroundModePermitted()) {
                    imagePoint = KisWrappedRect::ptToWrappedPt(imagePoint.toPoint(), image->bounds());
                }
                KoColor pickedColor = KoColor();
                image->projection()->pixel(imagePoint.x(), imagePoint.y(), &pickedColor);
                return pickedColor;
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

void KisScreenColorPicker::updateColorLabelText(const QPoint &globalPos)
{
    KoColor col = grabScreenColor(globalPos);
    QString colname = KoColor::toQString(col);
    QString location = QString::number(globalPos.x())+QString(", ")+QString::number(globalPos.y());
    m_d->lblScreenColorInfo->setWordWrap(true);
    m_d->lblScreenColorInfo->setText(location+QString(": ")+colname);
}

bool KisScreenColorPicker::handleColorPickingMouseMove(QMouseEvent *e)
{
    // If the cross is visible the grabbed color will be black most of the times
    //cp->setCrossVisible(!cp->geometry().contains(e->pos()));

    continueUpdateColorPicking(e->globalPos());
    return true;
}

bool KisScreenColorPicker::handleColorPickingMouseButtonRelease(QMouseEvent *e)
{
    setCurrentColor(grabScreenColor(e->globalPos()));
    Q_EMIT sigNewColorPicked(currentColor());
    releaseColorPicking();
    return true;
}

bool KisScreenColorPicker::handleColorPickingKeyPress(QKeyEvent *e)
{
#if QT_VERSION >= 0x050600
    if (e->matches(QKeySequence::Cancel)) {
#else
    if (e->key() == Qt::Key_Escape) {
#endif
        releaseColorPicking();
        setCurrentColor(m_d->beforeScreenColorPicking);
    } else if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
        setCurrentColor(grabScreenColor(QCursor::pos()));
        releaseColorPicking();
    }
    e->accept();
    return true;
}

void KisScreenColorPicker::releaseColorPicking()
{

    removeEventFilter(m_d->colorPickingEventFilter);
    releaseMouse();
#ifdef Q_OS_WIN32
    m_d->updateTimer->stop();
    m_d->dummyTransparentWindow.setVisible(false);
#endif
    releaseKeyboard();
    setMouseTracking(false);
    m_d->lblScreenColorInfo->setText(QLatin1String("\n"));
    //emit enable signal
    m_d->screenColorPickerButton->setDisabled(false);
}

void KisScreenColorPicker::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
}

void KisScreenColorPicker::updateColorPicking()
{
    static QPoint lastGlobalPos;
    QPoint newGlobalPos = QCursor::pos();
    if (lastGlobalPos == newGlobalPos)
        return;
    lastGlobalPos = newGlobalPos;

    if (!rect().contains(mapFromGlobal(newGlobalPos))) { // Inside the dialog mouse tracking works, handleColorPickingMouseMove will be called
        continueUpdateColorPicking(newGlobalPos);
#ifdef Q_OS_WIN32
        m_d->dummyTransparentWindow.setPosition(newGlobalPos);
#endif
    }
}

void KisScreenColorPicker::continueUpdateColorPicking(const QPoint &globalPos)
{
    const KoColor color = grabScreenColor(globalPos);
    // QTBUG-39792, do not change standard, custom color selectors while moving as
    // otherwise it is not possible to pre-select a custom cell for assignment.
    setCurrentColor(color);
    updateColorLabelText(globalPos);

}

// Event filter to be installed on the dialog while in color-picking mode.
KisScreenColorPickingEventFilter::KisScreenColorPickingEventFilter(KisScreenColorPicker *w, QObject *parent)
    : QObject(parent)
    , m_w(w)
{}

bool KisScreenColorPickingEventFilter::eventFilter(QObject *, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseMove:
        return m_w->handleColorPickingMouseMove(static_cast<QMouseEvent *>(event));
    case QEvent::MouseButtonRelease:
        return m_w->handleColorPickingMouseButtonRelease(static_cast<QMouseEvent *>(event));
    case QEvent::KeyPress:
        return m_w->handleColorPickingKeyPress(static_cast<QKeyEvent *>(event));
    default:
        break;
    }
    return false;
}

// Register the color picker factory with the internal color selector
struct ColorPickerRegistrar {
    ColorPickerRegistrar()
    {
        KisDlgInternalColorSelector::setScreenColorPickerFactory(KisScreenColorPicker::createScreenColorPicker);
    }
};

static ColorPickerRegistrar s_colorPickerRegistrar;

