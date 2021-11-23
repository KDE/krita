/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2006 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2006 C. Boemann Rasmussen <cbo@boemann.dk>
   SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoPointerEvent.h"
#include <QTabletEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <cmath>
#include <boost/variant2/variant.hpp>

namespace detail {

// Qt's events do not have copy-ctors yet, so we should emulate them
// See https://bugreports.qt.io/browse/QTBUG-72488

template <class Event> void copyEventHack(const Event *src, QScopedPointer<QEvent> &dst);

template<> void copyEventHack(const QMouseEvent *src, QScopedPointer<QEvent> &dst) {
    QMouseEvent *tmp = new QMouseEvent(src->type(),
                                       src->localPos(), src->windowPos(), src->screenPos(),
                                       src->button(), src->buttons(), src->modifiers(),
                                       src->source());
    tmp->setTimestamp(src->timestamp());
    dst.reset(tmp);
}

template<> void copyEventHack(const QTabletEvent *src, QScopedPointer<QEvent> &dst) {
    QTabletEvent *tmp = new QTabletEvent(src->type(),
                                         src->posF(), src->globalPosF(),
                                         src->device(), src->pointerType(),
                                         src->pressure(),
                                         src->xTilt(), src->yTilt(),
                                         src->tangentialPressure(),
                                         src->rotation(),
                                         src->z(),
                                         src->modifiers(),
                                         src->uniqueId(),
                                         src->button(), src->buttons());
    tmp->setTimestamp(src->timestamp());
    dst.reset(tmp);
}

template<> void copyEventHack(const QTouchEvent *src, QScopedPointer<QEvent> &dst) {
    QTouchEvent *tmp = new QTouchEvent(src->type(),
                                       src->device(),
                                       src->modifiers(),
                                       src->touchPointStates(),
                                       src->touchPoints());
    tmp->setTimestamp(src->timestamp());
    dst.reset(tmp);
}

}

class Q_DECL_HIDDEN KoPointerEvent::Private
{
public:
    template <typename Event>
    Private(Event *event)
        : eventPtr(event)
    {
    }

    boost::variant2::variant<QMouseEvent*, QTabletEvent*, QTouchEvent*> eventPtr;
};

KoPointerEvent::KoPointerEvent(QMouseEvent *ev, const QPointF &pnt)
    : point(pnt),
      d(new Private(ev))
{
}

KoPointerEvent::KoPointerEvent(QTabletEvent *ev, const QPointF &pnt)
    : point(pnt),
      d(new Private(ev))
{
}

KoPointerEvent::KoPointerEvent(QTouchEvent* ev, const QPointF &pnt)
    : point(pnt),
      d(new Private(ev))
{
}

KoPointerEvent::KoPointerEvent(KoPointerEvent *event, const QPointF &point)
    : point(point)
    , d(new Private(*(event->d)))
{
}

KoPointerEvent::KoPointerEvent(const KoPointerEvent &rhs)
    : point(rhs.point)
    , d(new Private(*(rhs.d)))
{
}

KoPointerEvent &KoPointerEvent::operator=(const KoPointerEvent &rhs)
{
    if (&rhs != this) {
        *d = *rhs.d;
        point = rhs.point;
    }

    return *this;
}

KoPointerEvent::~KoPointerEvent()
{
}

template <typename Event>
KoPointerEventWrapper::KoPointerEventWrapper(Event *_event, const QPointF &point)
    : event(_event, point),
      baseQtEvent(QSharedPointer<QEvent>(static_cast<QEvent*>(_event)))
{
}

struct DeepCopyVisitor
{
    QPointF point;

    template <typename T>
    KoPointerEventWrapper operator() (const T *event) {
        QScopedPointer<QEvent> baseEvent;
        detail::copyEventHack(event, baseEvent);
        return {static_cast<T*>(baseEvent.take()), point};
    }
};

KoPointerEventWrapper KoPointerEvent::deepCopyEvent() const
{
    return visit(DeepCopyVisitor{point}, d->eventPtr);
}

Qt::MouseButton KoPointerEvent::button() const
{
    struct Visitor {
        Qt::MouseButton operator() (const QMouseEvent *event) {
            return event->button();
        }
        Qt::MouseButton operator() (const QTabletEvent *event) {
            return event->button();
        }
        Qt::MouseButton operator() (const QTouchEvent *) {
            return Qt::LeftButton;
        }
    };

    return visit(Visitor(), d->eventPtr);
}

Qt::MouseButtons KoPointerEvent::buttons() const
{
    struct Visitor {
        Qt::MouseButtons operator() (const QMouseEvent *event) {
            return event->buttons();
        }
        Qt::MouseButtons operator() (const QTabletEvent *event) {
            return event->buttons();
        }
        Qt::MouseButtons operator() (const QTouchEvent *) {
            return Qt::LeftButton;
        }
    };

    return visit(Visitor(), d->eventPtr);
}

QPoint KoPointerEvent::globalPos() const
{
    struct Visitor {
        QPoint operator() (const QMouseEvent *event) {
            return event->globalPos();
        }
        QPoint operator() (const QTabletEvent *event) {
            return event->globalPos();
        }
        QPoint operator() (const QTouchEvent *) {
            return QPoint();
        }
    };

    return visit(Visitor(), d->eventPtr);
}

QPoint KoPointerEvent::pos() const
{
    struct Visitor {
        QPoint operator() (const QMouseEvent *event) {
            return event->pos();
        }
        QPoint operator() (const QTabletEvent *event) {
            return event->pos();
        }
        QPoint operator() (const QTouchEvent *event) {
            return event->touchPoints().at(0).pos().toPoint();
        }
    };

    return visit(Visitor(), d->eventPtr);
}

int KoPointerEvent::x() const
{
    return pos().x();
}

int KoPointerEvent::y() const
{
    return pos().y();
}

qreal KoPointerEvent::pressure() const
{
    struct Visitor {
        qreal operator() (const QTabletEvent *event) {
            return event->pressure();
        }
        qreal operator() (...) {
            return 1.0;
        }
    };

    return visit(Visitor(), d->eventPtr);
}

qreal KoPointerEvent::rotation() const
{
    struct Visitor {
        qreal operator() (const QTabletEvent *event) {
            return event->rotation();
        }
        qreal operator() (...) {
            return 0.0;
        }
    };

    return visit(Visitor(), d->eventPtr);
}

qreal KoPointerEvent::tangentialPressure() const
{
    struct Visitor {
        qreal operator() (const QTabletEvent *event) {
            return std::fmod((event->tangentialPressure() - (-1.0)) / (1.0 - (-1.0)), 2.0);
        }
        qreal operator() (...) {
            return 0.0;
        }
    };

    return visit(Visitor(), d->eventPtr);
}

int KoPointerEvent::xTilt() const
{
    struct Visitor {
        int operator() (const QTabletEvent *event) {
            return event->xTilt();
        }
        int operator() (...) {
            return 0;
        }
    };

    return visit(Visitor(), d->eventPtr);
}


int KoPointerEvent::yTilt() const
{
    struct Visitor {
        int operator() (const QTabletEvent *event) {
            return event->yTilt();
        }
        int operator() (...) {
            return 0;
        }
    };

    return visit(Visitor(), d->eventPtr);
}

int KoPointerEvent::z() const
{
    struct Visitor {
        int operator() (const QTabletEvent *event) {
            return event->z();
        }
        int operator() (...) {
            return 0;
        }
    };

    return visit(Visitor(), d->eventPtr);
}

ulong KoPointerEvent::time() const
{
    struct Visitor {
        ulong operator() (const QInputEvent *event) {
            return event->timestamp();
        }
    };

    return visit(Visitor(), d->eventPtr);
}

bool KoPointerEvent::isTabletEvent()
{
    return d->eventPtr.index() == 1;
}

Qt::KeyboardModifiers KoPointerEvent::modifiers() const
{
    struct Visitor {
        Qt::KeyboardModifiers operator() (const QInputEvent *event) {
            return event->modifiers();
        }
    };

    return visit(Visitor(), d->eventPtr);
}

void KoPointerEvent::accept()
{
    struct Visitor {
        void operator() (QInputEvent *event) {
            event->accept();
        }
    };

    return visit(Visitor(), d->eventPtr);
}

void KoPointerEvent::ignore()
{
    struct Visitor {
        void operator() (QInputEvent *event) {
            event->ignore();
        }
    };

    return visit(Visitor(), d->eventPtr);
}

bool KoPointerEvent::isAccepted() const
{
    struct Visitor {
        bool operator() (const QInputEvent *event) {
            return event->isAccepted();
        }
    };

    return visit(Visitor(), d->eventPtr);
}

bool KoPointerEvent::spontaneous() const
{
    struct Visitor {
        bool operator() (const QInputEvent *event) {
            return event->spontaneous();
        }
    };

    return visit(Visitor(), d->eventPtr);
}

void KoPointerEvent::copyQtPointerEvent(const QMouseEvent *event, QScopedPointer<QEvent> &dst)
{
    detail::copyEventHack(event, dst);
}

void KoPointerEvent::copyQtPointerEvent(const QTabletEvent *event, QScopedPointer<QEvent> &dst)
{
    detail::copyEventHack(event, dst);
}

void KoPointerEvent::copyQtPointerEvent(const QTouchEvent *event, QScopedPointer<QEvent> &dst)
{
    detail::copyEventHack(event, dst);
}
