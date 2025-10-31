/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisQQuickPopupWidget.h"

#include <KisQQuickWidget.h>
#include <QQmlComponent>
#include <QQmlError>
#include <QHBoxLayout>

struct KisQQuickPopupWidget::Private {
    Private(QWidget *parent) : quickWidget(new KisQQuickWidget(parent)) {}
    KisQQuickWidget *quickWidget;

};

KisQQuickPopupWidget::KisQQuickPopupWidget(QWidget *parent)
    : QFrame(parent)
    , d(new Private(this))
{
    setWindowFlags(Qt::Popup);
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    QHBoxLayout *layout = new QHBoxLayout(this);
    d->quickWidget->setSource(QUrl("qrc:/KisQQuickPopupWidget.qml"));
    connect(d->quickWidget, SIGNAL(statusChanged(QQuickWidget::Status)), this, SLOT(emitRootObjectReady()));

    if (!d->quickWidget->errors().empty()) {
        qWarning() << "Errors in Popup:" << d->quickWidget->errors();
    }

    d->quickWidget->setResizeMode(QQuickWidget::SizeViewToRootObject);

    layout->addWidget(d->quickWidget);
    layout->setContentsMargins(0, 0, 0, 0);
}

KisQQuickPopupWidget::~KisQQuickPopupWidget()
{

}

QQuickItem *KisQQuickPopupWidget::rootObject() const
{
    if (d->quickWidget->rootObject()) {
        return d->quickWidget->rootObject();
    }
    qWarning() << "No Root Object!";
    return nullptr;
}

QMargins KisQQuickPopupWidget::layoutContentMargins() const
{
    return this->layout()->contentsMargins();
}

void KisQQuickPopupWidget::setLayoutMargins(const QMargins margins)
{
    this->layout()->setContentsMargins(margins);
}

void KisQQuickPopupWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Cancel)) {
        event->accept();
        hide();
    } else {
        QFrame::keyPressEvent(event);
    }
}

bool KisQQuickPopupWidget::event(QEvent *e)
{
    if (e->type() == QEvent::Close) {
        e->ignore();
        hide();
        return true;
    }
    return QFrame::event(e);
}

void KisQQuickPopupWidget::emitRootObjectReady()
{
    if (d->quickWidget->status() == QQuickWidget::Ready) {
        Q_EMIT signalRootObjectReady();
    }
}
