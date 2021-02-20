/*
  SPDX-FileCopyrightText: 2006 Gábor Lehel <illissius@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "KoItemToolTip.h"

#include <QApplication>
#include <QBasicTimer>
#include <QDesktopWidget>
#include <QModelIndex>
#include <QPainter>
#include <QPaintEvent>
#include <QPersistentModelIndex>
#include <QStyleOptionViewItem>
#include <QTextDocument>
#include <QTimerEvent>
#include <QToolTip>

class Q_DECL_HIDDEN KoItemToolTip::Private
{
    public:
        QTextDocument *document;
        QPersistentModelIndex index;
        QPoint pos;
        QBasicTimer timer;

        Private(): document(0) { }
};

KoItemToolTip::KoItemToolTip()
    : d(new Private)
{
    d->document = new QTextDocument(this);
    setWindowFlags(Qt::FramelessWindowHint  | Qt::ToolTip
                  | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    QApplication::instance()->installEventFilter(this);
}

KoItemToolTip::~KoItemToolTip()
{
    delete d;
}

void KoItemToolTip::showTip(QWidget *widget, const QPoint &pos, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QTextDocument *doc = createDocument(index);

    QPoint p = (isVisible() && index == d->index) ? d->pos : pos;

    if (!isVisible() || index != d->index || doc->toHtml() != d->document->toHtml())
    {
        d->pos = p;
        d->index = index;
        delete d->document;
        d->document = doc;
        updatePosition(widget, p, option);
        if (!isVisible())
            show();
        else
            update();
        d->timer.start(10000, this);
    }
    else
        delete doc;
}

void KoItemToolTip::updatePosition(QWidget *widget, const QPoint &pos, const QStyleOptionViewItem &option)
{
    const QRect drect = QApplication::desktop()->availableGeometry(widget);
    const QSize size = sizeHint();
    const int width = size.width(), height = size.height();
    const QPoint gpos = widget->mapToGlobal(pos);
    const QRect irect(widget->mapToGlobal(option.rect.topLeft()), option.rect.size());

    int y = gpos.y() + 20;
    if (y + height > drect.bottom())
        y = qMax(drect.top(), irect.top() - height);

    int x;
    if (gpos.x() + width < drect.right())
        x = gpos.x();
    else
        x = qMax(drect.left(), gpos.x() - width);

    move(x, y);

    resize(sizeHint());
 }

QSize KoItemToolTip::sizeHint() const
{
    return d->document->size().toSize();
}

void KoItemToolTip::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    d->document->drawContents(&p, rect());
    p.drawRect(0, 0, width() - 1, height() - 1);
}

void KoItemToolTip::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == d->timer.timerId()) {
        hide();
    }
}

bool KoItemToolTip::eventFilter(QObject *object, QEvent *event)
{
    switch(event->type())
    {
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::Enter:
        case QEvent::Leave:
            hide();
        default: break;
    }

    return QFrame::eventFilter(object, event);
}
