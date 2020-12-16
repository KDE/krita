/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2014 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */

#include "CurveEditorItem.h"

#include "kis_curve_widget.h"
#include <QPainter>
#include <QMouseEvent>
#include <kis_debug.h>

class CurveEditorItem::Private
{
public:
    Private(CurveEditorItem* qq)
        : q(qq)
    {
        curveWidget = new KisCurveWidget();
    }
    ~Private()
    {
        delete curveWidget;
    }
    void repaint();

    CurveEditorItem* q;
    KisCurveWidget* curveWidget;
    QImage contents;

};

CurveEditorItem::CurveEditorItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
    , d(new Private(this))
{
    setFlag(QQuickItem::ItemHasContents, true);
    setAcceptedMouseButtons( Qt::LeftButton | Qt::RightButton | Qt::MidButton );
    connect(d->curveWidget, SIGNAL(pointSelectedChanged()), SIGNAL(pointSelectedChanged()));
    connect(d->curveWidget, SIGNAL(modified()), SIGNAL(curveChanged()));
    qRegisterMetaType<KisCubicCurve>();
}

CurveEditorItem::~CurveEditorItem()
{
    delete d;
}

void CurveEditorItem::paint(QPainter* p)
{
    p->drawImage(boundingRect(), d->contents);
}

KisCubicCurve CurveEditorItem::curve() const
{
    return d->curveWidget->curve();
}

void CurveEditorItem::setCurve(KisCubicCurve curve)
{
    d->curveWidget->setCurve(curve);
    emit curveChanged();
}

bool CurveEditorItem::pointSelected() const
{
    return d->curveWidget->pointSelected();
}

void CurveEditorItem::deleteSelectedPoint()
{
    if(pointSelected()) {
        QKeyEvent* event = new QKeyEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
        d->curveWidget->keyPressEvent(event);
        d->repaint();
    }
}

void CurveEditorItem::Private::repaint()
{
    curveWidget->resize(q->boundingRect().size().toSize());
    contents = QImage(q->boundingRect().size().toSize(), QImage::Format_ARGB32_Premultiplied);
    contents.fill(Qt::blue);
    curveWidget->render(&contents);
    q->update();
}

void CurveEditorItem::geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry)
{
    d->repaint();
    QQuickPaintedItem::geometryChanged(newGeometry, oldGeometry);
}

void CurveEditorItem::mousePressEvent(QMouseEvent* event)
{
    QMouseEvent* mouseEvent = new QMouseEvent(event->type(), event->pos(), event->button(), event->buttons(), event->modifiers());
    d->curveWidget->mousePressEvent(mouseEvent);
    if(mouseEvent->isAccepted()) {
        event->accept();
    }
    d->repaint();
}

void CurveEditorItem::mouseMoveEvent(QMouseEvent* event)
{
    QMouseEvent* mouseEvent = new QMouseEvent(event->type(), event->pos(), event->button(), event->buttons(), event->modifiers());
    d->curveWidget->mouseMoveEvent(mouseEvent);
    if(mouseEvent->isAccepted()) {
        event->accept();
    }
    d->repaint();
}

void CurveEditorItem::mouseReleaseEvent(QMouseEvent* event)
{
    QMouseEvent* mouseEvent = new QMouseEvent(event->type(), event->pos(), event->button(), event->buttons(), event->modifiers());
    d->curveWidget->mouseReleaseEvent(mouseEvent);
    if(mouseEvent->isAccepted()) {
        event->accept();
    }
    d->repaint();
}

