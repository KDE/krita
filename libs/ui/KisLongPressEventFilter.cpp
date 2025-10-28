// SPDX-License-Identifier: GPL-3.0-or-later
#include "KisLongPressEventFilter.h"
#include <QAbstractButton>
#include <QAbstractScrollArea>
#include <QAbstractSlider>
#include <QAbstractSpinBox>
#include <QApplication>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QStyleHints>
#include <QTimer>
#ifdef Q_OS_ANDROID
#include <QtAndroid>
#endif

KisLongPressEventFilter::KisLongPressEventFilter(QObject *parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setTimerType(Qt::CoarseTimer);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &KisLongPressEventFilter::triggerLongPress);
}

bool KisLongPressEventFilter::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
        handleMousePress(qobject_cast<QWidget *>(watched), static_cast<QMouseEvent *>(event));
        break;
    case QEvent::MouseMove:
        handleMouseMove(static_cast<QMouseEvent *>(event));
        break;
    case QEvent::MouseButtonRelease:
        cancel();
        break;
    default:
        break;
    }
    return QObject::eventFilter(watched, event);
}

void KisLongPressEventFilter::handleMousePress(QWidget *target, const QMouseEvent *me)
{
    if (isContextMenuTarget(target)) {
        const QStyleHints *sh = qApp->styleHints();
        long long distance = qMax(MINIMUM_DISTANCE, sh->startDragDistance());
        m_distanceSquared = distance * distance;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        m_pressLocalPos = me->pos();
        m_pressGlobalPos = me->globalPos();
#else
        m_pressLocalPos = me->position().toPoint();
        m_pressGlobalPos = me->globalPosition().toPoint();
#endif
        m_target = target;
#ifdef Q_OS_ANDROID
        int longPressInterval =
            QAndroidJniObject::callStaticMethod<jint>("org/krita/android/MainActivity", "getLongPressTimeout", "()I");
#else
        int longPressInterval = sh->mousePressAndHoldInterval();
#endif
        m_timer->start(qMax(MINIMUM_DELAY, longPressInterval));
    } else {
        cancel();
    }
}

void KisLongPressEventFilter::handleMouseMove(const QMouseEvent *me)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QPoint globalPos = me->globalPos();
#else
    QPoint globalPos = me->globalPosition().toPoint();
#endif
    if (m_timer->isActive() && !isWithinDistance(globalPos)) {
        cancel();
    }
}

void KisLongPressEventFilter::cancel()
{
    m_timer->stop();
    m_target.clear();
}

bool KisLongPressEventFilter::isWithinDistance(const QPoint &globalPos) const
{
    long long x = globalPos.x() - m_pressGlobalPos.x();
    long long y = globalPos.y() - m_pressGlobalPos.y();
    return (x * x) + (y * y) <= m_distanceSquared;
}

void KisLongPressEventFilter::triggerLongPress()
{
    QWidget *target = m_target.data();
    if (isContextMenuTarget(target)) {
        qApp->postEvent(target, new QContextMenuEvent(QContextMenuEvent::Mouse, m_pressLocalPos, m_pressGlobalPos));
    }
}

bool KisLongPressEventFilter::isContextMenuTarget(QWidget *target)
{
    while (target && target->isVisible() && isLongPressableWidget(target)) {
        switch (target->contextMenuPolicy()) {
        case Qt::NoContextMenu:
            target = target->parentWidget();
            break;
        case Qt::PreventContextMenu:
            return false;
        default:
            return true;
        }
    }
    return false;
}

bool KisLongPressEventFilter::isLongPressableWidget(QWidget *target)
{
    QVariant prop = target->property(ENABLED_PROPERTY);
    if (prop.isValid()) {
        return prop.toBool();
    }

    // Several widget types don't have desirable long-press behavior. If they're
    // our own widgets, we should just fix that in the widget, but we can"t
    // really sensibly do that to Qt's widgets. Luckily however, those widgets
    // really shouldn't have context menus in the first place, so we can just
    // disregard them as long-press targets and be done with it.

    // Buttons get depressed when you click and hold on them and won't pop back
    // up if a context menu is opened during that. They may also have a menu
    // attached to them that the user operates like a context menu.
    if (qobject_cast<QAbstractButton *>(target)) {
        return false;
    }

    // Slightly non-obvious, but this thing has many child classes, like text
    // areas or the canvas, none of which have sensible context menus.
    if (qobject_cast<QAbstractScrollArea *>(target)) {
        return false;
    }

    // Sliders (including scrollbars) may get dragged slowly.
    if (qobject_cast<QAbstractSlider *>(target)) {
        return false;
    }

    // Spinners may get held down to keep a number spinning.
    if (qobject_cast<QAbstractSpinBox *>(target)) {
        return false;
    }

    // Combo boxes get held down to pick an item.
    if (qobject_cast<QComboBox *>(target)) {
        return false;
    }

    // Text editing will have its own long-press handling.
    if (qobject_cast<QLineEdit *>(target)) {
        return false;
    }

    // Menus don't have context menus.
    if (qobject_cast<QMenu *>(target)) {
        return false;
    }

    // Menu bars don't have context menus either.
    if (qobject_cast<QMenuBar *>(target)) {
        return false;
    }

    return true;
}
